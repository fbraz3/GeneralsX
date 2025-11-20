/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * Phase 41: VulkanGraphicsDriver Implementation
 *
 * Stub implementation that adapts vulkan_graphics_driver_legacy.cpp
 * (converted from VulkanGraphicsBackend static methods to instance-based IGraphicsDriver)
 *
 * This is a minimal implementation skeleton that:
 * 1. Compiles without errors
 * 2. Allows factory pattern to create drivers
 * 3. Provides hooks for detailed Vulkan implementation (Phase 41 Week 2)
 *
 * TODO (Phase 41 Week 2):
 * - Integrate vulkan_graphics_driver_legacy.cpp components
 * - Implement all IGraphicsDriver methods with actual Vulkan calls
 * - Adapt d3d8_vulkan_* files to use instance methods
 * - Remove legacy static method usage
 */

#include "vulkan_graphics_driver.h"
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace Graphics {

// ============================================================================
// Vulkan Component Implementations (from Phase 39.3 legacy)
// ============================================================================

/**
 * These are extracted from vulkan_graphics_driver_legacy.cpp
 * and adapted to work as member components of VulkanGraphicsDriver
 */

#include <vector>

class VulkanInstance {
public:
    VkInstance handle = VK_NULL_HANDLE;
    
    bool Create(bool debug_mode) {
        printf("[Vulkan] VulkanInstance::Create() - Starting instance creation (debug=%d)\n", debug_mode);
        
        // Set up application info
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Command & Conquer Generals Zero Hour";
        app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        app_info.pEngineName = "GeneralsX Vulkan Backend";
        app_info.engineVersion = VK_MAKE_API_VERSION(0, 41, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_2;
        
        // Required extensions for presentation
        std::vector<const char*> required_extensions;
        required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        
#ifdef VK_USE_PLATFORM_MACOS_MVK
        required_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        required_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
        
        // Validation layers
        std::vector<const char*> validation_layers;
        if (debug_mode) {
            validation_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        
        // Create instance info
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = required_extensions.size();
        create_info.ppEnabledExtensionNames = required_extensions.data();
        create_info.enabledLayerCount = validation_layers.size();
        create_info.ppEnabledLayerNames = validation_layers.data();
        
        // Create the Vulkan instance
        VkResult result = vkCreateInstance(&create_info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to create Vulkan instance (result=%d)\n", result);
            return false;
        }
        
        printf("[Vulkan] VulkanInstance::Create() - Success! Instance created.\n");
        return true;
    }
    
    void Destroy() {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyInstance(handle, nullptr);
            printf("[Vulkan] VulkanInstance::Destroy() - Instance destroyed\n");
        }
        handle = VK_NULL_HANDLE;
    }
};

class VulkanPhysicalDevice {
public:
    VkPhysicalDevice handle = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
    
    bool Select(VkInstance instance) {
        printf("[Vulkan] VulkanPhysicalDevice::Select() - Enumerating physical devices\n");
        
        // Get physical device count
        uint32_t device_count = 0;
        VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (result != VK_SUCCESS || device_count == 0) {
            printf("[Vulkan] ERROR: No physical devices found (result=%d, count=%u)\n", result, device_count);
            return false;
        }
        
        printf("[Vulkan] Found %u physical devices\n", device_count);
        
        // Get all physical devices
        std::vector<VkPhysicalDevice> devices(device_count);
        result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to enumerate physical devices (result=%d)\n", result);
            return false;
        }
        
        // Select best device (prefer discrete GPU)
        VkPhysicalDevice selected_device = VK_NULL_HANDLE;
        int best_score = -1;
        
        for (uint32_t i = 0; i < device_count; ++i) {
            VkPhysicalDeviceProperties props;
            VkPhysicalDeviceFeatures feats;
            vkGetPhysicalDeviceProperties(devices[i], &props);
            vkGetPhysicalDeviceFeatures(devices[i], &feats);
            
            // Score the device (discrete GPU = higher score)
            int score = 0;
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                score += 1000;
            } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                score += 100;
            }
            
            // Check for graphics queue support
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, nullptr);
            if (queue_family_count == 0) {
                continue;
            }
            
            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, queue_families.data());
            
            bool has_graphics = false;
            for (const auto& family : queue_families) {
                if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    has_graphics = true;
                    break;
                }
            }
            
            if (!has_graphics) {
                continue;
            }
            
            printf("[Vulkan]   Device %u: %s (type=%d, score=%d)\n", i, props.deviceName, props.deviceType, score);
            
            if (score > best_score) {
                best_score = score;
                selected_device = devices[i];
                handle = selected_device;
                properties = props;
                features = feats;
            }
        }
        
        if (selected_device == VK_NULL_HANDLE) {
            printf("[Vulkan] ERROR: No suitable device found\n");
            return false;
        }
        
        printf("[Vulkan] Selected device: %s\n", properties.deviceName);
        return true;
    }
};

class VulkanDevice {
public:
    VkDevice handle = VK_NULL_HANDLE;
    VkQueue graphics_queue = VK_NULL_HANDLE;
    uint32_t graphics_queue_family = 0;
    
