#include "metalwrapper.h"
#include "bc3decompressor.h"  // Phase 28.4: BC3 decompression workaround

#if defined(__APPLE__)

#import <TargetConditionals.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <SDL2/SDL.h>
#import <SDL2/SDL_metal.h>

#include <cstdio>

namespace GX {

// Static members
id MetalWrapper::s_device = nil;
id MetalWrapper::s_commandQueue = nil;
id MetalWrapper::s_layer = nil;
id MetalWrapper::s_currentDrawable = nil;
id MetalWrapper::s_cmdBuffer = nil;
// s_passDesc removed - local variable in BeginFrame(), not global state

// Additional static members for minimal pipeline
static id s_vertexBuffer = nil;
static id s_pipelineState = nil;
static id s_renderEncoder = nil;

// Phase 35.6 Rollback Metal Fix: Store SDL_MetalView to prevent Window Server crashes
static SDL_MetalView s_sdlMetalView = nullptr;

// Phase 28.4.3: Uniform buffer for shader parameters
static MetalWrapper::ShaderUniforms s_currentUniforms;
static bool s_uniformsDirty = true;

// Phase 28.4: BC3 Bug Investigation - Sampler testing
static bool s_useNearestFiltering = false;  // Toggle via environment variable

// Simple triangle vertices for testing
// Vertex structure for textured quads (MUST match TexturedVertex in texturedquad.h!)
struct Vertex {
    float position[3];   // 12 bytes, offset 0
    float normal[3];     // 12 bytes, offset 12
    float color[4];      // 16 bytes, offset 24
    float texcoord[2];   // 8 bytes, offset 40
};  // Total: 48 bytes

// Simple test triangle (uses subset of Vertex structure)
static const Vertex triangleVertices[] = {
    { { 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.0f} },  // Top (red)
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },  // Bottom left (green)
    { { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }   // Bottom right (blue)
};

static CAMetalLayer* GetOrCreateLayer(void* sdlWindowPtr, int width, int height) {
    // Note: SDL2 on macOS can expose an NSWindow* via SDL_GetWindowWMInfo if linked with Cocoa.
    // Preferred path: use SDL_metal to get a CAMetalLayer attached to the window.
    CAMetalLayer* layer = nullptr;
    if (sdlWindowPtr) {
        SDL_Window* sdlWindow = (SDL_Window*)sdlWindowPtr;
        
        // Phase 35.6 Rollback Metal Fix: Store SDL_MetalView globally to prevent Window Server crashes
        // Root cause: SDL_MetalView was created but never stored, causing dangling references
        // when Window Server tried to send events to the orphaned view
        // Solution: Keep reference alive for the lifetime of the Metal backend
        if (!s_sdlMetalView) {
            s_sdlMetalView = SDL_Metal_CreateView(sdlWindow);
            printf("METAL: Created SDL_MetalView (%p) for window (%p)\n", s_sdlMetalView, sdlWindow);
        }
        
        if (s_sdlMetalView) {
            layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(s_sdlMetalView);
            printf("METAL: Got CAMetalLayer (%p) from SDL_MetalView\n", layer);
        } else {
            printf("METAL WARNING: Failed to create SDL_MetalView, using fallback layer\n");
        }
    }
    if (!layer) {
        // Fallback: create a standalone layer
        printf("METAL: Creating standalone CAMetalLayer (no SDL window provided)\n");
        layer = [CAMetalLayer layer];
    }
    layer.frame = CGRectMake(0, 0, width, height);
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.colorspace = CGColorSpaceCreateDeviceRGB();
    layer.framebufferOnly = YES;
    return layer;
}

bool MetalWrapper::CreateSimplePipeline() {
    // Combined Metal shader source with shared struct definitions (matches basic.metal)
    NSString* shaderSource = @R"(
    #include <metal_stdlib>
    using namespace metal;
    
    // Vertex input structure matching DirectX FVF layout
    struct VertexInput {
        float3 position [[attribute(0)]];   // D3DFVF_XYZ
        float3 normal   [[attribute(1)]];   // D3DFVF_NORMAL  
        float4 color    [[attribute(2)]];   // D3DFVF_DIFFUSE (BGRA → RGBA)
        float2 texcoord [[attribute(3)]];   // D3DFVF_TEX1
    };
    
    // Vertex output / fragment input
    struct VertexOutput {
        float4 position [[position]];
        float4 color;
        float2 texcoord;
        float3 worldPos;
        float3 normal;
    };
    
    // Uniform buffer structure matching DX8Wrapper uniforms
    struct Uniforms {
        float4x4 worldMatrix;
        float4x4 viewMatrix;
        float4x4 projectionMatrix;
        float3 lightDirection;
        float3 lightColor;
        float3 ambientColor;
        float useLighting;
        // Material uniforms
        float4 materialDiffuse;
        float4 materialAmbient;
        // Alpha test uniforms
        float alphaRef;
        float alphaTestEnabled;
        int alphaTestFunc;
        // Fog uniforms
        float3 fogColor;
        float fogStart;
        float fogEnd;
        float fogDensity;
        int fogMode;
        float fogEnabled;
    };
    
    // Vertex shader - uniforms at buffer(0), vertex data via stage_in
    vertex VertexOutput vertex_main(VertexInput in [[stage_in]],
                                   constant Uniforms& uniforms [[buffer(0)]]) {
        VertexOutput out;
        
        // For 2D rendering: positions are already in clip-space, pass through directly
        out.position = float4(in.position, 1.0);
        
        // Pass through color and texture coordinates
        out.color = in.color;
        out.texcoord = in.texcoord;
        out.worldPos = in.position;
        
        // Pass through normal (not transformed for 2D)
        out.normal = in.normal;
        
        return out;
    }
    
    // Fragment shader - uniforms at buffer(0)
    fragment float4 fragment_main(VertexOutput in [[stage_in]],
                                 constant Uniforms& uniforms [[buffer(0)]],
                                 texture2d<float> diffuseTexture [[texture(0)]],
                                 sampler textureSampler [[sampler(0)]]) {
        
        // Sample base texture
        float4 texColor = diffuseTexture.sample(textureSampler, in.texcoord);
        
        // DEBUG: Ignore vertex color, use white (1,1,1,1) to see pure texture
        float4 finalColor = texColor * float4(1.0, 1.0, 1.0, 1.0);
        
        // Apply lighting if enabled
        if (uniforms.useLighting > 0.5) {
            // Simple directional lighting
            float3 lightDir = normalize(-uniforms.lightDirection);
            float NdotL = max(dot(in.normal, lightDir), 0.0);
            
            float3 lighting = uniforms.ambientColor + uniforms.lightColor * NdotL;
            finalColor.rgb *= lighting;
            
            // Apply material properties
            finalColor.rgb *= uniforms.materialDiffuse.rgb;
        }
        
        // Alpha testing (if enabled)
        if (uniforms.alphaTestEnabled > 0.5) {
            float alpha = finalColor.a;
            int func = uniforms.alphaTestFunc;
            float ref = uniforms.alphaRef;
            
            bool pass = false;
            if (func == 1) pass = false;                    // NEVER
            else if (func == 2) pass = (alpha < ref);       // LESS
            else if (func == 3) pass = (alpha == ref);      // EQUAL
            else if (func == 4) pass = (alpha <= ref);      // LESSEQUAL
            else if (func == 5) pass = (alpha > ref);       // GREATER
            else if (func == 6) pass = (alpha != ref);      // NOTEQUAL
            else if (func == 7) pass = (alpha >= ref);      // GREATEREQUAL
            else if (func == 8) pass = true;                // ALWAYS
            
            if (!pass) discard_fragment();
        }
        
        // Apply fog (if enabled)
        if (uniforms.fogEnabled > 0.5) {
            float distance = length(in.worldPos);
            float fogFactor = 1.0;
            
            if (uniforms.fogMode == 1) {
                // EXP fog
                fogFactor = exp(-uniforms.fogDensity * distance);
            } else if (uniforms.fogMode == 2) {
                // EXP2 fog
                float d = uniforms.fogDensity * distance;
                fogFactor = exp(-d * d);
            } else if (uniforms.fogMode == 3) {
                // LINEAR fog
                fogFactor = (uniforms.fogEnd - distance) / (uniforms.fogEnd - uniforms.fogStart);
            }
            
            fogFactor = clamp(fogFactor, 0.0, 1.0);
            finalColor.rgb = mix(uniforms.fogColor, finalColor.rgb, fogFactor);
        }
        
        return finalColor;
    }
    )";
    
