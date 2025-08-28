#include "W3DRendererAdapter.h"
#include "GraphicsRenderer.h"
#include <iostream>

// Silence OpenGL deprecation warnings on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

// Include OpenGL headers for texture operations
#ifdef __APPLE__
#include <OpenGL/gl.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#elif defined(__linux__)
#include <GL/gl.h>
#endif

// Enable W3D integration for this adapter
#define ENABLE_W3D_INTEGRATION

// Mock W3D classes for compilation compatibility
// These will be replaced with real W3D headers when fully integrated
#ifndef FULL_W3D_INTEGRATION
class ShaderClass {
public:
    uint32_t Get_CRC() const { return 0; }
    // Add other methods as needed
};

class TextureClass {
public:
    static void Apply(int stage) {
        // Mock implementation - does nothing for now
    }
    // Add other methods as needed
};
#else
// Include real W3D classes when fully integrated
#include "shader.h"
#include "texture.h"
#endif

// Static members
IGraphicsRenderer* W3DRendererAdapter::s_renderer = nullptr;
bool W3DRendererAdapter::s_useNewRenderer = false;

bool W3DRendererAdapter::Initialize(GraphicsAPI preferredAPI) {
    // Detect best API if not specified
#ifndef ENABLE_OPENGL
    if (preferredAPI == GraphicsAPI::OPENGL) {
        preferredAPI = GraphicsRendererFactory::DetectBestAPI();
    }
#endif
    
    s_renderer = GraphicsRendererFactory::CreateRenderer(preferredAPI);
    if (!s_renderer) {
        std::cerr << "Failed to create graphics renderer for API: " 
                  << static_cast<int>(preferredAPI) << std::endl;
        
        // Try alternative API
        GraphicsAPI fallback = GraphicsRendererFactory::DetectBestAPI();
        if (fallback != preferredAPI) {
            s_renderer = GraphicsRendererFactory::CreateRenderer(fallback);
        }
    }
    
    if (s_renderer) {
        // Initialize with default values - later get from existing system
        bool success = s_renderer->Initialize(1024, 768, true);
        if (success) {
            s_useNewRenderer = true;
            std::cout << "Graphics Renderer initialized: " 
                      << s_renderer->GetAPIString() << std::endl;
        } else {
            GraphicsRendererFactory::DestroyRenderer(s_renderer);
            s_renderer = nullptr;
        }
        return success;
    }
    
    return false;
}

void W3DRendererAdapter::Shutdown() {
    if (s_renderer) {
        s_renderer->Shutdown();
        GraphicsRendererFactory::DestroyRenderer(s_renderer);
        s_renderer = nullptr;
    }
    s_useNewRenderer = false;
}

void W3DRendererAdapter::BeginFrame() {
    if (s_useNewRenderer && s_renderer) {
        s_renderer->BeginFrame();
    }
    // Otherwise, let the original DX8 system handle it
}

void W3DRendererAdapter::EndFrame() {
    if (s_useNewRenderer && s_renderer) {
        s_renderer->EndFrame();
    }
}

void W3DRendererAdapter::Present() {
    if (s_useNewRenderer && s_renderer) {
        s_renderer->Present();
    }
}

void W3DRendererAdapter::SetTexture(int stage, TextureClass* texture) {
    if (s_useNewRenderer && s_renderer) {
        s_renderer->SetTexture(stage, texture);
    }
}

void W3DRendererAdapter::SetShader(ShaderClass shader) {
    if (s_useNewRenderer && s_renderer) {
        s_renderer->SetShader(&shader);
    }
}

void W3DRendererAdapter::SetTransform(TransformType type, const Matrix4& matrix) {
    if (s_useNewRenderer && s_renderer) {
        switch (type) {
            case TRANSFORM_WORLD:
                s_renderer->SetWorldMatrix(matrix);
                break;
            case TRANSFORM_VIEW:
                s_renderer->SetViewMatrix(matrix);
                break;
            case TRANSFORM_PROJECTION:
                s_renderer->SetProjectionMatrix(matrix);
                break;
        }
    }
}

void W3DRendererAdapter::DrawIndexedPrimitive(
    PrimitiveType type,
    int baseVertex,
    int minIndex,
    int numVertices,
    int startIndex,
    int primitiveCount) {
    
    if (s_useNewRenderer && s_renderer) {
        // Convert DX8 call to new interface
        // Here it would be necessary to adapt the parameters
        s_renderer->DrawIndexedPrimitives(type, nullptr, nullptr, startIndex, primitiveCount);
    }
}

// W3DTextureWrapper implementation
W3DTextureWrapper::W3DTextureWrapper() 
    : m_glTexture(0), m_isGLTexture(false), m_width(0), m_height(0) {
}

W3DTextureWrapper::~W3DTextureWrapper() {
    if (m_isGLTexture && m_glTexture) {
        glDeleteTextures(1, &m_glTexture);
    }
}

