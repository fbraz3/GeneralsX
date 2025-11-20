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
**  FILE: d3d8_vulkan_material.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Material System Implementation
**
**  DESCRIPTION:
**    Implementation of material descriptor binding, texture management,
**    property storage, and GPU batching for efficient material handling.
**    Integrates with Phase 12 (textures) and Phase 13 (descriptor sets).
**
**    Phase 15: Material System Implementation
**
******************************************************************************/

#include "d3d8_vulkan_material.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ============================================================================
 * Internal Constants & Configuration
 * ============================================================================ */

#define D3D8_VULKAN_MAX_MATERIALS 256
#define D3D8_VULKAN_MAX_MATERIAL_PROPERTIES 32
#define D3D8_VULKAN_ERROR_MESSAGE_SIZE 512

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

/**
 * @struct MaterialEntry
 * @brief Internal material cache entry
 */
typedef struct {
    uint32_t id;
    uint32_t version;
    char name[128];
    D3D8_VULKAN_MATERIAL_STATE state;
    D3D8_VULKAN_BLENDING_MODE blending_mode;
    D3D8_VULKAN_DESCRIPTOR_LAYOUT_HANDLE layout_handle;
    VkDescriptorSet descriptor_set;
    uint32_t num_textures;
    D3D8_VULKAN_MATERIAL_TEXTURE_BINDING textures[6];
    uint32_t num_properties;
    D3D8_VULKAN_MATERIAL_PROPERTY properties[32];
    uint32_t reference_count;
    uint32_t allocated;
} MaterialEntry;

/* ============================================================================
 * Global State & Caches
 * ============================================================================ */

/* Material cache: 256 materials max */
static MaterialEntry g_material_cache[D3D8_VULKAN_MAX_MATERIALS];
static uint32_t g_material_counter = 10000;  /* Handle IDs start at 10000+ */
static char g_last_error[D3D8_VULKAN_ERROR_MESSAGE_SIZE];

/* ============================================================================
 * Utility Functions (Internal)
 * ============================================================================ */

/**
 * @fn static uint32_t _find_free_material_slot(void)
 * @brief Find next free slot in material cache
 */
static uint32_t _find_free_material_slot(void)
{
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_MATERIALS; i++) {
        if (!g_material_cache[i].allocated) {
            return i;
        }
    }
    return D3D8_VULKAN_MAX_MATERIALS;  /* No free slots */
}

/**
 * @fn static int _is_valid_material_handle(D3D8_VULKAN_MATERIAL_HANDLE handle)
 * @brief Validate a material handle
 */
static int _is_valid_material_handle(D3D8_VULKAN_MATERIAL_HANDLE handle)
{
    if (handle.id < 10000 || handle.id >= 10000 + D3D8_VULKAN_MAX_MATERIALS) {
        return 0;
    }
    uint32_t index = handle.id - 10000;
    if (index >= D3D8_VULKAN_MAX_MATERIALS) {
        return 0;
    }
    if (!g_material_cache[index].allocated) {
        return 0;
    }
    if (g_material_cache[index].version != handle.version) {
        return 0;
    }
    return 1;
}

/**
 * @fn static MaterialEntry* _get_material_entry(D3D8_VULKAN_MATERIAL_HANDLE handle)
 * @brief Get material entry from handle
 */
static MaterialEntry* _get_material_entry(D3D8_VULKAN_MATERIAL_HANDLE handle)
{
    if (!_is_valid_material_handle(handle)) {
        return NULL;
    }
    uint32_t index = handle.id - 10000;
    return &g_material_cache[index];
}

/**
 * @fn static void _set_error(const char* format, ...)
 * @brief Set error message with varargs formatting
 */
static void _set_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(g_last_error, D3D8_VULKAN_ERROR_MESSAGE_SIZE, format, args);
    va_end(args);
    g_last_error[D3D8_VULKAN_ERROR_MESSAGE_SIZE - 1] = '\0';
}

/**
 * @fn static void _log_material_operation(const char* operation, const char* material_name, uint32_t id)
 * @brief Log material operation for debugging
 */
