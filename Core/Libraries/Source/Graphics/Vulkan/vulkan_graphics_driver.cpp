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
 * Phase 54: Frame Rendering Implementation - BeginFrame/EndFrame/Present with actual Vulkan rendering
 *
 * Stub implementation that adapts vulkan_graphics_driver_legacy.cpp
 * (converted from VulkanGraphicsBackend static methods to instance-based IGraphicsDriver)
 *
 * Phase 54 implements:
 * - Synchronization objects (semaphores, fences)
 * - Command buffer recording with render pass
 * - Graphics pipeline with embedded SPIR-V shaders
 * - Proper frame cycling and presentation
 */

#include "vulkan_graphics_driver.h"
#include "vulkan_embedded_shaders.h"
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>  // Phase 56: FVF cache
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace Graphics {

// ============================================================================
// Phase 54: Frame Synchronization Objects
// ============================================================================

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

// Frame synchronization - one set per frame in flight
static std::vector<VkSemaphore> g_imageAvailableSemaphores;
static std::vector<VkSemaphore> g_renderFinishedSemaphores;
static std::vector<VkFence> g_inFlightFences;

// Command buffers for frame rendering
static VkCommandPool g_commandPool = VK_NULL_HANDLE;
static std::vector<VkCommandBuffer> g_commandBuffers;

// Graphics pipeline
static VkPipelineLayout g_pipelineLayout = VK_NULL_HANDLE;
static VkPipeline g_graphicsPipeline = VK_NULL_HANDLE;

// Shader modules (created from embedded SPIR-V)
static VkShaderModule g_vertShaderModule = VK_NULL_HANDLE;
static VkShaderModule g_fragShaderModule = VK_NULL_HANDLE;

// ============================================================================
// Phase 57: Texture Binding and Sampling Infrastructure
// ============================================================================

// Descriptor pool for texture bindings
static VkDescriptorPool g_descriptorPool = VK_NULL_HANDLE;
static VkDescriptorSetLayout g_textureDescriptorSetLayout = VK_NULL_HANDLE;

// Default sampler (created once, used for all textures by default)
static VkSampler g_defaultSampler = VK_NULL_HANDLE;

// Maximum texture stages supported
static constexpr uint32_t MAX_TEXTURE_STAGES = 8;

// Currently bound textures per stage
static TextureHandle g_boundTextures[MAX_TEXTURE_STAGES] = {
    INVALID_HANDLE, INVALID_HANDLE, INVALID_HANDLE, INVALID_HANDLE,
    INVALID_HANDLE, INVALID_HANDLE, INVALID_HANDLE, INVALID_HANDLE
};

// Texture dirty flag - indicates descriptor sets need updating
static bool g_textureDirty = false;

// ============================================================================
// Phase 58: Transform Matrices and Camera System
// ============================================================================

/**
 * Transform matrix storage
 * DirectX uses row-major matrices, Vulkan expects column-major in shaders.
 * We store row-major and transpose when uploading to push constants.
 */
static Matrix4x4 g_worldMatrix;        // Object → World space (model matrix)
static Matrix4x4 g_viewMatrix;         // World → Camera space
static Matrix4x4 g_projectionMatrix;   // Camera → Clip space
static Matrix4x4 g_mvpMatrix;          // Combined MVP for shader

// Transform dirty flag - recalculate MVP when any matrix changes
static bool g_transformDirty = true;

/**
 * Push constant structure for vertex shader
 * Total size: 64 bytes (mat4 MVP) + 4 bytes (flags) + padding
 * Must match shader push_constant block layout
 */
struct VertexPushConstants {
    float mvp[16];       // 64 bytes - Model-View-Projection matrix (column-major)
};

/**
 * Push constant structure for fragment shader
 * Separate range for fragment stage
 */
struct FragmentPushConstants {
    float color[4];      // 16 bytes - Solid color for fragment shader
};

// Combined push constant size
static constexpr uint32_t VERTEX_PUSH_CONSTANT_SIZE = sizeof(VertexPushConstants);   // 64 bytes
static constexpr uint32_t FRAGMENT_PUSH_CONSTANT_SIZE = sizeof(FragmentPushConstants); // 16 bytes
static constexpr uint32_t TOTAL_PUSH_CONSTANT_SIZE = VERTEX_PUSH_CONSTANT_SIZE + FRAGMENT_PUSH_CONSTANT_SIZE; // 80 bytes

/**
 * Matrix multiplication: result = a * b
 * Row-major order (DirectX convention)
 */
static void MultiplyMatrix4x4(const Matrix4x4& a, const Matrix4x4& b, Matrix4x4& result)
{
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.m[row][col] = 
                a.m[row][0] * b.m[0][col] +
                a.m[row][1] * b.m[1][col] +
                a.m[row][2] * b.m[2][col] +
                a.m[row][3] * b.m[3][col];
        }
    }
}

/**
 * Transpose matrix for Vulkan shader (column-major)
 * DirectX uses row-major, Vulkan shaders expect column-major
 */
static void TransposeMatrix4x4(const Matrix4x4& src, float* dst)
{
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            dst[col * 4 + row] = src.m[row][col];
        }
    }
}

/**
 * Apply Vulkan clip space correction to projection matrix
 * DirectX: Y up, Z [0,1]
 * Vulkan: Y down, Z [0,1]
 * We flip Y in the projection to match Vulkan's coordinate system
 */
static void ApplyVulkanClipCorrection(Matrix4x4& projection)
{
    // Flip Y axis for Vulkan's inverted Y coordinate
    projection.m[1][0] *= -1.0f;
    projection.m[1][1] *= -1.0f;
    projection.m[1][2] *= -1.0f;
    projection.m[1][3] *= -1.0f;
}

/**
 * Update combined MVP matrix from world, view, projection
 * MVP = Projection * View * World (right-to-left application)
 */
static void UpdateMVPMatrix()
{
    if (!g_transformDirty) {
        return;
    }
    
    // First: World * View
    Matrix4x4 worldView;
    MultiplyMatrix4x4(g_worldMatrix, g_viewMatrix, worldView);
    
    // Then: WorldView * Projection
    // Note: We need to apply Vulkan clip correction
    Matrix4x4 correctedProjection = g_projectionMatrix;
    ApplyVulkanClipCorrection(correctedProjection);
    
    MultiplyMatrix4x4(worldView, correctedProjection, g_mvpMatrix);
    
    g_transformDirty = false;
}

/**
 * Bind transform matrices to command buffer via push constants
 * Called before each draw call to ensure shader has current MVP
 */
static void BindTransformPushConstants(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout)
{
    // Ensure MVP is up to date
    UpdateMVPMatrix();
    
    // Prepare push constant data
    VertexPushConstants vertexPC;
    TransposeMatrix4x4(g_mvpMatrix, vertexPC.mvp);
    
    // Push to vertex shader (offset 0)
    vkCmdPushConstants(
        cmdBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(VertexPushConstants),
        &vertexPC
    );
}

// ============================================================================
// Phase 56: Pipeline Cache for Different FVF/Topology Combinations
// ============================================================================

/**
 * Key for pipeline cache - combines FVF and primitive topology
 */
struct PipelineCacheKey {
    uint32_t fvf;
    VkPrimitiveTopology topology;
    
    bool operator==(const PipelineCacheKey& other) const {
        return fvf == other.fvf && topology == other.topology;
    }
};

/**
 * Hash function for PipelineCacheKey
 */
struct PipelineCacheKeyHash {
    size_t operator()(const PipelineCacheKey& key) const {
        return std::hash<uint64_t>()(((uint64_t)key.fvf << 32) | key.topology);
    }
};

/**
 * Pipeline cache - stores created pipelines indexed by FVF + topology
 * This avoids recreating pipelines for the same vertex format + draw mode
 */
static std::unordered_map<PipelineCacheKey, VkPipeline, PipelineCacheKeyHash> g_pipeline_cache;

/**
 * Current pipeline state tracking
 */
static uint32_t g_current_fvf = 0;
static VkPrimitiveTopology g_current_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
static VkPipeline g_current_bound_pipeline = VK_NULL_HANDLE;

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
        fprintf(stderr, "[Vulkan] VulkanInstance::Create() - Starting instance creation (debug=%d)\n", debug_mode);
        fflush(stderr);
        
        // DEBUG: Platform detection
        fprintf(stderr, "[Vulkan] Platform detection:\n");
#ifdef __APPLE__
        fprintf(stderr, "[Vulkan]   __APPLE__ is DEFINED\n");
#else
        fprintf(stderr, "[Vulkan]   __APPLE__ is NOT defined\n");
#endif
        fflush(stderr);
        
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
        
#ifdef __APPLE__
        // macOS with MoltenVK requires portability enumeration
        required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        required_extensions.push_back("VK_EXT_metal_surface");
        fprintf(stderr, "[Vulkan] macOS: Added VK_KHR_PORTABILITY_ENUMERATION and VK_EXT_metal_surface extensions\n");
        fflush(stderr);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        required_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
        
        // DEBUG: Print all extensions requested
        fprintf(stderr, "[Vulkan] Requesting %zu extensions:\n", required_extensions.size());
        for (size_t i = 0; i < required_extensions.size(); i++) {
            fprintf(stderr, "[Vulkan]   [%zu]: %s\n", i, required_extensions[i]);
        }
        fflush(stderr);
        
        // Validation layers
        std::vector<const char*> validation_layers;
        if (debug_mode) {
            validation_layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        
        // Create instance info
        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
        create_info.ppEnabledExtensionNames = required_extensions.data();
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
        create_info.flags = 0;
        
#ifdef __APPLE__
        // macOS with MoltenVK requires portability enumeration flag
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        fprintf(stderr, "[Vulkan] macOS: Set VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR flag (flags=0x%x)\n", create_info.flags);
        fflush(stderr);
#endif
        
        // Create the Vulkan instance
        fprintf(stderr, "[Vulkan] Calling vkCreateInstance with flags=0x%x...\n", create_info.flags);
        fflush(stderr);
        VkResult result = vkCreateInstance(&create_info, nullptr, &handle);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "[Vulkan] ERROR: Failed to create Vulkan instance (result=%d)\n", result);
            fflush(stderr);
            return false;
        }
        
        fprintf(stderr, "[Vulkan] VulkanInstance::Create() - Success! Instance created.\n");
        fflush(stderr);
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
        // Note: MoltenVK on macOS does NOT support geometry shaders (Metal limitation)
        VkPhysicalDeviceFeatures device_features{};
        // Only request features that MoltenVK actually supports
        device_features.fillModeNonSolid = VK_TRUE;  // For wireframe rendering
        // Do NOT request geometryShader - unsupported by MoltenVK/Metal
        
        // Query what features are actually supported
        VkPhysicalDeviceFeatures supported_features{};
        vkGetPhysicalDeviceFeatures(physical_device, &supported_features);
        printf("[Vulkan] Supported features: fillModeNonSolid=%d, geometryShader=%d\n",
               supported_features.fillModeNonSolid, supported_features.geometryShader);
        
        // Only enable features that are actually supported
        if (!supported_features.fillModeNonSolid) {
            device_features.fillModeNonSolid = VK_FALSE;
            printf("[Vulkan] Warning: fillModeNonSolid not supported, disabling\n");
        }
        
        // Enable required device extensions for swapchain
        std::vector<const char*> device_extensions;
        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        
        // On macOS/MoltenVK, we may need VK_KHR_portability_subset
        #ifdef __APPLE__
        // Check if portability subset extension is available
        uint32_t ext_count = 0;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(ext_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ext_count, available_extensions.data());
        
        for (const auto& ext : available_extensions) {
            if (strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
                device_extensions.push_back("VK_KHR_portability_subset");
                printf("[Vulkan] Enabling VK_KHR_portability_subset extension\n");
                break;
            }
        }
        #endif
        
        printf("[Vulkan] Enabling %zu device extensions:\n", device_extensions.size());
        for (const auto& ext : device_extensions) {
            printf("[Vulkan]   - %s\n", ext);
        }
        
        // Create device
        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = 1;
        device_create_info.pQueueCreateInfos = &queue_create_info;
        device_create_info.pEnabledFeatures = &device_features;
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        device_create_info.ppEnabledExtensionNames = device_extensions.data();
        
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
// Phase 54: Helper functions for sync objects, command buffers, pipeline
// ============================================================================

static VkShaderModule CreateShaderModule(VkDevice device, const unsigned char* code, size_t size)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code);
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create shader module (result=%d)\n", result);
        return VK_NULL_HANDLE;
    }
    
    return shaderModule;
}

// ============================================================================
// VulkanGraphicsDriver Implementation
// ============================================================================

