/*
 * Vertex Buffer Management for DXVK Graphics Backend
 * 
 * Phase 44.2: Implements vertex buffer creation, binding, and management
 * using Vulkan API for geometry data storage and submission.
 * 
 * Functions:
 * - CreateVertexBuffer() - Create VkBuffer and allocate device memory
 * - DestroyVertexBuffer() - Clean up vertex buffers
 * - BindVertexBuffer() - Bind buffer to command buffer for rendering
 * - SetVertexFormat() - Configure vertex input layout (position, color, UV, etc)
 * - UpdateVertexBuffer() - Copy vertex data to GPU memory
 * - GetVertexBuffer() - Query vertex buffer handle
 * - IsVertexBufferReady() - Status check
 * - ReportVertexBufferState() - Diagnostics
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <cstdio>

// ============================================================================
// Vertex Buffer Creation and Initialization
// ============================================================================

HRESULT DXVKGraphicsBackend::CreateVertexBuffer() {
    if (!m_device || m_device == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Invalid Vulkan device in CreateVertexBuffer\n");
        return E_FAIL;
    }

    // Allocate vertex buffer (16MB for initial capacity - can hold ~2.6M vertices)
    const VkDeviceSize VERTEX_BUFFER_SIZE = 16 * 1024 * 1024;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = VERTEX_BUFFER_SIZE;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = nullptr;

    HRESULT hr = vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer);
    if (hr != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to create vertex buffer (0x%08lX)\n", hr);
        return E_FAIL;
    }

    printf("[DXVK] Vertex buffer created successfully (size: %zu bytes)\n", VERTEX_BUFFER_SIZE);

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirements);

    // Allocate device memory for vertex buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    hr = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexBufferMemory);
    if (hr != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to allocate vertex buffer memory (0x%08lX)\n", hr);
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
        return E_FAIL;
    }

    // Bind memory to buffer
    hr = vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);
    if (hr != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to bind vertex buffer memory (0x%08lX)\n", hr);
        vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
        m_vertexBufferMemory = VK_NULL_HANDLE;
        return E_FAIL;
    }

    m_vertexBufferSize = VERTEX_BUFFER_SIZE;
    m_vertexBufferOffset = 0;

    printf("[DXVK] Vertex buffer memory allocated and bound successfully\n");
    return S_OK;
}

// ============================================================================
// Vertex Buffer Cleanup
// ============================================================================

void DXVKGraphicsBackend::DestroyVertexBuffer() {
    if (m_device != VK_NULL_HANDLE) {
        if (m_vertexBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
            m_vertexBuffer = VK_NULL_HANDLE;
            printf("[DXVK] Vertex buffer destroyed\n");
        }

        if (m_vertexBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
            m_vertexBufferMemory = VK_NULL_HANDLE;
            printf("[DXVK] Vertex buffer memory freed\n");
        }
    }

    m_vertexBufferSize = 0;
    m_vertexBufferOffset = 0;
}

// ============================================================================
// Vertex Format Definition
// ============================================================================

HRESULT DXVKGraphicsBackend::SetVertexFormat(VertexFormat format) {
    if (!m_device || m_device == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Invalid Vulkan device in SetVertexFormat\n");
        return E_FAIL;
    }

    // Clear previous vertex attributes
    m_vertexAttributeDescriptions.clear();
    m_vertexBindingDescription = {};

    uint32_t offset = 0;
    uint32_t stride = 0;

    // Determine stride and create attribute descriptions based on format
    switch (format) {
        case VERTEX_FORMAT_POSITION_ONLY:
            // Position only: 3x float (12 bytes)
            stride = 12;
            m_vertexAttributeDescriptions.push_back({
                0,                                          // location
                0,                                          // binding
                VK_FORMAT_R32G32B32_SFLOAT,                // format
                offset                                      // offset
            });
            break;

        case VERTEX_FORMAT_POSITION_COLOR:
            // Position (12) + Color (4) = 16 bytes
            stride = 16;
            m_vertexAttributeDescriptions.push_back({
                0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0        // position
            });
            m_vertexAttributeDescriptions.push_back({
                1, 0, VK_FORMAT_R8G8B8A8_UNORM, 12         // color
            });
            break;

        case VERTEX_FORMAT_POSITION_UV:
            // Position (12) + UV (8) = 20 bytes
            stride = 20;
            m_vertexAttributeDescriptions.push_back({
                0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0        // position
            });
            m_vertexAttributeDescriptions.push_back({
                1, 0, VK_FORMAT_R32G32_SFLOAT, 12          // uv
            });
            break;

        case VERTEX_FORMAT_FULL:
            // Position (12) + Normal (12) + UV (8) + Color (4) = 36 bytes
            stride = 36;
            m_vertexAttributeDescriptions.push_back({
                0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0        // position
            });
            m_vertexAttributeDescriptions.push_back({
                1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12       // normal
            });
            m_vertexAttributeDescriptions.push_back({
                2, 0, VK_FORMAT_R32G32_SFLOAT, 24          // uv
            });
            m_vertexAttributeDescriptions.push_back({
                3, 0, VK_FORMAT_R8G8B8A8_UNORM, 32         // color
            });
            break;

        default:
            printf("[DXVK] ERROR: Unknown vertex format: %d\n", format);
            return E_INVALIDARG;
    }

    // Setup vertex binding description
    m_vertexBindingDescription.binding = 0;
    m_vertexBindingDescription.stride = stride;
    m_vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_currentVertexFormat = format;
    m_vertexStride = stride;

    printf("[DXVK] Vertex format set: format=%d, stride=%u, attributes=%zu\n",
           format, stride, m_vertexAttributeDescriptions.size());

    return S_OK;
}

// ============================================================================
// Vertex Data Upload
// ============================================================================

HRESULT DXVKGraphicsBackend::UpdateVertexBuffer(const void* vertexData, uint32_t vertexCount) {
    if (!vertexData || vertexCount == 0) {
        printf("[DXVK] ERROR: Invalid vertex data in UpdateVertexBuffer\n");
        return E_INVALIDARG;
    }

    if (m_vertexBuffer == VK_NULL_HANDLE || m_vertexBufferMemory == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Vertex buffer not initialized\n");
        return E_FAIL;
    }

    if (m_vertexStride == 0) {
        printf("[DXVK] ERROR: Vertex format not set\n");
        return E_FAIL;
    }

    // Calculate required size
    uint32_t requiredSize = vertexCount * m_vertexStride;

    if (requiredSize > m_vertexBufferSize) {
        printf("[DXVK] ERROR: Vertex data exceeds buffer capacity (%u > %zu)\n",
               requiredSize, m_vertexBufferSize);
        return E_FAIL;
    }

    // Use staging buffer to copy data (GPU device local memory)
    // For now, we'll use direct mapping if HOST_VISIBLE is available
    // In production, use staging buffer + transfer queue

    // Map memory and copy data
    void* mappedMemory = nullptr;
    HRESULT hr = vkMapMemory(m_device, m_vertexBufferMemory, 0, requiredSize, 0, &mappedMemory);
    if (hr != VK_SUCCESS) {
        printf("[DXVK] ERROR: Failed to map vertex buffer memory (0x%08lX)\n", hr);
        return E_FAIL;
    }

    std::memcpy(mappedMemory, vertexData, requiredSize);
    vkUnmapMemory(m_device, m_vertexBufferMemory);

    m_vertexCount = vertexCount;
    m_vertexBufferOffset = 0;

    printf("[DXVK] Vertex buffer updated: %u vertices (%u bytes)\n", vertexCount, requiredSize);
    return S_OK;
}

// ============================================================================
// Vertex Buffer Binding
// ============================================================================

HRESULT DXVKGraphicsBackend::BindVertexBuffer(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Invalid command buffer in BindVertexBuffer\n");
        return E_INVALIDARG;
    }

    if (m_vertexBuffer == VK_NULL_HANDLE) {
        printf("[DXVK] ERROR: Vertex buffer not initialized\n");
        return E_FAIL;
    }

    // Bind vertex buffer to command buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(
        commandBuffer,
        0,                          // firstBinding
        1,                          // bindingCount
        &m_vertexBuffer,            // pBuffers
        &offset                     // pOffsets
    );

    printf("[DXVK] Vertex buffer bound to command buffer\n");
    return S_OK;
}

// ============================================================================
// Vertex Buffer Queries
// ============================================================================

VkBuffer DXVKGraphicsBackend::GetVertexBuffer() const {
    return m_vertexBuffer;
}

VkDeviceMemory DXVKGraphicsBackend::GetVertexBufferMemory() const {
    return m_vertexBufferMemory;
}

uint32_t DXVKGraphicsBackend::GetVertexCount() const {
    return m_vertexCount;
}

uint32_t DXVKGraphicsBackend::GetVertexStride() const {
    return m_vertexStride;
}

bool DXVKGraphicsBackend::IsVertexBufferReady() const {
    return m_vertexBuffer != VK_NULL_HANDLE &&
           m_vertexBufferMemory != VK_NULL_HANDLE &&
           m_vertexCount > 0;
}

// ============================================================================
// Vertex Attribute Management
// ============================================================================

const std::vector<VkVertexInputAttributeDescription>& DXVKGraphicsBackend::GetVertexAttributeDescriptions() const {
    return m_vertexAttributeDescriptions;
}

const VkVertexInputBindingDescription& DXVKGraphicsBackend::GetVertexBindingDescription() const {
    return m_vertexBindingDescription;
}

// ============================================================================
// Diagnostics and State Reporting
// ============================================================================

void DXVKGraphicsBackend::ReportVertexBufferState() {
    printf("\n");
    printf("====== Vertex Buffer State Report ======\n");
    printf("Buffer handle:           %s\n", m_vertexBuffer != VK_NULL_HANDLE ? "VALID" : "NULL");
    printf("Memory handle:           %s\n", m_vertexBufferMemory != VK_NULL_HANDLE ? "VALID" : "NULL");
    printf("Buffer size:             %zu bytes\n", m_vertexBufferSize);
    printf("Current offset:          %zu bytes\n", m_vertexBufferOffset);
    printf("Vertex count:            %u\n", m_vertexCount);
    printf("Vertex stride:           %u bytes\n", m_vertexStride);
    printf("Vertex format:           %d\n", m_currentVertexFormat);
    printf("Vertex attributes:       %zu\n", m_vertexAttributeDescriptions.size());

    for (size_t i = 0; i < m_vertexAttributeDescriptions.size(); ++i) {
        const auto& attr = m_vertexAttributeDescriptions[i];
        printf("  [%zu] location=%u, offset=%u, format=%u\n",
               i, attr.location, attr.offset, attr.format);
    }

    printf("Binding stride:          %u bytes\n", m_vertexBindingDescription.stride);
    printf("Buffer ready:            %s\n", IsVertexBufferReady() ? "YES" : "NO");
    printf("========================================\n\n");
}
