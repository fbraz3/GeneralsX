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
**  FILE: d3d8_vulkan_descriptor.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Descriptor Sets, Samplers, and Binding Management
**
**  DESCRIPTION:
**    Provides Vulkan descriptor set layout, pool, and resource binding
**    management for DirectX 8 compatibility. Enables cross-platform shader
**    resource binding with support for textures, samplers, buffers, and
**    material properties.
**
**    Phase 13: Sampler & Descriptor Sets Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_DESCRIPTOR_H_
#define D3D8_VULKAN_DESCRIPTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkDescriptorSetLayout;
typedef void* VkDescriptorPool;
typedef void* VkDescriptorSet;
typedef void* VkSampler;
typedef void* VkImageView;
typedef void* VkBuffer;
typedef unsigned int uint32_t;
// #ifndef uint64_t
// typedef unsigned long long uint64_t;
// #endif
typedef int int32_t;

/* ============================================================================
 * Descriptor Binding Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_DESCRIPTOR_TYPE
 * @brief Descriptor resource type
 */
typedef enum {
    D3D8_VULKAN_DESCRIPTOR_SAMPLER = 0x1,              /* Sampler only */
    D3D8_VULKAN_DESCRIPTOR_SAMPLED_IMAGE = 0x2,        /* Sampled texture (read-only) */
    D3D8_VULKAN_DESCRIPTOR_STORAGE_IMAGE = 0x4,        /* Storage image (read-write) */
    D3D8_VULKAN_DESCRIPTOR_UNIFORM_BUFFER = 0x8,       /* Uniform buffer (constant) */
    D3D8_VULKAN_DESCRIPTOR_STORAGE_BUFFER = 0x10,      /* Storage buffer (read-write) */
    D3D8_VULKAN_DESCRIPTOR_COMBINED_SAMPLER = 0x20,    /* Sampler + Texture combined */
} D3D8_VULKAN_DESCRIPTOR_TYPE;

/**
 * @enum D3D8_VULKAN_SHADER_STAGE
 * @brief Shader stage classification
 */
typedef enum {
    D3D8_VULKAN_SHADER_VERTEX = 0x1,                   /* Vertex shader */
    D3D8_VULKAN_SHADER_FRAGMENT = 0x2,                 /* Fragment/pixel shader */
    D3D8_VULKAN_SHADER_GEOMETRY = 0x4,                 /* Geometry shader */
    D3D8_VULKAN_SHADER_COMPUTE = 0x8,                  /* Compute shader */
} D3D8_VULKAN_SHADER_STAGE;

/**
 * @enum D3D8_VULKAN_SAMPLER_ADDRESS_MODE
 * @brief Texture coordinate addressing
 */
typedef enum {
    D3D8_VULKAN_SAMPLER_ADDRESS_CLAMP = 0,             /* Clamp to edge */
    D3D8_VULKAN_SAMPLER_ADDRESS_REPEAT = 1,            /* Repeat/tile */
    D3D8_VULKAN_SAMPLER_ADDRESS_MIRROR = 2,            /* Mirror repeat */
    D3D8_VULKAN_SAMPLER_ADDRESS_BORDER = 3,            /* Use border color */
} D3D8_VULKAN_SAMPLER_ADDRESS_MODE;

/**
 * @enum D3D8_VULKAN_SAMPLER_FILTER
 * @brief Texture filtering mode
 */
typedef enum {
    D3D8_VULKAN_SAMPLER_FILTER_NEAREST = 0,            /* Point sampling */
    D3D8_VULKAN_SAMPLER_FILTER_LINEAR = 1,             /* Bilinear */
    D3D8_VULKAN_SAMPLER_FILTER_CUBIC = 2,              /* Cubic */
} D3D8_VULKAN_SAMPLER_FILTER;

/* ============================================================================
 * Configuration Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_SAMPLER_CREATE_INFO
 * @brief Sampler configuration
 */
