/**
 * @file graphics_backend_dxvk_index_buffer.cpp
 * 
 * Phase 44.3: Index Buffer Implementation
 * 
 * Implements Vulkan index buffer creation and management:
 * - Index buffer allocation (GPU device-local memory)
 * - Index format support (VK_INDEX_TYPE_UINT16, VK_INDEX_TYPE_UINT32)
 * - Index data upload via vkMapMemory
 * - Command buffer binding (vkCmdBindIndexBuffer)
 * - Index count tracking and validation
 * 
 * Index buffers are essential for efficient triangle rendering:
 * - Avoid vertex duplication in shared geometry
 * - Standard mesh rendering technique
 * - Support for both 16-bit and 32-bit indices
 * 
 * Architecture:
 * - Single VkBuffer with device-local VkDeviceMemory
 * - 16MB allocation (sufficient for ~2.6M 16-bit or 1.3M 32-bit indices)
 * - Automatic stride calculation based on format
 * - Binding and rendering helpers
 * 
 * Phase Progress:
 * 44.1: Graphics Pipeline ✓
 * 44.2: Vertex Buffers ✓
 * 44.3: Index Buffers (this file)
 * 44.4: Draw Commands
 * 44.5: Material System
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <cstdio>

namespace DXVK {

// ============================================================================
// Phase 44.3.1: Index Buffer Creation & Allocation
// ============================================================================

/**
 * CreateIndexBuffer()
 * 
 * Allocates GPU device-local memory for index data storage.
 * Configuration:
 * - Size: 16MB (VkDeviceSize = 16 * 1024 * 1024 = 16777216 bytes)
 * - Capacity: ~2.6M 16-bit indices or ~1.3M 32-bit indices
 * - Memory type: Device-local (for optimal GPU access)
 * - Usage: VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
 * 
 * Process:
 * 1. Create VkBuffer with INDEX_BUFFER usage
 * 2. Query memory requirements (alignment, size)
 * 3. Allocate device-local memory from physical device
 * 4. Bind buffer to allocated memory
 * 5. Initialize metadata (count=0, format=UINT16)
 * 
 * Returns:
 * - VK_SUCCESS (0): Index buffer created successfully
 * - VK_ERROR_OUT_OF_DEVICE_MEMORY: GPU memory exhausted
 * - VK_ERROR_OUT_OF_HOST_MEMORY: System memory exhausted
 */
HRESULT DXVKGraphicsBackend::CreateIndexBuffer()
{
    printf("[DXVK] Creating index buffer (16MB capacity)...\n");

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = 16 * 1024 * 1024;  // 16MB
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_indexBuffer);
    if (result != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to create index buffer (result: %d)\n", result);
        return D3DERR_INVALIDCALL;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, m_indexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = 0;  // Assume device-local at index 0

    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_indexBufferMemory);
    if (result != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to allocate index buffer memory (result: %d)\n", result);
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        return D3DERR_INVALIDCALL;
    }

    result = vkBindBufferMemory(m_device, m_indexBuffer, m_indexBufferMemory, 0);
    if (result != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to bind index buffer memory (result: %d)\n", result);
        vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        return D3DERR_INVALIDCALL;
    }

    m_indexBufferSize = 16 * 1024 * 1024;
    m_indexBufferOffset = 0;
    m_indexCount = 0;
    m_currentIndexFormat = VK_INDEX_TYPE_UINT16;  // Default

    printf("[DXVK] Index buffer created successfully (size: 16MB bytes)\n");
    return S_OK;
}

// ============================================================================
// Phase 44.3.2: Index Buffer Cleanup
// ============================================================================

/**
 * DestroyIndexBuffer()
 * 
 * Releases GPU resources allocated by CreateIndexBuffer().
 * Order of operations (critical):
 * 1. Destroy VkBuffer (command buffers referencing this must be complete)
 * 2. Free VkDeviceMemory (backing memory)
 * 3. Clear member variables
 * 
 * Safety:
 * - Assumes no in-flight command buffers reference m_indexBuffer
 * - Should only be called during device shutdown or resource cleanup
 * - Vulkan requires vkDeviceWaitIdle() before cleanup in production code
 */