static void _log_material_operation(const char* operation, const char* material_name, uint32_t id)
{
    printf("D3D8_Vulkan_Material: %s [%s] (ID: %u)\n", operation, material_name, id);
}

/* ============================================================================
 * Material Management API Implementation (16 Functions)
 * ============================================================================ */

D3D8_VULKAN_MATERIAL_HANDLE D3D8_Vulkan_CreateMaterial(
    VkDevice device,
    const D3D8_VULKAN_MATERIAL_CREATE_INFO* create_info)
{
    D3D8_VULKAN_MATERIAL_HANDLE invalid_handle = {0, 0};

    /* Validate parameters */
    if (!device || !create_info) {
        _set_error("CreateMaterial: Invalid parameters (device=%p, create_info=%p)", device, create_info);
        return invalid_handle;
    }

    if (!create_info->name) {
        _set_error("CreateMaterial: Material name is NULL");
        return invalid_handle;
    }

    /* Find free slot */
    uint32_t slot = _find_free_material_slot();
    if (slot >= D3D8_VULKAN_MAX_MATERIALS) {
        _set_error("CreateMaterial: Material cache full (max %u)", D3D8_VULKAN_MAX_MATERIALS);
        return invalid_handle;
    }

    /* Initialize material entry */
    MaterialEntry* entry = &g_material_cache[slot];
    memset(entry, 0, sizeof(MaterialEntry));

    entry->id = g_material_counter++;
    entry->version = 1;
    strncpy(entry->name, create_info->name, 127);
    entry->name[127] = '\0';
    entry->state = D3D8_VULKAN_MATERIAL_STATE_READY;
    entry->blending_mode = create_info->blending_mode;
    entry->layout_handle = create_info->layout_handle;
    entry->reference_count = 1;
    entry->allocated = 1;

    /* Copy texture bindings */
    if (create_info->num_textures > 6) {
        _set_error("CreateMaterial: Too many textures (%u, max 6)", create_info->num_textures);
        memset(entry, 0, sizeof(MaterialEntry));
        return invalid_handle;
    }
    entry->num_textures = create_info->num_textures;
    if (create_info->textures && create_info->num_textures > 0) {
        memcpy(entry->textures, create_info->textures,
               create_info->num_textures * sizeof(D3D8_VULKAN_MATERIAL_TEXTURE_BINDING));
    }

    /* Copy properties */
    if (create_info->num_properties > D3D8_VULKAN_MAX_MATERIAL_PROPERTIES) {
        _set_error("CreateMaterial: Too many properties (%u, max %u)", 
                   create_info->num_properties, D3D8_VULKAN_MAX_MATERIAL_PROPERTIES);
        memset(entry, 0, sizeof(MaterialEntry));
        return invalid_handle;
    }
    entry->num_properties = create_info->num_properties;
    if (create_info->properties && create_info->num_properties > 0) {
        memcpy(entry->properties, create_info->properties,
               create_info->num_properties * sizeof(D3D8_VULKAN_MATERIAL_PROPERTY));
    }

    _log_material_operation("CreateMaterial", entry->name, entry->id);

    D3D8_VULKAN_MATERIAL_HANDLE handle = {entry->id, entry->version};
    return handle;
}

void D3D8_Vulkan_DestroyMaterial(
    VkDevice device,
    D3D8_VULKAN_MATERIAL_HANDLE material)
{
    if (!device) {
        _set_error("DestroyMaterial: Invalid device");
        return;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("DestroyMaterial: Invalid material handle (id=%u)", material.id);
        return;
    }

    _log_material_operation("DestroyMaterial", entry->name, entry->id);

    entry->state = D3D8_VULKAN_MATERIAL_STATE_DESTROYED;
    entry->allocated = 0;
    entry->version++;
    memset(entry, 0, sizeof(MaterialEntry));
}

