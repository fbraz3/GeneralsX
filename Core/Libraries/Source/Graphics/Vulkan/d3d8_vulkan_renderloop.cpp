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
**  FILE: d3d8_vulkan_renderloop.cpp
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
**    Phase 17: Render Loop Integration - Implementation
**
******************************************************************************/

#include "d3d8_vulkan_renderloop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* ============================================================================
 * Internal Constants & Configuration
 * ============================================================================ */

#define D3D8_VULKAN_MAX_RENDERLOOPS 4
#define D3D8_VULKAN_ERROR_MESSAGE_SIZE 512
#define D3D8_VULKAN_MAX_FRAMES_IN_FLIGHT 3

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

/**
 * @struct RenderLoopEntry
 * @brief Internal render loop cache entry
 */
typedef struct {
    uint32_t id;
    uint32_t version;
    D3D8_VULKAN_RENDERLOOP_STATE state;
    uint32_t frame_count;
    double elapsed_time;
    double last_frame_time;
    double current_frame_time;
    uint32_t target_fps;
    D3D8_VULKAN_SYNC_MODE sync_mode;
    D3D8_VULKAN_FRAME_PHASE current_phase;
    uint32_t frames_in_flight;
    uint32_t max_frames_in_flight;
    uint32_t enable_statistics;
    uint32_t enable_profiling;
    D3D8_VULKAN_FRAME_STATS current_stats;
    D3D8_VULKAN_FRAME_STATS last_stats;
    double gpu_time_accumulated;
    double cpu_time_accumulated;
    uint32_t draw_calls;
    uint32_t vertices_rendered;
    uint32_t triangles_rendered;
    uint32_t allocated;
} RenderLoopEntry;

/* ============================================================================
 * Global State & Caches
 * ============================================================================ */

static RenderLoopEntry g_renderloop_cache[D3D8_VULKAN_MAX_RENDERLOOPS];
static uint32_t g_renderloop_counter = 12000;
static char g_last_error[D3D8_VULKAN_ERROR_MESSAGE_SIZE];

/* ============================================================================
 * Utility Functions (Internal)
 * ============================================================================ */

/**
 * @fn static uint32_t _find_free_renderloop_slot(void)
 * @brief Find next free slot in render loop cache
 */
static uint32_t _find_free_renderloop_slot(void)
{
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_RENDERLOOPS; i++) {
        if (!g_renderloop_cache[i].allocated) {
            return i;
        }
    }
    return D3D8_VULKAN_MAX_RENDERLOOPS;
}

/**
 * @fn static int _is_valid_renderloop_handle(D3D8_VULKAN_RENDERLOOP_HANDLE handle)
 * @brief Validate render loop handle
 */
static int _is_valid_renderloop_handle(D3D8_VULKAN_RENDERLOOP_HANDLE handle)
{
    if (handle.id < 12000 || handle.id >= 12000 + D3D8_VULKAN_MAX_RENDERLOOPS) {
        return 0;
    }
    uint32_t index = handle.id - 12000;
    if (index >= D3D8_VULKAN_MAX_RENDERLOOPS) {
        return 0;
    }
    if (!g_renderloop_cache[index].allocated) {
        return 0;
    }
    if (g_renderloop_cache[index].version != handle.version) {
        return 0;
    }
    return 1;
}

/**
 * @fn static RenderLoopEntry* _get_renderloop_entry(D3D8_VULKAN_RENDERLOOP_HANDLE handle)
 * @brief Get render loop entry from handle
 */