typedef struct {
    D3D8_VULKAN_SAMPLER_FILTER mag_filter;             /* Magnification */
    D3D8_VULKAN_SAMPLER_FILTER min_filter;             /* Minification */
    D3D8_VULKAN_SAMPLER_FILTER mipmap_filter;          /* Mipmap filter */
    D3D8_VULKAN_SAMPLER_ADDRESS_MODE address_u;        /* U wrapping */
    D3D8_VULKAN_SAMPLER_ADDRESS_MODE address_v;        /* V wrapping */
    D3D8_VULKAN_SAMPLER_ADDRESS_MODE address_w;        /* W wrapping */
    float lod_bias;                                    /* LOD bias */
    float min_lod;                                     /* Minimum LOD */
    float max_lod;                                     /* Maximum LOD */
    float max_anisotropy;                              /* Anisotropic level */
    int compare_enable;                                /* Comparison enabled */
    uint32_t border_color;                             /* Border color (RGBA8) */
} D3D8_VULKAN_SAMPLER_CREATE_INFO;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_BINDING
 * @brief Descriptor binding specification
 */
typedef struct {
    uint32_t binding;                                  /* Binding index */
    D3D8_VULKAN_DESCRIPTOR_TYPE descriptor_type;       /* Resource type */
    D3D8_VULKAN_SHADER_STAGE shader_stage;             /* Shader stage visibility */
    uint32_t descriptor_count;                         /* Count for arrays */
} D3D8_VULKAN_DESCRIPTOR_BINDING;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
 * @brief Descriptor set layout configuration
 */
typedef struct {
    D3D8_VULKAN_DESCRIPTOR_BINDING* bindings;          /* Binding array */
    uint32_t binding_count;                            /* Number of bindings */
} D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_POOL_CREATE_INFO
 * @brief Descriptor pool allocation parameters
 */
typedef struct {
    uint32_t sampler_count;                            /* Max samplers */
    uint32_t sampled_image_count;                      /* Max sampled images */
    uint32_t storage_image_count;                      /* Max storage images */
    uint32_t uniform_buffer_count;                     /* Max uniform buffers */
    uint32_t storage_buffer_count;                     /* Max storage buffers */
    uint32_t combined_sampler_count;                   /* Max combined samplers */
    uint32_t max_sets;                                 /* Max descriptor sets */
} D3D8_VULKAN_DESCRIPTOR_POOL_CREATE_INFO;

/**
 * @struct D3D8_VULKAN_WRITE_DESCRIPTOR_SET
 * @brief Descriptor set write operation
 */
typedef struct {
    VkDescriptorSet dst_set;                           /* Destination set */
    uint32_t dst_binding;                              /* Destination binding */
    uint32_t dst_array_element;                        /* Array element */
    D3D8_VULKAN_DESCRIPTOR_TYPE descriptor_type;       /* Type */
    uint32_t descriptor_count;                         /* Count */
    /* Pointers to resource data (sampler, image, or buffer) */
    void* descriptor_data;                             /* Raw descriptor data */
} D3D8_VULKAN_WRITE_DESCRIPTOR_SET;

/* ============================================================================
 * Handle Types
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_SAMPLER_HANDLE
 * @brief Opaque sampler handle
 */
typedef struct {
    VkSampler sampler;                                 /* Vulkan sampler */
    uint32_t sampler_id;                               /* Unique ID */
} D3D8_VULKAN_SAMPLER_HANDLE;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE
 * @brief Opaque descriptor set layout handle
 */
typedef struct {
    VkDescriptorSetLayout layout;                      /* Vulkan layout */
    uint32_t layout_id;                                /* Unique ID */
    uint32_t binding_count;                            /* Binding count */
} D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE
 * @brief Opaque descriptor pool handle
 */
typedef struct {
    VkDescriptorPool pool;                             /* Vulkan pool */
    uint32_t pool_id;                                  /* Unique ID */
    uint32_t sets_allocated;                           /* Sets allocated */
    uint32_t max_sets;                                 /* Max sets */
} D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE;

/**
 * @struct D3D8_VULKAN_DESCRIPTOR_SET_HANDLE
 * @brief Opaque descriptor set handle
 */
typedef struct {
    VkDescriptorSet set;                               /* Vulkan set */
    uint32_t set_id;                                   /* Unique ID */
    VkDescriptorSetLayout layout;                      /* Associated layout */
} D3D8_VULKAN_DESCRIPTOR_SET_HANDLE;

/* ============================================================================
 * API Functions - Samplers
 * ============================================================================ */

/**
 * @brief Create sampler with filtering and addressing options
 * @param device Vulkan device
 * @param create_info Sampler configuration
 * @param out_handle Pointer to receive sampler handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateSampler(VkDevice device, const D3D8_VULKAN_SAMPLER_CREATE_INFO* create_info,
                              D3D8_VULKAN_SAMPLER_HANDLE* out_handle);

/**
 * @brief Destroy sampler
 * @param device Vulkan device
 * @param handle Sampler handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_DestroySampler(VkDevice device, D3D8_VULKAN_SAMPLER_HANDLE* handle);

/* ============================================================================
 * API Functions - Descriptor Set Layouts
 * ============================================================================ */

