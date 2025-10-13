/**
 * @file ddsloader.cpp
 * @brief DirectDraw Surface (DDS) texture loader implementation
 * 
 * Phase 28.1: DDS Texture Loader Implementation
 * Date: October 13, 2025
 */

#include "ddsloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to read 32-bit little-endian value
static inline uint32_t ReadUInt32LE(const unsigned char* buffer) {
    return (uint32_t)buffer[0] | 
           ((uint32_t)buffer[1] << 8) | 
           ((uint32_t)buffer[2] << 16) | 
           ((uint32_t)buffer[3] << 24);
}

bool DDSLoader::Load(const char* filename, DDSTextureData* outData) {
    if (!filename || !outData) {
        printf("DDSLoader::Load - Invalid parameters (filename=%p, outData=%p)\n", 
            (void*)filename, (void*)outData);
        return false;
    }
    
    // Initialize output structure
    memset(outData, 0, sizeof(DDSTextureData));
    
    // Open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("DDSLoader::Load - Failed to open file: %s\n", filename);
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fileSize < 128) {  // Minimum DDS file size (header only)
        printf("DDSLoader::Load - File too small: %ld bytes (minimum 128)\n", fileSize);
        fclose(file);
        return false;
    }
    
    // Read entire file into memory
    unsigned char* buffer = (unsigned char*)malloc(fileSize);
    if (!buffer) {
        printf("DDSLoader::Load - Failed to allocate %ld bytes for file buffer\n", fileSize);
        fclose(file);
        return false;
    }
    
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    fclose(file);
    
    if (bytesRead != (size_t)fileSize) {
        printf("DDSLoader::Load - Read error: expected %ld bytes, got %zu\n", 
            fileSize, bytesRead);
        free(buffer);
        return false;
    }
    
    // Parse from memory buffer
    bool result = LoadFromMemory(buffer, fileSize, outData);
    
    // Free temporary buffer (texture data is copied internally)
    free(buffer);
    
    if (result) {
        printf("DDSLoader::Load - Successfully loaded: %s\n", filename);
        printf("  Dimensions: %ux%u\n", outData->width, outData->height);
        printf("  Format: %s\n", GetFormatName(outData->format));
        printf("  Mipmaps: %u\n", outData->mipMapCount);
        printf("  Data size: %lu bytes\n", outData->dataSize);
    }
    
    return result;
}

bool DDSLoader::LoadFromMemory(const unsigned char* buffer, unsigned long bufferSize, 
                               DDSTextureData* outData) {
    if (!buffer || !outData || bufferSize < 128) {
        printf("DDSLoader::LoadFromMemory - Invalid parameters\n");
        return false;
    }
    
    // Initialize output
    memset(outData, 0, sizeof(DDSTextureData));
    
    // Parse header
    DDSHeader header;
    if (!ParseHeader(buffer, &header)) {
        printf("DDSLoader::LoadFromMemory - Failed to parse header\n");
        return false;
    }
    
    // Validate header
    if (!ValidateHeader(header)) {
        printf("DDSLoader::LoadFromMemory - Invalid header\n");
        return false;
    }
    
    // Determine format
    DDSFormat format = DetermineFormat(header.ddspf);
    if (format == DDS_FORMAT_UNKNOWN) {
        printf("DDSLoader::LoadFromMemory - Unsupported pixel format\n");
        printf("  Flags: 0x%08X, FourCC: 0x%08X\n", 
            header.ddspf.dwFlags, header.ddspf.dwFourCC);
        return false;
    }
    
    // Extract texture parameters
    outData->width = header.dwWidth;
    outData->height = header.dwHeight;
    outData->mipMapCount = (header.dwFlags & DDSD_MIPMAPCOUNT) ? header.dwMipMapCount : 0;
    outData->format = format;
    
    // Calculate expected data size
    unsigned long expectedSize = CalculateDataSize(outData->width, outData->height, 
                                                   format, outData->mipMapCount);
    
    // Verify buffer has enough data
    unsigned long dataOffset = 128;  // Header size (4 magic + 124 header)
    unsigned long availableSize = bufferSize - dataOffset;
    
    if (availableSize < expectedSize) {
        printf("DDSLoader::LoadFromMemory - Insufficient data: expected %lu bytes, available %lu\n",
            expectedSize, availableSize);
        return false;
    }
    
    // Allocate and copy texture data
    outData->dataSize = expectedSize;
    outData->data = (unsigned char*)malloc(expectedSize);
    if (!outData->data) {
        printf("DDSLoader::LoadFromMemory - Failed to allocate %lu bytes for texture data\n",
            expectedSize);
        return false;
    }
    
    memcpy(outData->data, buffer + dataOffset, expectedSize);
    outData->isValid = true;
    
    return true;
}

