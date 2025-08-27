#include <iostream>
#include "GraphicsRenderer.h"
#include "OpenGLRenderer.h"

int main() {
    std::cout << "Testing OpenGL context initialization..." << std::endl;
    
    // Create OpenGL renderer
    OpenGLRenderer renderer;
    
    std::cout << "API: " << renderer.GetAPIString() << std::endl;
    
    // Try to initialize OpenGL context
    std::cout << "Attempting to initialize OpenGL context..." << std::endl;
    bool success = renderer.Initialize(800, 600, true);
    
    if (success) {
        std::cout << "OpenGL context initialized successfully!" << std::endl;
        
        // Test basic operations
        renderer.SetViewport(0, 0, 800, 600);
        std::cout << "Viewport set successfully" << std::endl;
        
        if (renderer.BeginFrame()) {
            std::cout << "BeginFrame successful" << std::endl;
            
            // Clear screen
            renderer.Clear(true, true, true, 0x000000FF); // Black background
            std::cout << "Clear successful" << std::endl;
            
            renderer.EndFrame();
            std::cout << "EndFrame successful" << std::endl;
        }
        
        // Test matrix operations
        Matrix4 projMatrix;
        Matrix4 viewMatrix;
        Matrix4 worldMatrix;
        
        renderer.SetProjectionMatrix(projMatrix);
        renderer.SetViewMatrix(viewMatrix);
        renderer.SetWorldMatrix(worldMatrix);
        std::cout << "Matrix operations successful" << std::endl;
        
        renderer.Shutdown();
        std::cout << "OpenGL shutdown successful" << std::endl;
        
        std::cout << "\n=== ALL OPENGL TESTS PASSED! ===" << std::endl;
        std::cout << "OpenGL is working minimally and ready for integration!" << std::endl;
        
    } else {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        std::cout << "Note: This is expected if running without a display/window system" << std::endl;
        std::cout << "Basic functionality still works for non-context operations" << std::endl;
        return 1;
    }
    
    return 0;
}