static RenderLoopEntry* _get_renderloop_entry(D3D8_VULKAN_RENDERLOOP_HANDLE handle)
{
    if (!_is_valid_renderloop_handle(handle)) {
        return NULL;
    }
    uint32_t index = handle.id - 12000;
    return &g_renderloop_cache[index];
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
 * @fn static double _get_time_seconds(void)
 * @brief Get current time in seconds
 */
static double _get_time_seconds(void)
{
    static double start_time = -1.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    double current_time = ts.tv_sec + ts.tv_nsec / 1e9;
    
    if (start_time < 0.0) {
        start_time = current_time;
    }
    
    return current_time - start_time;
}

/* ============================================================================
 * Render Loop Management API Implementation (14 Functions)
 * ============================================================================ */

D3D8_VULKAN_RENDERLOOP_HANDLE D3D8_Vulkan_InitializeRenderLoop(
    VkDevice device,
    const D3D8_VULKAN_RENDERLOOP_CONFIG* config)
{
    D3D8_VULKAN_RENDERLOOP_HANDLE invalid_handle = {0, 0};

    if (!device || !config) {
        _set_error("InitializeRenderLoop: Invalid parameters");
        return invalid_handle;
    }

    if (config->target_fps == 0) {
        _set_error("InitializeRenderLoop: target_fps must be > 0");
        return invalid_handle;
    }

    uint32_t slot = _find_free_renderloop_slot();
    if (slot >= D3D8_VULKAN_MAX_RENDERLOOPS) {
        _set_error("InitializeRenderLoop: Cache full");
        return invalid_handle;
    }

    RenderLoopEntry* entry = &g_renderloop_cache[slot];
    memset(entry, 0, sizeof(RenderLoopEntry));

    entry->id = g_renderloop_counter++;
    entry->version = 1;
    entry->state = D3D8_VULKAN_LOOP_STATE_IDLE;
    entry->frame_count = 0;
    entry->elapsed_time = 0.0;
    entry->target_fps = config->target_fps;
    entry->sync_mode = config->sync_mode;
    entry->max_frames_in_flight = config->max_frames_in_flight > 0 ? 
        config->max_frames_in_flight : D3D8_VULKAN_MAX_FRAMES_IN_FLIGHT;
    entry->enable_statistics = config->enable_statistics;
    entry->enable_profiling = config->enable_profiling;
    entry->current_phase = D3D8_VULKAN_PHASE_BEGIN;
    entry->allocated = 1;

    printf("D3D8_Vulkan_RenderLoop: InitializeRenderLoop (ID: %u, FPS: %u, Mode: %u)\n",
           entry->id, entry->target_fps, entry->sync_mode);

    D3D8_VULKAN_RENDERLOOP_HANDLE handle = {entry->id, entry->version};
    return handle;
}

void D3D8_Vulkan_ShutdownRenderLoop(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("ShutdownRenderLoop: Invalid handle");
        return;
    }

    printf("D3D8_Vulkan_RenderLoop: ShutdownRenderLoop (ID: %u, Frames: %u)\n",
           entry->id, entry->frame_count);

    entry->state = D3D8_VULKAN_LOOP_STATE_SHUTDOWN;
    entry->allocated = 0;
    entry->version++;
    memset(entry, 0, sizeof(RenderLoopEntry));
}

int D3D8_Vulkan_BeginFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    double delta_time)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("BeginFrame: Invalid handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_LOOP_STATE_IDLE && 
        entry->state != D3D8_VULKAN_LOOP_STATE_RENDERING) {
        _set_error("BeginFrame: Invalid state (expected IDLE or RENDERING)");
        return -1;
    }

    entry->state = D3D8_VULKAN_LOOP_STATE_UPDATING;
    entry->current_phase = D3D8_VULKAN_PHASE_BEGIN;
    entry->last_frame_time = entry->current_frame_time;
    entry->current_frame_time = _get_time_seconds();
    entry->elapsed_time += delta_time;

    if (entry->enable_statistics) {
        entry->current_stats.frame_number = entry->frame_count;
        entry->current_stats.delta_time = delta_time;
    }

    return 0;
}

int D3D8_Vulkan_EndFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("EndFrame: Invalid handle");
        return -1;
    }

    entry->state = D3D8_VULKAN_LOOP_STATE_IDLE;
    entry->current_phase = D3D8_VULKAN_PHASE_END;
    entry->frame_count++;

    if (entry->enable_statistics) {
        double frame_time = _get_time_seconds() - entry->current_frame_time;
        entry->current_stats.frame_time = frame_time;
        entry->current_stats.fps = (uint32_t)(1.0 / (frame_time > 0.001 ? frame_time : 0.001));
        
        memcpy(&entry->last_stats, &entry->current_stats, sizeof(D3D8_VULKAN_FRAME_STATS));
    }

    return 0;
}

int D3D8_Vulkan_UpdatePhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("UpdatePhase: Invalid handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_LOOP_STATE_UPDATING) {
        _set_error("UpdatePhase: Invalid state (expected UPDATING)");
        return -1;
    }

    entry->current_phase = D3D8_VULKAN_PHASE_UPDATE;

    if (entry->enable_profiling) {
        double cpu_start = _get_time_seconds();
        // CPU profiling would occur here
        double cpu_time = _get_time_seconds() - cpu_start;
        entry->cpu_time_accumulated += cpu_time;
    }

    return 0;
}

