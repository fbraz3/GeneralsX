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
**  FILE: d3d8_vulkan_descriptor.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Descriptor Management Implementation
**
**  DESCRIPTION:
**    Stub implementation of Vulkan descriptor set, sampler, and resource
**    binding management. Provides validation layer and architectural
**    foundation for shader resource descriptors.
**
**    Phase 13: Sampler & Descriptor Sets Implementation
**
******************************************************************************/

#include "d3d8_vulkan_descriptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ============================================================================
 * Internal State Management
 * ============================================================================ */

#define MAX_SAMPLERS 256
#define MAX_DESCRIPTOR_SET_LAYOUTS 64
#define MAX_DESCRIPTOR_POOLS 32
#define MAX_DESCRIPTOR_SETS 512

/**
 * @struct SamplerEntry
 * @brief Internal sampler tracking
 */
typedef struct {
    void* sampler;
    uint32_t sampler_id;
    int is_allocated;
} SamplerEntry;

/**
 * @struct DescriptorSetLayoutEntry
 * @brief Internal layout tracking
 */
typedef struct {
    void* layout;
    uint32_t layout_id;
    uint32_t binding_count;
    int is_allocated;
} DescriptorSetLayoutEntry;

/**
 * @struct DescriptorPoolEntry
 * @brief Internal pool tracking
 */
typedef struct {
    void* pool;
    uint32_t pool_id;
    uint32_t sets_allocated;
    uint32_t max_sets;
    int is_allocated;
} DescriptorPoolEntry;

/**
 * @struct DescriptorSetEntry
 * @brief Internal descriptor set tracking
 */
typedef struct {
    void* set;
    uint32_t set_id;
    void* layout;
    int is_allocated;
} DescriptorSetEntry;

/* Global caches */
static SamplerEntry g_sampler_cache[MAX_SAMPLERS];
static uint32_t g_sampler_counter = 3000;

static DescriptorSetLayoutEntry g_layout_cache[MAX_DESCRIPTOR_SET_LAYOUTS];
static uint32_t g_layout_counter = 4000;

static DescriptorPoolEntry g_pool_cache[MAX_DESCRIPTOR_POOLS];
static uint32_t g_pool_counter = 5000;

static DescriptorSetEntry g_descriptor_set_cache[MAX_DESCRIPTOR_SETS];
static uint32_t g_descriptor_set_counter = 6000;

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

static int _find_free_sampler_slot(void) {
    for (int i = 0; i < MAX_SAMPLERS; i++) {
        if (!g_sampler_cache[i].is_allocated) return i;
    }
    return -1;
}

static int _find_free_layout_slot(void) {
    for (int i = 0; i < MAX_DESCRIPTOR_SET_LAYOUTS; i++) {
        if (!g_layout_cache[i].is_allocated) return i;
    }
    return -1;
}

static int _find_free_pool_slot(void) {
    for (int i = 0; i < MAX_DESCRIPTOR_POOLS; i++) {
        if (!g_pool_cache[i].is_allocated) return i;
    }
    return -1;
}

static int _find_free_descriptor_set_slot(void) {
    for (int i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
        if (!g_descriptor_set_cache[i].is_allocated) return i;
    }
    return -1;
}

/* ============================================================================
 * Sampler API
 * ============================================================================ */

