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
**  FILE: d3d8_vulkan_command_buffer.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Command Buffer & GPU/CPU Synchronization Abstraction
**
**  DESCRIPTION:
**    Provides Vulkan VkCommandPool, VkCommandBuffer, and synchronization
**    primitive creation and management for DirectX 8 compatibility layer.
**    Enables cross-platform graphics command recording and frame pacing.
**
**    Phase 10: Command Buffers & Synchronization Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_COMMAND_BUFFER_H_
#define D3D8_VULKAN_COMMAND_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t

/* ============================================================================
 * Forward Declarations (to avoid including Vulkan headers directly)
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkCommandPool;
typedef void* VkCommandBuffer;
typedef void* VkSemaphore;
typedef void* VkFence;
typedef void* VkQueue;

/* ============================================================================
 * Command Buffer Configuration Types
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_COMMAND_BUFFER_LEVEL
 * @brief Command buffer level enumeration
 */
typedef enum {
    D3D8_VULKAN_CMD_BUFFER_PRIMARY = 0,    /* Primary command buffer (can be submitted) */
    D3D8_VULKAN_CMD_BUFFER_SECONDARY = 1,  /* Secondary command buffer (reusable) */
} D3D8_VULKAN_COMMAND_BUFFER_LEVEL;

/**
 * @enum D3D8_VULKAN_COMMAND_POOL_FLAGS
 * @brief Command pool creation flags
 */
typedef enum {
    D3D8_VULKAN_CMD_POOL_TRANSIENT = 0x1,          /* Optimized for short-lived buffers */
    D3D8_VULKAN_CMD_POOL_RESET_INDIVIDUAL = 0x2,   /* Allow individual buffer reset */
} D3D8_VULKAN_COMMAND_POOL_FLAGS;

/**
 * @struct D3D8_VULKAN_COMMAND_BUFFER_CONFIG
 * @brief Configuration for command buffer operations
 */
typedef struct {
    uint32_t graphics_queue_family;         /* Graphics queue family index */
    uint32_t frames_in_flight;              /* Number of frames to buffer (typically 2-3) */
    uint32_t command_buffer_per_frame;      /* Number of command buffers per frame */
    int reset_individual_buffers;           /* Boolean: enable individual buffer reset */
} D3D8_VULKAN_COMMAND_BUFFER_CONFIG;

/**
 * @struct D3D8_VULKAN_SYNC_PRIMITIVES
 * @brief GPU/CPU synchronization primitive handles
 */
typedef struct {
    VkSemaphore image_available_semaphore;  /* Signals when swapchain image available */
    VkSemaphore render_complete_semaphore;  /* Signals when rendering complete */
    VkFence in_flight_fence;                /* Signals when GPU finishes frame */
} D3D8_VULKAN_SYNC_PRIMITIVES;

/* ============================================================================
 * Public API Functions - Command Pool Management
 * ============================================================================ */

/**
 * @brief Create a Vulkan command pool
 *
 * @param device Vulkan device
 * @param graphics_queue_family Graphics queue family index
 * @param flags Command pool creation flags
 * @param command_pool Output command pool handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateCommandPool(
    VkDevice device,
    uint32_t graphics_queue_family,
    uint32_t flags,
    VkCommandPool* command_pool);

/**
 * @brief Destroy a Vulkan command pool
 *
 * @param device Vulkan device
 * @param command_pool Command pool to destroy
 */
void D3D8_Vulkan_DestroyCommandPool(
    VkDevice device,
    VkCommandPool command_pool);

/**
 * @brief Reset all command buffers in a pool
 *
 * @param device Vulkan device
 * @param command_pool Command pool to reset
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_ResetCommandPool(
    VkDevice device,
    VkCommandPool command_pool);

/* ============================================================================
 * Public API Functions - Command Buffer Management
 * ============================================================================ */

/**
 * @brief Allocate command buffers from a pool
 *
 * @param device Vulkan device
 * @param command_pool Command pool
 * @param level Command buffer level (primary/secondary)
 * @param count Number of command buffers to allocate
 * @param command_buffers Output array of command buffer handles
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_AllocateCommandBuffers(
    VkDevice device,
    VkCommandPool command_pool,
    D3D8_VULKAN_COMMAND_BUFFER_LEVEL level,
    uint32_t count,
    VkCommandBuffer* command_buffers);

/**
 * @brief Free command buffers back to pool
 *
 * @param device Vulkan device
 * @param command_pool Command pool
 * @param count Number of command buffers to free
 * @param command_buffers Array of command buffer handles to free
 */
