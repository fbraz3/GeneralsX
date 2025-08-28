#include "OpenGLRendererW3D.h"
#include <cstring>
#include <iostream>

// Enable OpenGL for this file
#define ENABLE_OPENGL

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/OpenGL.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#elif defined(__linux__)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

OpenGLRendererW3D::OpenGLRendererW3D() 
    : m_width(0), m_height(0), m_windowed(true), m_initialized(false)
#ifdef ENABLE_OPENGL
    , m_currentVBO(0), m_currentIBO(0)
#ifdef ENABLE_W3D_INTEGRATION
    , m_matricesNeedUpdate(true)
#endif
#else
    , m_currentVBO(0), m_currentIBO(0), m_platformContext(nullptr)
#endif
{
    // Initialize platform-specific members
#ifdef ENABLE_OPENGL
#ifdef _WIN32
    m_hWnd = NULL;
    m_hDC = NULL;
    m_hRC = NULL;
#elif defined(__linux__)
    m_display = nullptr;
    m_window = 0;
    m_context = nullptr;
#elif defined(__APPLE__)
    m_context = nullptr;
#endif
#endif

    // Initialize texture array
    for (int i = 0; i < 8; i++) {
        m_currentTexture[i] = 0;
    }
    
    // Allocate matrices
#ifdef ENABLE_W3D_INTEGRATION
    m_projectionMatrix = new W3DMatrix4();
    m_viewMatrix = new W3DMatrix4();
    m_worldMatrix = new W3DMatrix4();
    
    // Initialize identity matrices
    m_projectionMatrix->Make_Identity();
    m_viewMatrix->Make_Identity();
    m_worldMatrix->Make_Identity();
#else
    m_projectionMatrix = new Matrix4();
    m_viewMatrix = new Matrix4();
    m_worldMatrix = new Matrix4();
#endif
}

OpenGLRendererW3D::~OpenGLRendererW3D() {
    if (m_initialized) {
        Shutdown();
    }
    
    // Clean up matrices
    delete m_projectionMatrix;
    delete m_viewMatrix;
    delete m_worldMatrix;
}

bool OpenGLRendererW3D::Initialize(int width, int height, bool windowed) {
    m_width = width;
    m_height = height;
    m_windowed = windowed;
    
#ifdef _WIN32
    return InitializeWindows();
#elif defined(__linux__)
    return InitializeLinux();
#elif defined(__APPLE__)
    return InitializeMacOS();
#else
    std::cout << "OpenGL not supported on this platform" << std::endl;
    return false;
#endif
}

#ifdef _WIN32
bool OpenGLRendererW3D::InitializeWindows() {
    // Create temporary window to get context
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"OpenGLTemp";
    RegisterClass(&wc);
    
    HWND tempWnd = CreateWindow(L"OpenGLTemp", L"Temp", WS_OVERLAPPEDWINDOW,
                               0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    
    HDC tempDC = GetDC(tempWnd);
    
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    
    int pixelFormat = ChoosePixelFormat(tempDC, &pfd);
    SetPixelFormat(tempDC, pixelFormat, &pfd);
    
    HGLRC tempRC = wglCreateContext(tempDC);
    wglMakeCurrent(tempDC, tempRC);
    
    // Load WGL extensions
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    
    // Create real context
    m_hWnd = tempWnd; // For now, use temporary window
    m_hDC = tempDC;
    
    if (wglCreateContextAttribsARB) {
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, 0,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        
        m_hRC = wglCreateContextAttribsARB(m_hDC, nullptr, attribs);
        if (!m_hRC) {
            // Fallback to legacy context
            m_hRC = tempRC;
        } else {
            wglDeleteContext(tempRC);
        }
    } else {
        m_hRC = tempRC;
    }
    
    wglMakeCurrent(m_hDC, m_hRC);
    
    return LoadOpenGLExtensions();
}
#endif

#ifdef __linux__
bool OpenGLRendererW3D::InitializeLinux() {
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        std::cerr << "Failed to open X display" << std::endl;
        return false;
    }
    
    int screen = DefaultScreen(m_display);
    
    GLint attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None
    };
    
    XVisualInfo* visual = glXChooseVisual(m_display, screen, attributes);
    if (!visual) {
        std::cerr << "Failed to choose visual" << std::endl;
        XCloseDisplay(m_display);
        return false;
    }
    
    Colormap colormap = XCreateColormap(m_display, RootWindow(m_display, screen),
                                       visual->visual, AllocNone);
    
    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask;
    
    m_window = XCreateWindow(m_display, RootWindow(m_display, screen),
                            0, 0, m_width, m_height, 0,
                            visual->depth, InputOutput, visual->visual,
                            CWColormap | CWEventMask, &swa);
    
    XMapWindow(m_display, m_window);
    XStoreName(m_display, m_window, "Generals - OpenGL");
    
    m_context = glXCreateContext(m_display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(m_display, m_window, m_context);
    
    return LoadOpenGLExtensions();
}
#endif

