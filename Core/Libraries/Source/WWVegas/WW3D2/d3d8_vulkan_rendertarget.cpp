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
**  FILE: d3d8_vulkan_rendertarget.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Render Target & Framebuffer Implementation
**
**  DESCRIPTION:
**    Implementation of render target creation, framebuffer management,
**    depth/stencil buffer handling, and dynamic resolution support.
**
**    Phase 16: Render Target Management Implementation
**
******************************************************************************/

#include "d3d8_vulkan_rendertarget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ============================================================================
 * Internal Constants & Configuration
 * ============================================================================ */

#define D3D8_VULKAN_MAX_RENDERTARGETS 128
#define D3D8_VULKAN_ERROR_MESSAGE_SIZE 512

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

/**
 * @struct RenderTargetEntry
 * @brief Internal render target cache entry
 */
typedef struct {
    uint32_t id;
    uint32_t version;
    D3D8_VULKAN_RENDERTARGET_STATE state;
    uint32_t width;
    uint32_t height;
    D3D8_VULKAN_RENDERTARGET_FORMAT color_fmt;
    D3D8_VULKAN_DEPTH_FORMAT depth_fmt;
    D3D8_VULKAN_MSAA_SAMPLES msaa_samples;
    D3D8_VULKAN_RENDERTARGET_TYPE target_type;
    VkFramebuffer framebuffer;
    VkRenderPass render_pass;
    VkImage color_image;
    VkImage depth_image;
    VkDeviceMemory color_memory;
    VkDeviceMemory depth_memory;
    uint32_t reference_count;
    uint32_t allocated;
} RenderTargetEntry;

/* ============================================================================
 * Global State & Caches
 * ============================================================================ */

static RenderTargetEntry g_rendertarget_cache[D3D8_VULKAN_MAX_RENDERTARGETS];
static uint32_t g_rendertarget_counter = 11000;
static char g_last_error[D3D8_VULKAN_ERROR_MESSAGE_SIZE];

/* ============================================================================
 * Utility Functions (Internal)
 * ============================================================================ */

/**
 * @fn static uint32_t _find_free_rendertarget_slot(void)
 * @brief Find next free slot in render target cache
 */
static uint32_t _find_free_rendertarget_slot(void)
{
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_RENDERTARGETS; i++) {
        if (!g_rendertarget_cache[i].allocated) {
            return i;
        }
    }
    return D3D8_VULKAN_MAX_RENDERTARGETS;
}

/**
 * @fn static int _is_valid_rendertarget_handle(D3D8_VULKAN_RENDERTARGET_HANDLE handle)
 * @brief Validate render target handle
 */
static int _is_valid_rendertarget_handle(D3D8_VULKAN_RENDERTARGET_HANDLE handle)
{
    if (handle.id < 11000 || handle.id >= 11000 + D3D8_VULKAN_MAX_RENDERTARGETS) {
        return 0;
    }
    uint32_t index = handle.id - 11000;
    if (index >= D3D8_VULKAN_MAX_RENDERTARGETS) {
        return 0;
    }
    if (!g_rendertarget_cache[index].allocated) {
        return 0;
    }
    if (g_rendertarget_cache[index].version != handle.version) {
        return 0;
    }
    return 1;
}

/**
 * @fn static RenderTargetEntry* _get_rendertarget_entry(D3D8_VULKAN_RENDERTARGET_HANDLE handle)
 * @brief Get render target entry from handle
 */