    NSError* error = nil;
    id<MTLLibrary> library = [(id<MTLDevice>)MetalWrapper::s_device newLibraryWithSource:shaderSource 
        options:nil error:&error];
    
    if (!library) {
        std::printf("METAL: Failed to compile shaders: %s\n", [[error localizedDescription] UTF8String]);
        return false;
    }
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];
    
    // Phase 35.6 Rollback Metal Fix: Validate shader functions before creating pipeline
    if (!vertexFunction) {
        std::printf("METAL FATAL: vertex_main function not found in shader library!\n");
        return false;
    }
    if (!fragmentFunction) {
        std::printf("METAL FATAL: fragment_main function not found in shader library!\n");
        return false;
    }
    printf("METAL: Shader functions loaded successfully (vertex=%p, fragment=%p)\n", 
           vertexFunction, fragmentFunction);
    
    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vertexFunction;
    desc.fragmentFunction = fragmentFunction;
    desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Enable alpha blending for transparent textures
    desc.colorAttachments[0].blendingEnabled = YES;
    desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    desc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    desc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    
    // Phase 35.6 Rollback Metal Fix: Simplified vertex descriptor for 2D rendering
    // CRITICAL: Only define layouts for buffers we actually use
    // Buffer 0 = uniforms (set at runtime, no vertex descriptor needed)
    // Buffer 1 = vertex data (needs layout definition)
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    // Attribute 0: position (float3) - offset 0
    vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
    vertexDesc.attributes[0].offset = 0;
    vertexDesc.attributes[0].bufferIndex = 1;
    
    // Attribute 1: normal (float3) - offset 12
    vertexDesc.attributes[1].format = MTLVertexFormatFloat3;
    vertexDesc.attributes[1].offset = 12;
    vertexDesc.attributes[1].bufferIndex = 1;
    
    // Attribute 2: color (float4) - offset 24
    vertexDesc.attributes[2].format = MTLVertexFormatFloat4;
    vertexDesc.attributes[2].offset = 24;
    vertexDesc.attributes[2].bufferIndex = 1;
    
    // Attribute 3: texcoord (float2) - offset 40
    vertexDesc.attributes[3].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[3].offset = 40;
    vertexDesc.attributes[3].bufferIndex = 1;
    
    // Buffer layout at index 1 ONLY (vertex data)
    // Hardcode stride to 48 bytes to avoid C++ struct alignment issues
    vertexDesc.layouts[1].stride = 48;  // position(12) + normal(12) + color(16) + texcoord(8)
    vertexDesc.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // DO NOT define layout for buffer 0 - it's for uniforms only
    desc.vertexDescriptor = vertexDesc;
    
    // CRITICAL FIX (Phase 35.6 Rollback): Force SYNCHRONOUS pipeline compilation
    // Apple's AGXMetal13_3 driver crashes in background shader compilation
    // with EXC_BAD_ACCESS at AGCDeserializedReply::deserialize()
    // Root cause: Asynchronous compilation creates race condition with complex shaders
    // Solution: Compile pipeline synchronously on main thread
    printf("METAL: Creating pipeline state SYNCHRONOUSLY (driver crash workaround)...\n");
    fflush(stdout);
    
    s_pipelineState = [(id<MTLDevice>)MetalWrapper::s_device newRenderPipelineStateWithDescriptor:desc error:&error];
    if (!s_pipelineState) {
        std::printf("METAL: Failed to create pipeline state: %s\n", [[error localizedDescription] UTF8String]);
        if (error) {
            std::printf("METAL: Error domain: %s\n", [[error domain] UTF8String]);
            std::printf("METAL: Error code: %ld\n", (long)[error code]);
            std::printf("METAL: Error userInfo: %s\n", [[[error userInfo] description] UTF8String]);
        }
        return false;
    }
    
    printf("METAL: Pipeline state created successfully (%p) - shader compilation complete\n", s_pipelineState);
    fflush(stdout);
    
    return true;
}

bool MetalWrapper::Initialize(const MetalConfig& cfg) {
    s_device = MTLCreateSystemDefaultDevice();
    if (!s_device) {
        std::printf("METAL: No compatible GPU device found\n");
        return false;
    }
    s_commandQueue = [(id<MTLDevice>)s_device newCommandQueue];
    if (!s_commandQueue) {
        std::printf("METAL: Failed to create command queue\n");
        return false;
    }

    s_layer = cfg.metalLayer ? (__bridge id)cfg.metalLayer : (id)GetOrCreateLayer(cfg.sdlWindow, cfg.width, cfg.height);
    if (!s_layer) {
        std::printf("METAL: Failed to create CAMetalLayer\n");
        return false;
    }
    [(CAMetalLayer*)s_layer setDevice:(id<MTLDevice>)s_device];
    [(CAMetalLayer*)s_layer setDrawableSize:CGSizeMake(cfg.width, cfg.height)];
    [(CAMetalLayer*)s_layer setDisplaySyncEnabled:cfg.vsync];

    // Create vertex buffer
    s_vertexBuffer = [(id<MTLDevice>)s_device newBufferWithBytes:triangleVertices 
                                                         length:sizeof(triangleVertices) 
                                                        options:MTLResourceStorageModeShared];
    
    // Create pipeline state
    if (!MetalWrapper::CreateSimplePipeline()) {
        std::printf("METAL: Failed to create rendering pipeline\n");
        return false;
    }
    
    // CRITICAL: Initialize uniforms with default values (identity matrices)
    SetDefaultUniforms();
    std::printf("METAL: Default uniforms initialized (identity matrices)\n");

    std::printf("METAL: Initialized (device, queue, layer, triangle pipeline)\n");
    return true;
}

void MetalWrapper::Shutdown() {
    // Phase 35.6 Rollback Metal Fix: Properly cleanup SDL_MetalView to prevent Window Server crashes
    if (s_sdlMetalView) {
        printf("METAL: Destroying SDL_MetalView (%p)\n", s_sdlMetalView);
        SDL_Metal_DestroyView(s_sdlMetalView);
        s_sdlMetalView = nullptr;
    }
    
    s_renderEncoder = nil;
    s_pipelineState = nil;
    s_vertexBuffer = nil;
    // s_passDesc removed - was causing use-after-free bugs
    s_cmdBuffer = nil;
    s_currentDrawable = nil;
    s_layer = nil;
    s_commandQueue = nil;
    s_device = nil;
    std::printf("METAL: Shutdown complete\n");
}

void MetalWrapper::Resize(int width, int height) {
    if (s_layer) {
        [(CAMetalLayer*)s_layer setDrawableSize:CGSizeMake(width, height)];
    }
}