VulkanGraphicsDriver::VulkanGraphicsDriver()
    : m_initialized(false)
    , m_in_frame(false)
    , m_display_width(800)
    , m_display_height(600)
    , m_fullscreen(false)
    , m_current_frame(0)
    , m_current_image_index(0)
    , m_frame_started(false)
    , m_clear_color(0.0f, 0.2f, 0.4f, 1.0f)  // Dark blue to verify rendering works
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
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] width=%u height=%u fullscreen=%d\n", width, height, fullscreen);
    fflush(stderr);
    
    if (m_initialized) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Already initialized, returning\n");
        fflush(stderr);
        return true;
    }
    
    m_display_width = width;
    m_display_height = height;
    m_fullscreen = fullscreen;
    
    // Phase 41 Stage 1: Create Vulkan instance
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating Vulkan instance...\n");
    fflush(stderr);
    m_instance = std::make_unique<VulkanInstance>();
    if (!m_instance->Create(false)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create Vulkan instance\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 1: Select physical device
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Selecting physical device...\n");
    fflush(stderr);
    m_physical_device = std::make_unique<VulkanPhysicalDevice>();
    if (!m_physical_device->Select(m_instance->handle)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to select physical device\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 1: Create logical device
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating logical device...\n");
    fflush(stderr);
    m_device = std::make_unique<VulkanDevice>();
    if (!m_device->Create(m_physical_device->handle)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create logical device\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 2: Create swapchain
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating swapchain...\n");
    fflush(stderr);
    m_swapchain = std::make_unique<VulkanSwapchain>();
    if (!m_swapchain->Create(m_device->handle, m_physical_device->handle, m_instance->handle, 
                             windowHandle, width, height)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create swapchain\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 2: Create memory allocator
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating memory allocator...\n");
    fflush(stderr);
    m_memory_allocator = std::make_unique<VulkanMemoryAllocator>();
    if (!m_memory_allocator->Create(m_instance->handle, m_physical_device->handle, m_device->handle)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create memory allocator\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 2: Create render pass
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating render pass...\n");
    fflush(stderr);
    m_render_pass = std::make_unique<VulkanRenderPass>();
    if (!m_render_pass->Create(m_device->handle, VK_FORMAT_B8G8R8A8_UNORM)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create render pass\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 41 Stage 2.5: Create framebuffers for swapchain
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating framebuffers...\n");
    fflush(stderr);
    if (!m_swapchain->CreateFramebuffers(m_device->handle, VK_FORMAT_B8G8R8A8_UNORM, m_render_pass->handle)) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create framebuffers\n");
        fflush(stderr);
        return false;
    }
    m_swapchain->UpdateCurrentFramebuffer();
    
    // Phase 54: Create synchronization objects
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating synchronization objects...\n");
    fflush(stderr);
    if (!CreateSyncObjects()) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create sync objects\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 54: Create command buffers
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating command buffers for rendering...\n");
    fflush(stderr);
    if (!CreateCommandBuffers()) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create command buffers\n");
        fflush(stderr);
        return false;
    }
    
    // Phase 54: Create graphics pipeline with embedded shaders
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] Creating graphics pipeline...\n");
    fflush(stderr);
    if (!CreateGraphicsPipeline()) {
        fprintf(stderr, "[VulkanGraphicsDriver::Initialize] ERROR: Failed to create graphics pipeline\n");
        fflush(stderr);
        return false;
    }
    
    fprintf(stderr, "[VulkanGraphicsDriver::Initialize] SUCCESS - Vulkan initialized with full rendering infrastructure\n");
    fflush(stderr);
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
    
    // Phase 54: Destroy graphics pipeline and shaders
    DestroyGraphicsPipeline();
    
    // Phase 54: Destroy command buffers
    DestroyCommandBuffers();
    
    // Phase 54: Destroy sync objects
    DestroySyncObjects();
    
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
    static uint32_t s_frame_count = 0;
    
    if (!m_initialized || !m_device || !m_swapchain || !m_render_pass) {
        printf("[Vulkan] BeginFrame: Not properly initialized\n");
        return false;
    }
    
    if (m_frame_started) {
        // Already started - just continue
        return true;
    }
    
    // Wait for the previous frame using this slot to complete
    vkWaitForFences(m_device->handle, 1, &g_inFlightFences[m_current_frame], VK_TRUE, UINT64_MAX);
    
    // Acquire next image from swapchain
    VkResult result = vkAcquireNextImageKHR(
        m_device->handle,
        m_swapchain->handle,
        UINT64_MAX,
        g_imageAvailableSemaphores[m_current_frame],
        VK_NULL_HANDLE,
        &m_current_image_index
    );
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        printf("[Vulkan] BeginFrame: Swapchain out of date, needs recreation\n");
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("[Vulkan] BeginFrame: Failed to acquire swapchain image (result=%d)\n", result);
        return false;
    }
    
    // Reset the fence only after we know we're submitting work
    vkResetFences(m_device->handle, 1, &g_inFlightFences[m_current_frame]);
    
    // Reset and begin command buffer recording
    VkCommandBuffer cmdBuffer = g_commandBuffers[m_current_frame];
    vkResetCommandBuffer(cmdBuffer, 0);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    result = vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] BeginFrame: Failed to begin command buffer (result=%d)\n", result);
        return false;
    }
    
    // Begin render pass with clear color
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_render_pass->handle;
    renderPassInfo.framebuffer = m_swapchain->framebuffers[m_current_image_index];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapchain->extent;
    
    // Clear to a visible color (dark blue) to verify rendering works
    VkClearValue clearColor = {{{m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain->extent.width);
    viewport.height = static_cast<float>(m_swapchain->extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapchain->extent;
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    m_in_frame = true;
    m_frame_started = true;
    
    // Only log every 100 frames to reduce noise
    if (s_frame_count % 100 == 0) {
        printf("[Vulkan] BeginFrame - Frame %u started (clear=%.2f,%.2f,%.2f)\n", 
               s_frame_count, m_clear_color.r, m_clear_color.g, m_clear_color.b);
    }
    s_frame_count++;
    
    return true;
}

void VulkanGraphicsDriver::EndFrame()
{
    if (!m_frame_started || !m_in_frame) {
        return;
    }
    
    VkCommandBuffer cmdBuffer = g_commandBuffers[m_current_frame];
    
    // End render pass
    vkCmdEndRenderPass(cmdBuffer);
    
    // End command buffer recording
    VkResult result = vkEndCommandBuffer(cmdBuffer);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] EndFrame: Failed to end command buffer (result=%d)\n", result);
        m_in_frame = false;
        m_frame_started = false;
        return;
    }
    
    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {g_imageAvailableSemaphores[m_current_frame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    
    VkSemaphore signalSemaphores[] = {g_renderFinishedSemaphores[m_current_frame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    result = vkQueueSubmit(m_device->graphics_queue, 1, &submitInfo, g_inFlightFences[m_current_frame]);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] EndFrame: Failed to submit command buffer (result=%d)\n", result);
    }
    
    m_in_frame = false;
}

bool VulkanGraphicsDriver::Present()
{
    static uint32_t s_present_count = 0;
    
    if (!m_initialized || !m_device || !m_swapchain || !m_render_pass) {
        printf("[Vulkan] Present: Not properly initialized\n");
        return false;
    }
    
    if (!m_frame_started) {
        // No frame to present - likely called without BeginFrame
        return true;
    }
    
    // Present the rendered image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    VkSemaphore waitSemaphores[] = {g_renderFinishedSemaphores[m_current_frame]};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphores;
    
    VkSwapchainKHR swapChains[] = {m_swapchain->handle};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_current_image_index;
    
    VkResult result = vkQueuePresentKHR(m_device->graphics_queue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain needs recreation - not an error
    } else if (result != VK_SUCCESS) {
        printf("[Vulkan] Present: Failed to present (result=%d)\n", result);
        m_frame_started = false;
        return false;
    }
    
    // Only log every 100 frames
    if (s_present_count % 100 == 0) {
        printf("[Vulkan] Present - Frame %u presented\n", s_present_count);
    }
    s_present_count++;
    
    // Advance to next frame
    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    m_frame_started = false;
    
    return true;
}

void VulkanGraphicsDriver::Clear(float r, float g, float b, float a, bool clearDepth)
{
    if (!m_initialized) {
        printf("[Vulkan] Clear: Not initialized\n");
        return;
    }
    
    // Store clear color for use in BeginFrame
    m_clear_color = Color(r, g, b, a);
    
    // If we're already in a frame, issue a clear attachment command
    if (m_in_frame && m_frame_started) {
        VkCommandBuffer cmdBuffer = g_commandBuffers[m_current_frame];
        
        VkClearAttachment clearAttachment{};
        clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clearAttachment.colorAttachment = 0;
        clearAttachment.clearValue.color = {{r, g, b, a}};
        
        VkClearRect clearRect{};
        clearRect.rect.offset = {0, 0};
        clearRect.rect.extent = m_swapchain->extent;
        clearRect.baseArrayLayer = 0;
        clearRect.layerCount = 1;
        
        vkCmdClearAttachments(cmdBuffer, 1, &clearAttachment, 1, &clearRect);
        
        printf("[Vulkan] Clear - Cleared to (%.2f, %.2f, %.2f, %.2f)\n", r, g, b, a);
    } else {
        printf("[Vulkan] Clear - Set clear color (%.2f, %.2f, %.2f, %.2f) for next frame\n", r, g, b, a);
    }
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
    // Staging buffer for static buffer updates (Phase 51)
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;
    void* staging_mapped_ptr = nullptr;
    uint32_t staging_lock_offset = 0;
    uint32_t staging_lock_size = 0;
    bool using_staging = false;
};

struct VulkanTextureAllocation {
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    void* mappedStagingPtr = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::A8R8G8B8;
    bool cubeMap = false;
    bool renderTarget = false;
    bool depthStencil = false;
    bool dynamic = false;
};

// Storage for created buffers (simple map)
static std::vector<VulkanBufferAllocation> g_vertex_buffers;
static std::vector<VulkanBufferAllocation> g_index_buffers;
static std::vector<VulkanTextureAllocation> g_textures;

// ============================================================================
// Phase 56: Current Buffer Tracking
// ============================================================================
static VertexBufferHandle g_current_vertex_buffer = INVALID_HANDLE;
static IndexBufferHandle g_current_index_buffer = INVALID_HANDLE;
static uint32_t g_current_vertex_offset = 0;
static uint32_t g_current_vertex_stride = 0;
static uint32_t g_current_index_start = 0;  // Phase 56: Starting index offset
static VertexFormatHandle g_current_vertex_format = INVALID_HANDLE;
static bool g_current_index_is_32bit = false;

void VulkanGraphicsDriver::SetClearColor(float r, float g, float b, float a)
{
    m_clear_color = Color(r, g, b, a);
}

/**
 * Convert PrimitiveType (backend-agnostic) to VkPrimitiveTopology (Vulkan-specific)
 */
static VkPrimitiveTopology PrimitiveTypeToVkTopology(PrimitiveType primType)
{
    switch (primType) {
        case PrimitiveType::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveType::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveType::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveType::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveType::QuadList:
            // Vulkan doesn't have native quad support - convert to triangle list
            // (2 triangles per quad)
            printf("[Vulkan] WARNING: QuadList converted to TriangleList (2 tris per quad)\n");
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::QuadStrip:
            // Convert to triangle strip
            printf("[Vulkan] WARNING: QuadStrip converted to TriangleStrip\n");
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveType::TrianglePatch:
        case PrimitiveType::RectPatch:
        case PrimitiveType::TriNPatch:
            // Tessellation patches require different handling
            printf("[Vulkan] WARNING: Tessellation patch types not yet supported, using triangle list\n");
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:
            printf("[Vulkan] ERROR: Unknown primitive type %d\n", (int)primType);
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

/**
 * Calculate vertex count from primitive type and primitive count
 */
static uint32_t CalculateVertexCount(PrimitiveType primType, uint32_t primCount)
{
    switch (primType) {
        case PrimitiveType::PointList:
            return primCount;
        case PrimitiveType::LineList:
            return primCount * 2;
        case PrimitiveType::LineStrip:
            return primCount + 1;
        case PrimitiveType::TriangleList:
            return primCount * 3;
        case PrimitiveType::TriangleStrip:
        case PrimitiveType::TriangleFan:
            return primCount + 2;
        case PrimitiveType::QuadList:
            return primCount * 4;
        case PrimitiveType::QuadStrip:
            return primCount * 2 + 2;
        case PrimitiveType::TrianglePatch:
        case PrimitiveType::RectPatch:
        case PrimitiveType::TriNPatch:
            return primCount;  // Patch count varies
        default:
            return 0;
    }
}

// ============================================================================
// Phase 56: FVF to Vulkan Vertex Format Conversion
// ============================================================================

/**
 * FVF descriptor storing calculated vertex format information
 */
struct FVFDescriptor {
    uint32_t fvf;
    uint32_t stride;
    std::vector<VkVertexInputAttributeDescription> attributes;
    VkVertexInputBindingDescription binding;
};

/**
 * D3D FVF constants (for reference, defined in d3dx8_vulkan_fvf_compat.h)
 * D3DFVF_XYZ      = 0x0002  Position xyz
 * D3DFVF_XYZRHW   = 0x0004  Pre-transformed position
 * D3DFVF_NORMAL   = 0x0010  Normal vector
 * D3DFVF_PSIZE    = 0x0020  Point size
 * D3DFVF_DIFFUSE  = 0x0040  Diffuse color
 * D3DFVF_SPECULAR = 0x0080  Specular color
 * D3DFVF_TEX1-8   = 0x0100-0x0800  Texture coordinate count
 */

/**
 * Convert DirectX FVF (Flexible Vertex Format) to Vulkan vertex input descriptions
 * 
 * @param fvf D3D8 FVF bitfield
 * @return FVFDescriptor with Vulkan-compatible vertex attribute descriptions
 */
static FVFDescriptor ConvertFVFToVulkan(uint32_t fvf) {
    FVFDescriptor desc;
    desc.fvf = fvf;
    desc.stride = 0;
    
    uint32_t location = 0;
    uint32_t offset = 0;
    
    // Position (XYZ or XYZRHW)
    if (fvf & 0x0002) {  // D3DFVF_XYZ
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 12;  // 3 floats
    } else if (fvf & 0x0004) {  // D3DFVF_XYZRHW
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;  // vec4
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 16;  // 4 floats
    }
    
    // Normal
    if (fvf & 0x0010) {  // D3DFVF_NORMAL
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_R32G32B32_SFLOAT;  // vec3
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 12;  // 3 floats
    }
    
    // Point size
    if (fvf & 0x0020) {  // D3DFVF_PSIZE
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_R32_SFLOAT;  // float
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 4;  // 1 float
    }
    
    // Diffuse color (D3DCOLOR = BGRA 32-bit)
    if (fvf & 0x0040) {  // D3DFVF_DIFFUSE
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_B8G8R8A8_UNORM;  // D3DCOLOR is BGRA
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 4;  // 1 DWORD
    }
    
    // Specular color
    if (fvf & 0x0080) {  // D3DFVF_SPECULAR
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_B8G8R8A8_UNORM;  // D3DCOLOR is BGRA
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 4;  // 1 DWORD
    }
    
    // Texture coordinates (TEX0-8 encoded in bits 8-11)
    uint32_t texCount = (fvf & 0x0F00) >> 8;  // Extract texture coordinate count
    for (uint32_t i = 0; i < texCount; i++) {
        VkVertexInputAttributeDescription attr = {};
        attr.location = location++;
        attr.binding = 0;
        attr.format = VK_FORMAT_R32G32_SFLOAT;  // vec2 (2D texture coordinates)
        attr.offset = offset;
        desc.attributes.push_back(attr);
        offset += 8;  // 2 floats
    }
    
    desc.stride = offset;
    
    // Setup binding description
    desc.binding.binding = 0;
    desc.binding.stride = desc.stride;
    desc.binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    printf("[Vulkan] ConvertFVFToVulkan: fvf=0x%08X stride=%u attributes=%zu\n",
           fvf, desc.stride, desc.attributes.size());
    
    return desc;
}

// Static FVF descriptor cache to avoid repeated conversions
static std::unordered_map<uint32_t, FVFDescriptor> g_fvf_cache;

/**
 * Get FVF descriptor from cache, or create and cache it
 */
static FVFDescriptor& GetCachedFVFDescriptor(uint32_t fvf) {
    auto it = g_fvf_cache.find(fvf);
    if (it != g_fvf_cache.end()) {
        return it->second;
    }
    
    // Create new descriptor and cache it
    g_fvf_cache[fvf] = ConvertFVFToVulkan(fvf);
    return g_fvf_cache[fvf];
}

/**
 * Phase 56: Get or create pipeline for FVF + topology combination
 * 
 * NOTE: Currently returns the default pipeline since we don't have 
 * shaders that support vertex input yet. This will be expanded in 
 * a future phase when proper geometry shaders are implemented.
 * 
 * @param fvf Flexible Vertex Format flags
 * @param topology Vulkan primitive topology
 * @return VkPipeline handle (default pipeline for now)
 */
static VkPipeline GetOrCreatePipeline([[maybe_unused]] uint32_t fvf, 
                                      [[maybe_unused]] VkPrimitiveTopology topology) {
    // For Phase 56, we use the default fullscreen pipeline
    // Future phases will create FVF-specific pipelines with vertex input
    
    // Track current state for future optimization
    g_current_fvf = fvf;
    g_current_topology = topology;
    
    // Return default pipeline (fullscreen shader)
    return g_graphicsPipeline;
}

/**
 * Phase 56: Cleanup pipeline cache
 */
static void CleanupPipelineCache(VkDevice device) {
    // Destroy all cached pipelines (except default)
    for (auto& pair : g_pipeline_cache) {
        if (pair.second != VK_NULL_HANDLE && pair.second != g_graphicsPipeline) {
            vkDestroyPipeline(device, pair.second, nullptr);
        }
    }
    g_pipeline_cache.clear();
    printf("[Vulkan] CleanupPipelineCache: Pipeline cache cleared\n");
}

/**
 * Phase 57: Bind texture descriptor sets before draw calls
 * Returns true if at least one texture is bound and valid
 */
static bool BindTextureDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout) {
    // Check if we have textures bound to stage 0 (primary texture)
    if (g_boundTextures[0] == INVALID_HANDLE) {
        return false;
    }
    
    TextureHandle texHandle = g_boundTextures[0];
    if (texHandle >= g_textures.size()) {
        return false;
    }
    
    const VulkanTextureAllocation& tex = g_textures[texHandle];
    if (tex.descriptorSet == VK_NULL_HANDLE) {
        return false;
    }
    
    // Bind descriptor set for texture sampling
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &tex.descriptorSet, 0, nullptr);
    
    return true;
}

void VulkanGraphicsDriver::DrawPrimitive(PrimitiveType primType, uint32_t vertexCount)
{
    if (!m_initialized || !m_device) {
        printf("[Vulkan] DrawPrimitive: Driver not initialized\n");
        return;
    }
    
    if (!m_frame_started || !m_in_frame) {
        printf("[Vulkan] DrawPrimitive: Not in frame - call BeginFrame first\n");
        return;
    }
    
    // Check if we have a valid vertex buffer bound
    if (g_current_vertex_buffer == INVALID_HANDLE) {
        printf("[Vulkan] DrawPrimitive: No vertex buffer bound\n");
        return;
    }
    
    // Get the current command buffer
    VkCommandBuffer cmdBuffer = g_commandBuffers[m_current_frame];
    
    // Bind the graphics pipeline
    if (g_graphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline);
    } else {
        printf("[Vulkan] DrawPrimitive: No graphics pipeline available\n");
        return;
    }
    
    // Phase 57: Bind texture descriptor sets if available
    if (g_pipelineLayout != VK_NULL_HANDLE) {
        BindTextureDescriptorSets(cmdBuffer, g_pipelineLayout);
    }
    
    // Phase 58: Bind transform matrices (MVP) via push constants
    if (g_pipelineLayout != VK_NULL_HANDLE) {
        BindTransformPushConstants(cmdBuffer, g_pipelineLayout);
    }
    
    // Get vertex buffer (handle is 1-based, array is 0-based)
    if (g_current_vertex_buffer > (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] DrawPrimitive: Invalid vertex buffer handle %llu\n", g_current_vertex_buffer);
        return;
    }
    
    VulkanBufferAllocation& vbAlloc = g_vertex_buffers[g_current_vertex_buffer - 1];
    if (vbAlloc.buffer == VK_NULL_HANDLE) {
        printf("[Vulkan] DrawPrimitive: Vertex buffer not allocated\n");
        return;
    }
    
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {vbAlloc.buffer};
    VkDeviceSize offsets[] = {g_current_vertex_offset};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    
    // Draw non-indexed
    vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);
    
    // Reduce logging noise - only log occasionally
    static uint32_t s_draw_call_count = 0;
    if (++s_draw_call_count % 1000 == 0) {
        printf("[Vulkan] DrawPrimitive: %u draw calls executed\n", s_draw_call_count);
    }
}

