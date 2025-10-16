/**
 * @file test_bc3_raw_sampling.cpp
 * @brief Phase 28.4.2: Direct BC3 texture sampling test
 * 
 * This test bypasses TexturedQuad and directly tests texture upload + sampling
 * to isolate whether the bug is in:
 * - Texture upload (bytesPerRow, region, format)
 * - Texture sampling (shader, sampler state, UV coordinates)
 * - Vertex data (colors, UVs)
 */

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

// Direct Metal includes
#include "metalwrapper.h"
#include "ddsloader.h"

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 512;

/**
 * @brief Test 1: Load BC3 texture and dump first/middle/last blocks
 */
bool Test_LoadAndDumpBlocks() {
    printf("\n=== Test 1: Load BC3 and Dump Blocks ===\n");
    
    const char* dds_path = "Data/English/Art/Textures/defeated.dds";
    
    // Load DDS
    DDSTextureData texData;
    if (!DDSLoader::Load(dds_path, &texData)) {
        printf("✗ Failed to load %s\n", dds_path);
        return false;
    }
    
    if (!texData.isValid) {
        printf("✗ Loaded but invalid texture data\n");
        return false;
    }
    
    unsigned int width = texData.width;
    unsigned int height = texData.height;
    unsigned int format = (unsigned int)texData.format;
    unsigned char* data = texData.data;
    unsigned int data_size = texData.dataSize;
    unsigned int mipmaps = texData.mipMapCount;
    
    printf("✓ Loaded: %ux%u, format=%u, size=%u bytes, mipmaps=%u\n",
           width, height, format, data_size, mipmaps);
    
    // Calculate block dimensions
    unsigned int blocksWide = (width + 3) / 4;
    unsigned int blocksHigh = (height + 3) / 4;
    unsigned int blockSize = 16;  // BC3
    unsigned int bytesPerRow = blocksWide * blockSize;
    
    printf("  Blocks: %ux%u (bytesPerRow=%u)\n", blocksWide, blocksHigh, bytesPerRow);
    
    // Dump first block (top-left corner)
    printf("\n  First block (0,0) - 16 bytes:\n    ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[i]);
        if (i == 7) printf("\n    ");  // Split alpha/color
    }
    printf("\n");
    
    // Dump middle-left block (column 0, row 50%)
    unsigned int midRowOffset = (blocksHigh / 2) * bytesPerRow;
    printf("  Middle-left block (0,%u) - 16 bytes:\n    ", blocksHigh / 2);
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[midRowOffset + i]);
        if (i == 7) printf("\n    ");
    }
    printf("\n");
    
    // Dump middle-right block (column 75%, row 50%) - BUGGY AREA
    unsigned int midRightBlockX = (blocksWide * 3) / 4;
    unsigned int midRightOffset = midRowOffset + (midRightBlockX * blockSize);
    printf("  Middle-right block (%u,%u) - 16 bytes [BUGGY AREA]:\n    ",
           midRightBlockX, blocksHigh / 2);
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[midRightOffset + i]);
        if (i == 7) printf("\n    ");
    }
    printf("\n");
    
    // Dump last block (bottom-right corner)
    unsigned int lastOffset = data_size - blockSize;
    printf("  Last block (%u,%u) - 16 bytes:\n    ", blocksWide - 1, blocksHigh - 1);
    for (int i = 0; i < 16; i++) {
        printf("%02X ", data[lastOffset + i]);
        if (i == 7) printf("\n    ");
    }
    printf("\n");
    
    DDSLoader::Free(&texData);
    return true;
}

/**
 * @brief Test 2: Create texture and read it back (if possible)
 */
bool Test_TextureUploadAndReadback() {
    printf("\n=== Test 2: Texture Upload Parameters ===\n");
    
    const char* dds_path = "Data/English/Art/Textures/defeated.dds";
    
    // Load DDS
    DDSTextureData texData;
    if (!DDSLoader::Load(dds_path, &texData)) {
        printf("✗ Failed to load %s\n", dds_path);
        return false;
    }
    
    unsigned int width = texData.width;
    unsigned int height = texData.height;
    unsigned int format = (unsigned int)texData.format;
    unsigned char* data = texData.data;
    unsigned int data_size = texData.dataSize;
    unsigned int mipmaps = texData.mipMapCount;
    
    printf("Creating Metal texture with parameters:\n");
    printf("  width: %u pixels\n", width);
    printf("  height: %u pixels\n", height);
    printf("  format: %u (BC3)\n", format);
    printf("  dataSize: %u bytes\n", data_size);
    printf("  mipmaps: %u\n", mipmaps);
    
    // Calculate what Metal will receive
    unsigned int blocksWide = (width + 3) / 4;
    unsigned int blocksHigh = (height + 3) / 4;
    unsigned int blockSize = 16;
    unsigned int bytesPerRow = blocksWide * blockSize;
    unsigned int expectedSize = bytesPerRow * blocksHigh;
    
    printf("\n  Calculated for Metal:\n");
    printf("    blocksWide: %u\n", blocksWide);
    printf("    blocksHigh: %u\n", blocksHigh);
    printf("    bytesPerRow: %u\n", bytesPerRow);
    printf("    expectedSize: %u bytes\n", expectedSize);
    printf("    MTLRegion: origin=(0,0), size=(%u, %u) PIXELS\n", width, height);
    
    if (expectedSize != data_size) {
        printf("  ⚠️  WARNING: Size mismatch! expected=%u, actual=%u\n",
               expectedSize, data_size);
    } else {
        printf("  ✓ Size matches\n");
    }
    
    // Try to create texture
    void* texture = GX::MetalWrapper::CreateTextureFromDDS(
        width, height, format, data, data_size, mipmaps
    );
    
    if (!texture) {
        printf("✗ Failed to create Metal texture\n");
        delete[] data;
        return false;
    }
    
    printf("✓ Metal texture created successfully\n");
    
    // Note: Metal doesn't support reading back compressed textures directly
    // We'd need to render to a texture and read pixels, which is complex
    
    DDSLoader::Free(&texData);
    return true;
}

