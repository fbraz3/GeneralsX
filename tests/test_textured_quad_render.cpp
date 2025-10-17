/**
 * @file test_textured_quad_render.cpp
 * @brief Phase 28.4.2-28.4.3: Test TexturedQuad rendering with actual game textures
 * 
 * This test:
 * 1. Initializes Metal backend with SDL window
 * 2. Loads defeated.dds (1024x256 BC3) and GameOver.tga (1024x256 RGBA8)
 * 3. Renders multiple textured quads at different positions
 * 4. Tests UV mapping, color tinting, and alpha blending
 * 5. Validates visual output (requires manual inspection)
 */

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

// Game engine includes
#include "texturedquad.h"
#include "texturecache.h"
#include "metalwrapper.h"

// Test configuration
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;
const char* WINDOW_TITLE = "Phase 28.4 - TexturedQuad Rendering Test";

// Asset paths (relative to game data directory)
const char* TEXTURE_DEFEATED = "Data/English/Art/Textures/defeated.dds";
const char* TEXTURE_GAMEOVER = "Data/English/Art/Textures/GameOver.tga";
const char* TEXTURE_WATER = "Data/WaterPlane/caust00.tga";

// Test mode flag
bool USE_MANUAL_TEST_PATTERN = (getenv("USE_MANUAL_PATTERN") != nullptr);

/**
 * @brief Initialize SDL and Metal
 */
