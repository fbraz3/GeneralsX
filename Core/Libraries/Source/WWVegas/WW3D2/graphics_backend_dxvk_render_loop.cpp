/**
 * @file graphics_backend_dxvk_render_loop.cpp
 * 
 * Phase 43: Render Loop & Frame Presentation
 * 
 * Implements the complete render loop lifecycle:
 * - BeginScene() - Acquire swapchain image, reset command buffer
 * - EndScene() - Submit commands to GPU
 * - Present() - Present frame to screen
 * 
 * Architecture:
 * - Single in-flight frame (no triple-buffering yet)
 * - Fence-based synchronization for CPU-GPU coordination
 * - Semaphore-based synchronization for GPU pipeline
 * - Automatic frame pacing to 60 FPS
 * 
 * Phase Breakdown:
 * - 43.1: Basic render loop (this file)
 * - 43.2: Frame synchronization & timing
 * - 43.3: Command recording & render pass
 * - 43.4: Error handling & swapchain recreation
 * - 43.5: Performance monitoring & profiling
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>
#include <chrono>

// D3D error codes (from d3d8.h compatibility layer)
#define D3DERR_INVALIDCALL 0x8876086CL
#define D3DERR_DEVICELOST 0x88760868L

// ============================================================================
// Phase 43.1: BeginScene - Start Frame Rendering
// ============================================================================

/**
 * Begin a new scene (frame) rendering cycle.
 * 
 * Sequence:
 * 1. Wait for previous frame's GPU work to complete (if using multiple frames)
 * 2. Acquire next swapchain image
 * 3. Reset command buffer for new recording
 * 4. Begin render pass with clear color
 * 
 * @return S_OK on success, D3DERR_DEVICELOST on device loss
 */