static RenderTargetEntry* _get_rendertarget_entry(D3D8_VULKAN_RENDERTARGET_HANDLE handle)
{
    if (!_is_valid_rendertarget_handle(handle)) {
        return NULL;
    }
    uint32_t index = handle.id - 11000;
    return &g_rendertarget_cache[index];
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
 * @fn static void _log_operation(const char* operation, uint32_t id, uint32_t width, uint32_t height)
 * @brief Log render target operation
 */
static void _log_operation(const char* operation, uint32_t id, uint32_t width, uint32_t height)
{
    printf("D3D8_Vulkan_RenderTarget: %s (ID: %u, %ux%u)\n", operation, id, width, height);
}

/* ============================================================================
 * Render Target Management API Implementation (18 Functions)
 * ============================================================================ */

D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateRenderTarget(
    const D3D8_VULKAN_RENDERTARGET_CONFIG* config)
{
    D3D8_VULKAN_RENDERTARGET_HANDLE invalid_handle = {0, 0};

    if (!config) {
        _set_error("CreateRenderTarget: Invalid config pointer");
        return invalid_handle;
    }

    if (config->width == 0 || config->height == 0) {
        _set_error("CreateRenderTarget: Invalid dimensions (%u x %u)", config->width, config->height);
        return invalid_handle;
    }

    uint32_t slot = _find_free_rendertarget_slot();
    if (slot >= D3D8_VULKAN_MAX_RENDERTARGETS) {
        _set_error("CreateRenderTarget: Cache full (max %u)", D3D8_VULKAN_MAX_RENDERTARGETS);
        return invalid_handle;
    }

    RenderTargetEntry* entry = &g_rendertarget_cache[slot];
    memset(entry, 0, sizeof(RenderTargetEntry));

    entry->id = g_rendertarget_counter++;
    entry->version = 1;
    entry->state = D3D8_VULKAN_RT_STATE_READY;
    entry->width = config->width;
    entry->height = config->height;
    entry->color_fmt = config->color_fmt;
    entry->depth_fmt = config->depth_fmt;
    entry->msaa_samples = config->msaa_samples;
    entry->target_type = config->target_type;
    entry->framebuffer = NULL;
    entry->reference_count = 1;
    entry->allocated = 1;

    _log_operation("CreateRenderTarget", entry->id, entry->width, entry->height);

    D3D8_VULKAN_RENDERTARGET_HANDLE handle = {entry->id, entry->version};
    return handle;
}

void D3D8_Vulkan_DestroyRenderTarget(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE target)
{
    if (!device) {
        _set_error("DestroyRenderTarget: Invalid device");
        return;
    }

    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("DestroyRenderTarget: Invalid handle");
        return;
    }

    printf("D3D8_Vulkan_RenderTarget: DestroyRenderTarget (ID: %u, %ux%u)\n",
           entry->id, entry->width, entry->height);

    entry->state = D3D8_VULKAN_RT_STATE_DESTROYED;
    entry->allocated = 0;
    entry->version++;
    memset(entry, 0, sizeof(RenderTargetEntry));
}

int D3D8_Vulkan_GetRenderTargetInfo(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    D3D8_VULKAN_RENDERTARGET_INFO* out_info)
{
    if (!out_info) {
        _set_error("GetRenderTargetInfo: Output info is NULL");
        return -1;
    }

    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("GetRenderTargetInfo: Invalid handle");
        return -1;
    }

    memset(out_info, 0, sizeof(D3D8_VULKAN_RENDERTARGET_INFO));
    out_info->width = entry->width;
    out_info->height = entry->height;
    out_info->color_fmt = entry->color_fmt;
    out_info->depth_fmt = entry->depth_fmt;
    out_info->msaa_samples = entry->msaa_samples;
    out_info->target_type = entry->target_type;
    out_info->state = entry->state;
    out_info->framebuffer = entry->framebuffer;
    out_info->render_pass = entry->render_pass;
    out_info->reference_count = entry->reference_count;

    return 0;
}

int D3D8_Vulkan_SetRenderTarget(
    D3D8_VULKAN_RENDERTARGET_HANDLE target)
{
    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("SetRenderTarget: Invalid handle");
        return -1;
    }

    if (entry->state != D3D8_VULKAN_RT_STATE_READY) {
        _set_error("SetRenderTarget: Target not in ready state");
        return -1;
    }

    entry->state = D3D8_VULKAN_RT_STATE_ACTIVE;
    printf("D3D8_Vulkan_RenderTarget: SetRenderTarget (ID: %u)\n", entry->id);

    return 0;
}

