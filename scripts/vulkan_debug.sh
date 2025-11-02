#!/bin/bash

# Script de debug para investigar Vulkan no macOS

echo "=== Vulkan SDK Information ==="
echo "Vulkan Version: $(vulkaninfo --version 2>/dev/null || echo 'NOT FOUND')"
echo ""

echo "=== MoltenVK ICD Location ==="
if [ -f "$HOME/.vulkan/icd.d/MoltenVK_icd.json" ]; then
    echo "✓ Found: $HOME/.vulkan/icd.d/MoltenVK_icd.json"
    cat "$HOME/.vulkan/icd.d/MoltenVK_icd.json"
else
    echo "✗ NOT FOUND at standard location"
fi
echo ""

echo "=== Environment Variables ==="
echo "VK_ICD_FILENAMES=${VK_ICD_FILENAMES:-NOT SET}"
echo "VK_LAYER_PATH=${VK_LAYER_PATH:-NOT SET}"
echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH:-NOT SET}"
echo "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH:-NOT SET}"
echo ""

echo "=== Vulkan Instance Extensions (from vulkaninfo) ==="
vulkaninfo 2>/dev/null | grep -A 100 "Instance Extensions" | head -20 || echo "vulkaninfo failed"
echo ""

echo "=== MoltenVK Library Check ==="
if command -v find &> /dev/null; then
    echo "Searching for libMoltenVK..."
    find /opt/vulkan-sdk -name "libMoltenVK*" 2>/dev/null | head -10
    
    # find command can be slow, switch macos native tool
    #find $HOME -name "libMoltenVK*" 2>/dev/null | head -10

    #use macos native tool to check if the library can be found
    mdfind -name "libMoltenVK" 2>/dev/null
fi
echo ""

echo "=== Test: Create simple Vulkan instance ==="
# Try to create a simple Vulkan app that just creates/destroys an instance
cat > /tmp/test_vulkan.c << 'EOF'
#include <stdio.h>
#include <vulkan/vulkan.h>

int main() {
    printf("Testing Vulkan instance creation...\n");
    
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_0
    };
    
    const char *ext[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_METAL_SURFACE_EXTENSION_NAME
    };
    
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = ext
    };
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    
    printf("vkCreateInstance returned: %d\n", result);
    if (result == VK_SUCCESS) {
        printf("✓ SUCCESS: Instance created\n");
        vkDestroyInstance(instance, NULL);
    } else {
        printf("✗ FAILED: Error code %d\n", result);
    }
    
    return result;
}
EOF

echo "Compiling test_vulkan.c..."
clang -o /tmp/test_vulkan /tmp/test_vulkan.c -I/opt/vulkan-sdk/include -L/opt/vulkan-sdk/lib -lvulkan 2>&1 || \
clang -o /tmp/test_vulkan /tmp/test_vulkan.c $(pkg-config --cflags --libs vulkan) 2>&1 || \
echo "Failed to compile"

if [ -f /tmp/test_vulkan ]; then
    echo "Running test..."
    /tmp/test_vulkan
else
    echo "Compilation failed"
fi