/**
 * @brief Create descriptor set layout
 * @param device Vulkan device
 * @param create_info Layout configuration
 * @param out_handle Pointer to receive layout handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateDescriptorSetLayout(VkDevice device,
                                          const D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO* create_info,
                                          D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* out_handle);

/**
 * @brief Destroy descriptor set layout
 * @param device Vulkan device
 * @param handle Layout handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_DestroyDescriptorSetLayout(VkDevice device, D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* handle);

/* ============================================================================
 * API Functions - Descriptor Pools
 * ============================================================================ */

/**
 * @brief Create descriptor pool
 * @param device Vulkan device
 * @param create_info Pool configuration
 * @param out_handle Pointer to receive pool handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_CreateDescriptorPool(VkDevice device, const D3D8_VULKAN_DESCRIPTOR_POOL_CREATE_INFO* create_info,
                                     D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* out_handle);

/**
 * @brief Destroy descriptor pool
 * @param device Vulkan device
 * @param handle Pool handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_DestroyDescriptorPool(VkDevice device, D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* handle);

/**
 * @brief Reset descriptor pool (free all allocated sets)
 * @param device Vulkan device
 * @param pool_handle Pool handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_ResetDescriptorPool(VkDevice device, D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle);

/* ============================================================================
 * API Functions - Descriptor Sets
 * ============================================================================ */

/**
 * @brief Allocate descriptor set from pool
 * @param device Vulkan device
 * @param pool_handle Descriptor pool handle
 * @param layout_handle Descriptor set layout handle
 * @param out_handle Pointer to receive descriptor set handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_AllocateDescriptorSet(VkDevice device,
                                      D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                      D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* layout_handle,
                                      D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* out_handle);

/**
 * @brief Allocate multiple descriptor sets
 * @param device Vulkan device
 * @param pool_handle Descriptor pool handle
 * @param layout_handle Descriptor set layout handle
 * @param count Number of sets to allocate
 * @param out_handles Array to receive descriptor set handles
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_AllocateDescriptorSets(VkDevice device,
                                       D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                       D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* layout_handle,
                                       uint32_t count, D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* out_handles);

/**
 * @brief Free descriptor set
 * @param device Vulkan device
 * @param pool_handle Descriptor pool handle
 * @param set_handle Descriptor set handle
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_FreeDescriptorSet(VkDevice device,
                                  D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                  D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* set_handle);

/**
 * @brief Update descriptor set with new resources
 * @param device Vulkan device
 * @param write_descriptor_set Write operation specification
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_UpdateDescriptorSet(VkDevice device, const D3D8_VULKAN_WRITE_DESCRIPTOR_SET* write_descriptor_set);

/**
 * @brief Update multiple descriptor sets
 * @param device Vulkan device
 * @param writes Array of write operations
 * @param write_count Number of write operations
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_UpdateDescriptorSets(VkDevice device, const D3D8_VULKAN_WRITE_DESCRIPTOR_SET* writes,
                                     uint32_t write_count);

/**
 * @brief Bind descriptor set to specific binding point
 * @param device Vulkan device
 * @param set_handle Descriptor set handle
 * @param binding_point Pipeline binding point (0 = graphics, 1 = compute)
 * @param first_set First set index
 * @return 0 on success, non-zero on failure
 */
int D3D8_Vulkan_BindDescriptorSet(VkDevice device, D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* set_handle,
                                  uint32_t binding_point, uint32_t first_set);

/* ============================================================================
 * API Functions - Sampler Presets
 * ============================================================================ */

/**
 * @brief Get standard sampler config (point, clamp)
 * @param out_config Pointer to receive configuration
 */
void D3D8_Vulkan_GetSamplerPreset_PointClamp(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config);

/**
 * @brief Get standard sampler config (linear, repeat)
 * @param out_config Pointer to receive configuration
 */
void D3D8_Vulkan_GetSamplerPreset_LinearRepeat(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config);

/**
 * @brief Get standard sampler config (anisotropic, mirror)
 * @param out_config Pointer to receive configuration
 */
void D3D8_Vulkan_GetSamplerPreset_AnisotropicMirror(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_DESCRIPTOR_H_ */