void DDSLoader::Free(DDSTextureData* data) {
    if (data && data->data) {
        free(data->data);
        data->data = nullptr;
        data->isValid = false;
    }
}

bool DDSLoader::ParseHeader(const unsigned char* buffer, DDSHeader* header) {
    if (!buffer || !header) {
        return false;
    }
    
    // Read magic number
    header->dwMagic = ReadUInt32LE(buffer);
    if (header->dwMagic != DDS_MAGIC) {
        printf("DDSLoader::ParseHeader - Invalid magic number: 0x%08X (expected 0x%08X)\n",
            header->dwMagic, DDS_MAGIC);
        return false;
    }
    
    // Read header size
    header->dwSize = ReadUInt32LE(buffer + 4);
    if (header->dwSize != DDS_HEADER_SIZE) {
        printf("DDSLoader::ParseHeader - Invalid header size: %u (expected %u)\n",
            header->dwSize, DDS_HEADER_SIZE);
        return false;
    }
    
    // Read header fields
    header->dwFlags = ReadUInt32LE(buffer + 8);
    header->dwHeight = ReadUInt32LE(buffer + 12);
    header->dwWidth = ReadUInt32LE(buffer + 16);
    header->dwPitchOrLinearSize = ReadUInt32LE(buffer + 20);
    header->dwDepth = ReadUInt32LE(buffer + 24);
    header->dwMipMapCount = ReadUInt32LE(buffer + 28);
    
    // Skip reserved fields (11 * 4 bytes = 44 bytes at offset 32)
    
    // Read pixel format (32 bytes at offset 76)
    const unsigned char* pfBuffer = buffer + 76;
    header->ddspf.dwSize = ReadUInt32LE(pfBuffer);
    header->ddspf.dwFlags = ReadUInt32LE(pfBuffer + 4);
    header->ddspf.dwFourCC = ReadUInt32LE(pfBuffer + 8);
    header->ddspf.dwRGBBitCount = ReadUInt32LE(pfBuffer + 12);
    header->ddspf.dwRBitMask = ReadUInt32LE(pfBuffer + 16);
    header->ddspf.dwGBitMask = ReadUInt32LE(pfBuffer + 20);
    header->ddspf.dwBBitMask = ReadUInt32LE(pfBuffer + 24);
    header->ddspf.dwABitMask = ReadUInt32LE(pfBuffer + 28);
    
    // Read caps (16 bytes at offset 108)
    header->dwCaps = ReadUInt32LE(buffer + 108);
    header->dwCaps2 = ReadUInt32LE(buffer + 112);
    header->dwCaps3 = ReadUInt32LE(buffer + 116);
    header->dwCaps4 = ReadUInt32LE(buffer + 120);
    header->dwReserved2 = ReadUInt32LE(buffer + 124);
    
    return true;
}

DDSFormat DDSLoader::DetermineFormat(const DDSPixelFormat& pixelFormat) {
    // Check for compressed formats (FourCC)
    if (pixelFormat.dwFlags & DDPF_FOURCC) {
        switch (pixelFormat.dwFourCC) {
            case FOURCC_DXT1:
                return DDS_FORMAT_BC1_RGBA;
            case FOURCC_DXT3:
                return DDS_FORMAT_BC2_RGBA;
            case FOURCC_DXT5:
                return DDS_FORMAT_BC3_RGBA;
            default:
                printf("DDSLoader::DetermineFormat - Unknown FourCC: 0x%08X\n", 
                    pixelFormat.dwFourCC);
                return DDS_FORMAT_UNKNOWN;
        }
    }
    
    // Check for uncompressed RGB/RGBA
    if (pixelFormat.dwFlags & DDPF_RGB) {
        if (pixelFormat.dwRGBBitCount == 32) {
            // 32-bit RGBA
            return DDS_FORMAT_RGBA8_UNORM;
        } else if (pixelFormat.dwRGBBitCount == 24) {
            // 24-bit RGB (will be converted to RGBA)
            return DDS_FORMAT_RGB8_UNORM;
        }
    }
    
    printf("DDSLoader::DetermineFormat - Unsupported format flags: 0x%08X, bitCount: %u\n",
        pixelFormat.dwFlags, pixelFormat.dwRGBBitCount);
    return DDS_FORMAT_UNKNOWN;
}

