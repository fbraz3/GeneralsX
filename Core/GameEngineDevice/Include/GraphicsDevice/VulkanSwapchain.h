/*
 * Phase 09.3: VulkanSwapchain - Window Surface and Swapchain Management
 *
 * Manages the Vulkan swapchain which handles:
 * - VkSurfaceKHR creation from SDL2 window
 * - Swapchain creation and configuration
 * - Image acquisition and presentation
 * - Swapchain recreation on window resize
 */

#ifndef VULKANSWAPCHAIN_H
#define VULKANSWAPCHAIN_H

#include "GraphicsDevice/vulkan_stubs.h"

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Vulkan swapchain manager
 *
 * Encapsulates swapchain operations for window presentation.
 * Handles:
 * - Creating Vulkan surface from SDL2 window
 * - Creating swapchain with appropriate format and extent
 * - Managing swapchain images and image views
 * - Swapchain recreation on window resize
 */
class VulkanSwapchain {
public:
    /**
     * Constructor
     */
    VulkanSwapchain(VkInstance instance, VkPhysicalDevice physicalDevice,
                    VkDevice device, VkQueue presentQueue);

    /**
     * Destructor
     */
    ~VulkanSwapchain();

    /**
     * Create swapchain from SDL2 window
     * @param window SDL2 window pointer (SDL_Window*)
     * @param width Desired width
     * @param height Desired height
     * @return true if creation successful
     */
    bool create(void* window, int width, int height);

    /**
     * Acquire next image from swapchain
     * @param availableSemaphore Semaphore signaled when image is available
     * @param fence Fence signaled when image is available (can be VK_NULL_HANDLE)
     * @param outImageIndex [out] Index of acquired image
     * @return true if acquisition successful
     */
    bool acquireNextImage(VkSemaphore availableSemaphore, VkFence fence, uint32_t& outImageIndex);

    /**
     * Present image to screen
     * @param presentQueue Queue to submit present command
     * @param imageIndex Index of image to present
     * @param waitSemaphore Semaphore to wait on before presenting
     * @return true if presentation successful
     */
    bool present(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore);

    /**
     * Recreate swapchain (on window resize)
     * @param width New width
     * @param height New height
     * @return true if recreation successful
     */
    bool recreate(int width, int height);

    /**
     * Get Vulkan surface
     */
    VkSurfaceKHR getVkSurface() const { return surface; }

    /**
     * Get Vulkan swapchain
     */
    VkSwapchainKHR getVkSwapchain() const { return swapchain; }

    /**
     * Get swapchain image count
     */
    uint32_t getImageCount() const { return imageCount; }

    /**
     * Get swapchain image at index
     */
    VkImage getImage(uint32_t index) const { return images[index]; }

    /**
     * Get swapchain image view at index
     */
    VkImageView getImageView(uint32_t index) const { return imageViews[index]; }

    /**
     * Get swapchain extent
     */
    VkExtent2D getExtent() const { return extent; }

    /**
     * Get swapchain format
     */
    VkFormat getFormat() const { return format; }

    /**
     * Check if out of date
     */
    bool isOutOfDate() const { return outOfDate; }

private:
    VkInstance instance;                ///< Vulkan instance
    VkPhysicalDevice physicalDevice;    ///< Physical device
    VkDevice device;                    ///< Logical device
    VkSurfaceKHR surface;               ///< Window surface
    VkSwapchainKHR swapchain;           ///< Swapchain handle
    VkFormat format;                    ///< Image format
    VkColorSpaceKHR colorSpace;         ///< Color space
    VkExtent2D extent;                  ///< Image extent (width, height)
    VkImage* images;                    ///< Swapchain images
    VkImageView* imageViews;            ///< Image views
    uint32_t imageCount;                ///< Number of images
    bool outOfDate;                     ///< Is swapchain out of date?

    /**
     * Choose surface format
     */
    bool chooseSurfaceFormat();

    /**
     * Choose presentation mode
     */
    VkPresentModeKHR choosePresentMode();

    /**
     * Choose swapchain extent
     */
    VkExtent2D chooseExtent(int width, int height);

    /**
     * Create image views from swapchain images
     */
    bool createImageViews();

    /**
     * Destroy current swapchain
     */
    void destroySwapchain();
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANSWAPCHAIN_H
