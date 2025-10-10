//============================================================================
// dds_loader.cpp
//
// DDS (DirectDraw Surface) texture loader implementation
//
// Part of Phase 28: Texture System Implementation
// Created: October 9, 2025
//============================================================================

#include "dds_loader.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

#ifndef _WIN32
#include <glad/glad.h>
#endif

//============================================================================
// Helper Functions
//============================================================================

static size_t CalculateMipSize(DDSFormat format, int width, int height) {
    switch (format) {
        case DDS_FORMAT_DXT1: {
            // BC1: 4 bits per pixel (8 bytes per 4x4 block)
            int blocks_wide = std::max(1, (width + 3) / 4);
            int blocks_high = std::max(1, (height + 3) / 4);
            return blocks_wide * blocks_high * 8;
        }
        
        case DDS_FORMAT_DXT3:
        case DDS_FORMAT_DXT5: {
            // BC2/BC3: 8 bits per pixel (16 bytes per 4x4 block)
            int blocks_wide = std::max(1, (width + 3) / 4);
            int blocks_high = std::max(1, (height + 3) / 4);
            return blocks_wide * blocks_high * 16;
        }
        
        case DDS_FORMAT_RGBA8:
            return width * height * 4;
            
        case DDS_FORMAT_RGB8:
            return width * height * 3;
            
        default:
            return 0;
    }
}

//============================================================================
// Public API Implementation
//============================================================================

bool ValidateDDSHeader(const DDSHeader& header) {
    // Check structure size
    if (header.dwSize != 124) {
        printf("DDS: Invalid header size %u (expected 124)\n", header.dwSize);
        return false;
    }
    
    // Check required flags
    const uint32_t required_flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    if ((header.dwFlags & required_flags) != required_flags) {
        printf("DDS: Missing required flags (got 0x%08X)\n", header.dwFlags);
        return false;
    }
    
    // Check dimensions
    if (header.dwWidth == 0 || header.dwHeight == 0) {
        printf("DDS: Invalid dimensions %ux%u\n", header.dwWidth, header.dwHeight);
        return false;
    }
    
    // Validate pixel format size
    if (header.ddspf.dwSize != 32) {
        printf("DDS: Invalid pixel format size %u (expected 32)\n", header.ddspf.dwSize);
        return false;
    }
    
    // Check for supported formats
    if (header.ddspf.dwFlags & DDPF_FOURCC) {
        uint32_t fourcc = header.ddspf.dwFourCC;
        if (fourcc != FOURCC_DXT1 && fourcc != FOURCC_DXT3 && fourcc != FOURCC_DXT5) {
            printf("DDS: Unsupported FourCC 0x%08X\n", fourcc);
            return false;
        }
    } else if (header.ddspf.dwFlags & DDPF_RGB) {
        // Uncompressed RGB/RGBA
        if (header.ddspf.dwRGBBitCount != 24 && header.ddspf.dwRGBBitCount != 32) {
            printf("DDS: Unsupported bit depth %u\n", header.ddspf.dwRGBBitCount);
            return false;
        }
    } else {
        printf("DDS: Unsupported pixel format flags 0x%08X\n", header.ddspf.dwFlags);
        return false;
    }
    
    return true;
}

DDSFormat GetFormatFromFourCC(uint32_t fourcc) {
    switch (fourcc) {
        case FOURCC_DXT1: return DDS_FORMAT_DXT1;
        case FOURCC_DXT3: return DDS_FORMAT_DXT3;
        case FOURCC_DXT5: return DDS_FORMAT_DXT5;
        default: return DDS_FORMAT_UNKNOWN;
    }
}

size_t CalculateCompressedSize(DDSFormat format, int width, int height) {
    return CalculateMipSize(format, width, height);
}

#ifndef _WIN32
uint32_t GetGLInternalFormat(DDSFormat format) {
    switch (format) {
        case DDS_FORMAT_DXT1: return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case DDS_FORMAT_DXT3: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case DDS_FORMAT_DXT5: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case DDS_FORMAT_RGBA8: return GL_RGBA8;
        case DDS_FORMAT_RGB8: return GL_RGB8;
        default: return 0;
    }
}
#endif