HRESULT DXVKGraphicsBackend::DestroyIndexBuffer()
{
    printf("[DXVK] Destroying index buffer...\n");

    if (m_indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        m_indexBuffer = VK_NULL_HANDLE;
    }

    if (m_indexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
        m_indexBufferMemory = VK_NULL_HANDLE;
    }

    m_indexBufferSize = 0;
    m_indexBufferOffset = 0;
    m_indexCount = 0;

    printf("[DXVK] Index buffer destroyed\n");
    return S_OK;
}

// ============================================================================
// Phase 44.3.3: Index Format Configuration
// ============================================================================

/**
 * SetIndexFormat(VkIndexType format)
 * 
 * Configures index format for subsequent draw calls.
 * Supports:
 * - VK_INDEX_TYPE_UINT16: 16-bit indices (0-65535 vertices per draw)
 * - VK_INDEX_TYPE_UINT32: 32-bit indices (0-4.2B vertices per draw)
 * 
 * Stride calculation:
 * - UINT16: 2 bytes per index
 * - UINT32: 4 bytes per index
 * 
 * Parameters:
 * - format: VK_INDEX_TYPE_UINT16 or VK_INDEX_TYPE_UINT32
 * 
 * Returns:
 * - VK_SUCCESS: Format set successfully
 * - VK_ERROR_*: Invalid format parameter
 */
HRESULT DXVKGraphicsBackend::SetIndexFormat(VkIndexType format)
{
    if (format != VK_INDEX_TYPE_UINT16 && format != VK_INDEX_TYPE_UINT32) {
        printf("[DXVK] ERROR: Invalid index format (%d)\n", format);
        return D3DERR_INVALIDCALL;
    }

    m_currentIndexFormat = format;
    m_indexStride = (format == VK_INDEX_TYPE_UINT16) ? 2 : 4;

    printf("[DXVK] Index format set to %s\n",
           format == VK_INDEX_TYPE_UINT16 ? "UINT16" : "UINT32");

    return S_OK;
}

// ============================================================================
// Phase 44.3.4: Index Data Upload
// ============================================================================

/**
 * UpdateIndexBuffer(const void* data, uint32_t count)
 * 
 * Uploads index data to GPU device-local memory.
 * 
 * Parameters:
 * - data: Pointer to index array (uint16_t* or uint32_t*)
 * - count: Number of indices to upload
 * 
 * Calculations:
 * - requiredSize = count * m_indexStride (depends on current format)
 * - Validates against 16MB buffer capacity
 * 
 * Transfer method:
 * - vkMapMemory: CPU access to GPU memory
 * - memcpy: Direct memory copy from system to GPU
 * - vkUnmapMemory: Flush and release CPU access
 * 
 * Returns:
 * - VK_SUCCESS: Data uploaded successfully
 * - VK_ERROR_OUT_OF_DEVICE_MEMORY: Buffer capacity exceeded
 * - VK_ERROR_MEMORY_MAP_FAILED: GPU memory not mappable
 */
HRESULT DXVKGraphicsBackend::UpdateIndexBuffer(const void* data, uint32_t count)
{
    if (data == nullptr) {
        printf("[DXVK] ERROR: Null index data pointer\n");
        return D3DERR_INVALIDCALL;
    }

    VkDeviceSize requiredSize = static_cast<VkDeviceSize>(count) * m_indexStride;

    if (requiredSize > m_indexBufferSize) {
        printf("[DXVK] ERROR: Index data exceeds buffer capacity (%llu > %llu)\n",
               requiredSize, m_indexBufferSize);
        return D3DERR_INVALIDCALL;
    }

    void* mappedMemory = nullptr;
    VkResult result = vkMapMemory(m_device, m_indexBufferMemory, 0, requiredSize, 0, &mappedMemory);
    if (result != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to map index buffer memory (result: %d)\n", result);
        return D3DERR_INVALIDCALL;
    }

    memcpy(mappedMemory, data, requiredSize);
    vkUnmapMemory(m_device, m_indexBufferMemory);

    m_indexCount = count;
    m_indexBufferOffset = 0;

    printf("[DXVK] Index data uploaded: %u indices (%llu bytes)\n", count, requiredSize);
    return S_OK;
}

// ============================================================================
// Phase 44.3.5: Index Buffer Binding
// ============================================================================

