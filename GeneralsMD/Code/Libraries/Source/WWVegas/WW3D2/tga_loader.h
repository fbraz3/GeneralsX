/**
 * @file tga_loader.h
 * @brief TGA (Targa) texture loader for uncompressed and RLE-compressed images
 *
 * Supports TGA formats used by Command & Conquer Generals:
 * - Type 2: Uncompressed RGB/RGBA (24/32-bit)
 * - Type 10: RLE-compressed RGB/RGBA (24/32-bit)
 * 
 * Features:
 * - RLE packet decompression (run-length encoding)
 * - BGR to RGBA conversion (TGA uses BGR, OpenGL uses RGBA)
 * - Vertical flip handling (TGA can be bottom-up or top-down origin)
 * - Font atlas support (used for UI text rendering)
 * 
 * TGA Format Reference:
 * - Header: 18 bytes (ID length, color map, image type, dimensions, bits per pixel, descriptor)
 * - Pixel data: RGB(A) in BGR(A) byte order
 * - RLE packets: MSB=type (0=raw, 1=RLE), bits 0-6=count (0-127), followed by pixel data
 * - Origin: bit 5 of descriptor (0=bottom-left, 1=top-left)
 * 
 * Example usage:
 *   TGAData* tga = LoadTGA("Data/English/Art/Textures/AptButton.tga");
 *   if (tga) {
 *       GLuint tex_id = Create_GL_Texture_From_TGA(tga);
 *       delete tga;
 *   }
 */

#ifndef TGA_LOADER_H
#define TGA_LOADER_H

#include <cstdint>
#include <cstring>

/**
 * @brief TGA file header structure (18 bytes)
 * 
 * Packed structure matching the TGA file format specification.
 * All multi-byte fields are little-endian.
 */
#pragma pack(push, 1)
struct TGAHeader {
    uint8_t  id_length;           ///< Length of image ID field (0-255)
    uint8_t  color_map_type;      ///< 0 = no color map, 1 = color map present
    uint8_t  image_type;          ///< 2 = RGB uncompressed, 10 = RGB RLE compressed
    
    // Color map specification (5 bytes) - usually zeros for RGB images
    uint16_t color_map_first_entry; ///< Index of first color map entry
    uint16_t color_map_length;      ///< Number of color map entries
    uint8_t  color_map_entry_size;  ///< Bits per color map entry (15/16/24/32)
    
    // Image specification (10 bytes)
    uint16_t x_origin;            ///< X origin (usually 0)
    uint16_t y_origin;            ///< Y origin (usually 0)
    uint16_t width;               ///< Image width in pixels
    uint16_t height;              ///< Image height in pixels
    uint8_t  bits_per_pixel;      ///< 24 or 32 (RGB or RGBA)
    uint8_t  image_descriptor;    ///< Bit 5: origin (0=bottom, 1=top), bits 0-3: alpha depth
};
#pragma pack(pop)

/**
 * @brief TGA image format enum
 */
enum class TGAFormat {
    RGB8,   ///< 24-bit RGB (3 bytes per pixel)
    RGBA8   ///< 32-bit RGBA (4 bytes per pixel)
};

/**
 * @brief Parsed TGA image data
 * 
 * Contains decoded pixel data in RGBA8 format (OpenGL-compatible).
 * Caller is responsible for deleting the TGAData object.
 */
struct TGAData {
    uint32_t width;         ///< Image width in pixels
    uint32_t height;        ///< Image height in pixels
    TGAFormat format;       ///< Pixel format (RGB8 or RGBA8)
    uint8_t* pixels;        ///< Pixel data in RGBA8 format (always 4 bytes per pixel after conversion)
    
    TGAData() : width(0), height(0), format(TGAFormat::RGBA8), pixels(nullptr) {}
    
    ~TGAData() {
        if (pixels) {
            delete[] pixels;
            pixels = nullptr;
        }
    }
    
