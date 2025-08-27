#include "GraphicsRenderer.h"

#ifdef ENABLE_OPENGL
#include "OpenGLRenderer.h"
#endif

#ifdef ENABLE_DIRECTX8
#include "dx8renderer.h"
// Create DX8 wrapper that implements IGraphicsRenderer
#endif

// Global renderer instance
IGraphicsRenderer* g_theRenderer = nullptr;

IGraphicsRenderer* GraphicsRendererFactory::CreateRenderer(GraphicsAPI api) {
    switch (api) {
#ifdef ENABLE_OPENGL
        case GraphicsAPI::OPENGL:
            return new OpenGLRenderer();
#endif

#ifdef ENABLE_DIRECTX8
        case GraphicsAPI::DIRECTX8:
            // return new DX8RendererWrapper();
            break;
#endif

        default:
            break;
    }
    
    return nullptr;
}

void GraphicsRendererFactory::DestroyRenderer(IGraphicsRenderer* renderer) {
    delete renderer;
}

GraphicsAPI GraphicsRendererFactory::DetectBestAPI() {
#if defined(DEFAULT_GRAPHICS_API)
    return DEFAULT_GRAPHICS_API;
#elif defined(ENABLE_OPENGL)
    return GraphicsAPI::OPENGL;
#elif defined(ENABLE_DIRECTX8)
    return GraphicsAPI::DIRECTX8;
#else
    return GraphicsAPI::OPENGL; // Fallback
#endif
}
