#pragma once

// Enable OpenGL for this implementation
#define ENABLE_OPENGL

#include "GraphicsRenderer.h"
#include <cstring>  // For memset

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
// Using simple types for testing to avoid dependency conflicts
struct W3DMatrix4Test {
    float m[4][4];
    W3DMatrix4Test() {
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }
    
    void Make_Identity() {
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }
    
    float* operator[](int i) { return m[i]; }
    const float* operator[](int i) const { return m[i]; }
};

struct W3DVector3Test {
    float x, y, z;
    W3DVector3Test() : x(0), y(0), z(0) {}
    W3DVector3Test(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

struct W3DVector4Test {
    float x, y, z, w;
    W3DVector4Test() : x(0), y(0), z(0), w(0) {}
    W3DVector4Test(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

// Utility functions for test types
namespace W3DTestUtils {
    inline void MatrixToOpenGL(const W3DMatrix4Test& w3dMatrix, float glMatrix[16]) {
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                glMatrix[col * 4 + row] = w3dMatrix[row][col];
            }
        }
    }
    
    inline void GetMatrixData(const W3DMatrix4Test& matrix, float data[16]) {
        MatrixToOpenGL(matrix, data);
    }
    
    inline W3DMatrix4Test MultiplyMatrices(const W3DMatrix4Test& a, const W3DMatrix4Test& b) {
        W3DMatrix4Test result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result[i][j] += a[i][k] * b[k][j];
                }
            }
        }
        return result;
    }
}

// Use test types for compatibility
#define W3DMatrix4 W3DMatrix4Test
#define W3DVector3 W3DVector3Test
#define W3DVector4 W3DVector4Test
#define W3DOpenGLUtils W3DTestUtils
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
    
    GraphicsAPIType GetAPI() const override { return GraphicsAPIType::OPENGL; }
    const char* GetAPIString() const override { return "OpenGL (W3D)"; }
    
private:
    void UpdateMatrices();
    GLenum ConvertPrimitiveType(PrimitiveType type);
};
