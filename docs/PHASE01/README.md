# PHASE 01: Vulkan Instance & Physical Device Selection

**Date**: November 11, 2025  
**Phase Number**: 01  
**Objective**: Initialize Vulkan and select appropriate graphics hardware  
**Dependencies**: Build system (PHASE00.5), Vulkan SDK installed  
**Estimated Scope**: MEDIUM  
**Status**: not-started  

---

## Overview

This phase establishes the Vulkan foundation by:
1. Creating a Vulkan instance (VkInstance)
2. Enumerating physical devices (VkPhysicalDevice)
3. Selecting appropriate device (Apple Silicon, dedicated GPU, or fallback)
4. Querying device capabilities and memory

This is the first step in the Vulkan object hierarchy: `VkInstance` → `VkPhysicalDevice` → `VkDevice` (Phase 02).

---

## Key Deliverables

- ✅ `VkInstance` created successfully
- ✅ Physical devices enumerated and logged
- ✅ Appropriate device selected (strategy defined below)
- ✅ Device capabilities validated (memory, queue families, features)
- ✅ Unit test confirms instance creation and device selection

---

## Technical Approach

### Vulkan Instance Creation

```cpp
// Core code structure (pseudo-C++)
VkApplicationInfo appInfo = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "GeneralsX",
    .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
    .pEngineName = "GeneralsX Engine",
    .engineVersion = VK_MAKE_VERSION(0, 1, 0),
    .apiVersion = VK_API_VERSION_1_3
};

VkInstanceCreateInfo createInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &appInfo,
    .enabledExtensionCount = extensionCount,
    .ppEnabledExtensionNames = extensions,
    .enabledLayerCount = layerCount,
    .ppEnabledLayerNames = layers
};

VkInstance instance;
vkCreateInstance(&createInfo, nullptr, &instance);
```

### Platform-Specific Extensions

**macOS (via MoltenVK)**:
- `VK_KHR_SURFACE`
- `VK_MVK_MACOS_SURFACE`
- `VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2`

**Linux**:
- `VK_KHR_SURFACE`
- `VK_KHR_XLIB_SURFACE` or `VK_KHR_WAYLAND_SURFACE`

**Windows**:
- `VK_KHR_SURFACE`
- `VK_KHR_WIN32_SURFACE`

### Physical Device Selection Strategy

**Priority Order**:
1. **Discrete GPU** (dedicated graphics card) - if available
2. **Integrated GPU** (Apple Silicon / Intel iGPU) - always available
3. **Software Rasterizer** (VKSC) - fallback only

**Selection Criteria**:
```cpp
// Rank devices by type and memory
int32_t selectDevice(const std::vector<VkPhysicalDevice>& devices) {
    int bestScore = 0;
    int bestIndex = 0;
    
    for (int i = 0; i < devices.size(); i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[i], &props);
        
        int score = 0;
        // Discrete GPUs score 1000 points
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        // Integrated GPUs score 500 points
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 500;
        
        // Add memory bonus
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(devices[i], &memProps);
        VkDeviceSize totalMemory = getTotalDeviceMemory(memProps);
        score += (totalMemory / (1024 * 1024)); // 1 point per MB
        
        if (score > bestScore) {
            bestScore = score;
            bestIndex = i;
        }
    }
    
    return bestIndex;
}
```

---

## Acceptance Criteria (Checklist)

- [ ] VkInstance created without errors
- [ ] At least one physical device enumerated
- [ ] Device properties logged (name, type, memory)
- [ ] Selected device has adequate queue families
- [ ] Device supports required extensions:
  - `VK_KHR_SWAPCHAIN` (for presentation)
  - `VK_KHR_MAINTENANCE1` or equivalent (memory management)
- [ ] Validation layers enabled (if VULKAN_DEBUG=1)
- [ ] No segmentation faults during initialization
- [ ] Can clean up instance with vkDestroyInstance

---

## Implementation Files

**Files to Create**:
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_instance.h` - Instance creation/management
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_instance.cpp` - Implementation
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_device_selector.h` - Device selection logic
- `Core/Libraries/Source/WWVegas/WW3D2/vulkan_device_selector.cpp` - Selector implementation
- `tests/vulkan_instance_test.cpp` - Unit test

**Files to Modify**:
- `CMakeLists.txt` - Add Vulkan SDK linkage
- `cmake/vulkan.cmake` - Create Vulkan configuration module

---

## Testing Strategy

### Unit Test: Instance Creation

```cpp
// tests/vulkan_instance_test.cpp
void test_vulkan_instance_creation() {
    VulkanInstance instance;
    
    ASSERT_TRUE(instance.Initialize());
    ASSERT_TRUE(instance.GetInstance() != VK_NULL_HANDLE);
    ASSERT_GT(instance.GetDeviceCount(), 0);
    
    instance.PrintDeviceInfo();
    instance.Shutdown();
}
```

### Debug Output Example

```
VULKAN: Initializing Vulkan instance...
VULKAN: Instance created successfully
VULKAN: Enumerating physical devices...
VULKAN: Found 2 physical devices:
  Device 0: Apple M1 (Integrated GPU) - 10GB memory
  Device 1: Apple M1 Cores (Secondary) - shared memory
VULKAN: Selected device 0 (Apple M1)
VULKAN: Queue families available:
  Graphics: queue_family=0 (count=1)
  Compute: queue_family=1 (count=2)
  Transfer: queue_family=1 (available)
VULKAN: Phase 01 COMPLETE
```

---

## Critical Notes from Lessons Learned

- ✅ **No global state**: Don't store VkInstance in globals (RAII/smart pointers only)
- ✅ **Specific exceptions**: If initialization fails, throw meaningful error (not silent)
- ✅ **Platform detection**: Use macOS/Linux/Windows ifdefs for extension loading
- ✅ **Debug output**: Log all device information for troubleshooting

---

## Reference Documentation

- **Vulkan Spec**: `docs/Vulkan/getting_started.html` (section: Creating an Instance)
- **Device Selection**: Vulkan Tutorial - Physical Devices
- **MoltenVK**: https://github.com/KhronosGroup/MoltenVK (macOS-specific details)

---

## Next Phase

PHASE02: VkDevice, VkQueue, VkCommandPool, VkCommandBuffer creation  
(Can start once PHASE01 successfully selects a device)

---

**Last Updated**: November 11, 2025  
**Language**: English  
**Status**: Ready for implementation
