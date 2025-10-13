/**
 * @file ddsloader.h
 * @brief DirectDraw Surface (DDS) texture loader for Metal backend
 * 
 * Supports BC1/BC2/BC3 (DXT1/DXT3/DXT5) compressed formats and uncompressed RGB/RGBA.
 * 
 * Phase 28.1: DDS Texture Loader Implementation
 * Date: October 13, 2025
 */

#ifndef DDSLOADER_H
#define DDSLOADER_H

#include <stdint.h>

// DDS file format constants
#define DDS_MAGIC           0x20534444  // "DDS " in little-endian
#define DDS_HEADER_SIZE     124

// DDS header flags
#define DDSD_CAPS           0x00000001
#define DDSD_HEIGHT         0x00000002
#define DDSD_WIDTH          0x00000004
#define DDSD_PITCH          0x00000008
#define DDSD_PIXELFORMAT    0x00001000
#define DDSD_MIPMAPCOUNT    0x00020000
#define DDSD_LINEARSIZE     0x00080000
#define DDSD_DEPTH          0x00800000

// Pixel format flags
#define DDPF_ALPHAPIXELS    0x00000001
#define DDPF_ALPHA          0x00000002
#define DDPF_FOURCC         0x00000004
#define DDPF_RGB            0x00000040
#define DDPF_RGBA           0x00000041
#define DDPF_LUMINANCE      0x00020000

// FourCC compression formats
#define FOURCC_DXT1         0x31545844  // "DXT1" in little-endian
#define FOURCC_DXT3         0x33545844  // "DXT3" in little-endian
#define FOURCC_DXT5         0x35545844  // "DXT5" in little-endian

// Texture format enumeration (matches Metal pixel formats)
enum DDSFormat {
    DDS_FORMAT_UNKNOWN = 0,
    DDS_FORMAT_BC1_RGBA,        // DXT1 - RGB with 1-bit alpha
    DDS_FORMAT_BC2_RGBA,        // DXT3 - RGBA explicit alpha
    DDS_FORMAT_BC3_RGBA,        // DXT5 - RGBA interpolated alpha
    DDS_FORMAT_RGBA8_UNORM,     // Uncompressed RGBA8
    DDS_FORMAT_RGB8_UNORM       // Uncompressed RGB8 (converted to RGBA8)
};

/**
 * @struct DDSPixelFormat
 * @brief DDS pixel format descriptor (32 bytes)
 */
struct DDSPixelFormat {
    uint32_t dwSize;            // Structure size (32 bytes)
    uint32_t dwFlags;           // Pixel format flags
    uint32_t dwFourCC;          // FourCC compression code
    uint32_t dwRGBBitCount;     // Bits per pixel (for uncompressed)
    uint32_t dwRBitMask;        // Red channel mask
    uint32_t dwGBitMask;        // Green channel mask
    uint32_t dwBBitMask;        // Blue channel mask
    uint32_t dwABitMask;        // Alpha channel mask
};

/**
 * @struct DDSHeader
 * @brief DDS file header (124 bytes + 4 byte magic)
 */
struct DDSHeader {
    uint32_t dwMagic;           // Magic number "DDS " (0x20534444)
    uint32_t dwSize;            // Header size (124 bytes)
    uint32_t dwFlags;           // Header flags
    uint32_t dwHeight;          // Texture height
    uint32_t dwWidth;           // Texture width
    uint32_t dwPitchOrLinearSize;  // Pitch for uncompressed, linear size for compressed
    uint32_t dwDepth;           // Depth for volume textures
    uint32_t dwMipMapCount;     // Number of mipmap levels
    uint32_t dwReserved1[11];   // Reserved
    DDSPixelFormat ddspf;       // Pixel format (32 bytes)
    uint32_t dwCaps;            // Capability flags
    uint32_t dwCaps2;           // Additional capability flags
    uint32_t dwCaps3;           // Reserved
    uint32_t dwCaps4;           // Reserved
    uint32_t dwReserved2;       // Reserved
};

/**
 * @struct DDSTextureData
 * @brief Parsed DDS texture data
 */
struct DDSTextureData {
    unsigned int width;         // Texture width
    unsigned int height;        // Texture height
    unsigned int mipMapCount;   // Number of mipmap levels (0 = no mipmaps)
    DDSFormat format;           // Texture format
    unsigned char* data;        // Pixel data (compressed or uncompressed)
    unsigned long dataSize;     // Size of pixel data in bytes
    bool isValid;               // Validation flag
};

/**
 * @class DDSLoader
 * @brief DDS texture file loader
 */
class DDSLoader {
public:
    /**
     * @brief Load DDS texture from file
     * @param filename Path to DDS file
     * @param outData Output texture data structure
     * @return true if successful, false otherwise
     */
    static bool Load(const char* filename, DDSTextureData* outData);
    
    /**
     * @brief Load DDS texture from memory buffer
     * @param buffer Memory buffer containing DDS file
     * @param bufferSize Size of buffer in bytes
     * @param outData Output texture data structure
     * @return true if successful, false otherwise
     */
    static bool LoadFromMemory(const unsigned char* buffer, unsigned long bufferSize, DDSTextureData* outData);
    
    /**
     * @brief Free texture data allocated by Load/LoadFromMemory
     * @param data Texture data structure to free
     */
    static void Free(DDSTextureData* data);
    
    /**
     * @brief Get block size for compressed format
     * @param format DDS format
     * @return Block size in bytes (8 for BC1, 16 for BC2/BC3, 0 for uncompressed)
     */
    static unsigned int GetBlockSize(DDSFormat format);
    
    /**
     * @brief Calculate compressed texture data size
     * @param width Texture width
     * @param height Texture height
     * @param format DDS format
     * @param mipMapCount Number of mipmap levels
     * @return Total data size in bytes
     */
    static unsigned long CalculateDataSize(unsigned int width, unsigned int height, 
                                          DDSFormat format, unsigned int mipMapCount);
    
    /**
     * @brief Get format name as string (for debugging)
     * @param format DDS format
     * @return Format name string
     */
    static const char* GetFormatName(DDSFormat format);

private:
    /**
     * @brief Parse DDS header from buffer
     * @param buffer Buffer containing DDS header
     * @param header Output header structure
     * @return true if valid, false otherwise
     */
    static bool ParseHeader(const unsigned char* buffer, DDSHeader* header);
    
    /**
     * @brief Determine DDS format from pixel format descriptor
     * @param pixelFormat Pixel format descriptor
     * @return DDS format enumeration
     */
    static DDSFormat DetermineFormat(const DDSPixelFormat& pixelFormat);
    
    /**
     * @brief Validate DDS header
     * @param header Header to validate
     * @return true if valid, false otherwise
     */
    static bool ValidateHeader(const DDSHeader& header);
};

#endif // DDSLOADER_H