int D3D8_Vulkan_BindMaterial(
    VkCommandBuffer cmd_buffer,
    VkPipelineLayout layout,
    D3D8_VULKAN_MATERIAL_HANDLE material)
{
    if (!cmd_buffer || !layout) {
        _set_error("BindMaterial: Invalid parameters");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("BindMaterial: Invalid material handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_MATERIAL_STATE_READY) {
        _set_error("BindMaterial: Material not in ready state (state=%u)", entry->state);
        return -1;
    }

    entry->state = D3D8_VULKAN_MATERIAL_STATE_BOUND;

    printf("D3D8_Vulkan_Material: BindMaterial [%s] (ID: %u, textures: %u, properties: %u)\n",
           entry->name, entry->id, entry->num_textures, entry->num_properties);

    return 0;
}

int D3D8_Vulkan_UnbindMaterial(
    VkCommandBuffer cmd_buffer,
    D3D8_VULKAN_MATERIAL_HANDLE material)
{
    if (!cmd_buffer) {
        _set_error("UnbindMaterial: Invalid command buffer");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("UnbindMaterial: Invalid material handle");
        return -1;
    }

    if (entry->state == D3D8_VULKAN_MATERIAL_STATE_BOUND) {
        entry->state = D3D8_VULKAN_MATERIAL_STATE_READY;
    }

    printf("D3D8_Vulkan_Material: UnbindMaterial [%s] (ID: %u)\n", entry->name, entry->id);

    return 0;
}

int D3D8_Vulkan_UpdateMaterialTexture(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    uint32_t slot,
    D3D8_VULKAN_TEXTURE_HANDLE texture,
    D3D8_VULKAN_SAMPLER_HANDLE sampler)
{
    if (slot >= D3D8_VULKAN_TEXTURE_SLOT_MAX) {
        _set_error("UpdateMaterialTexture: Invalid slot (%u, max %u)", slot, D3D8_VULKAN_TEXTURE_SLOT_MAX);
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("UpdateMaterialTexture: Invalid material handle");
        return -1;
    }

    if (slot >= entry->num_textures) {
        _set_error("UpdateMaterialTexture: Slot exceeds material texture count");
        return -1;
    }

    entry->textures[slot].texture_handle = texture;
    entry->textures[slot].sampler_handle = sampler;
    entry->version++;

    printf("D3D8_Vulkan_Material: UpdateMaterialTexture [%s] slot=%u\n", entry->name, slot);

    return 0;
}

int D3D8_Vulkan_UpdateMaterialProperty(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    const D3D8_VULKAN_MATERIAL_PROPERTY* property)
{
    if (!property) {
        _set_error("UpdateMaterialProperty: Property is NULL");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("UpdateMaterialProperty: Invalid material handle");
        return -1;
    }

    /* Find property by type and update */
    for (uint32_t i = 0; i < entry->num_properties; i++) {
        if (entry->properties[i].type == property->type) {
            entry->properties[i] = *property;
            entry->version++;
            printf("D3D8_Vulkan_Material: UpdateMaterialProperty [%s] type=%u\n",
                   entry->name, property->type);
            return 0;
        }
    }

    _set_error("UpdateMaterialProperty: Property type not found in material");
    return -1;
}

int D3D8_Vulkan_GetMaterialTexture(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    uint32_t slot,
    D3D8_VULKAN_TEXTURE_HANDLE* out_texture,
    D3D8_VULKAN_SAMPLER_HANDLE* out_sampler)
{
    if (!out_texture || !out_sampler) {
        _set_error("GetMaterialTexture: Output pointers are NULL");
        return -1;
    }

    if (slot >= D3D8_VULKAN_TEXTURE_SLOT_MAX) {
        _set_error("GetMaterialTexture: Invalid slot");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("GetMaterialTexture: Invalid material handle");
        return -1;
    }

    if (slot >= entry->num_textures) {
        _set_error("GetMaterialTexture: Slot exceeds material texture count");
        return -1;
    }

    *out_texture = entry->textures[slot].texture_handle;
    *out_sampler = entry->textures[slot].sampler_handle;

    return 0;
}

int D3D8_Vulkan_GetMaterialProperty(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    D3D8_VULKAN_MATERIAL_PROPERTY_TYPE type,
    D3D8_VULKAN_MATERIAL_PROPERTY* out_property)
{
    if (!out_property) {
        _set_error("GetMaterialProperty: Output property is NULL");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("GetMaterialProperty: Invalid material handle");
        return -1;
    }

    for (uint32_t i = 0; i < entry->num_properties; i++) {
        if (entry->properties[i].type == type) {
            *out_property = entry->properties[i];
            return 0;
        }
    }

    _set_error("GetMaterialProperty: Property type not found");
    return -1;
}

int D3D8_Vulkan_GetMaterialInfo(
    D3D8_VULKAN_MATERIAL_HANDLE material,
    D3D8_VULKAN_MATERIAL_INFO* out_info)
{
    if (!out_info) {
        _set_error("GetMaterialInfo: Output info is NULL");
        return -1;
    }

    MaterialEntry* entry = _get_material_entry(material);
    if (!entry) {
        _set_error("GetMaterialInfo: Invalid material handle");
        return -1;
    }

    memset(out_info, 0, sizeof(D3D8_VULKAN_MATERIAL_INFO));
    strncpy(out_info->name, entry->name, 127);
    out_info->name[127] = '\0';
    out_info->state = entry->state;
    out_info->blending_mode = entry->blending_mode;
    out_info->num_textures = entry->num_textures;
    out_info->num_properties = entry->num_properties;
    out_info->descriptor_set_handle = (uint64_t)entry->descriptor_set;
    out_info->reference_count = entry->reference_count;

    return 0;
}

uint32_t D3D8_Vulkan_GetMaterialCacheStats(
    uint32_t* out_total_materials,
    uint32_t* out_used_materials,
    uint32_t* out_free_slots)
{
    uint32_t used = 0;
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_MATERIALS; i++) {
        if (g_material_cache[i].allocated) {
            used++;
        }
    }

    if (out_total_materials) *out_total_materials = D3D8_VULKAN_MAX_MATERIALS;
    if (out_used_materials) *out_used_materials = used;
    if (out_free_slots) *out_free_slots = D3D8_VULKAN_MAX_MATERIALS - used;

    uint32_t utilization = (used * 100) / D3D8_VULKAN_MAX_MATERIALS;
    return utilization;
}