void VulkanGraphicsDriver::DrawIndexedPrimitive(PrimitiveType primType, uint32_t indexCount,
                                               IndexBufferHandle ibHandle, uint32_t startIndex)
{
    if (!m_initialized || !m_device) {
        printf("[Vulkan] DrawIndexedPrimitive: Driver not initialized\n");
        return;
    }
    
    if (!m_frame_started || !m_in_frame) {
        printf("[Vulkan] DrawIndexedPrimitive: Not in frame - call BeginFrame first\n");
        return;
    }
    
    // Determine which index buffer to use:
    // If ibHandle is INVALID_HANDLE, use the currently bound index buffer (g_current_index_buffer)
    IndexBufferHandle effectiveHandle = ibHandle;
    if (effectiveHandle == INVALID_HANDLE) {
        effectiveHandle = g_current_index_buffer;
    }
    
    // Validate index buffer handle
    if (effectiveHandle == INVALID_HANDLE || effectiveHandle > (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] DrawIndexedPrimitive: Invalid index buffer handle %llu\n", effectiveHandle);
        return;
    }
    
    // Check if we have a valid vertex buffer bound
    if (g_current_vertex_buffer == INVALID_HANDLE) {
        printf("[Vulkan] DrawIndexedPrimitive: No vertex buffer bound\n");
        return;
    }
    
    // Get the current command buffer
    VkCommandBuffer cmdBuffer = g_commandBuffers[m_current_frame];
    
    // Bind the graphics pipeline
    if (g_graphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline);
    } else {
        printf("[Vulkan] DrawIndexedPrimitive: No graphics pipeline available\n");
        return;
    }
    
    // Phase 57: Bind texture descriptor sets if available
    if (g_pipelineLayout != VK_NULL_HANDLE) {
        BindTextureDescriptorSets(cmdBuffer, g_pipelineLayout);
    }
    
    // Phase 58: Bind transform matrices (MVP) via push constants
    if (g_pipelineLayout != VK_NULL_HANDLE) {
        BindTransformPushConstants(cmdBuffer, g_pipelineLayout);
    }
    
    // Get vertex buffer (handle is 1-based, array is 0-based)
    if (g_current_vertex_buffer > (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] DrawIndexedPrimitive: Invalid vertex buffer handle %llu\n", g_current_vertex_buffer);
        return;
    }
    
    VulkanBufferAllocation& vbAlloc = g_vertex_buffers[g_current_vertex_buffer - 1];
    if (vbAlloc.buffer == VK_NULL_HANDLE) {
        printf("[Vulkan] DrawIndexedPrimitive: Vertex buffer not allocated\n");
        return;
    }
    
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {vbAlloc.buffer};
    VkDeviceSize offsets[] = {g_current_vertex_offset};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
    
    // Get index buffer (handle is 1-based, array is 0-based)
    VulkanBufferAllocation& ibAlloc = g_index_buffers[effectiveHandle - 1];
    if (ibAlloc.buffer == VK_NULL_HANDLE) {
        printf("[Vulkan] DrawIndexedPrimitive: Index buffer not allocated\n");
        return;
    }
    
    // Bind index buffer (assume 16-bit indices - TODO: track index type properly)
    VkIndexType indexType = g_current_index_is_32bit ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
    vkCmdBindIndexBuffer(cmdBuffer, ibAlloc.buffer, 0, indexType);
    
    // Use the effective start index (from parameter or currently bound buffer)
    uint32_t effectiveStartIndex = startIndex;
    if (ibHandle == INVALID_HANDLE && g_current_index_start > 0) {
        effectiveStartIndex = g_current_index_start;
    }
    
    // Draw indexed
    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, effectiveStartIndex, 0, 0);
    
    // Reduce logging noise - only log occasionally
    static uint32_t s_indexed_draw_call_count = 0;
    if (++s_indexed_draw_call_count % 1000 == 0) {
        printf("[Vulkan] DrawIndexedPrimitive: %u indexed draw calls executed\n", s_indexed_draw_call_count);
    }
}

void VulkanGraphicsDriver::DrawPrimitiveUP(PrimitiveType primType, uint32_t primCount,
                                          const void* vertexData, uint32_t vertexStride)
{
    if (!m_initialized || !m_device) {
        printf("[Vulkan] DrawPrimitiveUP: Driver not initialized\n");
        return;
    }
    
    if (!vertexData) {
        printf("[Vulkan] DrawPrimitiveUP: ERROR - NULL vertex data\n");
        return;
    }
    
    uint32_t vertexCount = CalculateVertexCount(primType, primCount);
    printf("[Vulkan] DrawPrimitiveUP: primType=%d primCount=%u vertexCount=%u stride=%u\n",
           (int)primType, primCount, vertexCount, vertexStride);
    
    // Convert primitive type to Vulkan topology
    VkPrimitiveTopology topology = PrimitiveTypeToVkTopology(primType);
    
    // TODO Phase 41 Week 2 Day 3: Implement user-provided vertex data drawing
    // - Create temporary vertex buffer from provided data
    // - Copy vertexData to GPU memory
    // - Bind temporary vertex buffer
    // - Record vkCmdDraw() command
    // - Track temporary buffers for cleanup after frame
    // - Handle stride for proper vertex layout
    // - Support various vertex formats
    
    // For now, log the operation
    printf("[Vulkan] DrawPrimitiveUP: topology=%u vertexCount=%u (implementation TBD)\n", topology, vertexCount);
}

