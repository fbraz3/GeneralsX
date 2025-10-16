/**
 * @file test_bc3_sampling_debug.cpp
 * @brief Phase 28.4.2 BC3 Bug Investigation: Debug texture sampling
 * 
 * This test creates visual debug outputs to isolate the BC3 texture bug:
 * 1. UV coordinate visualization (shows what coordinates are being sampled)
 * 2. Raw texture sampling (bypasses vertex colors)
 * 3. Channel isolation (R, G, B, A separately)
 * 4. Coordinate-based coloring (verify UV mapping is correct)
 */

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

// Game engine includes
#include "metalwrapper.h"
#include "texturecache.h"

// Test configuration
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;
const char* WINDOW_TITLE = "Phase 28.4.2 - BC3 Sampling Debug";

// Asset paths
const char* TEXTURE_DEFEATED = "Data/English/Art/Textures/defeated.dds";
const char* TEXTURE_GAMEOVER = "Data/English/Art/Textures/GameOver.tga";

/**
 * @brief Debug shader source (embedded Metal code)
 * 
 * This shader provides 5 debug modes:
 * - Mode 0: Normal sampling (current behavior)
 * - Mode 1: UV coordinates as colors (R=U, G=V, B=0)
 * - Mode 2: Raw texture sampling (no vertex color multiply)
 * - Mode 3: Red channel only
 * - Mode 4: Checkerboard pattern (verify UV mapping)
 */
const char* DEBUG_SHADER_SOURCE = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexInput {
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float4 color    [[attribute(2)]];
    float2 texcoord [[attribute(3)]];
};

struct VertexOutput {
    float4 position [[position]];
    float4 color;
    float2 texcoord;
};

struct Uniforms {
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    int debugMode;  // New: debug visualization mode
};

vertex VertexOutput vertex_debug(VertexInput in [[stage_in]],
                                 constant Uniforms& uniforms [[buffer(0)]]) {
    VertexOutput out;
    
    float4 worldPos = uniforms.worldMatrix * float4(in.position, 1.0);
    float4 viewPos = uniforms.viewMatrix * worldPos;
    out.position = uniforms.projectionMatrix * viewPos;
    
    out.color = in.color;
    out.texcoord = in.texcoord;
    
    return out;
}

fragment float4 fragment_debug(VertexOutput in [[stage_in]],
                              constant Uniforms& uniforms [[buffer(0)]],
                              texture2d<float> diffuseTexture [[texture(0)]],
                              sampler textureSampler [[sampler(0)]]) {
    
    int mode = uniforms.debugMode;
    
    // Mode 0: Normal sampling (current behavior)
    if (mode == 0) {
        float4 texColor = diffuseTexture.sample(textureSampler, in.texcoord);
        return texColor * in.color;
    }
    
    // Mode 1: UV coordinates as colors
    if (mode == 1) {
        return float4(in.texcoord.x, in.texcoord.y, 0.0, 1.0);
    }
    
    // Mode 2: Raw texture sampling (ignore vertex color)
    if (mode == 2) {
        return diffuseTexture.sample(textureSampler, in.texcoord);
    }
    
    // Mode 3: Red channel only (check for data corruption)
    if (mode == 3) {
        float4 texColor = diffuseTexture.sample(textureSampler, in.texcoord);
        return float4(texColor.r, 0.0, 0.0, 1.0);
    }
    
    // Mode 4: Checkerboard pattern (verify UV mapping)
    if (mode == 4) {
        int x = int(in.texcoord.x * 8.0);
        int y = int(in.texcoord.y * 8.0);
        float checker = float((x + y) % 2);
        return float4(checker, checker, checker, 1.0);
    }
    
    // Mode 5: Sample at fixed UV (0.25, 0.5) - left side of texture
    if (mode == 5) {
        float4 texColor = diffuseTexture.sample(textureSampler, float2(0.25, 0.5));
        return texColor;
    }
    
    // Mode 6: Sample at fixed UV (0.75, 0.5) - right side (buggy area)
    if (mode == 6) {
        float4 texColor = diffuseTexture.sample(textureSampler, float2(0.75, 0.5));
        return texColor;
    }
    
    // Default: Magenta error color
    return float4(1.0, 0.0, 1.0, 1.0);
}
)";

/**
 * @brief Initialize SDL and Metal with debug shader
 */
bool InitializeGraphics(SDL_Window** window) {
    printf("Initializing graphics with debug shader...\n");
    
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
    
    // TODO: Load debug shader (requires MetalWrapper shader loading API)
    // For now, we'll use the default shader and test with different parameters
    
    return true;
}

/**
 * @brief Render a simple quad with texture
 */
