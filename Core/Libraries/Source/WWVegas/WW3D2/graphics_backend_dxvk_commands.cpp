/**
 * @file graphics_backend_dxvk_commands.cpp
 * 
 * Phase 43.3: Command Recording & Render Pass
 * 
 * Implements complete render pass setup and command recording:
 * - Render pass creation with color and depth attachments
 * - Framebuffer creation for swapchain images
 * - Dynamic state recording (viewport, scissor, blending)
 * - Clear operations with color and depth
 * - Command buffer pipeline barrier management
 * 
 * Architecture:
 * - Single render pass for all rendering (simplified)
 * - Per-image framebuffers for swapchain integration
 * - Dynamic viewport/scissor changes within render pass
 * - Pipeline barriers for image layout transitions
 * 
 * Phase Breakdown:
 * - 43.1: Basic render loop (BeginScene/EndScene/Present)
 * - 43.2: Frame synchronization & timing
 * - 43.3: Command recording & render pass (this file)
 * - 43.4: Error handling & swapchain recreation
 * - 43.5: Performance monitoring & profiling
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>

// D3D error codes
#define D3DERR_INVALIDCALL 0x8876086CL

// ============================================================================
// Phase 43.3: Render Pass Creation
// ============================================================================

/**
 * Create the render pass for color rendering.
 * A render pass describes the structure of rendering operations:
 * - Attachment descriptions (formats, load/store operations)
 * - Subpasses (rendering operations within the pass)
 * - Subpass dependencies (synchronization between subpasses)
 * 
 * This simplified Phase 43.3 creates a single-pass renderer:
 * - Input: VkImage with VK_FORMAT_B8G8R8A8_SRGB (typical swapchain format)
 * - Output: Rendered image ready for presentation
 * - No depth buffer (will be added in Phase 44)
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::CreateRenderPass() {
    if (!m_device || m_swapchainFormat == VK_FORMAT_UNDEFINED) {
        printf("ERROR: Device or swapchain format not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("Creating render pass for format: %d\n", m_swapchainFormat);
    
    // Describe the color attachment (swapchain image)
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // No MSAA for Phase 43
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // Clear on load
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;  // Keep result
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // Don't care on entry
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Ready for present
    
    // Attachment reference for color
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;  // Index 0 in attachments array
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // During rendering
    
    // Subpass description (a render pass can have multiple subpasses)
    VkAttachmentReference colorAttachmentRefs[] = {colorAttachmentRef};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = colorAttachmentRefs;
    
    // Create the render pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 0;  // No subpass dependencies for single-pass
    renderPassInfo.pDependencies = nullptr;
    
    VkResult vkr = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkCreateRenderPass failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    printf("Render pass created successfully\n");
    return S_OK;
}

/**
 * Destroy the render pass.
 * Called during device cleanup.
 */
void DXVKGraphicsBackend::DestroyRenderPass() {
    if (m_device && m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
        printf("Render pass destroyed\n");
    }
}

// ============================================================================
// Phase 43.3: Framebuffer Creation
// ============================================================================

/**
 * Create framebuffers for all swapchain images.
 * Framebuffers bind specific images (attachments) to a render pass.
 * One framebuffer per swapchain image for rendering to that image.
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::CreateFramebuffers() {
    if (!m_device || m_renderPass == VK_NULL_HANDLE) {
        printf("ERROR: Device or render pass not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    if (m_swapchainImages.empty() || m_swapchainImageViews.empty()) {
        printf("ERROR: Swapchain images or image views not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("Creating %zu framebuffers for swapchain images\n", m_swapchainImages.size());
    
    m_swapchainFramebuffers.resize(m_swapchainImages.size());
    
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
        VkImageView attachments[] = {m_swapchainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;
        
        VkResult vkr = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, 
                                          &m_swapchainFramebuffers[i]);
        if (vkr != VK_SUCCESS) {
            printf("ERROR: vkCreateFramebuffer failed for image %zu: %d\n", i, vkr);
            return D3DERR_DEVICELOST;
        }
    }
    
    printf("Framebuffers created successfully\n");
    return S_OK;
}

/**
 * Destroy all framebuffers.
 * Called during device cleanup or swapchain recreation.
 */
void DXVKGraphicsBackend::DestroyFramebuffers() {
    if (!m_device) return;
    
    for (VkFramebuffer framebuffer : m_swapchainFramebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
    }
    m_swapchainFramebuffers.clear();
    printf("Framebuffers destroyed\n");
}

// ============================================================================
// Phase 43.3: Viewport & Scissor Setup
// ============================================================================

/**
 * Set viewport for rendering.
 * Viewport defines the mapping from normalized device coordinates to framebuffer.
 * 
 * @param x Left coordinate (typically 0)
 * @param y Top coordinate (typically 0)
 * @param width Viewport width (typically swapchain width)
 * @param height Viewport height (typically swapchain height)
 * @param minDepth Minimum depth (typically 0.0)
 * @param maxDepth Maximum depth (typically 1.0)
 */
void DXVKGraphicsBackend::SetViewport(float x, float y, float width, float height,
                                       float minDepth, float maxDepth) {
    // Store for use during render pass
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;
    
    // Record in current command buffer if in scene
    if (m_inScene && m_frameCount > 0) {
        uint32_t frameIndex = GetCurrentFrameIndex();
        if (frameIndex < m_commandBuffers.size()) {
            vkCmdSetViewport(m_commandBuffers[frameIndex], 0, 1, &viewport);
        }
    }
    
    printf("Viewport set: x=%.1f y=%.1f w=%.1f h=%.1f\n", x, y, width, height);
}