void VulkanGraphicsDriver::DrawIndexedPrimitiveUP(PrimitiveType primType, uint32_t minVertexIndex,
                                                 uint32_t vertexCount, uint32_t primCount,
                                                 const void* indexData, const void* vertexData,
                                                 uint32_t vertexStride)
{
    if (!m_initialized || !m_device) {
        printf("[Vulkan] DrawIndexedPrimitiveUP: Driver not initialized\n");
        return;
    }
    
    if (!indexData || !vertexData) {
        printf("[Vulkan] DrawIndexedPrimitiveUP: ERROR - NULL data (indexData=%p vertexData=%p)\n",
               indexData, vertexData);
        return;
    }
    
    printf("[Vulkan] DrawIndexedPrimitiveUP: primType=%d primCount=%u vertexCount=%u stride=%u\n",
           (int)primType, primCount, vertexCount, vertexStride);
    
    // Convert primitive type to Vulkan topology
    VkPrimitiveTopology topology = PrimitiveTypeToVkTopology(primType);
    
    // TODO Phase 41 Week 2 Day 3: Implement user-provided vertex+index data drawing
    // - Create temporary vertex buffer from vertexData
    // - Create temporary index buffer from indexData
    // - Copy both to GPU memory
    // - Bind both buffers
    // - Determine index type (16-bit vs 32-bit)
    // - Record vkCmdDrawIndexed() command
    // - Track temporary buffers for cleanup
    // - Calculate correct index count from primCount
    // - Apply minVertexIndex offset to vertex buffer binding
    
    // For now, log the operation
    printf("[Vulkan] DrawIndexedPrimitiveUP: topology=%u indexCount=%u (implementation TBD)\n",
           topology, primCount * 3);  // Assuming triangle list
}

// ============================================================================
// Render State Conversion Helpers
// ============================================================================

/**
 * Convert BlendMode to VkBlendFactor for Vulkan blending
 */
static VkBlendFactor BlendModeToVkBlendFactor(BlendMode mode)
{
    switch (mode) {
        case BlendMode::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendMode::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendMode::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendMode::InvSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendMode::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendMode::InvSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendMode::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendMode::InvDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendMode::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendMode::InvDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendMode::SrcAlphaSat:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendMode::BlendFactor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendMode::InvBlendFactor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendMode::BothSrcAlpha:
        case BlendMode::BothInvSrcAlpha:
        case BlendMode::SrcColor1:
        case BlendMode::InvSrcColor1:
            printf("[Vulkan] WARNING: BlendMode %d not directly supported, using SrcAlpha\n", (int)mode);
            return VK_BLEND_FACTOR_SRC_ALPHA;
        default:
            printf("[Vulkan] ERROR: Unknown BlendMode %d\n", (int)mode);
            return VK_BLEND_FACTOR_ONE;
    }
}

/**
 * Convert ComparisonFunc to VkCompareOp for Vulkan depth/stencil comparisons
 */
static VkCompareOp ComparisonFuncToVkCompareOp(ComparisonFunc func)
{
    switch (func) {
        case ComparisonFunc::Never:
            return VK_COMPARE_OP_NEVER;
        case ComparisonFunc::Less:
            return VK_COMPARE_OP_LESS;
        case ComparisonFunc::Equal:
            return VK_COMPARE_OP_EQUAL;
        case ComparisonFunc::LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case ComparisonFunc::Greater:
            return VK_COMPARE_OP_GREATER;
        case ComparisonFunc::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case ComparisonFunc::GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ComparisonFunc::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            printf("[Vulkan] ERROR: Unknown ComparisonFunc %d\n", (int)func);
            return VK_COMPARE_OP_ALWAYS;
    }
}

/**
 * Convert StencilOp to VkStencilOp for Vulkan stencil operations
 */
static VkStencilOp StencilOpToVkStencilOp(StencilOp op)
{
    switch (op) {
        case StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case StencilOp::IncrSat:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecrSat:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        case StencilOp::Incr:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::Decr:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            printf("[Vulkan] ERROR: Unknown StencilOp %d\n", (int)op);
            return VK_STENCIL_OP_KEEP;
    }
}

/**
 * Convert CullMode to VkCullModeFlagBits for Vulkan rasterization
 */
static VkCullModeFlagBits CullModeToVkCullMode(CullMode mode)
{
    switch (mode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Clockwise:
            return VK_CULL_MODE_BACK_BIT;  // DirectX convention differs - map CW to back
        case CullMode::CounterClockwise:
            return VK_CULL_MODE_FRONT_BIT;
        default:
            printf("[Vulkan] ERROR: Unknown CullMode %d\n", (int)mode);
            return VK_CULL_MODE_BACK_BIT;
    }
}

/**
 * Convert FillMode to VkPolygonMode for Vulkan rasterization
 */
static VkPolygonMode FillModeToVkPolygonMode(FillMode mode)
{
    switch (mode) {
        case FillMode::Point:
            return VK_POLYGON_MODE_POINT;
        case FillMode::Wireframe:
            return VK_POLYGON_MODE_LINE;
        case FillMode::Solid:
            return VK_POLYGON_MODE_FILL;
        default:
            printf("[Vulkan] ERROR: Unknown FillMode %d\n", (int)mode);
            return VK_POLYGON_MODE_FILL;
    }
}

bool VulkanGraphicsDriver::SetRenderState(RenderState state, uint64_t value)
{
    // Initialize cache if needed
    if (m_render_state_cache.empty()) {
        m_render_state_cache.resize(static_cast<size_t>(RenderState::SliceCount) + 1, 0);
    }
    
    // Cache the state value for GetRenderState() retrieval
    size_t index = static_cast<size_t>(state);
    if (index <= static_cast<size_t>(RenderState::SliceCount)) {
        m_render_state_cache[index] = value;
    }
    
    // Handle specific render states that affect Vulkan pipeline
    // Note: Some states (like Lighting, Fog, etc.) are shader-related and handled in future phases
    // Phase 41 focuses on core graphics states (blend, depth/stencil, rasterizer)
    
    switch (state) {
        case RenderState::Lighting:
            // TODO: Will be handled in lighting phase (convert to shader uniform)
            printf("[Vulkan] SetRenderState: Lighting=%llu (lighting pass TBD)\n", value);
            break;
            
        case RenderState::FogEnable:
            // TODO: Will be handled in fog phase (shader uniform)
            printf("[Vulkan] SetRenderState: FogEnable=%llu (fog pass TBD)\n", value);
            break;
            
        case RenderState::AlphaBlendEnable:
            // TODO: Integrate with SetBlendState() - for now cache only
            printf("[Vulkan] SetRenderState: AlphaBlendEnable=%llu (blend state TBD)\n", value);
            break;
            
        case RenderState::SrcBlend:
        case RenderState::DstBlend:
        case RenderState::SrcBlendAlpha:
        case RenderState::DstBlendAlpha:
            // TODO: These are handled via SetBlendState() descriptor
            printf("[Vulkan] SetRenderState: BlendMode (blend state TBD)\n");
            break;
            
        case RenderState::ZEnable:
            // TODO: Integrate with SetDepthStencilState()
            printf("[Vulkan] SetRenderState: ZEnable=%llu (depth state TBD)\n", value);
            break;
            
        case RenderState::ZWriteEnable:
            // TODO: Integrate with SetDepthStencilState()
            printf("[Vulkan] SetRenderState: ZWriteEnable=%llu (depth state TBD)\n", value);
            break;
            
        case RenderState::ZFunc:
            // TODO: Integrate with SetDepthStencilState()
            printf("[Vulkan] SetRenderState: ZFunc=%llu (depth state TBD)\n", value);
            break;
            
        case RenderState::CullMode:
            // TODO: Integrate with SetRasterizerState()
            printf("[Vulkan] SetRenderState: CullMode=%llu (raster state TBD)\n", value);
            break;
            
        case RenderState::FillMode:
            // TODO: Integrate with SetRasterizerState()
            printf("[Vulkan] SetRenderState: FillMode=%llu (raster state TBD)\n", value);
            break;
            
        case RenderState::ScissorTestEnable:
            // TODO: Integrate with SetScissorRect()
            printf("[Vulkan] SetRenderState: ScissorTestEnable=%llu (scissor TBD)\n", value);
            break;
            
        case RenderState::Stencil:
        case RenderState::StencilFunc:
        case RenderState::StencilRef:
        case RenderState::StencilMask:
        case RenderState::StencilFail:
        case RenderState::StencilZFail:
        case RenderState::StencilPass:
            // TODO: Handled via SetDepthStencilState()
            printf("[Vulkan] SetRenderState: StencilOp (stencil state TBD)\n");
            break;
            
        case RenderState::Ambient:
        case RenderState::Specular:
        case RenderState::AlphaFunc:
        case RenderState::AlphaRef:
        case RenderState::TextureFactor:
        case RenderState::FogStart:
        case RenderState::FogEnd:
        case RenderState::FogDensity:
        case RenderState::FogColor:
        case RenderState::ColorWriteEnable:
        case RenderState::PointSize:
        case RenderState::BlendFactor:
        case RenderState::BlendOp:
        case RenderState::BlendOpAlpha:
        case RenderState::MultisampleAntialias:
        case RenderState::NormalizeNormals:
        case RenderState::ClipPlaneEnable:
        case RenderState::DitherEnable:
        case RenderState::SeparateAlphaBlendEnable:
        case RenderState::PointScaleEnable:
        case RenderState::PointSizeMin:
        case RenderState::PointSizeMax:
        case RenderState::PointScaleA:
        case RenderState::PointScaleB:
        case RenderState::PointScaleC:
        case RenderState::MultisampleType:
        case RenderState::DebugMonitorToken:
        case RenderState::IndexedVertexBlendEnable:
        case RenderState::TweenFactor:
        case RenderState::PositionDegree:
        case RenderState::NormalDegree:
        case RenderState::SliceEnable:
        case RenderState::SliceCount:
            // These states are either shader-related or not directly used in Vulkan rendering
            printf("[Vulkan] SetRenderState: State %d = %llu (cached, not directly applied)\n", (int)state, value);
            break;
    }
    
    return true;
}

uint64_t VulkanGraphicsDriver::GetRenderState(RenderState state) const
{
    // Return cached render state value
    size_t index = static_cast<size_t>(state);
    
    if (m_render_state_cache.empty() || index > static_cast<size_t>(RenderState::SliceCount)) {
        printf("[Vulkan] GetRenderState: Invalid state index %zu\n", index);
        return 0;
    }
    
    return m_render_state_cache[index];
}

bool VulkanGraphicsDriver::SetBlendState(const BlendStateDescriptor& desc)
{
    // TODO: Create Vulkan pipeline for this blend state
    // For now, cache the blend state and log it
    
    printf("[Vulkan] SetBlendState: enabled=%d srcBlend=%d dstBlend=%d\n",
           desc.enabled, (int)desc.srcBlend, (int)desc.dstBlend);
    printf("[Vulkan] SetBlendState: srcBlendAlpha=%d dstBlendAlpha=%d\n",
           (int)desc.srcBlendAlpha, (int)desc.dstBlendAlpha);
    
    if (!desc.enabled) {
        printf("[Vulkan] SetBlendState: Blending disabled\n");
        // TODO: Create pipeline with blending disabled
        return true;
    }
    
    // Convert blend modes to Vulkan factors
    VkBlendFactor vk_src_blend = BlendModeToVkBlendFactor(desc.srcBlend);
    VkBlendFactor vk_dst_blend = BlendModeToVkBlendFactor(desc.dstBlend);
    VkBlendFactor vk_src_blend_alpha = BlendModeToVkBlendFactor(desc.srcBlendAlpha);
    VkBlendFactor vk_dst_blend_alpha = BlendModeToVkBlendFactor(desc.dstBlendAlpha);
    
    printf("[Vulkan] SetBlendState: Converted to VkBlendFactor (RGB: %u->%u, Alpha: %u->%u)\n",
           vk_src_blend, vk_dst_blend, vk_src_blend_alpha, vk_dst_blend_alpha);
    
    // TODO: Build VkPipelineColorBlendAttachmentState
    // VkPipelineColorBlendAttachmentState blend_attachment = {};
    // blend_attachment.blendEnable = VK_TRUE;
    // blend_attachment.srcColorBlendFactor = vk_src_blend;
    // blend_attachment.dstColorBlendFactor = vk_dst_blend;
    // blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    // blend_attachment.srcAlphaBlendFactor = vk_src_blend_alpha;
    // blend_attachment.dstAlphaBlendFactor = vk_dst_blend_alpha;
    // blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    // blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    //                                   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    // TODO: Create or retrieve cached Vulkan pipeline with this blend state
    // TODO: Bind pipeline in next DrawPrimitive() call
    
    printf("[Vulkan] SetBlendState: Blend state configured (pipeline binding TBD)\n");
    
    return true;
}

