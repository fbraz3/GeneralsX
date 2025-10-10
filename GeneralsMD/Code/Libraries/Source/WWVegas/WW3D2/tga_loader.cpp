/**
 * @file tga_loader.cpp
 * @brief TGA texture loader implementation
 */

#ifndef _WIN32  // OpenGL backend only

#include "tga_loader.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

/**
 * @brief Validate TGA header for supported formats
 */
bool ValidateTGAHeader(const TGAHeader& header) {
    // Check image type (2 = uncompressed RGB, 10 = RLE compressed RGB)
    if (header.image_type != 2 && header.image_type != 10) {
        printf("TGA ERROR: Unsupported image type %d (only types 2 and 10 supported)\n", header.image_type);
        return false;
    }
    
    // Check dimensions
    if (header.width == 0 || header.height == 0) {
        printf("TGA ERROR: Invalid dimensions %dx%d\n", header.width, header.height);
        return false;
    }
    
    // Check bits per pixel (24 or 32)
    if (header.bits_per_pixel != 24 && header.bits_per_pixel != 32) {
        printf("TGA ERROR: Unsupported bits per pixel %d (only 24 and 32 supported)\n", header.bits_per_pixel);
        return false;
    }
    
    // Check color map type (should be 0 for RGB images)
    if (header.color_map_type != 0) {
        printf("TGA ERROR: Color mapped images not supported (color_map_type = %d)\n", header.color_map_type);
        return false;
    }
    
    return true;
}

/**
 * @brief Decompress RLE-encoded TGA pixel data
 */
bool DecompressRLE(const uint8_t* compressed, size_t compressed_size,
                   uint8_t* decompressed, size_t decompressed_size,
                   uint8_t bytes_per_pixel) {
    size_t compressed_offset = 0;
    size_t decompressed_offset = 0;
    
    while (compressed_offset < compressed_size && decompressed_offset < decompressed_size) {
        // Read packet header
        uint8_t packet_header = compressed[compressed_offset++];
        
        // Bit 7: 1 = RLE packet, 0 = raw packet
        bool is_rle_packet = (packet_header & 0x80) != 0;
        
        // Bits 0-6: repeat count (0-127, actual count is value+1)
        uint8_t pixel_count = (packet_header & 0x7F) + 1;
        
        if (is_rle_packet) {
            // RLE packet: read 1 pixel, repeat pixel_count times
            if (compressed_offset + bytes_per_pixel > compressed_size) {
                printf("TGA ERROR: RLE packet extends beyond compressed data\n");
                return false;
            }
            
            // Read the pixel to repeat
            const uint8_t* pixel = &compressed[compressed_offset];
            compressed_offset += bytes_per_pixel;
            
            // Repeat the pixel
            for (uint8_t i = 0; i < pixel_count; ++i) {
                if (decompressed_offset + bytes_per_pixel > decompressed_size) {
                    printf("TGA ERROR: RLE decompression exceeds output buffer\n");
                    return false;
                }
                
                memcpy(&decompressed[decompressed_offset], pixel, bytes_per_pixel);
                decompressed_offset += bytes_per_pixel;
            }
        } else {
            // Raw packet: read pixel_count pixels
            size_t raw_bytes = pixel_count * bytes_per_pixel;
            
            if (compressed_offset + raw_bytes > compressed_size) {
                printf("TGA ERROR: Raw packet extends beyond compressed data\n");
                return false;
            }
            
            if (decompressed_offset + raw_bytes > decompressed_size) {
                printf("TGA ERROR: Raw decompression exceeds output buffer\n");
                return false;
            }
            
            memcpy(&decompressed[decompressed_offset], &compressed[compressed_offset], raw_bytes);
            compressed_offset += raw_bytes;
            decompressed_offset += raw_bytes;
        }
    }
    
    // Verify we decompressed the expected amount
    if (decompressed_offset != decompressed_size) {
        printf("TGA WARNING: Decompressed size mismatch (expected %zu, got %zu)\n", 
               decompressed_size, decompressed_offset);
    }
    
    return true;
}

/**
 * @brief Convert BGR(A) pixel data to RGBA
 */
uint8_t* ConvertBGRToRGBA(const uint8_t* bgr_data, uint32_t width, uint32_t height, uint8_t bytes_per_pixel) {
    uint32_t pixel_count = width * height;
    uint8_t* rgba_data = new uint8_t[pixel_count * 4];
    
    if (bytes_per_pixel == 3) {
        // BGR → RGBA (add alpha channel)
        for (uint32_t i = 0; i < pixel_count; ++i) {
            rgba_data[i * 4 + 0] = bgr_data[i * 3 + 2]; // R ← B
            rgba_data[i * 4 + 1] = bgr_data[i * 3 + 1]; // G ← G
            rgba_data[i * 4 + 2] = bgr_data[i * 3 + 0]; // B ← R
            rgba_data[i * 4 + 3] = 255;                 // A = opaque
        }
    } else if (bytes_per_pixel == 4) {
        // BGRA → RGBA (swap R and B)
        for (uint32_t i = 0; i < pixel_count; ++i) {
            rgba_data[i * 4 + 0] = bgr_data[i * 4 + 2]; // R ← B
            rgba_data[i * 4 + 1] = bgr_data[i * 4 + 1]; // G ← G
            rgba_data[i * 4 + 2] = bgr_data[i * 4 + 0]; // B ← R
            rgba_data[i * 4 + 3] = bgr_data[i * 4 + 3]; // A ← A
        }
    } else {
        printf("TGA ERROR: Invalid bytes_per_pixel %d in ConvertBGRToRGBA\n", bytes_per_pixel);
        delete[] rgba_data;
        return nullptr;
    }
    
    return rgba_data;
}