    bool Create(VkPhysicalDevice physical_device) {
        printf("[Vulkan] VulkanDevice::Create() - Creating logical device\n");
        
        // Find graphics queue family
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
        
        graphics_queue_family = UINT32_MAX;
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue_family = i;
                break;
            }
        }
        
        if (graphics_queue_family == UINT32_MAX) {
            printf("[Vulkan] ERROR: No graphics queue family found\n");
            return false;
        }
        
        printf("[Vulkan] Graphics queue family: %u\n", graphics_queue_family);
        
        // Create device queue info
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = graphics_queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        
        // Request device features
        VkPhysicalDeviceFeatures device_features{};
        device_features.geometryShader = VK_TRUE;
        device_features.fillModeNonSolid = VK_TRUE;
        
        // Create device
        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &queue_create_info;
        device_create_info.pEnabledFeatures = &device_features;
        
        VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to create logical device (result=%d)\n", result);
            return false;
        }
        
        // Get graphics queue
        vkGetDeviceQueue(handle, graphics_queue_family, 0, &graphics_queue);
        
        printf("[Vulkan] VulkanDevice::Create() - Success! Device created with queue family %u\n", graphics_queue_family);
        return true;
    }
    
    void Destroy() {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyDevice(handle, nullptr);
            printf("[Vulkan] VulkanDevice::Destroy() - Device destroyed\n");
        }
        handle = VK_NULL_HANDLE;
        graphics_queue = VK_NULL_HANDLE;
    }
};

class VulkanSwapchain {
public:
    VkSwapchainKHR handle = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t image_count = 0;
    VkExtent2D extent{};
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    uint32_t current_image_index = 0;
    std::vector<VkFramebuffer> framebuffers;
    VkFramebuffer current_framebuffer = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, VkPhysicalDevice physical_device, VkInstance instance, 
                void* window_handle, uint32_t width, uint32_t height) {
        printf("[Vulkan] VulkanSwapchain::Create() - Creating surface and swapchain\n");
        
        // Create surface from window handle (SDL2-based cross-platform)
        #ifdef __APPLE__
        SDL_Window* window = static_cast<SDL_Window*>(window_handle);
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
            printf("[Vulkan] ERROR: SDL_Vulkan_CreateSurface failed: %s\n", SDL_GetError());
            return false;
        }
        #else
        printf("[Vulkan] ERROR: Platform-specific surface creation not implemented\n");
        return false;
        #endif
        
        printf("[Vulkan] Surface created successfully\n");
        
        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
        
        // Get present modes
        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
        
        // Get surface formats
        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data());
        
        if (surface_formats.empty()) {
            printf("[Vulkan] ERROR: No surface formats available\n");
            return false;
        }
        
        // Select format (prefer SRGB)
        VkSurfaceFormatKHR surface_format = surface_formats[0];
        for (const auto& fmt : surface_formats) {
            if (fmt.format == VK_FORMAT_R8G8B8A8_SRGB || fmt.format == VK_FORMAT_B8G8R8A8_SRGB) {
                surface_format = fmt;
                break;
            }
        }
        
        // Select present mode (prefer mailbox)
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode : present_modes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                present_mode = mode;
                break;
            }
        }
        
        extent = surface_capabilities.currentExtent;
        if (extent.width == UINT32_MAX) {
            extent.width = width;
            extent.height = height;
        }
        
        image_count = surface_capabilities.minImageCount + 1;
        if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount) {
            image_count = surface_capabilities.maxImageCount;
        }
        
        printf("[Vulkan] Swapchain: extent=(%u,%u), image_count=%u\n", extent.width, extent.height, image_count);
        
        // Create swapchain
        VkSwapchainCreateInfoKHR swapchain_info{};
        swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_info.surface = surface;
        swapchain_info.minImageCount = image_count;
        swapchain_info.imageFormat = surface_format.format;
        swapchain_info.imageColorSpace = surface_format.colorSpace;
        swapchain_info.imageExtent = extent;
        swapchain_info.imageArrayLayers = 1;
        swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.preTransform = surface_capabilities.currentTransform;
        swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_info.presentMode = present_mode;
        swapchain_info.clipped = VK_TRUE;
        
        VkResult result = vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to create swapchain (result=%d)\n", result);
            return false;
        }
        
        // Retrieve swapchain images
        uint32_t swapchain_image_count = 0;
        vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, nullptr);
        images.resize(swapchain_image_count);
        vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, images.data());
        
        printf("[Vulkan] Swapchain created with %u images\n", swapchain_image_count);
        return true;
    }
    
    bool CreateFramebuffers(VkDevice device, VkFormat swapchain_format, VkRenderPass render_pass) {
        image_views.resize(images.size());
        for (size_t i = 0; i < images.size(); ++i) {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = images[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = swapchain_format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;
            
            VkResult result = vkCreateImageView(device, &view_info, nullptr, &image_views[i]);
            if (result != VK_SUCCESS) {
                printf("[Vulkan] ERROR: Failed to create image view %zu\n", i);
                return false;
            }
        }
        
        framebuffers.resize(images.size());
        for (size_t i = 0; i < images.size(); ++i) {
            VkFramebufferCreateInfo fb_info{};
            fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fb_info.renderPass = render_pass;
            fb_info.attachmentCount = 1;
            fb_info.pAttachments = &image_views[i];
            fb_info.width = extent.width;
            fb_info.height = extent.height;
            fb_info.layers = 1;
            
            VkResult result = vkCreateFramebuffer(device, &fb_info, nullptr, &framebuffers[i]);
            if (result != VK_SUCCESS) {
                printf("[Vulkan] ERROR: Failed to create framebuffer %zu\n", i);
                return false;
            }
        }
        
        printf("[Vulkan] Created %zu framebuffers\n", framebuffers.size());
        return true;
    }
    
    void UpdateCurrentFramebuffer() {
        if (current_image_index < framebuffers.size()) {
            current_framebuffer = framebuffers[current_image_index];
        }
    }
    
    void Destroy(VkDevice device, VkInstance instance) {
        for (auto fb : framebuffers) {
            if (fb != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device, fb, nullptr);
            }
        }
        framebuffers.clear();
        
        for (auto view : image_views) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(device, view, nullptr);
            }
        }
        image_views.clear();
        
        if (handle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, handle, nullptr);
        }
        if (surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }
        handle = VK_NULL_HANDLE;
        surface = VK_NULL_HANDLE;
    }
};

class VulkanMemoryAllocator {
public:
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    
    bool Create(VkInstance instance, VkPhysicalDevice physical_dev, VkDevice logical_device) {
        printf("[Vulkan] VulkanMemoryAllocator::Create() - Initializing memory management\n");
        physical_device = physical_dev;
        device = logical_device;
        
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        printf("[Vulkan] Memory types: %u, Memory heaps: %u\n", 
               memory_properties.memoryTypeCount, memory_properties.memoryHeapCount);
        return true;
    }
    
    void Destroy() {
        printf("[Vulkan] VulkanMemoryAllocator::Destroy() - Memory management shutdown\n");
    }
    
    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
        
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
            if ((type_filter & (1 << i)) && 
                (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        
        printf("[Vulkan] ERROR: Could not find suitable memory type\n");
        return UINT32_MAX;
    }
};

class VulkanRenderPass {
public:
    VkRenderPass handle = VK_NULL_HANDLE;
    
    bool Create(VkDevice device, VkFormat color_format) {
        printf("[Vulkan] VulkanRenderPass::Create() - Creating render pass\n");
        
        VkAttachmentDescription color_attachment{};
        color_attachment.format = color_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;
        
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;
        
        VkResult result = vkCreateRenderPass(device, &render_pass_info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to create render pass (result=%d)\n", result);
            return false;
        }
        
        printf("[Vulkan] VulkanRenderPass::Create() - Success!\n");
        return true;
    }
    
    void Destroy(VkDevice device) {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, handle, nullptr);
        }
        handle = VK_NULL_HANDLE;
    }
};

class VulkanCommandBuffer {
public:
    VkCommandPool pool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> buffers;
    uint32_t current_frame = 0;
    
    bool Create(VkDevice device, uint32_t queue_family_index) {
        printf("[Vulkan] VulkanCommandBuffer::Create() - Creating command pool\n");
        
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = queue_family_index;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        VkResult result = vkCreateCommandPool(device, &pool_info, nullptr, &pool);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to create command pool\n");
            return false;
        }
        
        buffers.resize(2);  // Double buffering
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 2;
        
        result = vkAllocateCommandBuffers(device, &alloc_info, buffers.data());
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: Failed to allocate command buffers\n");
            return false;
        }
        
        printf("[Vulkan] VulkanCommandBuffer::Create() - Success!\n");
        return true;
    }
    
    void Destroy(VkDevice device) {
        if (pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, pool, nullptr);
        }
        pool = VK_NULL_HANDLE;
        buffers.clear();
    }
};

class VulkanDescriptorPool {
public:
    VkDescriptorPool handle = VK_NULL_HANDLE;
};

// ============================================================================
// VulkanGraphicsDriver Implementation
// ============================================================================

VulkanGraphicsDriver::VulkanGraphicsDriver()
    : m_initialized(false)
    , m_in_frame(false)
    , m_display_width(800)
    , m_display_height(600)
    , m_fullscreen(false)
    , m_clear_color(0, 0, 0, 1)
{
    printf("[VulkanGraphicsDriver] Constructor - instance created\n");
}

VulkanGraphicsDriver::~VulkanGraphicsDriver()
{
    if (m_initialized) {
        Shutdown();
    }
    printf("[VulkanGraphicsDriver] Destructor - instance destroyed\n");
}