bool VulkanGraphicsDriver::SetDepthStencilState(const DepthStencilStateDescriptor& desc)
{
    printf("[Vulkan] SetDepthStencilState: depthEnable=%d depthWriteEnable=%d depthFunc=%d\n",
           desc.depthEnable, desc.depthWriteEnable, (int)desc.depthFunc);
    printf("[Vulkan] SetDepthStencilState: stencilEnable=%d\n", desc.stencilEnable);
    
    if (desc.depthEnable) {
        VkCompareOp vk_depth_func = ComparisonFuncToVkCompareOp(desc.depthFunc);
        printf("[Vulkan] SetDepthStencilState: Depth test enabled, func converted to %u\n", vk_depth_func);
    }
    
    if (desc.stencilEnable) {
        printf("[Vulkan] SetDepthStencilState: Stencil test enabled\n");
        
        // Convert stencil functions and operations
        VkCompareOp vk_front_func = ComparisonFuncToVkCompareOp(desc.frontStencilFunc);
        VkStencilOp vk_front_fail = StencilOpToVkStencilOp(desc.frontStencilFail);
        VkStencilOp vk_front_z_fail = StencilOpToVkStencilOp(desc.frontStencilZFail);
        VkStencilOp vk_front_pass = StencilOpToVkStencilOp(desc.frontStencilPass);
        
        printf("[Vulkan] SetDepthStencilState: Front stencil - func=%u fail=%u zfail=%u pass=%u\n",
               vk_front_func, vk_front_fail, vk_front_z_fail, vk_front_pass);
        
        VkCompareOp vk_back_func = ComparisonFuncToVkCompareOp(desc.backStencilFunc);
        VkStencilOp vk_back_fail = StencilOpToVkStencilOp(desc.backStencilFail);
        VkStencilOp vk_back_z_fail = StencilOpToVkStencilOp(desc.backStencilZFail);
        VkStencilOp vk_back_pass = StencilOpToVkStencilOp(desc.backStencilPass);
        
        printf("[Vulkan] SetDepthStencilState: Back stencil - func=%u fail=%u zfail=%u pass=%u\n",
               vk_back_func, vk_back_fail, vk_back_z_fail, vk_back_pass);
    }
    
    // TODO: Build VkPipelineDepthStencilStateCreateInfo
    // VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    // depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depth_stencil.depthTestEnable = desc.depthEnable ? VK_TRUE : VK_FALSE;
    // depth_stencil.depthWriteEnable = desc.depthWriteEnable ? VK_TRUE : VK_FALSE;
    // depth_stencil.depthCompareOp = ComparisonFuncToVkCompareOp(desc.depthFunc);
    // depth_stencil.stencilTestEnable = desc.stencilEnable ? VK_TRUE : VK_FALSE;
    // if (desc.stencilEnable) {
    //     depth_stencil.front.compareOp = ComparisonFuncToVkCompareOp(desc.front.function);
    //     depth_stencil.front.failOp = StencilOpToVkStencilOp(desc.front.failOp);
    //     depth_stencil.front.depthFailOp = StencilOpToVkStencilOp(desc.front.depthFailOp);
    //     depth_stencil.front.passOp = StencilOpToVkStencilOp(desc.front.passOp);
    //     // ... similar for back face
    // }
    
    // TODO: Create or retrieve cached Vulkan pipeline with this depth/stencil state
    // TODO: Bind pipeline in next DrawPrimitive() call
    
    printf("[Vulkan] SetDepthStencilState: Depth/Stencil state configured (pipeline binding TBD)\n");
    
    return true;
}

bool VulkanGraphicsDriver::SetRasterizerState(const RasterizerStateDescriptor& desc)
{
    printf("[Vulkan] SetRasterizerState: fillMode=%d cullMode=%d\n",
           (int)desc.fillMode, (int)desc.cullMode);
    printf("[Vulkan] SetRasterizerState: frontCounterClockwise=%d scissorEnable=%d\n",
           desc.frontCounterClockwise, desc.scissorEnable);
    
    // Convert fill mode and cull mode
    VkPolygonMode vk_fill_mode = FillModeToVkPolygonMode(desc.fillMode);
    VkCullModeFlagBits vk_cull_mode = CullModeToVkCullMode(desc.cullMode);
    
    printf("[Vulkan] SetRasterizerState: Converted to VkPolygonMode=%u VkCullMode=%u\n",
           vk_fill_mode, vk_cull_mode);
    
    // TODO: Build VkPipelineRasterizationStateCreateInfo
    // VkPipelineRasterizationStateCreateInfo rasterizer = {};
    // rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // rasterizer.polygonMode = vk_fill_mode;
    // rasterizer.cullMode = vk_cull_mode;
    // rasterizer.frontFace = desc.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    // rasterizer.depthBiasEnable = (desc.depthBias != 0 || desc.depthBiasClamp != 0.0f) ? VK_TRUE : VK_FALSE;
    // rasterizer.depthBiasConstantFactor = static_cast<float>(desc.depthBias);
    // rasterizer.depthBiasClamp = desc.depthBiasClamp;
    // rasterizer.depthBiasSlopeFactor = desc.slopeScaledDepthBias;
    // rasterizer.lineWidth = 1.0f;  // TODO: Support dynamic line width
    // rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // rasterizer.depthClampEnable = desc.depthClipEnable ? VK_FALSE : VK_TRUE;  // Note: logic inverted
    
    // TODO: Create or retrieve cached Vulkan pipeline with this rasterizer state
    // TODO: Bind pipeline in next DrawPrimitive() call
    
    printf("[Vulkan] SetRasterizerState: Rasterizer state configured (pipeline binding TBD)\n");
    
    return true;
}

bool VulkanGraphicsDriver::SetScissorRect(const Rect& rect)
{
    printf("[Vulkan] SetScissorRect: left=%d top=%d right=%d bottom=%d\n",
           rect.left, rect.top, rect.right, rect.bottom);
    
    // Validate scissor rect dimensions
    if (rect.left >= rect.right || rect.top >= rect.bottom) {
        printf("[Vulkan] ERROR: Invalid scissor rect (left >= right or top >= bottom)\n");
        return false;
    }
    
    // TODO: Create VkRect2D and record vkCmdSetScissor() in command buffer
    // VkRect2D scissor = {};
    // scissor.offset.x = rect.left;
    // scissor.offset.y = rect.top;
    // scissor.extent.width = rect.right - rect.left;
    // scissor.extent.height = rect.bottom - rect.top;
    // 
    // TODO: Record command in current command buffer:
    // vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);
    
    printf("[Vulkan] SetScissorRect: Scissor configured (vkCmd binding TBD)\n");
    
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
// Buffer Management Implementation (REAL Vulkan)
// ============================================================================

/**
 * Phase 51: Helper function to create a staging buffer for static buffer uploads
 * Creates a HOST_VISIBLE buffer that can be used for CPU->GPU transfers
 */
static bool CreateStagingBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                               uint32_t size, VkBuffer* outBuffer, VkDeviceMemory* outMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, outBuffer);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: CreateStagingBuffer - vkCreateBuffer failed (result=%d)\n", result);
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *outBuffer, &memRequirements);
    
    // Get memory properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    // Find HOST_VISIBLE and HOST_COHERENT memory type
    VkMemoryPropertyFlags required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t memTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & required) == required) {
            memTypeIndex = i;
            break;
        }
    }
    
    if (memTypeIndex == UINT32_MAX) {
        printf("[Vulkan] ERROR: CreateStagingBuffer - No suitable memory type found\n");
        vkDestroyBuffer(device, *outBuffer, nullptr);
        *outBuffer = VK_NULL_HANDLE;
        return false;
    }
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memTypeIndex;
    
    result = vkAllocateMemory(device, &allocInfo, nullptr, outMemory);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: CreateStagingBuffer - vkAllocateMemory failed (result=%d)\n", result);
        vkDestroyBuffer(device, *outBuffer, nullptr);
        *outBuffer = VK_NULL_HANDLE;
        return false;
    }
    
    result = vkBindBufferMemory(device, *outBuffer, *outMemory, 0);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] ERROR: CreateStagingBuffer - vkBindBufferMemory failed (result=%d)\n", result);
        vkFreeMemory(device, *outMemory, nullptr);
        vkDestroyBuffer(device, *outBuffer, nullptr);
        *outBuffer = VK_NULL_HANDLE;
        *outMemory = VK_NULL_HANDLE;
        return false;
    }
    
    printf("[Vulkan] CreateStagingBuffer: size=%u SUCCESS\n", size);
    return true;
}

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

/**
 * Convert TextureFormat (backend-agnostic) to VkFormat (Vulkan-specific)
 */
static VkFormat TextureFormatToVkFormat(TextureFormat format)
{
    switch (format) {
        case TextureFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::A8R8G8B8:
        case TextureFormat::X8R8G8B8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::R5G6B5:
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case TextureFormat::A1R5G5B5:
        case TextureFormat::X1R5G5B5:
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case TextureFormat::A4R4G4B4:
        case TextureFormat::X4R4G4B4:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case TextureFormat::A8:
            return VK_FORMAT_R8_UNORM;
        case TextureFormat::G16R16:
            return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::A16B16G16R16:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case TextureFormat::L8:
            return VK_FORMAT_R8_UNORM;
        case TextureFormat::A8L8:
            return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::V8U8:
            return VK_FORMAT_R8G8_SNORM;
        case TextureFormat::V16U16:
            return VK_FORMAT_R16G16_SNORM;
        case TextureFormat::DXT1:
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case TextureFormat::DXT2:
        case TextureFormat::DXT3:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::DXT4:
        case TextureFormat::DXT5:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        default:
            printf("[Vulkan] WARNING: Unknown texture format %d, using R8G8B8A8\n", (int)format);
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

/**
 * Calculate byte size per pixel for a given format
 */
static uint32_t GetPixelSize(TextureFormat format)
{
    switch (format) {
        case TextureFormat::R8G8B8:
            return 3;
        case TextureFormat::A8R8G8B8:
        case TextureFormat::X8R8G8B8:
        case TextureFormat::A8B8G8R8:
        case TextureFormat::X8B8G8R8:
            return 4;
        case TextureFormat::R5G6B5:
        case TextureFormat::A1R5G5B5:
        case TextureFormat::X1R5G5B5:
        case TextureFormat::A4R4G4B4:
        case TextureFormat::X4R4G4B4:
        case TextureFormat::A8L8:
        case TextureFormat::G16R16:
        case TextureFormat::V8U8:
        case TextureFormat::V16U16:
            return 2;
        case TextureFormat::A8:
        case TextureFormat::L8:
        case TextureFormat::R3G3B2:
            return 1;
        case TextureFormat::A16B16G16R16:
            return 8;
        case TextureFormat::DXT1:
            return 0;  // Compressed, not used
        case TextureFormat::DXT2:
        case TextureFormat::DXT3:
        case TextureFormat::DXT4:
        case TextureFormat::DXT5:
            return 0;  // Compressed, not used
        default:
            return 4;
    }
}

// ============================================================================
// Phase 57: Texture Helper Functions
// ============================================================================

/**
 * Check if a texture format is block-compressed (DXT/BC)
 */
static bool IsCompressedFormat(TextureFormat format)
{
    switch (format) {
        case TextureFormat::DXT1:
        case TextureFormat::DXT2:
        case TextureFormat::DXT3:
        case TextureFormat::DXT4:
        case TextureFormat::DXT5:
            return true;
        default:
            return false;
    }
}

/**
 * Get block size for compressed formats
 */
static uint32_t GetCompressedBlockSize(TextureFormat format)
{
    switch (format) {
        case TextureFormat::DXT1:
            return 8;  // BC1: 8 bytes per 4x4 block
        case TextureFormat::DXT2:
        case TextureFormat::DXT3:
        case TextureFormat::DXT4:
        case TextureFormat::DXT5:
            return 16; // BC2/BC3: 16 bytes per 4x4 block
        default:
            return 0;
    }
}

/**
 * Calculate texture data size in bytes
 */
static uint32_t CalculateTextureDataSize(uint32_t width, uint32_t height, TextureFormat format)
{
    if (IsCompressedFormat(format)) {
        // Compressed: blocks of 4x4 pixels
        uint32_t blocksX = (width + 3) / 4;
        uint32_t blocksY = (height + 3) / 4;
        return blocksX * blocksY * GetCompressedBlockSize(format);
    } else {
        return width * height * GetPixelSize(format);
    }
}

/**
 * Create the default texture sampler with common settings
 */
static VkSampler CreateDefaultSampler(VkDevice device)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    
    VkSampler sampler = VK_NULL_HANDLE;
    VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to create default sampler (result=%d)\n", result);
        return VK_NULL_HANDLE;
    }
    
    printf("[Vulkan] Phase 57: Created default sampler successfully\n");
    return sampler;
}