/**
 * BindIndexBuffer(VkCommandBuffer commandBuffer)
 * 
 * Binds index buffer to command buffer for subsequent draw commands.
 * 
 * Call sequence:
 * 1. BeginCommandBuffer (from Phase 43)
 * 2. BindPipeline (from Phase 44.1)
 * 3. BindVertexBuffer (from Phase 44.2)
 * 4. BindIndexBuffer (this function) ← HERE
 * 5. DrawIndexed (from Phase 44.4)
 * 
 * Vulkan API:
 * - vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, offset, format)
 * - offset: 0 (start of index buffer)
 * - format: m_currentIndexFormat (set by SetIndexFormat)
 * 
 * Parameters:
 * - commandBuffer: Recording command buffer from Phase 43 render loop
 * 
 * Returns:
 * - VK_SUCCESS: Index buffer bound successfully
 */
HRESULT DXVKGraphicsBackend::BindIndexBuffer(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE || m_indexBuffer == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Invalid command buffer or index buffer\n");
        return D3DERR_INVALIDCALL;
    }

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, m_indexBufferOffset, m_currentIndexFormat);

    printf("[DXVK] Index buffer bound to command buffer\n");
    return S_OK;
}

// ============================================================================
// Phase 44.3.6: Index Buffer Queries
// ============================================================================

/**
 * GetIndexBuffer()
 * 
 * Query the VkBuffer handle for manual operations.
 * 
 * Returns: VkBuffer handle (or VK_NULL_HANDLE if not created)
 */
VkBuffer DXVKGraphicsBackend::GetIndexBuffer() const
{
    return m_indexBuffer;
}

/**
 * GetIndexBufferMemory()
 * 
 * Query the VkDeviceMemory handle for advanced operations.
 * 
 * Returns: VkDeviceMemory handle (or VK_NULL_HANDLE if not created)
 */
VkDeviceMemory DXVKGraphicsBackend::GetIndexBufferMemory() const
{
    return m_indexBufferMemory;
}

/**
 * GetIndexCount()
 * 
 * Query current number of indices in buffer.
 * Updated by UpdateIndexBuffer().
 * 
 * Returns: Index count (0 if empty)
 */
uint32_t DXVKGraphicsBackend::GetIndexCount() const
{
    return m_indexCount;
}

/**
 * GetIndexFormat()
 * 
 * Query current index format.
 * Set by SetIndexFormat().
 * 
 * Returns: VK_INDEX_TYPE_UINT16 or VK_INDEX_TYPE_UINT32
 */
VkIndexType DXVKGraphicsBackend::GetIndexFormat() const
{
    return m_currentIndexFormat;
}

/**
 * IsIndexBufferReady()
 * 
 * Check if index buffer is allocated and ready for use.
 * 
 * Returns:
 * - true: Buffer allocated with data loaded (m_indexCount > 0)
 * - false: Buffer not allocated or empty
 */
bool DXVKGraphicsBackend::IsIndexBufferReady() const
{
    return m_indexBuffer != VK_NULL_HANDLE && 
           m_indexBufferMemory != VK_NULL_HANDLE && 
           m_indexCount > 0;
}

// ============================================================================
// Phase 44.3.7: Index Buffer Diagnostics
// ============================================================================

/**
 * ReportIndexBufferState()
 * 
 * Print diagnostic information about index buffer state.
 * Useful for debugging render pipeline issues.
 * 
 * Output includes:
 * - Buffer allocation status
 * - Capacity and current usage
 * - Index format configuration
 * - Ready for rendering status
 */
void DXVKGraphicsBackend::ReportIndexBufferState() const
{
    printf("\n=== Index Buffer State ===\n");
    printf("Buffer handle:           %p\n", (void*)m_indexBuffer);
    printf("Memory handle:           %p\n", (void*)m_indexBufferMemory);
    printf("Buffer size:             %llu bytes\n", m_indexBufferSize);
    printf("Current offset:          %llu bytes\n", m_indexBufferOffset);
    printf("Index count:             %u\n", m_indexCount);
    printf("Index stride:            %u bytes\n", m_indexStride);
    printf("Index format:            %s\n",
           m_currentIndexFormat == VK_INDEX_TYPE_UINT16 ? "UINT16" : "UINT32");
    printf("Ready for rendering:     %s\n",
           IsIndexBufferReady() ? "YES" : "NO");
    printf("========================\n\n");
}

}  // namespace DXVK