#ifdef __APPLE__
bool OpenGLRendererW3D::InitializeMacOS() {
    // For now, create a simple context without window management
    // Window management should be handled by the game's existing system
    
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAAccelerated,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
        kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
        kCGLPFADoubleBuffer,
        (CGLPixelFormatAttribute)0
    };
    
    CGLPixelFormatObj pixelFormat;
    GLint numPixelFormats;
    CGLError error = CGLChoosePixelFormat(attributes, &pixelFormat, &numPixelFormats);
    
    if (error != kCGLNoError) {
        std::cerr << "Failed to choose pixel format: " << CGLErrorString(error) << std::endl;
        return false;
    }
    
    error = CGLCreateContext(pixelFormat, nullptr, &m_context);
    CGLReleasePixelFormat(pixelFormat);
    
    if (error != kCGLNoError) {
        std::cerr << "Failed to create OpenGL context: " << CGLErrorString(error) << std::endl;
        return false;
    }
    
    error = CGLSetCurrentContext(m_context);
    if (error != kCGLNoError) {
        std::cerr << "Failed to set current context: " << CGLErrorString(error) << std::endl;
        CGLReleaseContext(m_context);
        return false;
    }
    
    return LoadOpenGLExtensions();
}
#endif

bool OpenGLRendererW3D::LoadOpenGLExtensions() {
    // Check OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;
    
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    std::cout << "GPU Vendor: " << vendor << std::endl;
    
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "GPU Renderer: " << renderer << std::endl;
    
    // Initial configurations
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_initialized = true;
    return true;
}

void OpenGLRendererW3D::Shutdown() {
    if (!m_initialized) return;
    
#ifdef _WIN32
    if (m_hRC) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hRC);
        m_hRC = nullptr;
    }
    if (m_hDC) {
        ReleaseDC(m_hWnd, m_hDC);
        m_hDC = nullptr;
    }
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
#elif defined(__linux__)
    if (m_context) {
        glXMakeCurrent(m_display, None, nullptr);
        glXDestroyContext(m_display, m_context);
        m_context = nullptr;
    }
    if (m_window) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
    }
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
#elif defined(__APPLE__)
    if (m_context) {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(m_context);
        m_context = nullptr;
    }
#endif
    
    m_initialized = false;
}

void OpenGLRendererW3D::Reset() {
    // Implement reset if needed
}

bool OpenGLRendererW3D::BeginFrame() {
    if (!m_initialized) return false;
    
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
#ifdef ENABLE_W3D_INTEGRATION
    if (m_matricesNeedUpdate) {
        UpdateMatrixCache();
        m_matricesNeedUpdate = false;
    }
#endif
    
    return true;
}

void OpenGLRendererW3D::EndFrame() {
    // Nothing specific needed here
}

void OpenGLRendererW3D::Present() {
#ifdef _WIN32
    SwapBuffers(m_hDC);
#elif defined(__linux__)
    glXSwapBuffers(m_display, m_window);
#elif defined(__APPLE__)
    CGLFlushDrawable(m_context);
#endif
}