/**
 * Create descriptor set layout for texture binding
 */
static VkDescriptorSetLayout CreateTextureDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;
    
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to create descriptor set layout (result=%d)\n", result);
        return VK_NULL_HANDLE;
    }
    
    printf("[Vulkan] Phase 57: Created texture descriptor set layout\n");
    return layout;
}

/**
 * Create descriptor pool for texture bindings
 */
static VkDescriptorPool CreateTextureDescriptorPool(VkDevice device, uint32_t maxSets)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = maxSets;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = maxSets;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    
    VkDescriptorPool pool = VK_NULL_HANDLE;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to create descriptor pool (result=%d)\n", result);
        return VK_NULL_HANDLE;
    }
    
    printf("[Vulkan] Phase 57: Created descriptor pool with maxSets=%u\n", maxSets);
    return pool;
}

/**
 * Allocate descriptor set for a texture
 */
static VkDescriptorSet AllocateTextureDescriptorSet(VkDevice device, VkDescriptorPool pool, 
                                                     VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to allocate descriptor set (result=%d)\n", result);
        return VK_NULL_HANDLE;
    }
    
    return descriptorSet;
}

/**
 * Update descriptor set with texture image and sampler
 */
static void UpdateTextureDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet,
                                       VkImageView imageView, VkSampler sampler)
{
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;
    
    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

/**
 * Transition image layout using a command buffer
 */
static void TransitionImageLayout(VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                                  VkImage image, VkFormat format,
                                  VkImageLayout oldLayout, VkImageLayout newLayout)
{
    // Allocate temporary command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);
    
    // Begin recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    
    // Setup image memory barrier
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        printf("[Vulkan] Phase 57: WARNING - Unsupported layout transition\n");
        srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    
    vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
    
    // End and submit
    vkEndCommandBuffer(cmdBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    // Free temporary command buffer
    vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

/**
 * Copy buffer data to image
 */
static void CopyBufferToImage(VkDevice device, VkCommandPool cmdPool, VkQueue queue,
                              VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    // Allocate temporary command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);
    
    // Begin recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuffer, &beginInfo);
    
    // Setup copy region
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;   // Tightly packed
    region.bufferImageHeight = 0; // Tightly packed
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    // End and submit
    vkEndCommandBuffer(cmdBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    // Free temporary command buffer
    vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
}

// ============================================================================
// Vertex Buffer Implementation
// ============================================================================

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
    
    // Store buffer and return handle as 1-based index (0 is INVALID_HANDLE)
    g_vertex_buffers.push_back(allocation);
    VertexBufferHandle handle = (VertexBufferHandle)g_vertex_buffers.size();
    
    printf("[Vulkan] CreateVertexBuffer: SUCCESS handle=%llu\n", handle);
    return handle;
}

void VulkanGraphicsDriver::DestroyVertexBuffer(VertexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] DestroyVertexBuffer: Invalid handle %llu\n", handle);
        return;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle - 1];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
    }
    
    // Phase 51: Clean up staging buffer if present
    if (alloc.staging_mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.staging_memory);
    }
    if (alloc.staging_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->handle, alloc.staging_buffer, nullptr);
    }
    if (alloc.staging_memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.staging_memory, nullptr);
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
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_vertex_buffers.size() || !lockedData) {
        printf("[Vulkan] LockVertexBuffer: Invalid parameters (handle=%llu)\n", handle);
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle - 1];
    
    // Adjust size if 0 (meaning lock entire buffer)
    if (size == 0) {
        size = alloc.size - offset;
    }
    
    if (offset + size > alloc.size) {
        printf("[Vulkan] ERROR: Lock range exceeds buffer size\n");
        return false;
    }
    
    // Phase 51: For static buffers, use staging buffer approach
    if (!alloc.is_dynamic) {
        printf("[Vulkan] LockVertexBuffer: Static buffer - using staging buffer approach\n");
        
        // Create staging buffer if not already created
        if (alloc.staging_buffer == VK_NULL_HANDLE) {
            if (!CreateStagingBuffer(m_device->handle, m_physical_device->handle,
                                    alloc.size, &alloc.staging_buffer, &alloc.staging_memory)) {
                printf("[Vulkan] ERROR: Failed to create staging buffer for static vertex buffer\n");
                return false;
            }
        }
        
        // Map the staging buffer
        VkResult result = vkMapMemory(m_device->handle, alloc.staging_memory, 0, alloc.size, 0, &alloc.staging_mapped_ptr);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: vkMapMemory for staging buffer failed (result=%d)\n", result);
            return false;
        }
        
        alloc.staging_lock_offset = offset;
        alloc.staging_lock_size = size;
        alloc.using_staging = true;
        
        // Return pointer to the staging buffer at the correct offset
        *lockedData = (char*)alloc.staging_mapped_ptr + offset;
        printf("[Vulkan] LockVertexBuffer (static): handle=%llu offset=%u size=%u (staging)\n", handle, offset, size);
        return true;
    }
    
    // Dynamic buffer - direct map
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
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] UnlockVertexBuffer: Invalid handle %llu\n", handle);
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_vertex_buffers[handle - 1];
    
    // Phase 51: Handle staging buffer for static buffers
    if (alloc.using_staging && alloc.staging_mapped_ptr) {
        // Unmap staging buffer
        vkUnmapMemory(m_device->handle, alloc.staging_memory);
        alloc.staging_mapped_ptr = nullptr;
        
        // Copy from staging to device-local buffer via command buffer
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
        
        // Create a temporary command pool and buffer for the transfer
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_device->graphics_queue_family;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        
        VkCommandPool cmdPool = VK_NULL_HANDLE;
        VkResult result = vkCreateCommandPool(m_device->handle, &poolInfo, nullptr, &cmdPool);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: UnlockVertexBuffer - Failed to create command pool\n");
            alloc.using_staging = false;
            return false;
        }
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cmdPool;
        allocInfo.commandBufferCount = 1;
        
        result = vkAllocateCommandBuffers(m_device->handle, &allocInfo, &cmdBuffer);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: UnlockVertexBuffer - Failed to allocate command buffer\n");
            vkDestroyCommandPool(m_device->handle, cmdPool, nullptr);
            alloc.using_staging = false;
            return false;
        }
        
        // Begin recording
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        
        // Copy buffer
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = alloc.staging_lock_offset;
        copyRegion.dstOffset = alloc.staging_lock_offset;
        copyRegion.size = alloc.staging_lock_size;
        
        vkCmdCopyBuffer(cmdBuffer, alloc.staging_buffer, alloc.buffer, 1, &copyRegion);
        
        vkEndCommandBuffer(cmdBuffer);
        
        // Submit and wait
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        
        vkQueueSubmit(m_device->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->graphics_queue);
        
        // Cleanup
        vkFreeCommandBuffers(m_device->handle, cmdPool, 1, &cmdBuffer);
        vkDestroyCommandPool(m_device->handle, cmdPool, nullptr);
        
        alloc.using_staging = false;
        printf("[Vulkan] UnlockVertexBuffer (static): handle=%llu - staging copy complete\n", handle);
        return true;
    }
    
    // Dynamic buffer - direct unmap
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
    if (handle == INVALID_HANDLE || handle > (uint64_t)g_vertex_buffers.size()) {
        return 0;
    }
    
    return g_vertex_buffers[handle - 1].size;
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
    
    // Store buffer and return handle as 1-based index (0 is INVALID_HANDLE)
    g_index_buffers.push_back(allocation);
    IndexBufferHandle handle = (IndexBufferHandle)g_index_buffers.size();
    
    printf("[Vulkan] CreateIndexBuffer: SUCCESS handle=%llu\n", handle);
    return handle;
}

void VulkanGraphicsDriver::DestroyIndexBuffer(IndexBufferHandle handle)
{
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] DestroyIndexBuffer: Invalid handle %llu\n", handle);
        return;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle - 1];
    
    if (alloc.mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.memory);
    }
    
    // Phase 51: Clean up staging buffer if present
    if (alloc.staging_mapped_ptr) {
        vkUnmapMemory(m_device->handle, alloc.staging_memory);
    }
    if (alloc.staging_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->handle, alloc.staging_buffer, nullptr);
    }
    if (alloc.staging_memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.staging_memory, nullptr);
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
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_index_buffers.size() || !lockedData) {
        printf("[Vulkan] LockIndexBuffer: Invalid parameters (handle=%llu)\n", handle);
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle - 1];
    
    // Adjust size if 0 (meaning lock entire buffer)
    if (size == 0) {
        size = alloc.size - offset;
    }
    
    if (offset + size > alloc.size) {
        printf("[Vulkan] ERROR: Lock range exceeds buffer size\n");
        return false;
    }
    
    // Phase 51: For static buffers, use staging buffer approach
    if (!alloc.is_dynamic) {
        printf("[Vulkan] LockIndexBuffer: Static buffer - using staging buffer approach\n");
        
        // Create staging buffer if not already created
        if (alloc.staging_buffer == VK_NULL_HANDLE) {
            if (!CreateStagingBuffer(m_device->handle, m_physical_device->handle,
                                    alloc.size, &alloc.staging_buffer, &alloc.staging_memory)) {
                printf("[Vulkan] ERROR: Failed to create staging buffer for static index buffer\n");
                return false;
            }
        }
        
        // Map the staging buffer
        VkResult result = vkMapMemory(m_device->handle, alloc.staging_memory, 0, alloc.size, 0, &alloc.staging_mapped_ptr);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: vkMapMemory for staging buffer failed (result=%d)\n", result);
            return false;
        }
        
        alloc.staging_lock_offset = offset;
        alloc.staging_lock_size = size;
        alloc.using_staging = true;
        
        // Return pointer to the staging buffer at the correct offset
        *lockedData = (char*)alloc.staging_mapped_ptr + offset;
        printf("[Vulkan] LockIndexBuffer (static): handle=%llu offset=%u size=%u (staging)\n", handle, offset, size);
        return true;
    }
    
    // Dynamic buffer - direct map
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
    if (!m_initialized || !m_device || handle == INVALID_HANDLE || handle > (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] UnlockIndexBuffer: Invalid handle %llu\n", handle);
        return false;
    }
    
    VulkanBufferAllocation& alloc = g_index_buffers[handle - 1];
    
    // Phase 51: Handle staging buffer for static buffers
    if (alloc.using_staging && alloc.staging_mapped_ptr) {
        // Unmap staging buffer
        vkUnmapMemory(m_device->handle, alloc.staging_memory);
        alloc.staging_mapped_ptr = nullptr;
        
        // Copy from staging to device-local buffer via command buffer
        VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
        
        // Create a temporary command pool and buffer for the transfer
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_device->graphics_queue_family;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        
        VkCommandPool cmdPool = VK_NULL_HANDLE;
        VkResult result = vkCreateCommandPool(m_device->handle, &poolInfo, nullptr, &cmdPool);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: UnlockIndexBuffer - Failed to create command pool\n");
            alloc.using_staging = false;
            return false;
        }
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = cmdPool;
        allocInfo.commandBufferCount = 1;
        
        result = vkAllocateCommandBuffers(m_device->handle, &allocInfo, &cmdBuffer);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] ERROR: UnlockIndexBuffer - Failed to allocate command buffer\n");
            vkDestroyCommandPool(m_device->handle, cmdPool, nullptr);
            alloc.using_staging = false;
            return false;
        }
        
        // Begin recording
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        
        vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        
        // Copy buffer
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = alloc.staging_lock_offset;
        copyRegion.dstOffset = alloc.staging_lock_offset;
        copyRegion.size = alloc.staging_lock_size;
        
        vkCmdCopyBuffer(cmdBuffer, alloc.staging_buffer, alloc.buffer, 1, &copyRegion);
        
        vkEndCommandBuffer(cmdBuffer);
        
        // Submit and wait
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        
        vkQueueSubmit(m_device->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_device->graphics_queue);
        
        // Cleanup
        vkFreeCommandBuffers(m_device->handle, cmdPool, 1, &cmdBuffer);
        vkDestroyCommandPool(m_device->handle, cmdPool, nullptr);
        
        alloc.using_staging = false;
        printf("[Vulkan] UnlockIndexBuffer (static): handle=%llu - staging copy complete\n", handle);
        return true;
    }
    
    // Dynamic buffer - direct unmap
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
    if (handle == INVALID_HANDLE || handle > (uint64_t)g_index_buffers.size()) {
        return 0;
    }
    
    return g_index_buffers[handle - 1].size;
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
    // Phase 56: Track current vertex buffer binding
    // Only stream 0 is currently supported
    if (streamIndex != 0) {
        printf("[Vulkan] SetVertexStreamSource: Only stream 0 supported, got %u\n", streamIndex);
        return false;
    }
    
    // Validate handle (can be INVALID_HANDLE to unbind)
    if (vbHandle != INVALID_HANDLE && vbHandle > (uint64_t)g_vertex_buffers.size()) {
        printf("[Vulkan] SetVertexStreamSource: Invalid handle %llu (max=%zu)\n", 
               vbHandle, g_vertex_buffers.size());
        return false;
    }
    
    g_current_vertex_buffer = vbHandle;
    g_current_vertex_offset = offset;
    g_current_vertex_stride = stride;
    
    return true;
}

