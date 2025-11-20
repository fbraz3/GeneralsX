/**
 * @file d3d8_vulkan_drawing.h
 * @brief Phase 21: Draw Command System - Vulkan draw command recording and batching
 *
 * Implements DirectX 8 draw command emulation via Vulkan vkCmdDraw and vkCmdDrawIndexed,
 * including instancing support, multi-pass rendering, and draw call optimization.
 *
 * Handle range: 21000-21999
 */

#ifndef D3D8_VULKAN_DRAWING_H
#define D3D8_VULKAN_DRAWING_H

#include "d3d8_vulkan_types_compat.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// D3D Error codes
#define D3D_OK                      0
#define D3D_ERROR_INVALID_CALL      1

/**
 * @brief Draw command type enumeration
 */
typedef enum {
    D3D_DRAW_UNDEFINED = 0,
    D3D_DRAW_PRIMITIVES = 1,
    D3D_DRAW_INDEXED_PRIMITIVES = 2,
    D3D_DRAW_PRIMITIVES_UP = 3,        // Unaligned pointer (user memory)
    D3D_DRAW_INDEXED_PRIMITIVES_UP = 4, // Indexed with user memory
} D3DDrawType;

/**
 * @brief Draw primitive types (D3DPRIMITIVETYPE equivalent)
 */
typedef enum {
    D3D_PT_UNDEFINED = 0,
    D3D_PT_POINTLIST = 1,
    D3D_PT_LINELIST = 2,
    D3D_PT_LINESTRIP = 3,
    D3D_PT_TRIANGLELIST = 4,
    D3D_PT_TRIANGLESTRIP = 5,
    D3D_PT_TRIANGLEFAN = 6,
} D3DPrimitiveType;

/**
 * @brief Draw call batch descriptor
 * Groups multiple draw calls for optimization
 */
typedef struct {
    uint32_t handle;                // Batch handle (21000+)
    uint32_t draw_count;            // Number of draws in batch
    uint32_t total_vertices;        // Sum of all vertices
    uint32_t total_indices;         // Sum of all indices
    uint64_t vertex_buffer;         // Merged vertex buffer
    uint64_t index_buffer;          // Merged index buffer
    int is_dirty;                   // Needs re-recording
    int is_active;                  // Currently recording
} DrawBatch;

/**
 * @brief Single draw command within a batch
 */
typedef struct {
    D3DDrawType draw_type;
    D3DPrimitiveType prim_type;
    uint32_t vertex_count;          // For DrawPrimitives
    uint32_t start_vertex;          // Offset in VB
    uint32_t start_index;           // Offset in IB (for indexed)
    uint32_t prim_count;            // Triangle/line count
    uint32_t instance_count;        // For instanced draws
    uint32_t base_instance;         // First instance
    uint32_t base_vertex;           // Vertex offset
} DrawCommand;

/**
 * @brief Instancing parameters
 */
typedef struct {
    uint32_t handle;                // Instancing context handle
    uint32_t instance_count;
    uint32_t instance_stride;       // Bytes per instance
    uint64_t instance_buffer;       // VkBuffer
    int per_instance_data;          // Has per-instance data
} InstanceInfo;

/**
 * @brief Multi-pass rendering context
 */
typedef struct {
    uint32_t handle;                // Pass context handle
    uint32_t pass_count;
    uint32_t current_pass;
    uint32_t* pass_shaders;         // Array of shader handles
    uint32_t* pass_states;          // Array of render state handles
    int needs_recompile;
} RenderPass;

/**
 * @brief Draw call statistics
 */
typedef struct {
    uint64_t total_draw_calls;
    uint64_t total_vertices_drawn;
    uint64_t total_triangles_drawn;
    uint64_t batches_created;
    uint64_t batches_executed;
    uint32_t max_batch_size;
    uint32_t avg_batch_size;
    float batch_optimization_ratio;  // Reduction percentage
} DrawStats;

/**
 * @brief Initialize draw command system
 * @return Handle to draw system context (21000+), or D3D_ERROR_* on failure
 */
uint32_t D3D8_Drawing_Initialize(void);

/**
 * @brief Shutdown draw command system
 * @param handle Draw system handle
 * @return D3D_OK on success, error code otherwise
 */
int D3D8_Drawing_Shutdown(uint32_t handle);

/**
 * @brief Record a simple draw command (non-indexed)
 * @param batch_handle Batch to record into
 * @param prim_type Primitive type (triangles, etc.)
 * @param start_vertex First vertex index
 * @param vertex_count Number of vertices to draw
 * @return D3D_OK on success
 */
int D3D8_Drawing_DrawPrimitives(
    uint32_t batch_handle,
    D3DPrimitiveType prim_type,
    uint32_t start_vertex,
    uint32_t vertex_count
);

/**
 * @brief Record an indexed draw command
 * @param batch_handle Batch to record into
 * @param prim_type Primitive type
 * @param start_index First index in index buffer
 * @param index_count Number of indices to draw
 * @param start_vertex Vertex offset
 * @return D3D_OK on success
 */
