// Metal Wrapper Stub for Vulkan-only builds
// When ENABLE_METAL is OFF, this provides empty implementations
// of all MetalWrapper functions to allow compilation without Metal

#ifndef __METALWRAPPER_STUB_H__
#define __METALWRAPPER_STUB_H__

namespace GX {
    struct MetalConfig {
        void* sdlWindow;
        int width{800};
        int height{600};
        bool vsync{true};
        void* metalLayer{nullptr};
    };
    
    class MetalWrapper {
    public:
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
        
        static bool Initialize(void* config) { return true; }
        static bool Initialize(const MetalConfig& cfg) { return true; }
        static void Shutdown() {}
        static void Resize(int width, int height) {}
        static void BeginFrame(float r, float g, float b, float a) {}
        static void EndFrame() {}
        
        static void* CreateVertexBuffer(unsigned int size, const void* data, bool dynamic = false) {
            return malloc(size);
        }
        static void DeleteVertexBuffer(void* buffer) {
            if (buffer) free(buffer);
        }
        static void UpdateVertexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0) {}
        
        static void* CreateIndexBuffer(unsigned int size, const void* data, bool dynamic = false) {
            return malloc(size);
        }
        static void DeleteIndexBuffer(void* buffer) {
            if (buffer) free(buffer);
        }
        static void UpdateIndexBuffer(void* buffer, const void* data, unsigned int size, unsigned int offset = 0) {}
        
        static void SetVertexBuffer(void* buffer, unsigned int offset, unsigned int slot) {}
        static void SetIndexBuffer(void* buffer, unsigned int offset) {}
        
        static void DrawPrimitive(unsigned int primitiveType, unsigned int startVertex, unsigned int vertexCount) {}
        static void DrawIndexedPrimitive(unsigned int primitiveType, int baseVertexIndex, 
                                        unsigned int minVertex, unsigned int numVertices,
                                        unsigned int startIndex, unsigned int primitiveCount) {}
        
        static void* CreateTextureFromMemory(unsigned int width, unsigned int height, unsigned int glFormat, 
                                            const void* data, unsigned int dataSize) {
            return malloc(width * height * 4);
        }
        static void* CreateTextureFromDDS(unsigned int width, unsigned int height, 
                                         unsigned int format, const void* data, 
                                         unsigned int dataSize, unsigned int mipLevels = 1) {
            return malloc(width * height * 4);
        }
        static void* CreateTextureFromTGA(unsigned int width, unsigned int height, const void* data, unsigned int dataSize) {
            return malloc(width * height * 4);
        }
        static void DeleteTexture(void* texture) {
            if (texture) free(texture);
        }
        static void BindTexture(void* texture, unsigned int slot = 0) {}
        static void UnbindTexture(unsigned int slot = 0) {}
        
        static void SetDefaultUniforms() {}
        static void SetUniforms(const ShaderUniforms& uniforms) {}
        
        static void SetUseLighting(bool enabled) {}
        static void SetAmbientColor(float r, float g, float b) {}
        static void SetLightDirection(float x, float y, float z) {}
        static void SetLightColor(float r, float g, float b) {}
        static void SetMaterialDiffuse(float r, float g, float b, float a) {}
        static void SetMaterialAmbient(float r, float g, float b, float a) {}
        
        static void SetFogColor(float r, float g, float b) {}
        static void SetFogRange(float start, float end) {}
        static void SetFogDensity(float density) {}
        static void SetFogMode(int mode) {}
        static void SetFogEnabled(bool enabled) {}
        
        static void SetAlphaTestEnabled(bool enabled) {}
        static void SetAlphaTestFunc(int func, float ref) {}
        
        static void SetStencilEnabled(bool enabled) {}
        static void SetStencilFunc(int func, unsigned int ref, unsigned int mask) {}
        static void SetStencilRef(unsigned int ref) {}
        static void SetStencilMask(unsigned int mask) {}
        static void SetStencilWriteMask(unsigned int mask) {}
        static void SetStencilOp(int sfail, int dpfail, int dppass) {}
        
        static void SetPointSpriteEnabled(bool enabled) {}
        static void SetPointSize(float size) {}
        static void SetPointScaleEnabled(bool enabled) {}
        static void SetPointScaleFactors(float a, float b, float c) {}
        static void SetPointSizeMin(float minSize) {}
        static void SetPointSizeMax(float maxSize) {}
    }; // class MetalWrapper
} // namespace GX

#endif // __METALWRAPPER_STUB_H__
