#!/bin/bash

# Test Vulkan instance creation with direct strings (no macro constants)

cat > /tmp/test_vulkan_simple.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

int main() {
    printf("Testing Vulkan instance creation with direct extension strings...\n\n");
    
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "GeneralsX",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "GeneralsX",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };
    
    printf("Test 1: VK_KHR_surface only\n");
    const char *ext1[] = {
        "VK_KHR_surface"
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = ext1
    };
    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    printf("  Result: %d ", result);
    if (result == VK_SUCCESS) {
        printf("✓ SUCCESS\n");
        vkDestroyInstance(instance, NULL);
    } else {
        printf("✗ FAILED\n");
    }
    
    printf("\nTest 2: VK_KHR_surface + VK_EXT_metal_surface\n");
    const char *ext2[] = {
        "VK_KHR_surface",
        "VK_EXT_metal_surface"
    };
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = ext2;
    instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&createInfo, NULL, &instance);
    printf("  Result: %d ", result);
    if (result == VK_SUCCESS) {
        printf("✓ SUCCESS\n");
        vkDestroyInstance(instance, NULL);
    } else {
        printf("✗ FAILED\n");
    }
    
    printf("\nTest 3: VK_KHR_surface + VK_EXT_metal_surface + VK_KHR_portability_enumeration\n");
    const char *ext3[] = {
        "VK_KHR_surface",
        "VK_EXT_metal_surface",
        "VK_KHR_portability_enumeration"
    };
    createInfo.enabledExtensionCount = 3;
    createInfo.ppEnabledExtensionNames = ext3;
    
    // Try WITH portability flag
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&createInfo, NULL, &instance);
    printf("  Result (WITH flag): %d ", result);
    if (result == VK_SUCCESS) {
        printf("✓ SUCCESS\n");
        vkDestroyInstance(instance, NULL);
    } else {
        printf("✗ FAILED\n");
    }
    
    // Try WITHOUT portability flag
    createInfo.flags = 0;
    instance = VK_NULL_HANDLE;
    result = vkCreateInstance(&createInfo, NULL, &instance);
    printf("  Result (WITHOUT flag): %d ", result);
    if (result == VK_SUCCESS) {
        printf("✓ SUCCESS\n");
        vkDestroyInstance(instance, NULL);
    } else {
        printf("✗ FAILED\n");
    }
    
    printf("\nTest 4: Check available extensions\n");
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    printf("  Available extensions: %u\n", extensionCount);
    
    VkExtensionProperties *extensions = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
    
    printf("  Checking for required extensions:\n");
    
    for (uint32_t i = 0; i < extensionCount; i++) {
        if (strcmp(extensions[i].extensionName, "VK_KHR_surface") == 0) {
            printf("    ✓ VK_KHR_surface found\n");
        }
        if (strcmp(extensions[i].extensionName, "VK_EXT_metal_surface") == 0) {
            printf("    ✓ VK_EXT_metal_surface found\n");
        }
        if (strcmp(extensions[i].extensionName, "VK_KHR_portability_enumeration") == 0) {
            printf("    ✓ VK_KHR_portability_enumeration found\n");
        }
    }
    
    free(extensions);
    
    return 0;
}
EOF

echo "=== Compiling test_vulkan_simple.c ==="
clang -o /tmp/test_vulkan_simple /tmp/test_vulkan_simple.c \
    -I/Users/felipebraz/VulkanSDK/1.4.328.1/macOS/include \
    -L/Users/felipebraz/VulkanSDK/1.4.328.1/macOS/lib \
    -lvulkan -framework Cocoa 2>&1

if [ -f /tmp/test_vulkan_simple ]; then
    echo ""
    echo "=== Running test_vulkan_simple ==="
    /tmp/test_vulkan_simple
else
    echo "Compilation failed"
fi
