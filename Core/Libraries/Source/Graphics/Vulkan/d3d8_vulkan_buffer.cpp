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
**  FILE: d3d8_vulkan_buffer.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 12, 2025
**
**  PURPOSE: Stub Implementation of Vulkan Vertex & Index Buffer Management
**
**  DESCRIPTION:
**    Implements Vulkan buffer allocation, GPU memory management, and data
**    upload mechanisms for DirectX 8 compatibility layer. Uses stub
**    implementations with comprehensive logging for architectural validation.
**
**    Phase 11: Vertex & Index Buffers Implementation
**
******************************************************************************/

#include "d3d8_vulkan_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <cstdint>
#include <stdint.h>
/* ============================================================================
 * Internal State Management
 * ============================================================================ */

/**
 * @struct BufferPoolEntry
 * @brief Internal buffer pool tracking entry
 */
typedef struct {
    void* buffer_handle;                  /* VkBuffer handle */
    void* memory_handle;                  /* VkDeviceMemory handle */
    unsigned long long size_bytes;        /* Pool total size */
    unsigned long long allocated_bytes;   /* Bytes allocated from pool */
    unsigned int buffer_type;             /* D3D8_VULKAN_BUFFER_TYPE */
    int mapped;                           /* Boolean: currently mapped? */
    void* mapped_ptr;                     /* Mapped CPU pointer */
} BufferPoolEntry;

/**
 * @struct BufferEntry
 * @brief Internal buffer tracking entry
 */
typedef struct {
    void* buffer_handle;                  /* VkBuffer handle */
    void* memory_handle;                  /* VkDeviceMemory handle */
    unsigned long long size_bytes;        /* Allocation size */
    unsigned int buffer_type;             /* D3D8_VULKAN_BUFFER_TYPE */
    unsigned int memory_access;           /* D3D8_VULKAN_MEMORY_ACCESS */
    int mapped;                           /* Boolean: currently mapped? */
    void* mapped_ptr;                     /* Mapped CPU pointer */
    int coherent;                         /* Boolean: coherent memory? */
} BufferEntry;

/* Internal cache entries (max 64 for buffers, max 16 for pools) */
static BufferEntry buffer_cache[64] = {0};
static BufferPoolEntry pool_cache[16] = {0};

/* Internal counters for unique handle generation */
static unsigned int buffer_counter = 5000;
static unsigned int pool_counter = 6000;

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

/**
 * @brief Generate unique buffer handle
 * @return Unique handle pointer
 */
static VkBuffer generate_buffer_handle(void)
{
    return (VkBuffer)(uintptr_t)(buffer_counter++);
}

/**
 * @brief Generate unique pool handle
 * @return Unique handle pointer
 */
static VkBuffer generate_pool_handle(void)
{
    return (VkBuffer)(uintptr_t)(pool_counter++);
}

/**
 * @brief Find free slot in buffer cache
 * @return Cache index, or -1 if full
 */
static int find_free_buffer_slot(void)
{
    for (int i = 0; i < 64; i++) {
        if (buffer_cache[i].buffer_handle == NULL) {
            return i;
        }
    }
    printf("[Phase 11] WARNING: Buffer cache full (64 buffers)\n");
    return -1;
}

/**
 * @brief Find buffer entry by handle
 * @return Cache index, or -1 if not found
 */
