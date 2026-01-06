# Phase 49.1: Vulkan Swapchain Implementation

## Overview

Implements the Vulkan presentation layer (swapchain) that manages a pool of images for double/triple buffering and coordinates frame acquisition and presentation to the display.

**Status**: Ready to Implement
**Depends on**: Phase 48 (Vulkan device and surface ready)
**Complexity**: Medium
**Duration**: 1-2 days

## Objectives

1. Query swapchain capabilities from physical device
2. Create VkSwapchainKHR for presenting frames  
3. Create image views for swapchain images
4. Implement frame acquisition (vkAcquireNextImageKHR)
5. Implement frame presentation (vkQueuePresentKHR)
6. Handle swapchain recreation on window resize

## Acceptance Criteria

- ✅ vkCreateSwapchainKHR succeeds without errors
- ✅ Can acquire 60+ frames per second without errors
- ✅ Frame indices are valid in range [0, image_count)
- ✅ Swapchain can be recreated on window resize
- ✅ No Vulkan validation layer errors
- ✅ All VkImage and VkImageView objects properly destroyed

## Key Vulkan Concepts

**VkSwapchain**: Collection of images that get presented to the display. Enables double/triple buffering.

**Image Acquisition**: Getting an available image from the swapchain pool (vkAcquireNextImageKHR). Returns image index for rendering.

**Presentation**: Submitting rendered image to display (vkQueuePresentKHR). Returns to swapchain after presentation.

**Double Buffering**: Two images alternate: CPU renders to one while GPU displays the other.

**Capabilities**: Surface format, present modes, image count limits. Queried before swapchain creation.

## Implementation Steps

### Step 1: Query Swapchain Capabilities

File: `graphics_backend_dxvk.cpp`

```cpp
// Query surface capabilities
VkSurfaceCapabilitiesKHR capabilities;
VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    m_physicalDevice, m_surface, &capabilities);
ASSERT(result == VK_SUCCESS, "Failed to query surface capabilities");

printf("[DXVK] Surface capabilities:\n");
printf("  minImageCount: %u\n", capabilities.minImageCount);
printf("  maxImageCount: %u\n", capabilities.maxImageCount);
printf("  currentExtent: %ux%u\n", 
    capabilities.currentExtent.width, 
    capabilities.currentExtent.height);

// Query supported formats
uint32_t format_count = 0;
vkGetPhysicalDeviceSurfaceFormatsKHR(
    m_physicalDevice, m_surface, &format_count, nullptr);

std::vector<VkSurfaceFormatKHR> formats(format_count);
vkGetPhysicalDeviceSurfaceFormatsKHR(
    m_physicalDevice, m_surface, &format_count, formats.data());

printf("[DXVK] Available surface formats: %u\n", format_count);
for (const auto& fmt : formats) {
    printf("  - Format: %d, ColorSpace: %d\n", fmt.format, fmt.colorSpace);
}

// Query supported present modes
uint32_t present_mode_count = 0;
vkGetPhysicalDeviceSurfacePresentModesKHR(
    m_physicalDevice, m_surface, &present_mode_count, nullptr);

std::vector<VkPresentModeKHR> present_modes(present_mode_count);
vkGetPhysicalDeviceSurfacePresentModesKHR(
    m_physicalDevice, m_surface, &present_mode_count, present_modes.data());

printf("[DXVK] Available present modes: %u\n", present_mode_count);
for (const auto& mode : present_modes) {
    printf("  - Mode: %d\n", mode);
}

// Select best format (prefer SRGB)
VkSurfaceFormatKHR selected_format = formats[0];
for (const auto& fmt : formats) {
    if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB) {
        selected_format = fmt;
        break;
    }
}

printf("[DXVK] Selected format: %d\n", selected_format.format);
```

### Step 2: Create Swapchain

File: `graphics_backend_dxvk.cpp`

```cpp
// Choose present mode (FIFO = guaranteed vsync)
VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
for (const auto& mode : present_modes) {
    if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
        selected_present_mode = mode;
        break;
    }
}

printf("[DXVK] Selected present mode: %d\n", selected_present_mode);

// Create swapchain
VkSwapchainCreateInfoKHR swapchain_info = {};
swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
swapchain_info.surface = m_surface;
swapchain_info.minImageCount = std::max(2u, capabilities.minImageCount);
swapchain_info.imageFormat = selected_format.format;
swapchain_info.imageColorSpace = selected_format.colorSpace;
swapchain_info.imageExtent = capabilities.currentExtent;
swapchain_info.imageArrayLayers = 1;
swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
swapchain_info.presentMode = selected_present_mode;
swapchain_info.clipped = VK_TRUE;
swapchain_info.oldSwapchain = VK_NULL_HANDLE;

result = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
ASSERT(result == VK_SUCCESS, "Failed to create swapchain");

printf("[DXVK] Swapchain created: %p\n", (void*)m_swapchain);
printf("  Image count: %u\n", swapchain_info.minImageCount);
printf("  Format: %d, ColorSpace: %d\n", 
    swapchain_info.imageFormat, swapchain_info.imageColorSpace);
printf("  Extent: %ux%u\n", 
    swapchain_info.imageExtent.width, 
    swapchain_info.imageExtent.height);
```

