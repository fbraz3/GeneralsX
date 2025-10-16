// Phase 28.4.2-28.4.3: Textured Quad Render Test (API Corrected)
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
    GX::MetalConfig config;
    config.width = 1280;
    config.height = 768;
    
    if (!GX::MetalWrapper::Initialize(config)) {
        printf("ERROR: MetalWrapper::Initialize failed\n");
        return false;
    }
    printf("✅ Metal backend initialized\n");
    
    // Initialize shader uniforms (identity matrices, effects disabled)
    GX::MetalWrapper::SetDefaultUniforms();
    printf("✅ Shader uniforms initialized\n");
    
    // Set viewport size for TexturedQuad screen-to-clip conversion
    GX::TexturedQuad::SetViewportSize(1280.0f, 768.0f);
    
    return true;
}

bool Test1_LoadTextures() {
    printf("\n=== TEST 1: LOADING TEXTURES ===\n");
    
    // Get texture cache singleton
    TextureCache* cache = TextureCache::GetInstance();
    
    // Load defeated.dds (BC3 compressed)
    printf("\n1. Loading defeated.dds (BC3)...\n");
    void* tex1 = cache->LoadTexture("Data/English/Art/Textures/defeated.dds");
    if (!tex1) {
        printf("ERROR: Failed to load defeated.dds\n");
        return false;
    }
    printf("✅ defeated.dds loaded: %p\n", tex1);
    
    // Load GameOver.tga (RGBA8)
    printf("\n2. Loading GameOver.tga (RGBA8)...\n");
    void* tex2 = cache->LoadTexture("Data/English/Art/Textures/GameOver.tga");
    if (!tex2) {
        printf("ERROR: Failed to load GameOver.tga\n");
        return false;
    }
    printf("✅ GameOver.tga loaded: %p\n", tex2);
    
    // Load victorious.dds (BC3, small)
    printf("\n3. Loading victorious.dds (BC3)...\n");
    void* tex3 = cache->LoadTexture("Data/English/Art/Textures/victorious.dds");
    if (!tex3) {
        printf("ERROR: Failed to load victorious.dds\n");
        return false;
    }
    printf("✅ victorious.dds loaded: %p\n", tex3);
    
    // Check cache stats
    int cached;
    unsigned long total;
    cache->GetCacheStats(cached, total);
    printf("\n📊 TextureCache stats: %d cached, %lu bytes\n", cached, total);
    
    printf("\n✅ TEST 1 PASSED: All textures loaded successfully\n");
    return true;
}

bool Test2_CreateQuads() {
    printf("\n=== TEST 2: CREATING TEXTURED QUADS ===\n");
    
    // Quad 0: Full-size defeated.dds at top-left
    printf("\n1. Creating Quad 0: defeated.dds (50, 50, 1024x256)...\n");
    g_quads[0] = new GX::TexturedQuad();
    g_quads[0]->SetTexture("Data/English/Art/Textures/defeated.dds");
    g_quads[0]->SetPosition(50, 50, 1024, 256);
    g_quads[0]->SetColor(1.0f, 1.0f, 1.0f, 1.0f);  // Fully opaque
    printf("✅ Quad 0 created\n");
    
    // Quad 1: Half-size GameOver.tga at center with alpha
    printf("\n2. Creating Quad 1: GameOver.tga (200, 350, 512x128) with 70%% alpha...\n");
    g_quads[1] = new GX::TexturedQuad();
    g_quads[1]->SetTexture("Data/English/Art/Textures/GameOver.tga");
    g_quads[1]->SetPosition(200, 350, 512, 128);
    g_quads[1]->SetColor(1.0f, 1.0f, 1.0f, 0.7f);  // 70% opacity
    printf("✅ Quad 1 created\n");
    
    // Quad 2: victorious.dds at bottom-right with green tint
    printf("\n3. Creating Quad 2: victorious.dds (800, 500, 256x128) with green tint...\n");
    g_quads[2] = new GX::TexturedQuad();
    g_quads[2]->SetTexture("Data/English/Art/Textures/victorious.dds");
    g_quads[2]->SetPosition(800, 500, 256, 128);
    g_quads[2]->SetColor(0.5f, 1.0f, 0.5f, 1.0f);  // Green tint
    printf("✅ Quad 2 created\n");
    
    // Quad 3: defeated.dds with custom UVs (top half only)
    printf("\n4. Creating Quad 3: defeated.dds (50, 500, 512x128) with custom UVs...\n");
    g_quads[3] = new GX::TexturedQuad();
    g_quads[3]->SetTexture("Data/English/Art/Textures/defeated.dds");
    g_quads[3]->SetPosition(50, 500, 512, 128);
    g_quads[3]->SetUVs(0.0f, 0.0f, 1.0f, 0.5f);  // Top half only
    g_quads[3]->SetColor(1.0f, 0.6f, 0.3f, 1.0f);  // Orange tint
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
        // BeginFrame clears to cornflower blue (Generals menu color)
        GX::MetalWrapper::BeginFrame(0.39f, 0.58f, 0.93f, 1.0f);
        
        // Render all quads
        for (int i = 0; i < 4; i++) {
            if (g_quads[i]) {
                g_quads[i]->Render();
            }
        }
        
        // === END METAL RENDER PASS (CRITICAL FIX) ===
        GX::MetalWrapper::EndFrame();
        
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
    
    // Clear texture cache
    TextureCache* cache = TextureCache::GetInstance();
    cache->ClearCache();
    printf("✅ Texture cache cleared\n");
    
    // Shutdown Metal
    GX::MetalWrapper::Shutdown();
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
    printf("   Phase 28.4.2-28.4.3: Textured Quad Render Test\n");
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
        printf("   [ ] Quad 2: victorious.dds at bottom-right (256x128, green)\n");
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
