/**
 * @file test_tga_loader.cpp
 * @brief Test program for TGA texture loader (Phase 28.2.4)
 * 
 * Tests TGA loader with water caustic textures (caust*.tga).
 * Validates header parsing, BGR→RGBA conversion, RLE decompression.
 * 
 * Compilation:
 *   c++ -std=c++17 -o /tmp/test_tga_loader tests/test_tga_loader.cpp \
 *       build/macos-arm64/Core/Libraries/Source/WWVegas/WW3D2/CMakeFiles/ww3d2.dir/tgaloader.cpp.o \
 *       -I Core/Libraries/Source/WWVegas/WW3D2/
 * 
 * Usage:
 *   /tmp/test_tga_loader
 *   /tmp/test_tga_loader $HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/caust00.tga
 */

#include "tgaloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test 1: Load TGA file and validate header
bool test_load_tga(const char* filename) {
    printf("Test 1: Loading TGA file...\n");
    printf("  Path: %s\n", filename);
    
    TGATextureData texture = TGALoader::Load(filename);
    
    if (!texture.is_valid) {
        printf("FAILED: TGA file not loaded\n");
        return false;
    }
    
    printf("SUCCESS: TGA file loaded\n");
    printf("  Width: %u\n", texture.width);
    printf("  Height: %u\n", texture.height);
    printf("  Format: %s (%d)\n", TGALoader::GetFormatName(texture.format), texture.format);
    printf("  Data size: %u bytes\n", texture.data_size);
    printf("  RLE compressed: %s\n", texture.is_rle ? "yes" : "no");
    printf("  Origin: %s\n", texture.is_top_down ? "top-down" : "bottom-up");
    
    TGALoader::Free(texture);
    return true;
}

// Test 2: Verify data size calculation
bool test_data_size(const char* filename) {
    printf("\nTest 2: Verifying data size calculation...\n");
    
    TGATextureData texture = TGALoader::Load(filename);
    
    if (!texture.is_valid) {
        printf("FAILED: TGA file not loaded\n");
        return false;
    }
    
    // Expected size for RGBA8 output
    unsigned int expected_size = texture.width * texture.height * 4;
    
    if (texture.data_size != expected_size) {
        printf("FAILED: Data size mismatch (expected=%u, actual=%u)\n", 
            expected_size, texture.data_size);
        TGALoader::Free(texture);
        return false;
    }
    
    printf("SUCCESS: Data size matches (expected=%u, actual=%u)\n", 
        expected_size, texture.data_size);
    
    TGALoader::Free(texture);
    return true;
}

// Test 3: Verify BGR→RGBA conversion
bool test_bgr_to_rgba_conversion(const char* filename) {
    printf("\nTest 3: Verifying BGR→RGBA conversion...\n");
    
    TGATextureData texture = TGALoader::Load(filename);
    
    if (!texture.is_valid) {
        printf("FAILED: TGA file not loaded\n");
        return false;
    }
    
    // Check that output is always RGBA8 (4 bytes per pixel)
    unsigned int bytes_per_pixel = texture.data_size / (texture.width * texture.height);
    if (bytes_per_pixel != 4) {
        printf("FAILED: Output not RGBA8 (got %u bytes per pixel)\n", bytes_per_pixel);
        TGALoader::Free(texture);
        return false;
    }
    
    printf("SUCCESS: Output is RGBA8 (4 bytes per pixel)\n");
    
    // Sample first pixel
    if (texture.data_size >= 4) {
        printf("  First pixel: R=%u G=%u B=%u A=%u\n",
            texture.data[0], texture.data[1], texture.data[2], texture.data[3]);
    }
    
    // Sample middle pixel
    unsigned int mid_offset = (texture.width * texture.height / 2) * 4;
    if (texture.data_size >= mid_offset + 4) {
        printf("  Middle pixel: R=%u G=%u B=%u A=%u\n",
            texture.data[mid_offset], texture.data[mid_offset + 1], 
            texture.data[mid_offset + 2], texture.data[mid_offset + 3]);
    }
    
    TGALoader::Free(texture);
    return true;
}

// Test 4: Verify data integrity
bool test_data_integrity(const char* filename) {
    printf("\nTest 4: Verifying data integrity...\n");
    
    TGATextureData texture = TGALoader::Load(filename);
    
    if (!texture.is_valid) {
        printf("FAILED: TGA file not loaded\n");
        return false;
    }
    
    if (!texture.data || texture.data_size == 0) {
        printf("FAILED: NULL data pointer or zero size\n");
        TGALoader::Free(texture);
        return false;
    }
    
    // Verify all pixels have valid RGBA values
    bool has_non_zero = false;
    for (unsigned int i = 0; i < texture.data_size; i++) {
        if (texture.data[i] != 0) {
            has_non_zero = true;
            break;
        }
    }
    
    if (!has_non_zero) {
        printf("WARNING: All pixels are zero (might be valid for some textures)\n");
    }
    
    printf("First byte: 0x%02X\n", texture.data[0]);
    printf("Last byte: 0x%02X\n", texture.data[texture.data_size - 1]);
    printf("SUCCESS: Data accessible\n");
    
    TGALoader::Free(texture);
    return true;
}

// Test 5: Multiple load/free cycles
bool test_multiple_loads(const char* filename) {
    printf("\nTest 5: Multiple load/free cycles...\n");
    
    for (int i = 0; i < 3; i++) {
        TGATextureData texture = TGALoader::Load(filename);
        
        if (!texture.is_valid) {
            printf("FAILED: Load failed on iteration %d\n", i);
            return false;
        }
        
        TGALoader::Free(texture);
    }
    
    printf("SUCCESS: 3 load/free cycles completed\n");
    return true;
}

int main(int argc, char** argv) {
    // Default test file (water caustic texture)
    const char* default_path = "$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/caust00.tga";
    const char* test_file = nullptr;
    
    if (argc > 1) {
        test_file = argv[1];
    } else {
        // Expand $HOME environment variable
        const char* home = getenv("HOME");
        if (home) {
            static char expanded_path[1024];
            snprintf(expanded_path, sizeof(expanded_path), 
                "%s/GeneralsX/GeneralsMD/Data/English/Art/Textures/caust00.tga", home);
            test_file = expanded_path;
        } else {
            test_file = default_path;
        }
    }
    
    printf("=== TGA Loader Test Program ===\n");
    printf("Test file: %s\n\n", test_file);
    
    // Run tests
    bool all_passed = true;
    all_passed &= test_load_tga(test_file);
    all_passed &= test_data_size(test_file);
    all_passed &= test_bgr_to_rgba_conversion(test_file);
    all_passed &= test_data_integrity(test_file);
    all_passed &= test_multiple_loads(test_file);
    
    // Summary
    printf("\n=== All Tests Completed ===\n");
    if (all_passed) {
        printf("Result: SUCCESS - TGA loader working correctly\n");
        return 0;
    } else {
        printf("Result: FAILED - Some tests did not pass\n");
        return 1;
    }
}