void D3D8_Vulkan_FreeCommandBuffers(
    VkDevice device,
    VkCommandPool command_pool,
    uint32_t count,
    VkCommandBuffer* command_buffers);

/**
 * @brief Begin recording a command buffer
 *
 * @param command_buffer Command buffer to begin recording
 * @param is_one_time_submit Boolean: optimize for single-time submission
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_BeginCommandBuffer(
    VkCommandBuffer command_buffer,
    int is_one_time_submit);

/**
 * @brief End recording a command buffer
 *
 * @param command_buffer Command buffer to end
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_EndCommandBuffer(
    VkCommandBuffer command_buffer);

/**
 * @brief Reset a command buffer to initial state
 *
 * @param command_buffer Command buffer to reset
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_ResetCommandBuffer(
    VkCommandBuffer command_buffer);

/* ============================================================================
 * Public API Functions - Synchronization Primitives
 * ============================================================================ */

/**
 * @brief Create GPU-GPU synchronization semaphore
 *
 * @param device Vulkan device
 * @param semaphore Output semaphore handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateSemaphore(
    VkDevice device,
    VkSemaphore* semaphore);

/**
 * @brief Destroy a semaphore
 *
 * @param device Vulkan device
 * @param semaphore Semaphore to destroy
 */
void D3D8_Vulkan_DestroySemaphore(
    VkDevice device,
    VkSemaphore semaphore);

/**
 * @brief Create GPU-CPU synchronization fence
 *
 * @param device Vulkan device
 * @param initially_signaled Boolean: create in signaled state (for first frame)
 * @param fence Output fence handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateFence(
    VkDevice device,
    int initially_signaled,
    VkFence* fence);

/**
 * @brief Destroy a fence
 *
 * @param device Vulkan device
 * @param fence Fence to destroy
 */
void D3D8_Vulkan_DestroyFence(
    VkDevice device,
    VkFence fence);

/**
 * @brief Wait for a fence to be signaled (blocks CPU)
 *
 * @param device Vulkan device
 * @param fence Fence to wait for
 * @param timeout_ns Timeout in nanoseconds (UINT64_MAX for infinite)
 * @return 0 on success, 1 if timeout occurred, -1 on error
 */
int D3D8_Vulkan_WaitForFence(
    VkDevice device,
    VkFence fence,
    uint64_t timeout_ns);

/**
 * @brief Reset a fence to unsignaled state
 *
 * @param device Vulkan device
 * @param fence Fence to reset
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_ResetFence(
    VkDevice device,
    VkFence fence);

/* ============================================================================
 * Public API Functions - Frame Pacing
 * ============================================================================ */

/**
 * @brief Create synchronization primitives for a frame
 *
 * @param device Vulkan device
 * @param sync_primitives Output synchronization primitives
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateFrameSyncPrimitives(
    VkDevice device,
    D3D8_VULKAN_SYNC_PRIMITIVES* sync_primitives);

/**
 * @brief Destroy frame synchronization primitives
 *
 * @param device Vulkan device
 * @param sync_primitives Synchronization primitives to destroy
 */
void D3D8_Vulkan_DestroyFrameSyncPrimitives(
    VkDevice device,
    D3D8_VULKAN_SYNC_PRIMITIVES* sync_primitives);

/**
 * @brief Submit a command buffer to graphics queue
 *
 * @param graphics_queue Graphics queue
 * @param command_buffer Command buffer to submit
 * @param wait_semaphore Semaphore to wait for before execution (0 if none)
 * @param signal_semaphore Semaphore to signal on completion (0 if none)
 * @param fence Fence to signal on completion (0 if none)
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_SubmitCommandBuffer(
    VkQueue graphics_queue,
    VkCommandBuffer command_buffer,
    VkSemaphore wait_semaphore,
    VkSemaphore signal_semaphore,
    VkFence fence);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* D3D8_VULKAN_COMMAND_BUFFER_H_ */
