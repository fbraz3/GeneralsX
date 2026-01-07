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
**  FILE: d3d8_vulkan_buffer.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Vulkan Vertex & Index Buffer Management Abstraction
**
**  DESCRIPTION:
**    Provides Vulkan VkBuffer creation, memory management, and GPU data upload
**    for DirectX 8 compatibility layer. Enables cross-platform geometry data
**    management with support for static and dynamic buffers.
**
**    Phase 11: Vertex & Index Buffers Implementation
**
******************************************************************************/

#ifndef D3D8_VULKAN_BUFFER_H_
#define D3D8_VULKAN_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>  // For uint32_t, uint64_t, uint8_t

/* ============================================================================
 * Forward Declarations (to avoid including Vulkan headers directly)
 * ============================================================================ */

typedef void* VkDevice;
typedef void* VkBuffer;
typedef void* VkDeviceMemory;
typedef void* VkCommandBuffer;
typedef void* VkQueue;

/* ============================================================================
 * Buffer Type & Usage Enumerations
 * ============================================================================ */

/**
 * @enum D3D8_VULKAN_BUFFER_TYPE
 * @brief Vulkan buffer usage classification
 */
typedef enum {
    D3D8_VULKAN_BUFFER_VERTEX = 0x1,          /* Vertex buffer (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) */
    D3D8_VULKAN_BUFFER_INDEX = 0x2,           /* Index buffer (VK_BUFFER_USAGE_INDEX_BUFFER_BIT) */
    D3D8_VULKAN_BUFFER_UNIFORM = 0x4,         /* Uniform/constant buffer (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) */
    D3D8_VULKAN_BUFFER_STAGING = 0x8,         /* Staging buffer for CPU→GPU transfer */
} D3D8_VULKAN_BUFFER_TYPE;

/**
 * @enum D3D8_VULKAN_MEMORY_ACCESS
 * @brief CPU access pattern for buffer memory
 */
typedef enum {
    D3D8_VULKAN_MEMORY_GPU_ONLY = 0,          /* GPU-only (fast, no CPU access) */
    D3D8_VULKAN_MEMORY_GPU_OPTIMAL = 1,       /* GPU-optimal with staging (recommended) */
    D3D8_VULKAN_MEMORY_HOST_VISIBLE = 2,      /* Host-visible (CPU can read/write directly) */
    D3D8_VULKAN_MEMORY_HOST_COHERENT = 3,     /* Host-coherent (automatic sync) */
} D3D8_VULKAN_MEMORY_ACCESS;

/**
 * @enum D3D8_VULKAN_INDEX_FORMAT
 * @brief Index buffer data format
 */
typedef enum {
    D3D8_VULKAN_INDEX_16BIT = 0,              /* uint16_t indices */
    D3D8_VULKAN_INDEX_32BIT = 1,              /* uint32_t indices */
} D3D8_VULKAN_INDEX_FORMAT;

/* ============================================================================
 * Buffer Configuration Types
 * ============================================================================ */

/**
 * @struct D3D8_VULKAN_BUFFER_CONFIG
 * @brief Configuration for buffer creation
 */
typedef struct {
    uint64_t size_bytes;                      /* Buffer size in bytes */
    D3D8_VULKAN_BUFFER_TYPE usage;            /* Buffer usage flags */
    D3D8_VULKAN_MEMORY_ACCESS memory_access;  /* CPU access pattern */
    int coherent_memory;                      /* Boolean: use coherent memory if available */
} D3D8_VULKAN_BUFFER_CONFIG;

/**
 * @struct D3D8_VULKAN_BUFFER_HANDLE
 * @brief Opaque buffer handle with metadata
 */
typedef struct {
    VkBuffer gpu_buffer;                      /* GPU buffer object */
    VkDeviceMemory gpu_memory;                /* GPU memory allocation */
    uint64_t size_bytes;                      /* Allocation size */
    D3D8_VULKAN_BUFFER_TYPE usage;            /* Usage flags */
    D3D8_VULKAN_MEMORY_ACCESS memory_access;  /* Memory access pattern */
    void* cpu_mapped_ptr;                     /* Mapped pointer (if host-visible) */
    int is_coherent;                          /* Boolean: memory is coherent */
} D3D8_VULKAN_BUFFER_HANDLE;

/* ============================================================================
 * Public API Functions - Buffer Allocation
 * ============================================================================ */

/**
 * @brief Allocate a Vulkan buffer (GPU or host-visible)
 *
 * @param device Vulkan device
 * @param config Buffer configuration
 * @param buffer_handle Output buffer handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_AllocateBuffer(
    VkDevice device,
    const D3D8_VULKAN_BUFFER_CONFIG* config,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle);

/**
 * @brief Free an allocated buffer and its memory
 *
 * @param device Vulkan device
 * @param buffer_handle Buffer to free
 */
void D3D8_Vulkan_FreeBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle);

/**
 * @brief Allocate a staging buffer for CPU→GPU transfer
 *
 * @param device Vulkan device
 * @param size_bytes Buffer size in bytes
 * @param staging_buffer Output staging buffer handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_AllocateStagingBuffer(
    VkDevice device,
    uint64_t size_bytes,
    D3D8_VULKAN_BUFFER_HANDLE* staging_buffer);

/* ============================================================================
 * Public API Functions - Data Transfer
 * ============================================================================ */

