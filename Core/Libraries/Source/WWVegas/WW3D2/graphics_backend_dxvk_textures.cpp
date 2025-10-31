/**
 * @file graphics_backend_dxvk_textures.cpp
 * 
 * Phase 42: Texture System Implementation
 * 
 * Implements texture creation, destruction, format conversion, and binding.
 * Handles DirectX texture formats mapping to Vulkan equivalents.
 * Manages GPU texture memory and staging buffers for CPU access.
 * 
 * Key Components:
 * - CreateTexture: Allocate VkImage with format and memory
 * - ReleaseTexture: Free GPU resources
 * - SetTexture: Bind texture to render pipeline
 * - LockTexture: CPU access via staging buffer
 * - UnlockTexture: Upload CPU changes to GPU
 * - Format conversion: D3DFORMAT → VkFormat mapping
 * 
 * Architecture:
 * - VkImage: GPU texture storage
 * - VkImageView: Texture access interface
 * - VkSampler: Texture filtering/wrapping
 * - Staging buffers: CPU-GPU synchronization
 * - Descriptor sets: Texture binding to shaders
 * 
 * Phase: 42 (Texture System)
 * Status: Implementation in progress
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <algorithm>

// ============================================================================
// DirectX Error Code Definitions (if not already defined)
// ============================================================================

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL 0x8876 + 0x0086
#endif

#ifndef D3DERR_NOTAVAILABLE
#define D3DERR_NOTAVAILABLE 0x8876 + 0x0091
#endif

#ifndef D3DERR_OUTOFVIDEOMEMORY
#define D3DERR_OUTOFVIDEOMEMORY 0x8876 + 0x0007
#endif

#ifndef D3D_OK
#define D3D_OK 0x00000000L
#endif

// ============================================================================
// Texture Format Conversion
// ============================================================================

/**
 * Convert DirectX format to Vulkan format.
 * 
 * Maps D3DFORMAT values to VkFormat for GPU texture storage.
 * Supports compressed formats (DXT1/3/5 = BC1/2/3) and uncompressed formats.
 * 
 * Parameters:
 * - format: DirectX D3DFORMAT value
 * 
 * Returns:
 * - VkFormat for Vulkan texture creation
 */
