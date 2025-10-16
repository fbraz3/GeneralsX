// Phase 28.4.2-28.4.3: Textured Quad Render Test with Metal Render Pass
// Tests texture loading and rendering with actual game assets

#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>

// W3D includes
#include "texturecache.h"
#include "texturedquad.h"
#include "metalwrapper.h"

// Game assets
SDL_Window* g_window = nullptr;

// Test state
GX::TexturedQuad* g_quads[4] = {nullptr, nullptr, nullptr, nullptr};

bool InitializeGraphics() {
    printf("\n=== INITIALIZING GRAPHICS ===\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("ERROR: SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    printf("✅ SDL initialized\n");
    
    // Create window
    g_window = SDL_CreateWindow(
        "Phase 28.4 - Textured Quad Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 768,
        SDL_WINDOW_SHOWN | SDL_WINDOW_METAL
    );
    
    if (!g_window) {
        printf("ERROR: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    printf("✅ SDL window created (1280x768)\n");
    
    // Initialize Metal backend
    MetalWrapper::MetalConfig config;
    config.width = 1280;
    config.height = 768;
    config.window = g_window;
    
    if (!MetalWrapper::Initialize(config)) {
        printf("ERROR: MetalWrapper::Initialize failed\n");
        return false;
    }
    printf("✅ Metal backend initialized\n");
    
    return true;
}

bool Test1_LoadTextures() {
    printf("\n=== TEST 1: LOADING TEXTURES ===\n");
    
    // Load defeated.dds (BC3 compressed)
    printf("\n1. Loading defeated.dds (BC3)...\n");
    g_textures[0] = g_textureCache.GetTexture("Data/English/Art/Textures/defeated.dds");
    if (!g_textures[0]) {
        printf("ERROR: Failed to load defeated.dds\n");
        return false;
    }
    printf("✅ defeated.dds loaded: %p\n", g_textures[0]);
    
    // Load GameOver.tga (RGBA8)
    printf("\n2. Loading GameOver.tga (RGBA8)...\n");
    g_textures[1] = g_textureCache.GetTexture("Data/English/Art/Textures/GameOver.tga");
    if (!g_textures[1]) {
        printf("ERROR: Failed to load GameOver.tga\n");
        return false;
    }
    printf("✅ GameOver.tga loaded: %p\n", g_textures[1]);
    
    // Load caust00.tga (RGBA8, small)
    printf("\n3. Loading caust00.tga (RGBA8)...\n");
    g_textures[2] = g_textureCache.GetTexture("Data/English/Art/Textures/caust00.tga");
    if (!g_textures[2]) {
        printf("ERROR: Failed to load caust00.tga\n");
        return false;
    }
    printf("✅ caust00.tga loaded: %p\n", g_textures[2]);
    
    // Check cache stats
    int cached, total;
    g_textureCache.GetCacheStats(cached, total);
    printf("\n📊 TextureCache stats: %d cached, %d total\n", cached, total);
    
    printf("\n✅ TEST 1 PASSED: All textures loaded successfully\n");
    return true;
}

bool Test2_CreateQuads() {
    printf("\n=== TEST 2: CREATING TEXTURED QUADS ===\n");
    
    // Quad 0: Full-size defeated.dds at top-left
    printf("\n1. Creating Quad 0: defeated.dds (50, 50, 1024x256)...\n");
    g_quads[0] = NEW GX::TexturedQuad(50, 50, 1024, 256);
    g_quads[0]->SetTexture(g_textures[0]);
    g_quads[0]->SetAlpha(1.0f);  // Fully opaque
    printf("✅ Quad 0 created\n");
    
    // Quad 1: Half-size GameOver.tga at center with alpha
    printf("\n2. Creating Quad 1: GameOver.tga (200, 350, 512x128) with 70%% alpha...\n");
    g_quads[1] = NEW GX::TexturedQuad(200, 350, 512, 128);
    g_quads[1]->SetTexture(g_textures[1]);
    g_quads[1]->SetAlpha(0.7f);  // 70% opacity
    printf("✅ Quad 1 created\n");
    
    // Quad 2: Small caust00.tga at bottom-right with blue tint
    printf("\n3. Creating Quad 2: caust00.tga (1000, 600, 128x128) with blue tint...\n");
    g_quads[2] = NEW GX::TexturedQuad(1000, 600, 128, 128);
    g_quads[2]->SetTexture(g_textures[2]);
    g_quads[2]->SetColorTint(0.5f, 0.8f, 1.0f);  // Blue tint
    printf("✅ Quad 2 created\n");
    
    // Quad 3: defeated.dds with custom UVs (top half only)
    printf("\n4. Creating Quad 3: defeated.dds (50, 500, 512x128) with custom UVs...\n");
    g_quads[3] = NEW GX::TexturedQuad(50, 500, 512, 128);
    g_quads[3]->SetTexture(g_textures[0]);
    g_quads[3]->SetCustomUVs(0.0f, 0.0f, 1.0f, 0.5f);  // Top half only
    g_quads[3]->SetColorTint(1.0f, 0.6f, 0.3f);  // Orange tint
    printf("✅ Quad 3 created\n");
    
    printf("\n✅ TEST 2 PASSED: All quads created successfully\n");
    return true;
}

bool Test3_RenderLoop() {
    printf("\n=== TEST 3: RENDER LOOP ===\n");
    
    // Render for 5 seconds
    const float duration = 5.0f;
    Uint32 start_time = SDL_GetTicks();
    int frame_count = 0;
    
    printf("\nRendering for %.1f seconds...\n", duration);
    printf("(Window should display 4 textured quads)\n\n");
    
    while (true) {
        // Check time
        Uint32 current_time = SDL_GetTicks();
        float elapsed = (current_time - start_time) / 1000.0f;
        
        if (elapsed >= duration) {
            break;
        }
        
        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("\nUser closed window\n");
                return true;
            }
        }
        
        // === BEGIN METAL RENDER PASS (CRITICAL FIX) ===
        MetalWrapper::BeginFrame();
        
        // Clear to cornflower blue (Generals menu color)
        MetalWrapper::Clear(0.39f, 0.58f, 0.93f, 1.0f);
        
        // Render all quads
        for (int i = 0; i < 4; i++) {
            if (g_quads[i]) {
                g_quads[i]->Render();
            }
        }
        
        // === END METAL RENDER PASS (CRITICAL FIX) ===
        MetalWrapper::EndFrame();
        
        // Present to screen
        SDL_Delay(16);  // ~60 FPS
        frame_count++;
    }
    
    float final_elapsed = (SDL_GetTicks() - start_time) / 1000.0f;
    float fps = frame_count / final_elapsed;
    
    printf("\n📊 Render stats:\n");
    printf("   - Total frames: %d\n", frame_count);
    printf("   - Duration: %.2f seconds\n", final_elapsed);
    printf("   - Average FPS: %.1f\n", fps);
    
    printf("\n✅ TEST 3 PASSED: Render loop completed\n");
    return true;
}

void Cleanup() {
    printf("\n=== CLEANUP ===\n");
    
    // Delete quads
    for (int i = 0; i < 4; i++) {
        if (g_quads[i]) {
            delete g_quads[i];
            g_quads[i] = nullptr;
        }
    }
    printf("✅ Quads deleted\n");
    
    // Release textures
    for (int i = 0; i < 3; i++) {
        if (g_textures[i]) {
            g_textureCache.ReleaseTexture(g_textures[i]);
            g_textures[i] = nullptr;
        }
    }
    printf("✅ Textures released\n");
    
    // Shutdown Metal
    MetalWrapper::Shutdown();
    printf("✅ Metal shutdown\n");
    
    // Destroy SDL window
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    printf("✅ SDL window destroyed\n");
    
    SDL_Quit();
    printf("✅ SDL quit\n");
}

int main(int argc, char* argv[]) {
    printf("\n");
    printf("═════════════════════════════════════════════════════════════\n");
    printf("   Phase 28.4.2-28.4.3: Textured Quad Render Test (FIXED)\n");
    printf("═════════════════════════════════════════════════════════════\n");
    
    // Change to game directory
    const char* game_dir = getenv("HOME");
    if (game_dir) {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/GeneralsX/GeneralsMD", game_dir);
        if (chdir(full_path) == 0) {
            printf("✅ Changed to game directory: %s\n", full_path);
        } else {
            printf("WARNING: Could not change to game directory: %s\n", full_path);
        }
    }
    
    // Run tests
    bool success = true;
    
    if (!InitializeGraphics()) {
        printf("\n❌ Graphics initialization failed\n");
        success = false;
        goto cleanup;
    }
    
    if (!Test1_LoadTextures()) {
        printf("\n❌ TEST 1 FAILED\n");
        success = false;
        goto cleanup;
    }
    
    if (!Test2_CreateQuads()) {
        printf("\n❌ TEST 2 FAILED\n");
        success = false;
        goto cleanup;
    }
    
    if (!Test3_RenderLoop()) {
        printf("\n❌ TEST 3 FAILED\n");
        success = false;
        goto cleanup;
    }
    
cleanup:
    Cleanup();
    
    printf("\n");
    printf("═════════════════════════════════════════════════════════════\n");
    if (success) {
        printf("   ✅ ALL TESTS PASSED\n");
        printf("\n");
        printf("   Visual Validation Checklist:\n");
        printf("   [ ] Quad 0: defeated.dds at top-left (1024x256)\n");
        printf("   [ ] Quad 1: GameOver.tga at center (512x128, 70%% alpha)\n");
        printf("   [ ] Quad 2: caust00.tga at bottom-right (128x128, blue)\n");
        printf("   [ ] Quad 3: defeated.dds top-half at bottom-left (512x128, orange)\n");
        printf("   [ ] No texture distortion or UV mapping issues\n");
        printf("   [ ] Alpha blending working correctly\n");
        printf("   [ ] Color tinting working correctly\n");
        printf("   [ ] No visual artifacts\n");
    } else {
        printf("   ❌ TESTS FAILED\n");
    }
    printf("═════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    return success ? 0 : 1;
}
