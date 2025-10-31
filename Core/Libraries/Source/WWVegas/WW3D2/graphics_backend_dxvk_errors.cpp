/**
 * @file graphics_backend_dxvk_errors.cpp
 * 
 * Phase 43.4: Error Handling & Recovery
 * 
 * Implements robust error recovery mechanisms:
 * - Swapchain out-of-date detection and recreation
 * - Window resize handling
 * - Device lost recovery
 * - Comprehensive error logging and diagnostics
 * 
 * Architecture:
 * - Automatic swapchain rebuild on VK_ERROR_OUT_OF_DATE_KHR
 * - Window extent change detection
 * - Device-lost recovery with resource reallocation
 * - Detailed error context capture for debugging
 * 
 * Phase Breakdown:
 * - 43.1: Basic render loop (BeginScene/EndScene/Present)
 * - 43.2: Frame synchronization & timing
 * - 43.3: Command recording & render pass
 * - 43.4: Error handling & recovery (this file)
 * - 43.5: Performance monitoring & profiling
 * 
 * Created: October 31, 2025
 * Author: GeneralsX Development Team
 */

#include "graphics_backend_dxvk.h"
#include <cstring>

// D3D error codes
#define D3DERR_INVALIDCALL 0x8876086CL
#define D3DERR_DEVICELOST 0x88760868L

// ============================================================================
// Phase 43.4: Swapchain Recreation
// ============================================================================

/**
 * Recreate swapchain due to window resize or out-of-date error.
 * Called when:
 * - VK_ERROR_OUT_OF_DATE_KHR returned from vkQueuePresentKHR
 * - VK_SUBOPTIMAL_KHR returned from vkAcquireNextImageKHR
 * - Window is resized
 * 
 * Sequence:
 * 1. Destroy old framebuffers
 * 2. Destroy old swapchain
 * 3. Query new window surface capabilities
 * 4. Create new swapchain with updated extent
 * 5. Create new image views and framebuffers
 * 
 * @return S_OK on success
 */
HRESULT DXVKGraphicsBackend::RecreateSwapchain() {
    printf("Recreating swapchain...\n");
    
    if (!m_device || !m_physicalDevice || !m_surface) {
        printf("ERROR: Device, physical device, or surface not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    // Wait for device idle before destroying resources
    if (vkDeviceWaitIdle(m_device) != VK_SUCCESS) {
        printf("ERROR: vkDeviceWaitIdle failed\n");
        return D3DERR_DEVICELOST;
    }
    
    printf("Device idle, proceeding with swapchain recreation\n");
    
    // Destroy old framebuffers
    DestroyFramebuffers();
    
    // Get updated surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult vkr = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    printf("New surface capabilities: %ux%u\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
    
    // Handle minimized window (0x0 extent)
    if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) {
        printf("Window minimized, deferring recreation\n");
        return S_OK;  // Defer until window is restored
    }
    
    // Update extent if different
    VkExtent2D oldExtent = m_swapchainExtent;
    m_swapchainExtent = capabilities.currentExtent;
    
    printf("Swapchain extent: %ux%u → %ux%u\n", oldExtent.width, oldExtent.height,
           m_swapchainExtent.width, m_swapchainExtent.height);
    
    // Destroy old image views (old swapchain still needed temporarily)
    for (VkImageView imageView : m_swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, imageView, nullptr);
        }
    }
    m_swapchainImageViews.clear();
    m_swapchainImages.clear();
    
    // Destroy old swapchain
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    
    // Create new swapchain with updated extent
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = capabilities.minImageCount + 1;  // One extra for comfort
    swapchainInfo.imageFormat = m_swapchainFormat;
    swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainInfo.imageExtent = m_swapchainExtent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;  // vsync
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;  // Old swapchain already destroyed
    
    vkr = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkCreateSwapchainKHR failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    printf("New swapchain created\n");
    
    // Get new swapchain images
    uint32_t imageCount;
    vkr = vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkGetSwapchainImagesKHR (count) failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    m_swapchainImages.resize(imageCount);
    vkr = vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());
    if (vkr != VK_SUCCESS) {
        printf("ERROR: vkGetSwapchainImagesKHR (get) failed: %d\n", vkr);
        return D3DERR_DEVICELOST;
    }
    
    printf("Swapchain has %u images\n", imageCount);
    
    // Create new image views
    m_swapchainImageViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapchainFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        
        vkr = vkCreateImageView(m_device, &viewInfo, nullptr, &m_swapchainImageViews[i]);
        if (vkr != VK_SUCCESS) {
            printf("ERROR: vkCreateImageView failed for image %u: %d\n", i, vkr);
            return D3DERR_DEVICELOST;
        }
    }
    
    printf("Image views created\n");
    
    // Create new framebuffers
    HRESULT hr = CreateFramebuffers();
    if (FAILED(hr)) {
        printf("ERROR: CreateFramebuffers failed\n");
        return hr;
    }
    
    printf("Swapchain recreation complete\n");
    return S_OK;
}

