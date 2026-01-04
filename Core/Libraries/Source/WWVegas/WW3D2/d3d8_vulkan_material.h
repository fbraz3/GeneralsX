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
**  FILE: d3d8_vulkan_material.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Material System - Descriptor Binding & State Management
**
**  DESCRIPTION:
**    Provides material definition, descriptor set binding, and material caching
**    for efficient GPU batching. Integrates with Phase 12 (textures) and Phase 13
**    (descriptor sets/samplers) to enable complete material pipelines.
**
**    Phase 15: Material System Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_MATERIAL_H_
#define D3D8_VULKAN_MATERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Forward Declarations (to avoid including Vulkan/texture/descriptor headers)
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkCommandBuffer;
typedef void* VkPipeline;
typedef void* VkPipelineLayout;
typedef void* VkDescriptorSet;
typedef unsigned int uint32_t;
// typedef unsigned long long uint64_t;
typedef unsigned char uint8_t;
typedef int int32_t;
typedef float float32_t;

/* Opaque handles from Phase 12 (Textures) and Phase 13 (Descriptors) */
typedef struct _D3D8_VULKAN_TEXTURE_HANDLE* D3D8_VULKAN_TEXTURE_HANDLE;
typedef struct _D3D8_VULKAN_SAMPLER_HANDLE* D3D8_VULKAN_SAMPLER_HANDLE;
typedef struct _D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* D3D8_VULKAN_DESCRIPTOR_SET_HANDLE;
typedef struct _D3D8_VULKAN_DESCRIPTOR_LAYOUT_HANDLE* D3D8_VULKAN_DESCRIPTOR_LAYOUT_HANDLE;

/* ============================================================================
 * Material Property Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_MATERIAL_PROPERTY_TYPE
 * @brief Material property types (shader constants)
 */
typedef enum {
    D3D8_VULKAN_PROP_AMBIENT = 0x1,        /* RGBA ambient color */
    D3D8_VULKAN_PROP_DIFFUSE = 0x2,        /* RGBA diffuse color */
    D3D8_VULKAN_PROP_SPECULAR = 0x3,       /* RGBA specular color */
    D3D8_VULKAN_PROP_EMISSIVE = 0x4,       /* RGBA emissive color */
    D3D8_VULKAN_PROP_SHININESS = 0x5,      /* Float shininess exponent */
    D3D8_VULKAN_PROP_ALPHA = 0x6,          /* Float alpha value */
    D3D8_VULKAN_PROP_REFLECTIVITY = 0x7,   /* Float reflectivity [0-1] */
    D3D8_VULKAN_PROP_ROUGHNESS = 0x8,      /* Float roughness [0-1] */
    D3D8_VULKAN_PROP_METALLIC = 0x9,       /* Float metallic [0-1] */
} D3D8_VULKAN_MATERIAL_PROPERTY_TYPE;

/**
 * @enum D3D8_VULKAN_BLENDING_MODE
 * @brief Material blending modes
 */
typedef enum {
    D3D8_VULKAN_BLEND_OPAQUE = 0x0,        /* No blending (fully opaque) */
    D3D8_VULKAN_BLEND_ALPHA = 0x1,         /* Alpha blending */
    D3D8_VULKAN_BLEND_ADDITIVE = 0x2,      /* Additive blending */
    D3D8_VULKAN_BLEND_MULTIPLY = 0x3,      /* Multiply blending */
    D3D8_VULKAN_BLEND_SCREEN = 0x4,        /* Screen blending */
} D3D8_VULKAN_BLENDING_MODE;

/**
 * @enum D3D8_VULKAN_MATERIAL_STATE
 * @brief Material lifecycle state
 */
typedef enum {
    D3D8_VULKAN_MATERIAL_STATE_UNINITIALIZED = 0x0,
    D3D8_VULKAN_MATERIAL_STATE_READY = 0x1,
    D3D8_VULKAN_MATERIAL_STATE_BOUND = 0x2,
    D3D8_VULKAN_MATERIAL_STATE_DESTROYED = 0x3,
} D3D8_VULKAN_MATERIAL_STATE;

