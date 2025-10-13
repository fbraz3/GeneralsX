#include "tgaloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file tgaloader.cpp
 * @brief TGA texture loader implementation
 * 
 * Phase 28.2.2: TGA Parser with BGR→RGBA Conversion
 * Phase 28.2.3: RLE Compression Support
 */

// Helper function: Read little-endian 16-bit unsigned integer
unsigned short TGALoader::ReadUInt16LE(const unsigned char* buffer) {
    return (unsigned short)(buffer[0]) | ((unsigned short)(buffer[1]) << 8);
}

// Helper function: Convert BGR/BGRA pixel to RGBA
void TGALoader::ConvertBGRtoRGBA(const unsigned char* src, unsigned char* dst, unsigned int bytes_per_pixel) {
    if (bytes_per_pixel == 3) {
        // BGR → RGBA (add alpha=255)
        dst[0] = src[2]; // R
        dst[1] = src[1]; // G
        dst[2] = src[0]; // B
        dst[3] = 255;    // A
    } else if (bytes_per_pixel == 4) {
        // BGRA → RGBA
        dst[0] = src[2]; // R
        dst[1] = src[1]; // G
        dst[2] = src[0]; // B
        dst[3] = src[3]; // A
    }
}

// Parse TGA header from buffer
bool TGALoader::ParseHeader(const unsigned char* buffer, unsigned int buffer_size, TGAHeader& header) {
    if (buffer_size < sizeof(TGAHeader)) {
        printf("TGA ERROR: Buffer too small for header (need %zu bytes, got %u)\n", 
            sizeof(TGAHeader), buffer_size);
        return false;
    }
    
    // Read header fields (18 bytes)
    header.id_length = buffer[0];
    header.color_map_type = buffer[1];
    header.image_type = buffer[2];
    header.color_map_origin = ReadUInt16LE(buffer + 3);
    header.color_map_length = ReadUInt16LE(buffer + 5);
    header.color_map_depth = buffer[7];
    header.x_origin = ReadUInt16LE(buffer + 8);
    header.y_origin = ReadUInt16LE(buffer + 10);
    header.width = ReadUInt16LE(buffer + 12);
    header.height = ReadUInt16LE(buffer + 14);
    header.pixel_depth = buffer[16];
    header.image_descriptor = buffer[17];
    
    return true;
}

// Validate TGA header
bool TGALoader::ValidateHeader(const TGAHeader& header) {
    // Check image type (only support uncompressed and RLE RGB/RGBA)
    if (header.image_type != TGA_TYPE_UNCOMPRESSED && header.image_type != TGA_TYPE_RLE) {
        printf("TGA ERROR: Unsupported image type %u (only types 2 and 10 supported)\n", 
            header.image_type);
        return false;
    }
    
    // Check dimensions
    if (header.width == 0 || header.height == 0) {
        printf("TGA ERROR: Invalid dimensions %ux%u\n", header.width, header.height);
        return false;
    }
    
    // Check pixel depth (24-bit RGB or 32-bit RGBA)
    if (header.pixel_depth != 24 && header.pixel_depth != 32) {
        printf("TGA ERROR: Unsupported pixel depth %u (only 24 and 32 supported)\n", 
            header.pixel_depth);
        return false;
    }
    
    // Check color map (should be 0 for RGB/RGBA images)
    if (header.color_map_type != 0) {
        printf("TGA WARNING: Color map present (type %u), ignoring\n", 
            header.color_map_type);
    }
    
    return true;
}

// Determine pixel format from header
TGAFormat TGALoader::DetermineFormat(const TGAHeader& header) {
    if (header.pixel_depth == 24) {
        return TGA_FORMAT_RGB8;
    } else if (header.pixel_depth == 32) {
        return TGA_FORMAT_RGBA8;
    }
    return TGA_FORMAT_UNKNOWN;
}

// Get format name for debugging
const char* TGALoader::GetFormatName(TGAFormat format) {
    switch (format) {
        case TGA_FORMAT_RGB8:  return "RGB8";
        case TGA_FORMAT_RGBA8: return "RGBA8";
        default:               return "UNKNOWN";
    }
}