HRESULT DXVKGraphicsBackend::BeginScene() {
    if (m_inScene) {
        printf("ERROR: BeginScene called while already in scene\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (!m_device || m_swapchain == VK_NULL_HANDLE || !m_graphicsQueue) {
        printf("ERROR: Graphics device not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    // Using frame index 0 for single-frame (Phase 43.1 simplified)
    uint32_t frameIndex = 0;
    
    printf("[Frame %u] BeginScene\n", m_frameNumber);
    
    // Wait for previous frame GPU work to complete
    if (frameIndex < m_inFlightFences.size() && m_inFlightFences[frameIndex] != VK_NULL_HANDLE) {
        VkResult vkr = vkWaitForFences(m_device, 1, &m_inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
        if (vkr != VK_SUCCESS) {
            printf("ERROR: vkWaitForFences failed: %d\n", vkr);
            return D3DERR_DEVICELOST;
        }
        
        // Reset fence for next frame
        vkr = vkResetFences(m_device, 1, &m_inFlightFences[frameIndex]);
        if (vkr != VK_SUCCESS) {
            printf("ERROR: vkResetFences failed: %d\n", vkr);
            return D3DERR_DEVICELOST;
        }
    }
    
    // Acquire next swapchain image
    VkSemaphore imageAvailableSem = (frameIndex < m_imageAvailableSemaphores.size()) 
        ? m_imageAvailableSemaphores[frameIndex] 
        : VK_NULL_HANDLE;
    
    VkResult vkr = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                         imageAvailableSem, VK_NULL_HANDLE, 
                                         &m_currentImageIndex);
    
    if (vkr == VK_ERROR_OUT_OF_DATE_KHR) {
        printf("WARNING: Swapchain out-of-date, needs recreation\n");
        return D3DERR_DEVICELOST;  // Signal need for swapchain rebuild
    }
    
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkAcquireNextImageKHR failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    // Reset command buffer for new recording
    if (frameIndex >= m_commandBuffers.size()) {
        printf("ERROR: Frame index out of command buffer range\n");
        return D3DERR_DEVICELOST;
    }
    
    VkCommandBuffer cmdBuf = m_commandBuffers[frameIndex];
    VkCommandBufferResetFlags resetFlags = 0;
    vkr = vkResetCommandBuffer(cmdBuf, resetFlags);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkResetCommandBuffer failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    
    vkr = vkBeginCommandBuffer(cmdBuf, &beginInfo);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkBeginCommandBuffer failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    // Record image layout transition to color attachment optimal
    if (m_currentImageIndex < m_swapchainImages.size()) {
        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = m_swapchainImages[m_currentImageIndex];
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = 1;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = 1;
        imageBarrier.srcAccessMask = 0;
        imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        vkCmdPipelineBarrier(cmdBuf,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    }
    
    // Begin render pass with clear color (default: black)
    VkClearValue clearValue{};
    clearValue.color = {{m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]}};
    
    if (m_renderPass != VK_NULL_HANDLE && m_currentImageIndex < m_swapchainImageViews.size()) {
        // Create a simple framebuffer-like structure from image view
        // Note: Full Phase 43.3 will implement proper render passes
        printf("  Render pass not fully initialized in Phase 43.1 stub\n");
    }
    
    // Set viewport and scissor (will be updated by SetViewport if needed)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchainExtent.width);
    viewport.height = static_cast<float>(m_swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchainExtent;
    
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
    
    m_inScene = true;
    m_frameNumber++;
    
    printf("  Image index: %u, Scene started\n", m_currentImageIndex);
    return S_OK;
}

// ============================================================================
// Phase 43.1: EndScene - Complete Frame Command Recording
// ============================================================================

/**
 * End the current scene (frame) rendering and submit to GPU.
 * 
 * Sequence:
 * 1. End render pass
 * 2. End command buffer recording
 * 3. Submit commands to GPU queue
 * 4. Signal frame completion
 * 
 * @return S_OK on success, D3DERR_DEVICELOST on device loss
 */
HRESULT DXVKGraphicsBackend::EndScene() {
    if (!m_inScene) {
        printf("ERROR: EndScene called without BeginScene\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (!m_device || !m_graphicsQueue) {
        printf("ERROR: Graphics device not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("[Frame %u] EndScene\n", m_frameNumber);
    
    uint32_t frameIndex = 0;
    if (frameIndex >= m_commandBuffers.size()) {
        printf("ERROR: Frame index out of command buffer range\n");
        m_inScene = false;
        return D3DERR_DEVICELOST;
    }
    
    VkCommandBuffer cmdBuf = m_commandBuffers[frameIndex];
    
    // End render pass (stub for Phase 43.1, will be full in 43.3)
    // vkCmdEndRenderPass(cmdBuf);  // Not started in 43.1 stub
    
    // Transition image back to present source layout
    if (m_currentImageIndex < m_swapchainImages.size()) {
        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = m_swapchainImages[m_currentImageIndex];
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = 1;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = 1;
        imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imageBarrier.dstAccessMask = 0;
        
        vkCmdPipelineBarrier(cmdBuf,
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    }
    
    // End command buffer recording
    VkResult vkr = vkEndCommandBuffer(cmdBuf);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkEndCommandBuffer failed: %d\n", vkr);
        m_inScene = false;
        return D3DERR_DEVICELOST;
    }
    
    // Submit commands to GPU queue
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    VkSemaphore imageAvailableSem = (frameIndex < m_imageAvailableSemaphores.size()) 
        ? m_imageAvailableSemaphores[frameIndex] 
        : VK_NULL_HANDLE;
    
    VkSemaphore renderFinishedSem = (frameIndex < m_renderFinishedSemaphores.size()) 
        ? m_renderFinishedSemaphores[frameIndex] 
        : VK_NULL_HANDLE;
    
    VkFence inFlightFence = (frameIndex < m_inFlightFences.size()) 
        ? m_inFlightFences[frameIndex] 
        : VK_NULL_HANDLE;
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = (imageAvailableSem != VK_NULL_HANDLE) ? 1 : 0;
    submitInfo.pWaitSemaphores = &imageAvailableSem;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.signalSemaphoreCount = (renderFinishedSem != VK_NULL_HANDLE) ? 1 : 0;
    submitInfo.pSignalSemaphores = &renderFinishedSem;
    
    vkr = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, inFlightFence);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkQueueSubmit failed: %d\n", vkr);
        m_inScene = false;
        return D3DERR_DEVICELOST;
    }
    
    m_inScene = false;
    printf("  Commands submitted to GPU\n");
    return S_OK;
}

// ============================================================================
// Phase 43.1: Present - Display Frame on Screen
// ============================================================================

/**
 * Present the rendered frame to the display.
 * 
 * Sequence:
 * 1. Submit present request to queue
 * 2. Wait for display sync
 * 3. Handle presentation results (out-of-date, etc.)
 * 
 * @return S_OK on success, D3DERR_DEVICELOST on device loss
 */
HRESULT DXVKGraphicsBackend::Present() {
    if (m_inScene) {
        printf("ERROR: Present called while still in scene\n");
        return D3DERR_INVALIDCALL;
    }
    
    if (!m_graphicsQueue || m_swapchain == VK_NULL_HANDLE) {
        printf("ERROR: Graphics queue or swapchain not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("[Frame %u] Present\n", m_frameNumber);
    
    uint32_t frameIndex = 0;
    VkSemaphore renderFinishedSem = (frameIndex < m_renderFinishedSemaphores.size()) 
        ? m_renderFinishedSemaphores[frameIndex] 
        : VK_NULL_HANDLE;
    
    // Submit present request
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = (renderFinishedSem != VK_NULL_HANDLE) ? 1 : 0;
    presentInfo.pWaitSemaphores = &renderFinishedSem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImageIndex;
    presentInfo.pResults = nullptr;
    
    VkResult vkr = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
    
    if (vkr == VK_ERROR_OUT_OF_DATE_KHR || vkr == VK_SUBOPTIMAL_KHR) {
        printf("WARNING: Swapchain needs recreation (out-of-date or suboptimal)\n");
        // Will be handled in next BeginScene or via RecreateSwapchain()
        return S_OK;  // Not a fatal error, just needs recreation
    }
    
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkQueuePresentKHR failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    printf("  Frame presented to display\n");
    return S_OK;
}

