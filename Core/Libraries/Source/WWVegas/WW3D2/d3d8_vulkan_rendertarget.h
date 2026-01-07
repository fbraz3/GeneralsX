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
**  FILE: d3d8_vulkan_rendertarget.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Render Target & Framebuffer Management
**
**  DESCRIPTION:
**    Provides Vulkan framebuffer creation, depth/stencil buffer management,
**    off-screen rendering targets, and dynamic resolution support for
**    DirectX 8 compatibility.
**
**    Phase 16: Render Target Management Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_RENDERTARGET_H_
#define D3D8_VULKAN_RENDERTARGET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t, uint8_t, int32_t

/* ============================================================================
 * Forward Declarations
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkImage;
typedef void* VkImageView;
typedef void* VkDeviceMemory;
typedef void* VkFramebuffer;
typedef void* VkRenderPass;

/* ============================================================================
 * Render Target Format Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_RENDERTARGET_FORMAT
 * @brief Render target color formats
 */
typedef enum {
    D3D8_VULKAN_RT_FMT_RGBA8 = 0x1,            /* RGBA8 unorm */
    D3D8_VULKAN_RT_FMT_RGBA16F = 0x2,          /* RGBA16F floating-point */
    D3D8_VULKAN_RT_FMT_RGBA32F = 0x3,          /* RGBA32F floating-point */
    D3D8_VULKAN_RT_FMT_BGRA8 = 0x4,            /* BGRA8 unorm (for backbuffer) */
} D3D8_VULKAN_RENDERTARGET_FORMAT;

/**
 * @enum D3D8_VULKAN_DEPTH_FORMAT
 * @brief Depth/stencil buffer formats
 */
typedef enum {
    D3D8_VULKAN_DEPTH_FMT_D24_UNORM = 0x1,     /* D24 unorm */
    D3D8_VULKAN_DEPTH_FMT_D32F = 0x2,          /* D32F floating-point */
    D3D8_VULKAN_DEPTH_FMT_D24_UNORM_S8 = 0x3,  /* D24 + S8 stencil */
    D3D8_VULKAN_DEPTH_FMT_D32F_S8 = 0x4,       /* D32F + S8 stencil */
    D3D8_VULKAN_DEPTH_FMT_NONE = 0x5,          /* No depth buffer */
} D3D8_VULKAN_DEPTH_FORMAT;

/**
 * @enum D3D8_VULKAN_MSAA_SAMPLES
 * @brief Multisampling (MSAA) sample counts
 */
typedef enum {
    D3D8_VULKAN_MSAA_1X = 0x1,                 /* No multisampling */
    D3D8_VULKAN_MSAA_2X = 0x2,                 /* 2x MSAA */
    D3D8_VULKAN_MSAA_4X = 0x4,                 /* 4x MSAA */
    D3D8_VULKAN_MSAA_8X = 0x8,                 /* 8x MSAA */
    D3D8_VULKAN_MSAA_16X = 0x10,               /* 16x MSAA */
} D3D8_VULKAN_MSAA_SAMPLES;

/**
 * @enum D3D8_VULKAN_RENDERTARGET_TYPE
 * @brief Render target usage type
 */
typedef enum {
    D3D8_VULKAN_RT_TYPE_BACKBUFFER = 0x1,      /* Main display framebuffer */
    D3D8_VULKAN_RT_TYPE_OFFSCREEN = 0x2,       /* Off-screen/shadow map target */
    D3D8_VULKAN_RT_TYPE_CUBEMAP = 0x3,         /* Cubemap face target */
    D3D8_VULKAN_RT_TYPE_ARRAY = 0x4,           /* Array layer target */
} D3D8_VULKAN_RENDERTARGET_TYPE;

/**
 * @enum D3D8_VULKAN_RENDERTARGET_STATE
 * @brief Render target lifecycle state
 */
typedef enum {
    D3D8_VULKAN_RT_STATE_UNINITIALIZED = 0x0,
    D3D8_VULKAN_RT_STATE_READY = 0x1,
    D3D8_VULKAN_RT_STATE_ACTIVE = 0x2,
    D3D8_VULKAN_RT_STATE_DESTROYED = 0x3,
} D3D8_VULKAN_RENDERTARGET_STATE;

/* ============================================================================
 * Render Target Configuration & Handle Structures
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_RENDERTARGET_CONFIG
 * @brief Render target creation parameters
 */
