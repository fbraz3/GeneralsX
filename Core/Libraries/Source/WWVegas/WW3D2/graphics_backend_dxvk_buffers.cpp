/**
 * @file graphics_backend_dxvk_buffers.cpp
 * 
 * Vulkan Graphics Backend - Texture and Buffer Management
 * 
 * Implements texture creation, locking, vertex/index buffer management.
 * 
 * Phase: 39.2.5 (Remaining Methods Implementation)
 * 
 * Created: October 30, 2025
 */

#include "graphics_backend_dxvk.h"

// ============================================================================
// Texture Management
// ============================================================================

/**
 * Create a texture resource.
 */
HRESULT DXVKGraphicsBackend::CreateTexture(
    unsigned int width,
    unsigned int height,
    D3DFORMAT format,
    void** texture) {
    
    if (!m_initialized || texture == nullptr) {
        return E_INVALIDARG;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] CreateTexture: %ux%u, format=%u\n", width, height, format);
    }
    
    // Convert D3D format to Vulkan format
    VkFormat vkFormat = ConvertD3DFormatToVulkan(format);
    if (vkFormat == VK_FORMAT_UNDEFINED) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: Unsupported texture format: %u\n", format);
        }
        return E_INVALIDARG;
    }
    
    // Create Vulkan image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = vkFormat;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;  // Single mip level for Phase 39.2.5
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VkImage image = VK_NULL_HANDLE;
    VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &image);
    if (result != VK_SUCCESS) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: Failed to create image: %d\n", result);
        }
        return E_FAIL;
    }
    
    // Get memory requirements
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(m_device, image, &memReqs);
    
    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory);
    if (result != VK_SUCCESS) {
        vkDestroyImage(m_device, image, nullptr);
        if (m_debugOutput) {
            printf("[DXVK] ERROR: Failed to allocate image memory: %d\n", result);
        }
        return E_FAIL;
    }
    
    // Bind memory
    vkBindImageMemory(m_device, image, imageMemory, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView = VK_NULL_HANDLE;
    result = vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        vkFreeMemory(m_device, imageMemory, nullptr);
        vkDestroyImage(m_device, image, nullptr);
        if (m_debugOutput) {
            printf("[DXVK] ERROR: Failed to create image view: %d\n", result);
        }
        return E_FAIL;
    }
    
    // Create texture handle
    auto texHandle = new DXVKTextureHandle();
    texHandle->image = image;
    texHandle->imageMemory = imageMemory;
    texHandle->imageView = imageView;
    texHandle->format = vkFormat;
    texHandle->width = width;
    texHandle->height = height;
    texHandle->mipLevels = imageInfo.mipLevels;
    
    // Store in texture cache
    m_textureCache[texHandle] = std::make_shared<DXVKTextureHandle>(*texHandle);
    
    *texture = texHandle;
    
    if (m_debugOutput) {
        printf("[DXVK] CreateTexture: SUCCESS, handle=%p\n", texHandle);
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Lock texture for CPU access.
 */
HRESULT DXVKGraphicsBackend::LockTexture(
    void* texture,
    void** data,
    unsigned int* pitch) {
    
    if (!m_initialized || texture == nullptr || data == nullptr || pitch == nullptr) {
        return E_INVALIDARG;
    }
    
    auto texHandle = (DXVKTextureHandle*)texture;
    
    if (m_debugOutput) {
        printf("[DXVK] LockTexture: texture=%p\n", texture);
    }
    
    // In a real implementation, we would need to:
    // 1. Create a staging buffer
    // 2. Copy from GPU image to staging buffer
    // 3. Map the staging buffer for CPU access
    // 4. Return pointer to mapped memory
    
    // For Phase 39.2.5, we'll create a temporary staging buffer
    VkDeviceSize imageSize = texHandle->width * texHandle->height * 4; // Assuming RGBA
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    
    // Create staging buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer);
    if (result != VK_SUCCESS) {
        return E_FAIL;
    }
    
    // Allocate memory for staging buffer
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReqs);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &stagingMemory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        return E_FAIL;
    }
    
    // Bind memory
    vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0);
    
    // Map memory for CPU access
    void* mappedData = nullptr;
    result = vkMapMemory(m_device, stagingMemory, 0, imageSize, 0, &mappedData);
    if (result != VK_SUCCESS) {
        vkFreeMemory(m_device, stagingMemory, nullptr);
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        return E_FAIL;
    }
    
    // Store lock information for UnlockTexture
    texHandle->lockedData = mappedData;
    texHandle->lockedStagingBuffer = stagingBuffer;
    texHandle->lockedStagingMemory = stagingMemory;
    texHandle->lockedSize = imageSize;
    
    // Set locked data output
    *data = mappedData;
    *pitch = texHandle->width * 4; // Assuming RGBA (4 bytes per pixel)
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Unlock texture after CPU access.
 */
