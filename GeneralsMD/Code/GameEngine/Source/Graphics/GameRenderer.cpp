/**
 * @file GameRenderer.cpp
 * @brief Phase 30: Rendering Integration - Game renderer implementation
 */

#include "GameRenderer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define ERROR_BUFFER_SIZE 256
#define MAX_MATERIALS 256
#define MAX_RENDER_COMMANDS 10000

// Material storage
typedef struct {
    int is_valid;
    float color_r, color_g, color_b, color_a;
    float specular;
    float roughness;
    uint32_t texture_id;
} MaterialData;

// Object render state
typedef struct {
    int is_enabled;
    float depth_bias;
} ObjectRenderState;

// Renderer state
typedef struct {
    int is_initialized;
    uint32_t max_objects;
    
    // Matrices
    float view_matrix[16];
    float projection_matrix[16];
    float camera_x, camera_y, camera_z;
    
    // Materials
    MaterialData materials[MAX_MATERIALS];
    uint32_t material_count;
    
    // Render commands
    RenderCommand* queue;
    uint32_t queue_size;
    uint32_t queue_capacity;
    
    // Object render states
    ObjectRenderState* object_states;
    
    // Statistics
    RenderStats stats;
    
    // Settings
    int transparency_sorting_enabled;
    uint32_t debug_flags;
    
    char error_message[ERROR_BUFFER_SIZE];
} RendererState;

static RendererState g_renderer_state = {0};

// Set error message
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_renderer_state.error_message, ERROR_BUFFER_SIZE, format, args);
    va_end(args);
}

// Initialize identity matrix
static void InitIdentityMatrix(float* matrix) {
    memset(matrix, 0, 16 * sizeof(float));
    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;
}

// Compare function for depth sorting (back to front)
static int CompareDepth(const void* a, const void* b) {
    const RenderCommand* cmd_a = (const RenderCommand*)a;
    const RenderCommand* cmd_b = (const RenderCommand*)b;
    
    if (cmd_a->depth > cmd_b->depth) return -1;
    if (cmd_a->depth < cmd_b->depth) return 1;
    return 0;
}

// Calculate depth from position and camera
static float CalculateDepth(const float* position) {
    float dx = position[0] - g_renderer_state.camera_x;
    float dy = position[1] - g_renderer_state.camera_y;
    float dz = position[2] - g_renderer_state.camera_z;
    return dx * dx + dy * dy + dz * dz;  // Distance squared
}

