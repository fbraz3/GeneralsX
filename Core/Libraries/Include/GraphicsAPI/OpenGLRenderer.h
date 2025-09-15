#pragma once

#include "GraphicsRenderer.h"

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#ifdef ENABLE_OPENGL
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined(__linux__)
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/OpenGL.h> // For CGL types
#endif
#endif // ENABLE_OPENGL

/**
 * OpenGL implementation of the abstract renderer
 */
class OpenGLRenderer : public IGraphicsRenderer {
private:
    // OpenGL context specific to each platform
#ifdef ENABLE_OPENGL
#ifdef _WIN32
    HWND m_hWnd;
    HDC m_hDC;
    HGLRC m_hRC;
#elif defined(__linux__)
    Display* m_display;
    Window m_window;
    GLXContext m_context;
#elif defined(__APPLE__)
    CGLContextObj m_context;
#endif
    
    // Current buffers
    GLuint m_currentVBO;
    GLuint m_currentIBO;
    GLuint m_currentTexture[8]; // Support for multiple stages
#else
    // Dummy members when OpenGL is not available
    void* m_platformContext;
    uint32_t m_currentVBO;
    uint32_t m_currentIBO;
    uint32_t m_currentTexture[8];
#endif
    
    // Current state
    int m_width, m_height;
    bool m_windowed;
    bool m_initialized;
    
    // Matrices - use forward declared Matrix4
    Matrix4* m_projectionMatrix;
    Matrix4* m_viewMatrix;
    Matrix4* m_worldMatrix;
    
    // Function to load OpenGL extensions
    bool LoadOpenGLExtensions();
    
    // Platform-specific initialization functions
#ifdef _WIN32
    bool InitializeWindows();
#elif defined(__linux__)
    bool InitializeLinux();
#elif defined(__APPLE__)
    bool InitializeMacOS();
#endif
    
public:
    OpenGLRenderer();
    virtual ~OpenGLRenderer();
    
    // IGraphicsRenderer interface
    bool Initialize(int width, int height, bool windowed) override;
    void Shutdown() override;
    void Reset() override;
    
    bool BeginFrame() override;
    void EndFrame() override;
    void Present() override;
    
    void SetViewport(int x, int y, int width, int height) override;
    void SetProjectionMatrix(const Matrix4& matrix) override;
    void SetViewMatrix(const Matrix4& matrix) override;
    void SetWorldMatrix(const Matrix4& matrix) override;
    
    void DrawIndexedPrimitives(
        PrimitiveType type,
        VertexBufferClass* vertices,
        IndexBufferClass* indices,
        int startIndex,
        int primitiveCount
    ) override;
    
    void SetTexture(int stage, TextureClass* texture) override;
    void SetSampler(int stage, const SamplerState& state) override;
    void SetShader(ShaderClass* shader) override;
    
    void Clear(bool color, bool depth, bool stencil, 
              uint32_t clearColor = 0x00000000) override;
    
    GraphicsAPIType GetAPI() const override { return GraphicsAPIType::OPENGL; }
    const char* GetAPIString() const override { return "OpenGL"; }
    
private:
    // OpenGL-specific helper functions
#ifdef ENABLE_OPENGL
    GLenum PrimitiveTypeToGL(PrimitiveType type);
#else
    uint32_t PrimitiveTypeToGL(PrimitiveType type);
#endif
    void UpdateMatrices();
    void SetupBlendState(const ShaderClass* shader);
    void SetupDepthState(const ShaderClass* shader);
};
