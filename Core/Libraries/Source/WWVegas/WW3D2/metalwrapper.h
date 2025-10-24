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
    
    // Phase 28.4 REDESIGN: Generic Texture Creation from Memory
    // Accepts GLenum format (GL_RGBA8, GL_RGB8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, etc.)
    // Returns void* (id<MTLTexture>) or NULL on error
    static void* CreateTextureFromMemory(unsigned int width, unsigned int height,
                                          unsigned int glFormat, const void* data,
                                          unsigned int dataSize);
    
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
    
    // Phase 29.1: Lighting Uniform Updates
    static void SetLightDirection(float x, float y, float z);
    static void SetLightColor(float r, float g, float b);
    static void SetAmbientColor(float r, float g, float b);
    static void SetUseLighting(bool enabled);
    static void SetMaterialDiffuse(float r, float g, float b, float a);
    static void SetMaterialAmbient(float r, float g, float b, float a);
    
    // Phase 29.2: Fog Uniform Updates
    static void SetFogColor(float r, float g, float b);
    static void SetFogRange(float start, float end);
    static void SetFogDensity(float density);
    static void SetFogMode(int mode); // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR
    static void SetFogEnabled(bool enabled);
    
    // Phase 29.3: Alpha Test Uniform Updates
    static void SetAlphaTestEnabled(bool enabled);
    static void SetAlphaTestFunc(int func, float ref);
    
    // Phase 29.3: Stencil Buffer Operations
    static void SetStencilEnabled(bool enabled);
    static void SetStencilFunc(int func, unsigned int ref, unsigned int mask);
    static void SetStencilRef(unsigned int ref);
    static void SetStencilMask(unsigned int mask);
    static void SetStencilWriteMask(unsigned int mask);
    static void SetStencilOp(int sfail, int dpfail, int dppass);
    
    // Phase 29.4: Point Sprite Operations
    static void SetPointSpriteEnabled(bool enabled);
    static void SetPointSize(float size);
    static void SetPointScaleEnabled(bool enabled);
    static void SetPointScaleFactors(float a, float b, float c);
    static void SetPointSizeMin(float minSize);
    static void SetPointSizeMax(float maxSize);

private:
    // Internal helper to create a very simple test pipeline (triangle)
    static bool CreateSimplePipeline();

    static id s_device;         // MTLDevice
    static id s_commandQueue;   // MTLCommandQueue
    static id s_layer;          // CAMetalLayer
    static id s_currentDrawable;// CAMetalDrawable
    static id s_cmdBuffer;      // MTLCommandBuffer
    // s_passDesc removed - was local variable causing use-after-free (LESSONS_LEARNED.md pattern)
};

} // namespace GX

#endif // __APPLE__
