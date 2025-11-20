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
**  FILE: d3d8_vulkan_culling.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Visibility & Culling System
**
**  DESCRIPTION:
**    Frustum culling, distance-based culling, occlusion queries,
**    visibility determination for efficient rendering.
**
**    Phase 18: Visibility & Culling - Implementation
**
******************************************************************************/

#include "d3d8_vulkan_culling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/* ============================================================================
 * Internal Constants & Configuration
 * ============================================================================ */

#define D3D8_VULKAN_MAX_CULLING_SYSTEMS 2
#define D3D8_VULKAN_MAX_OCCLUSION_QUERIES 256
#define D3D8_VULKAN_ERROR_MESSAGE_SIZE 512

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

/**
 * @struct CullingEntry
 * @brief Internal culling system cache entry
 */
typedef struct {
    uint32_t id;
    uint32_t version;
    D3D8_VULKAN_CULLING_STATE state;
    D3D8_VULKAN_FRUSTUM current_frustum;
    D3D8_VULKAN_CULLING_CONFIG config;
    D3D8_VULKAN_CULLING_STATS statistics;
    uint32_t occlusion_query_counter;
    uint32_t allocated;
} CullingEntry;

/* ============================================================================
 * Global State & Caches
 * ============================================================================ */

static CullingEntry g_culling_cache[D3D8_VULKAN_MAX_CULLING_SYSTEMS];
static uint32_t g_culling_counter = 13000;
static char g_last_error[D3D8_VULKAN_ERROR_MESSAGE_SIZE];

/* ============================================================================
 * Utility Functions (Internal)
 * ============================================================================ */

/**
 * @fn static uint32_t _find_free_culling_slot(void)
 * @brief Find next free slot in culling cache
 */
static uint32_t _find_free_culling_slot(void)
{
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_CULLING_SYSTEMS; i++) {
        if (!g_culling_cache[i].allocated) {
            return i;
        }
    }
    return D3D8_VULKAN_MAX_CULLING_SYSTEMS;
}

/**
 * @fn static int _is_valid_culling_handle(D3D8_VULKAN_CULLING_HANDLE handle)
 * @brief Validate culling handle
 */
static int _is_valid_culling_handle(D3D8_VULKAN_CULLING_HANDLE handle)
{
    if (handle.id < 13000 || handle.id >= 13000 + D3D8_VULKAN_MAX_CULLING_SYSTEMS) {
        return 0;
    }
    uint32_t index = handle.id - 13000;
    if (index >= D3D8_VULKAN_MAX_CULLING_SYSTEMS) {
        return 0;
    }
    if (!g_culling_cache[index].allocated) {
        return 0;
    }
    if (g_culling_cache[index].version != handle.version) {
        return 0;
    }
    return 1;
}

/**
 * @fn static CullingEntry* _get_culling_entry(D3D8_VULKAN_CULLING_HANDLE handle)
 * @brief Get culling entry from handle
 */
static CullingEntry* _get_culling_entry(D3D8_VULKAN_CULLING_HANDLE handle)
{
    if (!_is_valid_culling_handle(handle)) {
        return NULL;
    }
    uint32_t index = handle.id - 13000;
    return &g_culling_cache[index];
}

/**
 * @fn static void _set_error(const char* format, ...)
 * @brief Set error message with varargs
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
 * @fn static float _plane_distance_to_point(const D3D8_VULKAN_PLANE_EQUATION* plane, float x, float y, float z)
 * @brief Calculate signed distance from plane to point
 */
static float _plane_distance_to_point(const D3D8_VULKAN_PLANE_EQUATION* plane, 
                                      float x, float y, float z)
{
    return plane->a * x + plane->b * y + plane->c * z + plane->d;
}

/* ============================================================================
 * Visibility & Culling API Implementation (16 Functions)
 * ============================================================================ */

D3D8_VULKAN_CULLING_HANDLE D3D8_Vulkan_InitializeCulling(
    VkDevice device,
    const D3D8_VULKAN_CULLING_CONFIG* config)
{
    D3D8_VULKAN_CULLING_HANDLE invalid_handle = {0, 0};

    if (!device || !config) {
        _set_error("InitializeCulling: Invalid parameters");
        return invalid_handle;
    }

    uint32_t slot = _find_free_culling_slot();
    if (slot >= D3D8_VULKAN_MAX_CULLING_SYSTEMS) {
        _set_error("InitializeCulling: Cache full");
        return invalid_handle;
    }

    CullingEntry* entry = &g_culling_cache[slot];
    memset(entry, 0, sizeof(CullingEntry));

    entry->id = g_culling_counter++;
    entry->version = 1;
    entry->state = D3D8_VULKAN_CULL_STATE_READY;
    memcpy(&entry->config, config, sizeof(D3D8_VULKAN_CULLING_CONFIG));
    entry->allocated = 1;

    printf("D3D8_Vulkan_Culling: InitializeCulling (ID: %u, Frustum: %u, Distance: %u, Occlusion: %u)\n",
           entry->id, config->enable_frustum_culling, config->enable_distance_culling, 
           config->enable_occlusion_queries);

    D3D8_VULKAN_CULLING_HANDLE handle = {entry->id, entry->version};
    return handle;
}

