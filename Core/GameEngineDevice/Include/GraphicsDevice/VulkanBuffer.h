/*
 * Phase 09.3: VulkanBuffer - Vertex, Index, and Uniform Buffer Management
 *
 * Encapsulates Vulkan buffer resource lifecycle and management.
 * Supports:
 * - Vertex buffers (dynamic/static)
 * - Index buffers (16-bit and 32-bit indices)
 * - Uniform/constant buffers (descriptor bindings)
 */

#ifndef VULKANBUFFER_H
#define VULKANBUFFER_H

#include "GraphicsDevice/vulkan_stubs.h"

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Buffer usage type enumeration
 */
enum VulkanBufferUsage {
    BUFFER_USAGE_VERTEX = 0,
    BUFFER_USAGE_INDEX = 1,
    BUFFER_USAGE_UNIFORM = 2,
};

/**
 * @brief Vulkan buffer resource wrapper
 *
 * Manages a single Vulkan buffer with associated device memory.
 * Handles buffer creation, updates, and destruction.
 */
class VulkanBuffer {
public:
    /**
     * Constructor
     */
    VulkanBuffer(VkDevice device, VulkanBufferUsage usage);

    /**
     * Destructor
     */
    ~VulkanBuffer();

    /**
     * Create buffer with initial data
     * @param data Initial buffer data (can be nullptr)
     * @param size Buffer size in bytes
     * @param stride Bytes per element (for vertex buffers)
     * @param indexSize Bytes per index (for index buffers: 2 or 4)
     * @return true if creation successful
     */
    bool create(const void* data, size_t size, int stride, int indexSize);

    /**
     * Update buffer contents
     * @param data New buffer data
     * @param size Data size in bytes
     * @param offset Byte offset into buffer
     * @return true if update successful
     */
    bool update(const void* data, size_t size, size_t offset);

    /**
     * Get Vulkan buffer handle
     */
    VkBuffer getVkBuffer() const { return buffer; }

    /**
     * Get buffer size in bytes
     */
    size_t getSize() const { return size; }

    /**
     * Get stride (for vertex buffers)
     */
    int getStride() const { return stride; }

    /**
     * Get index size in bytes (for index buffers)
     */
    int getIndexSize() const { return indexSize; }

    /**
     * Get buffer usage type
     */
    VulkanBufferUsage getUsage() const { return usage; }

private:
    VkDevice device;                    ///< Logical device
    VkBuffer buffer;                    ///< Vulkan buffer handle
    VkDeviceMemory memory;              ///< Device memory
    size_t size;                        ///< Buffer size in bytes
    int stride;                         ///< Stride (vertex buffers)
    int indexSize;                      ///< Index size (index buffers)
    VulkanBufferUsage usage;            ///< Buffer usage type

    /**
     * Allocate device memory for buffer
     */
    bool allocateMemory();

    /**
     * Free device memory
     */
    void freeMemory();
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANBUFFER_H
