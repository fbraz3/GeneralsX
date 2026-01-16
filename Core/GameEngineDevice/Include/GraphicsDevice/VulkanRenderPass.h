/*
 * Phase 09.3: VulkanRenderPass - Render Pass and Attachment Management
 *
 * Encapsulates Vulkan render pass creation and management.
 * Defines:
 * - Attachment descriptions (color, depth, etc.)
 * - Subpass descriptions
 * - Attachment dependencies
 */

#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

#include "GraphicsDevice/vulkan_stubs.h"

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Vulkan render pass wrapper
 *
 * Encapsulates a Vulkan render pass which defines:
 * - What attachments (color, depth) are used
 * - How attachments are loaded and stored
 * - Subpass structure and dependencies
 * - Attachment layout transitions
 *
 * Typically used for:
 * - Main render pass (color + depth)
 * - Off-screen passes (custom render targets)
 */
class VulkanRenderPass {
public:
    /**
     * Constructor
     */
    VulkanRenderPass(VkDevice device);

    /**
     * Destructor
     */
    ~VulkanRenderPass();

    /**
     * Create standard render pass (color + depth)
     * @param colorFormat Color attachment format
     * @param depthFormat Depth attachment format
     * @return true if creation successful
     */
    bool create(VkFormat colorFormat, VkFormat depthFormat);

    /**
     * Create custom render pass
     * @param attachmentCount Number of attachments
     * @param attachmentFormats Array of attachment formats
     * @param hasDepth Include depth attachment?
     * @return true if creation successful
     */
    bool createCustom(int attachmentCount, const VkFormat* attachmentFormats, bool hasDepth);

    /**
     * Get Vulkan render pass handle
     */
    VkRenderPass getVkRenderPass() const { return renderPass; }

    /**
     * Get attachment count
     */
    int getAttachmentCount() const { return attachmentCount; }

private:
    VkDevice device;                    ///< Logical device
    VkRenderPass renderPass;            ///< Vulkan render pass
    int attachmentCount;                ///< Number of attachments

    /**
     * Create render pass from attachment descriptions
     */
    bool createRenderPass(const VkAttachmentDescription* attachments,
                         int attachmentCount);
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANRENDERPASS_H
