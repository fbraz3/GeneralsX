/**
 * @file bc3decompressor.cpp
 * @brief Phase 28.4: BC3 (DXT5) texture decompression implementation
 */

#include "bc3decompressor.h"
#include <cstring>
#include <cstdio>

namespace GX {

// Helper: RGB565 to RGB888 conversion
void BC3Decompressor::RGB565ToRGB888(unsigned short rgb565, 
                                     unsigned char& r, 
                                     unsigned char& g, 
                                     unsigned char& b) {
    // Extract 5-bit red, 6-bit green, 5-bit blue
    unsigned int r5 = (rgb565 >> 11) & 0x1F;
    unsigned int g6 = (rgb565 >> 5) & 0x3F;
    unsigned int b5 = rgb565 & 0x1F;
    
    // Scale to 8-bit (replicate high bits to low bits for better precision)
    r = (r5 << 3) | (r5 >> 2);
    g = (g6 << 2) | (g6 >> 4);
    b = (b5 << 3) | (b5 >> 2);
}

// Helper: Interpolate two colors (weight in range 0-255)
static void InterpolateColor(unsigned char r0, unsigned char g0, unsigned char b0,
                            unsigned char r1, unsigned char g1, unsigned char b1,
                            unsigned int weight,  // 0-255
                            unsigned char& rOut, unsigned char& gOut, unsigned char& bOut) {
    // weight=0 → 100% color0, weight=255 → 100% color1
    rOut = ((r0 * (255 - weight)) + (r1 * weight)) / 255;
    gOut = ((g0 * (255 - weight)) + (g1 * weight)) / 255;
    bOut = ((b0 * (255 - weight)) + (b1 * weight)) / 255;
}

// Decompress BC4 alpha block (8 bytes → 16 alpha values)
void BC3Decompressor::DecompressAlphaBlock(const unsigned char* alphaBlock, 
                                           unsigned char* alphaValues) {
    // First 2 bytes: alpha0 and alpha1
    unsigned char alpha0 = alphaBlock[0];
    unsigned char alpha1 = alphaBlock[1];
    
    // Build alpha palette (8 values)
    unsigned char alphas[8];
    alphas[0] = alpha0;
    alphas[1] = alpha1;
    
    if (alpha0 > alpha1) {
        // 8-alpha block (6 interpolated values)
        alphas[2] = (6 * alpha0 + 1 * alpha1 + 3) / 7;
        alphas[3] = (5 * alpha0 + 2 * alpha1 + 3) / 7;
        alphas[4] = (4 * alpha0 + 3 * alpha1 + 3) / 7;
        alphas[5] = (3 * alpha0 + 4 * alpha1 + 3) / 7;
        alphas[6] = (2 * alpha0 + 5 * alpha1 + 3) / 7;
        alphas[7] = (1 * alpha0 + 6 * alpha1 + 3) / 7;
    } else {
        // 6-alpha block (4 interpolated + 2 special values)
        alphas[2] = (4 * alpha0 + 1 * alpha1 + 2) / 5;
        alphas[3] = (3 * alpha0 + 2 * alpha1 + 2) / 5;
        alphas[4] = (2 * alpha0 + 3 * alpha1 + 2) / 5;
        alphas[5] = (1 * alpha0 + 4 * alpha1 + 2) / 5;
        alphas[6] = 0;     // Transparent
        alphas[7] = 255;   // Opaque
    }
    
    // Extract 3-bit indices from remaining 6 bytes (48 bits = 16 × 3-bit indices)
    // Indices stored as: [byte2][byte3][byte4][byte5][byte6][byte7]
    unsigned long long indices = 0;
    for (int i = 0; i < 6; i++) {
        indices |= ((unsigned long long)alphaBlock[2 + i]) << (i * 8);
    }
    
    // Decode 16 alpha values
    for (int i = 0; i < 16; i++) {
        unsigned int index = (indices >> (i * 3)) & 0x7;  // Extract 3 bits
        alphaValues[i] = alphas[index];
    }
}

// Decompress BC1 color block (8 bytes → 16 RGB values)
void BC3Decompressor::DecompressColorBlock(const unsigned char* colorBlock, 
                                           unsigned char* rgbValues) {
    // First 4 bytes: two RGB565 colors
    unsigned short color0_565 = *(unsigned short*)&colorBlock[0];
    unsigned short color1_565 = *(unsigned short*)&colorBlock[2];
    
    // Convert to RGB888
    unsigned char r0, g0, b0, r1, g1, b1;
    RGB565ToRGB888(color0_565, r0, g0, b0);
    RGB565ToRGB888(color1_565, r1, g1, b1);
    
    // Build color palette (4 colors)
    unsigned char colors[4][3];
    colors[0][0] = r0; colors[0][1] = g0; colors[0][2] = b0;
    colors[1][0] = r1; colors[1][1] = g1; colors[1][2] = b1;
    
    // BC3 always uses 4-color mode (even if color0 <= color1)
    // Color2 = 2/3 color0 + 1/3 color1
    // Color3 = 1/3 color0 + 2/3 color1
    InterpolateColor(r0, g0, b0, r1, g1, b1, 85,  // 85 ≈ 1/3 * 255
                    colors[2][0], colors[2][1], colors[2][2]);
    InterpolateColor(r0, g0, b0, r1, g1, b1, 170, // 170 ≈ 2/3 * 255
                    colors[3][0], colors[3][1], colors[3][2]);
    
    // Extract 2-bit indices from last 4 bytes (32 bits = 16 × 2-bit indices)
    unsigned int indices = *(unsigned int*)&colorBlock[4];
    
    // Decode 16 RGB values
    for (int i = 0; i < 16; i++) {
        unsigned int index = (indices >> (i * 2)) & 0x3;  // Extract 2 bits
        rgbValues[i * 3 + 0] = colors[index][0];  // R
        rgbValues[i * 3 + 1] = colors[index][1];  // G
        rgbValues[i * 3 + 2] = colors[index][2];  // B
    }
}

// Decompress a single 4x4 BC3 block
void BC3Decompressor::DecompressBlock(const unsigned char* blockData, 
                                      unsigned char* destPixels) {
    // BC3 block structure:
    // [0-7]: Alpha block (BC4)
    // [8-15]: Color block (BC1)
    
    unsigned char alphaValues[16];
    unsigned char rgbValues[48];  // 16 pixels × 3 bytes
    
    DecompressAlphaBlock(blockData, alphaValues);
    DecompressColorBlock(blockData + 8, rgbValues);
    
    // Combine RGB + Alpha → RGBA8
    for (int i = 0; i < 16; i++) {
        destPixels[i * 4 + 0] = rgbValues[i * 3 + 0];  // R
        destPixels[i * 4 + 1] = rgbValues[i * 3 + 1];  // G
        destPixels[i * 4 + 2] = rgbValues[i * 3 + 2];  // B
        destPixels[i * 4 + 3] = alphaValues[i];         // A
    }
}

// Main decompression function
bool BC3Decompressor::Decompress(const unsigned char* compressedData, 
                                 unsigned int width, 
                                 unsigned int height,
                                 unsigned char* outData) {
    if (!compressedData || !outData) {
        std::printf("BC3Decompressor ERROR: NULL pointer\n");
        return false;
    }
    
    if (width % 4 != 0 || height % 4 != 0) {
        std::printf("BC3Decompressor ERROR: Dimensions must be multiples of 4 (got %ux%u)\n", 
                   width, height);
        return false;
    }
    
    unsigned int blocksWide = width / 4;
    unsigned int blocksHigh = height / 4;
    
    std::printf("BC3Decompressor: Decompressing %ux%u texture (%u×%u blocks)\n",
               width, height, blocksWide, blocksHigh);
    
    // Process each 4x4 block
    for (unsigned int blockY = 0; blockY < blocksHigh; blockY++) {
        for (unsigned int blockX = 0; blockX < blocksWide; blockX++) {
            // Source: compressed BC3 block (16 bytes)
            unsigned int blockIndex = blockY * blocksWide + blockX;
            const unsigned char* srcBlock = compressedData + (blockIndex * 16);
            
            // Destination: 4x4 pixels in RGBA8 output
            unsigned char blockPixels[64];  // 4×4 × 4 bytes
            DecompressBlock(srcBlock, blockPixels);
            
            // Copy decompressed pixels to output image
            for (unsigned int pixelY = 0; pixelY < 4; pixelY++) {
                for (unsigned int pixelX = 0; pixelX < 4; pixelX++) {
                    unsigned int outX = blockX * 4 + pixelX;
                    unsigned int outY = blockY * 4 + pixelY;
                    unsigned int outPixelIndex = (outY * width + outX) * 4;
                    unsigned int blockPixelIndex = (pixelY * 4 + pixelX) * 4;
                    
                    outData[outPixelIndex + 0] = blockPixels[blockPixelIndex + 0];  // R
                    outData[outPixelIndex + 1] = blockPixels[blockPixelIndex + 1];  // G
                    outData[outPixelIndex + 2] = blockPixels[blockPixelIndex + 2];  // B
                    outData[outPixelIndex + 3] = blockPixels[blockPixelIndex + 3];  // A
                }
            }
        }
    }
    
    std::printf("BC3Decompressor: Decompression complete (%u bytes → %u bytes)\n",
               blocksWide * blocksHigh * 16, width * height * 4);
    
    return true;
}

} // namespace GX