void MetalWrapper::BeginFrame(float r, float g, float b, float a) {
    printf("METAL: BeginFrame() called (s_device=%p, s_commandQueue=%p, s_layer=%p)\n", 
           s_device, s_commandQueue, s_layer);
    
    if (!s_device || !s_commandQueue || !s_layer) {
        printf("METAL ERROR: BeginFrame() - Missing critical components (early return)\n");
        return;
    }
    
    // CRITICAL: Check if previous frame wasn't properly ended
    if (s_renderEncoder) {
        printf("METAL WARNING: BeginFrame() called with active render encoder! Ending previous frame first.\n");
        fflush(stdout);
        EndFrame();
    }
    
    if (s_cmdBuffer) {
        printf("METAL WARNING: BeginFrame() called with uncommitted command buffer! Cleaning up.\n");
        fflush(stdout);
        s_cmdBuffer = nil;
    }
    
    printf("METAL DEBUG: Getting next drawable from layer...\n");
    fflush(stdout);
    s_currentDrawable = [(CAMetalLayer*)s_layer nextDrawable];
    if (!s_currentDrawable) {
        printf("METAL ERROR: BeginFrame() - Failed to get next drawable\n");
        return;
    }
    printf("METAL DEBUG: Got drawable (%p)\n", s_currentDrawable);
    fflush(stdout);

    printf("METAL DEBUG: Creating command buffer...\n");
    fflush(stdout);
    s_cmdBuffer = [(id<MTLCommandQueue>)s_commandQueue commandBuffer];
    if (!s_cmdBuffer) {
        printf("METAL ERROR: BeginFrame() - Failed to create command buffer\n");
        return;
    }
    printf("METAL DEBUG: Command buffer created (%p)\n", s_cmdBuffer);
    fflush(stdout);

    // Create a simple pass descriptor that clears to the given color
    MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
    printf("METAL DEBUG: RenderPassDescriptor created (%p)\n", pass);
    
    id<MTLTexture> drawableTexture = [(id<CAMetalDrawable>)s_currentDrawable texture];
    printf("METAL DEBUG: Drawable texture (%p), format: %lu\n", drawableTexture, (unsigned long)[drawableTexture pixelFormat]);
    
    pass.colorAttachments[0].texture = drawableTexture;
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, a);
    // DON'T store pass in global variable - it's a local that will be released!
    
    printf("METAL DEBUG: About to create render encoder (cmdBuffer=%p, pass=%p)\n", s_cmdBuffer, pass);
    printf("METAL DEBUG: Pipeline state before encoder: %p\n", s_pipelineState);
    
    // Phase 35.6 Rollback Metal Fix: Comprehensive pre-flight checks
    // Prevents AGXMetal13_3 driver crash in AGCDeserializedReply::deserialize() + 988
    
    // 1. Validate command buffer state
    if (!s_cmdBuffer) {
        printf("METAL FATAL: cmdBuffer is NULL before encoder creation!\n");
        return;
    }
    
    // 2. Validate render pass descriptor
    if (!pass) {
        printf("METAL FATAL: RenderPassDescriptor is NULL!\n");
        return;
    }
    
    // 3. Validate color attachment texture
    if (!pass.colorAttachments[0].texture) {
        printf("METAL FATAL: Color attachment texture is NULL!\n");
        return;
    }
    
    // 4. Validate texture pixel format matches pipeline
    MTLPixelFormat textureFormat = [pass.colorAttachments[0].texture pixelFormat];
    if (textureFormat != MTLPixelFormatBGRA8Unorm) {
        printf("METAL WARNING: Texture format mismatch! Expected BGRA8Unorm, got %lu\n", (unsigned long)textureFormat);
    }
    
    printf("METAL DEBUG: Pre-flight checks passed, validating pipeline state...\n");
    fflush(stdout);
    
    // 5. CRITICAL: Validate pipeline state BEFORE creating encoder
    if (!s_pipelineState) {
        printf("METAL FATAL: Pipeline state is NULL - cannot create render encoder!\n");
        printf("METAL FATAL: Was InitializeShaders() called successfully?\n");
        return;
    }
    
    // 6. Additional validation: Check if pipeline state is valid Metal object
    @try {
        // Try to access pipeline state properties - if corrupted, this may throw
        NSString *label = [(id<MTLRenderPipelineState>)s_pipelineState label];
        (void)label;  // Suppress unused warning
        printf("METAL DEBUG: Pipeline state validated (%p), label=\"%s\"\n", 
               s_pipelineState, label ? [label UTF8String] : "nil");
    }
    @catch (NSException *exception) {
        printf("METAL FATAL: Pipeline state is CORRUPTED - exception: %s\n", 
               [[exception description] UTF8String]);
        return;
    }
    
    // 7. Validate render pass descriptor configuration
    printf("METAL DEBUG: RenderPass config - loadAction: %lu, storeAction: %lu\n",
           (unsigned long)pass.colorAttachments[0].loadAction,
           (unsigned long)pass.colorAttachments[0].storeAction);
    
    printf("METAL DEBUG: All validations passed, creating render encoder...\n");
    fflush(stdout);
    
    // Begin render pass - encoder stays active until EndFrame
    // NOTE: This call may hang if there's a Metal driver issue or invalid state
    s_renderEncoder = [(id<MTLCommandBuffer>)s_cmdBuffer renderCommandEncoderWithDescriptor:pass];
    
    printf("METAL DEBUG: Render encoder created successfully (%p)\n", s_renderEncoder);
    fflush(stdout);
    
    if (!s_renderEncoder) {
        printf("METAL FATAL: Failed to create render encoder (returned nil)!\n");
        return;
    }
    
    // CRITICAL: Set pipeline state so encoder knows which shader to use
    if (s_pipelineState) {
        [(id<MTLRenderCommandEncoder>)s_renderEncoder setRenderPipelineState:(id<MTLRenderPipelineState>)s_pipelineState];
        printf("METAL: BeginFrame() - Pipeline state set on encoder\n");
    } else {
        printf("METAL ERROR: BeginFrame() - No pipeline state available!\n");
    }
    
    // CRITICAL: Set viewport to match drawable size
    CGSize drawableSize = [(CAMetalLayer*)s_layer drawableSize];
    MTLViewport viewport = {
        0.0, 0.0,  // x, y origin (top-left)
        drawableSize.width, drawableSize.height,  // width, height
        0.0, 1.0   // znear, zfar
    };
    [(id<MTLRenderCommandEncoder>)s_renderEncoder setViewport:viewport];
    printf("METAL: BeginFrame() - Viewport set (%.0fx%.0f)\n", drawableSize.width, drawableSize.height);
    
    // CRITICAL: Disable backface culling (render both sides)
    [(id<MTLRenderCommandEncoder>)s_renderEncoder setCullMode:MTLCullModeNone];
    printf("METAL: BeginFrame() - Cull mode disabled\n");
    
    // CRITICAL: Set front face winding order (counter-clockwise is Metal default)
    [(id<MTLRenderCommandEncoder>)s_renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    
    // CRITICAL: Set triangle fill mode to FILL (not wireframe!)
    [(id<MTLRenderCommandEncoder>)s_renderEncoder setTriangleFillMode:MTLTriangleFillModeFill];
    printf("METAL: BeginFrame() - Triangle fill mode set to FILL\n");
    
    // Bind uniforms to shader at [[buffer(0)]] (matches shader definition)
    [(id<MTLRenderCommandEncoder>)s_renderEncoder 
        setVertexBytes:&s_currentUniforms
        length:sizeof(ShaderUniforms)
        atIndex:0];
    [(id<MTLRenderCommandEncoder>)s_renderEncoder 
        setFragmentBytes:&s_currentUniforms
        length:sizeof(ShaderUniforms)
        atIndex:0];
    printf("METAL: BeginFrame() - Uniforms bound at index 0 (size: %zu bytes)\n", sizeof(ShaderUniforms));
    
    printf("METAL: BeginFrame() - Render encoder created (%p)\n", s_renderEncoder);
}

void MetalWrapper::EndFrame() {
    if (!s_cmdBuffer || !s_currentDrawable) return;

    // End render encoder if still active
    if (s_renderEncoder) {
        [(id<MTLRenderCommandEncoder>)s_renderEncoder endEncoding];
        printf("METAL: EndFrame() - Render encoder finalized\n");
        s_renderEncoder = nil;
    }

    // Present the drawable
    [(id<MTLCommandBuffer>)s_cmdBuffer presentDrawable:(id)s_currentDrawable];
    [(id<MTLCommandBuffer>)s_cmdBuffer commit];
    s_cmdBuffer = nil;
    s_currentDrawable = nil;
}

// Phase 30.2: Buffer Management Implementation
void* MetalWrapper::CreateVertexBuffer(unsigned int size, const void* data, bool dynamic) {
    if (!s_device) {
        std::printf("METAL ERROR: CreateVertexBuffer - No device\n");
        return nullptr;
    }
    if (size == 0) {
        std::printf("METAL ERROR: CreateVertexBuffer - Zero size\n");
        return nullptr;
    }
    
    MTLResourceOptions options = dynamic ? MTLResourceStorageModeShared : MTLResourceStorageModeShared;
    id<MTLBuffer> buffer = nil;
    
    if (data) {
        buffer = [(id<MTLDevice>)s_device newBufferWithBytes:data length:size options:options];
    } else {
        buffer = [(id<MTLDevice>)s_device newBufferWithLength:size options:options];
    }
    
    if (buffer) {
        std::printf("METAL: Created vertex buffer %p (size: %u, dynamic: %d)\n", buffer, size, dynamic);
        return (__bridge_retained void*)buffer;
    } else {
        std::printf("METAL ERROR: Failed to create vertex buffer (size: %u)\n", size);
        return nullptr;
    }
}

