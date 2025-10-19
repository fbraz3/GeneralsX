#ifndef TGALOADER_H
#define TGALOADER_H

/**
 * @file tgaloader.h
 * @brief TGA (Targa) texture loader for Command & Conquer Generals Zero Hour
 * 
 * Phase 28.2: TGA Texture Loader Implementation
 * Supports:
 * - Uncompressed RGB/RGBA (type 2)
 * - RLE compressed RGB/RGBA (type 10)
 * - BGR→RGBA color swizzle for Metal compatibility
 * - Bottom-up and top-down image origins
 * 
 * TGA Format Specification:
 * - Header: 18 bytes
 * - Footer: Optional 26 bytes (TGA 2.0)
 * - Image data: width × height × bytes_per_pixel
 * - Color order: BGR or BGRA (needs conversion to RGBA)
 */

// TGA File Format Constants
#define TGA_TYPE_NO_IMAGE       0
#define TGA_TYPE_UNCOMPRESSED   2   // Uncompressed RGB/RGBA
#define TGA_TYPE_RLE            10  // RLE compressed RGB/RGBA
#define TGA_TYPE_GREYSCALE      3   // Uncompressed greyscale (not implemented)
#define TGA_TYPE_RLE_GREYSCALE  11  // RLE compressed greyscale (not implemented)

// Image Descriptor Bits
#define TGA_ORIGIN_MASK         0x30  // Bits 4-5: origin
#define TGA_ORIGIN_BOTTOM_LEFT  0x00  // Bottom-left origin (common)
#define TGA_ORIGIN_BOTTOM_RIGHT 0x10  // Bottom-right origin
#define TGA_ORIGIN_TOP_LEFT     0x20  // Top-left origin
#define TGA_ORIGIN_TOP_RIGHT    0x30  // Top-right origin

/**
 * @brief TGA file header (18 bytes)
 * 
 * Packed structure matching on-disk layout.
 * All fields are little-endian.
 * 
 * Note: Renamed from TGAHeader to TGAFileHeader to avoid conflict with
 * legacy TARGA.H typedef (Phase 31.2 fix)
 */
#pragma pack(push, 1)
struct TGAFileHeader {
    unsigned char  id_length;          // Length of ID field (offset 0)
    unsigned char  color_map_type;     // 0 = no color map, 1 = has color map
    unsigned char  image_type;         // 2 = uncompressed RGB, 10 = RLE compressed RGB
    unsigned short color_map_origin;   // First color map entry index
    unsigned short color_map_length;   // Number of color map entries
    unsigned char  color_map_depth;    // Bits per color map entry (15/16/24/32)
    unsigned short x_origin;           // X coordinate of lower-left corner
    unsigned short y_origin;           // Y coordinate of lower-left corner
    unsigned short width;              // Image width in pixels
    unsigned short height;             // Image height in pixels
    unsigned char  pixel_depth;        // Bits per pixel (8/16/24/32)
    unsigned char  image_descriptor;   // Bits 0-3: alpha depth, bits 4-5: origin
};
#pragma pack(pop)

/**
 * @brief TGA texture format enumeration
 * 
 * Maps TGA pixel formats to internal representation.
 * Metal backend expects RGBA byte order.
 */
enum TGAFormat {
    TGA_FORMAT_UNKNOWN = 0,
    TGA_FORMAT_RGB8 = 1,    // 24-bit RGB (3 bytes per pixel)
    TGA_FORMAT_RGBA8 = 2    // 32-bit RGBA (4 bytes per pixel)
};

/**
 * @brief Parsed TGA texture data
 * 
 * Contains decoded pixel data and metadata.
 * Pixel data is always in RGBA8 format (BGR→RGBA conversion applied).
 */
struct TGATextureData {
    unsigned int width;         // Image width in pixels
    unsigned int height;        // Image height in pixels
    TGAFormat format;           // Pixel format (RGB8 or RGBA8)
    unsigned char* data;        // Pixel data (malloc'd, caller must free)
    unsigned int data_size;     // Size of pixel data in bytes
    bool is_valid;              // True if data loaded successfully
    bool is_rle;                // True if RLE compressed
    bool is_top_down;           // True if top-down origin
};

