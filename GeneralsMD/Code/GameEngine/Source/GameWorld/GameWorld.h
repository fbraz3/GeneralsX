/**
 * @file GameWorld.h
 * @brief Phase 28: World Management - Scene Graph and Spatial Query System
 *
 * Game world manager with spatial indexing (quadtree), radius queries, and frustum tests.
 */

#ifndef GAME_WORLD_H
#define GAME_WORLD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GAMEWORLD_OK                    0
#define GAMEWORLD_ERROR_INVALID_PARAM  -1
#define GAMEWORLD_ERROR_NOT_FOUND      -2
#define GAMEWORLD_ERROR_LIMIT_EXCEEDED -3

// Frustum plane
typedef struct {
    float a, b, c, d;  // ax + by + cz + d = 0
} FrustumPlane;

// View frustum for culling
typedef struct {
    FrustumPlane planes[6];  // near, far, left, right, top, bottom
} ViewFrustum;

// Bounding sphere
typedef struct {
    float x, y, z;
    float radius;
} BoundingSphere;

// Bounding box (AABB)
typedef struct {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
} BoundingBox;

// Quadtree node
typedef struct {
    float min_x, min_y;
    float max_x, max_y;
    uint32_t depth;
    uint32_t object_count;
    uint32_t children[4];  // NW, NE, SW, SE
} QuadtreeNode;

// Object in world
typedef struct {
    uint32_t object_id;
    float x, y, z;
    float radius;
    int is_active;
} WorldObject;

// Query result
typedef struct {
    uint32_t* object_ids;
    uint32_t count;
    uint32_t capacity;
} QueryResult;

// World update callback
typedef void (*WorldUpdateCallback)(uint32_t object_id, void* user_data);

/**
 * Initialize game world
 * @param world_width World width
 * @param world_height World height
 * @param max_objects Maximum objects in world
 * @return GAMEWORLD_OK on success
 */
int GameWorld_Initialize(float world_width, float world_height, uint32_t max_objects);

/**
 * Shutdown game world
 * @return GAMEWORLD_OK on success
 */
int GameWorld_Shutdown(void);

/**
 * Add object to world
 * @param object_id Object ID
 * @param x X position
 * @param y Y position
 * @param z Z position
 * @param radius Object radius
 * @return GAMEWORLD_OK on success
 */
int GameWorld_AddObject(uint32_t object_id, float x, float y, float z, float radius);

/**
 * Remove object from world
 * @param object_id Object ID
 * @return GAMEWORLD_OK on success
 */
int GameWorld_RemoveObject(uint32_t object_id);

/**
 * Update object position
 * @param object_id Object ID
 * @param x New X position
 * @param y New Y position
 * @param z New Z position
 * @return GAMEWORLD_OK on success
 */
int GameWorld_UpdateObjectPosition(uint32_t object_id, float x, float y, float z);

/**
 * Query objects in radius
 * @param center_x Center X position
 * @param center_y Center Y position
 * @param radius Query radius
 * @param result Query result array
 * @param max_results Maximum results to return
 * @return Number of objects found
 */
uint32_t GameWorld_QueryRadius(float center_x, float center_y, float radius,
                               uint32_t* result, uint32_t max_results);

/**
 * Query objects in box
 * @param min_x Bounding box min X
 * @param min_y Bounding box min Y
 * @param max_x Bounding box max X
 * @param max_y Bounding box max Y
 * @param result Query result array
 * @param max_results Maximum results to return
 * @return Number of objects found
 */
uint32_t GameWorld_QueryBox(float min_x, float min_y, float max_x, float max_y,
                           uint32_t* result, uint32_t max_results);

/**
 * Query objects in frustum (3D camera frustum)
 * @param frustum View frustum
 * @param result Query result array
 * @param max_results Maximum results to return
 * @return Number of objects found
 */
uint32_t GameWorld_QueryFrustum(const ViewFrustum* frustum,
                               uint32_t* result, uint32_t max_results);

/**
 * Find nearest object
 * @param center_x Center X position
 * @param center_y Center Y position
 * @param max_distance Maximum search distance
 * @param exclude_id Object ID to exclude from search
 * @return Nearest object ID, or 0 if none found
 */
uint32_t GameWorld_FindNearestObject(float center_x, float center_y,
                                     float max_distance, uint32_t exclude_id);

/**
 * Get object bounds
 * @param object_id Object ID
 * @param bounds Pointer to bounding sphere to receive data
 * @return GAMEWORLD_OK on success
 */
int GameWorld_GetObjectBounds(uint32_t object_id, BoundingSphere* bounds);

/**
 * Test sphere-sphere intersection
 * @param sphere1 First sphere
 * @param sphere2 Second sphere
 * @return 1 if intersecting, 0 if not
 */
int GameWorld_TestSphereSphere(const BoundingSphere* sphere1, const BoundingSphere* sphere2);

/**
 * Test sphere-box intersection
 * @param sphere Bounding sphere
 * @param box Bounding box
 * @return 1 if intersecting, 0 if not
 */
int GameWorld_TestSphereBox(const BoundingSphere* sphere, const BoundingBox* box);

/**
 * Test point in frustum
 * @param x Point X coordinate
 * @param y Point Y coordinate
 * @param z Point Z coordinate
 * @param frustum View frustum
 * @return 1 if point in frustum, 0 if not
 */
int GameWorld_TestPointInFrustum(float x, float y, float z, const ViewFrustum* frustum);

/**
 * Test sphere in frustum
 * @param sphere Bounding sphere
 * @param frustum View frustum
 * @return 1 if sphere in frustum, 0 if not
 */
int GameWorld_TestSphereInFrustum(const BoundingSphere* sphere, const ViewFrustum* frustum);

/**
 * Get visible objects from camera
 * @param frustum Camera frustum
 * @param result Visible object IDs
 * @param max_results Maximum results to return
 * @return Number of visible objects
 */
uint32_t GameWorld_GetVisibleObjects(const ViewFrustum* frustum,
                                     uint32_t* result, uint32_t max_results);

/**
 * Mark object for deletion (deferred)
 * @param object_id Object ID
 * @return GAMEWORLD_OK on success
 */
int GameWorld_MarkForDeletion(uint32_t object_id);

/**
 * Process deferred deletions
 * @return GAMEWORLD_OK on success
 */
int GameWorld_ProcessDeferredDeletions(void);

/**
 * Update world (rebuild spatial index, etc)
 * @return GAMEWORLD_OK on success
 */
int GameWorld_Update(void);

/**
 * Get object count
 * @return Total objects in world
 */
uint32_t GameWorld_GetObjectCount(void);

/**
 * Get quadtree depth
 * @return Current quadtree depth
 */
uint32_t GameWorld_GetQuadtreeDepth(void);

/**
 * Get quadtree node count
 * @return Number of quadtree nodes
 */
uint32_t GameWorld_GetQuadtreeNodeCount(void);

/**
 * Clear world
 * @return GAMEWORLD_OK on success
 */
int GameWorld_Clear(void);

/**
 * Register world update callback
 * @param callback Callback function
 * @param user_data User data to pass to callback
 * @return GAMEWORLD_OK on success
 */
int GameWorld_RegisterUpdateCallback(WorldUpdateCallback callback, void* user_data);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* GameWorld_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_WORLD_H