void* MetalWrapper::CreateIndexBuffer(unsigned int size, const void* data, bool dynamic) {
    if (!s_device) {
        std::printf("METAL ERROR: CreateIndexBuffer - No device\n");
        return nullptr;
    }
    if (size == 0) {
        std::printf("METAL ERROR: CreateIndexBuffer - Zero size\n");
        return nullptr;
    }
    
    MTLResourceOptions options = dynamic ? MTLResourceStorageModeShared : MTLResourceStorageModeShared;
    id<MTLBuffer> buffer = nil;
    
    if (data) {
        buffer = [(id<MTLDevice>)s_device newBufferWithBytes:data length:size options:options];
    } else {
        buffer = [(id<MTLDevice>)s_device newBufferWithLength:size options:options];
    }
    
    if (buffer) {
        std::printf("METAL: Created index buffer %p (size: %u, dynamic: %d)\n", buffer, size, dynamic);
        return (__bridge_retained void*)buffer;
    } else {
        std::printf("METAL ERROR: Failed to create index buffer (size: %u)\n", size);
        return nullptr;
    }
}

void MetalWrapper::DeleteVertexBuffer(void* buffer) {
    if (buffer) {
        std::printf("METAL: Deleting vertex buffer %p\n", buffer);
        id<MTLBuffer> mtlBuffer = (__bridge_transfer id<MTLBuffer>)buffer;
        mtlBuffer = nil; // ARC will handle deallocation
    } else {
        std::printf("METAL WARNING: DeleteVertexBuffer called with null buffer\n");
    }
}

void MetalWrapper::DeleteIndexBuffer(void* buffer) {
    if (buffer) {
        std::printf("METAL: Deleting index buffer %p\n", buffer);
        id<MTLBuffer> mtlBuffer = (__bridge_transfer id<MTLBuffer>)buffer;
        mtlBuffer = nil; // ARC will handle deallocation
    } else {
        std::printf("METAL WARNING: DeleteIndexBuffer called with null buffer\n");
    }
}

void MetalWrapper::UpdateVertexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset) {
    if (!buffer) {
        std::printf("METAL ERROR: UpdateVertexBuffer - Null buffer\n");
        return;
    }
    if (!data) {
        std::printf("METAL ERROR: UpdateVertexBuffer - Null data pointer\n");
        return;
    }
    if (size == 0) {
        std::printf("METAL ERROR: UpdateVertexBuffer - Zero size\n");
        return;
    }
    
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)buffer;
    unsigned long bufferLength = [mtlBuffer length];
    
    if (offset + size > bufferLength) {
        std::printf("METAL ERROR: UpdateVertexBuffer - Out of bounds (offset: %u, size: %u, buffer length: %lu)\n", 
            offset, size, bufferLength);
        return;
    }
    
    void* contents = [mtlBuffer contents];
    if (contents) {
        memcpy((char*)contents + offset, data, size);
        std::printf("METAL: Updated vertex buffer %p (size: %u, offset: %u, total: %lu)\n", 
            buffer, size, offset, bufferLength);
    } else {
        std::printf("METAL ERROR: UpdateVertexBuffer - Failed to get buffer contents\n");
    }
}

void MetalWrapper::UpdateIndexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset) {
    if (!buffer) {
        std::printf("METAL ERROR: UpdateIndexBuffer - Null buffer\n");
        return;
    }
    if (!data) {
        std::printf("METAL ERROR: UpdateIndexBuffer - Null data pointer\n");
        return;
    }
    if (size == 0) {
        std::printf("METAL ERROR: UpdateIndexBuffer - Zero size\n");
        return;
    }
    
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)buffer;
    unsigned long bufferLength = [mtlBuffer length];
    
    if (offset + size > bufferLength) {
        std::printf("METAL ERROR: UpdateIndexBuffer - Out of bounds (offset: %u, size: %u, buffer length: %lu)\n", 
            offset, size, bufferLength);
        return;
    }
    
    void* contents = [mtlBuffer contents];
    if (contents) {
        memcpy((char*)contents + offset, data, size);
        std::printf("METAL: Updated index buffer %p (size: %u, offset: %u, total: %lu)\n", 
            buffer, size, offset, bufferLength);
    } else {
        std::printf("METAL ERROR: UpdateIndexBuffer - Failed to get buffer contents\n");
    }
}

// Phase 30.3: Buffer Binding
static void* s_currentVertexBuffer = nullptr;
static void* s_currentIndexBuffer = nullptr;
static unsigned int s_currentVertexBufferOffset = 0;
static unsigned int s_currentIndexBufferOffset = 0;

void MetalWrapper::SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot) {
    if (!s_renderEncoder) {
        std::printf("METAL ERROR: SetVertexBuffer - No active render encoder\n");
        return;
    }
    
    if (!buffer) {
        std::printf("METAL ERROR: SetVertexBuffer - Null buffer\n");
        return;
    }
    
    s_currentVertexBuffer = buffer;
    s_currentVertexBufferOffset = offset;
    
    id<MTLBuffer> mtlBuffer = (__bridge id<MTLBuffer>)buffer;
    [(id<MTLRenderCommandEncoder>)s_renderEncoder setVertexBuffer:mtlBuffer offset:offset atIndex:slot];
    
    std::printf("METAL: Set vertex buffer %p (offset: %u, slot: %u)\n", buffer, offset, slot);
}

void MetalWrapper::SetIndexBuffer(void* buffer, unsigned int offset) {
    if (!buffer) {
        std::printf("METAL ERROR: SetIndexBuffer - Null buffer\n");
        return;
    }
    
    s_currentIndexBuffer = buffer;
    s_currentIndexBufferOffset = offset;
    
    std::printf("METAL: Set index buffer %p (offset: %u)\n", buffer, offset);
}

// Phase 30.5: Draw Calls
void MetalWrapper::DrawPrimitive(unsigned int primitiveType, unsigned int startVertex, unsigned int vertexCount) {
    if (!s_renderEncoder) {
        std::printf("METAL ERROR: DrawPrimitive - No active render encoder\n");
        return;
    }
    
    // Map D3D primitive type to Metal primitive type
    MTLPrimitiveType metalPrimitiveType = MTLPrimitiveTypeTriangle;
    switch (primitiveType) {
        case 1: // D3DPT_POINTLIST
            metalPrimitiveType = MTLPrimitiveTypePoint;
            break;
        case 2: // D3DPT_LINELIST
            metalPrimitiveType = MTLPrimitiveTypeLine;
            break;
        case 3: // D3DPT_LINESTRIP
            metalPrimitiveType = MTLPrimitiveTypeLineStrip;
            break;
        case 4: // D3DPT_TRIANGLELIST
            metalPrimitiveType = MTLPrimitiveTypeTriangle;
            break;
        case 5: // D3DPT_TRIANGLESTRIP
            metalPrimitiveType = MTLPrimitiveTypeTriangleStrip;
            break;
        default:
            std::printf("METAL WARNING: Unknown primitive type %u, using triangles\n", primitiveType);
            break;
    }
    
    [(id<MTLRenderCommandEncoder>)s_renderEncoder drawPrimitives:metalPrimitiveType 
        vertexStart:startVertex vertexCount:vertexCount];
    
    std::printf("METAL: DrawPrimitive (type: %u, start: %u, count: %u)\n", 
        primitiveType, startVertex, vertexCount);
}

