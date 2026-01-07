/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
**
**  FILE: d3d8_vulkan_texture.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Texture Creation, Loading, and GPU Management
**
**  DESCRIPTION:
**    Provides Vulkan VkImage creation, format conversion, layout transitions,
**    and GPU memory management for DirectX 8 texture compatibility. Enables
**    cross-platform texture loading with support for RGBA8, BC1/2/3 DXT
**    compression formats.
**
**    Phase 12: Texture System Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_TEXTURE_H_
#define D3D8_VULKAN_TEXTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t, uint8_t, int32_t

/* ============================================================================
 * Forward Declarations (to avoid including Vulkan headers directly)
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkImage;
typedef void* VkImageView;
typedef void* VkDeviceMemory;
typedef void* VkCommandBuffer;
typedef void* VkQueue;
typedef void* VkSampler;
typedef void* VkBuffer;

/* ============================================================================
 * Texture Format Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_TEXTURE_FORMAT
 * @brief Supported texture formats (DirectX 8 → Vulkan)
 */
typedef enum {
    D3D8_VULKAN_FMT_RGBA8 = 0x1,              /* RGBA8 unorm (32-bit) */
    D3D8_VULKAN_FMT_RGB8 = 0x2,               /* RGB8 unorm (24-bit) */
    D3D8_VULKAN_FMT_BC1 = 0x3,                /* DXT1 / BC1 compression */
    D3D8_VULKAN_FMT_BC2 = 0x4,                /* DXT3 / BC2 compression */
    D3D8_VULKAN_FMT_BC3 = 0x5,                /* DXT5 / BC3 compression */
    D3D8_VULKAN_FMT_RGBA_FLOAT = 0x6,         /* RGBA32F (floating-point) */
    D3D8_VULKAN_FMT_DEPTH24 = 0x7,            /* D24 depth buffer */
    D3D8_VULKAN_FMT_DEPTH32 = 0x8,            /* D32F depth buffer */
} D3D8_VULKAN_TEXTURE_FORMAT;

/**
 * @enum D3D8_VULKAN_TEXTURE_USAGE
 * @brief Texture usage patterns
 */
typedef enum {
    D3D8_VULKAN_TEXTURE_USAGE_SAMPLE = 0x1,           /* Sample texture in shaders */
    D3D8_VULKAN_TEXTURE_USAGE_COLOR_ATTACHMENT = 0x2, /* Use as render target */
    D3D8_VULKAN_TEXTURE_USAGE_DEPTH_ATTACHMENT = 0x4, /* Use as depth target */
    D3D8_VULKAN_TEXTURE_USAGE_STORAGE = 0x8,          /* Use as storage image */
} D3D8_VULKAN_TEXTURE_USAGE;

/**
 * @enum D3D8_VULKAN_TEXTURE_FILTER
 * @brief Texture filtering modes
 */
typedef enum {
    D3D8_VULKAN_FILTER_NEAREST = 0,                   /* Point sampling */
    D3D8_VULKAN_FILTER_LINEAR = 1,                    /* Bilinear filtering */
    D3D8_VULKAN_FILTER_CUBIC = 2,                     /* Cubic filtering */
} D3D8_VULKAN_TEXTURE_FILTER;

/**
 * @enum D3D8_VULKAN_TEXTURE_WRAP
 * @brief Texture wrap/address modes
 */
typedef enum {
    D3D8_VULKAN_WRAP_CLAMP = 0,                       /* Clamp to edge */
    D3D8_VULKAN_WRAP_REPEAT = 1,                      /* Repeat/tile */
    D3D8_VULKAN_WRAP_MIRROR = 2,                      /* Mirror repeat */
} D3D8_VULKAN_TEXTURE_WRAP;

/* ============================================================================
 * Texture Configuration Types
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_TEXTURE_CONFIG
 * @brief Configuration for texture creation
 */
typedef struct {
    uint32_t width;                            /* Texture width in pixels */
    uint32_t height;                           /* Texture height in pixels */
    uint32_t mip_levels;                       /* Number of mipmap levels (1 = no mipmaps) */
    uint32_t array_layers;                     /* Array layers (1 = 2D texture, 6 = cubemap) */
    D3D8_VULKAN_TEXTURE_FORMAT format;         /* Pixel format */
    D3D8_VULKAN_TEXTURE_USAGE usage;           /* Usage flags */
} D3D8_VULKAN_TEXTURE_CONFIG;

