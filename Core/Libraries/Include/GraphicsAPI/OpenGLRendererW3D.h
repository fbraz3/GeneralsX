#pragma once

#include "GraphicsRenderer.h"

#ifdef ENABLE_OPENGL
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined(__linux__)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/OpenGL.h> // For CGL types
#endif
#endif

// W3D Integration support
#ifdef ENABLE_W3D_INTEGRATION
#include "W3DTypes.h"
#endif

class OpenGLRendererW3D : public IGraphicsRenderer {
private:
#ifdef ENABLE_OPENGL
    // Platform-specific context
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
    
    // Matrix storage - use W3D types when available
#ifdef ENABLE_W3D_INTEGRATION
    W3DMatrix4* m_projectionMatrix;
    W3DMatrix4* m_viewMatrix;
    W3DMatrix4* m_worldMatrix;
    
    // OpenGL-compatible matrix cache
    float m_projectionGL[16];
    float m_viewGL[16];
    float m_worldGL[16];
    bool m_matricesNeedUpdate;
#else
    Matrix4* m_projectionMatrix;
    Matrix4* m_viewMatrix;
    Matrix4* m_worldMatrix;
#endif
    
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

    // Helper functions for W3D integration
#ifdef ENABLE_W3D_INTEGRATION
    void UpdateMatrixCache();
    void ConvertW3DMatrix(const W3DMatrix4& w3dMatrix, float* glMatrix);
#endif

public:
    OpenGLRendererW3D();
    virtual ~OpenGLRendererW3D();
    
    // IGraphicsRenderer interface
    bool Initialize(int width, int height, bool windowed) override;
    void Shutdown() override;
    void Reset() override;
    
    bool BeginFrame() override;
    void EndFrame() override;
    void Present() override;
    
    void SetViewport(int x, int y, int width, int height) override;
    
    // Matrix methods - handle both W3D and mock types
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
    
    GraphicsAPI GetAPI() const override { return GraphicsAPI::OPENGL; }
    const char* GetAPIString() const override { return "OpenGL (W3D)"; }
    
private:
    void UpdateMatrices();
    GLenum ConvertPrimitiveType(PrimitiveType type);
};