// ============================================================================
// Phase 43.4: Device Recovery
// ============================================================================

/**
 * Recover from device lost condition.
 * Waits for device to be available and reallocates critical resources.
 * 
 * @return S_OK when device recovered
 */
HRESULT DXVKGraphicsBackend::RecoverFromDeviceLost() {
    printf("Recovering from device lost...\n");
    
    if (!m_device) {
        printf("ERROR: Device is null\n");
        return D3DERR_DEVICELOST;
    }
    
    // Wait for device to become available again
    VkResult vkr = vkDeviceWaitIdle(m_device);
    if (vkr != VK_SUCCESS) {
        printf("WARNING: vkDeviceWaitIdle failed: %d\n", vkr);
        // Continue anyway - device might still be usable
    }
    
    // Attempt swapchain recreation
    HRESULT hr = RecreateSwapchain();
    if (FAILED(hr)) {
        printf("ERROR: Could not recreate swapchain\n");
        return hr;
    }
    
    printf("Device recovery complete\n");
    return S_OK;
}

// ============================================================================
// Phase 43.4: Error Context & Logging
// ============================================================================

/**
 * Log comprehensive error information.
 * Called on render errors to capture context for debugging.
 * 
 * @param errorCode D3D or Vulkan error code
 * @param context Description of where error occurred
 */
void DXVKGraphicsBackend::LogError(HRESULT errorCode, const char* context) {
    printf("\n=== RENDER ERROR ===\n");
    printf("Context: %s\n", context);
    printf("Error code: 0x%08X\n", errorCode);
    printf("Frame: %u\n", m_frameCount);
    printf("In scene: %s\n", m_inScene ? "YES" : "NO");
    
    // Decode known error codes
    switch (errorCode) {
        case D3DERR_DEVICELOST:
            printf("Description: Device lost\n");
            break;
        case D3DERR_INVALIDCALL:
            printf("Description: Invalid call\n");
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            printf("Description: Out of video memory\n");
            break;
        case S_OK:
            printf("Description: No error (S_OK)\n");
            break;
        default:
            printf("Description: Unknown error\n");
    }
    
    // Report current device state
    ReportSynchronizationState();
    ReportRenderPassState();
    
    printf("=== END ERROR REPORT ===\n\n");
}

/**
 * Check for and handle presentation errors.
 * Called after vkQueuePresentKHR to handle common failures.
 * 
 * @param presentResult Result from vkQueuePresentKHR
 * @return S_OK if handled, error code if unrecoverable
 */