int D3D8_Vulkan_RenderPhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("RenderPhase: Invalid handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_LOOP_STATE_UPDATING) {
        _set_error("RenderPhase: Invalid state (expected UPDATING)");
        return -1;
    }

    entry->state = D3D8_VULKAN_LOOP_STATE_RENDERING;
    entry->current_phase = D3D8_VULKAN_PHASE_RENDER;

    if (entry->enable_profiling) {
        double gpu_start = _get_time_seconds();
        // GPU profiling would occur here
        double gpu_time = _get_time_seconds() - gpu_start;
        entry->gpu_time_accumulated += gpu_time;
    }

    return 0;
}

int D3D8_Vulkan_PresentFrame(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("PresentFrame: Invalid handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_LOOP_STATE_RENDERING) {
        _set_error("PresentFrame: Invalid state (expected RENDERING)");
        return -1;
    }

    entry->current_phase = D3D8_VULKAN_PHASE_PRESENT;

    // Frame rate control based on sync mode
    if (entry->sync_mode == D3D8_VULKAN_SYNC_IMMEDIATE) {
        // No sync - unlimited FPS
    } else if (entry->sync_mode == D3D8_VULKAN_SYNC_VSYNC) {
        // VSYNC - wait for display refresh
        // Actual implementation would use presentation engine
    } else if (entry->sync_mode == D3D8_VULKAN_SYNC_TRIPLE_BUFFER) {
        // Triple buffering - check in-flight frames
        if (entry->frames_in_flight >= entry->max_frames_in_flight) {
            // Wait for oldest frame
        }
    }

    entry->frames_in_flight++;
    if (entry->frames_in_flight > entry->max_frames_in_flight) {
        entry->frames_in_flight = entry->max_frames_in_flight;
    }

    return 0;
}

int D3D8_Vulkan_WaitForGPU(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    uint32_t timeout_ms)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("WaitForGPU: Invalid handle");
        return -1;
    }

    // In real implementation, this would wait on Vulkan fences/semaphores
    // For now, simulate waiting
    if (entry->frames_in_flight > 0) {
        entry->frames_in_flight--;
    }

    return 0;
}

int D3D8_Vulkan_GetRenderLoopInfo(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_RENDERLOOP_INFO* out_info)
{
    if (!out_info) {
        _set_error("GetRenderLoopInfo: Output info is NULL");
        return -1;
    }

    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("GetRenderLoopInfo: Invalid handle");
        return -1;
    }

    memset(out_info, 0, sizeof(D3D8_VULKAN_RENDERLOOP_INFO));
    out_info->state = entry->state;
    out_info->frame_count = entry->frame_count;
    out_info->elapsed_time = entry->elapsed_time;
    out_info->average_fps = entry->frame_count > 0 ? 
        entry->frame_count / (entry->elapsed_time > 0.001 ? entry->elapsed_time : 0.001) : 0;
    
    memcpy(&out_info->current_frame, &entry->current_stats, sizeof(D3D8_VULKAN_FRAME_STATS));
    memcpy(&out_info->last_frame, &entry->last_stats, sizeof(D3D8_VULKAN_FRAME_STATS));

    return 0;
}

int D3D8_Vulkan_SetTargetFPS(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    uint32_t fps)
{
    if (fps == 0) {
        _set_error("SetTargetFPS: FPS must be > 0");
        return -1;
    }

    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("SetTargetFPS: Invalid handle");
        return -1;
    }

    entry->target_fps = fps;
    printf("D3D8_Vulkan_RenderLoop: SetTargetFPS (ID: %u, FPS: %u)\n", entry->id, fps);

    return 0;
}

int D3D8_Vulkan_SetSyncMode(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_SYNC_MODE mode)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("SetSyncMode: Invalid handle");
        return -1;
    }

    entry->sync_mode = mode;
    printf("D3D8_Vulkan_RenderLoop: SetSyncMode (ID: %u, Mode: %u)\n", entry->id, mode);

    return 0;
}

D3D8_VULKAN_FRAME_PHASE D3D8_Vulkan_GetCurrentPhase(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop)
{
    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        return D3D8_VULKAN_PHASE_BEGIN;
    }

    return entry->current_phase;
}

int D3D8_Vulkan_GetFrameStats(
    D3D8_VULKAN_RENDERLOOP_HANDLE loop,
    D3D8_VULKAN_FRAME_STATS* out_stats)
{
    if (!out_stats) {
        _set_error("GetFrameStats: Output stats is NULL");
        return -1;
    }

    RenderLoopEntry* entry = _get_renderloop_entry(loop);
    if (!entry) {
        _set_error("GetFrameStats: Invalid handle");
        return -1;
    }

    memcpy(out_stats, &entry->current_stats, sizeof(D3D8_VULKAN_FRAME_STATS));
    return 0;
}

const char* D3D8_Vulkan_GetRenderLoopError(void)
{
    return g_last_error;
}
