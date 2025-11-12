/**
 * @file GameWorld.cpp
 * @brief Phase 28: World Management - Spatial Indexing Implementation
 */

#include "GameWorld.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define MAX_WORLD_OBJECTS 5000
#define MAX_QUADTREE_NODES 2000
#define MAX_QUERY_RESULTS 500
#define MIN_QUADTREE_SIZE 32.0f
#define ERROR_BUFFER_SIZE 256

// World state
typedef struct {
    int is_initialized;
    float width;
    float height;
    uint32_t max_objects;
    uint32_t object_count;
    WorldObject* objects;
    uint32_t* deletion_queue;
    uint32_t deletion_queue_count;
    
    QuadtreeNode* quadtree_nodes;
    uint32_t quadtree_node_count;
    uint32_t quadtree_depth;
    uint32_t root_node;
    
    WorldUpdateCallback update_callback;
    void* callback_user_data;
    
    char error_message[ERROR_BUFFER_SIZE];
} GameWorldState;

static GameWorldState g_world_state = {0};

// Set error message
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_world_state.error_message, ERROR_BUFFER_SIZE, format, args);
    va_end(args);
}

// Create quadtree node
static uint32_t CreateQuadtreeNode(float min_x, float min_y, float max_x, float max_y, uint32_t depth) {
    if (g_world_state.quadtree_node_count >= MAX_QUADTREE_NODES) {
        SetError("Quadtree node limit exceeded");
        return 0;
    }
    
    uint32_t node_idx = g_world_state.quadtree_node_count++;
    QuadtreeNode* node = &g_world_state.quadtree_nodes[node_idx];
    
    node->min_x = min_x;
    node->min_y = min_y;
    node->max_x = max_x;
    node->max_y = max_y;
    node->depth = depth;
    node->object_count = 0;
    for (int i = 0; i < 4; i++) {
        node->children[i] = 0;
    }
    
    return node_idx;
}

// Build quadtree
static void BuildQuadtree(void) {
    // Clear existing tree
    g_world_state.quadtree_node_count = 0;
    g_world_state.quadtree_depth = 0;
    
    // Create root node
    g_world_state.root_node = CreateQuadtreeNode(0, 0, g_world_state.width, g_world_state.height, 0);
    
    // Insert all active objects
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (g_world_state.objects[i].is_active) {
            // Track in root node for now (simplified)
            g_world_state.quadtree_nodes[g_world_state.root_node].object_count++;
        }
    }
}

// Recursive quadtree search for radius query
static void RadiusSearchRecursive(uint32_t node_idx, float center_x, float center_y, float radius,
                                  uint32_t* result, uint32_t* result_count, uint32_t max_results) {
    if (node_idx == 0 || node_idx >= g_world_state.quadtree_node_count) {
        return;
    }
    
    QuadtreeNode* node = &g_world_state.quadtree_nodes[node_idx];
    
    // Check if node bounds intersect with query circle
    float closest_x = center_x;
    float closest_y = center_y;
    
    if (center_x < node->min_x) closest_x = node->min_x;
    else if (center_x > node->max_x) closest_x = node->max_x;
    
    if (center_y < node->min_y) closest_y = node->min_y;
    else if (center_y > node->max_y) closest_y = node->max_y;
    
    float dx = center_x - closest_x;
    float dy = center_y - closest_y;
    float dist_sq = dx * dx + dy * dy;
    
    if (dist_sq > radius * radius) {
        return;  // No intersection
    }
    
    // Check objects in this node
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (!g_world_state.objects[i].is_active) continue;
        
        // Check if object is in node bounds
        if (g_world_state.objects[i].x < node->min_x || g_world_state.objects[i].x > node->max_x) continue;
        if (g_world_state.objects[i].y < node->min_y || g_world_state.objects[i].y > node->max_y) continue;
        
        // Check distance
        float obj_dx = g_world_state.objects[i].x - center_x;
        float obj_dy = g_world_state.objects[i].y - center_y;
        float obj_dist_sq = obj_dx * obj_dx + obj_dy * obj_dy;
        
        if (obj_dist_sq <= (radius + g_world_state.objects[i].radius) * (radius + g_world_state.objects[i].radius)) {
            if (*result_count < max_results) {
                result[*result_count] = g_world_state.objects[i].object_id;
                (*result_count)++;
            }
        }
    }
    
    // Search children
    for (int i = 0; i < 4; i++) {
        if (node->children[i] != 0) {
            RadiusSearchRecursive(node->children[i], center_x, center_y, radius, result, result_count, max_results);
        }
    }
}

