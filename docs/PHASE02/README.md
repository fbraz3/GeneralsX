# PHASE 02: Logical Device, Queues & Command Pools

**Date**: November 11, 2025  
**Phase Number**: 02  
**Objective**: Create logical Vulkan device, queues, and command pool for GPU operations  
**Dependencies**: PHASE01 (VkPhysicalDevice selected)  
**Estimated Scope**: MEDIUM  
**Status**: not-started  

---

## Overview

This phase builds on PHASE01 by creating:
1. Logical device (VkDevice) from selected physical device
2. Graphics queue (VkQueue) for rendering commands
3. Transfer queue (VkQueue) for memory operations
4. Command pool (VkCommandPool) to allocate command buffers
5. Primary command buffer (VkCommandBuffer) for frame recording

---

## Key Deliverables

- ✅ VkDevice created and initialized
- ✅ Graphics queue extracted successfully
- ✅ Transfer queue extracted (if available)
- ✅ VkCommandPool created
- ✅ Primary command buffer allocated
- ✅ Unit test validates queue operations

---

## Technical Approach

### Logical Device Creation

```cpp
// Queue creation requests
std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

VkDeviceQueueCreateInfo graphicsQueueInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = graphicsQueueFamily,
    .queueCount = 1,
    .pQueuePriorities = (float[]){1.0f}
};
queueCreateInfos.push_back(graphicsQueueInfo);

// Device features
VkPhysicalDeviceFeatures deviceFeatures = {
    .geometryShader = VK_TRUE,
    .textureCompressionBC = VK_TRUE,
    .multiViewport = VK_TRUE,
    .samplerAnisotropy = VK_TRUE
};

// Device creation
VkDeviceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = (uint32_t)queueCreateInfos.size(),
    .pQueueCreateInfos = queueCreateInfos.data(),
    .pEnabledFeatures = &deviceFeatures,
    .enabledExtensionCount = extensionCount,
    .ppEnabledExtensionNames = extensions
};

VkDevice device;
vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
```

### Queue Retrieval

```cpp
// Get graphics queue
VkQueue graphicsQueue;
vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);

// Get transfer queue (may be same as graphics queue)
VkQueue transferQueue;
vkGetDeviceQueue(device, transferQueueFamily, 0, &transferQueue);
```

### Command Pool Creation

```cpp
VkCommandPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = graphicsQueueFamily
};

VkCommandPool commandPool;
vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
```

### Command Buffer Allocation

```cpp
VkCommandBufferAllocateInfo allocInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
};

VkCommandBuffer commandBuffer;
vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
```

---

## Acceptance Criteria (Checklist)

- [ ] VkDevice created successfully
- [ ] Graphics queue obtained and verified
- [ ] Transfer queue obtained (may differ from graphics queue)
- [ ] VkCommandPool created with reset capability
- [ ] VkCommandBuffer allocated and ready
- [ ] All Vulkan extensions required for rendering enabled
- [ ] No memory leaks in allocation
- [ ] Can clean up with vkDestroyCommandPool, vkDestroyDevice

---

## Implementation Files

**Files to Create**:
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_device.h` - Device wrapper
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_device.cpp` - Device implementation
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_queue.h` - Queue wrapper
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_command_pool.h` - Command pool wrapper
- `tests/vulkan_device_test.cpp` - Unit test

---

## Testing Strategy

### Unit Test: Device & Queue Creation

```cpp
void test_device_creation() {
    VulkanInstance instance;
    instance.Initialize();
    
    VulkanDevice device(instance);
    ASSERT_TRUE(device.Initialize());
    ASSERT_TRUE(device.GetLogicalDevice() != VK_NULL_HANDLE);
    
    VulkanQueue graphicsQueue = device.GetGraphicsQueue();
    ASSERT_TRUE(graphicsQueue.IsValid());
    
    device.Shutdown();
}
```

### Debug Output Example

```
VULKAN: Creating logical device...
VULKAN: Device created successfully
VULKAN: Graphics queue obtained (family=0, index=0)
VULKAN: Transfer queue obtained (family=2, index=0)
VULKAN: Creating command pool...
VULKAN: Command pool created successfully
VULKAN: Allocating command buffer...
VULKAN: Primary command buffer allocated
VULKAN: Phase 02 COMPLETE
```

---

## Critical Notes

- ✅ **Queue families**: Not all queue families support graphics; verify with vkGetPhysicalDeviceQueueFamilyProperties
- ✅ **VkDeviceFeatures**: Enable features needed for game (geometry shaders, texture compression, anisotropy)
- ✅ **Memory leaks**: Always clean up in reverse order (command buffers → pool → device)

---

## Next Phase

PHASE03: Shader compilation system (GLSL → SPIR-V)

---

**Last Updated**: November 11, 2025  
**Status**: Ready for implementation