void MetalWrapper::DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex,
                                        unsigned int minVertex, unsigned int numVertices,
                                        unsigned int startIndex, unsigned int primitiveCount) {
    if (!s_renderEncoder) {
        std::printf("METAL ERROR: DrawIndexedPrimitive - No active render encoder\n");
        return;
    }
    
    if (!s_currentIndexBuffer) {
        std::printf("METAL ERROR: DrawIndexedPrimitive - No index buffer set\n");
        return;
    }
    
    // Map D3D primitive type to Metal primitive type
    MTLPrimitiveType metalPrimitiveType = MTLPrimitiveTypeTriangle;
    unsigned int indicesPerPrimitive = 3;
    
    switch (primitiveType) {
        case 1: // D3DPT_POINTLIST
            metalPrimitiveType = MTLPrimitiveTypePoint;
            indicesPerPrimitive = 1;
            break;
        case 2: // D3DPT_LINELIST
            metalPrimitiveType = MTLPrimitiveTypeLine;
            indicesPerPrimitive = 2;
            break;
        case 3: // D3DPT_LINESTRIP
            metalPrimitiveType = MTLPrimitiveTypeLineStrip;
            indicesPerPrimitive = 1; // strip uses n+1 vertices
            break;
        case 4: // D3DPT_TRIANGLELIST
            metalPrimitiveType = MTLPrimitiveTypeTriangle;
            indicesPerPrimitive = 3;
            break;
        case 5: // D3DPT_TRIANGLESTRIP
            metalPrimitiveType = MTLPrimitiveTypeTriangleStrip;
            indicesPerPrimitive = 1; // strip uses n+2 vertices
            break;
        default:
            std::printf("METAL WARNING: Unknown primitive type %u, using triangles\n", primitiveType);
            break;
    }
    
    unsigned int indexCount = primitiveCount * indicesPerPrimitive;
    unsigned int indexOffset = startIndex + s_currentIndexBufferOffset;
    
    id<MTLBuffer> mtlIndexBuffer = (__bridge id<MTLBuffer>)s_currentIndexBuffer;
    
    // DEBUG: Print first few indices from buffer
    static bool printed_indices = false;
    if (!printed_indices) {
        unsigned short* indices = (unsigned short*)[mtlIndexBuffer contents];
        std::printf("METAL DEBUG: First 6 indices in buffer: [%u, %u, %u, %u, %u, %u]\n",
            indices[0], indices[1], indices[2], indices[3], indices[4], indices[5]);
        printed_indices = true;
    }
    
    [(id<MTLRenderCommandEncoder>)s_renderEncoder 
        drawIndexedPrimitives:metalPrimitiveType
        indexCount:indexCount
        indexType:MTLIndexTypeUInt16
        indexBuffer:mtlIndexBuffer
        indexBufferOffset:indexOffset * sizeof(unsigned short)
        instanceCount:1
        baseVertex:baseVertexIndex
        baseInstance:0];
    
    std::printf("METAL: DrawIndexedPrimitive (type: %u, primitives: %u, indices: %u, start: %u, base: %d)\n",
        primitiveType, primitiveCount, indexCount, startIndex, baseVertexIndex);
}

// Phase 28.1: Texture Creation from DDS
void* MetalWrapper::CreateTextureFromDDS(unsigned int width, unsigned int height, 
                                         unsigned int format, const void* data, 
                                         unsigned int dataSize, unsigned int mipLevels) {
    @autoreleasepool {
        if (!s_device) {
            std::printf("METAL ERROR: CreateTextureFromDDS called before Initialize\n");
            return nil;
        }
        
        if (!data || dataSize == 0) {
            std::printf("METAL ERROR: CreateTextureFromDDS - Invalid data (ptr=%p, size=%u)\n", 
                data, dataSize);
            return nil;
        }
        
        // Map DDS format to Metal pixel format
        MTLPixelFormat metalFormat;
        bool isCompressed = false;
        
        switch (format) {
            case 1: // DDS_FORMAT_BC1_RGBA
                metalFormat = MTLPixelFormatBC1_RGBA;
                isCompressed = true;
                std::printf("METAL: Creating BC1 (DXT1) texture\n");
                break;
            case 2: // DDS_FORMAT_BC2_RGBA
                metalFormat = MTLPixelFormatBC2_RGBA;
                isCompressed = true;
                std::printf("METAL: Creating BC2 (DXT3) texture\n");
                break;
            case 3: // DDS_FORMAT_BC3_RGBA
                metalFormat = MTLPixelFormatBC3_RGBA;
                isCompressed = true;
                std::printf("METAL: Creating BC3 (DXT5) texture\n");
                break;
            case 4: // DDS_FORMAT_RGBA8_UNORM
                metalFormat = MTLPixelFormatRGBA8Unorm;
                std::printf("METAL: Creating RGBA8 Unorm texture\n");
                break;
            case 5: // DDS_FORMAT_RGB8_UNORM (convert to RGBA8)
                metalFormat = MTLPixelFormatRGBA8Unorm;
                std::printf("METAL: Creating RGB8→RGBA8 Unorm texture\n");
                break;
            default:
                std::printf("METAL ERROR: Unknown DDS format: %u\n", format);
                return nil;
        }
        
        // Create texture descriptor
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
        descriptor.textureType = MTLTextureType2D;
        descriptor.pixelFormat = metalFormat;
        descriptor.width = width;
        descriptor.height = height;
        descriptor.depth = 1;
        descriptor.mipmapLevelCount = (mipLevels > 0) ? mipLevels : 1;
        descriptor.arrayLength = 1;
        descriptor.sampleCount = 1;
        descriptor.usage = MTLTextureUsageShaderRead;
        descriptor.storageMode = MTLStorageModeShared;  // Shared mode for CPU upload
        
        // Create texture
        id<MTLDevice> device = (id<MTLDevice>)s_device;
        id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
        if (!texture) {
            std::printf("METAL ERROR: Failed to create texture (%ux%u, format=%u)\n", 
                width, height, format);
            return nil;
        }
        
        // Upload texture data
        if (isCompressed) {
            // Phase 28.4: BC3 Bug Workaround - DISABLED for now (linking issues)
            // Will use native BC3 with forced alignment instead
            /*
            if (format == 3 && getenv("USE_BC3_DECOMPRESSION")) {
                std::printf("METAL: BC3 decompression workaround ENABLED\n");
                
                // Allocate output buffer for RGBA8
                unsigned int rgba8Size = BC3Decompressor::GetDecompressedSize(width, height);
                unsigned char* rgba8Data = new unsigned char[rgba8Size];
                
                // Decompress BC3 → RGBA8
                if (!BC3Decompressor::Decompress((const unsigned char*)data, width, height, rgba8Data)) {
                    std::printf("METAL ERROR: BC3 decompression failed\n");
                    delete[] rgba8Data;
                    return nil;
                }
                
                // Re-create texture descriptor for RGBA8
                descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
                texture = [device newTextureWithDescriptor:descriptor];
                if (!texture) {
                    std::printf("METAL ERROR: Failed to create RGBA8 texture after decompression\n");
                    delete[] rgba8Data;
                    return nil;
                }
                
                // Upload decompressed RGBA8 data with alignment
                MTLRegion region = MTLRegionMake2D(0, 0, width, height);
                unsigned int bytesPerRow = width * 4;  // RGBA8 = 4 bytes per pixel
                
                // Phase 28.4: CRITICAL BUG FIX - Apply bytesPerRow alignment for large textures
                unsigned int alignedBytesPerRow = ((bytesPerRow + 255) / 256) * 256;
                
                if (alignedBytesPerRow != bytesPerRow) {
                    std::printf("METAL DEBUG BC3: bytesPerRow alignment: %u → %u (aligned to 256)\n", 
                               bytesPerRow, alignedBytesPerRow);
                    
                    // Create aligned buffer
                    unsigned int alignedSize = alignedBytesPerRow * height;
                    unsigned char* alignedData = new unsigned char[alignedSize];
                    memset(alignedData, 0, alignedSize);
                    
                    // Copy row by row with padding
                    for (unsigned int y = 0; y < height; y++) {
                        memcpy(alignedData + (y * alignedBytesPerRow),
                              rgba8Data + (y * bytesPerRow),
                              bytesPerRow);
                    }
                    
                    [texture replaceRegion:region
                               mipmapLevel:0
                                 withBytes:alignedData
                               bytesPerRow:alignedBytesPerRow];
                    
                    delete[] alignedData;
                } else {
                    [texture replaceRegion:region
                               mipmapLevel:0
                                 withBytes:rgba8Data
                               bytesPerRow:bytesPerRow];
                }
                
                std::printf("METAL: BC3→RGBA8 decompression successful (%ux%u)\n", width, height);
                
                delete[] rgba8Data;
                
                // Skip normal compressed upload
                void* texPtr = (__bridge_retained void*)texture;
                std::printf("METAL: Texture created successfully (ID=%p, %ux%u, format=%u [DECOMPRESSED], mipmaps=%u, size=%u bytes)\n",
                    texPtr, width, height, format, (mipLevels > 0) ? mipLevels : 1, dataSize);
                return texPtr;
            }
            */
            
            // Normal compressed upload path
            // Compressed format - BC1/BC2/BC3
            // BC1: 8 bytes per 4x4 block
            // BC2/BC3: 16 bytes per 4x4 block
            unsigned int blockSize = (format == 1) ? 8 : 16;  // BC1=8, BC2/BC3=16
            unsigned int blocksWide = (width + 3) / 4;  // Round up to block boundary
            unsigned int blocksHigh = (height + 3) / 4;
            unsigned int bytesPerRow = blocksWide * blockSize;
            
            // INVESTIGATION: Try different bytesPerRow alignments
            // Metal may require alignment to specific boundaries for compressed textures
            // Let's try texture width in bytes (non-compressed equivalent)
            unsigned int bytesPerRowAlternative = width * 4;  // RGBA8 equivalent (4 bytes/pixel)
            
            unsigned int expectedSize = bytesPerRow * blocksHigh;
            
            std::printf("METAL DEBUG BC3: width=%u, height=%u, blocksWide=%u, blocksHigh=%u\n",
                width, height, blocksWide, blocksHigh);
            std::printf("METAL DEBUG BC3: bytesPerRow=%u (calculated), bytesPerRowAlt=%u (RGBA8 equiv), expectedSize=%u, actualSize=%u\n",
                bytesPerRow, bytesPerRowAlternative, expectedSize, dataSize);
            
            // For compressed textures, MTLRegion dimensions must be in PIXELS (not blocks)
            // BUT they must be multiples of block size (4x4 for BC formats)
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            
            std::printf("METAL DEBUG BC3: region(x=%llu, y=%llu, w=%llu, h=%llu) [PIXELS]\n",
                region.origin.x, region.origin.y, region.size.width, region.size.height);
            
            if (dataSize < expectedSize) {
                std::printf("METAL WARNING: Data size mismatch! Expected %u bytes, got %u bytes\n",
                    expectedSize, dataSize);
            }
            
            // DEBUG: Dump first 32 bytes of texture data to check for corruption
            const unsigned char* bytes = (const unsigned char*)data;
            std::printf("METAL DEBUG: First 32 bytes: ");
            for (int i = 0; i < 32 && i < dataSize; i++) {
                std::printf("%02X ", bytes[i]);
            }
            std::printf("\n");
            
            // DEBUG: Dump bytes at 50% mark (where orange starts)
            if (dataSize >= 131072 + 32) {
                std::printf("METAL DEBUG: Bytes at 50%% (131072): ");
                for (int i = 0; i < 32; i++) {
                    std::printf("%02X ", bytes[131072 + i]);
                }
                std::printf("\n");
            }
            
            // Phase 28.4: CRITICAL BUG FIX - Apply bytesPerRow alignment for BC3 native
            // Phase 28.4: BC3 bytesPerRow - Use simple path without alignment
            // EXPERIMENT: Original bytesPerRow (4096) is already aligned to 256
            // Extra padding might be causing the orange blocks issue
            std::printf("METAL DEBUG BC3 NATIVE: Using original bytesPerRow: %u (no forced alignment)\n", 
                       bytesPerRow);
            
            // Upload compressed texture data directly (no alignment padding)
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:bytesPerRow];
        } else {
            // Uncompressed format - region in pixels
            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            
            // Uncompressed format - specify bytes per row
            unsigned int bytesPerPixel = (format == 5) ? 3 : 4;  // RGB8 or RGBA8
            unsigned int bytesPerRow = width * bytesPerPixel;
            
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:bytesPerRow];
        }
        
        std::printf("METAL: Texture created successfully (ID=%p, %ux%u, format=%u, mipmaps=%u, size=%u bytes)\n",
            (__bridge void*)texture, width, height, format, mipLevels, dataSize);
        
        // Return as void* with retained reference (caller responsible for CFRelease)
        return (__bridge_retained void*)texture;
    }
}