bool DDSLoader::ValidateHeader(const DDSHeader& header) {
    // Check required flags
    if (!(header.dwFlags & DDSD_WIDTH) || 
        !(header.dwFlags & DDSD_HEIGHT) || 
        !(header.dwFlags & DDSD_PIXELFORMAT)) {
        printf("DDSLoader::ValidateHeader - Missing required flags (0x%08X)\n", header.dwFlags);
        return false;
    }
    
    // Validate dimensions
    if (header.dwWidth == 0 || header.dwHeight == 0) {
        printf("DDSLoader::ValidateHeader - Invalid dimensions: %ux%u\n", 
            header.dwWidth, header.dwHeight);
        return false;
    }
    
    // Validate pixel format size
    if (header.ddspf.dwSize != 32) {
        printf("DDSLoader::ValidateHeader - Invalid pixel format size: %u (expected 32)\n",
            header.ddspf.dwSize);
        return false;
    }
    
    return true;
}

unsigned int DDSLoader::GetBlockSize(DDSFormat format) {
    switch (format) {
        case DDS_FORMAT_BC1_RGBA:
            return 8;   // DXT1: 8 bytes per 4x4 block
        case DDS_FORMAT_BC2_RGBA:
        case DDS_FORMAT_BC3_RGBA:
            return 16;  // DXT3/DXT5: 16 bytes per 4x4 block
        case DDS_FORMAT_RGBA8_UNORM:
        case DDS_FORMAT_RGB8_UNORM:
            return 0;   // Uncompressed (no blocks)
        default:
            return 0;
    }
}

unsigned long DDSLoader::CalculateDataSize(unsigned int width, unsigned int height, 
                                          DDSFormat format, unsigned int mipMapCount) {
    unsigned int blockSize = GetBlockSize(format);
    unsigned long totalSize = 0;
    
    // If uncompressed, calculate based on pixel size
    if (blockSize == 0) {
        unsigned int bytesPerPixel = (format == DDS_FORMAT_RGBA8_UNORM) ? 4 : 3;
        
        // Calculate size for all mipmap levels
        unsigned int w = width;
        unsigned int h = height;
        for (unsigned int i = 0; i <= mipMapCount; i++) {
            totalSize += w * h * bytesPerPixel;
            w = (w > 1) ? (w / 2) : 1;
            h = (h > 1) ? (h / 2) : 1;
        }
        
        return totalSize;
    }
    
    // Compressed format - calculate based on 4x4 blocks
    unsigned int w = width;
    unsigned int h = height;
    
    for (unsigned int i = 0; i <= mipMapCount; i++) {
        unsigned int blocksWide = (w + 3) / 4;  // Round up to multiple of 4
        unsigned int blocksHigh = (h + 3) / 4;
        unsigned int mipSize = blocksWide * blocksHigh * blockSize;
        
        totalSize += mipSize;
        
        // Next mipmap level (minimum 1x1)
        w = (w > 1) ? (w / 2) : 1;
        h = (h > 1) ? (h / 2) : 1;
    }
    
    return totalSize;
}

const char* DDSLoader::GetFormatName(DDSFormat format) {
    switch (format) {
        case DDS_FORMAT_BC1_RGBA:
            return "BC1 (DXT1) RGBA";
        case DDS_FORMAT_BC2_RGBA:
            return "BC2 (DXT3) RGBA";
        case DDS_FORMAT_BC3_RGBA:
            return "BC3 (DXT5) RGBA";
        case DDS_FORMAT_RGBA8_UNORM:
            return "RGBA8 Unorm";
        case DDS_FORMAT_RGB8_UNORM:
            return "RGB8 Unorm";
        default:
            return "Unknown";
    }
}