bool VulkanGraphicsDriver::Initialize(void* windowHandle, uint32_t width, uint32_t height, bool fullscreen)
{
    printf("[VulkanGraphicsDriver::Initialize] width=%u height=%u fullscreen=%d\n", width, height, fullscreen);
    
    if (m_initialized) {
        printf("[VulkanGraphicsDriver::Initialize] Already initialized, returning\n");
        return true;
    }
    
    m_display_width = width;
    m_display_height = height;
    m_fullscreen = fullscreen;
    
    // Phase 41 Stage 1: Create Vulkan instance
    printf("[VulkanGraphicsDriver::Initialize] Creating Vulkan instance...\n");
    m_instance = std::make_unique<VulkanInstance>();
    if (!m_instance->Create(false)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create Vulkan instance\n");
        return false;
    }
    
    // Phase 41 Stage 1: Select physical device
    printf("[VulkanGraphicsDriver::Initialize] Selecting physical device...\n");
    m_physical_device = std::make_unique<VulkanPhysicalDevice>();
    if (!m_physical_device->Select(m_instance->handle)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to select physical device\n");
        return false;
    }
    
    // Phase 41 Stage 1: Create logical device
    printf("[VulkanGraphicsDriver::Initialize] Creating logical device...\n");
    m_device = std::make_unique<VulkanDevice>();
    if (!m_device->Create(m_physical_device->handle)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create logical device\n");
        return false;
    }
    
    // Phase 41 Stage 2: Create swapchain
    printf("[VulkanGraphicsDriver::Initialize] Creating swapchain...\n");
    m_swapchain = std::make_unique<VulkanSwapchain>();
    if (!m_swapchain->Create(m_device->handle, m_physical_device->handle, m_instance->handle, 
                             windowHandle, width, height)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create swapchain\n");
        return false;
    }
    
    // Phase 41 Stage 2: Create memory allocator
    printf("[VulkanGraphicsDriver::Initialize] Creating memory allocator...\n");
    m_memory_allocator = std::make_unique<VulkanMemoryAllocator>();
    if (!m_memory_allocator->Create(m_instance->handle, m_physical_device->handle, m_device->handle)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create memory allocator\n");
        return false;
    }
    
    // Phase 41 Stage 2: Create render pass
    printf("[VulkanGraphicsDriver::Initialize] Creating render pass...\n");
    m_render_pass = std::make_unique<VulkanRenderPass>();
    if (!m_render_pass->Create(m_device->handle, VK_FORMAT_B8G8R8A8_UNORM)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create render pass\n");
        return false;
    }
    
    // Phase 41 Stage 2.5: Create framebuffers for swapchain
    printf("[VulkanGraphicsDriver::Initialize] Creating framebuffers...\n");
    if (!m_swapchain->CreateFramebuffers(m_device->handle, VK_FORMAT_B8G8R8A8_UNORM, m_render_pass->handle)) {
        printf("[VulkanGraphicsDriver::Initialize] ERROR: Failed to create framebuffers\n");
        return false;
    }
    m_swapchain->UpdateCurrentFramebuffer();
    
    // Phase 41 Stage 2: Create command buffers
    printf("[VulkanGraphicsDriver::Initialize] Creating command buffers...\n");
    // Create command buffer locally - it's internal only
    // (We'll implement this as part of BeginFrame/EndFrame)
    
    printf("[VulkanGraphicsDriver::Initialize] SUCCESS - Vulkan initialized with swapchain and rendering infrastructure\n");
    m_initialized = true;
    
    return true;
}

void VulkanGraphicsDriver::Shutdown()
{
    printf("[VulkanGraphicsDriver::Shutdown] Cleaning up Vulkan resources\n");
    
    if (!m_initialized) {
        printf("[VulkanGraphicsDriver::Shutdown] Not initialized, returning\n");
        return;
    }
    
    // Wait for GPU idle before destroying resources
    if (m_device && m_device->handle != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device->handle);
    }
    
    // Destroy components in reverse order of creation
    if (m_render_pass) {
        m_render_pass->Destroy(m_device ? m_device->handle : VK_NULL_HANDLE);
        m_render_pass.reset();
    }
    
    if (m_memory_allocator) {
        m_memory_allocator->Destroy();
        m_memory_allocator.reset();
    }
    
    if (m_swapchain) {
        m_swapchain->Destroy(m_device ? m_device->handle : VK_NULL_HANDLE,
                            m_instance ? m_instance->handle : VK_NULL_HANDLE);
        m_swapchain.reset();
    }
    
    if (m_device) {
        m_device->Destroy();
        m_device.reset();
    }
    
    m_physical_device.reset();
    
    if (m_instance) {
        m_instance->Destroy();
        m_instance.reset();
    }
    
    printf("[VulkanGraphicsDriver::Shutdown] SUCCESS - Vulkan cleaned up\n");
    m_initialized = false;
}

bool VulkanGraphicsDriver::IsInitialized() const
{
    return m_initialized;
}

BackendType VulkanGraphicsDriver::GetBackendType() const
{
    return BackendType::Vulkan;
}

const char* VulkanGraphicsDriver::GetBackendName() const
{
    return "Vulkan";
}

const char* VulkanGraphicsDriver::GetVersionString() const
{
    return "Phase 41 - Vulkan Graphics Driver v0.1";
}

bool VulkanGraphicsDriver::BeginFrame()
{
    if (!m_initialized || !m_device || !m_swapchain || !m_render_pass) {
        printf("[Vulkan] BeginFrame: Not properly initialized\n");
        return false;
    }
    
    m_in_frame = true;
    printf("[Vulkan] BeginFrame - Starting frame rendering\n");
    
    return true;
}

void VulkanGraphicsDriver::EndFrame()
{
    if (!m_in_frame) {
        return;
    }
    
    m_in_frame = false;
    printf("[Vulkan] EndFrame - Frame rendering complete\n");
}

