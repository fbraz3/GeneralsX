#pragma once

// Minimal Metal backend interface for macOS.
// This mirrors the DX8/OpenGL wrapper shape with init/draw/shutdown.

#if defined(__APPLE__)

// Forward declarations to avoid heavy ObjC headers here.
typedef struct objc_object* id;

namespace GX {

struct MetalConfig {
    void* sdlWindow; // SDL_Window*
    int width{800};
    int height{600};
    bool vsync{true};
    void* metalLayer{nullptr}; // Optional CAMetalLayer*
};

class MetalWrapper {
public:
    static bool Initialize(const MetalConfig& cfg);
    static void Shutdown();
    static void Resize(int width, int height);
    static void BeginFrame(float r, float g, float b, float a);
    static void EndFrame();

    // Phase 30.2: Buffer Management API
    static id CreateVertexBuffer(const void* data, unsigned int size, bool dynamic = false);
    static id CreateIndexBuffer(const void* data, unsigned int size, bool dynamic = false);
    static void DeleteBuffer(id buffer);
    static void UpdateBuffer(id buffer, const void* data, unsigned int size, unsigned int offset = 0);

    // Phase 30.5: Draw Calls (stub implementation - full rendering in future phase)
    static void DrawPrimitives(unsigned int primitiveType, unsigned int startVertex, unsigned int vertexCount);
    static void DrawIndexedPrimitives(unsigned int primitiveType, unsigned int startIndex, unsigned int indexCount, id vertexBuffer, id indexBuffer);

private:
    // Internal helper to create a very simple test pipeline (triangle)
    static bool CreateSimplePipeline();

    static id s_device;         // MTLDevice
    static id s_commandQueue;   // MTLCommandQueue
    static id s_layer;          // CAMetalLayer
    static id s_currentDrawable;// CAMetalDrawable
    static id s_cmdBuffer;      // MTLCommandBuffer
    static id s_passDesc;       // MTLRenderPassDescriptor
};

} // namespace GX

#endif // __APPLE__