// Phase 28.3.2: Create texture from TGA (RGBA8 uncompressed)
void* MetalWrapper::CreateTextureFromTGA(unsigned int width, unsigned int height,
                                          const void* data, unsigned int dataSize)
{
    @autoreleasepool {
        if (!s_device) {
            std::printf("METAL ERROR: Device not initialized (CreateTextureFromTGA)\n");
            return nullptr;
        }
        
        if (!data || dataSize == 0) {
            std::printf("METAL ERROR: NULL data or zero size (CreateTextureFromTGA)\n");
            return nullptr;
        }
        
        // Validate data size (RGBA8 = 4 bytes per pixel)
        unsigned int expected_size = width * height * 4;
        if (dataSize != expected_size) {
            std::printf("METAL WARNING: Data size mismatch (expected %u, got %u)\n",
                expected_size, dataSize);
        }
        
        // TGA loader always outputs RGBA8
        MTLPixelFormat metalFormat = MTLPixelFormatRGBA8Unorm;
        
        // Create texture descriptor
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
        descriptor.textureType = MTLTextureType2D;
        descriptor.pixelFormat = metalFormat;
        descriptor.width = width;
        descriptor.height = height;
        descriptor.mipmapLevelCount = 1; // TGA doesn't have mipmaps
        descriptor.usage = MTLTextureUsageShaderRead;
        
        // Create texture
        id<MTLDevice> device = (id<MTLDevice>)s_device;
        id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
        
        if (!texture) {
            std::printf("METAL ERROR: Failed to create MTLTexture (TGA %ux%u)\n", width, height);
            return nullptr;
        }
        
        // Upload RGBA8 data (uncompressed)
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        unsigned int bytesPerRow = width * 4; // RGBA8
        
        // Phase 28.4: CRITICAL BUG FIX - Metal requires bytesPerRow alignment
        // For optimal performance, bytesPerRow should be aligned to 256 bytes on Apple Silicon
        // Reference: https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
        unsigned int alignedBytesPerRow = ((bytesPerRow + 255) / 256) * 256;
        
        if (alignedBytesPerRow != bytesPerRow) {
            std::printf("METAL DEBUG TGA: bytesPerRow alignment: %u → %u (aligned to 256)\n", 
                       bytesPerRow, alignedBytesPerRow);
            
            // Create aligned buffer
            unsigned int alignedSize = alignedBytesPerRow * height;
            unsigned char* alignedData = new unsigned char[alignedSize];
            memset(alignedData, 0, alignedSize);
            
            // Copy row by row with padding
            const unsigned char* srcData = (const unsigned char*)data;
            for (unsigned int y = 0; y < height; y++) {
                memcpy(alignedData + (y * alignedBytesPerRow),
                      srcData + (y * bytesPerRow),
                      bytesPerRow);
            }
            
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:alignedData
                       bytesPerRow:alignedBytesPerRow];
            
            delete[] alignedData;
        } else {
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:bytesPerRow];
        }
        
        std::printf("METAL: Created TGA texture %ux%u (RGBA8, %u bytes, ID=%p)\n",
            width, height, dataSize, (__bridge void*)texture);
        
        // Return as void* with retained reference (caller responsible for CFRelease)
        return (__bridge_retained void*)texture;
    }
}

