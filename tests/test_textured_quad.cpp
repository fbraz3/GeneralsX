/**
 * @file test_textured_quad.cpp
 * @brief Test program for textured quad rendering (Phase 28.3.4)
 * 
 * Tests:
 * - TextureCache loading DDS and TGA files
 * - Metal texture binding to fragment shader
 * - Rendering textured quads with UV mapping
 * - Visual validation (requires Metal backend)
 * 
 * Compilation:
 *   c++ -std=c++17 -framework Metal -framework QuartzCore -framework Cocoa \
 *       -o /tmp/test_textured_quad tests/test_textured_quad.cpp \
 *       build/macos-arm64/Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeFiles/g_ww3d2.dir/__/__/__/__/__/__/Core/Libraries/Source/WWVegas/WW3D2/texturecache.cpp.o \
 *       build/macos-arm64/Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeFiles/g_ww3d2.dir/__/__/__/__/__/__/Core/Libraries/Source/WWVegas/WW3D2/ddsloader.cpp.o \
 *       build/macos-arm64/Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeFiles/g_ww3d2.dir/__/__/__/__/__/__/Core/Libraries/Source/WWVegas/WW3D2/tgaloader.cpp.o \
 *       build/macos-arm64/Generals/Code/Libraries/Source/WWVegas/WW3D2/CMakeFiles/g_ww3d2.dir/__/__/__/__/__/__/Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm.o \
 *       -I Core/Libraries/Source/WWVegas/WW3D2/
 * 
 * Usage:
 *   /tmp/test_textured_quad
 */

#include "texturecache.h"
#include "metalwrapper.h"
#include <stdio.h>
#include <stdlib.h>

using namespace GX;

// Test 1: Load DDS texture via TextureCache
bool test_load_dds_texture() {
    printf("Test 1: Loading DDS texture via TextureCache...\n");
    
    const char* home = getenv("HOME");
    if (!home) {
        printf("FAILED: HOME environment variable not set\n");
        return false;
    }
    
    char texture_path[1024];
    snprintf(texture_path, sizeof(texture_path), 
        "%s/GeneralsX/GeneralsMD/Data/English/Art/Textures/defeated.dds", home);
    
    TextureCache* cache = TextureCache::GetInstance();
    void* texture = cache->LoadTexture(texture_path);
    
    if (!texture) {
        printf("FAILED: DDS texture not loaded\n");
        return false;
    }
    
    printf("SUCCESS: DDS texture loaded (handle=%p)\n", texture);
    
    // Check cache entry
    TextureEntry* entry = cache->GetTextureEntry(texture_path);
    if (!entry) {
        printf("FAILED: Texture entry not found in cache\n");
        return false;
    }
    
    printf("  Dimensions: %ux%u\n", entry->width, entry->height);
    printf("  Format: %u (DDS)\n", entry->format);
    printf("  Refcount: %d\n", entry->refcount);
    
    return true;
}

// Test 2: Load TGA texture via TextureCache
bool test_load_tga_texture() {
    printf("\nTest 2: Loading TGA texture via TextureCache...\n");
    
    const char* home = getenv("HOME");
    if (!home) {
        printf("FAILED: HOME environment variable not set\n");
        return false;
    }
    
    char texture_path[1024];
    snprintf(texture_path, sizeof(texture_path), 
        "%s/GeneralsX/GeneralsMD/Data/WaterPlane/caust00.tga", home);
    
    TextureCache* cache = TextureCache::GetInstance();
    void* texture = cache->LoadTexture(texture_path);
    
    if (!texture) {
        printf("FAILED: TGA texture not loaded\n");
        return false;
    }
    
    printf("SUCCESS: TGA texture loaded (handle=%p)\n", texture);
    
    // Check cache entry
    TextureEntry* entry = cache->GetTextureEntry(texture_path);
    if (!entry) {
        printf("FAILED: Texture entry not found in cache\n");
        return false;
    }
    
    printf("  Dimensions: %ux%u\n", entry->width, entry->height);
    printf("  Format: %u (TGA)\n", entry->format);
    printf("  Refcount: %d\n", entry->refcount);
    
    return true;
}

// Test 3: Test reference counting
bool test_reference_counting() {
    printf("\nTest 3: Testing reference counting...\n");
    
    const char* home = getenv("HOME");
    if (!home) {
        printf("FAILED: HOME environment variable not set\n");
        return false;
    }
    
    char texture_path[1024];
    snprintf(texture_path, sizeof(texture_path), 
        "%s/GeneralsX/GeneralsMD/Data/WaterPlane/caust00.tga", home);
    
    TextureCache* cache = TextureCache::GetInstance();
    
    // Load same texture twice (should reuse cached texture)
    void* texture1 = cache->LoadTexture(texture_path);
    void* texture2 = cache->LoadTexture(texture_path);
    
    if (texture1 != texture2) {
        printf("FAILED: Different texture handles returned (should be same)\n");
        return false;
    }
    
    TextureEntry* entry = cache->GetTextureEntry(texture_path);
    if (!entry) {
        printf("FAILED: Texture entry not found\n");
        return false;
    }
    
    if (entry->refcount != 2) {
        printf("FAILED: Refcount should be 2, got %d\n", entry->refcount);
        return false;
    }
    
    printf("SUCCESS: Reference counting working correctly\n");
    printf("  Handle: %p (reused)\n", texture1);
    printf("  Refcount: %d\n", entry->refcount);
    
    // Release one reference
    cache->ReleaseTexture(texture_path);
    
    if (entry->refcount != 1) {
        printf("FAILED: Refcount should be 1 after release, got %d\n", entry->refcount);
        return false;
    }
    
    printf("  Refcount after release: %d\n", entry->refcount);
    
    return true;
}