/**
 * @enum D3D8_VULKAN_TEXTURE_SLOT
 * @brief Material texture binding slots (shader layout slots)
 */
typedef enum {
    D3D8_VULKAN_TEXTURE_SLOT_0_DIFFUSE = 0,         /* Diffuse/color map */
    D3D8_VULKAN_TEXTURE_SLOT_1_NORMAL = 1,          /* Normal map */
    D3D8_VULKAN_TEXTURE_SLOT_2_SPECULAR = 2,        /* Specular map */
    D3D8_VULKAN_TEXTURE_SLOT_3_EMISSIVE = 3,        /* Emissive/glow map */
    D3D8_VULKAN_TEXTURE_SLOT_4_HEIGHTMAP = 4,       /* Height/parallax map */
    D3D8_VULKAN_TEXTURE_SLOT_5_ENVIRONMENT = 5,     /* Cubemap/environment */
    D3D8_VULKAN_TEXTURE_SLOT_MAX = 6,               /* Maximum slots */
} D3D8_VULKAN_TEXTURE_SLOT;

/* ============================================================================
 * Material Configuration & Handle Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_MATERIAL_TEXTURE_BINDING
 * @brief Texture binding for a material slot
 */
typedef struct {
    uint32_t slot;                                      /* Texture slot (0-5) */
    D3D8_VULKAN_TEXTURE_HANDLE texture_handle;        /* Texture from Phase 12 */
    D3D8_VULKAN_SAMPLER_HANDLE sampler_handle;        /* Sampler from Phase 13 */
    uint32_t binding_point;                            /* Shader binding point */
} D3D8_VULKAN_MATERIAL_TEXTURE_BINDING;

/**
 * @struct D3D8_VULKAN_MATERIAL_PROPERTY
 * @brief Material property value
 */
typedef struct {
    D3D8_VULKAN_MATERIAL_PROPERTY_TYPE type;
    union {
        struct { float32_t r, g, b, a; } color;       /* RGBA color properties */
        float32_t scalar;                              /* Scalar properties */
    } value;
} D3D8_VULKAN_MATERIAL_PROPERTY;

/**
 * @struct D3D8_VULKAN_MATERIAL_CREATE_INFO
 * @brief Material creation parameters
 */
typedef struct {
    const char* name;                                   /* Material name/identifier */
    D3D8_VULKAN_BLENDING_MODE blending_mode;          /* Alpha blending mode */
    uint32_t num_textures;                            /* Number of texture bindings */
    D3D8_VULKAN_MATERIAL_TEXTURE_BINDING* textures;   /* Texture array pointer */
    uint32_t num_properties;                          /* Number of properties */
    D3D8_VULKAN_MATERIAL_PROPERTY* properties;        /* Property array pointer */
    D3D8_VULKAN_DESCRIPTOR_LAYOUT_HANDLE layout_handle;  /* Descriptor layout from Phase 13 */
    uint32_t flags;                                   /* Material flags (reserved) */
} D3D8_VULKAN_MATERIAL_CREATE_INFO;

/**
 * @struct D3D8_VULKAN_MATERIAL_HANDLE
 * @brief Opaque material handle (returned to caller)
 */
typedef struct {
    uint32_t id;                                       /* Unique material ID */
    uint32_t version;                                  /* Modification counter */
} D3D8_VULKAN_MATERIAL_HANDLE;

/**
 * @struct D3D8_VULKAN_MATERIAL_INFO
 * @brief Material information retrieval structure
 */
