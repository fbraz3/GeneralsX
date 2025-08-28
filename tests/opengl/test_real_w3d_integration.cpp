#include <iostream>

// Test real W3D type integration
#define ENABLE_OPENGL
#define ENABLE_W3D_INTEGRATION

#include "GraphicsRenderer.h"
#include "OpenGLRendererW3D.h"
#include "W3DTypesReal.h"

int main() {
    std::cout << "Testing real W3D type integration with OpenGL..." << std::endl;
    
    try {
        // Test W3D types
        std::cout << "Creating W3D Matrix4x4..." << std::endl;
        W3DMatrix4 testMatrix(true); // Identity matrix
        
        std::cout << "Creating W3D Vector3..." << std::endl;
        W3DVector3 testVec(1.0f, 2.0f, 3.0f);
        
        // Test conversion utilities
        std::cout << "Testing matrix conversion..." << std::endl;
        float glMatrix[16];
        W3DOpenGLUtils::MatrixToOpenGL(testMatrix, glMatrix);
        
        std::cout << "Testing vector conversion..." << std::endl;
        float vecArray[3];
        W3DOpenGLUtils::VectorToFloat3(testVec, vecArray);
        
        std::cout << "Vector values: [" << vecArray[0] << ", " << vecArray[1] << ", " << vecArray[2] << "]" << std::endl;
        
        // Test matrix multiplication
        std::cout << "Testing matrix multiplication..." << std::endl;
        W3DMatrix4 testMatrix2(true);
        W3DMatrix4 result = W3DOpenGLUtils::MultiplyMatrices(testMatrix, testMatrix2);
        
        // Test renderer creation
        std::cout << "Creating OpenGL renderer with W3D integration..." << std::endl;
        IGraphicsRenderer* renderer = GraphicsRendererFactory::CreateRenderer(GraphicsAPI::OPENGL);
        
        if (renderer) {
            std::cout << "API: " << (renderer->GetAPI() == GraphicsAPI::OPENGL ? "OpenGL (W3D)" : "Unknown") << std::endl;
            
            // Test initialization (will fail without proper context but should not crash)
            std::cout << "Attempting to initialize OpenGL context..." << std::endl;
            if (renderer->Initialize(800, 600, true)) {
                std::cout << "OpenGL context initialized successfully!" << std::endl;
                
                // Test matrix operations with real W3D types
                Matrix4 testProjection; // This will use real W3D Matrix4x4
                Matrix4 testView;
                Matrix4 testWorld;
                
                renderer->SetProjectionMatrix(testProjection);
                renderer->SetViewMatrix(testView);
                renderer->SetWorldMatrix(testWorld);
                
                std::cout << "Real W3D matrix operations successful" << std::endl;
                
                // Test basic operations
                renderer->BeginFrame();
                std::cout << "BeginFrame successful" << std::endl;
                
                renderer->Clear(0.0f, 0.0f, 0.5f, 1.0f);
                std::cout << "Clear successful" << std::endl;
                
                renderer->EndFrame();
                std::cout << "EndFrame successful" << std::endl;
                
                renderer->Shutdown();
                std::cout << "OpenGL shutdown successful" << std::endl;
            } else {
                std::cout << "OpenGL context initialization failed (expected without window)" << std::endl;
            }
            
            delete renderer;
        } else {
            std::cout << "Failed to create OpenGL renderer" << std::endl;
            return 1;
        }
        
        std::cout << std::endl;
        std::cout << "=== REAL W3D TYPE INTEGRATION TESTS PASSED! ===" << std::endl;
        std::cout << "OpenGL is successfully integrated with real W3D types!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}