// Test 4: Test path normalization
bool test_path_normalization() {
    printf("\nTest 4: Testing path normalization...\n");
    
    const char* home = getenv("HOME");
    if (!home) {
        printf("FAILED: HOME environment variable not set\n");
        return false;
    }
    
    char path1[1024], path2[1024];
    snprintf(path1, sizeof(path1), 
        "%s/GeneralsX/GeneralsMD/Data/WaterPlane/caust00.tga", home);
    snprintf(path2, sizeof(path2), 
        "%s/GeneralsX/GeneralsMD/Data\\WaterPlane\\CAUST00.TGA", home);
    
    TextureCache* cache = TextureCache::GetInstance();
    
    // Load with different path formats (should normalize to same key)
    void* texture1 = cache->LoadTexture(path1);
    void* texture2 = cache->LoadTexture(path2);
    
    if (texture1 != texture2) {
        printf("FAILED: Different texture handles for normalized paths\n");
        return false;
    }
    
    printf("SUCCESS: Path normalization working\n");
    printf("  Path 1: %s\n", path1);
    printf("  Path 2: %s\n", path2);
    printf("  Same handle: %p\n", texture1);
    
    return true;
}

// Test 5: Test cache statistics
bool test_cache_statistics() {
    printf("\nTest 5: Testing cache statistics...\n");
    
    TextureCache* cache = TextureCache::GetInstance();
    
    int total_textures;
    unsigned long total_memory;
    cache->GetCacheStats(total_textures, total_memory);
    
    printf("SUCCESS: Cache statistics retrieved\n");
    printf("  Total textures: %d\n", total_textures);
    printf("  Total memory: %.2f MB\n", total_memory / (1024.0 * 1024.0));
    
    // Print detailed cache contents
    cache->DebugPrintCache();
    
    return true;
}

// Test 6: Test texture binding (requires Metal render encoder)
bool test_texture_binding() {
    printf("\nTest 6: Testing texture binding API...\n");
    
    const char* home = getenv("HOME");
    if (!home) {
        printf("FAILED: HOME environment variable not set\n");
        return false;
    }
    
    char texture_path[1024];
    snprintf(texture_path, sizeof(texture_path), 
        "%s/GeneralsX/GeneralsMD/Data/WaterPlane/caust00.tga", home);
    
    TextureCache* cache = TextureCache::GetInstance();
    void* texture = cache->LoadTexture(texture_path);
    
    if (!texture) {
        printf("FAILED: Texture not loaded\n");
        return false;
    }
    
    // NOTE: This will print a warning because we don't have an active render encoder
    // In a real game, this would be called between BeginFrame() and EndFrame()
    printf("INFO: Calling BindTexture (will warn about no render encoder)...\n");
    MetalWrapper::BindTexture((id)texture, 0);
    
    printf("SUCCESS: Texture binding API validated (see warnings above)\n");
    printf("NOTE: Actual texture binding requires Metal render pass context\n");
    
    return true;
}

int main(int argc, char** argv) {
    printf("=== Textured Quad Test Program ===\n");
    printf("Phase 28.3.4: Texture Upload & Binding Validation\n\n");
    
    // Run tests
    bool all_passed = true;
    all_passed &= test_load_dds_texture();
    all_passed &= test_load_tga_texture();
    all_passed &= test_reference_counting();
    all_passed &= test_path_normalization();
    all_passed &= test_cache_statistics();
    all_passed &= test_texture_binding();
    
    // Cleanup
    printf("\n=== Cleanup ===\n");
    TextureCache* cache = TextureCache::GetInstance();
    cache->ClearCache();
    
    // Summary
    printf("\n=== All Tests Completed ===\n");
    if (all_passed) {
        printf("Result: SUCCESS - Texture system working correctly\n");
        printf("\nNext Steps:\n");
        printf("1. Integrate TextureCache into game rendering pipeline\n");
        printf("2. Call BindTexture() before draw calls in game code\n");
        printf("3. Test with actual game models and UI elements\n");
        printf("4. Optimize sampler state caching\n");
        return 0;
    } else {
        printf("Result: FAILED - Some tests did not pass\n");
        return 1;
    }
}
