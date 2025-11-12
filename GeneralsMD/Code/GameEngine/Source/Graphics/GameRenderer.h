/**
 * @file GameRenderer.h
 * @brief Phase 30: Rendering Integration - Vulkan-based rendering system for game objects
 *
 * Game object rendering with transforms, material batching, transparency sorting, and debug visualization.
 */

#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GAMERENDERER_OK                0
#define GAMERENDERER_ERROR_INVALID     -1
#define GAMERENDERER_ERROR_NOT_INIT    -2
#define GAMERENDERER_ERROR_NO_SPACE    -3

// Transform structure
typedef struct {
    float position_x, position_y, position_z;
    float rotation_x, rotation_y, rotation_z;
    float scale_x, scale_y, scale_z;
} Transform;

// Material definition
typedef struct {
    uint32_t material_id;
    float color_r, color_g, color_b, color_a;
    float specular;
    float roughness;
    uint32_t texture_id;
} Material;

// Render command
typedef struct {
    uint32_t object_id;
    Transform transform;
    Material material;
    float depth;  // For sorting
    int is_transparent;
} RenderCommand;

// Render statistics
typedef struct {
    uint32_t total_objects;
    uint32_t visible_objects;
    uint32_t draw_calls;
    uint32_t vertices_rendered;
    uint32_t triangles_rendered;
    uint32_t batches_created;
    float render_time_ms;
} RenderStats;

// Debug visualization flags
#define DEBUG_RENDER_NONE       0x0
#define DEBUG_RENDER_WIREFRAME  0x1
#define DEBUG_RENDER_NORMALS    0x2
#define DEBUG_RENDER_BOUNDS     0x4
#define DEBUG_RENDER_GRID       0x8
#define DEBUG_RENDER_SHADOWS    0x10

/**
 * Initialize renderer
 * @param max_objects Maximum objects that can be rendered
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_Initialize(uint32_t max_objects);

/**
 * Shutdown renderer
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_Shutdown(void);

/**
 * Prepare render frame
 * Must be called before queuing render commands
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_BeginFrame(void);

/**
 * Flush queued render commands
 * Must be called after all render commands queued
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_EndFrame(void);

/**
 * Submit render command for object
 * @param object_id Object ID to render
 * @param transform Object transform
 * @param material Object material
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SubmitObject(uint32_t object_id, const Transform* transform, const Material* material);

/**
 * Queue render command
 * @param command Render command to queue
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_QueueCommand(const RenderCommand* command);

/**
 * Clear render queue
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_ClearQueue(void);

/**
 * Set camera view matrix
 * @param view_matrix 4x4 view matrix (row-major, 16 floats)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetViewMatrix(const float* view_matrix);

/**
 * Set camera projection matrix
 * @param projection_matrix 4x4 projection matrix (row-major, 16 floats)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetProjectionMatrix(const float* projection_matrix);

/**
 * Set camera position
 * @param x Camera X position
 * @param y Camera Y position
 * @param z Camera Z position
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetCameraPosition(float x, float y, float z);

/**
 * Get camera position
 * @param x Pointer to receive X position
 * @param y Pointer to receive Y position
 * @param z Pointer to receive Z position
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_GetCameraPosition(float* x, float* y, float* z);

/**
 * Create material
 * @param material_id Material ID to create
 * @param r Red channel (0.0-1.0)
 * @param g Green channel (0.0-1.0)
 * @param b Blue channel (0.0-1.0)
 * @param a Alpha channel (0.0-1.0)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_CreateMaterial(uint32_t material_id, float r, float g, float b, float a);

/**
 * Update material properties
 * @param material_id Material ID to update
 * @param specular Specular value
 * @param roughness Roughness value
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_UpdateMaterial(uint32_t material_id, float specular, float roughness);

/**
 * Set material texture
 * @param material_id Material ID
 * @param texture_id Texture ID
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetMaterialTexture(uint32_t material_id, uint32_t texture_id);

/**
 * Sort render commands by depth (back-to-front for transparency)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SortByDepth(void);

/**
 * Batch render commands by material
 * @return Number of batches created
 */
uint32_t GameRenderer_BatchByMaterial(void);

/**
 * Enable/disable transparency sorting
 * @param enabled 1 to enable, 0 to disable
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetTransparencySorting(int enabled);

/**
 * Check if transparency sorting is enabled
 * @return 1 if enabled, 0 if disabled
 */
int GameRenderer_IsTransparentySortingEnabled(void);

/**
 * Set debug visualization flags
 * @param flags Debug flags (combination of DEBUG_RENDER_*)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetDebugFlags(uint32_t flags);

/**
 * Get debug visualization flags
 * @return Current debug flags
 */
uint32_t GameRenderer_GetDebugFlags(void);

/**
 * Render debug bounds for object
 * @param object_id Object ID
 * @param transform Object transform
 * @param color RGBA color (8 bits per channel)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_RenderDebugBounds(uint32_t object_id, const Transform* transform, uint32_t color);

/**
 * Render debug line
 * @param x1 Start X
 * @param y1 Start Y
 * @param z1 Start Z
 * @param x2 End X
 * @param y2 End Y
 * @param z2 End Z
 * @param color RGBA color (8 bits per channel)
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_RenderDebugLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t color);

/**
 * Get render statistics
 * @param stats Pointer to stats structure to fill
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_GetStats(RenderStats* stats);

/**
 * Reset render statistics
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_ResetStats(void);

/**
 * Enable/disable object rendering
 * @param object_id Object ID
 * @param enabled 1 to enable, 0 to disable
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetObjectEnabled(uint32_t object_id, int enabled);

/**
 * Check if object is enabled for rendering
 * @param object_id Object ID
 * @return 1 if enabled, 0 if disabled
 */
int GameRenderer_IsObjectEnabled(uint32_t object_id);

/**
 * Set object depth bias for depth sorting
 * @param object_id Object ID
 * @param depth_bias Depth bias value
 * @return GAMERENDERER_OK on success
 */
int GameRenderer_SetObjectDepthBias(uint32_t object_id, float depth_bias);

/**
 * Get render queue size
 * @return Number of queued render commands
 */
uint32_t GameRenderer_GetQueueSize(void);

/**
 * Get maximum render queue capacity
 * @return Maximum capacity
 */
uint32_t GameRenderer_GetQueueCapacity(void);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* GameRenderer_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_RENDERER_H