    /**
     * @brief Get the size of the pixel buffer in bytes
     * @return Buffer size (width * height * 4 for RGBA8)
     */
    uint32_t GetPixelBufferSize() const {
        return width * height * 4; // Always RGBA8 after conversion
    }
};

/**
 * @brief Validate TGA header for supported formats
 * 
 * Checks for:
 * - Supported image types (2 = uncompressed RGB, 10 = RLE compressed RGB)
 * - Valid dimensions (width > 0, height > 0)
 * - Valid bits per pixel (24 or 32)
 * - No color map (color_map_type == 0)
 * 
 * @param header TGA header to validate
 * @return true if header is valid and supported, false otherwise
 */
bool ValidateTGAHeader(const TGAHeader& header);

/**
 * @brief Load TGA image from memory buffer
 * 
 * Parses TGA header, decompresses RLE if needed, converts BGR to RGBA,
 * and flips vertically if bottom-origin.
 * 
 * Supported formats:
 * - Type 2: Uncompressed RGB (24-bit) or RGBA (32-bit)
 * - Type 10: RLE-compressed RGB (24-bit) or RGBA (32-bit)
 * 
 * Output format:
 * - Always RGBA8 (4 bytes per pixel)
 * - Top-left origin (OpenGL standard)
 * - RGB channels in correct order (not BGR)
 * 
 * @param buffer Pointer to TGA file data in memory
 * @param size Size of buffer in bytes
 * @return TGAData* on success (caller must delete), nullptr on failure
 */
TGAData* LoadTGAFromMemory(const uint8_t* buffer, size_t size);

/**
 * @brief Load TGA image from file
 * 
 * Wrapper around LoadTGAFromMemory that handles file I/O.
 * 
 * @param file_path Path to TGA file (relative or absolute)
 * @return TGAData* on success (caller must delete), nullptr on failure
 */
TGAData* LoadTGA(const char* file_path);

/**
 * @brief Decompress RLE-encoded TGA pixel data
 * 
 * RLE packet format:
 * - Byte 0: Packet header
 *   - Bit 7 (MSB): 1 = RLE packet, 0 = raw packet
 *   - Bits 0-6: Repeat count (0-127, actual count is value+1)
 * - RLE packet: 1 pixel follows, repeat (count+1) times
 * - Raw packet: (count+1) pixels follow
 * 
 * @param compressed Compressed pixel data
 * @param compressed_size Size of compressed data in bytes
 * @param decompressed Output buffer (must be pre-allocated)
 * @param decompressed_size Size of output buffer in bytes
 * @param bytes_per_pixel Pixel size (3 for RGB, 4 for RGBA)
 * @return true on success, false on error
 */
bool DecompressRLE(const uint8_t* compressed, size_t compressed_size,
                   uint8_t* decompressed, size_t decompressed_size,
                   uint8_t bytes_per_pixel);

/**
 * @brief Convert BGR(A) pixel data to RGBA
 * 
 * TGA stores pixels in BGR byte order, but OpenGL expects RGBA.
 * This function swaps R and B channels and adds alpha channel if needed.
 * 
 * @param bgr_data Input BGR or BGRA pixel data
 * @param width Image width
 * @param height Image height
 * @param bytes_per_pixel Input bytes per pixel (3 for BGR, 4 for BGRA)
 * @return uint8_t* RGBA8 pixel data (caller must delete[])
 */
uint8_t* ConvertBGRToRGBA(const uint8_t* bgr_data, uint32_t width, uint32_t height, uint8_t bytes_per_pixel);

/**
 * @brief Flip image vertically (bottom-up to top-down)
 * 
 * TGA images can have bottom-left or top-left origin.
 * OpenGL expects top-left origin, so bottom-origin images need flipping.
 * 
 * @param pixels Pixel data to flip (modified in-place)
 * @param width Image width
 * @param height Image height
 * @param bytes_per_pixel Bytes per pixel (4 for RGBA8)
 */
void FlipVertical(uint8_t* pixels, uint32_t width, uint32_t height, uint8_t bytes_per_pixel);

#endif // TGA_LOADER_H