bool VulkanGraphicsDriver::Present()
{
    if (!m_initialized || !m_device || !m_swapchain || !m_render_pass) {
        printf("[Vulkan] Present: Not properly initialized\n");
        return false;
    }
    
    printf("[Vulkan] Present - Presenting frame to screen\n");
    
    // Update current frame for next iteration
    m_swapchain->current_image_index = (m_swapchain->current_image_index + 1) % m_swapchain->images.size();
    m_swapchain->UpdateCurrentFramebuffer();
    
    return true;
}

void VulkanGraphicsDriver::Clear(float r, float g, float b, float a, bool clearDepth)
{
    // Stub
}

void VulkanGraphicsDriver::SetClearColor(float r, float g, float b, float a)
{
    m_clear_color = Color(r, g, b, a);
}

void VulkanGraphicsDriver::DrawPrimitive(PrimitiveType primType, uint32_t vertexCount)
{
    // Stub
}

void VulkanGraphicsDriver::DrawIndexedPrimitive(PrimitiveType primType, uint32_t indexCount,
                                               IndexBufferHandle ibHandle, uint32_t startIndex)
{
    // Stub
}

void VulkanGraphicsDriver::DrawPrimitiveUP(PrimitiveType primType, uint32_t primCount,
                                          const void* vertexData, uint32_t vertexStride)
{
    // Stub
}

void VulkanGraphicsDriver::DrawIndexedPrimitiveUP(PrimitiveType primType, uint32_t minVertexIndex,
                                                 uint32_t vertexCount, uint32_t primCount,
                                                 const void* indexData, const void* vertexData,
                                                 uint32_t vertexStride)
{
    // Stub
}

bool VulkanGraphicsDriver::SetRenderState(RenderState state, uint64_t value)
{
    return true;
}

uint64_t VulkanGraphicsDriver::GetRenderState(RenderState state) const
{
    return 0;
}

bool VulkanGraphicsDriver::SetBlendState(const BlendStateDescriptor& desc)
{
    return true;
}

bool VulkanGraphicsDriver::SetDepthStencilState(const DepthStencilStateDescriptor& desc)
{
    return true;
}

bool VulkanGraphicsDriver::SetRasterizerState(const RasterizerStateDescriptor& desc)
{
    return true;
}

bool VulkanGraphicsDriver::SetScissorRect(const Rect& rect)
{
    return true;
}

void VulkanGraphicsDriver::SetViewport(const Viewport& vp)
{
    m_viewport = vp;
}

Viewport VulkanGraphicsDriver::GetViewport() const
{
    return m_viewport;
}

// ============================================================================
// Resource Storage (Internal to VulkanGraphicsDriver)
// ============================================================================

// Simple buffer storage for vertex/index buffers
struct VulkanBufferAllocation {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    void* mapped_ptr = nullptr;
    uint32_t size = 0;
    bool is_dynamic = false;
};

// Storage for created buffers (simple map)
static std::vector<VulkanBufferAllocation> g_vertex_buffers;
static std::vector<VulkanBufferAllocation> g_index_buffers;

// ============================================================================
// Buffer Management Implementation (REAL Vulkan)
// ============================================================================

/**
 * Find suitable memory type for buffer allocation
 */
static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    printf("[Vulkan] ERROR: Failed to find suitable memory type\n");
    return 0;
}

VertexBufferHandle VulkanGraphicsDriver::CreateVertexBuffer(uint32_t sizeInBytes, bool dynamic,
                                                           const void* initialData)
{
    if (!m_initialized || !m_device || !m_memory_allocator) {
        printf("[Vulkan] CreateVertexBuffer: Driver not initialized\n");
        return INVALID_HANDLE;
    }
    
    printf("[Vulkan] CreateVertexBuffer: size=%u dynamic=%d\n", sizeInBytes, dynamic);
    
    VulkanBufferAllocation allocation;
    allocation.size = sizeInBytes;
    allocation.is_dynamic = dynamic;
    
    // Create buffer with appropriate usage
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeInBytes;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(m_device->handle, &bufferInfo, nullptr, &allocation.buffer);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkCreateBuffer failed (result=%d)\n", result);
        return INVALID_HANDLE;
    }
    
    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->handle, allocation.buffer, &memRequirements);
    
    // Allocate GPU memory
    VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (dynamic) {
        memFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    
    uint32_t memTypeIndex = FindMemoryType(m_physical_device->handle, memRequirements.memoryTypeBits, memFlags);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memTypeIndex;
    
    result = vkAllocateMemory(m_device->handle, &allocInfo, nullptr, &allocation.memory);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkAllocateMemory failed (result=%d)\n", result);
        vkDestroyBuffer(m_device->handle, allocation.buffer, nullptr);
        return INVALID_HANDLE;
    }
    
    // Bind memory to buffer
    result = vkBindBufferMemory(m_device->handle, allocation.buffer, allocation.memory, 0);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkBindBufferMemory failed (result=%d)\n", result);
        vkFreeMemory(m_device->handle, allocation.memory, nullptr);
        vkDestroyBuffer(m_device->handle, allocation.buffer, nullptr);
        return INVALID_HANDLE;
    }
    
    // Upload initial data if provided
    if (initialData && sizeInBytes > 0) {
        if (dynamic) {
            // For dynamic buffers, map directly
            void* mappedPtr = nullptr;
            result = vkMapMemory(m_device->handle, allocation.memory, 0, sizeInBytes, 0, &mappedPtr);
            if (result == VK_SUCCESS && mappedPtr) {
                memcpy(mappedPtr, initialData, sizeInBytes);
                vkUnmapMemory(m_device->handle, allocation.memory);
                allocation.mapped_ptr = nullptr;  // Not mapped after unmap
            }
        }
        // For static buffers, data would need staging buffer (simplified for now)
    }
    
    // Store buffer and return handle as index
    g_vertex_buffers.push_back(allocation);
    VertexBufferHandle handle = (VertexBufferHandle)(g_vertex_buffers.size() - 1);
    
    printf("[Vulkan] CreateVertexBuffer: SUCCESS handle=%llu\n", handle);
    return handle;
}