bool InitializeGraphics(SDL_Window** window) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    
    // Create window
    *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    if (!*window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // Initialize Metal backend
    GX::MetalConfig config;
    config.sdlWindow = *window;
    config.width = WINDOW_WIDTH;
    config.height = WINDOW_HEIGHT;
    config.vsync = true;
    
    if (!GX::MetalWrapper::Initialize(config)) {
        printf("MetalWrapper::Initialize failed\n");
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }
    
    printf("✓ Graphics initialized: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

/**
 * @brief Shutdown graphics and SDL
 */
void ShutdownGraphics(SDL_Window* window) {
    GX::MetalWrapper::Shutdown();
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

/**
 * @brief Create test texture pattern: RED left / GREEN right
 * @return Texture handle (pointer) or nullptr on failure
 */
void* CreateTestTexturePattern(unsigned int width, unsigned int height) {
    printf("\n=== Creating Manual Test Pattern ===\n");
    printf("Size: %ux%u\n", width, height);
    printf("Pattern: LEFT RED (255,0,0) | RIGHT GREEN (0,255,0)\n");
    
    // Allocate RGBA8 data
    unsigned int pixelCount = width * height;
    unsigned int dataSize = pixelCount * 4;
    unsigned char* pixelData = new unsigned char[dataSize];
    
    // Fill with pattern
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int index = (y * width + x) * 4;
            
            if (x < width / 2) {
                // LEFT HALF: RED
                pixelData[index + 0] = 255;  // R
                pixelData[index + 1] = 0;    // G
                pixelData[index + 2] = 0;    // B
                pixelData[index + 3] = 255;  // A
            } else {
                // RIGHT HALF: GREEN
                pixelData[index + 0] = 0;    // R
                pixelData[index + 1] = 255;  // G
                pixelData[index + 2] = 0;    // B
                pixelData[index + 3] = 255;  // A
            }
        }
    }
    
    // Upload to Metal
    void* texture = GX::MetalWrapper::CreateTextureFromTGA(
        width, height,
        pixelData,
        dataSize
    );
    
    delete[] pixelData;
    
    if (texture) {
        printf("✓ Test texture created (ID=%p)\n", texture);
    } else {
        printf("✗ Failed to create test texture\n");
    }
    
    return texture;
}

/**
 * @brief Test 1: Load textures from game assets
 */
bool Test1_LoadTextures() {
    printf("\n=== Test 1: Load Textures ===\n");
    
    // Load defeated.dds (BC3 compressed)
    void* tex_defeated = TextureCache::GetInstance()->LoadTexture(TEXTURE_DEFEATED);
    if (!tex_defeated) {
        printf("✗ FAILED: Could not load %s\n", TEXTURE_DEFEATED);
        return false;
    }
    printf("✓ Loaded defeated.dds\n");
    
    // Load GameOver.tga (RGBA8 uncompressed)
    void* tex_gameover = TextureCache::GetInstance()->LoadTexture(TEXTURE_GAMEOVER);
    if (!tex_gameover) {
        printf("✗ FAILED: Could not load %s\n", TEXTURE_GAMEOVER);
        return false;
    }
    printf("✓ Loaded GameOver.tga\n");
    
    // Load water caustic (small TGA for testing)
    void* tex_water = TextureCache::GetInstance()->LoadTexture(TEXTURE_WATER);
    if (!tex_water) {
        printf("✗ FAILED: Could not load %s\n", TEXTURE_WATER);
        return false;
    }
    printf("✓ Loaded caust00.tga\n");
    
    // Check cache stats
    int total_textures;
    unsigned long estimated_memory;
    TextureCache::GetInstance()->GetCacheStats(total_textures, estimated_memory);
    printf("✓ TextureCache stats: %d textures, ~%lu MB\n", 
           total_textures, estimated_memory / (1024 * 1024));
    
    return true;
}

/**
 * @brief Test 2: Create TexturedQuad instances
 */
bool Test2_CreateQuads(GX::TexturedQuad* quads, int count) {
    printf("\n=== Test 2: Create TexturedQuads ===\n");
    
    // MANUAL TEST PATTERN MODE
    if (USE_MANUAL_TEST_PATTERN) {
        printf("\n*** MANUAL TEST PATTERN MODE ***\n");
        printf("Creating 1024x256 texture: LEFT RED | RIGHT GREEN\n\n");
        
        void* testTexture = CreateTestTexturePattern(1024, 256);
        if (!testTexture) {
            printf("✗ FAILED: Test pattern creation\n");
            return false;
        }
        
        // Quad 1: Test pattern fullscreen
        if (!quads[0].SetTextureHandle(testTexture)) {
            printf("✗ FAILED: Quad 0 texture bind\n");
            return false;
        }
        quads[0].SetPosition(50, 50, 1024, 256);
        quads[0].SetColor(1.0f, 1.0f, 1.0f, 1.0f); // Fully opaque
        printf("✓ Quad 0: Test pattern (50, 50, 1024x256)\n");
        
        printf("\n=== EXPECTED RESULT ===\n");
        printf("LEFT side: PURE RED (255, 0, 0)\n");
        printf("RIGHT side: PURE GREEN (0, 255, 0)\n");
        printf("If RIGHT side is ORANGE: BUG CONFIRMED (sampling issue)\n");
        printf("If RIGHT side is GREEN: BUG FIXED\n\n");
        
        return true;
    }
    
    // NORMAL MODE: Load game textures
    // Quad 1: defeated.dds at top-left (full size)
    if (!quads[0].SetTexture(TEXTURE_DEFEATED)) {
        printf("✗ FAILED: Quad 0 texture load\n");
        return false;
    }
    quads[0].SetPosition(50, 50, 1024, 256);
    quads[0].SetColor(1.0f, 1.0f, 1.0f, 1.0f); // Fully opaque
    printf("✓ Quad 0: defeated.dds (50, 50, 1024x256) - opaque\n");
    
    // Quad 2: GameOver.tga at center (half size, semi-transparent)
    if (!quads[1].SetTexture(TEXTURE_GAMEOVER)) {
        printf("✗ FAILED: Quad 1 texture load\n");
        return false;
    }
    quads[1].SetPosition(200, 350, 512, 128);
    quads[1].SetColor(1.0f, 1.0f, 1.0f, 0.7f); // 70% opacity
    printf("✓ Quad 1: GameOver.tga (200, 350, 512x128) - 70%% alpha\n");
    
    // Quad 3: caust00.tga at bottom-right (small, color tinted)
    if (!quads[2].SetTexture(TEXTURE_WATER)) {
        printf("✗ FAILED: Quad 2 texture load\n");
        return false;
    }
    quads[2].SetPosition(1000, 600, 128, 128);
    quads[2].SetColor(0.5f, 0.8f, 1.0f, 0.9f); // Blue tint, 90% opacity
    printf("✓ Quad 2: caust00.tga (1000, 600, 128x128) - blue tint\n");
    
    // Quad 4: defeated.dds with custom UVs (top half only)
    if (!quads[3].SetTexture(TEXTURE_DEFEATED)) {
        printf("✗ FAILED: Quad 3 texture load\n");
        return false;
    }
    quads[3].SetPosition(50, 500, 512, 128);
    quads[3].SetUVs(0.0f, 0.0f, 1.0f, 0.5f); // Top half of texture
    quads[3].SetColor(1.0f, 0.8f, 0.6f, 1.0f); // Orange tint
    printf("✓ Quad 3: defeated.dds (50, 500, 512x128) - custom UVs (top half)\n");
    
    return true;
}

/**
 * @brief Test 3: Render quads in frame loop
 */
void Test3_RenderLoop(SDL_Window* window, GX::TexturedQuad* quads, int count) {
    printf("\n=== Test 3: Render Loop ===\n");
    printf("Rendering %d quads for 5 seconds...\n", count);
    printf("Press ESC or close window to exit early\n\n");
    
    bool running = true;
    SDL_Event event;
    Uint32 start_time = SDL_GetTicks();
    Uint32 frame_count = 0;
    
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
        
        // Check if 5 seconds elapsed
        Uint32 current_time = SDL_GetTicks();
        if (current_time - start_time >= 5000) {
            running = false;
        }
        
        // Begin frame (clear to dark blue)
        GX::MetalWrapper::BeginFrame(0.1f, 0.1f, 0.2f, 1.0f);
        
        // Render all quads
        for (int i = 0; i < count; i++) {
            quads[i].Render();
        }
        
        // End frame and present
        GX::MetalWrapper::EndFrame();
        
        frame_count++;
        
        // Print progress every 60 frames (~1 second at 60 FPS)
        if (frame_count % 60 == 0) {
            float elapsed = (current_time - start_time) / 1000.0f;
            printf("Frame %u (%.1fs elapsed)\n", frame_count, elapsed);
        }
    }
    
    // Calculate FPS
    Uint32 total_time = SDL_GetTicks() - start_time;
    float fps = (frame_count * 1000.0f) / total_time;
    printf("\n✓ Rendered %u frames in %.2fs (%.1f FPS)\n", 
           frame_count, total_time / 1000.0f, fps);
}

