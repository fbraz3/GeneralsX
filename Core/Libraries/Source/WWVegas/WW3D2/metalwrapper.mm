#include "metalwrapper.h"

#if defined(__APPLE__)

#import <TargetConditionals.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <SDL2/SDL.h>
#import <SDL2/SDL_metal.h>

#include <cstdio>

namespace WW3D {

// Static members
id MetalWrapper::s_device = nil;
id MetalWrapper::s_commandQueue = nil;
id MetalWrapper::s_layer = nil;
id MetalWrapper::s_currentDrawable = nil;
id MetalWrapper::s_cmdBuffer = nil;
id MetalWrapper::s_passDesc = nil;

// Additional static members for minimal pipeline
static id s_vertexBuffer = nil;
static id s_pipelineState = nil;
static id s_renderEncoder = nil;

// Simple triangle vertices for testing
struct Vertex {
    float position[3];
    float color[4];
};

static const Vertex triangleVertices[] = {
    { { 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },  // Top (red)
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },  // Bottom left (green)
    { { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }   // Bottom right (blue)
};

static CAMetalLayer* GetOrCreateLayer(void* sdlWindowPtr, int width, int height) {
    // Note: SDL2 on macOS can expose an NSWindow* via SDL_GetWindowWMInfo if linked with Cocoa.
    // Preferred path: use SDL_metal to get a CAMetalLayer attached to the window.
    CAMetalLayer* layer = nullptr;
    if (sdlWindowPtr) {
        SDL_Window* sdlWindow = (SDL_Window*)sdlWindowPtr;
        SDL_MetalView metalView = SDL_Metal_CreateView(sdlWindow);
        if (metalView) {
            layer = (CAMetalLayer*)SDL_Metal_GetLayer(metalView);
        }
    }
    if (!layer) {
        // Fallback: create a standalone layer
        layer = [CAMetalLayer layer];
    }
    layer.frame = CGRectMake(0, 0, width, height);
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.colorspace = CGColorSpaceCreateDeviceRGB();
    layer.framebufferOnly = YES;
    return layer;
}

static bool CreateSimplePipeline() {
    // Simple vertex shader source
    NSString* vertexShaderSource = @R"(
    #include <metal_stdlib>
    using namespace metal;
    
    struct VertexInput {
        float3 position [[attribute(0)]];
        float4 color    [[attribute(1)]];
    };
    
    struct VertexOutput {
        float4 position [[position]];
        float4 color;
    };
    
    vertex VertexOutput vertex_main(VertexInput in [[stage_in]]) {
        VertexOutput out;
        out.position = float4(in.position, 1.0);
        out.color = in.color;
        return out;
    }
    )";
    
    // Simple fragment shader source
    NSString* fragmentShaderSource = @R"(
    #include <metal_stdlib>
    using namespace metal;
    
    struct VertexOutput {
        float4 position [[position]];
        float4 color;
    };
    
    fragment float4 fragment_main(VertexOutput in [[stage_in]]) {
        return in.color;
    }
    )";
    
    NSError* error = nil;
    id<MTLLibrary> library = [(id<MTLDevice>)MetalWrapper::s_device newLibraryWithSource:
        [NSString stringWithFormat:@"%@\n%@", vertexShaderSource, fragmentShaderSource] 
        options:nil error:&error];
    
    if (!library) {
        std::printf("METAL: Failed to compile shaders: %s\n", [[error localizedDescription] UTF8String]);
        return false;
    }
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];
    
    MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
    desc.vertexFunction = vertexFunction;
    desc.fragmentFunction = fragmentFunction;
    desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Vertex descriptor
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    vertexDesc.attributes[0].format = MTLVertexFormatFloat3;
    vertexDesc.attributes[0].offset = 0;
    vertexDesc.attributes[0].bufferIndex = 0;
    vertexDesc.attributes[1].format = MTLVertexFormatFloat4;
    vertexDesc.attributes[1].offset = 12; // sizeof(float) * 3
    vertexDesc.attributes[1].bufferIndex = 0;
    vertexDesc.layouts[0].stride = sizeof(Vertex);
    desc.vertexDescriptor = vertexDesc;
    
    s_pipelineState = [(id<MTLDevice>)MetalWrapper::s_device newRenderPipelineStateWithDescriptor:desc error:&error];
    if (!s_pipelineState) {
        std::printf("METAL: Failed to create pipeline state: %s\n", [[error localizedDescription] UTF8String]);
        return false;
    }
    
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

    s_layer = cfg.metalLayer ? (id)cfg.metalLayer : (id)GetOrCreateLayer(cfg.sdlWindow, cfg.width, cfg.height);
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
    if (!CreateSimplePipeline()) {
        std::printf("METAL: Failed to create rendering pipeline\n");
        return false;
    }

    std::printf("METAL: Initialized (device, queue, layer, triangle pipeline)\n");
    return true;
}

void MetalWrapper::Shutdown() {
    s_renderEncoder = nil;
    s_pipelineState = nil;
    s_vertexBuffer = nil;
    s_passDesc = nil;
    s_cmdBuffer = nil;
    s_currentDrawable = nil;
    s_layer = nil;
    s_commandQueue = nil;
    s_device = nil;
    std::printf("METAL: Shutdown\n");
}

void MetalWrapper::Resize(int width, int height) {
    if (s_layer) {
        [(CAMetalLayer*)s_layer setDrawableSize:CGSizeMake(width, height)];
    }
}

void MetalWrapper::BeginFrame(float r, float g, float b, float a) {
    if (!s_device || !s_commandQueue || !s_layer) return;
    s_currentDrawable = [(CAMetalLayer*)s_layer nextDrawable];
    if (!s_currentDrawable) return;

    s_cmdBuffer = [(id<MTLCommandQueue>)s_commandQueue commandBuffer];
    if (!s_cmdBuffer) return;

    // Create a simple pass descriptor that clears to the given color
    MTLRenderPassDescriptor* pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].texture = [(id) [s_currentDrawable texture]];
    pass.colorAttachments[0].loadAction = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, a);
    s_passDesc = pass;
    
    // Begin render pass and draw triangle
    if (s_pipelineState && s_vertexBuffer) {
        s_renderEncoder = [(id<MTLCommandBuffer>)s_cmdBuffer renderCommandEncoderWithDescriptor:pass];
        [(id<MTLRenderCommandEncoder>)s_renderEncoder setRenderPipelineState:(id<MTLRenderPipelineState>)s_pipelineState];
        [(id<MTLRenderCommandEncoder>)s_renderEncoder setVertexBuffer:(id<MTLBuffer>)s_vertexBuffer offset:0 atIndex:0];
        [(id<MTLRenderCommandEncoder>)s_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        [(id<MTLRenderCommandEncoder>)s_renderEncoder endEncoding];
        s_renderEncoder = nil;
    }
}

void MetalWrapper::EndFrame() {
    if (!s_cmdBuffer || !s_currentDrawable) return;

    // Present the drawable
    [(id<MTLCommandBuffer>)s_cmdBuffer presentDrawable:(id)s_currentDrawable];
    [(id<MTLCommandBuffer>)s_cmdBuffer commit];
    s_cmdBuffer = nil;
    s_currentDrawable = nil;
}

} // namespace WW3D

#endif // __APPLE__