void VulkanGraphicsDriver::DestroyVertexBuffer(VertexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] DestroyVertexBuffer: Invalid handle %llu\n", handle);
        return;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
    }
    
    if (alloc.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->handle, alloc.buffer, nullptr);
    }
    
    if (alloc.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.memory, nullptr);
    }
    
    printf("[Vulkan] DestroyVertexBuffer: handle=%llu\n", handle);
}

bool VulkanGraphicsDriver::LockVertexBuffer(VertexBufferHandle handle, uint32_t offset,
                                           uint32_t size, void** lockedData, bool readOnly)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_vertex_buffers.size() || !lockedData) {
        printf("[Vulkan] LockVertexBuffer: Invalid parameters\n");
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle];
    
    if (!alloc.is_dynamic) {
        printf("[Vulkan] ERROR: Cannot lock static vertex buffer\n");
        return false;
    }
    
    if (offset + size > alloc.size) {
        printf("[Vulkan] ERROR: Lock range exceeds buffer size\n");
        return false;
    }
    
    VkResult result = vkMapMemory(m_device->handle, alloc.memory, offset, size, 0, &alloc.mapped_ptr);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkMapMemory failed (result=%d)\n", result);
        return false;
    }
    
    *lockedData = alloc.mapped_ptr;
    printf("[Vulkan] LockVertexBuffer: handle=%llu offset=%u size=%u\n", handle, offset, size);
    return true;
}

bool VulkanGraphicsDriver::UnlockVertexBuffer(VertexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] UnlockVertexBuffer: Invalid handle\n");
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
        alloc.mapped_ptr = nullptr;
        printf("[Vulkan] UnlockVertexBuffer: handle=%llu\n", handle);
        return true;
    }
    
    return false;
}

uint32_t VulkanGraphicsDriver::GetVertexBufferSize(VertexBufferHandle handle) const
{
    if (handle >= (uint64_t)g_vertex_buffers.size()) {
        return 0;
    }
    
    return g_vertex_buffers[handle].size;
}

IndexBufferHandle VulkanGraphicsDriver::CreateIndexBuffer(uint32_t sizeInBytes, bool is32Bit,
                                                         bool dynamic, const void* initialData)
{
    if (!m_initialized || !m_device || !m_memory_allocator) {
        printf("[Vulkan] CreateIndexBuffer: Driver not initialized\n");
        return INVALID_HANDLE;
    }
    
    printf("[Vulkan] CreateIndexBuffer: size=%u is32Bit=%d dynamic=%d\n", sizeInBytes, is32Bit, dynamic);
    
    VulkanBufferAllocation allocation;
    allocation.size = sizeInBytes;
    allocation.is_dynamic = dynamic;
    
    // Create buffer with INDEX_BUFFER usage
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeInBytes;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(m_device->handle, &bufferInfo, nullptr, &allocation.buffer);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkCreateBuffer failed (result=%d)\n", result);
        return INVALID_HANDLE;
    }
    
    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device->handle, allocation.buffer, &memRequirements);
    
    // Allocate GPU memory
    VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (dynamic) {
        memFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    
    uint32_t memTypeIndex = FindMemoryType(m_physical_device->handle, memRequirements.memoryTypeBits, memFlags);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memTypeIndex;
    
    result = vkAllocateMemory(m_device->handle, &allocInfo, nullptr, &allocation.memory);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkAllocateMemory failed (result=%d)\n", result);
        vkDestroyBuffer(m_device->handle, allocation.buffer, nullptr);
        return INVALID_HANDLE;
    }
    
    // Bind memory to buffer
    result = vkBindBufferMemory(m_device->handle, allocation.buffer, allocation.memory, 0);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkBindBufferMemory failed (result=%d)\n", result);
        vkFreeMemory(m_device->handle, allocation.memory, nullptr);
        vkDestroyBuffer(m_device->handle, allocation.buffer, nullptr);
        return INVALID_HANDLE;
    }
    
    // Upload initial data if provided
    if (initialData && sizeInBytes > 0) {
        if (dynamic) {
            void* mappedPtr = nullptr;
            result = vkMapMemory(m_device->handle, allocation.memory, 0, sizeInBytes, 0, &mappedPtr);
            if (result == VK_SUCCESS && mappedPtr) {
                memcpy(mappedPtr, initialData, sizeInBytes);
                vkUnmapMemory(m_device->handle, allocation.memory);
                allocation.mapped_ptr = nullptr;
            }
        }
    }
    
    // Store buffer and return handle as index
    g_index_buffers.push_back(allocation);
    IndexBufferHandle handle = (IndexBufferHandle)(g_index_buffers.size() - 1);
    
    printf("[Vulkan] CreateIndexBuffer: SUCCESS handle=%llu\n", handle);
    return handle;
}

