# Phase 48.1: Vulkan Swapchain Implementation

## Overview

Implements the Vulkan presentation layer (swapchain) that enables rendered frames to appear on screen. The swapchain manages a pool of images for double/triple buffering and coordinates frame acquisition and presentation.

**Status**: Ready to implement  
**Depends on**: Phase 47 (Vulkan device and surface ready)  
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

- vkCreateSwapchainKHR succeeds without errors
- Can acquire 60+ frames per second without errors
- Frame indices are valid in range [0, image_count)
- Swapchain can be recreated on window resize
- No Vulkan validation layer errors

## Key Vulkan Concepts

**VkSwapchain**: Collection of images that get presented to the display

**Image Acquisition**: Getting an available image from the swapchain (vkAcquireNextImageKHR)

**Presentation**: Submitting rendered image to display (vkQueuePresentKHR)

**Double Buffering**: Two images (CPU renders to one, GPU displays the other)

**Capabilities**: Surface format, present modes, image count limits

## Implementation Steps

### Step 1: Query Swapchain Capabilities

```cpp
// In graphics_backend_dxvk.cpp

VkSurfaceCapabilitiesKHR capabilities;
vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

uint32_t format_count;
vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
std::vector<VkSurfaceFormatKHR> formats(format_count);
vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

uint32_t present_mode_count;
vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
std::vector<VkPresentModeKHR> present_modes(present_mode_count);
vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
```

### Step 2: Create Swapchain

```cpp
VkSwapchainCreateInfoKHR create_info = {};
create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
create_info.surface = surface;
create_info.minImageCount = std::max(2u, capabilities.minImageCount);
create_info.imageFormat = format.format;
create_info.imageColorSpace = format.colorSpace;
create_info.imageExtent = capabilities.currentExtent;
create_info.imageArrayLayers = 1;
create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
create_info.clipped = VK_TRUE;

VkResult result = vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain);
```

### Step 3: Create Image Views

```cpp
uint32_t image_count;
vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);

std::vector<VkImage> swapchain_images(image_count);
vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.data());

for (uint32_t i = 0; i < image_count; i++) {
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = swapchain_images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    
    VkImageView image_view;
    vkCreateImageView(device, &view_info, nullptr, &image_view);
    swapchain_image_views.push_back(image_view);
}
```

### Step 4: Acquire and Present Frames

```cpp
// In main render loop

// Acquire next available image
VkSemaphore image_available_semaphore = /* create semaphore */;
uint32_t image_index;
vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, 
                      image_available_semaphore, VK_NULL_HANDLE, &image_index);

// Render to swapchain_image_views[image_index]
// (Implementation in Phase 48.2 and 48.3)

// Present rendered image
VkSemaphore render_complete_semaphore = /* create semaphore */;
VkPresentInfoKHR present_info = {};
present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
present_info.waitSemaphoreCount = 1;
present_info.pWaitSemaphores = &render_complete_semaphore;
present_info.swapchainCount = 1;
present_info.pSwapchains = &swapchain;
present_info.pImageIndices = &image_index;

vkQueuePresentKHR(present_queue, &present_info);
```

## Code Location

**Primary file**: `Core/GameEngineDevice/Source/DXVKGraphicsBackend/graphics_backend_dxvk_frame.cpp`

**Header**: `Core/GameEngineDevice/Source/DXVKGraphicsBackend/graphics_backend_dxvk.h`

**Existing methods to extend**:
- `BeginFrame()` - Call vkAcquireNextImageKHR here
- `EndFrame()` - Call vkQueuePresentKHR here

## Related Code

**VkSurfaceKHR creation** (already done in Phase 40):
- `graphics_backend_dxvk.cpp::CreateVulkanSurface()`

**VkDevice creation** (already done in Phase 40):
- `graphics_backend_dxvk_device.cpp::CreateVulkanDevice()`

**Queue families** (already done in Phase 40):
- Methods to get graphics and present queue families

## Testing Approach

### Test 1: Swapchain Creation
```bash
Build and run with logging:
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=0 ./GeneralsXZH 2>&1 | tee /tmp/swapchain_test.log

Expected output:
- "Swapchain created: <address>"
- "Image count: N" (typically 2 or 3)
- No "VK_ERROR" messages
```

### Test 2: Frame Acquisition
```bash
grep "vkAcquireNextImageKHR" /tmp/swapchain_test.log | wc -l
Expected: 600+ lines for 10 second run at 60 FPS
```

### Test 3: Presentation Loop
```bash
grep "vkQueuePresentKHR" /tmp/swapchain_test.log | wc -l
Expected: 600+ lines matching acquisition count
```

## Error Handling

```cpp
// Always check VkResult return values
if (result != VK_SUCCESS) {
    switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
            // Swapchain is out of date (e.g., window resized)
            // Recreate swapchain
            break;
        case VK_SUBOPTIMAL_KHR:
            // Suboptimal presentation (can still render)
            // Consider recreating for better performance
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            // Surface destroyed (critical error)
            break;
        default:
            // Other Vulkan error
            break;
    }
}
```

## Swapchain Recreation

When window is resized, swapchain becomes invalid. Recreation steps:

```cpp
void RecreateSwapchain() {
    vkDeviceWaitIdle(device);  // Wait for rendering to finish
    
    // Destroy old swapchain
    for (auto view : swapchain_image_views) {
        vkDestroyImageView(device, view, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    
    // Create new swapchain with current window extent
    // ... repeat swapchain creation steps above ...
}
```

## References

- Vulkan SDK: `/Users/felipebraz/VulkanSDK/1.4.328.1/macOS/`
- Vulkan Specification: Swapchain chapter
- MoltenVK specifics: Handles Metal layer coordination

## Next Phase

After Task 1 completion, proceed to Phase 48.2: Graphics Pipeline Creation

---

**Status**: Not started  
**Created**: November 2, 2025  