### Step 3: Create Image Views

File: `graphics_backend_dxvk.cpp`

```cpp
// Get swapchain images
uint32_t image_count = 0;
vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);

std::vector<VkImage> swapchain_images(image_count);
vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, swapchain_images.data());

printf("[DXVK] Retrieved %u swapchain images\n", image_count);

// Create image views for each swapchain image
m_swapchain_image_views.resize(image_count);
for (uint32_t i = 0; i < image_count; i++) {
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = swapchain_images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = selected_format.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(m_device, &view_info, nullptr, &m_swapchain_image_views[i]);
    ASSERT(result == VK_SUCCESS, "Failed to create image view");
    
    printf("[DXVK] Image view %u created: %p\n", i, (void*)m_swapchain_image_views[i]);
}

// Store images and format for later use
m_swapchain_images = swapchain_images;
m_swapchain_format = selected_format.format;
m_swapchain_extent = capabilities.currentExtent;
```

### Step 4: Implement Frame Acquisition

File: `graphics_backend_dxvk_frame.cpp`

```cpp
VkResult DXVKGraphicsBackend::AcquireNextFrame(uint32_t& image_index)
{
    // Acquire next available image
    VkResult result = vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        UINT64_MAX,  // Timeout: wait indefinitely
        m_image_available_semaphore,  // Signaled when image available
        VK_NULL_HANDLE,  // No fence
        &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        printf("[DXVK] Swapchain out of date, needs recreation\n");
        return RecreateSwapchain();
    }

    ASSERT(result == VK_SUCCESS, "Failed to acquire next image");
    
    printf("[DXVK] Acquired image %u from swapchain\n", image_index);
    
    return VK_SUCCESS;
}
```

### Step 5: Implement Frame Presentation

File: `graphics_backend_dxvk_frame.cpp`

```cpp
VkResult DXVKGraphicsBackend::PresentFrame(uint32_t image_index)
{
    // Present the rendered frame
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_render_complete_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = &image_index;

    VkResult result = vkQueuePresentKHR(m_presentation_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        printf("[DXVK] Swapchain needs recreation (result: %d)\n", result);
        return RecreateSwapchain();
    }

    ASSERT(result == VK_SUCCESS, "Failed to present frame");
    
    printf("[DXVK] Presented image %u to display\n", image_index);
    
    return VK_SUCCESS;
}
```

### Step 6: Swapchain Recreation (Window Resize)

File: `graphics_backend_dxvk.cpp`

```cpp
VkResult DXVKGraphicsBackend::RecreateSwapchain()
{
    printf("[DXVK] Recreating swapchain...\n");

    // Wait for GPU to finish
    vkDeviceWaitIdle(m_device);

    // Destroy old image views
    for (auto& view : m_swapchain_image_views) {
        vkDestroyImageView(m_device, view, nullptr);
    }
    m_swapchain_image_views.clear();

    // Re-query capabilities (window may have been resized)
    VkSurfaceCapabilitiesKHR capabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_physicalDevice, m_surface, &capabilities);
    ASSERT(result == VK_SUCCESS, "Failed to query surface capabilities");

    printf("[DXVK] New surface extent: %ux%u\n", 
        capabilities.currentExtent.width, 
        capabilities.currentExtent.height);

    // Create new swapchain (same code as Step 2, but pass old swapchain)
    VkSwapchainKHR old_swapchain = m_swapchain;

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.surface = m_surface;
    swapchain_info.minImageCount = std::max(2u, capabilities.minImageCount);
    swapchain_info.imageFormat = m_swapchain_format;
    swapchain_info.imageColorSpace = m_swapchain_format;  // TODO: store color space
    swapchain_info.imageExtent = capabilities.currentExtent;
    swapchain_info.imageArrayLayers = 1;
    swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_info.clipped = VK_TRUE;
    swapchain_info.oldSwapchain = old_swapchain;

    result = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
    ASSERT(result == VK_SUCCESS, "Failed to recreate swapchain");

    // Destroy old swapchain
    vkDestroySwapchainKHR(m_device, old_swapchain, nullptr);

    // Recreate image views (same code as Step 3)
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);

    std::vector<VkImage> swapchain_images(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, swapchain_images.data());

    m_swapchain_image_views.resize(image_count);
    for (uint32_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = swapchain_images[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_swapchain_format;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(m_device, &view_info, nullptr, &m_swapchain_image_views[i]);
        ASSERT(result == VK_SUCCESS, "Failed to create image view");
    }

    printf("[DXVK] Swapchain recreation complete\n");
    
    return VK_SUCCESS;
}
```