static int find_buffer_by_handle(VkBuffer buffer)
{
    for (int i = 0; i < 64; i++) {
        if (buffer_cache[i].buffer_handle == buffer) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Find free pool slot
 * @return Cache index, or -1 if full
 */
static int find_free_pool_slot(void)
{
    for (int i = 0; i < 16; i++) {
        if (pool_cache[i].buffer_handle == NULL) {
            return i;
        }
    }
    printf("[Phase 11] WARNING: Buffer pool cache full (16 pools)\n");
    return -1;
}

/**
 * @brief Convert buffer type to string
 * @return Type string
 */
static const char* buffer_type_to_string(unsigned int type)
{
    switch (type) {
        case 0x1: return "VERTEX";
        case 0x2: return "INDEX";
        case 0x4: return "UNIFORM";
        case 0x8: return "STAGING";
        default: return "UNKNOWN";
    }
}

/**
 * @brief Convert memory access to string
 * @return Access string
 */
static const char* memory_access_to_string(unsigned int access)
{
    switch (access) {
        case 0: return "GPU_ONLY";
        case 1: return "GPU_OPTIMAL";
        case 2: return "HOST_VISIBLE";
        case 3: return "HOST_COHERENT";
        default: return "UNKNOWN";
    }
}

/* ============================================================================
 * Buffer Allocation Implementation
 * ============================================================================ */

int D3D8_Vulkan_AllocateBuffer(
    VkDevice device,
    const D3D8_VULKAN_BUFFER_CONFIG* config,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle)
{
    printf("[Phase 11] D3D8_Vulkan_AllocateBuffer: size=%llu type=%s access=%s\n",
           config->size_bytes,
           buffer_type_to_string(config->usage),
           memory_access_to_string(config->memory_access));

    if (device == NULL || config == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (config->size_bytes == 0) {
        printf("[Phase 11] ERROR: Buffer size is zero\n");
        return -1;
    }

    int slot = find_free_buffer_slot();
    if (slot < 0) {
        printf("[Phase 11] ERROR: No free buffer slots\n");
        return -1;
    }

    VkBuffer buffer = generate_buffer_handle();
    VkBuffer memory = generate_buffer_handle();

    buffer_cache[slot].buffer_handle = buffer;
    buffer_cache[slot].memory_handle = memory;
    buffer_cache[slot].size_bytes = config->size_bytes;
    buffer_cache[slot].buffer_type = config->usage;
    buffer_cache[slot].memory_access = config->memory_access;
    buffer_cache[slot].mapped = 0;
    buffer_cache[slot].mapped_ptr = NULL;
    buffer_cache[slot].coherent = config->coherent_memory;

    buffer_handle->gpu_buffer = buffer;
    buffer_handle->gpu_memory = memory;
    buffer_handle->size_bytes = config->size_bytes;
    buffer_handle->usage = config->usage;
    buffer_handle->memory_access = config->memory_access;
    buffer_handle->cpu_mapped_ptr = NULL;
    buffer_handle->is_coherent = config->coherent_memory;

    printf("[Phase 11] Allocated buffer: handle=%p memory=%p size=%llu\n", buffer, memory, config->size_bytes);
    return 0;
}

void D3D8_Vulkan_FreeBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle)
{
    printf("[Phase 11] D3D8_Vulkan_FreeBuffer: buffer=%p\n", buffer_handle->gpu_buffer);

    if (device == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return;
    }

    int slot = find_buffer_by_handle(buffer_handle->gpu_buffer);
    if (slot >= 0) {
        if (buffer_cache[slot].mapped) {
            printf("[Phase 11] WARNING: Freeing mapped buffer, unmapping first\n");
            D3D8_Vulkan_UnmapBuffer(device, buffer_handle);
        }
        memset(&buffer_cache[slot], 0, sizeof(BufferEntry));
        printf("[Phase 11] Freed buffer: slot=%d\n", slot);
    } else {
        printf("[Phase 11] WARNING: Buffer not found in cache\n");
    }

    memset(buffer_handle, 0, sizeof(D3D8_VULKAN_BUFFER_HANDLE));
}

int D3D8_Vulkan_AllocateStagingBuffer(
    VkDevice device,
    unsigned long long size_bytes,
    D3D8_VULKAN_BUFFER_HANDLE* staging_buffer)
{
    printf("[Phase 11] D3D8_Vulkan_AllocateStagingBuffer: size=%llu\n", size_bytes);

    if (device == NULL || staging_buffer == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }

    D3D8_VULKAN_BUFFER_CONFIG config = {
        .size_bytes = size_bytes,
        .usage = D3D8_VULKAN_BUFFER_STAGING,
        .memory_access = D3D8_VULKAN_MEMORY_HOST_VISIBLE,
        .coherent_memory = 1
    };

    return D3D8_Vulkan_AllocateBuffer(device, &config, staging_buffer);
}

/* ============================================================================
 * Data Transfer Implementation
 * ============================================================================ */

int D3D8_Vulkan_UploadBufferData(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    const void* data,
    unsigned long long data_size,
    unsigned long long offset_bytes)
{
    printf("[Phase 11] D3D8_Vulkan_UploadBufferData: buffer=%p size=%llu offset=%llu\n",
           buffer_handle->gpu_buffer, data_size, offset_bytes);

    if (device == NULL || buffer_handle == NULL || data == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (offset_bytes + data_size > buffer_handle->size_bytes) {
        printf("[Phase 11] ERROR: Data exceeds buffer bounds\n");
        return -1;
    }

    /* For stub: simulate direct upload by mapping, copying, unmapping */
    void* mapped_ptr = NULL;
    if (D3D8_Vulkan_MapBuffer(device, buffer_handle, &mapped_ptr) != 0) {
        printf("[Phase 11] ERROR: Failed to map buffer\n");
        return -1;
    }

    memcpy((uint8_t*)mapped_ptr + offset_bytes, data, data_size);
    printf("[Phase 11] Uploaded %llu bytes at offset %llu\n", data_size, offset_bytes);

    D3D8_Vulkan_UnmapBuffer(device, buffer_handle);
    return 0;
}

int D3D8_Vulkan_UploadBufferDataStaged(
    VkDevice device,
    VkQueue transfer_queue,
    VkCommandBuffer transfer_cmd,
    D3D8_VULKAN_BUFFER_HANDLE* staging_buffer,
    D3D8_VULKAN_BUFFER_HANDLE* destination_buffer,
    const void* data,
    unsigned long long data_size)
{
    printf("[Phase 11] D3D8_Vulkan_UploadBufferDataStaged: dest=%p size=%llu\n",
           destination_buffer->gpu_buffer, data_size);

    if (device == NULL || staging_buffer == NULL || destination_buffer == NULL || data == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (data_size > staging_buffer->size_bytes) {
        printf("[Phase 11] ERROR: Data exceeds staging buffer size\n");
        return -1;
    }

    /* Copy data to staging buffer */
    if (D3D8_Vulkan_UploadBufferData(device, staging_buffer, data, data_size, 0) != 0) {
        printf("[Phase 11] ERROR: Failed to upload to staging buffer\n");
        return -1;
    }

    printf("[Phase 11] Staged transfer: %llu bytes → GPU buffer\n", data_size);
    printf("[Phase 11] Transfer queue=%p cmd=%p\n", transfer_queue, transfer_cmd);
    return 0;
}

int D3D8_Vulkan_ReadBufferData(
    VkDevice device,
    const D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    void* output_data,
    unsigned long long data_size,
    unsigned long long offset_bytes)
{
    printf("[Phase 11] D3D8_Vulkan_ReadBufferData: buffer=%p size=%llu offset=%llu\n",
           buffer_handle->gpu_buffer, data_size, offset_bytes);

    if (device == NULL || buffer_handle == NULL || output_data == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (offset_bytes + data_size > buffer_handle->size_bytes) {
        printf("[Phase 11] ERROR: Read exceeds buffer bounds\n");
        return -1;
    }
    if (buffer_handle->cpu_mapped_ptr == NULL) {
        printf("[Phase 11] ERROR: Buffer is not mapped for reading\n");
        return -1;
    }

    memcpy(output_data, (uint8_t*)buffer_handle->cpu_mapped_ptr + offset_bytes, data_size);
    printf("[Phase 11] Read %llu bytes from offset %llu\n", data_size, offset_bytes);
    return 0;
}

/* ============================================================================
 * Vertex Buffer Management Implementation
 * ============================================================================ */

int D3D8_Vulkan_CreateVertexBuffer(
    VkDevice device,
    unsigned int vertex_count,
    unsigned int vertex_stride,
    unsigned int memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* vertex_buffer)
{
    printf("[Phase 11] D3D8_Vulkan_CreateVertexBuffer: count=%u stride=%u\n",
           vertex_count, vertex_stride);

    if (device == NULL || vertex_buffer == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (vertex_count == 0 || vertex_stride == 0) {
        printf("[Phase 11] ERROR: Invalid vertex count or stride\n");
        return -1;
    }

    unsigned long long buffer_size = (unsigned long long)vertex_count * vertex_stride;
    printf("[Phase 11] Vertex buffer size: %u vertices * %u bytes = %llu bytes\n",
           vertex_count, vertex_stride, buffer_size);

    D3D8_VULKAN_BUFFER_CONFIG config = {
        .size_bytes = buffer_size,
        .usage = D3D8_VULKAN_BUFFER_VERTEX,
        .memory_access = (D3D8_VULKAN_MEMORY_ACCESS)memory_access,
        .coherent_memory = (memory_access == 3)  /* HOST_COHERENT */
    };

    return D3D8_Vulkan_AllocateBuffer(device, &config, vertex_buffer);
}

int D3D8_Vulkan_UpdateVertexBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* vertex_buffer,
    const void* vertex_data,
    unsigned int vertex_count,
    unsigned int vertex_stride,
    unsigned int start_vertex)
{
    printf("[Phase 11] D3D8_Vulkan_UpdateVertexBuffer: count=%u start=%u\n",
           vertex_count, start_vertex);

    if (device == NULL || vertex_buffer == NULL || vertex_data == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }

    unsigned long long data_size = (unsigned long long)vertex_count * vertex_stride;
    unsigned long long offset = (unsigned long long)start_vertex * vertex_stride;

    printf("[Phase 11] Updating %u vertices (%llu bytes) at offset %llu\n",
           vertex_count, data_size, offset);

    return D3D8_Vulkan_UploadBufferData(device, vertex_buffer, vertex_data, data_size, offset);
}

/* ============================================================================
 * Index Buffer Management Implementation
 * ============================================================================ */

int D3D8_Vulkan_CreateIndexBuffer(
    VkDevice device,
    unsigned int index_count,
    unsigned int index_format,
    unsigned int memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* index_buffer)
{
    printf("[Phase 11] D3D8_Vulkan_CreateIndexBuffer: count=%u format=%s\n",
           index_count,
           (index_format == 0) ? "16BIT" : "32BIT");

    if (device == NULL || index_buffer == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (index_count == 0) {
        printf("[Phase 11] ERROR: Index count is zero\n");
        return -1;
    }

    unsigned int index_size = (index_format == 0) ? 2 : 4;  /* 16-bit or 32-bit */
    unsigned long long buffer_size = (unsigned long long)index_count * index_size;
    printf("[Phase 11] Index buffer size: %u indices * %u bytes = %llu bytes\n",
           index_count, index_size, buffer_size);

    D3D8_VULKAN_BUFFER_CONFIG config = {
        .size_bytes = buffer_size,
        .usage = D3D8_VULKAN_BUFFER_INDEX,
        .memory_access = (D3D8_VULKAN_MEMORY_ACCESS)memory_access,
        .coherent_memory = (memory_access == 3)
    };

    return D3D8_Vulkan_AllocateBuffer(device, &config, index_buffer);
}

int D3D8_Vulkan_UpdateIndexBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* index_buffer,
    const void* index_data,
    unsigned int index_count,
    unsigned int index_format,
    unsigned int start_index)
{
    printf("[Phase 11] D3D8_Vulkan_UpdateIndexBuffer: count=%u format=%s start=%u\n",
           index_count,
           (index_format == 0) ? "16BIT" : "32BIT",
           start_index);

    if (device == NULL || index_buffer == NULL || index_data == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }

    unsigned int index_size = (index_format == 0) ? 2 : 4;
    unsigned long long data_size = (unsigned long long)index_count * index_size;
    unsigned long long offset = (unsigned long long)start_index * index_size;

    printf("[Phase 11] Updating %u indices (%llu bytes) at offset %llu\n",
           index_count, data_size, offset);

    return D3D8_Vulkan_UploadBufferData(device, index_buffer, index_data, data_size, offset);
}

/* ============================================================================
 * Buffer Pooling & Management Implementation
 * ============================================================================ */

int D3D8_Vulkan_CreateBufferPool(
    VkDevice device,
    unsigned long long pool_size_bytes,
    unsigned int buffer_type,
    unsigned int memory_access,
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle)
{
    printf("[Phase 11] D3D8_Vulkan_CreateBufferPool: size=%llu type=%s access=%s\n",
           pool_size_bytes,
           buffer_type_to_string(buffer_type),
           memory_access_to_string(memory_access));

    if (device == NULL || pool_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (pool_size_bytes == 0) {
        printf("[Phase 11] ERROR: Pool size is zero\n");
        return -1;
    }

    int slot = find_free_pool_slot();
    if (slot < 0) {
        printf("[Phase 11] ERROR: No free pool slots\n");
        return -1;
    }

    VkBuffer pool_buffer = generate_pool_handle();
    VkBuffer pool_memory = generate_pool_handle();

    pool_cache[slot].buffer_handle = pool_buffer;
    pool_cache[slot].memory_handle = pool_memory;
    pool_cache[slot].size_bytes = pool_size_bytes;
    pool_cache[slot].allocated_bytes = 0;
    pool_cache[slot].buffer_type = buffer_type;
    pool_cache[slot].mapped = 0;
    pool_cache[slot].mapped_ptr = NULL;

    pool_handle->gpu_buffer = pool_buffer;
    pool_handle->gpu_memory = pool_memory;
    pool_handle->size_bytes = pool_size_bytes;
    pool_handle->usage = (D3D8_VULKAN_BUFFER_TYPE)buffer_type;
    pool_handle->memory_access = (D3D8_VULKAN_MEMORY_ACCESS)memory_access;
    pool_handle->cpu_mapped_ptr = NULL;
    pool_handle->is_coherent = (memory_access == 3);

    printf("[Phase 11] Created buffer pool: %llu bytes (type=%s)\n",
           pool_size_bytes, buffer_type_to_string(buffer_type));
    return 0;
}

int D3D8_Vulkan_AllocateFromPool(
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle,
    unsigned long long size_bytes,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle)
{
    printf("[Phase 11] D3D8_Vulkan_AllocateFromPool: size=%llu\n", size_bytes);

    if (pool_handle == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (size_bytes == 0) {
        printf("[Phase 11] ERROR: Allocation size is zero\n");
        return -1;
    }

    /* Find pool entry */
    BufferPoolEntry* pool = NULL;
    for (int i = 0; i < 16; i++) {
        if (pool_cache[i].buffer_handle == pool_handle->gpu_buffer) {
            pool = &pool_cache[i];
            break;
        }
    }

    if (pool == NULL) {
        printf("[Phase 11] ERROR: Pool not found\n");
        return -1;
    }

    unsigned long long free_space = pool->size_bytes - pool->allocated_bytes;
    if (size_bytes > free_space) {
        printf("[Phase 11] ERROR: Insufficient pool space: requested=%llu available=%llu\n",
               size_bytes, free_space);
        return -1;
    }

    /* Create sub-buffer handle */
    buffer_handle->gpu_buffer = (VkBuffer)((uintptr_t)pool->buffer_handle + pool->allocated_bytes);
    buffer_handle->gpu_memory = pool->memory_handle;
    buffer_handle->size_bytes = size_bytes;
    buffer_handle->usage = (D3D8_VULKAN_BUFFER_TYPE)pool->buffer_type;
    buffer_handle->memory_access = pool_handle->memory_access;
    buffer_handle->cpu_mapped_ptr = pool->mapped_ptr;
    buffer_handle->is_coherent = pool_handle->is_coherent;

    pool->allocated_bytes += size_bytes;
    printf("[Phase 11] Allocated %llu bytes from pool: offset=%llu remaining=%llu\n",
           size_bytes, (unsigned long long)(uintptr_t)buffer_handle->gpu_buffer, free_space - size_bytes);
    return 0;
}

void D3D8_Vulkan_DeallocateFromPool(
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle)
{
    printf("[Phase 11] D3D8_Vulkan_DeallocateFromPool\n");

    if (pool_handle == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return;
    }

    /* Stub: In real implementation, would track allocations and defragment */
    printf("[Phase 11] Note: Stub version does not reclaim pool memory\n");
}

void D3D8_Vulkan_DestroyBufferPool(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* pool_handle)
{
    printf("[Phase 11] D3D8_Vulkan_DestroyBufferPool: pool=%p\n", pool_handle->gpu_buffer);

    if (device == NULL || pool_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return;
    }

    for (int i = 0; i < 16; i++) {
        if (pool_cache[i].buffer_handle == pool_handle->gpu_buffer) {
            memset(&pool_cache[i], 0, sizeof(BufferPoolEntry));
            printf("[Phase 11] Destroyed buffer pool: slot=%d\n", i);
            return;
        }
    }

    printf("[Phase 11] WARNING: Pool not found in cache\n");
}

/* ============================================================================
 * Buffer Mapping Implementation
 * ============================================================================ */

int D3D8_Vulkan_MapBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    void** mapped_ptr)
{
    printf("[Phase 11] D3D8_Vulkan_MapBuffer: buffer=%p\n", buffer_handle->gpu_buffer);

    if (device == NULL || buffer_handle == NULL || mapped_ptr == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }

    int slot = find_buffer_by_handle(buffer_handle->gpu_buffer);
    if (slot < 0) {
        printf("[Phase 11] WARNING: Buffer not found in cache, assuming valid\n");
        /* Stub: allocate temporary memory for mapping */
        void* temp_ptr = malloc(buffer_handle->size_bytes);
        *mapped_ptr = temp_ptr;
        buffer_handle->cpu_mapped_ptr = temp_ptr;
        printf("[Phase 11] Allocated temporary mapped memory: %p\n", temp_ptr);
        return 0;
    }

    if (buffer_cache[slot].mapped) {
        printf("[Phase 11] ERROR: Buffer already mapped\n");
        return -1;
    }

    /* Stub: allocate temporary memory */
    void* temp_ptr = malloc(buffer_handle->size_bytes);
    buffer_cache[slot].mapped = 1;
    buffer_cache[slot].mapped_ptr = temp_ptr;
    buffer_handle->cpu_mapped_ptr = temp_ptr;
    *mapped_ptr = temp_ptr;

    printf("[Phase 11] Mapped buffer: temporary=%p size=%llu\n", temp_ptr, buffer_handle->size_bytes);
    return 0;
}

void D3D8_Vulkan_UnmapBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle)
{
    printf("[Phase 11] D3D8_Vulkan_UnmapBuffer: buffer=%p\n", buffer_handle->gpu_buffer);

    if (device == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return;
    }

    int slot = find_buffer_by_handle(buffer_handle->gpu_buffer);
    if (slot >= 0) {
        if (buffer_cache[slot].mapped_ptr) {
            free(buffer_cache[slot].mapped_ptr);
            printf("[Phase 11] Freed temporary mapped memory\n");
        }
        buffer_cache[slot].mapped = 0;
        buffer_cache[slot].mapped_ptr = NULL;
    }

    if (buffer_handle->cpu_mapped_ptr) {
        free(buffer_handle->cpu_mapped_ptr);
    }
    buffer_handle->cpu_mapped_ptr = NULL;
    printf("[Phase 11] Unmapped buffer\n");
}

int D3D8_Vulkan_FlushMappedBuffer(
    VkDevice device,
    D3D8_VULKAN_BUFFER_HANDLE* buffer_handle,
    unsigned long long offset_bytes,
    unsigned long long size_bytes)
{
    printf("[Phase 11] D3D8_Vulkan_FlushMappedBuffer: offset=%llu size=%llu\n",
           offset_bytes, size_bytes);

    if (device == NULL || buffer_handle == NULL) {
        printf("[Phase 11] ERROR: Invalid parameters\n");
        return -1;
    }
    if (offset_bytes + size_bytes > buffer_handle->size_bytes) {
        printf("[Phase 11] ERROR: Flush range exceeds buffer bounds\n");
        return -1;
    }
    if (!buffer_handle->is_coherent) {
        printf("[Phase 11] WARNING: Buffer is not coherent, flush needed\n");
    }

    printf("[Phase 11] Flushed %llu bytes at offset %llu\n", size_bytes, offset_bytes);
    return 0;
}