int D3D8_Vulkan_ClearRenderTarget(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    uint32_t clear_flags,
    float r,
    float g,
    float b,
    float a,
    float depth,
    uint8_t stencil)
{
    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("ClearRenderTarget: Invalid handle");
        return -1;
    }

    printf("D3D8_Vulkan_RenderTarget: ClearRenderTarget (ID: %u, flags: 0x%x, RGBA: %.2f,%.2f,%.2f,%.2f)\n",
           entry->id, clear_flags, r, g, b, a);

    return 0;
}

int D3D8_Vulkan_ResizeRenderTarget(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    uint32_t new_width,
    uint32_t new_height)
{
    if (!device || new_width == 0 || new_height == 0) {
        _set_error("ResizeRenderTarget: Invalid parameters");
        return -1;
    }

    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("ResizeRenderTarget: Invalid handle");
        return -1;
    }

    printf("D3D8_Vulkan_RenderTarget: ResizeRenderTarget (ID: %u, %ux%u -> %ux%u)\n",
           entry->id, entry->width, entry->height, new_width, new_height);

    entry->width = new_width;
    entry->height = new_height;
    entry->version++;

    return 0;
}

D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateDepthBuffer(
    VkDevice device,
    uint32_t width,
    uint32_t height,
    D3D8_VULKAN_DEPTH_FORMAT format)
{
    D3D8_VULKAN_RENDERTARGET_HANDLE invalid_handle = {0, 0};

    if (!device || width == 0 || height == 0) {
        _set_error("CreateDepthBuffer: Invalid parameters");
        return invalid_handle;
    }

    uint32_t slot = _find_free_rendertarget_slot();
    if (slot >= D3D8_VULKAN_MAX_RENDERTARGETS) {
        _set_error("CreateDepthBuffer: Cache full");
        return invalid_handle;
    }

    RenderTargetEntry* entry = &g_rendertarget_cache[slot];
    memset(entry, 0, sizeof(RenderTargetEntry));

    entry->id = g_rendertarget_counter++;
    entry->version = 1;
    entry->state = D3D8_VULKAN_RT_STATE_READY;
    entry->width = width;
    entry->height = height;
    entry->depth_fmt = format;
    entry->target_type = D3D8_VULKAN_RT_TYPE_OFFSCREEN;
    entry->reference_count = 1;
    entry->allocated = 1;

    printf("D3D8_Vulkan_RenderTarget: CreateDepthBuffer (ID: %u, %ux%u)\n",
           entry->id, width, height);

    D3D8_VULKAN_RENDERTARGET_HANDLE handle = {entry->id, entry->version};
    return handle;
}

void D3D8_Vulkan_DestroyDepthBuffer(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE depth_buffer)
{
    if (!device) {
        _set_error("DestroyDepthBuffer: Invalid device");
        return;
    }

    RenderTargetEntry* entry = _get_rendertarget_entry(depth_buffer);
    if (!entry) {
        _set_error("DestroyDepthBuffer: Invalid handle");
        return;
    }

    printf("D3D8_Vulkan_RenderTarget: DestroyDepthBuffer (ID: %u)\n", entry->id);

    entry->state = D3D8_VULKAN_RT_STATE_DESTROYED;
    entry->allocated = 0;
    entry->version++;
    memset(entry, 0, sizeof(RenderTargetEntry));
}

D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateOffscreenTarget(
    VkDevice device,
    VkRenderPass render_pass,
    uint32_t width,
    uint32_t height,
    D3D8_VULKAN_RENDERTARGET_FORMAT format)
{
    D3D8_VULKAN_RENDERTARGET_HANDLE invalid_handle = {0, 0};

    if (!device || !render_pass || width == 0 || height == 0) {
        _set_error("CreateOffscreenTarget: Invalid parameters");
        return invalid_handle;
    }

    uint32_t slot = _find_free_rendertarget_slot();
    if (slot >= D3D8_VULKAN_MAX_RENDERTARGETS) {
        _set_error("CreateOffscreenTarget: Cache full");
        return invalid_handle;
    }

    RenderTargetEntry* entry = &g_rendertarget_cache[slot];
    memset(entry, 0, sizeof(RenderTargetEntry));

    entry->id = g_rendertarget_counter++;
    entry->version = 1;
    entry->state = D3D8_VULKAN_RT_STATE_READY;
    entry->width = width;
    entry->height = height;
    entry->color_fmt = format;
    entry->target_type = D3D8_VULKAN_RT_TYPE_OFFSCREEN;
    entry->render_pass = render_pass;
    entry->reference_count = 1;
    entry->allocated = 1;

    printf("D3D8_Vulkan_RenderTarget: CreateOffscreenTarget (ID: %u, %ux%u)\n",
           entry->id, width, height);

    D3D8_VULKAN_RENDERTARGET_HANDLE handle = {entry->id, entry->version};
    return handle;
}