/**
 * @brief Upload data to buffer from CPU memory
 *
 * @param device Vulkan device
 * @param buffer_handle Destination buffer
 * @param data Source data pointer
 * @param data_size Size of data to upload
 * @param offset_bytes Offset in buffer to write to
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_UploadBufferData(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    const void* data,
    uint64_t data_size,
    uint64_t offset_bytes);

/**
 * @brief Upload data using staging buffer (preferred for GPU-only buffers)
 *
 * @param device Vulkan device
 * @param transfer_queue Transfer/compute queue
 * @param transfer_cmd Command buffer for transfer
 * @param staging_buffer Temporary staging buffer
 * @param destination_buffer Target GPU buffer
 * @param data Source data pointer
 * @param data_size Size of data to transfer
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_UploadBufferDataStaged(
    VkDevice device,
    VkQueue transfer_queue,
    VkCommandBuffer transfer_cmd,
    D3D8_VULKAN_BUFFER_HANDLE* staging_buffer,
    D3D8_VULKAN_BUFFER_HANDLE* destination_buffer,
    const void* data,
    uint64_t data_size);

/**
 * @brief Read data back from buffer to CPU memory
 *
 * @param device Vulkan device
 * @param buffer_handle Source buffer
 * @param output_data Output buffer pointer
 * @param data_size Size of data to read
 * @param offset_bytes Offset in buffer to read from
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_ReadBufferData(
    VkDevice device,
    const D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    void* output_data,
    uint64_t data_size,
    uint64_t offset_bytes);

/* ============================================================================
 * Public API Functions - Vertex Buffer Management
 * ============================================================================ */

/**
 * @brief Create a vertex buffer
 *
 * @param device Vulkan device
 * @param vertex_count Number of vertices
 * @param vertex_stride Bytes per vertex
 * @param memory_access CPU access pattern
 * @param vertex_buffer Output buffer handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateVertexBuffer(
    VkDevice device,
    uint32_t vertex_count,
    uint32_t vertex_stride,
    D3D8_VULKAN_MEMORY_ACCESS memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* vertex_buffer);

/**
 * @brief Update vertex buffer data
 *
 * @param device Vulkan device
 * @param vertex_buffer Buffer to update
 * @param vertex_data Vertex data pointer
 * @param vertex_count Number of vertices
 * @param vertex_stride Bytes per vertex
 * @param start_vertex Starting vertex index
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_UpdateVertexBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* vertex_buffer,
    const void* vertex_data,
    uint32_t vertex_count,
    uint32_t vertex_stride,
    uint32_t start_vertex);

/* ============================================================================
 * Public API Functions - Index Buffer Management
 * ============================================================================ */

/**
 * @brief Create an index buffer
 *
 * @param device Vulkan device
 * @param index_count Number of indices
 * @param index_format 16-bit or 32-bit indices
 * @param memory_access CPU access pattern
 * @param index_buffer Output buffer handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateIndexBuffer(
    VkDevice device,
    uint32_t index_count,
    D3D8_VULKAN_INDEX_FORMAT index_format,
    D3D8_VULKAN_MEMORY_ACCESS memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* index_buffer);

/**
 * @brief Update index buffer data
 *
 * @param device Vulkan device
 * @param index_buffer Buffer to update
 * @param index_data Index data pointer
 * @param index_count Number of indices
 * @param index_format 16-bit or 32-bit format
 * @param start_index Starting index offset
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_UpdateIndexBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* index_buffer,
    const void* index_data,
    uint32_t index_count,
    D3D8_VULKAN_INDEX_FORMAT index_format,
    uint32_t start_index);

/* ============================================================================
 * Public API Functions - Buffer Pooling & Management
 * ============================================================================ */

/**
 * @brief Initialize a large pre-allocated buffer pool
 *
 * @param device Vulkan device
 * @param pool_size_bytes Total pool size
 * @param buffer_type Type of buffers to pool
 * @param memory_access Memory access pattern
 * @param pool_handle Output pool handle
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_CreateBufferPool(
    VkDevice device,
    uint64_t pool_size_bytes,
    D3D8_VULKAN_BUFFER_TYPE buffer_type,
    D3D8_VULKAN_MEMORY_ACCESS memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle);

/**
 * @brief Allocate buffer from pre-allocated pool
 *
 * @param pool_handle Pool to allocate from
 * @param size_bytes Size to allocate
 * @param buffer_handle Output sub-buffer handle
 * @return 0 on success (offset into pool), non-zero on error
 */
int D3D8_Vulkan_AllocateFromPool(
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle,
    uint64_t size_bytes,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle);

/**
 * @brief Deallocate buffer from pool (for reuse)
 *
 * @param pool_handle Pool handle
 * @param buffer_handle Buffer to deallocate
 */
void D3D8_Vulkan_DeallocateFromPool(
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle);

/**
 * @brief Destroy buffer pool
 *
 * @param device Vulkan device
 * @param pool_handle Pool to destroy
 */
void D3D8_Vulkan_DestroyBufferPool(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle);

/* ============================================================================
 * Public API Functions - Buffer Mapping
 * ============================================================================ */

/**
 * @brief Map buffer memory to CPU address space
 *
 * @param device Vulkan device
 * @param buffer_handle Buffer to map
 * @param mapped_ptr Output CPU pointer
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_MapBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    void** mapped_ptr);

/**
 * @brief Unmap buffer memory
 *
 * @param device Vulkan device
 * @param buffer_handle Buffer to unmap
 */
void D3D8_Vulkan_UnmapBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle);

/**
 * @brief Flush mapped buffer memory (for coherent memory)
 *
 * @param device Vulkan device
 * @param buffer_handle Buffer to flush
 * @param offset_bytes Offset to flush from
 * @param size_bytes Size to flush
 * @return 0 on success, non-zero on error
 */
int D3D8_Vulkan_FlushMappedBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    uint64_t offset_bytes,
    uint64_t size_bytes);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* D3D8_VULKAN_BUFFER_H_ */