typedef struct {
    uint32_t width;                             /* Target width in pixels */
    uint32_t height;                            /* Target height in pixels */
    D3D8_VULKAN_RENDERTARGET_FORMAT color_fmt; /* Color buffer format */
    D3D8_VULKAN_DEPTH_FORMAT depth_fmt;        /* Depth buffer format */
    D3D8_VULKAN_MSAA_SAMPLES msaa_samples;     /* MSAA sample count */
    D3D8_VULKAN_RENDERTARGET_TYPE target_type; /* Render target type */
    uint32_t num_attachments;                  /* Number of color attachments */
    uint32_t num_layers;                       /* Layers for array/cubemap */
    uint32_t flags;                            /* Reserved for future use */
} D3D8_VULKAN_RENDERTARGET_CONFIG;

/**
 * @struct D3D8_VULKAN_RENDERTARGET_HANDLE
 * @brief Opaque render target handle
 */
typedef struct {
    uint32_t id;                                /* Unique render target ID */
    uint32_t version;                           /* Modification counter */
} D3D8_VULKAN_RENDERTARGET_HANDLE;

/**
 * @struct D3D8_VULKAN_RENDERTARGET_INFO
 * @brief Render target information retrieval
 */
typedef struct {
    uint32_t width;
    uint32_t height;
    D3D8_VULKAN_RENDERTARGET_FORMAT color_fmt;
    D3D8_VULKAN_DEPTH_FORMAT depth_fmt;
    D3D8_VULKAN_MSAA_SAMPLES msaa_samples;
    D3D8_VULKAN_RENDERTARGET_TYPE target_type;
    D3D8_VULKAN_RENDERTARGET_STATE state;
    VkFramebuffer framebuffer;
    VkRenderPass render_pass;
    uint32_t reference_count;
} D3D8_VULKAN_RENDERTARGET_INFO;

/* ============================================================================
 * Render Target Management API (18 Functions)
 * ============================================================================ */

/**
 * @fn D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateRenderTarget(VkDevice device, VkRenderPass render_pass, const D3D8_VULKAN_RENDERTARGET_CONFIG* config)
 * @brief Create a new render target with framebuffer
 * @param device Vulkan logical device
 * @param render_pass Render pass for this target
 * @param config Render target configuration
 * @return Opaque render target handle
 */
D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateRenderTarget(
    VkDevice device,
    VkRenderPass render_pass,
    const D3D8_VULKAN_RENDERTARGET_CONFIG* config
);

/**
 * @fn void D3D8_Vulkan_DestroyRenderTarget(VkDevice device, D3D8_VULKAN_RENDERTARGET_HANDLE target)
 * @brief Destroy render target and free resources
 * @param device Vulkan logical device
 * @param target Render target handle to destroy
 */
void D3D8_Vulkan_DestroyRenderTarget(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE target
);

/**
 * @fn int D3D8_Vulkan_GetRenderTargetInfo(D3D8_VULKAN_RENDERTARGET_HANDLE target, D3D8_VULKAN_RENDERTARGET_INFO* out_info)
 * @brief Get render target information
 * @param target Render target handle
 * @param out_info Pointer to receive render target info
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_GetRenderTargetInfo(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    D3D8_VULKAN_RENDERTARGET_INFO* out_info
);

/**
 * @fn int D3D8_Vulkan_SetRenderTarget(D3D8_VULKAN_RENDERTARGET_HANDLE target)
 * @brief Set render target as active
 * @param target Render target handle to activate
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_SetRenderTarget(
    D3D8_VULKAN_RENDERTARGET_HANDLE target
);

/**
 * @fn int D3D8_Vulkan_ClearRenderTarget(D3D8_VULKAN_RENDERTARGET_HANDLE target, uint32_t clear_flags, float r, float g, float b, float a, float depth, uint8_t stencil)
 * @brief Clear render target color/depth/stencil
 * @param target Render target handle
 * @param clear_flags Flags indicating what to clear (bit 0=color, 1=depth, 2=stencil)
 * @param r, g, b, a Clear color RGBA values
 * @param depth Clear depth value
 * @param stencil Clear stencil value
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_ClearRenderTarget(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    uint32_t clear_flags,
    float r,
    float g,
    float b,
    float a,
    float depth,
    uint8_t stencil
);

/**
 * @fn int D3D8_Vulkan_ResizeRenderTarget(VkDevice device, D3D8_VULKAN_RENDERTARGET_HANDLE target, uint32_t new_width, uint32_t new_height)
 * @brief Resize render target dynamically
 * @param device Vulkan logical device
 * @param target Render target handle
 * @param new_width New width in pixels
 * @param new_height New height in pixels
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_ResizeRenderTarget(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    uint32_t new_width,
    uint32_t new_height
);

/**
 * @fn D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateDepthBuffer(VkDevice device, uint32_t width, uint32_t height, D3D8_VULKAN_DEPTH_FORMAT format)
 * @brief Create a standalone depth buffer
 * @param device Vulkan logical device
 * @param width Buffer width in pixels
 * @param height Buffer height in pixels
 * @param format Depth format
 * @return Opaque depth buffer handle
 */