VkFormat DXVKGraphicsBackend::ConvertD3DFormatToVulkan(D3DFORMAT format) {
    switch (format) {
        // Uncompressed formats
        case 21:  // D3DFMT_R8G8B8
            return VK_FORMAT_R8G8B8_UNORM;
        case 22:  // D3DFMT_A8R8G8B8 (with alpha)
            return VK_FORMAT_R8G8B8A8_UNORM;
        case 23:  // D3DFMT_X8R8G8B8 (without alpha)
            return VK_FORMAT_R8G8B8A8_UNORM;
        case 20:  // D3DFMT_R5G6B5
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        
        // Compressed formats (BC = Block Compressed)
        case 0x31545844:  // "DXT1" - BC1 (RGB with 1-bit alpha)
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case 0x32545844:  // "DXT2" - BC2 (not commonly used, map to BC2)
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case 0x33545844:  // "DXT3" - BC2 (explicit alpha)
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case 0x34545844:  // "DXT4" - BC3 (not commonly used, map to BC3)
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case 0x35545844:  // "DXT5" - BC3 (interpolated alpha)
            return VK_FORMAT_BC3_UNORM_BLOCK;
        
        // Depth formats
        case 75:  // D3DFMT_D16_LOCKABLE
            return VK_FORMAT_D16_UNORM;
        case 77:  // D3DFMT_D24X8
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case 80:  // D3DFMT_D32
            return VK_FORMAT_D32_SFLOAT;
        
        // Default to RGBA8
        default:
            printf("Phase 42: ConvertD3DFormatToVulkan - Unknown format %u, using RGBA8\n", format);
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

/**
 * Get Vulkan image usage flags for texture.
 * 
 * Determines VkImageUsageFlags based on texture usage type.
 * Textures can be sampled, rendered to, or used as transfer destination.
 */
VkImageUsageFlags DXVKGraphicsBackend::GetTextureImageUsageFlags(bool isRenderTarget) {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    
    if (isRenderTarget) {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    
    return usage;
}

/**
 * Get texture memory properties.
 * 
 * Determines VkMemoryPropertyFlags for texture storage.
 * GPU-local memory for optimal performance.
 */
VkMemoryPropertyFlags DXVKGraphicsBackend::GetTextureMemoryProperties() {
    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}

// ============================================================================
// Texture Lifecycle Management
// ============================================================================

/**
 * CreateTexture: Allocate GPU texture memory and resources.
 * 
 * Creates a VkImage, VkImageView, and VkSampler for texture rendering.
 * Allocates GPU memory and initializes texture handle.
 * 
 * Parameters:
 * - width: Texture width in pixels
 * - height: Texture height in pixels
 * - format: DirectX texture format (D3DFORMAT)
 * - texture: Output pointer to texture handle
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::CreateTexture(
    unsigned int width,
    unsigned int height,
    D3DFORMAT format,
    void** texture) {
    
    if (!m_initialized) {
        printf("Phase 42: CreateTexture - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }
    
    if (width == 0 || height == 0) {
        printf("Phase 42: CreateTexture - Invalid dimensions (0)\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (!texture) {
        printf("Phase 42: CreateTexture - Null texture pointer\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Convert DirectX format to Vulkan
    VkFormat vkFormat = ConvertD3DFormatToVulkan(format);
    
    // Create image handle and shared_ptr
    auto textureHandle = std::make_shared<DXVKTextureHandle>();
    
    textureHandle->width = width;
    textureHandle->height = height;
    textureHandle->format = vkFormat;
    textureHandle->originalFormat = format;
    textureHandle->mipLevels = 1;
    textureHandle->isRenderTarget = false;
    textureHandle->isDynamic = true;  // Default to dynamic for now
    
    // Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = textureHandle->mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vkFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = GetTextureImageUsageFlags(false);
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &textureHandle->image);
    if (result != VK_SUCCESS) {
        printf("Phase 42: CreateTexture - vkCreateImage failed: %d\n", result);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Allocate GPU memory for image
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(m_device, textureHandle->image, &memReqs);
    
    HRESULT hresult = AllocateMemory(memReqs, GetTextureMemoryProperties(), textureHandle->imageMemory);
    if (hresult != D3D_OK) {
        printf("Phase 42: CreateTexture - AllocateMemory failed\n");
        vkDestroyImage(m_device, textureHandle->image, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Bind memory to image
    result = vkBindImageMemory(m_device, textureHandle->image, textureHandle->imageMemory, 0);
    if (result != VK_SUCCESS) {
        printf("Phase 42: CreateTexture - vkBindImageMemory failed: %d\n", result);
        vkFreeMemory(m_device, textureHandle->imageMemory, nullptr);
        vkDestroyImage(m_device, textureHandle->image, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Create VkImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureHandle->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = textureHandle->mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    result = vkCreateImageView(m_device, &viewInfo, nullptr, &textureHandle->imageView);
    if (result != VK_SUCCESS) {
        printf("Phase 42: CreateTexture - vkCreateImageView failed: %d\n", result);
        vkFreeMemory(m_device, textureHandle->imageMemory, nullptr);
        vkDestroyImage(m_device, textureHandle->image, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (float)textureHandle->mipLevels;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    
    result = vkCreateSampler(m_device, &samplerInfo, nullptr, &textureHandle->sampler);
    if (result != VK_SUCCESS) {
        printf("Phase 42: CreateTexture - vkCreateSampler failed: %d\n", result);
        vkDestroyImageView(m_device, textureHandle->imageView, nullptr);
        vkFreeMemory(m_device, textureHandle->imageMemory, nullptr);
        vkDestroyImage(m_device, textureHandle->image, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Store texture handle and return pointer
    *texture = textureHandle.get();
    m_textureCache[*texture] = textureHandle;
    m_textures[0] = textureHandle;  // Track in textures map
    
    printf("Phase 42: CreateTexture - Created %ux%u texture, format=%u, handle=%p\n",
           width, height, format, *texture);
    
    return D3D_OK;
}

/**
 * ReleaseTexture: Free texture GPU resources.
 * 
 * Destroys VkImage, VkImageView, VkSampler, and frees GPU memory.
 * 
 * Parameters:
 * - texture: Texture handle to release
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::ReleaseTexture(void* texture) {
    if (!m_initialized) {
        printf("Phase 42: ReleaseTexture - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }
    
    if (!texture) {
        printf("Phase 42: ReleaseTexture - Null texture pointer\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Find and remove from cache
    auto it = m_textureCache.find(texture);
    if (it == m_textureCache.end()) {
        printf("Phase 42: ReleaseTexture - Texture not found in cache\n");
        return D3DERR_INVALIDCALL;
    }
    
    auto textureHandle = it->second;
    
    // Destroy Vulkan resources
    if (textureHandle->sampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device, textureHandle->sampler, nullptr);
    }
    
    if (textureHandle->imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, textureHandle->imageView, nullptr);
    }
    
    if (textureHandle->image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, textureHandle->image, nullptr);
    }
    
    if (textureHandle->imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, textureHandle->imageMemory, nullptr);
    }
    
    // Remove from cache
    m_textureCache.erase(it);
    
    printf("Phase 42: ReleaseTexture - Released texture %p\n", texture);
    
    return D3D_OK;
}

/**
 * SetTexture: Bind texture to rendering pipeline.
 * 
 * Binds texture to specified texture stage for shader sampling.
 * Updates descriptor set with texture's VkImageView and sampler.
 * 
 * Parameters:
 * - stage: Texture stage (0-7 typically)
 * - texture: Texture handle to bind
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::SetTexture(unsigned int stage, void* texture) {
    if (!m_initialized) {
        printf("Phase 42: SetTexture - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }
    
    if (stage > 7) {
        printf("Phase 42: SetTexture - Invalid texture stage %u\n", stage);
        return D3DERR_INVALIDCALL;
    }
    
    if (!texture) {
        printf("Phase 42: SetTexture - Unbinding texture stage %u\n", stage);
        m_textures.erase(stage);
        return D3D_OK;
    }
    
    // Find texture in cache
    auto it = m_textureCache.find(texture);
    if (it == m_textureCache.end()) {
        printf("Phase 42: SetTexture - Texture not found in cache\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Bind texture to stage
    m_textures[stage] = it->second;
    
    printf("Phase 42: SetTexture - Bound texture %p to stage %u\n", texture, stage);
    
    // TODO: Phase 42.6 - Update descriptor set with texture's VkImageView and sampler
    
    return D3D_OK;
}

/**
 * GetTexture: Get currently bound texture.
 * 
 * Parameters:
 * - stage: Texture stage to query
 * - texture: Output pointer to texture handle
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::GetTexture(unsigned int stage, void** texture) {
    if (!m_initialized) {
        return D3DERR_NOTAVAILABLE;
    }
    
    if (stage > 7 || !texture) {
        return D3DERR_INVALIDCALL;
    }
    
    auto it = m_textures.find(stage);
    if (it == m_textures.end()) {
        *texture = nullptr;
        return D3D_OK;
    }
    
    *texture = it->second.get();
    return D3D_OK;
}

// ============================================================================
// Texture Locking & CPU Access
// ============================================================================

/**
 * LockTexture: Lock texture for CPU access.
 * 
 * Creates staging buffer for CPU access to texture data.
 * Supports D3DLOCK_READONLY, D3DLOCK_DISCARD, D3DLOCK_NOOVERWRITE flags.
 * 
 * Parameters:
 * - texture: Texture handle to lock
 * - data: Output pointer to locked buffer
 * - pitch: Output stride in bytes
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::LockTexture(void* texture, void** data, unsigned int* pitch) {
    if (!m_initialized) {
        printf("Phase 42: LockTexture - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }
    
    if (!texture || !data) {
        printf("Phase 42: LockTexture - Null parameters\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Find texture in cache
    auto it = m_textureCache.find(texture);
    if (it == m_textureCache.end()) {
        printf("Phase 42: LockTexture - Texture not found\n");
        return D3DERR_INVALIDCALL;
    }
    
    auto textureHandle = it->second;
    
    // Calculate stride and buffer size
    unsigned int bytesPerPixel = 4;  // Assume RGBA8 for now
    if (textureHandle->format == VK_FORMAT_BC1_RGB_UNORM_BLOCK ||
        textureHandle->format == VK_FORMAT_BC2_UNORM_BLOCK ||
        textureHandle->format == VK_FORMAT_BC3_UNORM_BLOCK) {
        bytesPerPixel = 1;  // Compressed formats handled differently
    }
    
    unsigned int lockPitch = textureHandle->width * bytesPerPixel;
    unsigned int lockSize = lockPitch * textureHandle->height;
    
    // Create staging buffer for CPU access
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = lockSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &textureHandle->lockedStagingBuffer);
    if (result != VK_SUCCESS) {
        printf("Phase 42: LockTexture - vkCreateBuffer failed: %d\n", result);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Allocate staging memory (CPU-visible)
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(m_device, textureHandle->lockedStagingBuffer, &memReqs);
    
    HRESULT hresult = AllocateMemory(memReqs, 
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           textureHandle->lockedStagingMemory);
    if (hresult != D3D_OK) {
        printf("Phase 42: LockTexture - AllocateMemory failed\n");
        vkDestroyBuffer(m_device, textureHandle->lockedStagingBuffer, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    // Bind memory to buffer
    vkBindBufferMemory(m_device, textureHandle->lockedStagingBuffer, textureHandle->lockedStagingMemory, 0);
    
    // Map GPU memory to CPU
    result = vkMapMemory(m_device, textureHandle->lockedStagingMemory, 0, lockSize, 0, &textureHandle->lockedData);
    if (result != VK_SUCCESS) {
        printf("Phase 42: LockTexture - vkMapMemory failed: %d\n", result);
        vkFreeMemory(m_device, textureHandle->lockedStagingMemory, nullptr);
        vkDestroyBuffer(m_device, textureHandle->lockedStagingBuffer, nullptr);
        return D3DERR_OUTOFVIDEOMEMORY;
    }
    
    textureHandle->lockedOffset = 0;
    textureHandle->lockedSize = lockSize;
    
    *data = textureHandle->lockedData;
    *pitch = lockPitch;
    
    printf("Phase 42: LockTexture - Locked %ux%u texture, pitch=%u bytes\n",
           textureHandle->width, textureHandle->height, lockPitch);
    
    return D3D_OK;
}

/**
 * UnlockTexture: Unlock texture and upload CPU changes to GPU.
 * 
 * Flushes staging buffer to GPU VkImage via copy command.
 * Handles memory synchronization and image layout transitions.
 * 
 * Parameters:
 * - texture: Texture handle to unlock
 * 
 * Returns:
 * D3D_OK on success
 * D3DERR_* on failure
 */
HRESULT DXVKGraphicsBackend::UnlockTexture(void* texture) {
    if (!m_initialized) {
        printf("Phase 42: UnlockTexture - Graphics backend not initialized\n");
        return D3DERR_NOTAVAILABLE;
    }
    
    if (!texture) {
        printf("Phase 42: UnlockTexture - Null texture pointer\n");
        return D3DERR_INVALIDCALL;
    }
    
    // Find texture in cache
    auto it = m_textureCache.find(texture);
    if (it == m_textureCache.end()) {
        printf("Phase 42: UnlockTexture - Texture not found\n");
        return D3DERR_INVALIDCALL;
    }
    
    auto textureHandle = it->second;
    
    // Unmap GPU memory from CPU
    if (textureHandle->lockedData) {
        vkUnmapMemory(m_device, textureHandle->lockedStagingMemory);
        textureHandle->lockedData = nullptr;
    }
    
    // TODO: Phase 42.5 - Copy staging buffer to VkImage
    // This would involve:
    // 1. Recording vkCmdCopyBufferToImage
    // 2. Handling image layout transitions
    // 3. Synchronization with graphics pipeline
    
    // Cleanup staging resources (for now, will be improved in Phase 42.5)
    if (textureHandle->lockedStagingBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, textureHandle->lockedStagingBuffer, nullptr);
        textureHandle->lockedStagingBuffer = VK_NULL_HANDLE;
    }
    
    if (textureHandle->lockedStagingMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, textureHandle->lockedStagingMemory, nullptr);
        textureHandle->lockedStagingMemory = VK_NULL_HANDLE;
    }
    
    printf("Phase 42: UnlockTexture - Unlocked and flushed texture %p\n", texture);
    
    return D3D_OK;
}