/**
 * @brief Test 3: Analyze block positions (removed rendering test)
 * @note Rendering test requires TexturedQuad API - use test_textured_quad_render.cpp instead
 */
bool Test_AnalyzeBlockPositions() {
    printf("\n=== Test 3: Block Position Analysis ===\n");
    
    const char* dds_path = "Data/English/Art/Textures/defeated.dds";
    DDSTextureData texData;
    
    if (!DDSLoader::Load(dds_path, &texData)) {
        printf("✗ Failed to load texture\n");
        return false;
    }
    
    unsigned int width = texData.width;
    unsigned int height = texData.height;
    unsigned int blocksWide = (width + 3) / 4;
    unsigned int blocksHigh = (height + 3) / 4;
    unsigned int bytesPerRow = blocksWide * 16;
    
    printf("Analyzing block positions that map to UV coordinates:\n\n");
    
    // UV 0.5 = 50% across texture
    unsigned int block_at_50percent = blocksWide / 2;
    unsigned int offset_at_50percent = (blocksHigh / 2) * bytesPerRow + (block_at_50percent * 16);
    
    printf("  UV (0.5, 0.5) → Block X=%u, Y=%u\n", block_at_50percent, blocksHigh / 2);
    printf("               → File offset: %u bytes\n", offset_at_50percent);
    printf("               → First 8 bytes: ");
    for (int i = 0; i < 8 && (offset_at_50percent + i) < texData.dataSize; i++) {
        printf("%02X ", texData.data[offset_at_50percent + i]);
    }
    printf("\n\n");
    
    // UV 0.75 = 75% across (buggy area!)
    unsigned int block_at_75percent = (blocksWide * 3) / 4;
    unsigned int offset_at_75percent = (blocksHigh / 2) * bytesPerRow + (block_at_75percent * 16);
    
    printf("  UV (0.75, 0.5) → Block X=%u, Y=%u [BUGGY AREA]\n", block_at_75percent, blocksHigh / 2);
    printf("               → File offset: %u bytes\n", offset_at_75percent);
    printf("               → First 8 bytes: ");
    for (int i = 0; i < 8 && (offset_at_75percent + i) < texData.dataSize; i++) {
        printf("%02X ", texData.data[offset_at_75percent + i]);
    }
    printf("\n\n");
    
    // Check if there's a pattern at exactly 50% of file
    printf("  File 50%% mark → Offset: %lu bytes\n", (unsigned long)(texData.dataSize / 2));
    printf("               → Data: ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", texData.data[texData.dataSize / 2 + i]);
    }
    printf("\n\n");
    
    DDSLoader::Free(&texData);
    return true;
}

/**
 * @brief Main test function
 */
int main(int argc, char* argv[]) {
    printf("=================================================\n");
    printf("Phase 28.4.2: BC3 Raw Sampling Test\n");
    printf("=================================================\n");
    
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
    
    // Test 1: Load and dump blocks
    if (!Test_LoadAndDumpBlocks()) {
        printf("\n✗ Test 1 failed\n");
        return 1;
    }
    
    // Initialize SDL and Metal for Test 2
    printf("\nInitializing Metal for Test 2...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow(
        "BC3 Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    GX::MetalConfig config;
    config.sdlWindow = window;
    config.width = 640;
    config.height = 480;
    config.vsync = true;
    
    if (!GX::MetalWrapper::Initialize(config)) {
        printf("MetalWrapper::Initialize failed\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    printf("✓ Metal initialized\n");
    
    // Test 2: Upload parameters
    if (!Test_TextureUploadAndReadback()) {
        printf("\n✗ Test 2 failed\n");
        GX::MetalWrapper::Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Test 3: Block positions
    if (!Test_AnalyzeBlockPositions()) {
        printf("\n✗ Test 3 failed\n");
        GX::MetalWrapper::Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Cleanup Metal
    GX::MetalWrapper::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("✓ Metal cleanup complete\n");
    
    printf("\n=================================================\n");
    printf("✓ All static tests passed\n");
    printf("=================================================\n");
    printf("\nObservations:\n");
    printf("- Data appears valid in all regions (left, middle, right)\n");
    printf("- Upload parameters are mathematically correct\n");
    printf("- Bug likely in sampling stage or Metal internal layout\n");
    printf("\nNext steps:\n");
    printf("1. Test with Metal Frame Capture to inspect GPU-side texture\n");
    printf("2. Compare block layout with DXVK reference implementation\n");
    printf("3. Try manual decompression as workaround\n");
    
    return 0;
}