void RenderQuad(void* texture, float x, float y, float width, float height) {
    // Setup orthographic projection (2D)
    float projMatrix[16] = {
        2.0f / WINDOW_WIDTH, 0, 0, 0,
        0, -2.0f / WINDOW_HEIGHT, 0, 0,
        0, 0, 1, 0,
        -1, 1, 0, 1
    };
    
    float worldMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    };
    
    float viewMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    GX::MetalWrapper::SetTransform(0, worldMatrix);  // World
    GX::MetalWrapper::SetTransform(1, viewMatrix);   // View
    GX::MetalWrapper::SetTransform(2, projMatrix);   // Projection
    
    // Bind texture
    GX::MetalWrapper::BindTexture(texture, 0);
    
    // Create quad vertices (position, normal, color, UV)
    float vertices[] = {
        // X, Y, Z, NX, NY, NZ, R, G, B, A, U, V
        0, 0, 0,       0, 0, 1,  1, 1, 1, 1,  0, 0,  // Top-left
        width, 0, 0,   0, 0, 1,  1, 1, 1, 1,  1, 0,  // Top-right
        0, height, 0,  0, 0, 1,  1, 1, 1, 1,  0, 1,  // Bottom-left
        width, height, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1   // Bottom-right
    };
    
    unsigned short indices[] = { 0, 1, 2, 2, 1, 3 };
    
    // TODO: Use MetalWrapper draw calls
    // For now, this is a placeholder - actual rendering happens in MetalWrapper
    
    GX::MetalWrapper::UnbindTexture(0);
}

/**
 * @brief Test different debug modes
 */
void RunDebugTests(SDL_Window* window, void* texture_bc3, void* texture_tga) {
    printf("\n=== BC3 Sampling Debug Tests ===\n");
    printf("Each mode will render for 2 seconds\n");
    printf("Press ESC to skip to next mode\n\n");
    
    struct DebugMode {
        int id;
        const char* name;
        const char* description;
    };
    
    DebugMode modes[] = {
        { 0, "Normal Sampling", "Current behavior (shows bug)" },
        { 1, "UV Visualization", "Red=U, Green=V (should show gradient)" },
        { 2, "Raw Texture", "No vertex color multiply" },
        { 3, "Red Channel Only", "Isolate red channel data" },
        { 4, "Checkerboard", "Verify UV mapping is correct" },
        { 5, "Fixed UV 0.25", "Sample left side (should be OK)" },
        { 6, "Fixed UV 0.75", "Sample right side (buggy area)" }
    };
    
    const int mode_count = sizeof(modes) / sizeof(modes[0]);
    
    for (int i = 0; i < mode_count; i++) {
        printf("\n--- Mode %d: %s ---\n", modes[i].id, modes[i].name);
        printf("    %s\n", modes[i].description);
        printf("    Rendering for 2 seconds...\n");
        
        bool skip = false;
        SDL_Event event;
        Uint32 start_time = SDL_GetTicks();
        
        while (!skip && (SDL_GetTicks() - start_time < 2000)) {
            // Handle events
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    return;
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        skip = true;
                    }
                }
            }
            
            // Begin frame
            GX::MetalWrapper::BeginFrame(0.1f, 0.1f, 0.2f, 1.0f);
            
            // TODO: Set debug mode in shader uniform
            // This requires adding MetalWrapper::SetShaderInt() or similar
            
            // Render BC3 texture (defeated.dds) on left
            RenderQuad(texture_bc3, 50, 50, 500, 300);
            
            // Render TGA texture (GameOver.tga) on right for comparison
            RenderQuad(texture_tga, 650, 50, 500, 300);
            
            // End frame
            GX::MetalWrapper::EndFrame();
        }
        
        if (skip) {
            printf("    (Skipped)\n");
        }
    }
    
    printf("\n✓ Debug tests complete\n");
}

/**
 * @brief Main test function
 */
int main(int argc, char* argv[]) {
    printf("=================================================\n");
    printf("Phase 28.4.2: BC3 Texture Sampling Debug\n");
    printf("=================================================\n\n");
    
    // Change to game data directory
    const char* game_dir = getenv("HOME");
    if (game_dir) {
        char path[512];
        snprintf(path, sizeof(path), "%s/GeneralsX/GeneralsMD", game_dir);
        if (chdir(path) != 0) {
            printf("WARNING: Could not change to game directory: %s\n", path);
        } else {
            printf("Working directory: %s\n", path);
        }
    }
    
    SDL_Window* window = nullptr;
    
    // Initialize graphics
    if (!InitializeGraphics(&window)) {
        printf("\n✗ FATAL: Graphics initialization failed\n");
        return 1;
    }
    
    // Load test textures
    printf("\nLoading test textures...\n");
    void* texture_bc3 = TextureCache::GetInstance()->LoadTexture(TEXTURE_DEFEATED);
    if (!texture_bc3) {
        printf("✗ FATAL: Could not load %s\n", TEXTURE_DEFEATED);
        return 1;
    }
    printf("✓ Loaded defeated.dds (BC3)\n");
    
    void* texture_tga = TextureCache::GetInstance()->LoadTexture(TEXTURE_GAMEOVER);
    if (!texture_tga) {
        printf("✗ FATAL: Could not load %s\n", TEXTURE_GAMEOVER);
        return 1;
    }
    printf("✓ Loaded GameOver.tga (RGBA8)\n");
    
    // Run debug tests
    RunDebugTests(window, texture_bc3, texture_tga);
    
    // Cleanup
    printf("\n=== Cleanup ===\n");
    TextureCache::GetInstance()->ClearCache();
    GX::MetalWrapper::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("✓ Cleanup complete\n");
    
    printf("\n=================================================\n");
    printf("✓ Debug test complete\n");
    printf("=================================================\n");
    
    return 0;
}
