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
**  FILE: d3d8_vulkan_renderloop.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Render Loop & Game Loop Integration
**
**  DESCRIPTION:
**    Game loop and render loop coordination, frame pacing, GPU/CPU sync,
**    frame statistics, BeginFrame/EndFrame management.
**
**    Phase 17: Render Loop Integration - Header Definition
**
******************************************************************************/

#ifndef D3D8_VULKAN_RENDERLOOP_H
#define D3D8_VULKAN_RENDERLOOP_H

#include <stdint.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_RENDERLOOP_STATE
 * @brief Render loop state machine
 */
typedef enum {
    D3D8_VULKAN_LOOP_STATE_UNINITIALIZED = 0x0,
    D3D8_VULKAN_LOOP_STATE_IDLE = 0x1,
    D3D8_VULKAN_LOOP_STATE_UPDATING = 0x2,
    D3D8_VULKAN_LOOP_STATE_RENDERING = 0x3,
    D3D8_VULKAN_LOOP_STATE_PRESENTING = 0x4,
    D3D8_VULKAN_LOOP_STATE_SHUTDOWN = 0x5,
} D3D8_VULKAN_RENDERLOOP_STATE;

/**
 * @enum D3D8_VULKAN_SYNC_MODE
 * @brief GPU/CPU synchronization mode
 */
typedef enum {
    D3D8_VULKAN_SYNC_IMMEDIATE = 0x0,  /* No sync, max FPS */
    D3D8_VULKAN_SYNC_VSYNC = 0x1,      /* Vertical sync */
    D3D8_VULKAN_SYNC_TRIPLE_BUFFER = 0x2,  /* Triple buffering */
    D3D8_VULKAN_SYNC_VARIABLE_RATE = 0x3,  /* Variable rate sync */
} D3D8_VULKAN_SYNC_MODE;

/**
 * @enum D3D8_VULKAN_FRAME_PHASE
 * @brief Frame phase enumeration
 */
typedef enum {
    D3D8_VULKAN_PHASE_BEGIN = 0x0,
    D3D8_VULKAN_PHASE_UPDATE = 0x1,
    D3D8_VULKAN_PHASE_RENDER = 0x2,
    D3D8_VULKAN_PHASE_PRESENT = 0x3,
    D3D8_VULKAN_PHASE_END = 0x4,
} D3D8_VULKAN_FRAME_PHASE;

/* ============================================================================
 * Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_RENDERLOOP_CONFIG
 * @brief Render loop configuration
 */
typedef struct {
    uint32_t target_fps;
    D3D8_VULKAN_SYNC_MODE sync_mode;
    uint32_t max_frames_in_flight;
    uint32_t enable_statistics;
    uint32_t enable_profiling;
} D3D8_VULKAN_RENDERLOOP_CONFIG;

/**
 * @struct D3D8_VULKAN_FRAME_STATS
 * @brief Per-frame statistics
 */
typedef struct {
    uint32_t frame_number;
    double frame_time;
    double delta_time;
    double gpu_time;
    double cpu_time;
    uint32_t fps;
    uint32_t draw_calls;
    uint32_t vertices_rendered;
    uint32_t triangles_rendered;
} D3D8_VULKAN_FRAME_STATS;

/**
 * @struct D3D8_VULKAN_RENDERLOOP_HANDLE
 * @brief Opaque render loop handle
 */
typedef struct {
    uint32_t id;
    uint32_t version;
} D3D8_VULKAN_RENDERLOOP_HANDLE;

/**
 * @struct D3D8_VULKAN_RENDERLOOP_INFO
 * @brief Render loop information
 */
typedef struct {
    D3D8_VULKAN_RENDERLOOP_STATE state;
    uint32_t frame_count;
    double elapsed_time;
    double average_fps;
    D3D8_VULKAN_FRAME_STATS current_frame;
    D3D8_VULKAN_FRAME_STATS last_frame;
} D3D8_VULKAN_RENDERLOOP_INFO;

/* ============================================================================
 * Core Render Loop API (14 Functions)
 * ============================================================================ */

/**
 * Initialize render loop system
 */
D3D8_VULKAN_RENDERLOOP_HANDLE D3D8_Vulkan_InitializeRenderLoop(
    VkDevice device,
    const D3D8_VULKAN_RENDERLOOP_CONFIG* config);

/**
 * Shutdown render loop system
 */
void D3D8_Vulkan_ShutdownRenderLoop(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Begin frame - called at start of each game tick
 */
int D3D8_Vulkan_BeginFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    double delta_time);

/**
 * End frame - called at end of each game tick
 */
int D3D8_Vulkan_EndFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Update phase - game logic execution
 */
int D3D8_Vulkan_UpdatePhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Render phase - GPU rendering
 */
int D3D8_Vulkan_RenderPhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Present frame to screen
 */
int D3D8_Vulkan_PresentFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Wait for GPU to catch up (synchronization)
 */
int D3D8_Vulkan_WaitForGPU(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    uint32_t timeout_ms);

/**
 * Get current render loop information
 */
int D3D8_Vulkan_GetRenderLoopInfo(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_RENDERLOOP_INFO* out_info);

/**
 * Set frame pacing target FPS
 */
int D3D8_Vulkan_SetTargetFPS(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    uint32_t fps);

/**
 * Set synchronization mode
 */
int D3D8_Vulkan_SetSyncMode(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_SYNC_MODE mode);

/**
 * Get current frame phase
 */
D3D8_VULKAN_FRAME_PHASE D3D8_Vulkan_GetCurrentPhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop);

/**
 * Get frame statistics
 */
int D3D8_Vulkan_GetFrameStats(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_FRAME_STATS* out_stats);

/**
 * Get render loop error message
 */
const char* D3D8_Vulkan_GetRenderLoopError(void);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_RENDERLOOP_H */