/**
 * @brief Main test function
 */
int main(int argc, char* argv[]) {
    printf("=================================================\n");
    printf("Phase 28.4.2-28.4.3: TexturedQuad Rendering Test\n");
    printf("=================================================\n\n");
    
    // Change to game data directory
    const char* game_dir = getenv("HOME");
    if (game_dir) {
        char path[512];
        snprintf(path, sizeof(path), "%s/GeneralsX/GeneralsMD", game_dir);
        if (chdir(path) != 0) {
            printf("WARNING: Could not change to game directory: %s\n", path);
            printf("Make sure game assets are installed!\n");
        } else {
            printf("Working directory: %s\n", path);
        }
    }
    
    SDL_Window* window = nullptr;
    GX::TexturedQuad quads[4];
    bool success = true;
    
    // Initialize graphics
    if (!InitializeGraphics(&window)) {
        printf("\n✗ FATAL: Graphics initialization failed\n");
        return 1;
    }
    
    // Test 1: Load textures
    if (!Test1_LoadTextures()) {
        printf("\n✗ FATAL: Texture loading failed\n");
        success = false;
        goto cleanup;
    }
    
    // Test 2: Create quads
    if (!Test2_CreateQuads(quads, 4)) {
        printf("\n✗ FATAL: Quad creation failed\n");
        success = false;
        goto cleanup;
    }
    
    // Test 3: Render loop
    Test3_RenderLoop(window, quads, 4);
    
cleanup:
    // Cleanup
    printf("\n=== Cleanup ===\n");
    
    // Quads will auto-release textures in destructors
    printf("✓ Quads destroyed\n");
    
    // Check final cache state
    int total_textures;
    unsigned long estimated_memory;
    TextureCache::GetInstance()->GetCacheStats(total_textures, estimated_memory);
    printf("✓ Final cache: %d textures, ~%lu MB\n", 
           total_textures, estimated_memory / (1024 * 1024));
    
    // Clear cache
    TextureCache::GetInstance()->ClearCache();
    printf("✓ Cache cleared\n");
    
    // Shutdown graphics
    ShutdownGraphics(window);
    printf("✓ Graphics shutdown\n");
    
    // Final result
    printf("\n=================================================\n");
    if (success) {
        printf("✓ ALL TESTS PASSED\n");
        printf("=================================================\n\n");
        printf("Visual Validation Checklist (Phase 28.4.4):\n");
        printf("  [ ] Quad 0: defeated.dds visible at top-left\n");
        printf("  [ ] Quad 1: GameOver.tga semi-transparent at center\n");
        printf("  [ ] Quad 2: caust00.tga blue-tinted at bottom-right\n");
        printf("  [ ] Quad 3: defeated.dds top-half with orange tint\n");
        printf("  [ ] No texture distortion or UV mapping issues\n");
        printf("  [ ] Alpha blending working correctly\n");
        printf("  [ ] No visual artifacts or glitches\n");
        return 0;
    } else {
        printf("✗ SOME TESTS FAILED\n");
        printf("=================================================\n");
        return 1;
    }
}
