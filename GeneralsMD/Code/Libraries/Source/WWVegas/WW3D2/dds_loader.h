//============================================================================
// dds_loader.h
//
// DDS (DirectDraw Surface) texture loader for OpenGL backend
// Supports BC1 (DXT1), BC2 (DXT3), BC3 (DXT5) compressed formats
//
// Part of Phase 28: Texture System Implementation
// Created: October 9, 2025
//============================================================================

#pragma once

#include <cstdint>
#include <cstddef>

// DDS magic number "DDS " (0x20534444)
#define DDS_MAGIC 0x20534444

// DDS header flags
#define DDSD_CAPS         0x00000001
#define DDSD_HEIGHT       0x00000002
#define DDSD_WIDTH        0x00000004
#define DDSD_PITCH        0x00000008
#define DDSD_PIXELFORMAT  0x00001000
#define DDSD_MIPMAPCOUNT  0x00020000
#define DDSD_LINEARSIZE   0x00080000
#define DDSD_DEPTH        0x00800000

// DDS pixel format flags
#define DDPF_ALPHAPIXELS  0x00000001
#define DDPF_ALPHA        0x00000002
#define DDPF_FOURCC       0x00000004
#define DDPF_RGB          0x00000040
#define DDPF_LUMINANCE    0x00020000

// DDS caps flags
#define DDSCAPS_COMPLEX   0x00000008
#define DDSCAPS_TEXTURE   0x00001000
#define DDSCAPS_MIPMAP    0x00400000

// FourCC codes
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
     ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))

#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')

// Maximum supported mipmap levels
#define DDS_MAX_MIPMAPS 16

//============================================================================
// DDS Structures
//============================================================================

#pragma pack(push, 1)

struct DDSPixelFormat {
    uint32_t dwSize;          // Must be 32
    uint32_t dwFlags;         // DDPF_* flags
    uint32_t dwFourCC;        // Format code (e.g., "DXT1")
    uint32_t dwRGBBitCount;   // Bits per pixel
    uint32_t dwRBitMask;      // Red channel mask
    uint32_t dwGBitMask;      // Green channel mask
    uint32_t dwBBitMask;      // Blue channel mask
    uint32_t dwABitMask;      // Alpha channel mask
};

struct DDSHeader {
    uint32_t dwSize;                // Must be 124
    uint32_t dwFlags;               // DDSD_* flags
    uint32_t dwHeight;              // Texture height
    uint32_t dwWidth;               // Texture width
    uint32_t dwPitchOrLinearSize;   // Bytes per scanline or total size
    uint32_t dwDepth;               // Depth (for volume textures)
    uint32_t dwMipMapCount;         // Number of mipmap levels
    uint32_t dwReserved1[11];       // Reserved
    DDSPixelFormat ddspf;           // Pixel format
    uint32_t dwCaps;                // Surface complexity flags
    uint32_t dwCaps2;               // Additional surface flags
    uint32_t dwCaps3;               // Reserved
    uint32_t dwCaps4;               // Reserved
    uint32_t dwReserved2;           // Reserved
};

#pragma pack(pop)

//============================================================================
// DDS Compression Format
//============================================================================

enum DDSFormat {
    DDS_FORMAT_UNKNOWN = 0,
    DDS_FORMAT_DXT1,    // BC1 (RGB/1-bit alpha, 4 bits per pixel)
    DDS_FORMAT_DXT3,    // BC2 (RGBA, 8 bits per pixel, explicit alpha)
    DDS_FORMAT_DXT5,    // BC3 (RGBA, 8 bits per pixel, interpolated alpha)
    DDS_FORMAT_RGBA8,   // Uncompressed RGBA (32 bits per pixel)
    DDS_FORMAT_RGB8     // Uncompressed RGB (24 bits per pixel)
};

//============================================================================
// DDS Data Structure
//============================================================================

struct DDSData {
    int width;              // Base level width
    int height;             // Base level height
    DDSFormat format;       // Compression format
    int num_mipmaps;        // Number of mipmap levels (including base)
    
    // Mipmap data
    int mip_widths[DDS_MAX_MIPMAPS];    // Width of each mip level
    int mip_heights[DDS_MAX_MIPMAPS];   // Height of each mip level
    size_t mip_sizes[DDS_MAX_MIPMAPS];  // Size in bytes of each mip level
    const uint8_t* mip_data[DDS_MAX_MIPMAPS]; // Pointer to each mip level data
    
    // Full data buffer (owns memory)
    uint8_t* buffer;        // Entire pixel data buffer
    size_t buffer_size;     // Total buffer size
    
    DDSData() 
        : width(0), height(0), format(DDS_FORMAT_UNKNOWN), num_mipmaps(0)
        , buffer(nullptr), buffer_size(0) 
    {
        for (int i = 0; i < DDS_MAX_MIPMAPS; i++) {
            mip_widths[i] = 0;
            mip_heights[i] = 0;
            mip_sizes[i] = 0;
            mip_data[i] = nullptr;
        }
    }
    
    ~DDSData() {
        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
        }
    }
    
    // Prevent copying (use pointers/smart pointers instead)
    DDSData(const DDSData&) = delete;
    DDSData& operator=(const DDSData&) = delete;
};

//============================================================================
// DDS Loader API
//============================================================================

/**
 * Load a DDS file from disk
 * 
 * @param file_path Path to .dds file (can be inside .BIG archive)
 * @return Pointer to DDSData structure, or nullptr on failure
 *         Caller is responsible for deleting the returned pointer
 */
DDSData* LoadDDS(const char* file_path);

/**
 * Load a DDS file from memory buffer
 * 
 * @param buffer Pointer to DDS file data in memory
 * @param buffer_size Size of buffer in bytes
 * @return Pointer to DDSData structure, or nullptr on failure
 *         Caller is responsible for deleting the returned pointer
 */
DDSData* LoadDDSFromMemory(const uint8_t* buffer, size_t buffer_size);

/**
 * Validate DDS header
 * 
 * @param header DDS header to validate
 * @return true if header is valid, false otherwise
 */
bool ValidateDDSHeader(const DDSHeader& header);

/**
 * Get format from FourCC code
 * 
 * @param fourcc FourCC code from pixel format
 * @return DDSFormat enum value
 */
DDSFormat GetFormatFromFourCC(uint32_t fourcc);

/**
 * Calculate compressed data size for a given format and dimensions
 * 
 * @param format Compression format
 * @param width Texture width
 * @param height Texture height
 * @return Size in bytes
 */
size_t CalculateCompressedSize(DDSFormat format, int width, int height);

/**
 * Get OpenGL internal format for DDS format
 * (Only available when compiled for OpenGL backend)
 * 
 * @param format DDS format
 * @return OpenGL GL_COMPRESSED_* constant
 */
#ifndef _WIN32
uint32_t GetGLInternalFormat(DDSFormat format);
#endif