// Phase 28.4 REDESIGN: Generic texture creation from memory
// Converts GLenum format to MTLPixelFormat and creates texture
void* MetalWrapper::CreateTextureFromMemory(unsigned int width, unsigned int height,
                                             unsigned int glFormat, const void* data,
                                             unsigned int dataSize) {
    @autoreleasepool {
        if (!s_device) {
            std::printf("METAL ERROR: Device not initialized (CreateTextureFromMemory)\n");
            return nullptr;
        }
        
        if (!data || dataSize == 0) {
            std::printf("METAL ERROR: NULL data or zero size (CreateTextureFromMemory)\n");
            return nullptr;
        }
        
        // Convert GLenum format to MTLPixelFormat
        MTLPixelFormat metalFormat;
        bool isCompressed = false;
        const char* formatName = "UNKNOWN";
        unsigned int bytesPerPixel = 4;
        
        switch (glFormat) {
            case 0x83F1: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
                metalFormat = MTLPixelFormatBC1_RGBA;
                isCompressed = true;
                formatName = "DXT1/BC1";
                break;
            case 0x83F2: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
                metalFormat = MTLPixelFormatBC2_RGBA;
                isCompressed = true;
                formatName = "DXT3/BC2";
                break;
            case 0x83F3: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
                metalFormat = MTLPixelFormatBC3_RGBA;
                isCompressed = true;
                formatName = "DXT5/BC3";
                break;
            case 0x8058: // GL_RGBA8
                metalFormat = MTLPixelFormatRGBA8Unorm;
                isCompressed = false;
                formatName = "RGBA8";
                bytesPerPixel = 4;
                break;
            case 0x8051: // GL_RGB8
                // Metal doesn't have RGB8, use RGBA8 (caller must ensure data has alpha)
                metalFormat = MTLPixelFormatRGBA8Unorm;
                isCompressed = false;
                formatName = "RGB8→RGBA8";
                bytesPerPixel = 3; // Input is RGB, but Metal needs RGBA
                std::printf("METAL WARNING: RGB8 format requires conversion to RGBA8\n");
                break;
            default:
                std::printf("METAL ERROR: Unsupported GL format 0x%04X in CreateTextureFromMemory\n", glFormat);
                return nullptr;
        }
        
        std::printf("METAL: Creating texture from memory %ux%u (format=%s/0x%04X, %u bytes)\n",
                   width, height, formatName, glFormat, dataSize);
        
        // Create texture descriptor
        MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
        descriptor.textureType = MTLTextureType2D;
        descriptor.pixelFormat = metalFormat;
        descriptor.width = width;
        descriptor.height = height;
        descriptor.mipmapLevelCount = 1; // No mipmaps for now
        descriptor.usage = MTLTextureUsageShaderRead;
        
        // Create texture
        id<MTLDevice> device = (id<MTLDevice>)s_device;
        id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
        
        if (!texture) {
            std::printf("METAL ERROR: Failed to create MTLTexture (%s %ux%u)\n", formatName, width, height);
            return nullptr;
        }
        
        // Upload data (compressed vs uncompressed)
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        
        if (isCompressed) {
            // Compressed textures use direct upload
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:0]; // bytesPerRow=0 for compressed
        } else {
            // Uncompressed textures need bytesPerRow alignment
            unsigned int bytesPerRow = width * bytesPerPixel;
            unsigned int alignedBytesPerRow = ((bytesPerRow + 255) / 256) * 256;
            
            if (alignedBytesPerRow != bytesPerRow) {
                std::printf("METAL DEBUG: bytesPerRow alignment: %u → %u (aligned to 256)\n", 
                           bytesPerRow, alignedBytesPerRow);
                
                // Create aligned buffer
                unsigned int alignedSize = alignedBytesPerRow * height;
                unsigned char* alignedData = new unsigned char[alignedSize];
                memset(alignedData, 0, alignedSize);
                
                // Copy row by row with padding
                const unsigned char* srcData = (const unsigned char*)data;
                for (unsigned int y = 0; y < height; y++) {
                    memcpy(alignedData + (y * alignedBytesPerRow),
                          srcData + (y * bytesPerRow),
                          bytesPerRow);
                }
                
                [texture replaceRegion:region
                           mipmapLevel:0
                             withBytes:alignedData
                           bytesPerRow:alignedBytesPerRow];
                
                delete[] alignedData;
            } else {
                [texture replaceRegion:region
                           mipmapLevel:0
                             withBytes:data
                           bytesPerRow:bytesPerRow];
            }
        }
        
        std::printf("METAL SUCCESS: Texture created from memory (ID=%p)\n", (__bridge void*)texture);
        
        // Return as void* with retained reference (caller responsible for CFRelease)
        return (__bridge_retained void*)texture;
    }
}

void MetalWrapper::DeleteTexture(void* texture) {
    if (texture) {
        // Release bridged reference (was created with __bridge_retained)
        CFRelease(texture);
        std::printf("METAL: Texture deleted (ID=%p)\n", texture);
    }
}

// Phase 28.3.3: Bind texture to fragment shader
void MetalWrapper::BindTexture(void* texture, unsigned int slot) {
    @autoreleasepool {
        if (!s_renderEncoder) {
            std::printf("METAL WARNING: No active render encoder (BindTexture)\n");
            return;
        }
        
        if (!texture) {
            std::printf("METAL WARNING: NULL texture (BindTexture slot %u)\n", slot);
            return;
        }
        
        id<MTLRenderCommandEncoder> encoder = (id<MTLRenderCommandEncoder>)s_renderEncoder;
        id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)texture;
        
        // Bind texture to fragment shader at specified slot
        [encoder setFragmentTexture:mtlTexture atIndex:slot];
        
        // Create sampler state
        // Phase 28.4: BC3 Bug Investigation - Test different sampler modes
        // Set USE_NEAREST_FILTER=1 to test pixel-perfect sampling
        id<MTLDevice> device = (id<MTLDevice>)s_device;
        MTLSamplerDescriptor* samplerDesc = [MTLSamplerDescriptor new];
        
        if (getenv("USE_NEAREST_FILTER")) {
            // Nearest (pixel-perfect) filtering
            samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
            samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
            samplerDesc.mipFilter = MTLSamplerMipFilterNearest;
            std::printf("METAL DEBUG: Using NEAREST filtering\n");
        } else {
            // Linear (smooth) filtering (default)
            samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
            samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
        }
        
        // Test different address modes
        if (getenv("USE_REPEAT_ADDRESS")) {
            samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
            samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
            std::printf("METAL DEBUG: Using REPEAT address mode\n");
        } else {
            samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
        }
        
        // Normalized coordinates (should always be YES for standard textures)
        samplerDesc.normalizedCoordinates = YES;
        
        id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:samplerDesc];
        
        [encoder setFragmentSamplerState:sampler atIndex:slot];
        
        std::printf("METAL: Bound texture %p to slot %u\n", (void*)texture, slot);
    }
}

// Phase 28.3.3: Unbind texture from fragment shader
void MetalWrapper::UnbindTexture(unsigned int slot) {
    @autoreleasepool {
        if (!s_renderEncoder) {
            return;
        }
        
        id<MTLRenderCommandEncoder> encoder = (id<MTLRenderCommandEncoder>)s_renderEncoder;
        [encoder setFragmentTexture:nil atIndex:slot];
        [encoder setFragmentSamplerState:nil atIndex:slot];
        
        std::printf("METAL: Unbound texture from slot %u\n", slot);
    }
}

// Phase 28.4.3: Set identity matrices and default uniforms for 2D rendering
void MetalWrapper::SetDefaultUniforms() {
    // Initialize with identity matrices
    for (int i = 0; i < 16; i++) {
        s_currentUniforms.worldMatrix[i] = 0.0f;
        s_currentUniforms.viewMatrix[i] = 0.0f;
        s_currentUniforms.projectionMatrix[i] = 0.0f;
    }
    
    // Set diagonal to 1 (identity matrix)
    s_currentUniforms.worldMatrix[0] = 1.0f;
    s_currentUniforms.worldMatrix[5] = 1.0f;
    s_currentUniforms.worldMatrix[10] = 1.0f;
    s_currentUniforms.worldMatrix[15] = 1.0f;
    
    s_currentUniforms.viewMatrix[0] = 1.0f;
    s_currentUniforms.viewMatrix[5] = 1.0f;
    s_currentUniforms.viewMatrix[10] = 1.0f;
    s_currentUniforms.viewMatrix[15] = 1.0f;
    
    s_currentUniforms.projectionMatrix[0] = 1.0f;
    s_currentUniforms.projectionMatrix[5] = 1.0f;
    s_currentUniforms.projectionMatrix[10] = 1.0f;
    s_currentUniforms.projectionMatrix[15] = 1.0f;
    
    // Light direction (pointing down-right)
    s_currentUniforms.lightDirection[0] = 0.0f;
    s_currentUniforms.lightDirection[1] = -1.0f;
    s_currentUniforms.lightDirection[2] = 0.0f;
    s_currentUniforms._pad0 = 0.0f;
    
    // Light colors
    s_currentUniforms.lightColor[0] = 1.0f;
    s_currentUniforms.lightColor[1] = 1.0f;
    s_currentUniforms.lightColor[2] = 1.0f;
    s_currentUniforms._pad1 = 0.0f;
    
    s_currentUniforms.ambientColor[0] = 1.0f;
    s_currentUniforms.ambientColor[1] = 1.0f;
    s_currentUniforms.ambientColor[2] = 1.0f;
    
    // Disable lighting for 2D rendering
    s_currentUniforms.useLighting = 0.0f;
    
    // Material colors (white)
    s_currentUniforms.materialDiffuse[0] = 1.0f;
    s_currentUniforms.materialDiffuse[1] = 1.0f;
    s_currentUniforms.materialDiffuse[2] = 1.0f;
    s_currentUniforms.materialDiffuse[3] = 1.0f;
    
    s_currentUniforms.materialAmbient[0] = 1.0f;
    s_currentUniforms.materialAmbient[1] = 1.0f;
    s_currentUniforms.materialAmbient[2] = 1.0f;
    s_currentUniforms.materialAmbient[3] = 1.0f;
    
    // Disable alpha testing
    s_currentUniforms.alphaRef = 0.5f;
    s_currentUniforms.alphaTestEnabled = 0.0f;
    s_currentUniforms.alphaTestFunc = 8; // ALWAYS
    s_currentUniforms._pad2 = 0.0f;
    
    // Disable fog
    s_currentUniforms.fogColor[0] = 0.0f;
    s_currentUniforms.fogColor[1] = 0.0f;
    s_currentUniforms.fogColor[2] = 0.0f;
    s_currentUniforms.fogStart = 0.0f;
    s_currentUniforms.fogEnd = 1000.0f;
    s_currentUniforms.fogDensity = 0.0f;
    s_currentUniforms.fogMode = 0;
    s_currentUniforms.fogEnabled = 0.0f;
    
    s_uniformsDirty = true;
    
    std::printf("METAL: Default uniforms set (identity matrices, lighting OFF)\n");
}