HRESULT DXVKGraphicsBackend::UnlockTexture(void* texture) {
    if (!m_initialized || texture == nullptr) {
        return E_INVALIDARG;
    }
    
    auto texHandle = (DXVKTextureHandle*)texture;
    
    if (m_debugOutput) {
        printf("[DXVK] UnlockTexture: texture=%p\n", texture);
    }
    
    if (texHandle->lockedData) {
        // Unmap memory
        vkUnmapMemory(m_device, texHandle->lockedStagingMemory);
        
        // Copy from staging buffer to GPU image (would normally do this on GPU)
        // For Phase 39.2.5, this is a placeholder
        
        // Cleanup staging resources
        vkDestroyBuffer(m_device, texHandle->lockedStagingBuffer, nullptr);
        vkFreeMemory(m_device, texHandle->lockedStagingMemory, nullptr);
        
        texHandle->lockedData = nullptr;
        texHandle->lockedStagingBuffer = VK_NULL_HANDLE;
        texHandle->lockedStagingMemory = VK_NULL_HANDLE;
    }
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Release texture resource.
 */
HRESULT DXVKGraphicsBackend::ReleaseTexture(void* texture) {
    if (!m_initialized || texture == nullptr) {
        return E_INVALIDARG;
    }
    
    auto texHandle = (DXVKTextureHandle*)texture;
    
    if (m_debugOutput) {
        printf("[DXVK] ReleaseTexture: texture=%p\n", texture);
    }
    
    // Destroy image view
    if (texHandle->imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, texHandle->imageView, nullptr);
    }
    
    // Destroy image
    if (texHandle->image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, texHandle->image, nullptr);
    }
    
    // Free memory
    if (texHandle->imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, texHandle->imageMemory, nullptr);
    }
    
    // Remove from cache
    m_textureCache.erase(texHandle);
    
    // Delete handle
    delete texHandle;
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Vertex Buffer Management
// ============================================================================

/**
 * Create a vertex buffer.
 */
HRESULT DXVKGraphicsBackend::CreateVertexBuffer(
    unsigned int size,
    unsigned int usage,
    unsigned int format,
    void** buffer) {
    
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] CreateVertexBuffer: size=%u\n", size);
    }
    
    // Create Vulkan buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer vkBuffer;
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &vkBuffer);
    if (result != VK_SUCCESS) {
        return E_FAIL;
    }
    
    // Get memory requirements
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, vkBuffer, &memReqs);
    
    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkDeviceMemory bufferMemory;
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(m_device, vkBuffer, nullptr);
        return E_FAIL;
    }
    
    // Bind memory
    vkBindBufferMemory(m_device, vkBuffer, bufferMemory, 0);
    
    // Create buffer handle
    auto bufHandle = new DXVKBufferHandle();
    bufHandle->buffer = vkBuffer;
    bufHandle->bufferMemory = bufferMemory;
    bufHandle->size = size;
    
    *buffer = bufHandle;
    
    return S_OK;
}

/**
 * Release a vertex buffer.
 */
HRESULT DXVKGraphicsBackend::ReleaseVertexBuffer(void* buffer) {
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    auto bufHandle = (DXVKBufferHandle*)buffer;
    
    // Destroy buffer
    if (bufHandle->buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, bufHandle->buffer, nullptr);
    }
    
    // Free memory
    if (bufHandle->bufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, bufHandle->bufferMemory, nullptr);
    }
    
    // Delete handle
    delete bufHandle;
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Lock vertex buffer for CPU access.
 */
