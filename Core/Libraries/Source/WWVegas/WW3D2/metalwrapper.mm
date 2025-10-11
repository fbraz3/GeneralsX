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

    std::printf("METAL: Initialized (device, queue, layer)\n");
    return true;
}

void MetalWrapper::Shutdown() {
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
}

void MetalWrapper::EndFrame() {
    if (!s_cmdBuffer || !s_currentDrawable) return;

    // No draw calls yet; we just present the cleared drawable
    [(id<MTLCommandBuffer>)s_cmdBuffer presentDrawable:(id)s_currentDrawable];
    [(id<MTLCommandBuffer>)s_cmdBuffer commit];
    s_cmdBuffer = nil;
    s_currentDrawable = nil;
}

} // namespace WW3D

#endif // __APPLE__
