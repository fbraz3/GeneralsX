/*
 * Phase 09.3: VulkanTexture - 2D Texture and Render Target Management
 *
 * Encapsulates Vulkan image resources including:
 * - 2D textures (diffuse, normal maps, etc.)
 * - Render targets (for off-screen rendering)
 * - Depth buffers (for depth testing)
 */

#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "GraphicsDevice/GraphicsDevice.h"
#include "GraphicsDevice/vulkan_stubs.h"

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Vulkan texture resource wrapper
 *
 * Manages a single Vulkan image with associated:
 * - Device memory
 * - Image view (for sampling)
 * - Sampler (for texture filtering/addressing)
 *
 * Supports:
 * - Regular textures (loaded from files)
 * - Render targets (for off-screen rendering)
 * - Depth buffers (for depth testing)
 */
class VulkanTexture {
public:
    /**
     * Constructor
     */
    VulkanTexture(VkDevice device, VkPhysicalDevice physicalDevice);

    /**
     * Destructor
     */
    ~VulkanTexture();

    /**
     * Create 2D texture from pixel data
     * @param data Pixel data (can be nullptr for uninitialized texture)
     * @param width Texture width
     * @param height Texture height
     * @param format Pixel format
     * @param mipLevels Number of mipmap levels
     * @return true if creation successful
     */
    bool create(const void* data, int width, int height,
                TextureFormat format, int mipLevels);

    /**
     * Create render target texture
     * @param width Render target width
     * @param height Render target height
     * @param format Pixel format
     * @return true if creation successful
     */
    bool createRenderTarget(int width, int height, TextureFormat format);

    /**
     * Create depth texture
     * @param width Depth buffer width
     * @param height Depth buffer height
     * @param format Depth format (DEPTH24 or DEPTH32F)
     * @return true if creation successful
     */
    bool createDepth(int width, int height, TextureFormat format);

    /**
     * Update texture contents
     * @param data New pixel data
     * @param x Update region X offset
     * @param y Update region Y offset
     * @param width Update region width
     * @param height Update region height
     * @return true if update successful
     */
    bool update(const void* data, int x, int y, int width, int height);

    /**
     * Get Vulkan image handle
     */
    VkImage getVkImage() const { return image; }

    /**
     * Get Vulkan image view
     */
    VkImageView getVkImageView() const { return imageView; }

    /**
     * Get Vulkan sampler
     */
    VkSampler getVkSampler() const { return sampler; }

    /**
     * Get Vulkan image format
     */
    VkFormat getVkFormat() const { return vkFormat; }

    /**
     * Get texture dimensions
     */
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    /**
     * Check if this is a render target
     */
    bool isRenderTarget() const { return renderTarget; }

    /**
     * Check if this is a depth buffer
     */
    bool isDepthBuffer() const { return depthBuffer; }

private:
    VkDevice device;                    ///< Logical device
    VkPhysicalDevice physicalDevice;    ///< Physical device
    VkImage image;                      ///< Vulkan image handle
    VkImageView imageView;              ///< Image view for sampling
    VkSampler sampler;                  ///< Sampler (filtering/addressing)
    VkDeviceMemory memory;              ///< Device memory
    VkFormat vkFormat;                  ///< Vulkan pixel format
    int width, height;                  ///< Texture dimensions
    int mipLevels;                      ///< Number of mip levels
    TextureFormat format;               ///< Original format (for reference)
    bool renderTarget;                  ///< Is this a render target?
    bool depthBuffer;                   ///< Is this a depth buffer?

    /**
     * Allocate device memory for image
     */
    bool allocateMemory();

    /**
     * Create image view
     */
    bool createImageView();

    /**
     * Create sampler for texture filtering
     */
    bool createSampler();

    /**
     * Transition image layout for rendering
     */
    bool transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANTEXTURE_H