DDSData* LoadDDSFromMemory(const uint8_t* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 4 + sizeof(DDSHeader)) {
        printf("DDS: Buffer too small (%zu bytes)\n", buffer_size);
        return nullptr;
    }
    
    // Read magic number
    uint32_t magic;
    memcpy(&magic, buffer, sizeof(magic));
    if (magic != DDS_MAGIC) {
        printf("DDS: Invalid magic number 0x%08X (expected 0x%08X)\n", magic, DDS_MAGIC);
        return nullptr;
    }
    
    // Read header
    DDSHeader header;
    memcpy(&header, buffer + 4, sizeof(DDSHeader));
    
    // Validate header
    if (!ValidateDDSHeader(header)) {
        return nullptr;
    }
    
    // Determine format
    DDSFormat format = DDS_FORMAT_UNKNOWN;
    if (header.ddspf.dwFlags & DDPF_FOURCC) {
        format = GetFormatFromFourCC(header.ddspf.dwFourCC);
    } else if (header.ddspf.dwFlags & DDPF_RGB) {
        if (header.ddspf.dwRGBBitCount == 32) {
            format = DDS_FORMAT_RGBA8;
        } else if (header.ddspf.dwRGBBitCount == 24) {
            format = DDS_FORMAT_RGB8;
        }
    }
    
    if (format == DDS_FORMAT_UNKNOWN) {
        printf("DDS: Could not determine format\n");
        return nullptr;
    }
    
    // Calculate mipmap count
    int num_mipmaps = 1; // At least base level
    if (header.dwFlags & DDSD_MIPMAPCOUNT) {
        num_mipmaps = std::max(1u, header.dwMipMapCount);
        num_mipmaps = std::min(num_mipmaps, DDS_MAX_MIPMAPS);
    }
    
    // Calculate total data size
    size_t total_data_size = 0;
    int mip_widths[DDS_MAX_MIPMAPS];
    int mip_heights[DDS_MAX_MIPMAPS];
    size_t mip_sizes[DDS_MAX_MIPMAPS];
    
    for (int i = 0; i < num_mipmaps; i++) {
        mip_widths[i] = std::max(1, (int)header.dwWidth >> i);
        mip_heights[i] = std::max(1, (int)header.dwHeight >> i);
        mip_sizes[i] = CalculateMipSize(format, mip_widths[i], mip_heights[i]);
        total_data_size += mip_sizes[i];
    }
    
    // Verify buffer size
    size_t expected_size = 4 + sizeof(DDSHeader) + total_data_size;
    if (buffer_size < expected_size) {
        printf("DDS: Buffer too small (%zu bytes, expected %zu)\n", buffer_size, expected_size);
        return nullptr;
    }
    
    // Allocate DDSData
    DDSData* dds = new DDSData();
    dds->width = header.dwWidth;
    dds->height = header.dwHeight;
    dds->format = format;
    dds->num_mipmaps = num_mipmaps;
    
    // Copy pixel data
    dds->buffer_size = total_data_size;
    dds->buffer = new uint8_t[total_data_size];
    
    const uint8_t* src = buffer + 4 + sizeof(DDSHeader);
    uint8_t* dst = dds->buffer;
    
    for (int i = 0; i < num_mipmaps; i++) {
        dds->mip_widths[i] = mip_widths[i];
        dds->mip_heights[i] = mip_heights[i];
        dds->mip_sizes[i] = mip_sizes[i];
        dds->mip_data[i] = dst;
        
        memcpy(dst, src, mip_sizes[i]);
        
        src += mip_sizes[i];
        dst += mip_sizes[i];
    }
    
    printf("DDS: Loaded %ux%u texture with %d mipmaps (format: %d)\n", 
           dds->width, dds->height, dds->num_mipmaps, dds->format);
    
    return dds;
}

DDSData* LoadDDS(const char* file_path) {
    // Open file
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        printf("DDS: Could not open file '%s'\n", file_path);
        return nullptr;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read entire file into memory
    uint8_t* buffer = new uint8_t[file_size];
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != file_size) {
        printf("DDS: Failed to read file '%s' (%zu of %zu bytes)\n", 
               file_path, bytes_read, file_size);
        delete[] buffer;
        return nullptr;
    }
    
    // Parse from memory
    DDSData* dds = LoadDDSFromMemory(buffer, file_size);
    
    delete[] buffer;
    return dds;
}
