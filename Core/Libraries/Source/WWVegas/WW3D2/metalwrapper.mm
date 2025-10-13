#include "metalwrapper.h"

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

bool MetalWrapper::CreateSimplePipeline() {
    // Combined Metal shader source with shared struct definitions
    NSString* shaderSource = @R"(
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
    
    fragment float4 fragment_main(VertexOutput in [[stage_in]]) {
        return in.color;
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
    if (!MetalWrapper::CreateSimplePipeline()) {
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
    pass.colorAttachments[0].texture = [(id<CAMetalDrawable>)s_currentDrawable texture];
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
id MetalWrapper::CreateTextureFromDDS(unsigned int width, unsigned int height, 
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
        descriptor.storageMode = MTLStorageModePrivate;  // GPU-only for best performance
        
        // Create texture
        id<MTLTexture> texture = [(__bridge id<MTLDevice>)s_device newTextureWithDescriptor:descriptor];
        if (!texture) {
            std::printf("METAL ERROR: Failed to create texture (%ux%u, format=%u)\n", 
                width, height, format);
            return nil;
        }
        
        // Upload texture data
        // For compressed formats, bytesPerRow and bytesPerImage are ignored
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        
        if (isCompressed) {
            // Compressed format - Metal handles block alignment internally
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:0      // Ignored for compressed formats
                     bytesPerImage:0];    // Ignored for 2D textures
        } else {
            // Uncompressed format - specify bytes per row
            unsigned int bytesPerPixel = (format == 5) ? 3 : 4;  // RGB8 or RGBA8
            unsigned int bytesPerRow = width * bytesPerPixel;
            
            [texture replaceRegion:region
                       mipmapLevel:0
                         withBytes:data
                       bytesPerRow:bytesPerRow
                     bytesPerImage:0];    // Ignored for 2D textures
        }
        
        std::printf("METAL: Texture created successfully (ID=%p, %ux%u, format=%u, mipmaps=%u, size=%u bytes)\n",
            (void*)texture, width, height, format, mipLevels, dataSize);
        
        // Return retained reference (caller responsible for release)
        return (__bridge_retained id)texture;
    }
}

// Phase 28.3.2: Create texture from TGA (RGBA8 uncompressed)
id MetalWrapper::CreateTextureFromTGA(unsigned int width, unsigned int height,
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
        id<MTLDevice> device = (__bridge id<MTLDevice>)s_device;
        id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
        
        if (!texture) {
            std::printf("METAL ERROR: Failed to create MTLTexture (TGA %ux%u)\n", width, height);
            return nullptr;
        }
        
        // Upload RGBA8 data (uncompressed)
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        unsigned int bytesPerRow = width * 4; // RGBA8
        
        [texture replaceRegion:region
                   mipmapLevel:0
                     withBytes:data
                   bytesPerRow:bytesPerRow];
        
        std::printf("METAL: Created TGA texture %ux%u (RGBA8, %u bytes, ID=%p)\n",
            width, height, dataSize, (void*)texture);
        
        // Return retained reference (caller responsible for release)
        return (__bridge_retained id)texture;
    }
}

void MetalWrapper::DeleteTexture(id texture) {
    if (texture) {
        // Release bridged reference
        CFRelease((__bridge CFTypeRef)texture);
        std::printf("METAL: Texture deleted (ID=%p)\n", (void*)texture);
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
        
        id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)s_renderEncoder;
        id<MTLTexture> mtlTexture = (__bridge id<MTLTexture>)texture;
        
        // Bind texture to fragment shader at specified slot
        [encoder setFragmentTexture:mtlTexture atIndex:slot];
        
        // Create sampler state (linear filtering, clamp to edge)
        // TODO: Cache sampler state for performance
        id<MTLDevice> device = (__bridge id<MTLDevice>)s_device;
        MTLSamplerDescriptor* samplerDesc = [MTLSamplerDescriptor new];
        samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
        samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
        samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
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
        
        id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)s_renderEncoder;
        [encoder setFragmentTexture:nil atIndex:slot];
        [encoder setFragmentSamplerState:nil atIndex:slot];
        
        std::printf("METAL: Unbound texture from slot %u\n", slot);
    }
}

} // namespace WW3D

#endif // __APPLE__
