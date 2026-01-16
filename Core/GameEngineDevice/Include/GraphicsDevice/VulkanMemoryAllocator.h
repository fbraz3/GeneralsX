/*
 * Phase 09.3: VulkanMemoryAllocator - GPU Memory Management
 *
 * Manages GPU memory allocation using pooling strategy.
 * Provides separate pools for:
 * - Vertex buffers (frequent create/destroy)
 * - Textures (large, persistent allocations)
 * - Uniform buffers (frequent updates)
 */

#ifndef VULKANMEMORYALLOCATOR_H
#define VULKANMEMORYALLOCATOR_H

#include "GraphicsDevice/vulkan_stubs.h"
#include <cstddef>

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Memory pool for GPU allocations
 *
 * Manages a pool of VkDeviceMemory blocks.
 * Reduces fragmentation by pooling allocations by type.
 */
class VulkanMemoryPool {
public:
    /**
     * Constructor
     */
    VulkanMemoryPool();

    /**
     * Initialize memory pool
     * @param device Logical device
     * @param physicalDevice Physical device
     * @param memoryTypeIndex Memory type index for this pool
     * @param poolSize Initial pool size in bytes
     */
    bool init(VkDevice device, VkPhysicalDevice physicalDevice,
              uint32_t memoryTypeIndex, size_t poolSize);

    /**
     * Allocate memory from pool
     * @param size Allocation size
     * @param alignment Memory alignment requirement
     * @param outMemory [out] Allocated VkDeviceMemory
     * @param outOffset [out] Offset within memory block
     * @return true if allocation successful
     */
    bool allocate(size_t size, size_t alignment, VkDeviceMemory& outMemory, size_t& outOffset);

    /**
     * Free allocated memory
     * @param memory Memory block to free
     * @param offset Offset within memory block
     * @param size Size to free
     */
    void free(VkDeviceMemory memory, size_t offset, size_t size);

    /**
     * Get total pool size
     */
    size_t getTotalSize() const { return totalSize; }

    /**
     * Get used pool size
     */
    size_t getUsedSize() const { return usedSize; }

    /**
     * Cleanup pool
     */
    void cleanup();

private:
    VkDevice device;                    ///< Logical device
    VkDeviceMemory* memoryBlocks;       ///< Array of memory blocks
    int blockCount;                     ///< Number of blocks
    int maxBlocks;                      ///< Maximum blocks
    size_t totalSize;                   ///< Total pool size
    size_t usedSize;                    ///< Currently used
    uint32_t memoryTypeIndex;           ///< Memory type index

    /**
     * Allocate new memory block
     */
    bool allocateBlock(size_t minSize);
};

/**
 * @brief Vulkan memory allocator
 *
 * Manages GPU memory allocation with separate pools for different types.
 */
class VulkanMemoryAllocator {
public:
    /**
     * Constructor
     */
    VulkanMemoryAllocator();

    /**
     * Destructor
     */
    ~VulkanMemoryAllocator();

    /**
     * Initialize allocator
     * @param device Logical device
     * @param physicalDevice Physical device
     * @return true if initialization successful
     */
    bool init(VkDevice device, VkPhysicalDevice physicalDevice);

    /**
     * Allocate memory for buffer
     * @param requirements Memory requirements from vkGetBufferMemoryRequirements
     * @param usage VK_MEMORY_PROPERTY_* flags
     * @param outMemory [out] Allocated memory
     * @param outOffset [out] Offset within memory
     * @return true if allocation successful
     */
    bool allocateBufferMemory(const VkMemoryRequirements& requirements,
                             VkMemoryPropertyFlags usage,
                             VkDeviceMemory& outMemory, size_t& outOffset);

    /**
     * Allocate memory for image
     * @param requirements Memory requirements from vkGetImageMemoryRequirements
     * @param usage VK_MEMORY_PROPERTY_* flags
     * @param outMemory [out] Allocated memory
     * @param outOffset [out] Offset within memory
     * @return true if allocation successful
     */
    bool allocateImageMemory(const VkMemoryRequirements& requirements,
                            VkMemoryPropertyFlags usage,
                            VkDeviceMemory& outMemory, size_t& outOffset);

    /**
     * Free allocated memory
     */
    void freeMemory(VkDeviceMemory memory, size_t offset, size_t size);

    /**
     * Get memory statistics
     */
    size_t getTotalAllocated() const;
    size_t getTotalUsed() const;

    /**
     * Cleanup allocator
     */
    void cleanup();

private:
    VkDevice device;                    ///< Logical device
    VulkanMemoryPool vertexPool;        ///< Vertex buffer memory pool
    VulkanMemoryPool texturePool;       ///< Texture memory pool
    VulkanMemoryPool uniformPool;       ///< Uniform buffer memory pool
    bool initialized;                   ///< Initialization flag

    /**
     * Find suitable memory type
     */
    bool findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,
                       uint32_t& outTypeIndex) const;
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANMEMORYALLOCATOR_H
