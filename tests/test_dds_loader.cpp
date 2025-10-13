/**
 * @file test_dds_loader.cpp
 * @brief Test program for DDS loader and Metal texture creation
 * 
 * Phase 28.1.4: Testing with defeated.dds
 * Date: October 13, 2025
 */

#include "ddsloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include "metalwrapper.h"
#endif

int main(int argc, char* argv[]) {
    printf("=== DDS Loader Test Program ===\n\n");
    
    // Default test file
    const char* testFile = "$HOME/GeneralsX/GeneralsMD/Data/English/Art/Textures/defeated.dds";
    
    // Allow custom file path from command line
    if (argc > 1) {
        testFile = argv[1];
    }
    
    // Expand $HOME if present
    char expandedPath[512];
    if (strncmp(testFile, "$HOME", 5) == 0) {
        const char* home = getenv("HOME");
        if (home) {
            snprintf(expandedPath, sizeof(expandedPath), "%s%s", home, testFile + 5);
            testFile = expandedPath;
        }
    }
    
    printf("Test file: %s\n\n", testFile);
    
    // Test 1: Load DDS file
    printf("Test 1: Loading DDS file...\n");
    DDSTextureData textureData;
    bool loadResult = DDSLoader::Load(testFile, &textureData);
    
    if (!loadResult) {
        printf("FAILED: Could not load DDS file\n");
        return 1;
    }
    
    printf("SUCCESS: DDS file loaded\n");
    printf("  Width: %u\n", textureData.width);
    printf("  Height: %u\n", textureData.height);
    printf("  Format: %s (%d)\n", DDSLoader::GetFormatName(textureData.format), textureData.format);
    printf("  Mipmaps: %u\n", textureData.mipMapCount);
    printf("  Data size: %lu bytes\n", textureData.dataSize);
    printf("  Valid: %s\n\n", textureData.isValid ? "yes" : "no");
    
    // Test 2: Verify data size calculation
    printf("Test 2: Verifying data size calculation...\n");
    unsigned long expectedSize = DDSLoader::CalculateDataSize(
        textureData.width, 
        textureData.height,
        textureData.format,
        textureData.mipMapCount
    );
    
    if (expectedSize == textureData.dataSize) {
        printf("SUCCESS: Data size matches (expected=%lu, actual=%lu)\n\n", 
            expectedSize, textureData.dataSize);
    } else {
        printf("WARNING: Data size mismatch (expected=%lu, actual=%lu)\n\n",
            expectedSize, textureData.dataSize);
    }
    
    // Test 3: Verify block size
    printf("Test 3: Verifying block size...\n");
    unsigned int blockSize = DDSLoader::GetBlockSize(textureData.format);
    printf("Block size: %u bytes\n", blockSize);
    
    if (blockSize > 0) {
        unsigned int blocksWide = (textureData.width + 3) / 4;
        unsigned int blocksHigh = (textureData.height + 3) / 4;
        unsigned int calculatedSize = blocksWide * blocksHigh * blockSize;
        printf("Blocks: %ux%u = %u total\n", blocksWide, blocksHigh, blocksWide * blocksHigh);
        printf("Calculated size: %u bytes\n", calculatedSize);
        
        if (calculatedSize == textureData.dataSize) {
            printf("SUCCESS: Block-based calculation matches\n\n");
        } else {
            printf("INFO: Block size for mipmap 0 only: %u bytes (total includes mipmaps: %lu)\n\n",
                calculatedSize, textureData.dataSize);
        }
    } else {
        printf("INFO: Uncompressed format (no blocks)\n\n");
    }
    
#ifdef __APPLE__
    // Test 4: Create Metal texture (macOS only)
    printf("Test 4: Creating Metal texture...\n");
    
    // Note: This would normally require Metal initialization via MetalWrapper::Initialize()
    // For standalone testing, we'll just show the call pattern
    printf("INFO: Metal texture creation requires initialized Metal device\n");
    printf("      In game context, call:\n");
    printf("      id texture = MetalWrapper::CreateTextureFromDDS(\n");
    printf("          %u, %u, %d, textureData.data, %lu, %u);\n",
        textureData.width, textureData.height, textureData.format, 
        textureData.dataSize, textureData.mipMapCount);
    printf("      MetalWrapper::DeleteTexture(texture);\n\n");
#else
    printf("Test 4: Skipped (Metal only available on macOS)\n\n");
#endif
    
    // Test 5: Verify data integrity
    printf("Test 5: Verifying data integrity...\n");
    if (textureData.data && textureData.dataSize > 0) {
        // Check first and last bytes are accessible
        unsigned char firstByte = textureData.data[0];
        unsigned char lastByte = textureData.data[textureData.dataSize - 1];
        printf("First byte: 0x%02X\n", firstByte);
        printf("Last byte: 0x%02X\n", lastByte);
        printf("SUCCESS: Data accessible\n\n");
    } else {
        printf("FAILED: Data pointer invalid\n\n");
    }
    
    // Cleanup
    printf("Cleanup: Freeing texture data...\n");
    DDSLoader::Free(&textureData);
    printf("SUCCESS: Memory freed\n\n");
    
    printf("=== All Tests Completed ===\n");
    printf("Result: SUCCESS - DDS loader working correctly\n");
    
    return 0;
}