int GameRenderer_Initialize(uint32_t max_objects) {
    if (g_renderer_state.is_initialized) {
        SetError("Renderer already initialized");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    memset(&g_renderer_state, 0, sizeof(RendererState));
    
    g_renderer_state.max_objects = max_objects;
    
    // Allocate render queue
    g_renderer_state.queue = (RenderCommand*)malloc(MAX_RENDER_COMMANDS * sizeof(RenderCommand));
    if (!g_renderer_state.queue) {
        SetError("Failed to allocate render queue");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    // Allocate object states
    g_renderer_state.object_states = (ObjectRenderState*)malloc(max_objects * sizeof(ObjectRenderState));
    if (!g_renderer_state.object_states) {
        free(g_renderer_state.queue);
        SetError("Failed to allocate object states");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    // Initialize object states
    for (uint32_t i = 0; i < max_objects; i++) {
        g_renderer_state.object_states[i].is_enabled = 1;
        g_renderer_state.object_states[i].depth_bias = 0.0f;
    }
    
    g_renderer_state.queue_capacity = MAX_RENDER_COMMANDS;
    g_renderer_state.queue_size = 0;
    
    // Initialize matrices
    InitIdentityMatrix(g_renderer_state.view_matrix);
    InitIdentityMatrix(g_renderer_state.projection_matrix);
    
    g_renderer_state.camera_x = 0.0f;
    g_renderer_state.camera_y = 0.0f;
    g_renderer_state.camera_z = 0.0f;
    
    g_renderer_state.transparency_sorting_enabled = 1;
    g_renderer_state.debug_flags = DEBUG_RENDER_NONE;
    
    // Initialize materials
    g_renderer_state.material_count = 0;
    memset(g_renderer_state.materials, 0, sizeof(g_renderer_state.materials));
    
    // Initialize statistics
    memset(&g_renderer_state.stats, 0, sizeof(RenderStats));
    
    g_renderer_state.is_initialized = 1;
    
    return GAMERENDERER_OK;
}

int GameRenderer_Shutdown(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    free(g_renderer_state.queue);
    free(g_renderer_state.object_states);
    
    memset(&g_renderer_state, 0, sizeof(RendererState));
    
    return GAMERENDERER_OK;
}

int GameRenderer_BeginFrame(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    g_renderer_state.queue_size = 0;
    memset(&g_renderer_state.stats, 0, sizeof(RenderStats));
    
    return GAMERENDERER_OK;
}

int GameRenderer_EndFrame(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    // Count visible objects
    g_renderer_state.stats.visible_objects = g_renderer_state.queue_size;
    
    // Sort by depth if transparency sorting enabled
    if (g_renderer_state.transparency_sorting_enabled) {
        GameRenderer_SortByDepth();
    }
    
    // Batch by material
    g_renderer_state.stats.batches_created = GameRenderer_BatchByMaterial();
    
    // Simulate draw calls (one per batch)
    g_renderer_state.stats.draw_calls = g_renderer_state.stats.batches_created;
    
    // Calculate triangle/vertex counts (simplified)
    g_renderer_state.stats.vertices_rendered = g_renderer_state.stats.visible_objects * 6;
    g_renderer_state.stats.triangles_rendered = g_renderer_state.stats.visible_objects * 2;
    
    return GAMERENDERER_OK;
}

int GameRenderer_SubmitObject(uint32_t object_id, const Transform* transform, const Material* material) {
    if (!g_renderer_state.is_initialized || !transform || !material) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    if (object_id >= g_renderer_state.max_objects) {
        SetError("Object ID out of range");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    // Check if object is enabled
    if (!g_renderer_state.object_states[object_id].is_enabled) {
        return GAMERENDERER_OK;  // Skip disabled objects
    }
    
    // Queue render command
    if (g_renderer_state.queue_size >= g_renderer_state.queue_capacity) {
        SetError("Render queue full");
        return GAMERENDERER_ERROR_NO_SPACE;
    }
    
    RenderCommand* cmd = &g_renderer_state.queue[g_renderer_state.queue_size++];
    cmd->object_id = object_id;
    memcpy(&cmd->transform, transform, sizeof(Transform));
    memcpy(&cmd->material, material, sizeof(Material));
    
    // Calculate depth
    float pos[3] = {transform->position_x, transform->position_y, transform->position_z};
    cmd->depth = CalculateDepth(pos) + g_renderer_state.object_states[object_id].depth_bias;
    
    // Check if transparent
    cmd->is_transparent = material->color_a < 0.99f ? 1 : 0;
    
    return GAMERENDERER_OK;
}

int GameRenderer_QueueCommand(const RenderCommand* command) {
    if (!g_renderer_state.is_initialized || !command) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    if (g_renderer_state.queue_size >= g_renderer_state.queue_capacity) {
        SetError("Render queue full");
        return GAMERENDERER_ERROR_NO_SPACE;
    }
    
    memcpy(&g_renderer_state.queue[g_renderer_state.queue_size++], command, sizeof(RenderCommand));
    
    return GAMERENDERER_OK;
}

int GameRenderer_ClearQueue(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    g_renderer_state.queue_size = 0;
    
    return GAMERENDERER_OK;
}

int GameRenderer_SetViewMatrix(const float* view_matrix) {
    if (!g_renderer_state.is_initialized || !view_matrix) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    memcpy(g_renderer_state.view_matrix, view_matrix, 16 * sizeof(float));
    
    return GAMERENDERER_OK;
}

int GameRenderer_SetProjectionMatrix(const float* projection_matrix) {
    if (!g_renderer_state.is_initialized || !projection_matrix) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    memcpy(g_renderer_state.projection_matrix, projection_matrix, 16 * sizeof(float));
    
    return GAMERENDERER_OK;
}

int GameRenderer_SetCameraPosition(float x, float y, float z) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    g_renderer_state.camera_x = x;
    g_renderer_state.camera_y = y;
    g_renderer_state.camera_z = z;
    
    return GAMERENDERER_OK;
}

int GameRenderer_GetCameraPosition(float* x, float* y, float* z) {
    if (!g_renderer_state.is_initialized || !x || !y || !z) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    *x = g_renderer_state.camera_x;
    *y = g_renderer_state.camera_y;
    *z = g_renderer_state.camera_z;
    
    return GAMERENDERER_OK;
}

int GameRenderer_CreateMaterial(uint32_t material_id, float r, float g, float b, float a) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (material_id >= MAX_MATERIALS) {
        SetError("Material ID out of range");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    MaterialData* mat = &g_renderer_state.materials[material_id];
    mat->is_valid = 1;
    mat->color_r = r;
    mat->color_g = g;
    mat->color_b = b;
    mat->color_a = a;
    mat->specular = 0.5f;
    mat->roughness = 0.5f;
    mat->texture_id = 0;
    
    if (material_id >= g_renderer_state.material_count) {
        g_renderer_state.material_count = material_id + 1;
    }
    
    return GAMERENDERER_OK;
}

int GameRenderer_UpdateMaterial(uint32_t material_id, float specular, float roughness) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (material_id >= MAX_MATERIALS || !g_renderer_state.materials[material_id].is_valid) {
        SetError("Invalid material ID");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    MaterialData* mat = &g_renderer_state.materials[material_id];
    mat->specular = specular;
    mat->roughness = roughness;
    
    return GAMERENDERER_OK;
}

int GameRenderer_SetMaterialTexture(uint32_t material_id, uint32_t texture_id) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (material_id >= MAX_MATERIALS || !g_renderer_state.materials[material_id].is_valid) {
        SetError("Invalid material ID");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    MaterialData* mat = &g_renderer_state.materials[material_id];
    mat->texture_id = texture_id;
    
    return GAMERENDERER_OK;
}

int GameRenderer_SortByDepth(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (g_renderer_state.queue_size > 0) {
        qsort(g_renderer_state.queue, g_renderer_state.queue_size, sizeof(RenderCommand), CompareDepth);
    }
    
    return GAMERENDERER_OK;
}

uint32_t GameRenderer_BatchByMaterial(void) {
    if (!g_renderer_state.is_initialized) {
        return 0;
    }
    
    uint32_t batch_count = 0;
    uint32_t current_material = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < g_renderer_state.queue_size; i++) {
        uint32_t mat_id = g_renderer_state.queue[i].material.material_id;
        
        if (mat_id != current_material) {
            batch_count++;
            current_material = mat_id;
        }
    }
    
    return batch_count;
}

int GameRenderer_SetTransparencySorting(int enabled) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    g_renderer_state.transparency_sorting_enabled = enabled ? 1 : 0;
    
    return GAMERENDERER_OK;
}

int GameRenderer_IsTransparentySortingEnabled(void) {
    if (!g_renderer_state.is_initialized) return 0;
    return g_renderer_state.transparency_sorting_enabled;
}

int GameRenderer_SetDebugFlags(uint32_t flags) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    g_renderer_state.debug_flags = flags;
    
    return GAMERENDERER_OK;
}