// Decode uncompressed TGA image data
unsigned char* TGALoader::DecodeUncompressed(
    const unsigned char* buffer,
    unsigned int buffer_size,
    unsigned int width,
    unsigned int height,
    unsigned int bytes_per_pixel,
    bool is_top_down
) {
    unsigned int expected_size = width * height * bytes_per_pixel;
    if (buffer_size < expected_size) {
        printf("TGA ERROR: Buffer too small for uncompressed data (need %u bytes, got %u)\n",
            expected_size, buffer_size);
        return nullptr;
    }
    
    // Allocate output buffer (always RGBA8)
    unsigned int output_size = width * height * 4;
    unsigned char* output = (unsigned char*)malloc(output_size);
    if (!output) {
        printf("TGA ERROR: Failed to allocate %u bytes for output\n", output_size);
        return nullptr;
    }
    
    // Convert BGR/BGRA → RGBA with vertical flip if needed
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            // Calculate source row (flip if bottom-up origin)
            unsigned int src_y = is_top_down ? y : (height - 1 - y);
            unsigned int src_offset = (src_y * width + x) * bytes_per_pixel;
            unsigned int dst_offset = (y * width + x) * 4;
            
            ConvertBGRtoRGBA(buffer + src_offset, output + dst_offset, bytes_per_pixel);
        }
    }
    
    return output;
}

// Decode RLE compressed TGA image data
unsigned char* TGALoader::DecodeRLE(
    const unsigned char* buffer,
    unsigned int buffer_size,
    unsigned int width,
    unsigned int height,
    unsigned int bytes_per_pixel,
    bool is_top_down
) {
    // Allocate output buffer (always RGBA8)
    unsigned int output_size = width * height * 4;
    unsigned char* output = (unsigned char*)malloc(output_size);
    if (!output) {
        printf("TGA ERROR: Failed to allocate %u bytes for RLE output\n", output_size);
        return nullptr;
    }
    
    // Allocate temporary buffer for decoded BGR/BGRA data
    unsigned int temp_size = width * height * bytes_per_pixel;
    unsigned char* temp = (unsigned char*)malloc(temp_size);
    if (!temp) {
        printf("TGA ERROR: Failed to allocate %u bytes for RLE temp buffer\n", temp_size);
        free(output);
        return nullptr;
    }
    
    // Decode RLE packets
    unsigned int buffer_pos = 0;
    unsigned int pixel_count = width * height;
    unsigned int pixels_decoded = 0;
    
    while (pixels_decoded < pixel_count && buffer_pos < buffer_size) {
        // Read packet header
        unsigned char packet_header = buffer[buffer_pos++];
        bool is_rle_packet = (packet_header & 0x80) != 0;
        unsigned int run_length = (packet_header & 0x7F) + 1; // bits 0-6 + 1
        
        if (pixels_decoded + run_length > pixel_count) {
            printf("TGA ERROR: RLE packet overflow (decoded %u + %u > %u pixels)\n",
                pixels_decoded, run_length, pixel_count);
            free(temp);
            free(output);
            return nullptr;
        }
        
        if (is_rle_packet) {
            // RLE packet: repeat next pixel 'run_length' times
            if (buffer_pos + bytes_per_pixel > buffer_size) {
                printf("TGA ERROR: RLE buffer underflow (need %u bytes at pos %u)\n",
                    bytes_per_pixel, buffer_pos);
                free(temp);
                free(output);
                return nullptr;
            }
            
            // Read single pixel
            unsigned char pixel[4];
            memcpy(pixel, buffer + buffer_pos, bytes_per_pixel);
            buffer_pos += bytes_per_pixel;
            
            // Repeat pixel 'run_length' times
            for (unsigned int i = 0; i < run_length; i++) {
                unsigned int dst_offset = pixels_decoded * bytes_per_pixel;
                memcpy(temp + dst_offset, pixel, bytes_per_pixel);
                pixels_decoded++;
            }
        } else {
            // Raw packet: copy next 'run_length' pixels literally
            unsigned int raw_size = run_length * bytes_per_pixel;
            if (buffer_pos + raw_size > buffer_size) {
                printf("TGA ERROR: Raw packet buffer underflow (need %u bytes at pos %u)\n",
                    raw_size, buffer_pos);
                free(temp);
                free(output);
                return nullptr;
            }
            
            unsigned int dst_offset = pixels_decoded * bytes_per_pixel;
            memcpy(temp + dst_offset, buffer + buffer_pos, raw_size);
            buffer_pos += raw_size;
            pixels_decoded += run_length;
        }
    }
    
    if (pixels_decoded != pixel_count) {
        printf("TGA WARNING: RLE decoded %u pixels, expected %u\n", 
            pixels_decoded, pixel_count);
    }
    
    // Convert BGR/BGRA → RGBA with vertical flip if needed
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            // Calculate source row (flip if bottom-up origin)
            unsigned int src_y = is_top_down ? y : (height - 1 - y);
            unsigned int src_offset = (src_y * width + x) * bytes_per_pixel;
            unsigned int dst_offset = (y * width + x) * 4;
            
            ConvertBGRtoRGBA(temp + src_offset, output + dst_offset, bytes_per_pixel);
        }
    }
    
    free(temp);
    return output;
}