/**
 * @struct D3D8_VULKAN_SAMPLER_CONFIG
 * @brief Sampler configuration for texture filtering and wrapping
 */
typedef struct {
    D3D8_VULKAN_TEXTURE_FILTER mag_filter;     /* Magnification filter */
    D3D8_VULKAN_TEXTURE_FILTER min_filter;     /* Minification filter */
    D3D8_VULKAN_TEXTURE_FILTER mip_filter;     /* Mipmap filter */
    D3D8_VULKAN_TEXTURE_WRAP wrap_u;           /* U coordinate wrap mode */
    D3D8_VULKAN_TEXTURE_WRAP wrap_v;           /* V coordinate wrap mode */
    D3D8_VULKAN_TEXTURE_WRAP wrap_w;           /* W coordinate wrap mode (3D textures) */
    float lod_bias;                            /* Level-of-detail bias */
    float max_anisotropy;                      /* Anisotropic filtering level */
} D3D8_VULKAN_SAMPLER_CONFIG;

/**
 * @struct D3D8_VULKAN_TEXTURE_HANDLE
 * @brief Opaque handle for texture resource management
 * @note Internal structure - contents not meant for direct access
 */
typedef struct {
    VkImage image;                             /* Vulkan image resource */
    VkImageView view;                          /* Vulkan image view */
    VkDeviceMemory device_memory;              /* GPU memory allocation */
    VkSampler sampler;                         /* Vulkan sampler object */
    uint32_t width;                            /* Texture width */
    uint32_t height;                           /* Texture height */
    D3D8_VULKAN_TEXTURE_FORMAT format;         /* Pixel format */
    uint32_t mip_levels;                       /* Number of mipmap levels */
} D3D8_VULKAN_TEXTURE_HANDLE;

/**
 * @struct D3D8_VULKAN_TEXTURE_ATLAS_OFFSET
 * @brief Texture offset within atlas
 */
typedef struct {
    uint32_t x;                                /* X offset in pixels */
    uint32_t y;                                /* Y offset in pixels */
    uint32_t w;                                /* Width in pixels */
    uint32_t h;                                /* Height in pixels */
} D3D8_VULKAN_TEXTURE_ATLAS_OFFSET;

/* ============================================================================
 * Texture Management API Functions
 * ============================================================================ */

/**
 * @brief Create a new Vulkan texture
 * @param device Vulkan device
 * @param config Texture configuration
 * @param out_handle Pointer to receive texture handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateTexture(VkDevice device, const D3D8_VULKAN_TEXTURE_CONFIG* config,
                              D3D8_VULKAN_TEXTURE_HANDLE* out_handle);

/**
 * @brief Free texture resources and release GPU memory
 * @param device Vulkan device
 * @param handle Texture handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_FreeTexture(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle);

/**
 * @brief Upload texture data to GPU memory
 * @param device Vulkan device
 * @param handle Texture handle
 * @param data CPU buffer containing pixel data
 * @param data_size Size of pixel data in bytes
 * @param staging_buffer Staging buffer for upload (can be NULL for host-visible memory)
 * @param command_buffer Command buffer for recording copy commands
 * @param queue Graphics queue for submission
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_UploadTextureData(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                  const void* data, uint64_t data_size,
                                  VkBuffer staging_buffer, VkCommandBuffer command_buffer,
                                  VkQueue queue);

/**
 * @brief Perform texture layout transition (barrier)
 * @param device Vulkan device
 * @param handle Texture handle
 * @param new_layout New layout (e.g., SHADER_READ_ONLY, COLOR_ATTACHMENT_OPTIMAL)
 * @param command_buffer Command buffer for recording barrier
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_TransitionTextureLayout(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                        uint32_t new_layout, VkCommandBuffer command_buffer);

/**
 * @brief Create texture sampler for filtering and wrapping
 * @param device Vulkan device
 * @param config Sampler configuration
 * @param out_sampler Pointer to receive sampler handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateSampler(VkDevice device, const D3D8_VULKAN_SAMPLER_CONFIG* config,
                              VkSampler* out_sampler);

/**
 * @brief Free sampler resources
 * @param device Vulkan device
 * @param sampler Sampler handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_FreeSampler(VkDevice device, VkSampler sampler);

/**
 * @brief Create texture from file (PNG, TGA, DDS, etc.)
 * @param device Vulkan device
 * @param filename Path to texture file
 * @param out_handle Pointer to receive texture handle
 * @param out_config Pointer to receive texture configuration
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_LoadTextureFromFile(VkDevice device, const char* filename,
                                    D3D8_VULKAN_TEXTURE_HANDLE* out_handle,
                                    D3D8_VULKAN_TEXTURE_CONFIG* out_config);

/**
 * @brief Create mipmap chain for texture
 * @param device Vulkan device
 * @param handle Texture handle
 * @param command_buffer Command buffer for recording blit commands
 * @param queue Graphics queue for submission
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_GenerateMipmaps(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                VkCommandBuffer command_buffer, VkQueue queue);

/**
 * @brief Create texture view for specific mip level or array layer
 * @param device Vulkan device
 * @param handle Parent texture handle
 * @param base_mip_level First mip level to include
 * @param mip_level_count Number of mip levels
 * @param base_array_layer First array layer to include
 * @param array_layer_count Number of array layers
 * @param out_view Pointer to receive view handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateTextureView(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                  uint32_t base_mip_level, uint32_t mip_level_count,
                                  uint32_t base_array_layer, uint32_t array_layer_count,
                                  VkImageView* out_view);

/**
 * @brief Get texture format size in bits per pixel
 * @param format Texture format
 * @return Bits per pixel (0 for compressed formats - use block size instead)
 */
