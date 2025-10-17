// Test: Manual Pixel Test - Set specific colors to diagnose sampling issue
// Pattern: Left half RED, Right half GREEN on 1024x256 texture

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Metal wrapper functions
extern "C" {
    void MetalWrapper_Initialize(void* view);
    void MetalWrapper_BeginFrame();
    void MetalWrapper_EndFrame();
    void* MetalWrapper_CreateTextureRaw(unsigned int width, unsigned int height, const void* data);
    void MetalWrapper_BindTexture(void* texture, unsigned int slot);
    void* MetalWrapper_CreateVertexBuffer(unsigned long size, const void* data);
    void* MetalWrapper_CreateIndexBuffer(unsigned long size, const void* data);
    void MetalWrapper_UpdateVertexBuffer(void* buffer, const void* data, unsigned long size);
    void MetalWrapper_DrawIndexedPrimitive(unsigned int primitiveType, unsigned int primitiveCount, 
                                          unsigned int indicesCount, unsigned int startIndex, unsigned int baseVertexIndex);
    void MetalWrapper_SetProjectionMatrix(const float* matrix);
}

// Vertex structure (matches Metal shader)
struct Vertex {
    float x, y, z;    // Position
    float u, v;       // Texture coordinates
};

int main(int argc, char* argv[]) {
    printf("\n=== MANUAL PIXEL TEST: LEFT RED / RIGHT GREEN ===\n\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("ERROR: SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Manual Pixel Test - 1024x256",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 768,
        SDL_WINDOW_METAL | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    if (!window) {
        printf("ERROR: SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Get Metal view
    void* metalView = SDL_Metal_CreateView(window);
    if (!metalView) {
        printf("ERROR: SDL_Metal_CreateView failed\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize Metal
    MetalWrapper_Initialize(metalView);
    printf("✓ Metal initialized\n");
    
    // Create 1024x256 texture with RED left half, GREEN right half
    const unsigned int width = 1024;
    const unsigned int height = 256;
    const unsigned int pixelCount = width * height;
    const unsigned int dataSize = pixelCount * 4;  // RGBA8
    
    unsigned char* pixelData = new unsigned char[dataSize];
    
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int index = (y * width + x) * 4;
            
            if (x < width / 2) {
                // LEFT HALF: RED (255, 0, 0, 255)
                pixelData[index + 0] = 255;  // R
                pixelData[index + 1] = 0;    // G
                pixelData[index + 2] = 0;    // B
                pixelData[index + 3] = 255;  // A
            } else {
                // RIGHT HALF: GREEN (0, 255, 0, 255)
                pixelData[index + 0] = 0;    // R
                pixelData[index + 1] = 255;  // G
                pixelData[index + 2] = 0;    // B
                pixelData[index + 3] = 255;  // A
            }
        }
    }
    
    printf("✓ Created test pattern: LEFT RED / RIGHT GREEN (%ux%u, %u bytes)\n", 
           width, height, dataSize);
    
    // Upload to Metal
    void* texture = MetalWrapper_CreateTextureRaw(width, height, pixelData);
    if (!texture) {
        printf("ERROR: Failed to create texture\n");
        delete[] pixelData;
        SDL_Metal_DestroyView(metalView);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("✓ Texture uploaded to Metal (ID=%p)\n", texture);
    delete[] pixelData;
    
    // Create fullscreen quad vertices
    Vertex vertices[4] = {
        // Position (NDC)      UV
        { -1.0f,  1.0f, 0.5f,  0.0f, 0.0f },  // Top-left
        {  1.0f,  1.0f, 0.5f,  1.0f, 0.0f },  // Top-right
        {  1.0f, -1.0f, 0.5f,  1.0f, 1.0f },  // Bottom-right
        { -1.0f, -1.0f, 0.5f,  0.0f, 1.0f }   // Bottom-left
    };
    
    unsigned short indices[6] = { 0, 1, 2, 0, 2, 3 };
    
    void* vb = MetalWrapper_CreateVertexBuffer(sizeof(vertices), vertices);
    void* ib = MetalWrapper_CreateIndexBuffer(sizeof(indices), indices);
    
    if (!vb || !ib) {
        printf("ERROR: Failed to create buffers\n");
        SDL_Metal_DestroyView(metalView);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("✓ Vertex/Index buffers created\n");
    
    // Set orthographic projection (identity for fullscreen quad in NDC)
    float projMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    MetalWrapper_SetProjectionMatrix(projMatrix);
    
    printf("\n=== EXPECTED RESULT ===\n");
    printf("Left half of screen: RED\n");
    printf("Right half of screen: GREEN\n");
    printf("If you see orange on right: BUG CONFIRMED (sampling issue)\n");
    printf("If you see green on right: BUG FIXED\n\n");
    printf("Press ESC to exit\n\n");
    
    // Main loop
    bool running = true;
    SDL_Event event;
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }
        
        // Render
        MetalWrapper_BeginFrame();
        
        // Bind texture and draw
        MetalWrapper_BindTexture(texture, 0);
        MetalWrapper_DrawIndexedPrimitive(4, 2, 6, 0, 0);  // Type 4 = Triangle List
        
        MetalWrapper_EndFrame();
        
        SDL_Delay(16);  // ~60 FPS
    }
    
    // Cleanup
    SDL_Metal_DestroyView(metalView);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("\nTest completed.\n");
    
    return 0;
}
