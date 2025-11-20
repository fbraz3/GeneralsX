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
**  FILE: d3d8_vulkan_culling.h
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
**    Phase 18: Visibility & Culling - Header Definition
**
******************************************************************************/

#ifndef D3D8_VULKAN_CULLING_H
#define D3D8_VULKAN_CULLING_H

#include <stdint.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_CULLING_STATE
 * @brief Culling system state
 */
typedef enum {
    D3D8_VULKAN_CULL_STATE_UNINITIALIZED = 0x0,
    D3D8_VULKAN_CULL_STATE_READY = 0x1,
    D3D8_VULKAN_CULL_STATE_ACTIVE = 0x2,
    D3D8_VULKAN_CULL_STATE_SHUTDOWN = 0x3,
} D3D8_VULKAN_CULLING_STATE;

/**
 * @enum D3D8_VULKAN_PLANE
 * @brief Frustum plane indices
 */
typedef enum {
    D3D8_VULKAN_PLANE_NEAR = 0,
    D3D8_VULKAN_PLANE_FAR = 1,
    D3D8_VULKAN_PLANE_LEFT = 2,
    D3D8_VULKAN_PLANE_RIGHT = 3,
    D3D8_VULKAN_PLANE_TOP = 4,
    D3D8_VULKAN_PLANE_BOTTOM = 5,
} D3D8_VULKAN_PLANE;

/**
 * @enum D3D8_VULKAN_VISIBILITY_RESULT
 * @brief Visibility test result
 */
typedef enum {
    D3D8_VULKAN_VIS_OUTSIDE = 0,    /* Completely outside frustum */
    D3D8_VULKAN_VIS_INSIDE = 1,     /* Completely inside frustum */
    D3D8_VULKAN_VIS_INTERSECT = 2,  /* Partially visible */
} D3D8_VULKAN_VISIBILITY_RESULT;

/* ============================================================================
 * Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_PLANE_EQUATION
 * @brief Plane equation (ax + by + cz + d = 0)
 */
typedef struct {
    float a, b, c, d;
} D3D8_VULKAN_PLANE_EQUATION;

/**
 * @struct D3D8_VULKAN_FRUSTUM
 * @brief View frustum (6 planes)
 */
typedef struct {
    D3D8_VULKAN_PLANE_EQUATION planes[6];
} D3D8_VULKAN_FRUSTUM;

/**
 * @struct D3D8_VULKAN_AABB
 * @brief Axis-aligned bounding box
 */
typedef struct {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
} D3D8_VULKAN_AABB;

/**
 * @struct D3D8_VULKAN_SPHERE
 * @brief Bounding sphere
 */
typedef struct {
    float center_x, center_y, center_z;
    float radius;
} D3D8_VULKAN_SPHERE;

/**
 * @struct D3D8_VULKAN_CULLING_CONFIG
 * @brief Culling system configuration
 */
typedef struct {
    uint32_t max_occlusion_queries;
    uint32_t enable_frustum_culling;
    uint32_t enable_distance_culling;
    uint32_t enable_occlusion_queries;
    float fog_distance_near;
    float fog_distance_far;
} D3D8_VULKAN_CULLING_CONFIG;

/**
 * @struct D3D8_VULKAN_CULLING_STATS
 * @brief Culling statistics
 */
typedef struct {
    uint32_t total_objects;
    uint32_t visible_objects;
    uint32_t culled_frustum;
    uint32_t culled_distance;
    uint32_t culled_occlusion;
    uint32_t occlusion_queries_issued;
    uint32_t occlusion_queries_pending;
} D3D8_VULKAN_CULLING_STATS;

/**
 * @struct D3D8_VULKAN_CULLING_HANDLE
 * @brief Opaque culling system handle
 */
typedef struct {
    uint32_t id;
    uint32_t version;
} D3D8_VULKAN_CULLING_HANDLE;

/**
 * @struct D3D8_VULKAN_CULLING_INFO
 * @brief Culling system information
 */
typedef struct {
    D3D8_VULKAN_CULLING_STATE state;
    D3D8_VULKAN_FRUSTUM current_frustum;
    D3D8_VULKAN_CULLING_STATS statistics;
} D3D8_VULKAN_CULLING_INFO;

/* ============================================================================
 * Visibility & Culling API (16 Functions)
 * ============================================================================ */

/**
 * Initialize culling system
 */
D3D8_VULKAN_CULLING_HANDLE D3D8_Vulkan_InitializeCulling(
    VkDevice device,
    const D3D8_VULKAN_CULLING_CONFIG* config);

/**
 * Shutdown culling system
 */
void D3D8_Vulkan_ShutdownCulling(
    D3D8_VULKAN_CULLING_HANDLE culling);

/**
 * Build frustum from view/projection matrices
 */
int D3D8_Vulkan_BuildFrustum(
    D3D8_VULKAN_CULLING_HANDLE culling,
    const float* view_matrix,
    const float* projection_matrix,
    D3D8_VULKAN_FRUSTUM* out_frustum);

/**
 * Test point visibility in frustum
 */
D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestPointVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    float x, float y, float z);

/**
 * Test sphere visibility in frustum
 */
D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestSphereVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    const D3D8_VULKAN_SPHERE* sphere);

/**
 * Test AABB visibility in frustum
 */
D3D8_VULKAN_VISIBILITY_RESULT D3D8_Vulkan_TestAABBVisibility(
    const D3D8_VULKAN_FRUSTUM* frustum,
    const D3D8_VULKAN_AABB* aabb);

/**
 * Test distance-based visibility (fog of war)
 */
int D3D8_Vulkan_TestDistanceVisibility(
    D3D8_VULKAN_CULLING_HANDLE culling,
    float object_distance);

/**
 * Start occlusion query for object
 */
int D3D8_Vulkan_StartOcclusionQuery(
    D3D8_VULKAN_CULLING_HANDLE culling,
    uint32_t query_id);

/**
 * End occlusion query
 */
int D3D8_Vulkan_EndOcclusionQuery(
    D3D8_VULKAN_CULLING_HANDLE culling);

/**
 * Get occlusion query results
 */
int D3D8_Vulkan_GetOcclusionQueryResult(
    D3D8_VULKAN_CULLING_HANDLE culling,
    uint32_t query_id,
    uint32_t* out_sample_count);

/**
 * Get current culling statistics
 */
int D3D8_Vulkan_GetCullingStats(
    D3D8_VULKAN_CULLING_HANDLE culling,
    D3D8_VULKAN_CULLING_STATS* out_stats);

/**
 * Get culling system information
 */
int D3D8_Vulkan_GetCullingInfo(
    D3D8_VULKAN_CULLING_HANDLE culling,
    D3D8_VULKAN_CULLING_INFO* out_info);

/**
 * Reset statistics
 */
int D3D8_Vulkan_ResetCullingStats(
    D3D8_VULKAN_CULLING_HANDLE culling);

/**
 * Set frustum for culling
 */
int D3D8_Vulkan_SetFrustum(
    D3D8_VULKAN_CULLING_HANDLE culling,
    const D3D8_VULKAN_FRUSTUM* frustum);

/**
 * Set distance culling parameters
 */
int D3D8_VULKAN_SetDistanceCullingParams(
    D3D8_VULKAN_CULLING_HANDLE culling,
    float near_distance,
    float far_distance);

/**
 * Get culling system error message
 */
const char* D3D8_Vulkan_GetCullingError(void);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_CULLING_H */
