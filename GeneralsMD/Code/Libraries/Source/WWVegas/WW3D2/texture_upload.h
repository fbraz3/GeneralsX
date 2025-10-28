/**
 * @file texture_upload.h
 * @brief OpenGL texture upload pipeline for DDS and TGA formats
 * 
 * Provides high-level functions to create OpenGL textures from loaded
 * DDS (compressed) and TGA (uncompressed) image data. Handles:
 * - Compressed texture upload (DXT1/DXT3/DXT5)
 * - Uncompressed texture upload (RGBA8)
 * - Mipmap generation and upload
 * - Filtering (linear, anisotropic)
 * - Wrapping modes (repeat, clamp)
 * - Error checking
 * 
 * Example usage:
 *   DDSData* dds = LoadDDS("Data/Textures/Terrain.dds");
 *   GLuint tex_id = Create_GL_Texture_From_DDS(dds);
 *   delete dds;
 *   
 *   TGAData* tga = LoadTGA("Data/Textures/Button.tga");
 *   GLuint tex_id2 = Create_GL_Texture_From_TGA(tga);
 *   delete tga;
 */

#ifndef TEXTURE_UPLOAD_H
#define TEXTURE_UPLOAD_H

#include "dds_loader.h"
#include "tga_loader.h"
#include <glad/glad.h>
#include <cstdint>

/**
 * @brief Texture filtering modes
 */
enum class TextureFilter {
    NEAREST,                ///< Nearest-neighbor (pixelated)
    LINEAR,                 ///< Bilinear filtering
    LINEAR_MIPMAP_LINEAR,   ///< Trilinear filtering (default)
    ANISOTROPIC             ///< Anisotropic filtering (best quality)
};

/**
 * @brief Texture wrapping modes
 */
enum class TextureWrap {
    REPEAT,                 ///< Repeat texture (tiling)
    CLAMP_TO_EDGE,          ///< Clamp to edge (no tiling)
    MIRRORED_REPEAT         ///< Mirror repeat (flipped tiling)
};

/**
 * @brief Texture upload parameters
 */
struct TextureUploadParams {
    TextureFilter min_filter;       ///< Minification filter
    TextureFilter mag_filter;       ///< Magnification filter
    TextureWrap wrap_s;             ///< S (U) coordinate wrapping
    TextureWrap wrap_t;             ///< T (V) coordinate wrapping
    float anisotropy;               ///< Anisotropic filtering level (1.0-16.0)
    bool generate_mipmaps;          ///< Auto-generate mipmaps for TGA
    
    TextureUploadParams()
        : min_filter(TextureFilter::LINEAR_MIPMAP_LINEAR)
        , mag_filter(TextureFilter::LINEAR)
        , wrap_s(TextureWrap::REPEAT)
        , wrap_t(TextureWrap::REPEAT)
        , anisotropy(16.0f)
        , generate_mipmaps(true)
    {}
};

/**
 * @brief Create OpenGL texture from DDS data (compressed formats)
 * 
 * Uploads compressed DDS texture to OpenGL. Handles:
 * - DXT1/DXT3/DXT5 compressed formats
 * - Multiple mipmap levels (if present in DDS)
 * - RGB8/RGBA8 uncompressed fallback
 * 
 * Process:
 * 1. Generate texture ID (glGenTextures)
 * 2. Bind texture (glBindTexture)
 * 3. Upload all mipmap levels (glCompressedTexImage2D or glTexImage2D)
 * 4. Set filtering and wrapping parameters
 * 5. Apply anisotropic filtering if available
 * 
 * @param dds DDS image data (must be valid)
 * @param params Upload parameters (filtering, wrapping, etc.)
 * @return OpenGL texture ID (0 on failure)
 */
GLuint Create_GL_Texture_From_DDS(const DDSData* dds, const TextureUploadParams& params = TextureUploadParams());

/**
 * @brief Create OpenGL texture from TGA data (uncompressed formats)
 * 
 * Uploads uncompressed TGA texture to OpenGL. Handles:
 * - RGBA8 pixel data
 * - Optional mipmap generation (glGenerateMipmap)
 * - Linear/anisotropic filtering
 * 
 * Process:
 * 1. Generate texture ID (glGenTextures)
 * 2. Bind texture (glBindTexture)
 * 3. Upload base level (glTexImage2D)
 * 4. Generate mipmaps if requested
 * 5. Set filtering and wrapping parameters
 * 6. Apply anisotropic filtering if available
 * 
 * @param tga TGA image data (must be valid)
 * @param params Upload parameters (filtering, wrapping, etc.)
 * @return OpenGL texture ID (0 on failure)
 */
GLuint Create_GL_Texture_From_TGA(const TGAData* tga, const TextureUploadParams& params = TextureUploadParams());

/**
 * @brief Upload texture from raw memory data (for VFS integration)
 * 
 * Creates Metal/OpenGL texture directly from pixel data in memory.
 * Used when texture data comes from .big files (Virtual File System).
 * 
 * Supported formats:
 * - GL_RGBA8 (32-bit RGBA uncompressed)
 * - GL_RGB8 (24-bit RGB uncompressed)
 * - GL_COMPRESSED_RGBA_S3TC_DXT1_EXT (BC1/DXT1)
 * - GL_COMPRESSED_RGBA_S3TC_DXT3_EXT (BC2/DXT3)
 * - GL_COMPRESSED_RGBA_S3TC_DXT5_EXT (BC3/DXT5)
 * 
 * @param pixel_data Raw pixel data in memory
 * @param width Texture width in pixels
 * @param height Texture height in pixels
 * @param format OpenGL internal format (GL_RGBA8, GL_COMPRESSED_*, etc.)
 * @param data_size Size of pixel_data in bytes
 * @return OpenGL texture ID (0 on failure)
 */
GLuint Upload_Texture_From_Memory(const void* pixel_data, uint32_t width, uint32_t height,
                                   GLenum format, size_t data_size);

/**
 * @brief Upload texture from raw memory data and return Metal texture pointer (Phase 37.5)
 * 
 * Extended version that also returns Metal texture pointer for direct binding.
 * Uses same upload logic as Upload_Texture_From_Memory but exposes Metal texture.
 * 
 * @param pixel_data Raw pixel data in memory
 * @param width Texture width in pixels
 * @param height Texture height in pixels
 * @param format OpenGL internal format (GL_RGBA8, GL_COMPRESSED_*, etc.)
 * @param data_size Size of pixel_data in bytes
 * @param out_metal_texture Output: Metal texture pointer (id<MTLTexture>*) or nullptr
 * @return OpenGL texture ID (0 on failure)
 */
GLuint Upload_Texture_From_Memory_With_Metal(const void* pixel_data, uint32_t width, uint32_t height,
                                              GLenum format, size_t data_size,
                                              void** out_metal_texture);

/**
 * @brief Delete OpenGL texture
 * 
 * Wrapper around glDeleteTextures for consistency.
 * 
 * @param texture_id Texture ID to delete (0 is ignored)
 */
void Delete_GL_Texture(GLuint texture_id);

/**
 * @brief Check if anisotropic filtering is supported
 * 
 * Queries OpenGL for GL_EXT_texture_filter_anisotropic extension.
 * 
 * @return true if anisotropic filtering is available
 */
bool Is_Anisotropic_Filtering_Supported();

/**
 * @brief Get maximum anisotropic filtering level
 * 
 * Queries GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT.
 * 
 * @return Maximum anisotropy level (1.0 if not supported)
 */
float Get_Max_Anisotropy();

#endif // TEXTURE_UPLOAD_H
