/*
 * Vulkan Stubs - Minimal Type Definitions
 *
 * Provides stub definitions for Vulkan types when the Vulkan SDK is not available.
 * This allows the project to compile and test its abstraction layer without requiring
 * the full Vulkan SDK installation.
 *
 * When Vulkan SDK is installed, these stubs are not used (real vulkan.h is included instead).
 *
 * **IMPORTANT**: These stubs are for compilation ONLY. Runtime functionality requires real Vulkan SDK.
 */

#ifndef VULKAN_STUBS_H
#define VULKAN_STUBS_H

#ifdef RTS_HAS_VULKAN
    /* Vulkan SDK is available - use real Vulkan headers */
    #include <vulkan/vulkan.h>
#else
    /* Vulkan SDK not available - use stub definitions */

    #include <stdint.h>
    #include <stddef.h>

    // ============================================================
    // Basic Type Definitions
    // ============================================================

    typedef uint32_t VkResult;
    typedef uint32_t VkFlags;
    typedef uint64_t VkDeviceSize;
    typedef uint32_t VkBool32;
    typedef uintptr_t VkInstance;
    typedef uintptr_t VkPhysicalDevice;
    typedef uintptr_t VkDevice;
    typedef uintptr_t VkQueue;
    typedef uintptr_t VkCommandPool;
    typedef uintptr_t VkCommandBuffer;
    typedef uintptr_t VkBuffer;
    typedef uintptr_t VkBufferView;
    typedef uintptr_t VkImage;
    typedef uintptr_t VkImageView;
    typedef uintptr_t VkShaderModule;
    typedef uintptr_t VkPipeline;
    typedef uintptr_t VkPipelineLayout;
    typedef uintptr_t VkSampler;
    typedef uintptr_t VkDescriptorSet;
    typedef uintptr_t VkDescriptorSetLayout;
    typedef uintptr_t VkDescriptorPool;
    typedef uintptr_t VkFramebuffer;
    typedef uintptr_t VkRenderPass;
    typedef uintptr_t VkFence;
    typedef uintptr_t VkSemaphore;
    typedef uintptr_t VkEvent;
    typedef uintptr_t VkSurfaceKHR;
    typedef uintptr_t VkSwapchainKHR;
    typedef uintptr_t VkMemory;
    typedef uintptr_t VkDeviceMemory;

    // NULL handle
    #define VK_NULL_HANDLE 0

    // Result codes
    #define VK_SUCCESS 0
    #define VK_ERROR_OUT_OF_HOST_MEMORY 1
    #define VK_ERROR_OUT_OF_DEVICE_MEMORY 2
    #define VK_ERROR_DEVICE_LOST 3

    // Queue flags
    #define VK_QUEUE_GRAPHICS_BIT 0x00000001
    #define VK_QUEUE_COMPUTE_BIT 0x00000002
    #define VK_QUEUE_TRANSFER_BIT 0x00000004

    // Memory type flags
    #define VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 0x00000001
    #define VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 0x00000002
    #define VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 0x00000004

    // Buffer usage flags
    #define VK_BUFFER_USAGE_TRANSFER_DST_BIT 0x00000001
    #define VK_BUFFER_USAGE_VERTEX_BUFFER_BIT 0x00000010
    #define VK_BUFFER_USAGE_INDEX_BUFFER_BIT 0x00000020
    #define VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT 0x00000040

    // Image format enum
    typedef enum {
        VK_FORMAT_UNDEFINED = 0,
        VK_FORMAT_R8G8B8A8_UNORM = 37,
        VK_FORMAT_R8G8B8A8_SRGB = 43,
        VK_FORMAT_B8G8R8A8_UNORM = 44,
        VK_FORMAT_D32_SFLOAT = 126
    } VkFormat;

    // Primitive topology
    typedef enum {
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3
    } VkPrimitiveTopology;

    // Cull mode
    typedef enum {
        VK_CULL_MODE_NONE = 0,
        VK_CULL_MODE_FRONT_BIT = 0x00000001,
        VK_CULL_MODE_BACK_BIT = 0x00000002
    } VkCullModeFlagBits;

    // Blend factor
    typedef enum {
        VK_BLEND_FACTOR_ZERO = 0,
        VK_BLEND_FACTOR_ONE = 1,
        VK_BLEND_FACTOR_SRC_COLOR = 2,
        VK_BLEND_FACTOR_DST_COLOR = 5
    } VkBlendFactor;

    // Blend operation
    typedef enum {
        VK_BLEND_OP_ADD = 0,
        VK_BLEND_OP_SUBTRACT = 1
    } VkBlendOp;

    // Filter
    typedef enum {
        VK_FILTER_NEAREST = 0,
        VK_FILTER_LINEAR = 1
    } VkFilter;

    // Sampler address mode
    typedef enum {
        VK_SAMPLER_ADDRESS_MODE_REPEAT = 0,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2
    } VkSamplerAddressMode;

    // ============================================================
    // Structure Definitions
    // ============================================================

    typedef struct {
        VkFormat format;
        VkFlags flags;
    } VkAttachmentDescription;

    typedef struct {
        int queueCount;
        int queueFamilyIndex;
    } VkQueueFamilyProperties;

    typedef struct {
        uint32_t memoryTypeIndex;
        VkDeviceSize size;
        uint32_t memoryTypeBits;
    } VkMemoryRequirements;

#endif /* RTS_HAS_VULKAN */

#endif /* VULKAN_STUBS_H */