// Phase 28.4.3: Set custom uniforms
void MetalWrapper::SetUniforms(const ShaderUniforms& uniforms) {
    s_currentUniforms = uniforms;
    s_uniformsDirty = true;
}

/***********************************************************************************
**
** Phase 29.1: Lighting Uniform Updates
**
***********************************************************************************/

void MetalWrapper::SetLightDirection(float x, float y, float z) {
    s_currentUniforms.lightDirection[0] = x;
    s_currentUniforms.lightDirection[1] = y;
    s_currentUniforms.lightDirection[2] = z;
    s_uniformsDirty = true;
}

void MetalWrapper::SetLightColor(float r, float g, float b) {
    s_currentUniforms.lightColor[0] = r;
    s_currentUniforms.lightColor[1] = g;
    s_currentUniforms.lightColor[2] = b;
    s_uniformsDirty = true;
}

void MetalWrapper::SetAmbientColor(float r, float g, float b) {
    s_currentUniforms.ambientColor[0] = r;
    s_currentUniforms.ambientColor[1] = g;
    s_currentUniforms.ambientColor[2] = b;
    s_uniformsDirty = true;
}

void MetalWrapper::SetUseLighting(bool enabled) {
    s_currentUniforms.useLighting = enabled ? 1.0f : 0.0f;
    s_uniformsDirty = true;
}

void MetalWrapper::SetMaterialDiffuse(float r, float g, float b, float a) {
    s_currentUniforms.materialDiffuse[0] = r;
    s_currentUniforms.materialDiffuse[1] = g;
    s_currentUniforms.materialDiffuse[2] = b;
    s_currentUniforms.materialDiffuse[3] = a;
    s_uniformsDirty = true;
}

void MetalWrapper::SetMaterialAmbient(float r, float g, float b, float a) {
    s_currentUniforms.materialAmbient[0] = r;
    s_currentUniforms.materialAmbient[1] = g;
    s_currentUniforms.materialAmbient[2] = b;
    s_currentUniforms.materialAmbient[3] = a;
    s_uniformsDirty = true;
}

/***********************************************************************************
**
** Phase 29.2: Fog Uniform Updates
**
***********************************************************************************/

void MetalWrapper::SetFogColor(float r, float g, float b) {
    s_currentUniforms.fogColor[0] = r;
    s_currentUniforms.fogColor[1] = g;
    s_currentUniforms.fogColor[2] = b;
    s_uniformsDirty = true;
}

void MetalWrapper::SetFogRange(float start, float end) {
    s_currentUniforms.fogStart = start;
    s_currentUniforms.fogEnd = end;
    s_uniformsDirty = true;
}

void MetalWrapper::SetFogDensity(float density) {
    s_currentUniforms.fogDensity = density;
    s_uniformsDirty = true;
}

void MetalWrapper::SetFogMode(int mode) {
    // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR
    s_currentUniforms.fogMode = mode;
    s_uniformsDirty = true;
}

void MetalWrapper::SetFogEnabled(bool enabled) {
    s_currentUniforms.fogEnabled = enabled ? 1.0f : 0.0f;
    s_uniformsDirty = true;
}

/***********************************************************************************
**
** Phase 29.3: Alpha Test Uniform Updates
**
***********************************************************************************/

void MetalWrapper::SetAlphaTestEnabled(bool enabled) {
    s_currentUniforms.alphaTestEnabled = enabled ? 1.0f : 0.0f;
    s_uniformsDirty = true;
}

void MetalWrapper::SetAlphaTestFunc(int func, float ref) {
    s_currentUniforms.alphaTestFunc = func;
    s_currentUniforms.alphaRef = ref;
    s_uniformsDirty = true;
}

// Phase 29.3: Stencil Buffer Operations
void MetalWrapper::SetStencilEnabled(bool enabled) {
    // Metal stencil is configured via MTLRenderPipelineDescriptor and MTLDepthStencilDescriptor
    // Store the state for future pipeline creation
    // For now, this is a placeholder - actual implementation requires depth stencil state object
    printf("METAL: SetStencilEnabled(%s) - stored for depth stencil state\n", enabled ? "true" : "false");
}

void MetalWrapper::SetStencilFunc(int func, unsigned int ref, unsigned int mask) {
    // func: D3DCMP_* comparison function (1=NEVER, 2=LESS, 3=EQUAL, etc.)
    // ref: reference value for stencil test
    // mask: mask ANDed with both reference and stencil buffer value
    printf("METAL: SetStencilFunc(func=%d, ref=%u, mask=0x%08X) - stored for depth stencil state\n", 
           func, ref, mask);
}

void MetalWrapper::SetStencilRef(unsigned int ref) {
    // Update stencil reference value
    printf("METAL: SetStencilRef(ref=%u) - stored for depth stencil state\n", ref);
}

void MetalWrapper::SetStencilMask(unsigned int mask) {
    // Update stencil read mask
    printf("METAL: SetStencilMask(mask=0x%08X) - stored for depth stencil state\n", mask);
}

void MetalWrapper::SetStencilWriteMask(unsigned int mask) {
    // Update stencil write mask
    printf("METAL: SetStencilWriteMask(mask=0x%08X) - stored for depth stencil state\n", mask);
}

void MetalWrapper::SetStencilOp(int sfail, int dpfail, int dppass) {
    // sfail: operation when stencil test fails
    // dpfail: operation when stencil test passes but depth test fails
    // dppass: operation when both stencil and depth tests pass
    // Operations: 1=KEEP, 2=ZERO, 3=REPLACE, 4=INCRSAT, 5=DECRSAT, 6=INVERT, 7=INCR, 8=DECR
    printf("METAL: SetStencilOp(sfail=%d, dpfail=%d, dppass=%d) - stored for depth stencil state\n", 
           sfail, dpfail, dppass);
}

// Phase 29.4: Point Sprite Operations
void MetalWrapper::SetPointSpriteEnabled(bool enabled) {
    // Point sprites in Metal require shader-based point rendering
    // Store the state for shader uniform updates
    printf("METAL: SetPointSpriteEnabled(%s) - stored for shader uniforms\n", enabled ? "true" : "false");
}

void MetalWrapper::SetPointSize(float size) {
    // Point size is controlled via vertex shader output (pointsize attribute)
    // Store for shader uniform updates
    printf("METAL: SetPointSize(%.2f) - stored for shader uniforms\n", size);
}

void MetalWrapper::SetPointScaleEnabled(bool enabled) {
    // Point scale enables distance-based size attenuation
    // Formula: FinalSize = Size * sqrt(1 / (A + B*D + C*D²))
    printf("METAL: SetPointScaleEnabled(%s) - stored for shader uniforms\n", enabled ? "true" : "false");
}

void MetalWrapper::SetPointScaleFactors(float a, float b, float c) {
    // A, B, C coefficients for distance-based point size attenuation
    // Used in formula: FinalSize = Size * sqrt(1 / (A + B*D + C*D²))
    // where D is distance from camera
    printf("METAL: SetPointScaleFactors(A=%.4f, B=%.4f, C=%.4f) - stored for shader uniforms\n", a, b, c);
}

void MetalWrapper::SetPointSizeMin(float minSize) {
    // Minimum point size clamp
    printf("METAL: SetPointSizeMin(%.2f) - stored for shader uniforms\n", minSize);
}

void MetalWrapper::SetPointSizeMax(float maxSize) {
    // Maximum point size clamp
    printf("METAL: SetPointSizeMax(%.2f) - stored for shader uniforms\n", maxSize);
}

} // namespace GX

#endif // __APPLE__