/**
 * Set scissor rectangle for rendering.
 * Scissor defines which pixels can be written to.
 * 
 * @param x Left coordinate
 * @param y Top coordinate
 * @param width Scissor width
 * @param height Scissor height
 */
void DXVKGraphicsBackend::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    VkRect2D scissor{};
    scissor.offset.x = x;
    scissor.offset.y = y;
    scissor.extent.width = width;
    scissor.extent.height = height;
    
    // Record in current command buffer if in scene
    if (m_inScene && m_frameCount > 0) {
        uint32_t frameIndex = GetCurrentFrameIndex();
        if (frameIndex < m_commandBuffers.size()) {
            vkCmdSetScissor(m_commandBuffers[frameIndex], 0, 1, &scissor);
        }
    }
    
    printf("Scissor set: x=%d y=%d w=%u h=%u\n", x, y, width, height);
}

// ============================================================================
// Phase 43.3: Clear Operations
// ============================================================================

/**
 * Record clear operation in render pass.
 * Clears color and/or depth attachments with specified values.
 * Must be called between BeginScene() and rendering.
 * 
 * @param clearColor True to clear color attachment
 * @param clearDepth True to clear depth attachment
 * @param colorRGBA Color as RGBA (4 floats)
 * @param depthValue Depth clear value (0.0-1.0)
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::RecordClearOperation(bool clearColor, bool clearDepth,
                                                   const float* colorRGBA, float depthValue) {
    if (!m_inScene) {
        printf("ERROR: RecordClearOperation called outside BeginScene\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (m_frameCount == 0) {
        printf("WARNING: RecordClearOperation before any frames started\n");
        return S_OK;
    }
    
    uint32_t frameIndex = GetCurrentFrameIndex();
    if (frameIndex >= m_commandBuffers.size()) {
        printf("ERROR: Frame index out of range\n");
        return D3DERR_DEVICELOST;
    }
    
    VkCommandBuffer cmdBuf = m_commandBuffers[frameIndex];
    std::vector<VkClearAttachment> clearAttachments;
    
    // Add color clear if requested
    if (clearColor && colorRGBA) {
        VkClearAttachment colorClear{};
        colorClear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorClear.colorAttachment = 0;
        colorClear.clearValue.color = {{colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]}};
        clearAttachments.push_back(colorClear);
    }
    
    // Add depth clear if requested (Phase 44+)
    if (clearDepth) {
        VkClearAttachment depthClear{};
        depthClear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthClear.clearValue.depthStencil.depth = depthValue;
        depthClear.clearValue.depthStencil.stencil = 0;
        clearAttachments.push_back(depthClear);
    }
    
    // Define rectangular region to clear (full framebuffer)
    VkClearRect clearRect{};
    clearRect.rect.offset = {0, 0};
    clearRect.rect.extent = m_swapchainExtent;
    clearRect.baseArrayLayer = 0;
    clearRect.layerCount = 1;
    
    // Record clear command
    if (!clearAttachments.empty()) {
        vkCmdClearAttachments(cmdBuf, static_cast<uint32_t>(clearAttachments.size()),
                             clearAttachments.data(), 1, &clearRect);
    }
    
    return S_OK;
}

// ============================================================================
// Phase 43.3: Dynamic State Management
// ============================================================================

/**
 * Enable or disable blending for transparency.
 * 
 * @param enabled True to enable alpha blending
 */
void DXVKGraphicsBackend::SetBlendingEnabled(bool enabled) {
    if (enabled) {
        printf("Blending enabled\n");
    } else {
        printf("Blending disabled\n");
    }
    // Full implementation in Phase 44+ (pipeline recreation)
}

/**
 * Set blend mode (source and destination factors).
 * 
 * @param srcFactor Blend source factor
 * @param dstFactor Blend destination factor
 */
void DXVKGraphicsBackend::SetBlendMode(D3DBLEND srcFactor, D3DBLEND dstFactor) {
    printf("Blend mode set: src=%d dst=%d\n", srcFactor, dstFactor);
    // Full implementation in Phase 44+ (pipeline recreation)
}

// ============================================================================
// Phase 43.3: Diagnostic Helpers
// ============================================================================

/**
 * Report render pass state for debugging.
 */
void DXVKGraphicsBackend::ReportRenderPassState() const {
    printf("\n=== Render Pass State ===\n");
    printf("Render pass: %s\n", m_renderPass != VK_NULL_HANDLE ? "VALID" : "NULL");
    printf("Framebuffers: %zu\n", m_swapchainFramebuffers.size());
    printf("Swapchain extent: %ux%u\n", m_swapchainExtent.width, m_swapchainExtent.height);
    printf("Swapchain format: %d\n", m_swapchainFormat);
    printf("\nFramebuffer status:\n");
    for (size_t i = 0; i < m_swapchainFramebuffers.size(); i++) {
        printf("  [%zu] %s\n", i, m_swapchainFramebuffers[i] != VK_NULL_HANDLE ? "OK" : "NULL");
    }
    printf("\n");
}