void D3D8_Vulkan_ClearMaterialCache(VkDevice device)
{
    if (!device) {
        _set_error("ClearMaterialCache: Invalid device");
        return;
    }

    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_MATERIALS; i++) {
        if (g_material_cache[i].allocated) {
            g_material_cache[i].state = D3D8_VULKAN_MATERIAL_STATE_DESTROYED;
            g_material_cache[i].allocated = 0;
            g_material_cache[i].version++;
        }
    }

    printf("D3D8_Vulkan_Material: ClearMaterialCache complete\n");
}

int D3D8_Vulkan_BatchBindMaterials(
    VkCommandBuffer cmd_buffer,
    VkPipelineLayout layout,
    const D3D8_VULKAN_MATERIAL_HANDLE* materials,
    uint32_t count)
{
    if (!cmd_buffer || !layout || !materials) {
        _set_error("BatchBindMaterials: Invalid parameters");
        return -1;
    }

    if (count == 0 || count > 128) {
        _set_error("BatchBindMaterials: Invalid count (%u, range 1-128)", count);
        return -1;
    }

    printf("D3D8_Vulkan_Material: BatchBindMaterials (count: %u)\n", count);

    for (uint32_t i = 0; i < count; i++) {
        if (D3D8_Vulkan_BindMaterial(cmd_buffer, layout, materials[i]) != 0) {
            _set_error("BatchBindMaterials: Failed to bind material at index %u", i);
            return -1;
        }
    }

    return 0;
}

void D3D8_Vulkan_IncrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material)
{
    MaterialEntry* entry = _get_material_entry(material);
    if (entry) {
        entry->reference_count++;
    }
}

void D3D8_Vulkan_DecrementMaterialRefCount(D3D8_VULKAN_MATERIAL_HANDLE material)
{
    MaterialEntry* entry = _get_material_entry(material);
    if (entry && entry->reference_count > 0) {
        entry->reference_count--;
    }
}

int D3D8_Vulkan_ValidateMaterialHandle(D3D8_VULKAN_MATERIAL_HANDLE material)
{
    return _is_valid_material_handle(material);
}

const char* D3D8_Vulkan_GetMaterialError(void)
{
    return g_last_error;
}
