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
    static void* CreateVertexBuffer(unsigned int size, const void* data, bool dynamic = false);
    static void* CreateIndexBuffer(unsigned int size, const void* data, bool dynamic = false);
    static void DeleteVertexBuffer(void* buffer);
    static void DeleteIndexBuffer(void* buffer);
    static void UpdateVertexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0);
    static void UpdateIndexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0);
    
    // Phase 30.3: Buffer Binding
    static void SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot);
    static void SetIndexBuffer(void* buffer, unsigned int offset);
    
    // Phase 30.5: Draw Calls
    static void DrawPrimitive(unsigned int primitiveType, unsigned int startVertex, unsigned int vertexCount);
    static void DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex, 
                                     unsigned int minVertex, unsigned int numVertices,
                                     unsigned int startIndex, unsigned int primitiveCount);

    // Phase 28.1: Texture Creation from DDS
    // Supported formats: BC1/BC2/BC3 (DXT1/DXT3/DXT5), RGBA8, RGB8
    static void* CreateTextureFromDDS(unsigned int width, unsigned int height, 
                                       unsigned int format, const void* data, 
                                       unsigned int dataSize, unsigned int mipLevels = 1);
    
    // Phase 28.3.2: Texture Creation from TGA
    // Creates RGBA8 uncompressed texture (TGA data is already RGBA8)
    static void* CreateTextureFromTGA(unsigned int width, unsigned int height,
                                       const void* data, unsigned int dataSize);
    
    static void DeleteTexture(void* texture);
    
    // Phase 28.3.3: Texture Binding
    // Binds texture to fragment shader at specified slot (default slot 0)
    static void BindTexture(void* texture, unsigned int slot = 0);
    
    // Unbinds texture from fragment shader slot
    static void UnbindTexture(unsigned int slot = 0);
    
    // Phase 28.4.3: Uniform Buffer Management
    // Shader uniforms structure matching basic.metal
    struct ShaderUniforms {
        float worldMatrix[16];       // 4x4 matrix
        float viewMatrix[16];        // 4x4 matrix
        float projectionMatrix[16];  // 4x4 matrix
        float lightDirection[3];     // vec3
        float _pad0;                 // alignment
        float lightColor[3];         // vec3
        float _pad1;                 // alignment
        float ambientColor[3];       // vec3
        float useLighting;           // float (0=disabled, 1=enabled)
        float materialDiffuse[4];    // vec4
        float materialAmbient[4];    // vec4
        float alphaRef;              // float
        float alphaTestEnabled;      // float
        int alphaTestFunc;           // int
        float _pad2;                 // alignment
        float fogColor[3];           // vec3
        float fogStart;              // float
        float fogEnd;                // float
        float fogDensity;            // float
        int fogMode;                 // int
        float fogEnabled;            // float
    };
    
    // Set identity matrices and default uniforms for 2D rendering
    static void SetDefaultUniforms();
    
    // Set custom uniforms (advanced usage)
    static void SetUniforms(const ShaderUniforms& uniforms);

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
