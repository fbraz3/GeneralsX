#include "GraphicsAPI/GraphicsRenderer.h"
#include "GraphicsAPI/W3DRendererAdapter.h"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    std::cout << "=== Generals OpenGL Port Test ===" << std::endl;
    
    // Parse command line arguments
    GraphicsAPI requestedAPI = GraphicsAPI::OPENGL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dx8") == 0 || strcmp(argv[i], "--directx") == 0) {
            requestedAPI = GraphicsAPI::DIRECTX8;
        } else if (strcmp(argv[i], "--opengl") == 0 || strcmp(argv[i], "--gl") == 0) {
            requestedAPI = GraphicsAPI::OPENGL;
        }
    }
    
    std::cout << "Requested API: " << (requestedAPI == GraphicsAPI::OPENGL ? "OpenGL" : "DirectX 8") << std::endl;
    
    // Initialize graphics system
    if (!W3DRendererAdapter::Initialize(requestedAPI)) {
        std::cerr << "Failed to initialize graphics system!" << std::endl;
        return 1;
    }
    
    IGraphicsRenderer* renderer = W3DRendererAdapter::GetRenderer();
    if (!renderer) {
        std::cerr << "No renderer available!" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully initialized: " << renderer->GetAPIString() << std::endl;
    
    // Initialize with test resolution
    if (!renderer->Initialize(800, 600, true)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        W3DRendererAdapter::Shutdown();
        return 1;
    }
    
    std::cout << "Renderer initialized with 800x600 windowed mode" << std::endl;
    
    // Simple render loop for testing
    std::cout << "Starting render test (will run for 5 seconds)..." << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;
    
    while (true) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        
        if (elapsed.count() >= 5) {
            break;
        }
        
        // Begin frame
        if (!renderer->BeginFrame()) {
            std::cerr << "Failed to begin frame!" << std::endl;
            break;
        }
        
        // Clear to a nice blue color
        renderer->Clear(true, true, false, 0xFF4080FF);
        
        // End frame
        renderer->EndFrame();
        renderer->Present();
        
        frameCount++;
        
        // Sleep to limit to ~60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    float fps = frameCount / 5.0f;
    std::cout << "Render test completed. Average FPS: " << fps << std::endl;
    
    // Cleanup
    W3DRendererAdapter::Shutdown();
    
    std::cout << "Graphics system shut down successfully." << std::endl;
    std::cout << "=== Test Complete ===" << std::endl;
    
    return 0;
}