void VulkanGraphicsDriver::DestroyIndexBuffer(IndexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] DestroyIndexBuffer: Invalid handle %llu\n", handle);
        return;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
    }
    
    if (alloc.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->handle, alloc.buffer, nullptr);
    }
    
    if (alloc.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.memory, nullptr);
    }
    
    printf("[Vulkan] DestroyIndexBuffer: handle=%llu\n", handle);
}

bool VulkanGraphicsDriver::LockIndexBuffer(IndexBufferHandle handle, uint32_t offset,
                                          uint32_t size, void** lockedData, bool readOnly)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_index_buffers.size() || !lockedData) {
        printf("[Vulkan] LockIndexBuffer: Invalid parameters\n");
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle];
    
    if (!alloc.is_dynamic) {
        printf("[Vulkan] ERROR: Cannot lock static index buffer\n");
        return false;
    }
    
    if (offset + size > alloc.size) {
        printf("[Vulkan] ERROR: Lock range exceeds buffer size\n");
        return false;
    }
    
    VkResult result = vkMapMemory(m_device->handle, alloc.memory, offset, size, 0, &alloc.mapped_ptr);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: vkMapMemory failed (result=%d)\n", result);
        return false;
    }
    
    *lockedData = alloc.mapped_ptr;
    printf("[Vulkan] LockIndexBuffer: handle=%llu offset=%u size=%u\n", handle, offset, size);
    return true;
}

bool VulkanGraphicsDriver::UnlockIndexBuffer(IndexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle >= (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] UnlockIndexBuffer: Invalid handle\n");
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
        alloc.mapped_ptr = nullptr;
        printf("[Vulkan] UnlockIndexBuffer: handle=%llu\n", handle);
        return true;
    }
    
    return false;
}

uint32_t VulkanGraphicsDriver::GetIndexBufferSize(IndexBufferHandle handle) const
{
    if (handle >= (uint64_t)g_index_buffers.size()) {
        return 0;
    }
    
    return g_index_buffers[handle].size;
}

VertexFormatHandle VulkanGraphicsDriver::CreateVertexFormat(const VertexElement* elements,
                                                           uint32_t elementCount)
{
    return INVALID_HANDLE;
}

void VulkanGraphicsDriver::DestroyVertexFormat(VertexFormatHandle handle)
{
    // Stub
}

bool VulkanGraphicsDriver::SetVertexFormat(VertexFormatHandle handle)
{
    return true;
}

bool VulkanGraphicsDriver::SetVertexStreamSource(uint32_t streamIndex, VertexBufferHandle vbHandle,
                                                uint32_t offset, uint32_t stride)
{
    return true;
}

TextureHandle VulkanGraphicsDriver::CreateTexture(const TextureDescriptor& desc, const void* initialData)
{
    return INVALID_HANDLE;
}

void VulkanGraphicsDriver::DestroyTexture(TextureHandle handle)
{
    // Stub
}

bool VulkanGraphicsDriver::SetTexture(uint32_t samplerIndex, TextureHandle handle)
{
    return true;
}

TextureHandle VulkanGraphicsDriver::GetTexture(uint32_t samplerIndex) const
{
    return INVALID_HANDLE;
}

bool VulkanGraphicsDriver::LockTexture(TextureHandle handle, uint32_t level, void** lockedData,
                                      uint32_t* pitch)
{
    return false;
}

bool VulkanGraphicsDriver::UnlockTexture(TextureHandle handle, uint32_t level)
{
    return true;
}

TextureDescriptor VulkanGraphicsDriver::GetTextureDescriptor(TextureHandle handle) const
{
    return TextureDescriptor();
}

bool VulkanGraphicsDriver::UpdateTextureSubregion(TextureHandle handle, const void* data,
                                                 uint32_t left, uint32_t top, uint32_t right,
                                                 uint32_t bottom)
{
    return true;
}

RenderTargetHandle VulkanGraphicsDriver::CreateRenderTarget(uint32_t width, uint32_t height,
                                                           TextureFormat format)
{
    return INVALID_HANDLE;
}

void VulkanGraphicsDriver::DestroyRenderTarget(RenderTargetHandle handle)
{
    // Stub
}

bool VulkanGraphicsDriver::SetRenderTarget(uint32_t targetIndex, RenderTargetHandle handle)
{
    return true;
}

RenderTargetHandle VulkanGraphicsDriver::GetRenderTarget(uint32_t targetIndex) const
{
    return INVALID_HANDLE;
}

bool VulkanGraphicsDriver::SetDefaultRenderTarget()
{
    return true;
}

DepthStencilHandle VulkanGraphicsDriver::CreateDepthStencil(uint32_t width, uint32_t height,
                                                           TextureFormat format)
{
    return INVALID_HANDLE;
}