/**
 * @brief Flip image vertically (bottom-up to top-down)
 */
void FlipVertical(uint8_t* pixels, uint32_t width, uint32_t height, uint8_t bytes_per_pixel) {
    uint32_t row_size = width * bytes_per_pixel;
    uint8_t* temp_row = new uint8_t[row_size];
    
    for (uint32_t y = 0; y < height / 2; ++y) {
        uint32_t top_row = y;
        uint32_t bottom_row = height - 1 - y;
        
        uint8_t* top_ptr = &pixels[top_row * row_size];
        uint8_t* bottom_ptr = &pixels[bottom_row * row_size];
        
        // Swap rows
        memcpy(temp_row, top_ptr, row_size);
        memcpy(top_ptr, bottom_ptr, row_size);
        memcpy(bottom_ptr, temp_row, row_size);
    }
    
    delete[] temp_row;
}

/**
 * @brief Load TGA image from memory buffer
 */
TGAData* LoadTGAFromMemory(const uint8_t* buffer, size_t size) {
    // Validate buffer size (minimum: header only)
    if (!buffer || size < sizeof(TGAHeader)) {
        printf("TGA ERROR: Buffer too small (size %zu, need at least %zu for header)\n", 
               size, sizeof(TGAHeader));
        return nullptr;
    }
    
    // Read header
    TGAHeader header;
    memcpy(&header, buffer, sizeof(TGAHeader));
    
    // Validate header
    if (!ValidateTGAHeader(header)) {
        return nullptr; // Error already printed
    }
    
    // Calculate pixel data offset (skip header + ID field)
    size_t pixel_data_offset = sizeof(TGAHeader) + header.id_length;
    
    if (pixel_data_offset >= size) {
        printf("TGA ERROR: Pixel data offset %zu exceeds buffer size %zu\n", pixel_data_offset, size);
        return nullptr;
    }
    
    // Determine bytes per pixel
    uint8_t bytes_per_pixel = header.bits_per_pixel / 8;
    
    // Calculate expected pixel data size
    uint32_t pixel_count = header.width * header.height;
    size_t pixel_data_size = pixel_count * bytes_per_pixel;
    
    // Allocate buffer for raw pixel data (BGR or BGRA)
    uint8_t* raw_pixels = new uint8_t[pixel_data_size];
    
    // Decompress or copy pixel data
    bool success = false;
    
    if (header.image_type == 10) {
        // RLE-compressed image
        size_t compressed_data_size = size - pixel_data_offset;
        const uint8_t* compressed_data = buffer + pixel_data_offset;
        
        printf("TGA INFO: Decompressing RLE image (%dx%d, %d bpp, compressed size %zu)\n",
               header.width, header.height, header.bits_per_pixel, compressed_data_size);
        
        success = DecompressRLE(compressed_data, compressed_data_size,
                               raw_pixels, pixel_data_size,
                               bytes_per_pixel);
    } else if (header.image_type == 2) {
        // Uncompressed image
        if (pixel_data_offset + pixel_data_size > size) {
            printf("TGA ERROR: Pixel data size %zu exceeds buffer (available %zu)\n",
                   pixel_data_size, size - pixel_data_offset);
            delete[] raw_pixels;
            return nullptr;
        }
        
        memcpy(raw_pixels, buffer + pixel_data_offset, pixel_data_size);
        success = true;
    }
    
    if (!success) {
        delete[] raw_pixels;
        return nullptr;
    }
    
    // Convert BGR(A) to RGBA
    uint8_t* rgba_pixels = ConvertBGRToRGBA(raw_pixels, header.width, header.height, bytes_per_pixel);
    delete[] raw_pixels;
    
    if (!rgba_pixels) {
        return nullptr; // Error already printed
    }
    
    // Check origin (bit 5 of image_descriptor)
    // 0 = bottom-left origin (need to flip), 1 = top-left origin (OpenGL standard)
    bool is_bottom_origin = (header.image_descriptor & 0x20) == 0;
    
    if (is_bottom_origin) {
        printf("TGA INFO: Flipping bottom-origin image to top-left\n");
        FlipVertical(rgba_pixels, header.width, header.height, 4);
    }
    
    // Create TGAData structure
    TGAData* tga = new TGAData();
    tga->width = header.width;
    tga->height = header.height;
    tga->format = (bytes_per_pixel == 4) ? TGAFormat::RGBA8 : TGAFormat::RGB8;
    tga->pixels = rgba_pixels;
    
    printf("TGA SUCCESS: Loaded %dx%d image (%d bpp, %s, %s origin)\n",
           tga->width, tga->height, header.bits_per_pixel,
           (header.image_type == 10) ? "RLE" : "uncompressed",
           is_bottom_origin ? "bottom" : "top");
    
    return tga;
}

/**
 * @brief Load TGA image from file
 */
TGAData* LoadTGA(const char* file_path) {
    if (!file_path) {
        printf("TGA ERROR: NULL file path\n");
        return nullptr;
    }
    
    // Open file
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        printf("TGA ERROR: Failed to open file '%s'\n", file_path);
        return nullptr;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        printf("TGA ERROR: Invalid file size %ld for '%s'\n", file_size, file_path);
        fclose(file);
        return nullptr;
    }
    
    // Read file into buffer
    uint8_t* buffer = new uint8_t[file_size];
    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        printf("TGA ERROR: Failed to read file '%s' (expected %ld, got %zu bytes)\n",
               file_path, file_size, bytes_read);
        delete[] buffer;
        return nullptr;
    }
    
    // Parse TGA from buffer
    TGAData* tga = LoadTGAFromMemory(buffer, file_size);
    delete[] buffer;
    
    return tga;
}

#endif  // !_WIN32
