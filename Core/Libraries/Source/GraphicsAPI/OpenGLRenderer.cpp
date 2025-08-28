#include "OpenGLRenderer.h"
#include <cstring>
#include <iostream>

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
// WGL functions for Windows
typedef HGLRC (WINAPI *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#elif defined(__linux__)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

OpenGLRenderer::OpenGLRenderer() 
    : m_width(0), m_height(0), m_windowed(true), m_initialized(false)
#ifdef ENABLE_OPENGL
    , m_currentVBO(0), m_currentIBO(0)
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
    m_projectionMatrix = new Matrix4();
    m_viewMatrix = new Matrix4();
    m_worldMatrix = new Matrix4();
}

OpenGLRenderer::~OpenGLRenderer() {
    if (m_initialized) {
        Shutdown();
    }
    
    // Clean up matrices
    delete m_projectionMatrix;
    delete m_viewMatrix;
    delete m_worldMatrix;
}

bool OpenGLRenderer::Initialize(int width, int height, bool windowed) {
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
    return false;
#endif
}

#ifdef _WIN32
bool OpenGLRenderer::InitializeWindows() {
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
bool OpenGLRenderer::InitializeLinux() {
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
bool OpenGLRenderer::InitializeMacOS() {
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

bool OpenGLRenderer::LoadOpenGLExtensions() {
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

void OpenGLRenderer::Shutdown() {
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
    // Implement shutdown for macOS
#endif
    
    m_initialized = false;
}

void OpenGLRenderer::Reset() {
    // Implement reset if needed
}

bool OpenGLRenderer::BeginFrame() {
    if (!m_initialized) return false;
    
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    return true;
}

void OpenGLRenderer::EndFrame() {
    // Nothing specific needed here
}

void OpenGLRenderer::Present() {
#ifdef _WIN32
    SwapBuffers(m_hDC);
#elif defined(__linux__)
    glXSwapBuffers(m_display, m_window);
#elif defined(__APPLE__)
    // Implement swap for macOS
#endif
}

void OpenGLRenderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRenderer::Clear(bool color, bool depth, bool stencil, uint32_t clearColor) {
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
    
    if (mask) {
        glClear(mask);
    }
}

// Basic implementations - need to be expanded
void OpenGLRenderer::SetProjectionMatrix(const Matrix4& matrix) {
    *m_projectionMatrix = matrix;
    UpdateMatrices();
}

void OpenGLRenderer::SetViewMatrix(const Matrix4& matrix) {
    *m_viewMatrix = matrix;
    UpdateMatrices();
}

void OpenGLRenderer::SetWorldMatrix(const Matrix4& matrix) {
    *m_worldMatrix = matrix;
    UpdateMatrices();
}

void OpenGLRenderer::UpdateMatrices() {
    // Here we would need to send matrices to shaders
    // For now, use fixed matrix (legacy OpenGL compatibility)
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projectionMatrix->data());
    
    Matrix4 modelView = (*m_viewMatrix) * (*m_worldMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(modelView.data());
}

void OpenGLRenderer::DrawIndexedPrimitives(
    PrimitiveType type,
    VertexBufferClass* vertices,
    IndexBufferClass* indices,
    int startIndex,
    int primitiveCount) {
    
    // Basic implementation - needs expansion
    GLenum glType = PrimitiveTypeToGL(type);
    
    // Here we would need to bind vertex/index buffers
    // and call glDrawElements
    
    // For now, placeholder
    glDrawElements(glType, primitiveCount * 3, GL_UNSIGNED_SHORT, 
                  (void*)(startIndex * sizeof(unsigned short)));
}

GLenum OpenGLRenderer::PrimitiveTypeToGL(PrimitiveType type) {
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

void OpenGLRenderer::SetTexture(int stage, TextureClass* texture) {
    // Basic implementation
    if (stage >= 0 && stage < 8) {
        glActiveTexture(GL_TEXTURE0 + stage);
        
        if (texture) {
            // Here we would need to get the OpenGL texture ID
            // GLuint textureId = texture->GetGLTextureId();
            // glBindTexture(GL_TEXTURE_2D, textureId);
            glBindTexture(GL_TEXTURE_2D, 0); // Placeholder
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void OpenGLRenderer::SetSampler(int stage, const SamplerState& state) {
    // Implement sampler configuration
}

void OpenGLRenderer::SetShader(ShaderClass* shader) {
    // Implement shader configuration
}