/**
 * @brief TGA texture loader
 * 
 * Static class providing TGA file parsing and decoding.
 * Similar API to DDSLoader for consistency.
 */
class TGALoader {
public:
    /**
     * @brief Load TGA texture from file
     * 
     * @param filename Path to TGA file
     * @return TGATextureData struct with decoded pixels
     * 
     * Example:
     *   TGATextureData tex = TGALoader::Load("caust00.tga");
     *   if (tex.is_valid) {
     *       // Use tex.data (RGBA8 format)
     *       TGALoader::Free(tex);
     *   }
     */
    static TGATextureData Load(const char* filename);
    
    /**
     * @brief Load TGA texture from memory buffer
     * 
     * @param buffer Pointer to TGA file data in memory
     * @param buffer_size Size of buffer in bytes
     * @return TGATextureData struct with decoded pixels
     */
    static TGATextureData LoadFromMemory(const unsigned char* buffer, unsigned int buffer_size);
    
    /**
     * @brief Free TGA texture data
     * 
     * @param texture_data TGATextureData struct to free
     */
    static void Free(TGATextureData& texture_data);
    
    /**
     * @brief Get format name for debugging
     * 
     * @param format TGAFormat enum value
     * @return Human-readable format name
     */
    static const char* GetFormatName(TGAFormat format);
    
private:
    /**
     * @brief Parse TGA header from buffer
     * 
     * @param buffer Pointer to start of TGA file
     * @param buffer_size Size of buffer in bytes
     * @param header Output TGAFileHeader struct
     * @return True if header parsed successfully
     */
    static bool ParseHeader(const unsigned char* buffer, unsigned int buffer_size, TGAFileHeader& header);
    
    /**
     * @brief Validate TGA header
     * 
     * @param header TGAFileHeader struct to validate
     * @return True if header is valid
     */
    static bool ValidateHeader(const TGAFileHeader& header);
    
    /**
     * @brief Determine pixel format from header
     * 
     * @param header TGAFileHeader struct
     * @return TGAFormat enum value
     */
    static TGAFormat DetermineFormat(const TGAFileHeader& header);
    
    /**
     * @brief Decode uncompressed TGA image data
     * 
     * @param buffer Pointer to image data (after header)
     * @param buffer_size Size of image data in bytes
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param bytes_per_pixel Bytes per pixel (3 or 4)
     * @param is_top_down True if top-down origin
     * @return Pointer to decoded RGBA8 pixel data (malloc'd)
     */
    static unsigned char* DecodeUncompressed(
        const unsigned char* buffer,
        unsigned int buffer_size,
        unsigned int width,
        unsigned int height,
        unsigned int bytes_per_pixel,
        bool is_top_down
    );
    
    /**
     * @brief Decode RLE compressed TGA image data
     * 
     * @param buffer Pointer to RLE compressed data (after header)
     * @param buffer_size Size of compressed data in bytes
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @param bytes_per_pixel Bytes per pixel (3 or 4)
     * @param is_top_down True if top-down origin
     * @return Pointer to decoded RGBA8 pixel data (malloc'd)
     */
    static unsigned char* DecodeRLE(
        const unsigned char* buffer,
        unsigned int buffer_size,
        unsigned int width,
        unsigned int height,
        unsigned int bytes_per_pixel,
        bool is_top_down
    );
    
    /**
     * @brief Convert BGR/BGRA pixel to RGBA
     * 
     * @param src Source pixel data (BGR or BGRA)
     * @param dst Destination pixel data (RGBA)
     * @param bytes_per_pixel Bytes per pixel (3 or 4)
     */
    static void ConvertBGRtoRGBA(const unsigned char* src, unsigned char* dst, unsigned int bytes_per_pixel);
    
    /**
     * @brief Read little-endian 16-bit unsigned integer
     * 
     * @param buffer Pointer to 2-byte value
     * @return 16-bit value in host byte order
     */
    static unsigned short ReadUInt16LE(const unsigned char* buffer);
};

#endif // TGALOADER_H