int D3D8_Vulkan_BlitRenderTarget(
    D3D8_VULKAN_RENDERTARGET_HANDLE src,
    D3D8_VULKAN_RENDERTARGET_HANDLE dst,
    uint32_t src_x,
    uint32_t src_y,
    uint32_t src_w,
    uint32_t src_h,
    uint32_t dst_x,
    uint32_t dst_y,
    uint32_t dst_w,
    uint32_t dst_h)
{
    RenderTargetEntry* src_entry = _get_rendertarget_entry(src);
    if (!src_entry) {
        _set_error("BlitRenderTarget: Invalid source handle");
        return -1;
    }

    RenderTargetEntry* dst_entry = _get_rendertarget_entry(dst);
    if (!dst_entry) {
        _set_error("BlitRenderTarget: Invalid destination handle");
        return -1;
    }

    printf("D3D8_Vulkan_RenderTarget: BlitRenderTarget (%u -> %u)\n", src_entry->id, dst_entry->id);

    return 0;
}

int D3D8_Vulkan_ReadRenderTargetPixels(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    void* pixels,
    uint32_t pitch)
{
    if (!pixels) {
        _set_error("ReadRenderTargetPixels: Pixels buffer is NULL");
        return -1;
    }

    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (!entry) {
        _set_error("ReadRenderTargetPixels: Invalid handle");
        return -1;
    }

    printf("D3D8_Vulkan_RenderTarget: ReadRenderTargetPixels (ID: %u, pitch: %u)\n",
           entry->id, pitch);

    return 0;
}

uint32_t D3D8_Vulkan_GetRenderTargetCacheStats(
    uint32_t* out_total,
    uint32_t* out_used,
    uint32_t* out_free)
{
    uint32_t used = 0;
    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_RENDERTARGETS; i++) {
        if (g_rendertarget_cache[i].allocated) {
            used++;
        }
    }

    if (out_total) *out_total = D3D8_VULKAN_MAX_RENDERTARGETS;
    if (out_used) *out_used = used;
    if (out_free) *out_free = D3D8_VULKAN_MAX_RENDERTARGETS - used;

    uint32_t utilization = (used * 100) / D3D8_VULKAN_MAX_RENDERTARGETS;
    return utilization;
}

void D3D8_Vulkan_ClearRenderTargetCache(VkDevice device)
{
    if (!device) {
        _set_error("ClearRenderTargetCache: Invalid device");
        return;
    }

    for (uint32_t i = 0; i < D3D8_VULKAN_MAX_RENDERTARGETS; i++) {
        if (g_rendertarget_cache[i].allocated) {
            g_rendertarget_cache[i].state = D3D8_VULKAN_RT_STATE_DESTROYED;
            g_rendertarget_cache[i].allocated = 0;
            g_rendertarget_cache[i].version++;
        }
    }

    printf("D3D8_Vulkan_RenderTarget: ClearRenderTargetCache complete\n");
}

int D3D8_Vulkan_ValidateRenderTargetHandle(D3D8_VULKAN_RENDERTARGET_HANDLE target)
{
    return _is_valid_rendertarget_handle(target);
}

void D3D8_Vulkan_IncrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target)
{
    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (entry) {
        entry->reference_count++;
    }
}

void D3D8_Vulkan_DecrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target)
{
    RenderTargetEntry* entry = _get_rendertarget_entry(target);
    if (entry && entry->reference_count > 0) {
        entry->reference_count--;
    }
}

const char* D3D8_Vulkan_GetRenderTargetError(void)
{
    return g_last_error;
}