void OpenGLRendererW3D::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRendererW3D::Clear(bool color, bool depth, bool stencil, uint32_t clearColor) {
    GLbitfield mask = 0;
    
    if (color) {
        float r = ((clearColor >> 16) & 0xFF) / 255.0f;
        float g = ((clearColor >> 8) & 0xFF) / 255.0f;
        float b = (clearColor & 0xFF) / 255.0f;
        float a = ((clearColor >> 24) & 0xFF) / 255.0f;
        glClearColor(r, g, b, a);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    
    if (depth) {
        glClearDepth(1.0);
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    
    if (stencil) {
        glClearStencil(0);
        mask |= GL_STENCIL_BUFFER_BIT;
    }
    
    if (mask != 0) {
        glClear(mask);
    }
}

// Matrix operations
void OpenGLRendererW3D::SetProjectionMatrix(const Matrix4& matrix) {
#ifdef ENABLE_W3D_INTEGRATION
    *m_projectionMatrix = matrix;
    m_matricesNeedUpdate = true;
#else
    *m_projectionMatrix = matrix;
    UpdateMatrices();
#endif
}

void OpenGLRendererW3D::SetViewMatrix(const Matrix4& matrix) {
#ifdef ENABLE_W3D_INTEGRATION
    *m_viewMatrix = matrix;
    m_matricesNeedUpdate = true;
#else
    *m_viewMatrix = matrix;
    UpdateMatrices();
#endif
}

void OpenGLRendererW3D::SetWorldMatrix(const Matrix4& matrix) {
#ifdef ENABLE_W3D_INTEGRATION
    *m_worldMatrix = matrix;
    m_matricesNeedUpdate = true;
#else
    *m_worldMatrix = matrix;
    UpdateMatrices();
#endif
}

#ifdef ENABLE_W3D_INTEGRATION
void OpenGLRendererW3D::UpdateMatrixCache() {
    // Convert W3D matrices to OpenGL format
    ConvertW3DMatrix(*m_projectionMatrix, m_projectionGL);
    ConvertW3DMatrix(*m_viewMatrix, m_viewGL);
    ConvertW3DMatrix(*m_worldMatrix, m_worldGL);
}

void OpenGLRendererW3D::ConvertW3DMatrix(const W3DMatrix4& w3dMatrix, float* glMatrix) {
    W3DOpenGLUtils::MatrixToOpenGL(w3dMatrix, glMatrix);
}
#endif

void OpenGLRendererW3D::UpdateMatrices() {
#ifdef ENABLE_W3D_INTEGRATION
    // Use real W3D matrix operations with conversion utilities
    glMatrixMode(GL_PROJECTION);
    float projectionGL[16];
    W3DOpenGLUtils::GetMatrixData(*m_projectionMatrix, projectionGL);
    glLoadMatrixf(projectionGL);
    
    // Combine view and world matrices using real W3D operations
    W3DMatrix4 modelView = W3DOpenGLUtils::MultiplyMatrices(*m_viewMatrix, *m_worldMatrix);
    float modelViewGL[16];
    W3DOpenGLUtils::GetMatrixData(modelView, modelViewGL);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelViewGL);
#else
    // Use mock matrix system (for testing)
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMatrix->data());
    
    Matrix4 modelView = (*m_viewMatrix) * (*m_worldMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelView.data());
#endif
}

GLenum OpenGLRendererW3D::ConvertPrimitiveType(PrimitiveType type) {
    switch (type) {
        case PRIMITIVE_TRIANGLES: return GL_TRIANGLES;
        case PRIMITIVE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
        case PRIMITIVE_TRIANGLE_FAN: return GL_TRIANGLE_FAN;
        case PRIMITIVE_LINES: return GL_LINES;
        case PRIMITIVE_LINE_STRIP: return GL_LINE_STRIP;
        case PRIMITIVE_POINTS: return GL_POINTS;
        default: return GL_TRIANGLES;
    }
}

void OpenGLRendererW3D::DrawIndexedPrimitives(
    PrimitiveType type,
    VertexBufferClass* vertices,
    IndexBufferClass* indices,
    int startIndex,
    int primitiveCount) {
    
    // This is a placeholder implementation
    // In a real implementation, you'd bind the vertex/index buffers
    // and render using the OpenGL calls
    
    GLenum glType = ConvertPrimitiveType(type);
    
    // For now, just call a basic draw to validate the pipeline
    glDrawElements(glType, primitiveCount * 3, GL_UNSIGNED_SHORT, 
                   (void*)(startIndex * sizeof(uint16_t)));
}

void OpenGLRendererW3D::SetTexture(int stage, TextureClass* texture) {
    if (stage >= 0 && stage < 8) {
        glActiveTexture(GL_TEXTURE0 + stage);
        
        if (texture) {
            // In a real implementation, you'd get the OpenGL texture ID from the TextureClass
            glBindTexture(GL_TEXTURE_2D, 0); // Placeholder
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        
        m_currentTexture[stage] = texture ? 1 : 0; // Placeholder
    }
}

void OpenGLRendererW3D::SetSampler(int stage, const SamplerState& state) {
    // Placeholder implementation
    // In a real implementation, you'd set the OpenGL sampler state
}

void OpenGLRendererW3D::SetShader(ShaderClass* shader) {
    // Placeholder implementation
    // In a real implementation, you'd bind the OpenGL shader program
}