D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateDepthBuffer(
    VkDevice device,
    uint32_t width,
    uint32_t height,
    D3D8_VULKAN_DEPTH_FORMAT format
);

/**
 * @fn void D3D8_Vulkan_DestroyDepthBuffer(VkDevice device, D3D8_VULKAN_RENDERTARGET_HANDLE depth_buffer)
 * @brief Destroy depth buffer and free resources
 * @param device Vulkan logical device
 * @param depth_buffer Depth buffer handle to destroy
 */
void D3D8_Vulkan_DestroyDepthBuffer(
    VkDevice device,
    D3D8_VULKAN_RENDERTARGET_HANDLE depth_buffer
);

/**
 * @fn D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateOffscreenTarget(VkDevice device, VkRenderPass render_pass, uint32_t width, uint32_t height, D3D8_VULKAN_RENDERTARGET_FORMAT format)
 * @brief Create off-screen render target for shadows/reflections
 * @param device Vulkan logical device
 * @param render_pass Render pass for this target
 * @param width Target width in pixels
 * @param height Target height in pixels
 * @param format Color format
 * @return Opaque off-screen target handle
 */
D3D8_VULKAN_RENDERTARGET_HANDLE D3D8_Vulkan_CreateOffscreenTarget(
    VkDevice device,
    VkRenderPass render_pass,
    uint32_t width,
    uint32_t height,
    D3D8_VULKAN_RENDERTARGET_FORMAT format
);

/**
 * @fn int D3D8_Vulkan_BlitRenderTarget(D3D8_VULKAN_RENDERTARGET_HANDLE src, D3D8_VULKAN_RENDERTARGET_HANDLE dst, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h, uint32_t dst_x, uint32_t dst_y, uint32_t dst_w, uint32_t dst_h)
 * @brief Copy region from source to destination render target
 * @param src Source render target
 * @param dst Destination render target
 * @param src_x, src_y, src_w, src_h Source rectangle
 * @param dst_x, dst_y, dst_w, dst_h Destination rectangle
 * @return 0 on success, -1 on error
 */
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
    uint32_t dst_h
);

/**
 * @fn int D3D8_Vulkan_ReadRenderTargetPixels(D3D8_VULKAN_RENDERTARGET_HANDLE target, void* pixels, uint32_t pitch)
 * @brief Read pixels from render target to CPU memory
 * @param target Render target handle
 * @param pixels Destination buffer pointer
 * @param pitch Bytes per row
 * @return 0 on success, -1 on error
 */
int D3D8_Vulkan_ReadRenderTargetPixels(
    D3D8_VULKAN_RENDERTARGET_HANDLE target,
    void* pixels,
    uint32_t pitch
);

/**
 * @fn uint32_t D3D8_Vulkan_GetRenderTargetCacheStats(uint32_t* out_total, uint32_t* out_used, uint32_t* out_free)
 * @brief Get render target cache statistics
 * @param out_total Pointer to receive total capacity
 * @param out_used Pointer to receive used count
 * @param out_free Pointer to receive free slots
 * @return Cache utilization percentage (0-100)
 */
uint32_t D3D8_Vulkan_GetRenderTargetCacheStats(
    uint32_t* out_total,
    uint32_t* out_used,
    uint32_t* out_free
);

/**
 * @fn void D3D8_Vulkan_ClearRenderTargetCache(VkDevice device)
 * @brief Clear all render targets from cache
 * @param device Vulkan logical device
 */
void D3D8_Vulkan_ClearRenderTargetCache(VkDevice device);

/**
 * @fn int D3D8_Vulkan_ValidateRenderTargetHandle(D3D8_VULKAN_RENDERTARGET_HANDLE target)
 * @brief Check if render target handle is valid
 * @param target Render target handle to validate
 * @return 1 if valid, 0 if invalid
 */
int D3D8_Vulkan_ValidateRenderTargetHandle(D3D8_VULKAN_RENDERTARGET_HANDLE target);

/**
 * @fn void D3D8_Vulkan_IncrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target)
 * @brief Increment reference count
 * @param target Render target handle
 */
void D3D8_Vulkan_IncrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target);

/**
 * @fn void D3D8_Vulkan_DecrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target)
 * @brief Decrement reference count
 * @param target Render target handle
 */
void D3D8_Vulkan_DecrementRenderTargetRefCount(D3D8_VULKAN_RENDERTARGET_HANDLE target);

/**
 * @fn const char* D3D8_Vulkan_GetRenderTargetError(void)
 * @brief Get last render target system error message
 * @return Error message string (static buffer)
 */
const char* D3D8_Vulkan_GetRenderTargetError(void);

#ifdef __cplusplus
}
#endif

#endif /* D3D8_VULKAN_RENDERTARGET_H_ */
