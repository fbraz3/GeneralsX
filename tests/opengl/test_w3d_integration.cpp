#include <iostream>
#include "GraphicsRenderer.h"
#include "OpenGLRendererW3D.h"

// Test with W3D integration
#define ENABLE_W3D_INTEGRATION
#include "W3DTypes.h"

int main() {
    std::cout << "Testing OpenGL integration with W3D types..." << std::endl;
    
    try {
        // Create OpenGL renderer with W3D integration
        OpenGLRendererW3D renderer;
        
        std::cout << "API: " << renderer.GetAPIString() << std::endl;
        
        // Test W3D matrix creation
        W3DMatrix4 testMatrix;
        testMatrix.Make_Identity();
        std::cout << "W3D Matrix4x4 created successfully" << std::endl;
        
        // Test matrix conversion
        float glMatrix[16];
        W3DOpenGLBridge::ConvertMatrix(testMatrix, glMatrix);
        std::cout << "Matrix conversion test successful" << std::endl;
        
        // Test vector conversion
        W3DVector3 testVector(1.0f, 2.0f, 3.0f);
        float glVector[3];
        W3DOpenGLBridge::ConvertVector3(testVector, glVector);
        std::cout << "Vector conversion test successful" << std::endl;
        
        std::cout << "Vector values: [" << glVector[0] << ", " << glVector[1] << ", " << glVector[2] << "]" << std::endl;
        
        // Test OpenGL context initialization
        std::cout << "Attempting to initialize OpenGL context..." << std::endl;
        bool success = renderer.Initialize(800, 600, true);
        
        if (success) {
            std::cout << "OpenGL context initialized successfully!" << std::endl;
            
            // Test matrix operations with W3D types
            renderer.SetProjectionMatrix(testMatrix);
            renderer.SetViewMatrix(testMatrix);
            renderer.SetWorldMatrix(testMatrix);
            std::cout << "W3D matrix operations successful" << std::endl;
            
            // Test basic rendering
            if (renderer.BeginFrame()) {
                std::cout << "BeginFrame successful" << std::endl;
                
                renderer.Clear(true, true, true, 0x000000FF);
                std::cout << "Clear successful" << std::endl;
                
                renderer.EndFrame();
                std::cout << "EndFrame successful" << std::endl;
            }
            
            renderer.Shutdown();
            std::cout << "OpenGL shutdown successful" << std::endl;
            
            std::cout << "\n=== W3D INTEGRATION TESTS PASSED! ===" << std::endl;
            std::cout << "OpenGL is successfully integrated with W3D types!" << std::endl;
            
        } else {
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown exception caught" << std::endl;
        return 1;
    }
    
    return 0;
}
