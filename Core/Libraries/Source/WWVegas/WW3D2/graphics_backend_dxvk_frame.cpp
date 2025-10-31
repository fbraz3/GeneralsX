/**
 * @file graphics_backend_dxvk_frame.cpp
 * 
 * Vulkan Graphics Backend - Frame Management and Command Recording
 * 
 * Implements the frame lifecycle: BeginScene, EndScene, Present, and Clear operations.
 * Handles swapchain image acquisition, command buffer recording, and frame submission.
 * 
 * Phase: 39.2.3b (Frame Management Implementation)
 * 
 * Created: October 30, 2025
 */

#include "graphics_backend_dxvk.h"
#include <algorithm>

// ============================================================================
// Frame Lifecycle Implementation
// ============================================================================

/**
 * Begin a new frame.
 * 
 * This acquires a swapchain image and prepares for rendering commands.
 * Must be paired with EndScene().
 */
HRESULT DXVKGraphicsBackend::BeginScene() {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_inScene) {
        if (m_debugOutput) {
            printf("[DXVK] WARNING: BeginScene called while already in scene\n");
        }
        return S_FALSE;  // Already in scene
    }
    
    if (m_debugOutput) {
        printf("[DXVK] BeginScene: frame %zu\n", m_currentFrame);
    }
    
    // Wait for the fence to ensure the previous frame is done
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);
    
    // Acquire next image from swapchain
    VkResult result = vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &m_currentSwapchainIndex
    );
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Swapchain is out of date (e.g., window resize)
        if (m_debugOutput) {
            printf("[DXVK] Swapchain out of date, recreating...\n");
        }
        RecreateSwapchain();
        return S_FALSE;  // Frame cancelled, retry next frame
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: vkAcquireNextImageKHR failed: %d\n", result);
        }
        return E_FAIL;
    }
    
    // Reset command buffer for recording
    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);
    
    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    result = vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo);
    if (result != VK_SUCCESS) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: vkBeginCommandBuffer failed: %d\n", result);
        }
        return E_FAIL;
    }
    
    m_inScene = true;
    m_lastError = S_OK;
    
    return S_OK;
}

/**
 * End frame rendering and submit commands.
 * 
 * Records all pending drawing commands and submits them to the GPU.
 * Must be paired with BeginScene().
 */
HRESULT DXVKGraphicsBackend::EndScene() {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (!m_inScene) {
        if (m_debugOutput) {
            printf("[DXVK] WARNING: EndScene called without BeginScene\n");
        }
        return S_FALSE;  // Not in scene
    }
    
    if (m_debugOutput) {
        printf("[DXVK] EndScene: submitting frame\n");
    }
    
    // End command buffer recording
    VkResult result = vkEndCommandBuffer(m_commandBuffers[m_currentFrame]);
    if (result != VK_SUCCESS) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: vkEndCommandBuffer failed: %d\n", result);
        }
        m_inScene = false;
        return E_FAIL;
    }
    
    // Submit commands to GPU
    HRESULT hr = SubmitCommands();
    if (FAILED(hr)) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: SubmitCommands failed\n");
        }
        m_inScene = false;
        return hr;
    }
    
    m_inScene = false;
    m_lastError = S_OK;
    
    return S_OK;
}

/**
 * Present rendered frame to display.
 * 
 * This presents the frame to the window and advances to the next frame.
 */
HRESULT DXVKGraphicsBackend::Present() {
    if (!m_initialized) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Present: frame %zu\n", m_currentFrame);
    }
    
    // Advance to next frame
    m_currentFrame = (m_currentFrame + 1) % m_swapchainImages.size();
    
    m_lastError = S_OK;
    return S_OK;
}

/**
 * Clear render target and depth buffer.
 */
HRESULT DXVKGraphicsBackend::Clear(
    bool clear_color,
    bool clear_z_stencil,
    const void* color_vec3,
    float z,
    DWORD stencil) {
    
    if (!m_initialized || !m_inScene) {
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Clear: color=%p, z=%.2f\n", color_vec3, z);
    }
    
    // Convert color vector to RGBA (color_vec3 is a 3-element float array)
    float r = color_vec3 ? ((const float*)color_vec3)[0] : 0.0f;
    float g = color_vec3 ? ((const float*)color_vec3)[1] : 0.0f;
    float b = color_vec3 ? ((const float*)color_vec3)[2] : 0.0f;
    float a = 1.0f;
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[m_currentSwapchainIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchainExtent;
    
    VkClearValue clearValue = {{r, g, b, a}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;
    
    vkCmdBeginRenderPass(m_commandBuffers[m_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Set viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapchainExtent.width;
    viewport.height = (float)m_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
    
    // Set scissor
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchainExtent;
    
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
    
    // End render pass (the clear is implicit in BeginRenderPass with clearValues)
    vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);
    
    m_lastError = S_OK;
    return S_OK;
}

// ============================================================================
// Command Buffer Submission
// ============================================================================

/**
 * Submit recorded command buffer to graphics queue.
 */
HRESULT DXVKGraphicsBackend::SubmitCommands() {
    if (m_debugOutput) {
        printf("[DXVK] SubmitCommands: frame %zu\n", m_currentFrame);
    }
    
    // Setup wait and signal semaphores
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
    
    // Submit to graphics queue
    VkResult result = vkQueueSubmit(
        m_graphicsQueue,
        1,
        &submitInfo,
        m_inFlightFences[m_currentFrame]
    );
    
    if (result != VK_SUCCESS) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: vkQueueSubmit failed: %d\n", result);
        }
        return E_FAIL;
    }
    
    // Present frame
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentSwapchainIndex;
    
    result = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date or suboptimal
        if (m_debugOutput) {
            printf("[DXVK] Swapchain suboptimal/out of date, will recreate on next frame\n");
        }
        // Don't fail here, next BeginScene will handle recreation
    } else if (result != VK_SUCCESS) {
        if (m_debugOutput) {
            printf("[DXVK] ERROR: vkQueuePresentKHR failed: %d\n", result);
        }
        return E_FAIL;
    }
    
    if (m_debugOutput) {
        printf("[DXVK] Frame submitted and presented successfully\n");
    }
    
    return S_OK;
}

/**
 * Record frame commands.
 * 
 * This is a placeholder for recording actual drawing commands.
 * Will be expanded in Phase 39.2.4 when implementing DrawPrimitive, etc.
 */
void DXVKGraphicsBackend::RecordFrameCommands() {
    // Placeholder for Phase 39.2.4
    // This will record drawing commands between BeginScene and EndScene
    
    if (m_debugOutput) {
        printf("[DXVK] RecordFrameCommands: frame %zu\n", m_currentFrame);
    }
}
