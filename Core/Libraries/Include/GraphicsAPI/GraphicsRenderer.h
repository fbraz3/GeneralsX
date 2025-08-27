#pragma once

#include <cstdint>

// Forward declarations for W3D types
class Matrix4;
class Vector3;
class Vector4;

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
    PRIMITIVE_POINTLIST
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

enum class GraphicsAPI {
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
    virtual GraphicsAPI GetAPI() const = 0;
    virtual const char* GetAPIString() const = 0;
};

/**
 * Factory for creating renderers
 */
class GraphicsRendererFactory {
public:
    static IGraphicsRenderer* CreateRenderer(GraphicsAPI api);
    static void DestroyRenderer(IGraphicsRenderer* renderer);
    
    // Automatically detects the best available API
    static GraphicsAPI DetectBestAPI();
};

// Global renderer instance
extern IGraphicsRenderer* g_theRenderer;