int D3D8_Vulkan_CreateSampler(VkDevice device, const D3D8_VULKAN_SAMPLER_CREATE_INFO* create_info,
                              D3D8_VULKAN_SAMPLER_HANDLE* out_handle) {
    printf("[Phase 13] D3D8_Vulkan_CreateSampler called: mag=%d min=%d address_u=%d\n",
           create_info->mag_filter, create_info->min_filter, create_info->address_u);
    
    if (device == NULL || create_info == NULL || out_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    int slot = _find_free_sampler_slot();
    if (slot == -1) {
        printf("[Phase 13] ERROR: Sampler cache full\n");
        return -1;
    }
    
    uint32_t sampler_id = g_sampler_counter++;
    
    g_sampler_cache[slot].is_allocated = 1;
    g_sampler_cache[slot].sampler_id = sampler_id;
    g_sampler_cache[slot].sampler = (void*)(uintptr_t)(7000000 + sampler_id);
    
    out_handle->sampler = (VkSampler)g_sampler_cache[slot].sampler;
    out_handle->sampler_id = sampler_id;
    
    printf("[Phase 13] Sampler created: id=%u sampler=%p\n", sampler_id, out_handle->sampler);
    return 0;
}

int D3D8_Vulkan_DestroySampler(VkDevice device, D3D8_VULKAN_SAMPLER_HANDLE* handle) {
    printf("[Phase 13] D3D8_Vulkan_DestroySampler called\n");
    
    if (device == NULL || handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_SAMPLERS; i++) {
        if (g_sampler_cache[i].is_allocated && g_sampler_cache[i].sampler == handle->sampler) {
            g_sampler_cache[i].is_allocated = 0;
            printf("[Phase 13] Sampler destroyed: id=%u\n", handle->sampler_id);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Sampler not found\n");
    return -1;
}

/* ============================================================================
 * Descriptor Set Layout API
 * ============================================================================ */

int D3D8_Vulkan_CreateDescriptorSetLayout(VkDevice device,
                                          const D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO* create_info,
                                          D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* out_handle) {
    printf("[Phase 13] D3D8_Vulkan_CreateDescriptorSetLayout called: bindings=%u\n",
           create_info->binding_count);
    
    if (device == NULL || create_info == NULL || out_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (create_info->binding_count == 0 || create_info->bindings == NULL) {
        printf("[Phase 13] ERROR: No bindings specified\n");
        return -1;
    }
    
    int slot = _find_free_layout_slot();
    if (slot == -1) {
        printf("[Phase 13] ERROR: Layout cache full\n");
        return -1;
    }
    
    uint32_t layout_id = g_layout_counter++;
    
    g_layout_cache[slot].is_allocated = 1;
    g_layout_cache[slot].layout_id = layout_id;
    g_layout_cache[slot].binding_count = create_info->binding_count;
    g_layout_cache[slot].layout = (void*)(uintptr_t)(8000000 + layout_id);
    
    out_handle->layout = (VkDescriptorSetLayout)g_layout_cache[slot].layout;
    out_handle->layout_id = layout_id;
    out_handle->binding_count = create_info->binding_count;
    
    for (uint32_t i = 0; i < create_info->binding_count; i++) {
        printf("[Phase 13] Binding %u: type=%d stage=%d count=%u\n",
               create_info->bindings[i].binding, create_info->bindings[i].descriptor_type,
               create_info->bindings[i].shader_stage, create_info->bindings[i].descriptor_count);
    }
    
    printf("[Phase 13] Descriptor set layout created: id=%u\n", layout_id);
    return 0;
}

int D3D8_Vulkan_DestroyDescriptorSetLayout(VkDevice device, D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* handle) {
    printf("[Phase 13] D3D8_Vulkan_DestroyDescriptorSetLayout called\n");
    
    if (device == NULL || handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_DESCRIPTOR_SET_LAYOUTS; i++) {
        if (g_layout_cache[i].is_allocated && g_layout_cache[i].layout == handle->layout) {
            g_layout_cache[i].is_allocated = 0;
            printf("[Phase 13] Descriptor set layout destroyed: id=%u\n", handle->layout_id);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Layout not found\n");
    return -1;
}

/* ============================================================================
 * Descriptor Pool API
 * ============================================================================ */

int D3D8_Vulkan_CreateDescriptorPool(VkDevice device, const D3D8_VULKAN_DESCRIPTOR_POOL_CREATE_INFO* create_info,
                                     D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* out_handle) {
    printf("[Phase 13] D3D8_Vulkan_CreateDescriptorPool called: max_sets=%u\n", create_info->max_sets);
    
    if (device == NULL || create_info == NULL || out_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (create_info->max_sets == 0) {
        printf("[Phase 13] ERROR: Max sets must be > 0\n");
        return -1;
    }
    
    int slot = _find_free_pool_slot();
    if (slot == -1) {
        printf("[Phase 13] ERROR: Pool cache full\n");
        return -1;
    }
    
    uint32_t pool_id = g_pool_counter++;
    
    g_pool_cache[slot].is_allocated = 1;
    g_pool_cache[slot].pool_id = pool_id;
    g_pool_cache[slot].max_sets = create_info->max_sets;
    g_pool_cache[slot].sets_allocated = 0;
    g_pool_cache[slot].pool = (void*)(uintptr_t)(9000000 + pool_id);
    
    out_handle->pool = (VkDescriptorPool)g_pool_cache[slot].pool;
    out_handle->pool_id = pool_id;
    out_handle->max_sets = create_info->max_sets;
    out_handle->sets_allocated = 0;
    
    printf("[Phase 13] Descriptor pool created: id=%u max_sets=%u\n", pool_id, create_info->max_sets);
    return 0;
}

int D3D8_Vulkan_DestroyDescriptorPool(VkDevice device, D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* handle) {
    printf("[Phase 13] D3D8_Vulkan_DestroyDescriptorPool called\n");
    
    if (device == NULL || handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_DESCRIPTOR_POOLS; i++) {
        if (g_pool_cache[i].is_allocated && g_pool_cache[i].pool == handle->pool) {
            g_pool_cache[i].is_allocated = 0;
            printf("[Phase 13] Descriptor pool destroyed: id=%u\n", handle->pool_id);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Pool not found\n");
    return -1;
}

int D3D8_Vulkan_ResetDescriptorPool(VkDevice device, D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle) {
    printf("[Phase 13] D3D8_Vulkan_ResetDescriptorPool called\n");
    
    if (device == NULL || pool_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_DESCRIPTOR_POOLS; i++) {
        if (g_pool_cache[i].is_allocated && g_pool_cache[i].pool == pool_handle->pool) {
            uint32_t freed_sets = g_pool_cache[i].sets_allocated;
            g_pool_cache[i].sets_allocated = 0;
            pool_handle->sets_allocated = 0;
            
            printf("[Phase 13] Descriptor pool reset: freed %u sets\n", freed_sets);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Pool not found\n");
    return -1;
}

/* ============================================================================
 * Descriptor Set API
 * ============================================================================ */

int D3D8_Vulkan_AllocateDescriptorSet(VkDevice device,
                                      D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                      D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* layout_handle,
                                      D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* out_handle) {
    printf("[Phase 13] D3D8_Vulkan_AllocateDescriptorSet called\n");
    
    if (device == NULL || pool_handle == NULL || layout_handle == NULL || out_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_DESCRIPTOR_POOLS; i++) {
        if (g_pool_cache[i].is_allocated && g_pool_cache[i].pool == pool_handle->pool) {
            if (g_pool_cache[i].sets_allocated >= g_pool_cache[i].max_sets) {
                printf("[Phase 13] ERROR: Pool full (%u/%u sets)\n",
                       g_pool_cache[i].sets_allocated, g_pool_cache[i].max_sets);
                return -1;
            }
            
            int slot = _find_free_descriptor_set_slot();
            if (slot == -1) {
                printf("[Phase 13] ERROR: Descriptor set cache full\n");
                return -1;
            }
            
            uint32_t set_id = g_descriptor_set_counter++;
            
            g_descriptor_set_cache[slot].is_allocated = 1;
            g_descriptor_set_cache[slot].set_id = set_id;
            g_descriptor_set_cache[slot].layout = layout_handle->layout;
            g_descriptor_set_cache[slot].set = (void*)(uintptr_t)(10000000 + set_id);
            
            out_handle->set = (VkDescriptorSet)g_descriptor_set_cache[slot].set;
            out_handle->set_id = set_id;
            out_handle->layout = layout_handle->layout;
            
            g_pool_cache[i].sets_allocated++;
            pool_handle->sets_allocated++;
            
            printf("[Phase 13] Descriptor set allocated: id=%u pool=%u/%u\n",
                   set_id, g_pool_cache[i].sets_allocated, g_pool_cache[i].max_sets);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Pool not found\n");
    return -1;
}

int D3D8_Vulkan_AllocateDescriptorSets(VkDevice device,
                                       D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                       D3D8_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE* layout_handle,
                                       uint32_t count, D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* out_handles) {
    printf("[Phase 13] D3D8_Vulkan_AllocateDescriptorSets called: count=%u\n", count);
    
    if (device == NULL || pool_handle == NULL || layout_handle == NULL || out_handles == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        if (D3D8_Vulkan_AllocateDescriptorSet(device, pool_handle, layout_handle, &out_handles[i]) != 0) {
            printf("[Phase 13] ERROR: Failed to allocate set %u of %u\n", i, count);
            return -1;
        }
    }
    
    printf("[Phase 13] Descriptor sets allocated: %u sets\n", count);
    return 0;
}

int D3D8_Vulkan_FreeDescriptorSet(VkDevice device,
                                  D3D8_VULKAN_DESCRIPTOR_POOL_HANDLE* pool_handle,
                                  D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* set_handle) {
    printf("[Phase 13] D3D8_Vulkan_FreeDescriptorSet called\n");
    
    if (device == NULL || pool_handle == NULL || set_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (int i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
        if (g_descriptor_set_cache[i].is_allocated && g_descriptor_set_cache[i].set == set_handle->set) {
            g_descriptor_set_cache[i].is_allocated = 0;
            
            for (int j = 0; j < MAX_DESCRIPTOR_POOLS; j++) {
                if (g_pool_cache[j].is_allocated && g_pool_cache[j].pool == pool_handle->pool) {
                    if (g_pool_cache[j].sets_allocated > 0) {
                        g_pool_cache[j].sets_allocated--;
                        pool_handle->sets_allocated--;
                    }
                    break;
                }
            }
            
            printf("[Phase 13] Descriptor set freed: id=%u\n", set_handle->set_id);
            return 0;
        }
    }
    
    printf("[Phase 13] ERROR: Descriptor set not found\n");
    return -1;
}

int D3D8_Vulkan_UpdateDescriptorSet(VkDevice device, const D3D8_VULKAN_WRITE_DESCRIPTOR_SET* write_descriptor_set) {
    printf("[Phase 13] D3D8_Vulkan_UpdateDescriptorSet called: binding=%u type=%d count=%u\n",
           write_descriptor_set->dst_binding, write_descriptor_set->descriptor_type,
           write_descriptor_set->descriptor_count);
    
    if (device == NULL || write_descriptor_set == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    printf("[Phase 13] Descriptor set updated\n");
    return 0;
}

int D3D8_Vulkan_UpdateDescriptorSets(VkDevice device, const D3D8_VULKAN_WRITE_DESCRIPTOR_SET* writes,
                                     uint32_t write_count) {
    printf("[Phase 13] D3D8_Vulkan_UpdateDescriptorSets called: count=%u\n", write_count);
    
    if (device == NULL || writes == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    for (uint32_t i = 0; i < write_count; i++) {
        if (D3D8_Vulkan_UpdateDescriptorSet(device, &writes[i]) != 0) {
            printf("[Phase 13] ERROR: Failed to update descriptor set %u\n", i);
            return -1;
        }
    }
    
    printf("[Phase 13] Descriptor sets updated: %u writes\n", write_count);
    return 0;
}

int D3D8_Vulkan_BindDescriptorSet(VkDevice device, D3D8_VULKAN_DESCRIPTOR_SET_HANDLE* set_handle,
                                  uint32_t binding_point, uint32_t first_set) {
    printf("[Phase 13] D3D8_Vulkan_BindDescriptorSet called: binding_point=%u first_set=%u\n",
           binding_point, first_set);
    
    if (device == NULL || set_handle == NULL) {
        printf("[Phase 13] ERROR: Invalid parameters\n");
        return -1;
    }
    
    if (binding_point > 1) {
        printf("[Phase 13] ERROR: Invalid binding point: %u\n", binding_point);
        return -1;
    }
    
    printf("[Phase 13] Descriptor set bound: set_id=%u\n", set_handle->set_id);
    return 0;
}

/* ============================================================================
 * Sampler Presets
 * ============================================================================ */

void D3D8_Vulkan_GetSamplerPreset_PointClamp(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config) {
    memset(out_config, 0, sizeof(D3D8_VULKAN_SAMPLER_CREATE_INFO));
    out_config->mag_filter = D3D8_VULKAN_SAMPLER_FILTER_NEAREST;
    out_config->min_filter = D3D8_VULKAN_SAMPLER_FILTER_NEAREST;
    out_config->mipmap_filter = D3D8_VULKAN_SAMPLER_FILTER_NEAREST;
    out_config->address_u = D3D8_VULKAN_SAMPLER_ADDRESS_CLAMP;
    out_config->address_v = D3D8_VULKAN_SAMPLER_ADDRESS_CLAMP;
    out_config->address_w = D3D8_VULKAN_SAMPLER_ADDRESS_CLAMP;
    out_config->max_anisotropy = 1.0f;
    printf("[Phase 13] Sampler preset: PointClamp\n");
}

void D3D8_Vulkan_GetSamplerPreset_LinearRepeat(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config) {
    memset(out_config, 0, sizeof(D3D8_VULKAN_SAMPLER_CREATE_INFO));
    out_config->mag_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->min_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->mipmap_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->address_u = D3D8_VULKAN_SAMPLER_ADDRESS_REPEAT;
    out_config->address_v = D3D8_VULKAN_SAMPLER_ADDRESS_REPEAT;
    out_config->address_w = D3D8_VULKAN_SAMPLER_ADDRESS_REPEAT;
    out_config->max_anisotropy = 1.0f;
    printf("[Phase 13] Sampler preset: LinearRepeat\n");
}

void D3D8_Vulkan_GetSamplerPreset_AnisotropicMirror(D3D8_VULKAN_SAMPLER_CREATE_INFO* out_config) {
    memset(out_config, 0, sizeof(D3D8_VULKAN_SAMPLER_CREATE_INFO));
    out_config->mag_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->min_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->mipmap_filter = D3D8_VULKAN_SAMPLER_FILTER_LINEAR;
    out_config->address_u = D3D8_VULKAN_SAMPLER_ADDRESS_MIRROR;
    out_config->address_v = D3D8_VULKAN_SAMPLER_ADDRESS_MIRROR;
    out_config->address_w = D3D8_VULKAN_SAMPLER_ADDRESS_MIRROR;
    out_config->max_anisotropy = 16.0f;
    printf("[Phase 13] Sampler preset: AnisotropicMirror\n");
}