bool W3DTextureWrapper::CreateFromData(const void* data, int width, int height, TextureFormat format) {
    if (!data || width <= 0 || height <= 0) return false;
    
    m_width = width;
    m_height = height;
    m_format = format;
    
    glGenTextures(1, &m_glTexture);
    glBindTexture(GL_TEXTURE_2D, m_glTexture);
    
    GLenum glFormat = GL_RGBA;
    GLenum glType = GL_UNSIGNED_BYTE;
    
    // Converter formato
    switch (format) {
        case TEXTURE_FORMAT_RGBA8:
            glFormat = GL_RGBA;
            glType = GL_UNSIGNED_BYTE;
            break;
        case TEXTURE_FORMAT_RGB8:
            glFormat = GL_RGB;
            glType = GL_UNSIGNED_BYTE;
            break;
        case TEXTURE_FORMAT_DXT1:
            // Implement compression if supported
            glFormat = GL_RGB;
            glType = GL_UNSIGNED_BYTE;
            break;
        default:
            glFormat = GL_RGBA;
            glType = GL_UNSIGNED_BYTE;
            break;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, glType, data);
    
    // Default settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    m_isGLTexture = true;
    return true;
}

bool W3DTextureWrapper::CreateFromFile(const char* filename) {
    // Implement file loading
    // For now, return false
    return false;
}

void W3DTextureWrapper::Apply(int stage) {
    if (W3DRendererAdapter::IsUsingNewRenderer()) {
        BindGL(stage);
    } else {
        // Use original DX8 implementation
        TextureClass::Apply(stage);
    }
}

void W3DTextureWrapper::BindGL(int textureUnit) {
    if (m_isGLTexture && m_glTexture) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_glTexture);
    }
}

// ShaderConverter implementation
std::map<uint32_t, ShaderConverter::GLShaderProgram*> ShaderConverter::s_shaderCache;

ShaderConverter::GLShaderProgram* ShaderConverter::ConvertShader(const ShaderClass& dx8Shader) {
    // Generate shader hash for cache
    uint32_t hash = dx8Shader.Get_CRC();
    
    auto it = s_shaderCache.find(hash);
    if (it != s_shaderCache.end()) {
        return it->second;
    }
    
    // Create new shader program
    GLShaderProgram* program = new GLShaderProgram();
    
    const char* vertexSource = GetVertexShaderSource(dx8Shader);
    const char* fragmentSource = GetFragmentShaderSource(dx8Shader);
    
    program->vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    program->fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (program->vertexShader && program->fragmentShader) {
        program->program = LinkProgram(program->vertexShader, program->fragmentShader);
        
        if (program->program) {
            // Get uniform locations
            program->worldMatrix = glGetUniformLocation(program->program, "u_worldMatrix");
            program->viewMatrix = glGetUniformLocation(program->program, "u_viewMatrix");
            program->projectionMatrix = glGetUniformLocation(program->program, "u_projectionMatrix");
            program->diffuseTexture = glGetUniformLocation(program->program, "u_diffuseTexture");
            
            s_shaderCache[hash] = program;
            return program;
        }
    }
    
    // Creation failed
    DestroyShader(program);
    return nullptr;
}

const char* ShaderConverter::GetVertexShaderSource(const ShaderClass& shader) {
    // Basic shader - expand as needed
    return R"(
        #version 330 core
        
        layout(location = 0) in vec3 a_position;
        layout(location = 1) in vec3 a_normal;
        layout(location = 2) in vec2 a_texCoord;
        layout(location = 3) in vec4 a_color;
        
        uniform mat4 u_worldMatrix;
        uniform mat4 u_viewMatrix;
        uniform mat4 u_projectionMatrix;
        
        out vec2 v_texCoord;
        out vec4 v_color;
        out vec3 v_normal;
        
        void main() {
            mat4 mvp = u_projectionMatrix * u_viewMatrix * u_worldMatrix;
            gl_Position = mvp * vec4(a_position, 1.0);
            
            v_texCoord = a_texCoord;
            v_color = a_color;
            v_normal = normalize((u_worldMatrix * vec4(a_normal, 0.0)).xyz);
        }
    )";
}

const char* ShaderConverter::GetFragmentShaderSource(const ShaderClass& shader) {
    // Basic shader - expand as needed
    return R"(
        #version 330 core
        
        in vec2 v_texCoord;
        in vec4 v_color;
        in vec3 v_normal;
        
        uniform sampler2D u_diffuseTexture;
        
        out vec4 fragColor;
        
        void main() {
            vec4 texColor = texture(u_diffuseTexture, v_texCoord);
            fragColor = texColor * v_color;
        }
    )";
}

GLuint ShaderConverter::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            std::cerr << "Shader compile error: " << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint ShaderConverter::LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = new char[infoLen];
            glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
            std::cerr << "Program link error: " << infoLog << std::endl;
            delete[] infoLog;
        }
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

void ShaderConverter::DestroyShader(GLShaderProgram* program) {
    if (program) {
        if (program->program) glDeleteProgram(program->program);
        if (program->vertexShader) glDeleteShader(program->vertexShader);
        if (program->fragmentShader) glDeleteShader(program->fragmentShader);
        delete program;
    }
}

void ShaderConverter::UseShader(GLShaderProgram* program) {
    if (program && program->program) {
        glUseProgram(program->program);
    }
}