bool VulkanGraphicsDriver::SetIndexBuffer(IndexBufferHandle ibHandle, uint32_t startIndex)
{
    // Phase 56: Track current index buffer binding
    
    // Validate handle (can be INVALID_HANDLE to unbind)
    if (ibHandle != INVALID_HANDLE && ibHandle > (uint64_t)g_index_buffers.size()) {
        printf("[Vulkan] SetIndexBuffer: Invalid handle %llu (max=%zu)\n", 
               ibHandle, g_index_buffers.size());
        return false;
    }
    
    g_current_index_buffer = ibHandle;
    g_current_index_start = startIndex;
    
    return true;
}

TextureHandle VulkanGraphicsDriver::CreateTexture(const TextureDescriptor& desc, const void* initialData)
{
    if (!m_initialized || !m_device || !m_memory_allocator) {
        printf("[Vulkan] CreateTexture: Driver not initialized\n");
        return INVALID_HANDLE;
    }
    
    printf("[Vulkan] Phase 57 CreateTexture: width=%u height=%u format=%d mipLevels=%u data=%p\n",
           desc.width, desc.height, (int)desc.format, desc.mipLevels, initialData);
    
    // Initialize descriptor pool and sampler on first texture creation
    if (g_descriptorPool == VK_NULL_HANDLE) {
        g_descriptorPool = CreateTextureDescriptorPool(m_device->handle, 256);
        if (g_descriptorPool == VK_NULL_HANDLE) {
            printf("[Vulkan] Phase 57: Failed to create descriptor pool\n");
            return INVALID_HANDLE;
        }
    }
    
    if (g_textureDescriptorSetLayout == VK_NULL_HANDLE) {
        g_textureDescriptorSetLayout = CreateTextureDescriptorSetLayout(m_device->handle);
        if (g_textureDescriptorSetLayout == VK_NULL_HANDLE) {
            printf("[Vulkan] Phase 57: Failed to create descriptor set layout\n");
            return INVALID_HANDLE;
        }
    }
    
    if (g_defaultSampler == VK_NULL_HANDLE) {
        g_defaultSampler = CreateDefaultSampler(m_device->handle);
        if (g_defaultSampler == VK_NULL_HANDLE) {
            printf("[Vulkan] Phase 57: Failed to create default sampler\n");
            return INVALID_HANDLE;
        }
    }
    
    VulkanTextureAllocation allocation;
    allocation.width = desc.width;
    allocation.height = desc.height;
    allocation.depth = desc.depth;
    allocation.format = desc.format;
    allocation.mipLevels = desc.mipLevels;
    allocation.cubeMap = desc.cubeMap;
    allocation.renderTarget = desc.renderTarget;
    allocation.depthStencil = desc.depthStencil;
    allocation.dynamic = desc.dynamic;
    
    VkFormat vkFormat = TextureFormatToVkFormat(desc.format);
    printf("[Vulkan] Phase 57: Using VkFormat=%d\n", vkFormat);
    
    // Step 1: Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = desc.width;
    imageInfo.extent.height = desc.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = (desc.mipLevels > 0) ? desc.mipLevels : 1;
    imageInfo.arrayLayers = desc.cubeMap ? 6 : 1;
    imageInfo.format = vkFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (desc.cubeMap) {
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    
    VkResult result = vkCreateImage(m_device->handle, &imageInfo, nullptr, &allocation.image);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to create VkImage (result=%d)\n", result);
        return INVALID_HANDLE;
    }
    
    // Step 2: Allocate image memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->handle, allocation.image, &memRequirements);
    
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = FindMemoryType(m_physical_device->handle, 
                                                   memRequirements.memoryTypeBits,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    result = vkAllocateMemory(m_device->handle, &memAllocInfo, nullptr, &allocation.memory);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to allocate image memory (result=%d)\n", result);
        vkDestroyImage(m_device->handle, allocation.image, nullptr);
        return INVALID_HANDLE;
    }
    
    result = vkBindImageMemory(m_device->handle, allocation.image, allocation.memory, 0);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to bind image memory (result=%d)\n", result);
        vkFreeMemory(m_device->handle, allocation.memory, nullptr);
        vkDestroyImage(m_device->handle, allocation.image, nullptr);
        return INVALID_HANDLE;
    }
    
    // Step 3: Upload initial data if provided
    if (initialData != nullptr) {
        uint32_t dataSize = CalculateTextureDataSize(desc.width, desc.height, desc.format);
        printf("[Vulkan] Phase 57: Uploading %u bytes of texture data\n", dataSize);
        
        // Create staging buffer
        VkBufferCreateInfo stagingBufferInfo{};
        stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingBufferInfo.size = dataSize;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        result = vkCreateBuffer(m_device->handle, &stagingBufferInfo, nullptr, &allocation.stagingBuffer);
        if (result != VK_SUCCESS) {
            printf("[Vulkan] Phase 57: ERROR - Failed to create staging buffer (result=%d)\n", result);
            // Continue without initial data
        } else {
            // Allocate staging buffer memory
            VkMemoryRequirements stagingMemReq;
            vkGetBufferMemoryRequirements(m_device->handle, allocation.stagingBuffer, &stagingMemReq);
            
            VkMemoryAllocateInfo stagingMemInfo{};
            stagingMemInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            stagingMemInfo.allocationSize = stagingMemReq.size;
            stagingMemInfo.memoryTypeIndex = FindMemoryType(m_physical_device->handle,
                                                            stagingMemReq.memoryTypeBits,
                                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            
            result = vkAllocateMemory(m_device->handle, &stagingMemInfo, nullptr, &allocation.stagingMemory);
            if (result == VK_SUCCESS) {
                vkBindBufferMemory(m_device->handle, allocation.stagingBuffer, allocation.stagingMemory, 0);
                
                // Copy data to staging buffer
                void* mappedData;
                vkMapMemory(m_device->handle, allocation.stagingMemory, 0, dataSize, 0, &mappedData);
                memcpy(mappedData, initialData, dataSize);
                vkUnmapMemory(m_device->handle, allocation.stagingMemory);
                
                // Transition image layout and copy
                TransitionImageLayout(m_device->handle, g_commandPool, m_device->graphics_queue,
                                     allocation.image, vkFormat,
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                
                CopyBufferToImage(m_device->handle, g_commandPool, m_device->graphics_queue,
                                 allocation.stagingBuffer, allocation.image, desc.width, desc.height);
                
                TransitionImageLayout(m_device->handle, g_commandPool, m_device->graphics_queue,
                                     allocation.image, vkFormat,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                
                // Cleanup staging resources
                vkDestroyBuffer(m_device->handle, allocation.stagingBuffer, nullptr);
                vkFreeMemory(m_device->handle, allocation.stagingMemory, nullptr);
                allocation.stagingBuffer = VK_NULL_HANDLE;
                allocation.stagingMemory = VK_NULL_HANDLE;
                
                printf("[Vulkan] Phase 57: Texture data uploaded successfully\n");
            }
        }
    } else {
        // No initial data - transition to shader read layout anyway
        TransitionImageLayout(m_device->handle, g_commandPool, m_device->graphics_queue,
                             allocation.image, vkFormat,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    
    // Step 4: Create VkImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = allocation.image;
    viewInfo.viewType = desc.cubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = vkFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = (desc.mipLevels > 0) ? desc.mipLevels : 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = desc.cubeMap ? 6 : 1;
    
    result = vkCreateImageView(m_device->handle, &viewInfo, nullptr, &allocation.imageView);
    if (result != VK_SUCCESS) {
        printf("[Vulkan] Phase 57: ERROR - Failed to create VkImageView (result=%d)\n", result);
        vkFreeMemory(m_device->handle, allocation.memory, nullptr);
        vkDestroyImage(m_device->handle, allocation.image, nullptr);
        return INVALID_HANDLE;
    }
    
    // Step 5: Use default sampler (individual samplers per texture can be added later)
    allocation.sampler = g_defaultSampler;
    
    // Step 6: Allocate and update descriptor set
    allocation.descriptorSet = AllocateTextureDescriptorSet(m_device->handle, g_descriptorPool,
                                                            g_textureDescriptorSetLayout);
    if (allocation.descriptorSet != VK_NULL_HANDLE) {
        UpdateTextureDescriptorSet(m_device->handle, allocation.descriptorSet,
                                   allocation.imageView, allocation.sampler);
    }
    
    // Add to texture storage
    g_textures.push_back(allocation);
    TextureHandle handle = g_textures.size() - 1;
    
    printf("[Vulkan] Phase 57 CreateTexture: SUCCESS (handle=%llu, VkImage=%p, VkImageView=%p)\n", 
           handle, (void*)allocation.image, (void*)allocation.imageView);
    return handle;
}

void VulkanGraphicsDriver::DestroyTexture(TextureHandle handle)
{
    if (handle >= g_textures.size()) {
        printf("[Vulkan] DestroyTexture: Invalid handle %llu\n", handle);
        return;
    }
    
    VulkanTextureAllocation& alloc = g_textures[handle];
    printf("[Vulkan] Phase 57 DestroyTexture: Destroying texture (width=%u height=%u)\n", 
           alloc.width, alloc.height);
    
    // Wait for GPU to finish using the texture
    if (m_device && m_device->handle) {
        vkDeviceWaitIdle(m_device->handle);
    }
    
    // Clean up descriptor set (free back to pool)
    if (alloc.descriptorSet != VK_NULL_HANDLE && g_descriptorPool != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(m_device->handle, g_descriptorPool, 1, &alloc.descriptorSet);
        alloc.descriptorSet = VK_NULL_HANDLE;
    }
    
    // Destroy image view
    if (alloc.imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->handle, alloc.imageView, nullptr);
        alloc.imageView = VK_NULL_HANDLE;
    }
    
    // Free image memory
    if (alloc.memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.memory, nullptr);
        alloc.memory = VK_NULL_HANDLE;
    }
    
    // Destroy image
    if (alloc.image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device->handle, alloc.image, nullptr);
        alloc.image = VK_NULL_HANDLE;
    }
    
    // Clean up staging resources if any
    if (alloc.stagingBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device->handle, alloc.stagingBuffer, nullptr);
        alloc.stagingBuffer = VK_NULL_HANDLE;
    }
    if (alloc.stagingMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->handle, alloc.stagingMemory, nullptr);
        alloc.stagingMemory = VK_NULL_HANDLE;
    }
    
    // Note: sampler is shared (g_defaultSampler), don't destroy it per-texture
    alloc.sampler = VK_NULL_HANDLE;
    
    printf("[Vulkan] Phase 57 DestroyTexture: Texture destroyed successfully\n");
}

bool VulkanGraphicsDriver::SetTexture(uint32_t samplerIndex, TextureHandle handle)
{
    // Validate sampler index
    if (samplerIndex >= MAX_TEXTURE_STAGES) {
        printf("[Vulkan] Phase 57 SetTexture: Invalid sampler index %u (max=%u)\n", 
               samplerIndex, MAX_TEXTURE_STAGES);
        return false;
    }
    
    // Validate handle (INVALID_HANDLE is valid - unbinds texture)
    if (handle != INVALID_HANDLE && handle >= g_textures.size()) {
        printf("[Vulkan] Phase 57 SetTexture: Invalid handle %llu for sampler %u\n", 
               handle, samplerIndex);
        return false;
    }
    
    // Check if texture changed
    if (g_boundTextures[samplerIndex] != handle) {
        g_boundTextures[samplerIndex] = handle;
        g_textureDirty = true;
        
        if (handle != INVALID_HANDLE) {
            const VulkanTextureAllocation& alloc = g_textures[handle];
            printf("[Vulkan] Phase 57 SetTexture: Bound texture (handle=%llu, %ux%u) to sampler %u\n", 
                   handle, alloc.width, alloc.height, samplerIndex);
        } else {
            printf("[Vulkan] Phase 57 SetTexture: Unbound texture from sampler %u\n", samplerIndex);
        }
    }
    
    return true;
}

TextureHandle VulkanGraphicsDriver::GetTexture(uint32_t samplerIndex) const
{
    if (samplerIndex >= MAX_TEXTURE_STAGES) {
        return INVALID_HANDLE;
    }
    
    return g_boundTextures[samplerIndex];
}