typedef struct {
    char name[128];                                    /* Material name */
    D3D8_VULKAN_MATERIAL_STATE state;                 /* Current state */
    D3D8_VULKAN_BLENDING_MODE blending_mode;          /* Blending mode */
    uint32_t num_textures;                            /* Bound textures count */
    uint32_t num_properties;                          /* Property count */
    uint64_t descriptor_set_handle;                   /* Associated descriptor set */
    uint32_t reference_count;                         /* Usage reference count */
} D3D8_VULKAN_MATERIAL_INFO;

/* ============================================================================
 * Material Management API (16 Functions)
 * ============================================================================ */

/**
 * @fn D3D8_VULKAN_MATERIAL_HANDLE D3D8_Vulkan_CreateMaterial(VkDevice device, const D3D8_VULKAN_MATERIAL_CREATE_INFO* create_info)
 * @brief Create a new material with textures and properties
 * @param device Vulkan logical device
 * @param create_info Material configuration
 * @return Opaque material handle
 */
D3D8_VULKAN_MATERIAL_HANDLE D3D8_Vulkan_CreateMaterial(
    VkDevice device,
    const D3D8_VULKAN_MATERIAL_CREATE_INFO* create_info
);

/**
 * @fn void D3D8_Vulkan_DestroyMaterial(VkDevice device, D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Destroy a material and free resources
 * @param device Vulkan logical device
 * @param material Material handle to destroy
 */
void D3D8_Vulkan_DestroyMaterial(
    VkDevice device,
    D3D8_VULKAN_MATERIAL_HANDLE material
);

/**
 * @fn int D3D8_Vulkan_BindMaterial(VkCommandBuffer cmd_buffer, VkPipelineLayout layout, D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Bind material descriptor sets and state to command buffer
 * @param cmd_buffer Vulkan command buffer
 * @param layout Pipeline layout for descriptor binding
 * @param material Material handle to bind
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_BindMaterial(
    VkCommandBuffer cmd_buffer,
    VkPipelineLayout layout,
    D3D8_VULKAN_MATERIAL_HANDLE material
);

/**
 * @fn int D3D8_Vulkan_UnbindMaterial(VkCommandBuffer cmd_buffer, D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Unbind material and cleanup state
 * @param cmd_buffer Vulkan command buffer
 * @param material Material handle to unbind
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_UnbindMaterial(
    VkCommandBuffer cmd_buffer,
    D3D8_VULKAN_MATERIAL_HANDLE material
);

/**
 * @fn int D3D8_Vulkan_UpdateMaterialTexture(D3D8_VULKAN_MATERIAL_HANDLE material, uint32_t slot, D3D8_VULKAN_TEXTURE_HANDLE texture, D3D8_VULKAN_SAMPLER_HANDLE sampler)
 * @brief Update a material's texture binding
 * @param material Material handle
 * @param slot Texture slot (0-5)
 * @param texture Texture handle from Phase 12
 * @param sampler Sampler handle from Phase 13
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_UpdateMaterialTexture(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    uint32_t slot,
    D3D8_VULKAN_TEXTURE_HANDLE texture,
    D3D8_VULKAN_SAMPLER_HANDLE sampler
);

/**
 * @fn int D3D8_Vulkan_UpdateMaterialProperty(D3D8_VULKAN_MATERIAL_HANDLE material, const D3D8_VULKAN_MATERIAL_PROPERTY* property)
 * @brief Update a material property (color, scalar, etc.)
 * @param material Material handle
 * @param property Property to update
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_UpdateMaterialProperty(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    const D3D8_VULKAN_MATERIAL_PROPERTY* property
);

/**
 * @fn int D3D8_Vulkan_GetMaterialTexture(D3D8_VULKAN_MATERIAL_HANDLE material, uint32_t slot, D3D8_VULKAN_TEXTURE_HANDLE* out_texture, D3D8_VULKAN_SAMPLER_HANDLE* out_sampler)
 * @brief Retrieve texture binding from material
 * @param material Material handle
 * @param slot Texture slot (0-5)
 * @param out_texture Pointer to receive texture handle
 * @param out_sampler Pointer to receive sampler handle
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_GetMaterialTexture(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    uint32_t slot,
    D3D8_VULKAN_TEXTURE_HANDLE* out_texture,
    D3D8_VULKAN_SAMPLER_HANDLE* out_sampler
);

/**
 * @fn int D3D8_Vulkan_GetMaterialProperty(D3D8_VULKAN_MATERIAL_HANDLE material, D3D8_VULKAN_MATERIAL_PROPERTY_TYPE type, D3D8_VULKAN_MATERIAL_PROPERTY* out_property)
 * @brief Retrieve material property value
 * @param material Material handle
 * @param type Property type to retrieve
 * @param out_property Pointer to receive property
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_GetMaterialProperty(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    D3D8_VULKAN_MATERIAL_PROPERTY_TYPE type,
    D3D8_VULKAN_MATERIAL_PROPERTY* out_property
);

/**
 * @fn int D3D8_Vulkan_GetMaterialInfo(D3D8_VULKAN_MATERIAL_HANDLE material, D3D8_VULKAN_MATERIAL_INFO* out_info)
 * @brief Retrieve complete material information
 * @param material Material handle
 * @param out_info Pointer to receive material info
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_GetMaterialInfo(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    D3D8_VULKAN_MATERIAL_INFO* out_info
);

/**
 * @fn uint32_t D3D8_Vulkan_GetMaterialCacheStats(uint32_t* out_total_materials, uint32_t* out_used_materials, uint32_t* out_free_slots)
 * @brief Get material cache statistics
 * @param out_total_materials Pointer to receive total material count
 * @param out_used_materials Pointer to receive used count
 * @param out_free_slots Pointer to receive free slots
 * @return Cache utilization percentage (0-100)
 */