HRESULT DXVKGraphicsBackend::HandlePresentError(VkResult presentResult) {
    switch (presentResult) {
        case VK_SUCCESS:
            // No error
            return S_OK;
            
        case VK_ERROR_OUT_OF_DATE_KHR:
            printf("Present error: Swapchain out-of-date\n");
            return RecreateSwapchain();
            
        case VK_SUBOPTIMAL_KHR:
            printf("Present warning: Swapchain suboptimal (but still usable)\n");
            // Optional: RecreateSwapchain() for better performance
            // For now, continue rendering
            return S_OK;
            
        case VK_ERROR_SURFACE_LOST_KHR:
            printf("Present error: Surface lost\n");
            LogError(D3DERR_DEVICELOST, "Surface lost in Present");
            return D3DERR_DEVICELOST;
            
        default:
            printf("Present error: Unknown Vulkan error %d\n", presentResult);
            LogError(D3DERR_DEVICELOST, "Unknown error in Present");
            return D3DERR_DEVICELOST;
    }
}

/**
 * Check for and handle acquisition errors.
 * Called after vkAcquireNextImageKHR to handle common failures.
 * 
 * @param acquireResult Result from vkAcquireNextImageKHR
 * @return S_OK if handled, error code if unrecoverable
 */
HRESULT DXVKGraphicsBackend::HandleAcquisitionError(VkResult acquireResult) {
    switch (acquireResult) {
        case VK_SUCCESS:
            // No error
            return S_OK;
            
        case VK_ERROR_OUT_OF_DATE_KHR:
            printf("Acquisition error: Swapchain out-of-date\n");
            return RecreateSwapchain();
            
        case VK_SUBOPTIMAL_KHR:
            printf("Acquisition warning: Image acquired but suboptimal\n");
            // Image still usable, but quality degraded
            return S_OK;
            
        case VK_TIMEOUT:
            printf("Acquisition error: Timeout waiting for image\n");
            LogError(D3DERR_DEVICELOST, "Timeout in acquire");
            return D3DERR_DEVICELOST;
            
        case VK_NOT_READY:
            printf("Acquisition error: No image available\n");
            LogError(D3DERR_DEVICELOST, "No image in acquire");
            return D3DERR_DEVICELOST;
            
        default:
            printf("Acquisition error: Unknown Vulkan error %d\n", acquireResult);
            LogError(D3DERR_DEVICELOST, "Unknown error in acquire");
            return D3DERR_DEVICELOST;
    }
}

// ============================================================================
// Phase 43.4: State Validation
// ============================================================================

/**
 * Validate render pipeline state before rendering.
 * Checks that all required resources are initialized.
 * 
 * @return S_OK if valid, error if something is missing
 */
HRESULT DXVKGraphicsBackend::ValidateRenderState() const {
    if (!m_device) {
        printf("ERROR: Device not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    if (m_swapchain == VK_NULL_HANDLE) {
        printf("ERROR: Swapchain not initialized\n");
        return D3DERR_DEVICELOST;
    }
    
    if (m_renderPass == VK_NULL_HANDLE) {
        printf("ERROR: Render pass not created\n");
        return D3DERR_DEVICELOST;
    }
    
    if (m_swapchainFramebuffers.empty()) {
        printf("ERROR: No framebuffers created\n");
        return D3DERR_DEVICELOST;
    }
    
    if (m_commandBuffers.empty()) {
        printf("ERROR: No command buffers allocated\n");
        return D3DERR_DEVICELOST;
    }
    
    return S_OK;
}

/**
 * Perform comprehensive device diagnostics.
 * Reports all major resource status for debugging.
 */
void DXVKGraphicsBackend::PerformDiagnostics() const {
    printf("\n=== Device Diagnostics ===\n");
    printf("Device initialized: %s\n", m_device ? "YES" : "NO");
    printf("Swapchain: %s\n", m_swapchain != VK_NULL_HANDLE ? "OK" : "NULL");
    printf("Render pass: %s\n", m_renderPass != VK_NULL_HANDLE ? "OK" : "NULL");
    printf("Framebuffers: %zu\n", m_swapchainFramebuffers.size());
    printf("Swapchain extent: %ux%u\n", m_swapchainExtent.width, m_swapchainExtent.height);
    printf("Command buffers: %zu\n", m_commandBuffers.size());
    printf("In-flight fences: %zu\n", m_inFlightFences.size());
    printf("Synchronization state:\n");
    ReportSynchronizationState();
    printf("=== End Diagnostics ===\n\n");
}
