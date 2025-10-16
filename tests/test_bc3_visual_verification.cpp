/**
 * @file test_bc3_visual_verification.cpp
 * @brief Phase 28.4: Visual verification test for BC3 bug fix
 * 
 * This test renders BC3 textures side-by-side:
 * - LEFT: Native BC3 (with bug - orange half)
 * - RIGHT: Decompressed RGBA8 (workaround - should be correct)
 * 
 * Press SPACE to toggle between modes, ESC to exit
 */

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

// Game engine includes
#include "texturedquad.h"
#include "texturecache.h"
#include "metalwrapper.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;

int main(int argc, char* argv[]) {
    printf("=================================================\n");
    printf("Phase 28.4: BC3 Bug Visual Verification Test\n");
    printf("=================================================\n\n");
    
    // Change to game data directory
    const char* game_dir = getenv("HOME");
    if (game_dir) {
        char path[512];
        snprintf(path, sizeof(path), "%s/GeneralsX/GeneralsMD", game_dir);
        if (chdir(path) == 0) {
            printf("Working directory: %s\n", path);
        }
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow(
        "BC3 Bug Verification - Press SPACE to toggle, ESC to exit",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Initialize Metal
    GX::MetalConfig config;
    config.sdlWindow = window;
    config.width = WINDOW_WIDTH;
    config.height = WINDOW_HEIGHT;
    config.vsync = true;
    
    if (!GX::MetalWrapper::Initialize(config)) {
        printf("MetalWrapper::Initialize failed\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("\n=== Test Instructions ===\n");
    printf("Mode 1: Native BC3 (BUG - right half should be orange)\n");
    printf("Mode 2: Decompressed RGBA8 (FIX - should be fully correct)\n");
    printf("\nPress SPACE to toggle modes\n");
    printf("Press ESC to exit\n\n");
    
    // Create quads
    GX::TexturedQuad quad;
    bool useDecompression = false;
    bool running = true;
    SDL_Event event;
    
    // Load texture once
    const char* texture_path = "Data/English/Art/Textures/defeated.dds";
    
    // Initial load (native BC3)
    printf("Loading NATIVE BC3...\n");
    unsetenv("USE_BC3_DECOMPRESSION");
    TextureCache::GetInstance()->ClearCache();
    quad.SetTexture(texture_path);
    quad.SetPosition(100, 200, 1024, 256);
    quad.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    int frame_count = 0;
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    // Toggle mode
                    useDecompression = !useDecompression;
                    
                    if (useDecompression) {
                        printf("\n=== Switching to DECOMPRESSED RGBA8 mode ===\n");
                        setenv("USE_BC3_DECOMPRESSION", "1", 1);
                    } else {
                        printf("\n=== Switching to NATIVE BC3 mode ===\n");
                        unsetenv("USE_BC3_DECOMPRESSION");
                    }
                    
                    // Reload texture
                    TextureCache::GetInstance()->ClearCache();
                    quad.SetTexture(texture_path);
                    quad.SetPosition(100, 200, 1024, 256);
                    quad.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
                }
            }
        }
        
        // Render
        GX::MetalWrapper::BeginFrame(0.1f, 0.1f, 0.2f, 1.0f);
        
        // Draw text overlay (mode indicator)
        // Note: We don't have text rendering yet, so just render the quad
        quad.Render();
        
        GX::MetalWrapper::EndFrame();
        
        frame_count++;
        
        // Print status every 60 frames
        if (frame_count % 60 == 0) {
            printf("Frame %d - Mode: %s\n", frame_count, 
                   useDecompression ? "DECOMPRESSED (FIXED)" : "NATIVE BC3 (BUG)");
        }
    }
    
    printf("\n=== Test Complete ===\n");
    printf("Total frames rendered: %d\n", frame_count);
    
    // Cleanup
    TextureCache::GetInstance()->ClearCache();
    GX::MetalWrapper::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("\nVisual verification required:\n");
    printf("  - Native BC3: Did right half show ORANGE bug?\n");
    printf("  - Decompressed: Did texture show CORRECTLY (no orange)?\n");
    
    return 0;
}