## Class Members to Add

File: `graphics_backend_dxvk.h`

```cpp
private:
    // Swapchain resources
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_swapchain_format = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR m_swapchain_colorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D m_swapchain_extent = {0, 0};
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    
    // Frame synchronization
    VkSemaphore m_image_available_semaphore = VK_NULL_HANDLE;
    VkSemaphore m_render_complete_semaphore = VK_NULL_HANDLE;
    
public:
    VkResult AcquireNextFrame(uint32_t& image_index);
    VkResult PresentFrame(uint32_t image_index);
    
private:
    VkResult RecreateSwapchain();
```

## Testing

### Unit Test Template

File: `tests/graphics/test_swapchain.cpp`

```cpp
#include "tests/core/test_macros.h"

TEST(Swapchain, CanCreateSwapchain) {
    // Setup: Create graphics backend (from Phase 48)
    DXVKGraphicsBackend backend;
    ASSERT_TRUE(backend.Initialize());
    
    // Verify: Swapchain created
    ASSERT_NOT_NULL(backend.GetSwapchain());
}

TEST(Swapchain, CanAcquireFrames) {
    DXVKGraphicsBackend backend;
    ASSERT_TRUE(backend.Initialize());
    
    // Acquire 60 consecutive frames
    for (int i = 0; i < 60; i++) {
        uint32_t image_index = 0;
        VkResult result = backend.AcquireNextFrame(image_index);
        ASSERT_EQ(result, VK_SUCCESS);
        ASSERT_GE(image_index, 0);
        ASSERT_LT(image_index, backend.GetSwapchainImageCount());
    }
}

TEST(Swapchain, CanPresentFrames) {
    DXVKGraphicsBackend backend;
    ASSERT_TRUE(backend.Initialize());
    
    // Acquire and present frames
    for (int i = 0; i < 10; i++) {
        uint32_t image_index = 0;
        VkResult result = backend.AcquireNextFrame(image_index);
        ASSERT_EQ(result, VK_SUCCESS);
        
        result = backend.PresentFrame(image_index);
        ASSERT_EQ(result, VK_SUCCESS);
    }
}
```

### Integration Test

```bash
# Build
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Run
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=0 ./GeneralsXZH 2>&1 | tee /tmp/swapchain_test.log

# Verify
grep -c "vkAcquireNextImageKHR" /tmp/swapchain_test.log
# Expected: 600+ (60 FPS * 10 seconds)

grep -i "error\|validation" /tmp/swapchain_test.log
# Expected: No errors
```

## Cleanup on Destruction

File: `graphics_backend_dxvk.cpp`

```cpp
DXVKGraphicsBackend::~DXVKGraphicsBackend()
{
    // Wait for GPU
    vkDeviceWaitIdle(m_device);

    // Destroy image views
    for (auto& view : m_swapchain_image_views) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, view, nullptr);
        }
    }

    // Destroy swapchain
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }

    // Destroy synchronization primitives
    if (m_image_available_semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_image_available_semaphore, nullptr);
    }
    if (m_render_complete_semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_render_complete_semaphore, nullptr);
    }
}
```

## Success Checklist

- [ ] Swapchain created without errors
- [ ] Image views created for all swapchain images
- [ ] Can acquire 60+ frames per second
- [ ] Frame indices always in valid range [0, image_count)
- [ ] Swapchain recreates on window resize
- [ ] No Vulkan validation layer errors
- [ ] All objects properly destroyed in cleanup
- [ ] Unit tests pass (60+ frame acquisition)
- [ ] Integration test runs without crashes

## Next Steps

- ✅ Understand swapchain architecture
- ✅ Implement swapchain creation and image views
- ✅ Implement frame acquisition and presentation
- ⏳ Move to Task 2: Graphics Pipeline Creation

---

**Created**: November 6, 2025
**Status**: Ready to Implement
