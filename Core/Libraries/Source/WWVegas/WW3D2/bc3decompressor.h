/**
 * @file bc3decompressor.h
 * @brief Phase 28.4: BC3 (DXT5) texture decompression to RGBA8
 * 
 * Workaround for BC3 texture rendering bug on Metal backend.
 * Decompresses BC3 blocks to uncompressed RGBA8 format before GPU upload.
 * 
 * Based on reference implementation from fighter19-dxvk-port.
 */

#ifndef BC3DECOMPRESSOR_H
#define BC3DECOMPRESSOR_H

namespace GX {

/**
 * @class BC3Decompressor
 * @brief Decompress BC3 (DXT5) compressed texture data to RGBA8
 */
class BC3Decompressor {
public:
    /**
     * @brief Decompress BC3 texture to RGBA8
     * @param compressedData BC3 compressed data (16 bytes per 4x4 block)
     * @param width Texture width in pixels (must be multiple of 4)
     * @param height Texture height in pixels (must be multiple of 4)
     * @param outData Output RGBA8 data (width * height * 4 bytes) - caller must allocate
     * @return true if successful, false on error
     */
    static bool Decompress(const unsigned char* compressedData, 
                          unsigned int width, 
                          unsigned int height,
                          unsigned char* outData);

    /**
     * @brief Calculate size of decompressed RGBA8 data
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @return Size in bytes (width * height * 4)
     */
    static unsigned int GetDecompressedSize(unsigned int width, unsigned int height) {
        return width * height * 4;  // RGBA8 = 4 bytes per pixel
    }

private:
    /**
     * @brief Decompress a single 4x4 BC3 block
     * @param blockData 16-byte BC3 block (8 bytes alpha + 8 bytes color)
     * @param destPixels Output RGBA8 pixels (16 pixels = 64 bytes)
     */
    static void DecompressBlock(const unsigned char* blockData, unsigned char* destPixels);
    
    /**
     * @brief Decompress BC4 alpha block (8 bytes → 16 alpha values)
     * @param alphaBlock 8-byte alpha block
     * @param alphaValues Output 16 alpha values (0-255)
     */
    static void DecompressAlphaBlock(const unsigned char* alphaBlock, unsigned char* alphaValues);
    
    /**
     * @brief Decompress BC1 color block (8 bytes → 16 RGB values)
     * @param colorBlock 8-byte color block
     * @param rgbValues Output 48 bytes (16 pixels × 3 bytes RGB)
     */
    static void DecompressColorBlock(const unsigned char* colorBlock, unsigned char* rgbValues);
    
    /**
     * @brief Convert RGB565 to RGB888
     * @param rgb565 16-bit RGB565 value
     * @param r Output red (0-255)
     * @param g Output green (0-255)
     * @param b Output blue (0-255)
     */
    static void RGB565ToRGB888(unsigned short rgb565, 
                               unsigned char& r, 
                               unsigned char& g, 
                               unsigned char& b);
};

} // namespace GX

#endif // BC3DECOMPRESSOR_H