void D3D8_Vulkan_ShutdownCulling(
    D3D8_VULKAN_CULLING_HANDLE culling)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("ShutdownCulling: Invalid handle");
        return;
    }

    printf("D3D8_Vulkan_Culling: ShutdownCulling (ID: %u, Total: %u, Visible: %u)\n",
           entry->id, entry->statistics.total_objects, entry->statistics.visible_objects);

    entry->state = D3D8_VULKAN_CULL_STATE_SHUTDOWN;
    entry->allocated = 0;
    entry->version++;
    memset(entry, 0, sizeof(CullingEntry));
}

int D3D8_Vulkan_BuildFrustum(
    D3D8_VULKAN_CULLING_HANDLE culling,
    const float* view_matrix,
    const float* projection_matrix,
    D3D8_VULKAN_FRUSTUM* out_frustum)
{
    if (!view_matrix || !projection_matrix || !out_frustum) {
        _set_error("BuildFrustum: Invalid parameters");
        return -1;
    }

    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("BuildFrustum: Invalid handle");
        return -1;
    }

    // Simplified frustum extraction from view/projection matrices
    // In real implementation, would extract planes from concatenated matrix
    memset(out_frustum, 0, sizeof(D3D8_VULKAN_FRUSTUM));
    
    // Initialize plane equations (stub implementation)
    for (int i = 0; i < 6; i++) {
        out_frustum->planes[i].a = 0.0f;
        out_frustum->planes[i].b = 0.0f;
        out_frustum->planes[i].c = 1.0f;
        out_frustum->planes[i].d = -10.0f - i;
    }

    memcpy(&entry->current_frustum, out_frustum, sizeof(D3D8_VULKAN_FRUSTUM));

    printf("D3D8_Vulkan_Culling: BuildFrustum (ID: %u)\n", entry->id);

    return 0;
}

D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestPointVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    float x, float y, float z)
{
    if (!frustum) {
        return D3D8_VULKAN_VIS_OUTSIDE;
    }

    // Test point against all 6 planes
    for (int i = 0; i < 6; i++) {
        float dist = _plane_distance_to_point(&frustum->planes[i], x, y, z);
        if (dist < 0.0f) {
            return D3D8_VULKAN_VIS_OUTSIDE;
        }
    }

    return D3D8_VULKAN_VIS_INSIDE;
}

D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestSphereVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    const D3D8_VULKAN_SPHERE* sphere)
{
    if (!frustum || !sphere) {
        return D3D8_VULKAN_VIS_OUTSIDE;
    }

    int fully_inside = 1;

    // Test sphere against all 6 planes
    for (int i = 0; i < 6; i++) {
        float dist = _plane_distance_to_point(&frustum->planes[i], 
                                              sphere->center_x, sphere->center_y, sphere->center_z);
        
        if (dist < -sphere->radius) {
            return D3D8_VULKAN_VIS_OUTSIDE;  /* Outside this plane */
        }
        if (dist < sphere->radius) {
            fully_inside = 0;  /* Intersects this plane */
        }
    }

    return fully_inside ? D3D8_VULKAN_VIS_INSIDE : D3D8_VULKAN_VIS_INTERSECT;
}

D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestAABBVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    const D3D8_VULKAN_AABB* aabb)
{
    if (!frustum || !aabb) {
        return D3D8_VULKAN_VIS_OUTSIDE;
    }

    int fully_inside = 1;

    // Test AABB corners against all 6 planes
    for (int i = 0; i < 6; i++) {
        float dist_min = _plane_distance_to_point(&frustum->planes[i], aabb->min_x, aabb->min_y, aabb->min_z);
        float dist_max = _plane_distance_to_point(&frustum->planes[i], aabb->max_x, aabb->max_y, aabb->max_z);
        
        float dist_extreme = dist_max > dist_min ? dist_max : dist_min;
        if (dist_extreme < 0.0f) {
            return D3D8_VULKAN_VIS_OUTSIDE;
        }
        
        if (dist_min < 0.0f || dist_max < 0.0f) {
            fully_inside = 0;
        }
    }

    return fully_inside ? D3D8_VULKAN_VIS_INSIDE : D3D8_VULKAN_VIS_INTERSECT;
}

int D3D8_Vulkan_TestDistanceVisibility(
    D3D8_VULKAN_CULLING_HANDLE culling,
    float object_distance)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("TestDistanceVisibility: Invalid handle");
        return -1;
    }

    if (!entry->config.enable_distance_culling) {
        return 1;  /* Not culling by distance */
    }

    /* Check if object is within fog distance range */
    if (object_distance < entry->config.fog_distance_near) {
        return 1;  /* Within near distance - visible */
    }
    if (object_distance > entry->config.fog_distance_far) {
        return 0;  /* Beyond far distance - culled */
    }

    return 1;  /* In range - visible */
}

