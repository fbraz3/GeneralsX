#include <iostream>
#include <cstdlib>

// Simple test to validate Metal initialization without requiring full game engine
#ifdef __APPLE__
#include "metalwrapper.h"

int main() {
    std::cout << "Metal Init Test - Starting..." << std::endl;
    
    // Test without SDL window (headless mode)
    GX::MetalConfig config;
    config.sdlWindow = nullptr;  // Headless test
    config.width = 800;
    config.height = 600;
    config.vsync = false;  // Disable vsync for headless
    
    if (GX::MetalWrapper::Initialize(config)) {
        std::cout << "✅ Metal initialization successful" << std::endl;
        
        // Test a few frame cycles
        for (int i = 0; i < 3; ++i) {
            GX::MetalWrapper::BeginFrame(0.2f, 0.4f, 0.8f, 1.0f);  // Blue background
            GX::MetalWrapper::EndFrame();
        }
        std::cout << "✅ Frame rendering cycles completed" << std::endl;
        
        GX::MetalWrapper::Shutdown();
        std::cout << "✅ Metal shutdown successful" << std::endl;
        
        return 0;
    } else {
        std::cout << "❌ Metal initialization failed" << std::endl;
        return 1;
    }
}

#else
int main() {
    std::cout << "Metal Init Test - Skipped (not on macOS)" << std::endl;
    return 0;
}
#endif