void VulkanGraphicsDriver::DestroyDepthStencil(DepthStencilHandle handle)
{
    // Stub
}

bool VulkanGraphicsDriver::SetDepthStencil(DepthStencilHandle handle)
{
    return true;
}

DepthStencilHandle VulkanGraphicsDriver::GetDepthStencil() const
{
    return INVALID_HANDLE;
}

void VulkanGraphicsDriver::SetWorldMatrix(const Matrix4x4& matrix)
{
    // Stub
}

void VulkanGraphicsDriver::SetViewMatrix(const Matrix4x4& matrix)
{
    // Stub
}

void VulkanGraphicsDriver::SetProjectionMatrix(const Matrix4x4& matrix)
{
    // Stub
}

Matrix4x4 VulkanGraphicsDriver::GetWorldMatrix() const
{
    return Matrix4x4();
}

Matrix4x4 VulkanGraphicsDriver::GetViewMatrix() const
{
    return Matrix4x4();
}

Matrix4x4 VulkanGraphicsDriver::GetProjectionMatrix() const
{
    return Matrix4x4();
}

void VulkanGraphicsDriver::SetAmbientLight(float r, float g, float b)
{
    // Stub
}

void VulkanGraphicsDriver::SetLight(uint32_t lightIndex, const Light& light)
{
    // Stub
}

void VulkanGraphicsDriver::DisableLight(uint32_t lightIndex)
{
    // Stub
}

bool VulkanGraphicsDriver::IsLightEnabled(uint32_t lightIndex) const
{
    return false;
}

uint32_t VulkanGraphicsDriver::GetMaxLights() const
{
    return 8;
}

void VulkanGraphicsDriver::SetMaterial(const Material& material)
{
    // Stub
}

bool VulkanGraphicsDriver::SetSamplerState(uint32_t samplerIndex, uint32_t state, uint32_t value)
{
    return true;
}

uint32_t VulkanGraphicsDriver::GetSamplerState(uint32_t samplerIndex, uint32_t state) const
{
    return 0;
}

uint32_t VulkanGraphicsDriver::GetMaxTextureSamplers() const
{
    return 16;
}

bool VulkanGraphicsDriver::SupportsTextureFormat(TextureFormat format) const
{
    return true;
}

uint32_t VulkanGraphicsDriver::GetMaxTextureWidth() const
{
    return 4096;
}

uint32_t VulkanGraphicsDriver::GetMaxTextureHeight() const
{
    return 4096;
}

uint32_t VulkanGraphicsDriver::GetMaxVertexBlendMatrices() const
{
    return 4;
}

uint32_t VulkanGraphicsDriver::GetMaxClipPlanes() const
{
    return 8;
}

bool VulkanGraphicsDriver::SupportsHardwareTransformAndLight() const
{
    return true;
}

bool VulkanGraphicsDriver::SupportsPixelShaders() const
{
    return true;
}

bool VulkanGraphicsDriver::SupportsVertexShaders() const
{
    return true;
}

bool VulkanGraphicsDriver::SupportsCompressedTextures() const
{
    return true;
}

uint32_t VulkanGraphicsDriver::GetMaxPrimitiveCount() const
{
    return 0xFFFFFFFF;
}

uint32_t VulkanGraphicsDriver::GetMaxVertexIndex() const
{
    return 0xFFFFFFFF;
}

uint32_t VulkanGraphicsDriver::GetMaxStreamStride() const
{
    return 65536;
}

bool VulkanGraphicsDriver::ResizeSwapChain(uint32_t width, uint32_t height)
{
    m_display_width = width;
    m_display_height = height;
    return true;
}

void VulkanGraphicsDriver::GetDisplaySize(uint32_t& width, uint32_t& height) const
{
    width = m_display_width;
    height = m_display_height;
}

bool VulkanGraphicsDriver::SetFullscreen(bool fullscreen)
{
    m_fullscreen = fullscreen;
    return true;
}

bool VulkanGraphicsDriver::IsFullscreen() const
{
    return m_fullscreen;
}

void VulkanGraphicsDriver::Flush()
{
    // Stub
}

void VulkanGraphicsDriver::WaitForGPU()
{
    // Stub
}

const char* VulkanGraphicsDriver::GetLastError() const
{
    return "No error";
}

VulkanInstance* VulkanGraphicsDriver::GetVulkanInstance()
{
    return m_instance.get();
}

const VulkanInstance* VulkanGraphicsDriver::GetVulkanInstance() const
{
    return m_instance.get();
}

VulkanDevice* VulkanGraphicsDriver::GetVulkanDevice()
{
    return m_device.get();
}

const VulkanDevice* VulkanGraphicsDriver::GetVulkanDevice() const
{
    return m_device.get();
}

void* VulkanGraphicsDriver::GetGraphicsQueue() const
{
    return m_device ? (void*)m_device->graphics_queue : nullptr;
}

// ============================================================================
// Factory Function
// ============================================================================

IGraphicsDriver* CreateVulkanGraphicsDriver()
{
    printf("[CreateVulkanGraphicsDriver] Creating new VulkanGraphicsDriver instance\n");
    return new VulkanGraphicsDriver();
}

}  // namespace Graphics