uint32_t D3D8_Vulkan_GetFormatBPP(D3D8_VULKAN_TEXTURE_FORMAT format);

/**
 * @brief Get texture format block size for compressed formats
 * @param format Texture format
 * @param out_width Block width in pixels
 * @param out_height Block height in pixels
 * @return 0 on success, non-zero if format is not block-compressed
 */
int D3D8_Vulkan_GetFormatBlockSize(D3D8_VULKAN_TEXTURE_FORMAT format,
                                   uint32_t* out_width, uint32_t* out_height);

/**
 * @brief Convert DirectX 8 format to internal enum
 * @param dx8_format DirectX 8 D3DFORMAT value
 * @param out_format Pointer to receive converted format
 * @return 0 on success, non-zero if format not supported
 */
int D3D8_Vulkan_ConvertDX8Format(uint32_t dx8_format, D3D8_VULKAN_TEXTURE_FORMAT* out_format);

/**
 * @brief Update texture data (partial or full)
 * @param device Vulkan device
 * @param handle Texture handle
 * @param x X offset in pixels
 * @param y Y offset in pixels
 * @param width Width in pixels
 * @param height Height in pixels
 * @param data CPU buffer with new pixel data
 * @param data_size Size of pixel data
 * @param staging_buffer Staging buffer for upload
 * @param command_buffer Command buffer for recording
 * @param queue Graphics queue for submission
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_UpdateTextureRegion(VkDevice device, D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                    uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                    const void* data, uint64_t data_size,
                                    VkBuffer staging_buffer, VkCommandBuffer command_buffer,
                                    VkQueue queue);

/**
 * @brief Get texture statistics (memory usage, format info)
 * @param handle Texture handle
 * @param out_memory_size Pointer to receive memory size in bytes
 * @param out_mip_levels Pointer to receive mipmap level count
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_GetTextureStats(D3D8_VULKAN_TEXTURE_HANDLE* handle,
                                uint64_t* out_memory_size, uint32_t* out_mip_levels);

/**
 * @brief Create texture atlas from multiple source images
 * @param device Vulkan device
 * @param filenames Array of texture filenames
 * @param file_count Number of textures to atlas
 * @param max_atlas_size Maximum atlas dimension (width/height)
 * @param out_handle Pointer to receive atlas texture handle
 * @param out_offsets Array to receive UV offsets for each texture
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateTextureAtlas(VkDevice device, const char** filenames, uint32_t file_count,
                                   uint32_t max_atlas_size, D3D8_VULKAN_TEXTURE_HANDLE* out_handle,
                                   D3D8_VULKAN_TEXTURE_ATLAS_OFFSET* out_offsets);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_TEXTURE_H_ */