uint32_t GameRenderer_GetDebugFlags(void) {
    if (!g_renderer_state.is_initialized) return 0;
    return g_renderer_state.debug_flags;
}

int GameRenderer_RenderDebugBounds(uint32_t object_id, const Transform* transform, uint32_t color) {
    if (!g_renderer_state.is_initialized || !transform) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    if (!(g_renderer_state.debug_flags & DEBUG_RENDER_BOUNDS)) {
        return GAMERENDERER_OK;  // Debug bounds not enabled
    }
    
    // Bounds would be rendered here (simplified)
    
    return GAMERENDERER_OK;
}

int GameRenderer_RenderDebugLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t color) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    // Debug lines would be rendered here (simplified)
    
    return GAMERENDERER_OK;
}

int GameRenderer_GetStats(RenderStats* stats) {
    if (!g_renderer_state.is_initialized || !stats) {
        SetError("Invalid parameters");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    memcpy(stats, &g_renderer_state.stats, sizeof(RenderStats));
    
    return GAMERENDERER_OK;
}

int GameRenderer_ResetStats(void) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    memset(&g_renderer_state.stats, 0, sizeof(RenderStats));
    
    return GAMERENDERER_OK;
}

int GameRenderer_SetObjectEnabled(uint32_t object_id, int enabled) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (object_id >= g_renderer_state.max_objects) {
        SetError("Object ID out of range");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    g_renderer_state.object_states[object_id].is_enabled = enabled ? 1 : 0;
    
    return GAMERENDERER_OK;
}

int GameRenderer_IsObjectEnabled(uint32_t object_id) {
    if (!g_renderer_state.is_initialized) return 0;
    
    if (object_id >= g_renderer_state.max_objects) {
        return 0;
    }
    
    return g_renderer_state.object_states[object_id].is_enabled;
}

int GameRenderer_SetObjectDepthBias(uint32_t object_id, float depth_bias) {
    if (!g_renderer_state.is_initialized) {
        SetError("Renderer not initialized");
        return GAMERENDERER_ERROR_NOT_INIT;
    }
    
    if (object_id >= g_renderer_state.max_objects) {
        SetError("Object ID out of range");
        return GAMERENDERER_ERROR_INVALID;
    }
    
    g_renderer_state.object_states[object_id].depth_bias = depth_bias;
    
    return GAMERENDERER_OK;
}

uint32_t GameRenderer_GetQueueSize(void) {
    if (!g_renderer_state.is_initialized) return 0;
    return g_renderer_state.queue_size;
}

uint32_t GameRenderer_GetQueueCapacity(void) {
    if (!g_renderer_state.is_initialized) return 0;
    return g_renderer_state.queue_capacity;
}

const char* GameRenderer_GetError(void) {
    return g_renderer_state.error_message;
}