HRESULT DXVKGraphicsBackend::LockVertexBuffer(
    void* buffer,
    void** data,
    unsigned int flags) {
    
    if (!m_initialized || buffer == nullptr || data == nullptr) {
        return E_INVALIDARG;
    }
    
    auto bufHandle = (DXVKBufferHandle*)buffer;
    
    if (m_debugOutput) {
        printf("[DXVK] LockVertexBuffer: buffer=%p\n", buffer);
    }
    
    // Create staging buffer for CPU access
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    
    // Use full buffer size for lock
    VkDeviceSize lockSize = bufHandle->size;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = lockSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer);
    if (result != VK_SUCCESS) {
        return E_FAIL;
    }
    
    // Allocate memory
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReqs);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &stagingMemory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        return E_FAIL;
    }
    
    vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0);
    
    // Map memory
    void* mappedData;
    result = vkMapMemory(m_device, stagingMemory, 0, lockSize, 0, &mappedData);
    if (result != VK_SUCCESS) {
        vkFreeMemory(m_device, stagingMemory, nullptr);
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        return E_FAIL;
    }
    
    // Store lock info
    bufHandle->lockedData = mappedData;
    bufHandle->lockedStagingBuffer = stagingBuffer;
    bufHandle->lockedStagingMemory = stagingMemory;
    bufHandle->lockedOffset = 0;  // Lock from beginning
    bufHandle->lockedSize = lockSize;
    
    *data = mappedData;
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Unlock vertex buffer.
 */
HRESULT DXVKGraphicsBackend::UnlockVertexBuffer(void* buffer) {
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    auto bufHandle = (DXVKBufferHandle*)buffer;
    
    if (m_debugOutput) {
        printf("[DXVK] UnlockVertexBuffer: buffer=%p\n", buffer);
    }
    
    if (bufHandle->lockedData) {
        vkUnmapMemory(m_device, bufHandle->lockedStagingMemory);
        
        // Copy from staging buffer to GPU buffer
        // This would normally be done via GPU command buffer
        
        vkDestroyBuffer(m_device, bufHandle->lockedStagingBuffer, nullptr);
        vkFreeMemory(m_device, bufHandle->lockedStagingMemory, nullptr);
        
        bufHandle->lockedData = nullptr;
        bufHandle->lockedStagingBuffer = VK_NULL_HANDLE;
        bufHandle->lockedStagingMemory = VK_NULL_HANDLE;
    }
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Index Buffer Management
// ============================================================================

/**
 * Create an index buffer.
 */
HRESULT DXVKGraphicsBackend::CreateIndexBuffer(
    unsigned int size,
    D3DFORMAT format,
    void** buffer) {
    
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] CreateIndexBuffer: size=%u\n", size);
    }
    
    // Create Vulkan buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer vkBuffer;
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &vkBuffer);
    if (result != VK_SUCCESS) {
        return E_FAIL;
    }
    
    // Get memory requirements
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, vkBuffer, &memReqs);
    
    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkDeviceMemory bufferMemory;
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(m_device, vkBuffer, nullptr);
        return E_FAIL;
    }
    
    // Bind memory
    vkBindBufferMemory(m_device, vkBuffer, bufferMemory, 0);
    
    // Create buffer handle
    auto bufHandle = new DXVKBufferHandle();
    bufHandle->buffer = vkBuffer;
    bufHandle->bufferMemory = bufferMemory;
    bufHandle->size = size;
    
    *buffer = bufHandle;
    
    return S_OK;
}

/**
 * Release an index buffer.
 */
HRESULT DXVKGraphicsBackend::ReleaseIndexBuffer(void* buffer) {
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    auto bufHandle = (DXVKBufferHandle*)buffer;
    
    // Destroy buffer
    if (bufHandle->buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, bufHandle->buffer, nullptr);
    }
    
    // Free memory
    if (bufHandle->bufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, bufHandle->bufferMemory, nullptr);
    }
    
    // Delete handle
    delete bufHandle;
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Lock index buffer for CPU access.
 */
HRESULT DXVKGraphicsBackend::LockIndexBuffer(
    void* buffer,
    void** data,
    unsigned int flags) {
    
    if (!m_initialized || buffer == nullptr || data == nullptr) {
        return E_INVALIDARG;
    }
    
    auto bufHandle = (DXVKBufferHandle*)buffer;
    
    if (m_debugOutput) {
        printf("[DXVK] LockIndexBuffer: buffer=%p\n", buffer);
    }
    
    // Same process as LockVertexBuffer
    return LockVertexBuffer(buffer, data, flags);
}

/**
 * Unlock index buffer.
 */
HRESULT DXVKGraphicsBackend::UnlockIndexBuffer(void* buffer) {
    if (!m_initialized || buffer == nullptr) {
        return E_INVALIDARG;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] UnlockIndexBuffer: buffer=%p\n", buffer);
    }
    
    // Same process as UnlockVertexBuffer
    return UnlockVertexBuffer(buffer);
}