// Load TGA texture from memory buffer
TGATextureData TGALoader::LoadFromMemory(const unsigned char* buffer, unsigned int buffer_size) {
    TGATextureData result = {};
    result.is_valid = false;
    
    if (!buffer || buffer_size == 0) {
        printf("TGA ERROR: NULL buffer or zero size\n");
        return result;
    }
    
    // Parse header
    TGAHeader header;
    if (!ParseHeader(buffer, buffer_size, header)) {
        return result;
    }
    
    // Validate header
    if (!ValidateHeader(header)) {
        return result;
    }
    
    // Determine format
    TGAFormat format = DetermineFormat(header);
    if (format == TGA_FORMAT_UNKNOWN) {
        printf("TGA ERROR: Unknown format\n");
        return result;
    }
    
    // Check origin (bits 4-5 of image_descriptor)
    unsigned char origin_bits = header.image_descriptor & TGA_ORIGIN_MASK;
    bool is_top_down = (origin_bits == TGA_ORIGIN_TOP_LEFT || origin_bits == TGA_ORIGIN_TOP_RIGHT);
    
    // Calculate image data offset (header + ID field + color map)
    unsigned int data_offset = sizeof(TGAHeader) + header.id_length;
    if (header.color_map_type != 0) {
        unsigned int color_map_bytes = header.color_map_length * ((header.color_map_depth + 7) / 8);
        data_offset += color_map_bytes;
    }
    
    if (data_offset >= buffer_size) {
        printf("TGA ERROR: Data offset %u exceeds buffer size %u\n", data_offset, buffer_size);
        return result;
    }
    
    // Decode image data
    const unsigned char* image_data = buffer + data_offset;
    unsigned int image_data_size = buffer_size - data_offset;
    unsigned int bytes_per_pixel = header.pixel_depth / 8;
    
    unsigned char* decoded_data = nullptr;
    if (header.image_type == TGA_TYPE_UNCOMPRESSED) {
        decoded_data = DecodeUncompressed(image_data, image_data_size, 
            header.width, header.height, bytes_per_pixel, is_top_down);
    } else if (header.image_type == TGA_TYPE_RLE) {
        decoded_data = DecodeRLE(image_data, image_data_size,
            header.width, header.height, bytes_per_pixel, is_top_down);
    }
    
    if (!decoded_data) {
        printf("TGA ERROR: Failed to decode image data\n");
        return result;
    }
    
    // Fill result
    result.width = header.width;
    result.height = header.height;
    result.format = TGA_FORMAT_RGBA8; // Always output RGBA8
    result.data = decoded_data;
    result.data_size = header.width * header.height * 4;
    result.is_valid = true;
    result.is_rle = (header.image_type == TGA_TYPE_RLE);
    result.is_top_down = is_top_down;
    
    printf("TGA INFO: Loaded %ux%u %s texture (%s, %s origin)\n",
        result.width, result.height, GetFormatName(format),
        result.is_rle ? "RLE compressed" : "uncompressed",
        result.is_top_down ? "top-down" : "bottom-up");
    
    return result;
}

// Load TGA texture from file
TGATextureData TGALoader::Load(const char* filename) {
    TGATextureData result = {};
    result.is_valid = false;
    
    if (!filename) {
        printf("TGA ERROR: NULL filename\n");
        return result;
    }
    
    // Open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("TGA ERROR: Failed to open file '%s'\n", filename);
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > 100 * 1024 * 1024) { // 100 MB limit
        printf("TGA ERROR: Invalid file size %ld\n", file_size);
        fclose(file);
        return result;
    }
    
    // Read file to buffer
    unsigned char* buffer = (unsigned char*)malloc(file_size);
    if (!buffer) {
        printf("TGA ERROR: Failed to allocate %ld bytes for file\n", file_size);
        fclose(file);
        return result;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        printf("TGA ERROR: Read %zu bytes, expected %ld\n", bytes_read, file_size);
        free(buffer);
        return result;
    }
    
    // Parse from memory
    result = LoadFromMemory(buffer, (unsigned int)file_size);
    free(buffer);
    
    return result;
}

// Free TGA texture data
void TGALoader::Free(TGATextureData& texture_data) {
    if (texture_data.data) {
        free(texture_data.data);
        texture_data.data = nullptr;
    }
    texture_data.is_valid = false;
    texture_data.data_size = 0;
}