// Recursive frustum check
static void FrustumSearchRecursive(uint32_t node_idx, const ViewFrustum* frustum,
                                   uint32_t* result, uint32_t* result_count, uint32_t max_results) {
    if (node_idx == 0 || node_idx >= g_world_state.quadtree_node_count) {
        return;
    }
    
    QuadtreeNode* node = &g_world_state.quadtree_nodes[node_idx];
    
    // Check all objects in node
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (!g_world_state.objects[i].is_active) continue;
        
        // Check if object is in node bounds
        if (g_world_state.objects[i].x < node->min_x || g_world_state.objects[i].x > node->max_x) continue;
        if (g_world_state.objects[i].y < node->min_y || g_world_state.objects[i].y > node->max_y) continue;
        
        // Test sphere against frustum (simplified - all planes)
        int in_frustum = 1;
        for (int p = 0; p < 6; p++) {
            const FrustumPlane* plane = &frustum->planes[p];
            float dist = plane->a * g_world_state.objects[i].x +
                        plane->b * g_world_state.objects[i].y +
                        plane->c * g_world_state.objects[i].z +
                        plane->d;
            
            if (dist < -g_world_state.objects[i].radius) {
                in_frustum = 0;
                break;
            }
        }
        
        if (in_frustum && *result_count < max_results) {
            result[*result_count] = g_world_state.objects[i].object_id;
            (*result_count)++;
        }
    }
    
    // Search children
    for (int i = 0; i < 4; i++) {
        if (node->children[i] != 0) {
            FrustumSearchRecursive(node->children[i], frustum, result, result_count, max_results);
        }
    }
}

// Find object by ID
static int FindObjectIndex(uint32_t object_id) {
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (g_world_state.objects[i].object_id == object_id) {
            return i;
        }
    }
    return -1;
}

int GameWorld_Initialize(float world_width, float world_height, uint32_t max_objects) {
    if (g_world_state.is_initialized) {
        SetError("World already initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    if (max_objects > MAX_WORLD_OBJECTS || max_objects < 1) {
        SetError("Invalid max_objects parameter");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    // Allocate memory
    g_world_state.objects = (WorldObject*)malloc(max_objects * sizeof(WorldObject));
    if (!g_world_state.objects) {
        SetError("Failed to allocate objects array");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    g_world_state.quadtree_nodes = (QuadtreeNode*)malloc(MAX_QUADTREE_NODES * sizeof(QuadtreeNode));
    if (!g_world_state.quadtree_nodes) {
        free(g_world_state.objects);
        SetError("Failed to allocate quadtree nodes");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    g_world_state.deletion_queue = (uint32_t*)malloc(max_objects * sizeof(uint32_t));
    if (!g_world_state.deletion_queue) {
        free(g_world_state.objects);
        free(g_world_state.quadtree_nodes);
        SetError("Failed to allocate deletion queue");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    g_world_state.width = world_width;
    g_world_state.height = world_height;
    g_world_state.max_objects = max_objects;
    g_world_state.object_count = 0;
    g_world_state.deletion_queue_count = 0;
    g_world_state.is_initialized = 1;
    g_world_state.update_callback = NULL;
    g_world_state.callback_user_data = NULL;
    
    // Build initial quadtree
    BuildQuadtree();
    
    return GAMEWORLD_OK;
}

int GameWorld_Shutdown(void) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    free(g_world_state.objects);
    free(g_world_state.quadtree_nodes);
    free(g_world_state.deletion_queue);
    
    memset(&g_world_state, 0, sizeof(GameWorldState));
    
    return GAMEWORLD_OK;
}

int GameWorld_AddObject(uint32_t object_id, float x, float y, float z, float radius) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    if (g_world_state.object_count >= g_world_state.max_objects) {
        SetError("Object limit exceeded");
        return GAMEWORLD_ERROR_LIMIT_EXCEEDED;
    }
    
    // Check if object already exists
    if (FindObjectIndex(object_id) >= 0) {
        SetError("Object already exists");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    // Clamp position to world bounds
    if (x < 0) x = 0;
    if (x > g_world_state.width) x = g_world_state.width;
    if (y < 0) y = 0;
    if (y > g_world_state.height) y = g_world_state.height;
    
    WorldObject* obj = &g_world_state.objects[g_world_state.object_count++];
    obj->object_id = object_id;
    obj->x = x;
    obj->y = y;
    obj->z = z;
    obj->radius = radius > 0 ? radius : 1.0f;
    obj->is_active = 1;
    
    return GAMEWORLD_OK;
}

int GameWorld_RemoveObject(uint32_t object_id) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    int idx = FindObjectIndex(object_id);
    if (idx < 0) {
        SetError("Object not found");
        return GAMEWORLD_ERROR_NOT_FOUND;
    }
    
    // Remove by swapping with last
    g_world_state.objects[idx] = g_world_state.objects[g_world_state.object_count - 1];
    g_world_state.object_count--;
    
    return GAMEWORLD_OK;
}

int GameWorld_UpdateObjectPosition(uint32_t object_id, float x, float y, float z) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    int idx = FindObjectIndex(object_id);
    if (idx < 0) {
        SetError("Object not found");
        return GAMEWORLD_ERROR_NOT_FOUND;
    }
    
    // Clamp to bounds
    if (x < 0) x = 0;
    if (x > g_world_state.width) x = g_world_state.width;
    if (y < 0) y = 0;
    if (y > g_world_state.height) y = g_world_state.height;
    
    g_world_state.objects[idx].x = x;
    g_world_state.objects[idx].y = y;
    g_world_state.objects[idx].z = z;
    
    return GAMEWORLD_OK;
}

uint32_t GameWorld_QueryRadius(float center_x, float center_y, float radius,
                               uint32_t* result, uint32_t max_results) {
    if (!g_world_state.is_initialized || !result) {
        return 0;
    }
    
    uint32_t result_count = 0;
    RadiusSearchRecursive(g_world_state.root_node, center_x, center_y, radius, result, &result_count, max_results);
    
    return result_count;
}

uint32_t GameWorld_QueryBox(float min_x, float min_y, float max_x, float max_y,
                           uint32_t* result, uint32_t max_results) {
    if (!g_world_state.is_initialized || !result) {
        return 0;
    }
    
    uint32_t result_count = 0;
    
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (!g_world_state.objects[i].is_active) continue;
        
        // Check if object is in box
        if (g_world_state.objects[i].x >= min_x && g_world_state.objects[i].x <= max_x &&
            g_world_state.objects[i].y >= min_y && g_world_state.objects[i].y <= max_y) {
            if (result_count < max_results) {
                result[result_count++] = g_world_state.objects[i].object_id;
            }
        }
    }
    
    return result_count;
}

uint32_t GameWorld_QueryFrustum(const ViewFrustum* frustum,
                               uint32_t* result, uint32_t max_results) {
    if (!g_world_state.is_initialized || !frustum || !result) {
        return 0;
    }
    
    uint32_t result_count = 0;
    FrustumSearchRecursive(g_world_state.root_node, frustum, result, &result_count, max_results);
    
    return result_count;
}

uint32_t GameWorld_FindNearestObject(float center_x, float center_y,
                                     float max_distance, uint32_t exclude_id) {
    if (!g_world_state.is_initialized) {
        return 0;
    }
    
    uint32_t nearest_id = 0;
    float nearest_dist_sq = max_distance * max_distance;
    
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (!g_world_state.objects[i].is_active) continue;
        if (g_world_state.objects[i].object_id == exclude_id) continue;
        
        float dx = g_world_state.objects[i].x - center_x;
        float dy = g_world_state.objects[i].y - center_y;
        float dist_sq = dx * dx + dy * dy;
        
        if (dist_sq < nearest_dist_sq) {
            nearest_dist_sq = dist_sq;
            nearest_id = g_world_state.objects[i].object_id;
        }
    }
    
    return nearest_id;
}