bool VulkanGraphicsDriver::LockTexture(TextureHandle handle, uint32_t level, void** lockedData,
                                      uint32_t* pitch)
{
    if (handle >= g_textures.size()) {
        printf("[Vulkan] LockTexture: Invalid handle %llu\n", handle);
        return false;
    }
    
    VulkanTextureAllocation& alloc = g_textures[handle];
    printf("[Vulkan] LockTexture: Locking texture level %u (width=%u height=%u)\n", level, alloc.width, alloc.height);
    
    if (!alloc.dynamic) {
        printf("[Vulkan] LockTexture: ERROR - Texture is not dynamic\n");
        return false;
    }
    
    // TODO Phase 41 Week 2: Implement staging buffer mapping
    // - Create staging buffer if not exists (for dynamic textures)
    // - Calculate pitch (row alignment for GPU)
    // - Map staging buffer memory with vkMapMemory()
    // - Set lockedData pointer to mapped memory
    // - Store pitch for proper row alignment on GPU
    
    // For now, return failure as staging buffer not yet implemented
    if (lockedData) *lockedData = nullptr;
    if (pitch) *pitch = 0;
    
    return false;
}

bool VulkanGraphicsDriver::UnlockTexture(TextureHandle handle, uint32_t level)
{
    if (handle >= g_textures.size()) {
        printf("[Vulkan] UnlockTexture: Invalid handle %llu\n", handle);
        return false;
    }
    
    VulkanTextureAllocation& alloc = g_textures[handle];
    printf("[Vulkan] UnlockTexture: Unlocking texture level %u\n", level);
    
    // TODO Phase 41 Week 2: Implement staging buffer unmapping and copy
    // - Unmap staging buffer memory with vkUnmapMemory()
    // - Record command buffer for staging buffer → image copy
    // - Handle mip level selection and destination offset
    // - Submit copy command to graphics queue
    // - Ensure proper synchronization (wait for transfer complete)
    
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
    // Store world matrix and mark transforms as dirty
    g_worldMatrix = matrix;
    g_transformDirty = true;
}

void VulkanGraphicsDriver::SetViewMatrix(const Matrix4x4& matrix)
{
    // Store view matrix and mark transforms as dirty
    g_viewMatrix = matrix;
    g_transformDirty = true;
}

void VulkanGraphicsDriver::SetProjectionMatrix(const Matrix4x4& matrix)
{
    // Store projection matrix and mark transforms as dirty
    g_projectionMatrix = matrix;
    g_transformDirty = true;
}

Matrix4x4 VulkanGraphicsDriver::GetWorldMatrix() const
{
    return g_worldMatrix;
}

Matrix4x4 VulkanGraphicsDriver::GetViewMatrix() const
{
    return g_viewMatrix;
}

Matrix4x4 VulkanGraphicsDriver::GetProjectionMatrix() const
{
    return g_projectionMatrix;
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
// Phase 54: Frame Infrastructure Implementation
// ============================================================================

bool VulkanGraphicsDriver::CreateSyncObjects()
{
    g_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    g_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    g_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signaled so first frame doesn't wait forever
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult result1 = vkCreateSemaphore(m_device->handle, &semaphoreInfo, nullptr, &g_imageAvailableSemaphores[i]);
        VkResult result2 = vkCreateSemaphore(m_device->handle, &semaphoreInfo, nullptr, &g_renderFinishedSemaphores[i]);
        VkResult result3 = vkCreateFence(m_device->handle, &fenceInfo, nullptr, &g_inFlightFences[i]);
        
        if (result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS) {
            fprintf(stderr, "[Vulkan] ERROR: Failed to create sync objects for frame %u\n", i);
            return false;
        }
    }
    
    fprintf(stderr, "[Vulkan] CreateSyncObjects: Created %u sync object sets\n", MAX_FRAMES_IN_FLIGHT);
    return true;
}

void VulkanGraphicsDriver::DestroySyncObjects()
{
    if (!m_device || m_device->handle == VK_NULL_HANDLE) return;
    
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (i < g_imageAvailableSemaphores.size() && g_imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device->handle, g_imageAvailableSemaphores[i], nullptr);
        }
        if (i < g_renderFinishedSemaphores.size() && g_renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device->handle, g_renderFinishedSemaphores[i], nullptr);
        }
        if (i < g_inFlightFences.size() && g_inFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_device->handle, g_inFlightFences[i], nullptr);
        }
    }
    
    g_imageAvailableSemaphores.clear();
    g_renderFinishedSemaphores.clear();
    g_inFlightFences.clear();
    
    fprintf(stderr, "[Vulkan] DestroySyncObjects: Sync objects destroyed\n");
}

bool VulkanGraphicsDriver::CreateCommandBuffers()
{
    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_device->graphics_queue_family;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VkResult result = vkCreateCommandPool(m_device->handle, &poolInfo, nullptr, &g_commandPool);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create command pool (result=%d)\n", result);
        return false;
    }
    
    // Allocate command buffers
    g_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = g_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    
    result = vkAllocateCommandBuffers(m_device->handle, &allocInfo, g_commandBuffers.data());
    if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to allocate command buffers (result=%d)\n", result);
        return false;
    }
    
    fprintf(stderr, "[Vulkan] CreateCommandBuffers: Created pool and %u command buffers\n", MAX_FRAMES_IN_FLIGHT);
    return true;
}

void VulkanGraphicsDriver::DestroyCommandBuffers()
{
    if (!m_device || m_device->handle == VK_NULL_HANDLE) return;
    
    if (g_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device->handle, g_commandPool, nullptr);
        g_commandPool = VK_NULL_HANDLE;
    }
    
    g_commandBuffers.clear();
    
    fprintf(stderr, "[Vulkan] DestroyCommandBuffers: Command pool and buffers destroyed\n");
}

bool VulkanGraphicsDriver::CreateGraphicsPipeline()
{
    // Create shader modules from embedded SPIR-V
    g_vertShaderModule = CreateShaderModule(m_device->handle, 
        EmbeddedShaders::fullscreen_vert_spv, 
        EmbeddedShaders::fullscreen_vert_spv_len);
    
    if (g_vertShaderModule == VK_NULL_HANDLE) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create vertex shader module\n");
        return false;
    }
    
    g_fragShaderModule = CreateShaderModule(m_device->handle,
        EmbeddedShaders::solid_color_frag_spv,
        EmbeddedShaders::solid_color_frag_spv_len);
    
    if (g_fragShaderModule == VK_NULL_HANDLE) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create fragment shader module\n");
        return false;
    }
    
    fprintf(stderr, "[Vulkan] CreateGraphicsPipeline: Shader modules created\n");
    
    // Shader stage info
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = g_vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = g_fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input state - fullscreen shader generates vertices procedurally
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport and scissor (dynamic state)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // Dynamic states
    std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Phase 58: Push constant ranges for vertex (MVP) and fragment (color) shaders
    // Vertex shader: MVP matrix at offset 0 (64 bytes)
    // Fragment shader: color at offset 64 (16 bytes)
    std::array<VkPushConstantRange, 2> pushConstantRanges{};
    
    // Vertex stage: MVP matrix
    pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRanges[0].offset = 0;
    pushConstantRanges[0].size = VERTEX_PUSH_CONSTANT_SIZE;  // 64 bytes (mat4)
    
    // Fragment stage: color
    pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRanges[1].offset = VERTEX_PUSH_CONSTANT_SIZE;  // After MVP
    pushConstantRanges[1].size = FRAGMENT_PUSH_CONSTANT_SIZE;  // 16 bytes (vec4)
    
    fprintf(stderr, "[Vulkan] Phase 58: Push constants - Vertex MVP: %u bytes, Fragment color: %u bytes (total: %u)\n",
            VERTEX_PUSH_CONSTANT_SIZE, FRAGMENT_PUSH_CONSTANT_SIZE, TOTAL_PUSH_CONSTANT_SIZE);
    
    // Phase 57: Create texture descriptor set layout if not exists
    if (g_textureDescriptorSetLayout == VK_NULL_HANDLE) {
        g_textureDescriptorSetLayout = CreateTextureDescriptorSetLayout(m_device->handle);
        if (g_textureDescriptorSetLayout == VK_NULL_HANDLE) {
            fprintf(stderr, "[Vulkan] WARNING: Failed to create texture descriptor set layout\n");
            // Continue without texture support
        }
    }
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();
    
    // Phase 57: Include texture descriptor set layout if available
    if (g_textureDescriptorSetLayout != VK_NULL_HANDLE) {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &g_textureDescriptorSetLayout;
        fprintf(stderr, "[Vulkan] Phase 57: Pipeline layout will include texture descriptor set\n");
    } else {
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }
    
    VkResult result = vkCreatePipelineLayout(m_device->handle, &pipelineLayoutInfo, nullptr, &g_pipelineLayout);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create pipeline layout (result=%d)\n", result);
        return false;
    }
    
    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = g_pipelineLayout;
    pipelineInfo.renderPass = m_render_pass->handle;
    pipelineInfo.subpass = 0;
    
    result = vkCreateGraphicsPipelines(m_device->handle, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &g_graphicsPipeline);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "[Vulkan] ERROR: Failed to create graphics pipeline (result=%d)\n", result);
        return false;
    }
    
    fprintf(stderr, "[Vulkan] CreateGraphicsPipeline: SUCCESS - Pipeline created\n");
    return true;
}

void VulkanGraphicsDriver::DestroyGraphicsPipeline()
{
    if (!m_device || m_device->handle == VK_NULL_HANDLE) return;
    
    // Wait for GPU to finish
    vkDeviceWaitIdle(m_device->handle);
    
    if (g_graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->handle, g_graphicsPipeline, nullptr);
        g_graphicsPipeline = VK_NULL_HANDLE;
    }
    
    if (g_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->handle, g_pipelineLayout, nullptr);
        g_pipelineLayout = VK_NULL_HANDLE;
    }
    
    if (g_vertShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device->handle, g_vertShaderModule, nullptr);
        g_vertShaderModule = VK_NULL_HANDLE;
    }
    
    if (g_fragShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device->handle, g_fragShaderModule, nullptr);
        g_fragShaderModule = VK_NULL_HANDLE;
    }
    
    // Phase 57: Cleanup texture resources
    if (g_defaultSampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device->handle, g_defaultSampler, nullptr);
        g_defaultSampler = VK_NULL_HANDLE;
    }
    
    if (g_textureDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(m_device->handle, g_textureDescriptorSetLayout, nullptr);
        g_textureDescriptorSetLayout = VK_NULL_HANDLE;
    }
    
    if (g_descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device->handle, g_descriptorPool, nullptr);
        g_descriptorPool = VK_NULL_HANDLE;
    }
    
    // Reset bound textures
    for (uint32_t i = 0; i < MAX_TEXTURE_STAGES; ++i) {
        g_boundTextures[i] = INVALID_HANDLE;
    }
    g_textureDirty = false;
    
    fprintf(stderr, "[Vulkan] DestroyGraphicsPipeline: Pipeline, shaders and texture resources destroyed\n");
}

bool VulkanGraphicsDriver::RecordClearCommand()
{
    // This is now handled in BeginFrame via render pass clear
    return true;
}

}  // namespace Graphics

// ============================================================================
// Factory Function (C linkage for cross-module access)
// ============================================================================

/**
 * Factory function for creating Vulkan graphics driver instances.
 * 
 * This is called by GraphicsDriverFactory to create Vulkan drivers
 * with full initialization parameters.
 * 
 * NOTE: This function MUST be outside the namespace Graphics to have proper
 * C linkage for the factory pattern.
 * 
 * @param windowHandle Platform-specific window handle (HWND on Windows, NSWindow* on macOS, etc)
 * @param width Display width in pixels
 * @param height Display height in pixels  
 * @param fullscreen Full-screen mode flag
 * @return New IGraphicsDriver instance (Vulkan implementation), or nullptr on failure
 */
extern "C" Graphics::IGraphicsDriver* CreateVulkanGraphicsDriver(void* windowHandle, uint32_t width,
                                                                uint32_t height, bool fullscreen)
{
    fprintf(stderr, "[CreateVulkanGraphicsDriver] Creating new VulkanGraphicsDriver instance\n");
    fprintf(stderr, "[CreateVulkanGraphicsDriver] Window: %p, Size: %ux%u, Fullscreen: %d\n",
           windowHandle, width, height, fullscreen);
    fflush(stderr);
    
    // Create driver instance
    Graphics::VulkanGraphicsDriver* driver = new Graphics::VulkanGraphicsDriver();
    
    // Initialize driver with provided parameters
    if (!driver->Initialize(windowHandle, width, height, fullscreen)) {
        fprintf(stderr, "[CreateVulkanGraphicsDriver] ERROR: Initialization failed!\n");
        fflush(stderr);
        delete driver;
        return nullptr;
    }
    
    fprintf(stderr, "[CreateVulkanGraphicsDriver] VulkanGraphicsDriver initialized successfully\n");
    fflush(stderr);
    return driver;
}
