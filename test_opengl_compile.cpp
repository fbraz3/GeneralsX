/**
 * Simple test to verify OpenGL graphics system compilation
 * This file tests basic compilation without full W3D dependencies
 */

#include <iostream>
#include <cstdint>

// Define ENABLE_OPENGL to test OpenGL code paths
#define ENABLE_OPENGL 1

// Mock Matrix4 class for testing
class Matrix4 {
public:
    float m[16];
    Matrix4() { for(int i = 0; i < 16; i++) m[i] = 0.0f; }
};

// Mock Vector classes
class Vector3 {
public:
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
};

class Vector4 {
public:
    float x, y, z, w;
    Vector4() : x(0), y(0), z(0), w(0) {}
};

// Mock W3D classes for testing
class TextureClass {
public:
    virtual ~TextureClass() = default;
    virtual void Apply(int stage) = 0;
    virtual int Get_Width() const = 0;
    virtual int Get_Height() const = 0;
};

class VertexBufferClass {
public:
    virtual ~VertexBufferClass() = default;
};

class IndexBufferClass {
public:
    virtual ~IndexBufferClass() = default;
};

class ShaderClass {
public:
    virtual ~ShaderClass() = default;
    virtual uint32_t Get_CRC() const { return 0; }
};

// Include our graphics system
#include "Core/Libraries/Include/GraphicsAPI/GraphicsRenderer.h"
#include "Core/Libraries/Include/GraphicsAPI/OpenGLRenderer.h"
#include "Core/Libraries/Include/GraphicsAPI/W3DRendererAdapter.h"

int main() {
    std::cout << "Testing OpenGL Graphics System Compilation..." << std::endl;
    
    try {
        // Test factory creation
        IGraphicsRenderer* renderer = GraphicsRendererFactory::CreateRenderer(GraphicsAPI::OPENGL);
        
        if (renderer) {
            std::cout << "✓ OpenGL renderer created successfully" << std::endl;
            std::cout << "  API: " << renderer->GetAPIString() << std::endl;
            
            // Test basic initialization (will fail without proper window, but should compile)
            bool initialized = renderer->Initialize(800, 600, true);
            std::cout << "  Initialization test: " << (initialized ? "SUCCESS" : "EXPECTED_FAILURE") << std::endl;
            
            GraphicsRendererFactory::DestroyRenderer(renderer);
            std::cout << "✓ Renderer destroyed successfully" << std::endl;
        } else {
            std::cout << "✗ Failed to create OpenGL renderer" << std::endl;
            return 1;
        }
        
        // Test W3D adapter
        bool adapterInit = W3DRendererAdapter::Initialize(GraphicsAPI::OPENGL);
        std::cout << "✓ W3D Adapter initialization: " << (adapterInit ? "SUCCESS" : "EXPECTED_FAILURE") << std::endl;
        
        std::cout << "✓ All compilation tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Exception during test: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "✗ Unknown exception during test" << std::endl;
        return 1;
    }
}