int GameWorld_GetObjectBounds(uint32_t object_id, BoundingSphere* bounds) {
    if (!g_world_state.is_initialized || !bounds) {
        SetError("Invalid parameters");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    int idx = FindObjectIndex(object_id);
    if (idx < 0) {
        SetError("Object not found");
        return GAMEWORLD_ERROR_NOT_FOUND;
    }
    
    bounds->x = g_world_state.objects[idx].x;
    bounds->y = g_world_state.objects[idx].y;
    bounds->z = g_world_state.objects[idx].z;
    bounds->radius = g_world_state.objects[idx].radius;
    
    return GAMEWORLD_OK;
}

int GameWorld_TestSphereSphere(const BoundingSphere* sphere1, const BoundingSphere* sphere2) {
    if (!sphere1 || !sphere2) return 0;
    
    float dx = sphere1->x - sphere2->x;
    float dy = sphere1->y - sphere2->y;
    float dz = sphere1->z - sphere2->z;
    float dist_sq = dx * dx + dy * dy + dz * dz;
    float radius_sum = sphere1->radius + sphere2->radius;
    
    return dist_sq <= radius_sum * radius_sum;
}

int GameWorld_TestSphereBox(const BoundingSphere* sphere, const BoundingBox* box) {
    if (!sphere || !box) return 0;
    
    float closest_x = sphere->x;
    float closest_y = sphere->y;
    float closest_z = sphere->z;
    
    if (closest_x < box->min_x) closest_x = box->min_x;
    else if (closest_x > box->max_x) closest_x = box->max_x;
    
    if (closest_y < box->min_y) closest_y = box->min_y;
    else if (closest_y > box->max_y) closest_y = box->max_y;
    
    if (closest_z < box->min_z) closest_z = box->min_z;
    else if (closest_z > box->max_z) closest_z = box->max_z;
    
    float dx = sphere->x - closest_x;
    float dy = sphere->y - closest_y;
    float dz = sphere->z - closest_z;
    float dist_sq = dx * dx + dy * dy + dz * dz;
    
    return dist_sq <= sphere->radius * sphere->radius;
}

int GameWorld_TestPointInFrustum(float x, float y, float z, const ViewFrustum* frustum) {
    if (!frustum) return 0;
    
    for (int i = 0; i < 6; i++) {
        const FrustumPlane* plane = &frustum->planes[i];
        float dist = plane->a * x + plane->b * y + plane->c * z + plane->d;
        if (dist < 0) return 0;
    }
    
    return 1;
}

int GameWorld_TestSphereInFrustum(const BoundingSphere* sphere, const ViewFrustum* frustum) {
    if (!sphere || !frustum) return 0;
    
    for (int i = 0; i < 6; i++) {
        const FrustumPlane* plane = &frustum->planes[i];
        float dist = plane->a * sphere->x + plane->b * sphere->y + plane->c * sphere->z + plane->d;
        if (dist < -sphere->radius) return 0;
    }
    
    return 1;
}

uint32_t GameWorld_GetVisibleObjects(const ViewFrustum* frustum,
                                     uint32_t* result, uint32_t max_results) {
    if (!g_world_state.is_initialized || !frustum || !result) {
        return 0;
    }
    
    uint32_t result_count = 0;
    
    for (uint32_t i = 0; i < g_world_state.object_count; i++) {
        if (!g_world_state.objects[i].is_active) continue;
        
        BoundingSphere bounds;
        bounds.x = g_world_state.objects[i].x;
        bounds.y = g_world_state.objects[i].y;
        bounds.z = g_world_state.objects[i].z;
        bounds.radius = g_world_state.objects[i].radius;
        
        if (GameWorld_TestSphereInFrustum(&bounds, frustum)) {
            if (result_count < max_results) {
                result[result_count++] = g_world_state.objects[i].object_id;
            }
        }
    }
    
    return result_count;
}

int GameWorld_MarkForDeletion(uint32_t object_id) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    if (FindObjectIndex(object_id) < 0) {
        SetError("Object not found");
        return GAMEWORLD_ERROR_NOT_FOUND;
    }
    
    if (g_world_state.deletion_queue_count >= g_world_state.max_objects) {
        SetError("Deletion queue full");
        return GAMEWORLD_ERROR_LIMIT_EXCEEDED;
    }
    
    g_world_state.deletion_queue[g_world_state.deletion_queue_count++] = object_id;
    
    return GAMEWORLD_OK;
}

