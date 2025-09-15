#pragma once

#include "GraphicsRenderer.h"
#include <map>

#ifdef ENABLE_OPENGL
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#endif
#endif

// Forward declarations to avoid dependency issues
class DX8Renderer;
class ShaderClass;
class TextureClass;

/**
 * Adapter that allows the W3D system to use the new abstract renderer
 * while maintaining compatibility with existing code
 */
class W3DRendererAdapter {
private:
    static IGraphicsRenderer* s_renderer;
    static bool s_useNewRenderer;
    
public:
    // Initialization
    static bool Initialize(GraphicsAPIType preferredAPI = GraphicsAPIType::OPENGL);
    static void Shutdown();
    
    // Configuration
    static void SetUseNewRenderer(bool use) { s_useNewRenderer = use; }
    static bool IsUsingNewRenderer() { return s_useNewRenderer; }
    static IGraphicsRenderer* GetRenderer() { return s_renderer; }
    
    // Bridge functions for existing DX8 system
    static void BeginFrame();
    static void EndFrame();
    static void Present();
    
    static void SetTexture(int stage, TextureClass* texture);
    static void SetShader(ShaderClass shader);
    static void SetTransform(TransformType type, const Matrix4& matrix);
    
    static void DrawIndexedPrimitive(
        PrimitiveType type,
        int baseVertex,
        int minIndex,
        int numVertices,
        int startIndex,
        int primitiveCount
    );
};

/**
 * Wrapper for TextureClass that works with both systems
 * Note: This is a composition approach instead of inheritance
 * to avoid dependency issues with TextureClass
 */
class W3DTextureWrapper {
private:
#ifdef ENABLE_OPENGL
    GLuint m_glTexture;
#else
    uint32_t m_glTexture; // Dummy member when OpenGL is not available
#endif
    bool m_isGLTexture;
    int m_width, m_height;
    TextureFormat m_format;
    
public:
    W3DTextureWrapper();
    virtual ~W3DTextureWrapper();
    
    // Create OpenGL texture from existing data
    bool CreateFromData(const void* data, int width, int height, TextureFormat format);
    bool CreateFromFile(const char* filename);
    
    // Texture interface
    virtual void Apply(int stage);
    
private:
    virtual int Get_Width() const { return m_width; }
    virtual int Get_Height() const { return m_height; }
    
    // OpenGL specific
#ifdef ENABLE_OPENGL
    GLuint GetGLTexture() const { return m_glTexture; }
    void BindGL(int textureUnit);
#endif
};

/**
 * DX8 to OpenGL shader conversion system
 */
class ShaderConverter {
public:
    struct GLShaderProgram {
#ifdef ENABLE_OPENGL
        GLuint program;
        GLuint vertexShader;
        GLuint fragmentShader;
        
        // Uniform locations
        GLint worldMatrix;
        GLint viewMatrix;
        GLint projectionMatrix;
        GLint diffuseTexture;
        GLint normalTexture;
        GLint specularTexture;
#else
        uint32_t program;
        uint32_t vertexShader;
        uint32_t fragmentShader;
        
        // Uniform locations
        int worldMatrix;
        int viewMatrix;
        int projectionMatrix;
        int diffuseTexture;
        int normalTexture;
        int specularTexture;
#endif
    };
    
    static GLShaderProgram* ConvertShader(const ShaderClass& dx8Shader);
    static void DestroyShader(GLShaderProgram* program);
    static void UseShader(GLShaderProgram* program);
    
private:
    static std::map<uint32_t, GLShaderProgram*> s_shaderCache;
    
    static const char* GetVertexShaderSource(const ShaderClass& shader);
    static const char* GetFragmentShaderSource(const ShaderClass& shader);
#ifdef ENABLE_OPENGL
    static GLuint CompileShader(GLenum type, const char* source);
    static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
#else
    static uint32_t CompileShader(uint32_t type, const char* source);
    static uint32_t LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
#endif
};