int D3D8_Drawing_DrawIndexedPrimitives(
    uint32_t batch_handle,
    D3DPrimitiveType prim_type,
    uint32_t start_index,
    uint32_t index_count,
    uint32_t start_vertex
);

/**
 * @brief Create a new draw batch
 * @return Batch handle (21000+), or D3D_ERROR_* on failure
 */
uint32_t D3D8_Drawing_CreateBatch(void);

/**
 * @brief Begin recording commands into a batch
 * @param batch_handle Batch to record into
 * @return D3D_OK on success
 */
int D3D8_Drawing_BeginBatch(uint32_t batch_handle);

/**
 * @brief Finish recording commands into a batch
 * @param batch_handle Batch to close
 * @return D3D_OK on success
 */
int D3D8_Drawing_EndBatch(uint32_t batch_handle);

/**
 * @brief Execute a recorded batch
 * @param batch_handle Batch to execute
 * @return D3D_OK on success
 */
int D3D8_Drawing_ExecuteBatch(uint32_t batch_handle);

/**
 * @brief Setup instancing for draw calls
 * @param instance_count Number of instances
 * @param instance_buffer VkBuffer with instance data
 * @param instance_stride Bytes per instance
 * @return Instance context handle
 */
uint32_t D3D8_Drawing_SetupInstancing(
    uint32_t instance_count,
    uint64_t instance_buffer,
    uint32_t instance_stride
);

/**
 * @brief Draw instanced primitives
 * @param batch_handle Batch to record into
 * @param prim_type Primitive type
 * @param vertex_count Vertices per instance
 * @param instance_count Number of instances
 * @param instancing_handle Instance context from SetupInstancing
 * @return D3D_OK on success
 */
int D3D8_Drawing_DrawInstancedPrimitives(
    uint32_t batch_handle,
    D3DPrimitiveType prim_type,
    uint32_t vertex_count,
    uint32_t instance_count,
    uint32_t instancing_handle
);

/**
 * @brief Create a multi-pass rendering context
 * @param pass_count Number of rendering passes
 * @return Pass context handle
 */
uint32_t D3D8_Drawing_CreateRenderPass(uint32_t pass_count);

/**
 * @brief Set shader for a specific pass
 * @param pass_handle Pass context
 * @param pass_index Which pass (0-based)
 * @param shader_handle Shader to use
 * @return D3D_OK on success
 */
int D3D8_Drawing_SetPassShader(
    uint32_t pass_handle,
    uint32_t pass_index,
    uint32_t shader_handle
);

/**
 * @brief Set render state for a specific pass
 * @param pass_handle Pass context
 * @param pass_index Which pass (0-based)
 * @param state_handle Render state to use
 * @return D3D_OK on success
 */
int D3D8_Drawing_SetPassState(
    uint32_t pass_handle,
    uint32_t pass_index,
    uint32_t state_handle
);

/**
 * @brief Execute multi-pass rendering
 * @param batch_handle Batch with draw calls
 * @param pass_handle Pass context with shaders/states
 * @return D3D_OK on success
 */
int D3D8_Drawing_ExecuteMultiPass(
    uint32_t batch_handle,
    uint32_t pass_handle
);

/**
 * @brief Optimize and batch multiple draw calls
 * Analyzes draw calls and combines where possible to reduce CPU overhead
 * @param batch_handle Batch to optimize
 * @return Optimization ratio (1.0 = no optimization, 0.5 = 50% reduction)
 */
float D3D8_Drawing_OptimizeBatch(uint32_t batch_handle);

/**
 * @brief Sort draw calls by render state for better GPU cache utilization
 * @param batch_handle Batch to sort
 * @return D3D_OK on success
 */
int D3D8_Drawing_SortDrawCalls(uint32_t batch_handle);

/**
 * @brief Get information about a draw batch
 * @param batch_handle Batch to query
 * @param out_info Pointer to DrawBatch struct (filled on success)
 * @return D3D_OK on success
 */
int D3D8_Drawing_GetBatchInfo(uint32_t batch_handle, DrawBatch* out_info);

/**
 * @brief Get drawing statistics
 * @param out_stats Pointer to DrawStats struct (filled on success)
 * @return D3D_OK on success
 */
int D3D8_Drawing_GetStats(DrawStats* out_stats);

/**
 * @brief Clear cached drawing data
 * @return D3D_OK on success
 */
int D3D8_Drawing_ClearCache(void);

/**
 * @brief Validate drawing subsystem integrity
 * @return D3D_OK if valid, error code otherwise
 */
int D3D8_Drawing_Validate(void);

/**
 * @brief Get last error message
 * @return Error string (valid until next call)
 */
const char* D3D8_Drawing_GetError(void);

/**
 * @brief Reset drawing statistics
 * @return D3D_OK on success
 */
int D3D8_Drawing_ResetStats(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // D3D8_VULKAN_DRAWING_H