int GameWorld_ProcessDeferredDeletions(void) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    for (uint32_t i = 0; i < g_world_state.deletion_queue_count; i++) {
        GameWorld_RemoveObject(g_world_state.deletion_queue[i]);
    }
    
    g_world_state.deletion_queue_count = 0;
    
    return GAMEWORLD_OK;
}

int GameWorld_Update(void) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    // Rebuild quadtree periodically (simplified - every frame)
    BuildQuadtree();
    
    // Call update callback for each object
    if (g_world_state.update_callback) {
        for (uint32_t i = 0; i < g_world_state.object_count; i++) {
            if (g_world_state.objects[i].is_active) {
                g_world_state.update_callback(g_world_state.objects[i].object_id,
                                             g_world_state.callback_user_data);
            }
        }
    }
    
    return GAMEWORLD_OK;
}

uint32_t GameWorld_GetObjectCount(void) {
    if (!g_world_state.is_initialized) return 0;
    return g_world_state.object_count;
}

uint32_t GameWorld_GetQuadtreeDepth(void) {
    if (!g_world_state.is_initialized) return 0;
    return g_world_state.quadtree_depth;
}

uint32_t GameWorld_GetQuadtreeNodeCount(void) {
    if (!g_world_state.is_initialized) return 0;
    return g_world_state.quadtree_node_count;
}

int GameWorld_Clear(void) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    g_world_state.object_count = 0;
    g_world_state.deletion_queue_count = 0;
    BuildQuadtree();
    
    return GAMEWORLD_OK;
}

int GameWorld_RegisterUpdateCallback(WorldUpdateCallback callback, void* user_data) {
    if (!g_world_state.is_initialized) {
        SetError("World not initialized");
        return GAMEWORLD_ERROR_INVALID_PARAM;
    }
    
    g_world_state.update_callback = callback;
    g_world_state.callback_user_data = user_data;
    
    return GAMEWORLD_OK;
}

const char* GameWorld_GetError(void) {
    return g_world_state.error_message;
}