uint32_t D3D8_Vulkan_GetMaterialCacheStats(
    uint32_t* out_total_materials,
    uint32_t* out_used_materials,
    uint32_t* out_free_slots
);

/**
 * @fn void D3D8_Vulkan_ClearMaterialCache(VkDevice device)
 * @brief Clear all materials from cache (call at shutdown)
 * @param device Vulkan logical device
 */
void D3D8_Vulkan_ClearMaterialCache(VkDevice device);

/**
 * @fn int D3D8_Vulkan_BatchBindMaterials(VkCommandBuffer cmd_buffer, VkPipelineLayout layout, const D3D8_VULKAN_MATERIAL_HANDLE* materials, uint32_t count)
 * @brief Efficiently bind multiple materials for batching
 * @param cmd_buffer Vulkan command buffer
 * @param layout Pipeline layout
 * @param materials Array of material handles
 * @param count Number of materials to bind
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_BatchBindMaterials(
    VkCommandBuffer cmd_buffer,
    VkPipelineLayout layout,
    const D3D8_VULKAN_MATERIAL_HANDLE* materials,
    uint32_t count
);

/**
 * @fn void D3D8_Vulkan_IncrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Increment material reference count (for cache management)
 * @param material Material handle
 */
void D3D8_Vulkan_IncrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material);

/**
 * @fn void D3D8_Vulkan_DecrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Decrement material reference count
 * @param material Material handle
 */
void D3D8_Vulkan_DecrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material);

/**
 * @fn int D3D8_Vulkan_ValidateMaterialHandle(D3D8_VULKAN_MATERIAL_HANDLE material)
 * @brief Check if a material handle is valid
 * @param material Material handle to validate
 * @return 1 if valid, 0 if invalid
 */
int D3D8_Vulkan_ValidateMaterialHandle(D3D8_VULKAN_MATERIAL_HANDLE material);

/**
 * @fn const char* D3D8_Vulkan_GetMaterialError(void)
 * @brief Get last material system error message
 * @return Error message string (static buffer, valid until next call)
 */
const char* D3D8_Vulkan_GetMaterialError(void);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_MATERIAL_H_ */