int D3D8_Vulkan_StartOcclusionQuery(
    D3D8_VULKAN_CULLING_HANDLE culling,
    uint32_t query_id)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("StartOcclusionQuery: Invalid handle");
        return -1;
    }

    if (query_id >= D3D8_VULKAN_MAX_OCCLUSION_QUERIES) {
        _set_error("StartOcclusionQuery: Query ID out of range");
        return -1;
    }

    if (entry->config.enable_occlusion_queries) {
        entry->statistics.occlusion_queries_issued++;
        entry->statistics.occlusion_queries_pending++;
    }

    return 0;
}

int D3D8_Vulkan_EndOcclusionQuery(
    D3D8_VULKAN_CULLING_HANDLE culling)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("EndOcclusionQuery: Invalid handle");
        return -1;
    }

    if (entry->statistics.occlusion_queries_pending > 0) {
        entry->statistics.occlusion_queries_pending--;
    }

    return 0;
}

int D3D8_Vulkan_GetOcclusionQueryResult(
    D3D8_VULKAN_CULLING_HANDLE culling,
    uint32_t query_id,
    uint32_t* out_sample_count)
{
    if (!out_sample_count) {
        _set_error("GetOcclusionQueryResult: out_sample_count is NULL");
        return -1;
    }

    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("GetOcclusionQueryResult: Invalid handle");
        return -1;
    }

    if (query_id >= D3D8_VULKAN_MAX_OCCLUSION_QUERIES) {
        _set_error("GetOcclusionQueryResult: Query ID out of range");
        return -1;
    }

    /* Stub: Return sample count (in real impl, would fetch from Vulkan) */
    *out_sample_count = 1;

    return 0;
}

int D3D8_Vulkan_GetCullingStats(
    D3D8_VULKAN_CULLING_HANDLE culling,
    D3D8_VULKAN_CULLING_STATS* out_stats)
{
    if (!out_stats) {
        _set_error("GetCullingStats: out_stats is NULL");
        return -1;
    }

    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("GetCullingStats: Invalid handle");
        return -1;
    }

    memcpy(out_stats, &entry->statistics, sizeof(D3D8_VULKAN_CULLING_STATS));

    return 0;
}

int D3D8_Vulkan_GetCullingInfo(
    D3D8_VULKAN_CULLING_HANDLE culling,
    D3D8_VULKAN_CULLING_INFO* out_info)
{
    if (!out_info) {
        _set_error("GetCullingInfo: out_info is NULL");
        return -1;
    }

    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("GetCullingInfo: Invalid handle");
        return -1;
    }

    memset(out_info, 0, sizeof(D3D8_VULKAN_CULLING_INFO));
    out_info->state = entry->state;
    memcpy(&out_info->current_frustum, &entry->current_frustum, sizeof(D3D8_VULKAN_FRUSTUM));
    memcpy(&out_info->statistics, &entry->statistics, sizeof(D3D8_VULKAN_CULLING_STATS));

    return 0;
}

int D3D8_Vulkan_ResetCullingStats(
    D3D8_VULKAN_CULLING_HANDLE culling)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("ResetCullingStats: Invalid handle");
        return -1;
    }

    memset(&entry->statistics, 0, sizeof(D3D8_VULKAN_CULLING_STATS));

    printf("D3D8_Vulkan_Culling: ResetCullingStats (ID: %u)\n", entry->id);

    return 0;
}

int D3D8_Vulkan_SetFrustum(
    D3D8_VULKAN_CULLING_HANDLE culling,
    const D3D8_VULKAN_FRUSTUM* frustum)
{
    if (!frustum) {
        _set_error("SetFrustum: frustum is NULL");
        return -1;
    }

    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("SetFrustum: Invalid handle");
        return -1;
    }

    memcpy(&entry->current_frustum, frustum, sizeof(D3D8_VULKAN_FRUSTUM));

    return 0;
}

int D3D8_VULKAN_SetDistanceCullingParams(
    D3D8_VULKAN_CULLING_HANDLE culling,
    float near_distance,
    float far_distance)
{
    CullingEntry* entry = _get_culling_entry(culling);
    if (!entry) {
        _set_error("SetDistanceCullingParams: Invalid handle");
        return -1;
    }

    if (near_distance > far_distance) {
        _set_error("SetDistanceCullingParams: near_distance > far_distance");
        return -1;
    }

    entry->config.fog_distance_near = near_distance;
    entry->config.fog_distance_far = far_distance;

    printf("D3D8_Vulkan_Culling: SetDistanceCullingParams (ID: %u, Near: %.2f, Far: %.2f)\n",
           entry->id, near_distance, far_distance);

    return 0;
}

const char* D3D8_Vulkan_GetCullingError(void)
{
    return g_last_error;
}
