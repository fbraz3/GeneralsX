#pragma once

#include <cstdint>

// Forward declarations for W3D types - avoid conflicts with WWMath
class Vector3;
class Vector4;

// Only define mock types when W3D types are absolutely not available
#if !defined(WWMATH_VECTOR3_H) && !defined(_VECTOR3_H_) && !defined(ENABLE_W3D_INTEGRATION)
    
    // Mock types for standalone testing only - namespaced to avoid conflicts
    namespace GraphicsAPI {
        struct Matrix4 {
        float m[16];
        Matrix4() { 
            for(int i = 0; i < 16; i++) m[i] = 0.0f;
            m[0] = m[5] = m[10] = m[15] = 1.0f; // Identity
        }
        
        Matrix4(const Matrix4& other) {
            for(int i = 0; i < 16; i++) m[i] = other.m[i];
        }
        
        Matrix4& operator=(const Matrix4& other) {
            if (this != &other) {
                for(int i = 0; i < 16; i++) m[i] = other.m[i];
            }
            return *this;
        }
        
        Matrix4 operator*(const Matrix4& other) const {
            Matrix4 result;
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    result.m[i*4 + j] = 0;
                    for(int k = 0; k < 4; k++) {
                        result.m[i*4 + j] += m[i*4 + k] * other.m[k*4 + j];
                    }
                }
            }
            return result;
        }
        
        float* data() { return m; }
        const float* data() const { return m; }
    };
    
    // Don't define Vector3/Vector4 mock types to avoid conflicts with WWMath
    // Use forward declarations only
    }
    
    // Only use Matrix4 mock when necessary
    using Matrix4 = GraphicsAPI::Matrix4;
    
#endif

// Forward declarations
class TextureClass;
class VertexBufferClass;
class IndexBufferClass;
class ShaderClass;

// Define types that might not be available yet
enum PrimitiveType {
    PRIMITIVE_TRIANGLELIST,
    PRIMITIVE_TRIANGLESTRIP,
    PRIMITIVE_TRIANGLEFAN,
    PRIMITIVE_LINELIST,
    PRIMITIVE_LINESTRIP,
    PRIMITIVE_POINTLIST,
    // Additional aliases for compatibility
    PRIMITIVE_TRIANGLES = PRIMITIVE_TRIANGLELIST,
    PRIMITIVE_TRIANGLE_STRIP = PRIMITIVE_TRIANGLESTRIP,
    PRIMITIVE_TRIANGLE_FAN = PRIMITIVE_TRIANGLEFAN,
    PRIMITIVE_LINES = PRIMITIVE_LINELIST,
    PRIMITIVE_LINE_STRIP = PRIMITIVE_LINESTRIP,
    PRIMITIVE_POINTS = PRIMITIVE_POINTLIST
};

enum TransformType {
    TRANSFORM_WORLD,
    TRANSFORM_VIEW,
    TRANSFORM_PROJECTION
};

enum TextureFormat {
    TEXTURE_FORMAT_RGBA8,
    TEXTURE_FORMAT_RGB8,
    TEXTURE_FORMAT_DXT1,
    TEXTURE_FORMAT_DXT3,
    TEXTURE_FORMAT_DXT5
};

struct SamplerState {
    int minFilter;
    int magFilter;
    int wrapS;
    int wrapT;
};

enum class GraphicsAPIType {
    DIRECTX8,
    OPENGL,
    VULKAN  // Para o futuro
};

/**
 * Abstract interface for graphics rendering
 * Allows support for multiple graphics APIs (DX8, OpenGL, etc.)
 */
class IGraphicsRenderer {
public:
    virtual ~IGraphicsRenderer() = default;
    
    // Initialization and cleanup
    virtual bool Initialize(int width, int height, bool windowed) = 0;
    virtual void Shutdown() = 0;
    virtual void Reset() = 0;
    
    // Frame control
    virtual bool BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
    
    // Rendering states
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void SetProjectionMatrix(const Matrix4& matrix) = 0;
    virtual void SetViewMatrix(const Matrix4& matrix) = 0;
    virtual void SetWorldMatrix(const Matrix4& matrix) = 0;
    
    // Primitive rendering
    virtual void DrawIndexedPrimitives(
        PrimitiveType type,
        VertexBufferClass* vertices,
        IndexBufferClass* indices,
        int startIndex,
        int primitiveCount
    ) = 0;
    
    // Textures
    virtual void SetTexture(int stage, TextureClass* texture) = 0;
    virtual void SetSampler(int stage, const SamplerState& state) = 0;
    
    // Shaders
    virtual void SetShader(ShaderClass* shader) = 0;
    
    // Clear operations
    virtual void Clear(bool color, bool depth, bool stencil, 
                      uint32_t clearColor = 0x00000000) = 0;
    
    // API information
    virtual GraphicsAPIType GetAPI() const = 0;
    virtual const char* GetAPIString() const = 0;
};

/**
 * Factory for creating renderers
 */
class GraphicsRendererFactory {
public:
    static IGraphicsRenderer* CreateRenderer(GraphicsAPIType api);
    static void DestroyRenderer(IGraphicsRenderer* renderer);
    
    // Automatically detects the best available API
    static GraphicsAPIType DetectBestAPI();
};

// Global renderer instance
extern IGraphicsRenderer* g_theRenderer;
