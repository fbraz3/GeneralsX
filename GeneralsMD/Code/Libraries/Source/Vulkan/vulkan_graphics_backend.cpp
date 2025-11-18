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
 * Phase 39.3: VulkanGraphicsBackend Implementation
 * 
 * Stage 1: Core Vulkan Infrastructure (Week 1-2)
 * - Vulkan instance creation with validation layers
 * - Physical device selection
 * - Logical device creation with queue management
 * - Swapchain management and presentation
 * 
 * Stage 2: Resource Management (Week 2-3)
 * - VMA (Vulkan Memory Allocator) integration
 * - Buffer and image management
 * - Descriptor set management
 * 
 * Stage 3: D3D8 Method Implementation (Week 3-4)
 * - Map all 150+ D3D8 API calls to Vulkan
 * - Frame rendering pipeline
 * - Material and texture binding
 */

#include "vulkan_graphics_backend.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations of components
class VulkanInstance
{
public:
	VkInstance handle = VK_NULL_HANDLE;
	
	bool Create(bool debug_mode)
	{
		// Phase 39.3 Stage 1: Implement VkInstance creation with validation layers
		printf("[Vulkan] VulkanInstance::Create() - Starting instance creation (debug=%d)\n", debug_mode);
		
		// Set up application info
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Command & Conquer Generals Zero Hour";
		app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		app_info.pEngineName = "GeneralsX Vulkan Backend";
		app_info.engineVersion = VK_MAKE_API_VERSION(0, 39, 3, 0);
		app_info.apiVersion = VK_API_VERSION_1_2;
		
		// Required extensions for presentation
		std::vector<const char*> required_extensions;
		required_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		
		// Platform-specific surface extension
#ifdef VK_USE_PLATFORM_MACOS_MVK
		required_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
		required_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		required_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
		
		// Validation layer names
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
	
	void Destroy()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyInstance(handle, nullptr);
			printf("[Vulkan] VulkanInstance::Destroy() - Instance destroyed\n");
		}
		handle = VK_NULL_HANDLE;
	}
};

class VulkanPhysicalDevice
{
public:
	VkPhysicalDevice handle = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties properties{};
	VkPhysicalDeviceFeatures features{};
	
	bool Select(VkInstance instance)
	{
		// Phase 39.3 Stage 1: Implement physical device selection
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

class VulkanDevice
{
public:
	VkDevice handle = VK_NULL_HANDLE;
	VkQueue graphics_queue = VK_NULL_HANDLE;
	uint32_t graphics_queue_family = 0;
	
	bool Create(VkPhysicalDevice physical_device)
	{
		// Phase 39.3 Stage 1: Implement logical device creation
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
		// device_features can be extended based on needs
		
		// Create device info
		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = 1;
		device_create_info.pQueueCreateInfos = &queue_create_info;
		device_create_info.pEnabledFeatures = &device_features;
		
		// Create the logical device
		VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &handle);
		if (result != VK_SUCCESS) {
			printf("[Vulkan] ERROR: Failed to create logical device (result=%d)\n", result);
			return false;
		}
		
		// Get graphics queue
		vkGetDeviceQueue(handle, graphics_queue_family, 0, &graphics_queue);
		
		printf("[Vulkan] VulkanDevice::Create() - Success! Logical device created\n");
		return true;
	}
	
	void Destroy()
	{
		if (handle != VK_NULL_HANDLE) {
			// Wait for device to finish all operations
			vkDeviceWaitIdle(handle);
			vkDestroyDevice(handle, nullptr);
			printf("[Vulkan] VulkanDevice::Destroy() - Device destroyed\n");
		}
		handle = VK_NULL_HANDLE;
		graphics_queue = VK_NULL_HANDLE;
	}
};

class VulkanSwapchain
{
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
	
	bool Create(VkDevice device, VkPhysicalDevice physical_device, 
	           VkInstance instance, void* window_handle, uint32_t width, uint32_t height)
	{
		// Phase 39.3 Stage 1: Implement swapchain creation
		printf("[Vulkan] VulkanSwapchain::Create() - Creating surface and swapchain\n");
		
		// Create surface from window handle (platform-specific)
		// Note: This would normally use SDL_Vulkan_CreateSurface() but for now using vkCreateXXXSurfaceKHR
		if (!create_surface(instance, window_handle, surface)) {
			printf("[Vulkan] ERROR: Failed to create surface\n");
			return false;
		}
		
		printf("[Vulkan] Surface created successfully\n");
		
		// Get surface capabilities
		VkSurfaceCapabilitiesKHR surface_capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
		
		// Get available present modes
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
		
		// Select format (prefer SRGB if available, otherwise first format)
		VkSurfaceFormatKHR surface_format = surface_formats[0];
		for (const auto& fmt : surface_formats) {
			if (fmt.format == VK_FORMAT_R8G8B8A8_SRGB || fmt.format == VK_FORMAT_B8G8R8A8_SRGB) {
				surface_format = fmt;
				break;
			}
		}
		
		// Select present mode (prefer mailbox, fallback to FIFO)
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& mode : present_modes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				present_mode = mode;
				break;
			}
		}
		
		// Calculate swapchain extent
		extent = surface_capabilities.currentExtent;
		if (extent.width == UINT32_MAX) {
			extent.width = width;
			extent.height = height;
		}
		
		// Determine image count (use one more than minimum for triple buffering if supported)
		image_count = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount) {
			image_count = surface_capabilities.maxImageCount;
		}
		
		printf("[Vulkan] Swapchain: format=%d, extent=(%u,%u), image_count=%u, present_mode=%d\n",
		       surface_format.format, extent.width, extent.height, image_count, present_mode);
		
		// Create swapchain
		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = surface;
		swapchain_create_info.minImageCount = image_count;
		swapchain_create_info.imageFormat = surface_format.format;
		swapchain_create_info.imageColorSpace = surface_format.colorSpace;
		swapchain_create_info.imageExtent = extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.preTransform = surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = present_mode;
		swapchain_create_info.clipped = VK_TRUE;
		
		VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &handle);
		if (result != VK_SUCCESS) {
			printf("[Vulkan] ERROR: Failed to create swapchain (result=%d)\n", result);
			return false;
		}
		
		// Retrieve swapchain images
		uint32_t swapchain_image_count = 0;
		vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, nullptr);
		images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(device, handle, &swapchain_image_count, images.data());
		
		printf("[Vulkan] VulkanSwapchain::Create() - Success! Swapchain created with %u images\n", swapchain_image_count);
		return true;
	}
	
	bool CreateFramebuffers(VkDevice device, VkFormat swapchain_format, VkRenderPass render_pass, VkImageView depth_image_view)
	{
		// Create image views for each swapchain image
		image_views.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {
			VkImageViewCreateInfo image_view_info{};
			image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_info.image = images[i];
			image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_info.format = swapchain_format;
			image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_info.subresourceRange.baseMipLevel = 0;
			image_view_info.subresourceRange.levelCount = 1;
			image_view_info.subresourceRange.baseArrayLayer = 0;
			image_view_info.subresourceRange.layerCount = 1;
			
			VkResult result = vkCreateImageView(device, &image_view_info, nullptr, &image_views[i]);
			if (result != VK_SUCCESS) {
				printf("[Vulkan] ERROR: Failed to create image view %zu (result=%d)\n", i, result);
				return false;
			}
		}
		
		// Create framebuffers for each swapchain image
		framebuffers.resize(images.size());
		for (size_t i = 0; i < images.size(); ++i) {
			VkImageView attachments[] = { image_views[i], depth_image_view };
			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = render_pass;
			framebuffer_info.attachmentCount = (depth_image_view != VK_NULL_HANDLE) ? 2 : 1;
			framebuffer_info.pAttachments = attachments;
			framebuffer_info.width = extent.width;
			framebuffer_info.height = extent.height;
			framebuffer_info.layers = 1;
			
			VkResult result = vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]);
			if (result != VK_SUCCESS) {
				printf("[Vulkan] ERROR: Failed to create framebuffer %zu (result=%d)\n", i, result);
				return false;
			}
		}
		
		printf("[Vulkan] VulkanSwapchain::CreateFramebuffers() - Created %zu framebuffers\n", framebuffers.size());
		return true;
	}
	
	void UpdateCurrentFramebuffer()
	{
		if (current_image_index < framebuffers.size()) {
			current_framebuffer = framebuffers[current_image_index];
		}
	}
	
	void Destroy(VkDevice device, VkInstance instance)
	{
		// Destroy framebuffers
		for (auto framebuffer : framebuffers) {
			if (framebuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device, framebuffer, nullptr);
			}
		}
		framebuffers.clear();
		
		// Destroy image views
		for (auto image_view : image_views) {
			if (image_view != VK_NULL_HANDLE) {
				vkDestroyImageView(device, image_view, nullptr);
			}
		}
		image_views.clear();
		
		if (handle != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device, handle, nullptr);
			printf("[Vulkan] Swapchain destroyed\n");
		}
		if (surface != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(instance, surface, nullptr);
			printf("[Vulkan] Surface destroyed\n");
		}
		handle = VK_NULL_HANDLE;
		surface = VK_NULL_HANDLE;
	}
	
private:
	bool create_surface(VkInstance instance, void* window_handle, VkSurfaceKHR& out_surface)
	{
		// Platform-specific surface creation
		// Note: This is a simplified approach. In production, use SDL_Vulkan_CreateSurface()
#ifdef VK_USE_PLATFORM_MACOS_MVK
		VkMacOSSurfaceCreateInfoMVK surface_create_info{};
		surface_create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surface_create_info.pView = window_handle;
		return vkCreateMacOSSurfaceMVK(instance, &surface_create_info, nullptr, &out_surface) == VK_SUCCESS;
#else
		// For other platforms, would use different surface creation methods
		printf("[Vulkan] WARNING: Surface creation not implemented for this platform\n");
		return false;
#endif
	}
};

class VulkanMemoryAllocator
{
public:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	
	bool Create(VkInstance instance, VkPhysicalDevice physical_dev, 
	           VkDevice logical_device)
	{
		// Phase 39.3 Stage 2: Simple memory allocator (without VMA for now)
		// Note: In production, integrate Vulkan Memory Allocator (VMA)
		// For now, use native Vulkan memory management
		printf("[Vulkan] VulkanMemoryAllocator::Create() - Initializing memory management\n");
		
		physical_device = physical_dev;
		device = logical_device;
		
		// Query physical device memory properties
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
		
		printf("[Vulkan] Memory types: %u\n", memory_properties.memoryTypeCount);
		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
			printf("[Vulkan]   Type %u: flags=0x%x, heap=%u\n", 
			       i, memory_properties.memoryTypes[i].propertyFlags, 
			       memory_properties.memoryTypes[i].heapIndex);
		}
		
		printf("[Vulkan]   Memory heaps: %u\n", memory_properties.memoryHeapCount);
		for (uint32_t i = 0; i < memory_properties.memoryHeapCount; ++i) {
			printf("[Vulkan]     Heap %u: size=%llu bytes (%.2f GB)\n", 
			       i, memory_properties.memoryHeaps[i].size,
			       memory_properties.memoryHeaps[i].size / (1024.0 * 1024.0 * 1024.0));
		}
		
		printf("[Vulkan] VulkanMemoryAllocator::Create() - Success!\n");
		return true;
	}
	
	void Destroy()
	{
		// Cleanup is handled by device destruction
		printf("[Vulkan] VulkanMemoryAllocator::Destroy() - Memory management shutdown\n");
	}
	
	// Helper: Find memory type index
	uint32_t Find_Memory_Type(uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
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

class VulkanRenderPass
{
public:
	VkRenderPass handle = VK_NULL_HANDLE;
	VkFormat color_format = VK_FORMAT_UNDEFINED;
	VkFormat depth_format = VK_FORMAT_UNDEFINED;
	
	bool Create(VkDevice device, VkFormat color_fmt, VkFormat depth_fmt)
	{
		// Phase 39.3 Stage 2: Implement render pass creation
		printf("[Vulkan] VulkanRenderPass::Create() - Creating render pass\n");
		
		color_format = color_fmt;
		depth_format = depth_fmt;
		
		// Color attachment description
		VkAttachmentDescription color_attachment{};
		color_attachment.format = color_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		// Color attachment reference
		VkAttachmentReference color_ref{};
		color_ref.attachment = 0;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		// Subpass description (color only for now - depth will be added in Phase 40)
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_ref;
		subpass.pDepthStencilAttachment = nullptr;  // No depth attachment yet
		
		// Subpass dependency
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
		// Create render pass (color attachment only)
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
		
		printf("[Vulkan] VulkanRenderPass::Create() - Success! Render pass created\n");
		return true;
	}
	
	void Destroy(VkDevice device)
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, handle, nullptr);
			printf("[Vulkan] VulkanRenderPass::Destroy() - Render pass destroyed\n");
		}
		handle = VK_NULL_HANDLE;
	}
};

// Command buffer management (frame buffering support)
class VulkanCommandBuffer
{
public:
	VkCommandPool pool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> buffers;
	std::vector<VkFence> fences;
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	uint32_t current_frame = 0;
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;  // Double buffering
	
	bool Create(VkDevice device, uint32_t queue_family_index)
	{
		// Phase 39.3 Stage 2: Create command pool
		printf("[Vulkan] VulkanCommandBuffer::Create() - Creating command pool\n");
		
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = queue_family_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		VkResult result = vkCreateCommandPool(device, &pool_info, nullptr, &pool);
		if (result != VK_SUCCESS) {
			printf("[Vulkan] ERROR: Failed to create command pool (result=%d)\n", result);
			return false;
		}
		
		// Allocate command buffers (one per frame in flight)
		buffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		
		result = vkAllocateCommandBuffers(device, &alloc_info, buffers.data());
		if (result != VK_SUCCESS) {
			printf("[Vulkan] ERROR: Failed to allocate command buffers (result=%d)\n", result);
			return false;
		}
		
		// Create synchronization primitives
		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signaled
		
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		fences.resize(MAX_FRAMES_IN_FLIGHT);
		image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
		
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			result = vkCreateFence(device, &fence_info, nullptr, &fences[i]);
			if (result != VK_SUCCESS) {
				printf("[Vulkan] ERROR: Failed to create fence %u (result=%d)\n", i, result);
				return false;
			}
			
			result = vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]);
			if (result != VK_SUCCESS) {
				printf("[Vulkan] ERROR: Failed to create semaphore %u (result=%d)\n", i, result);
				return false;
			}
			
			result = vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]);
			if (result != VK_SUCCESS) {
				printf("[Vulkan] ERROR: Failed to create semaphore %u (result=%d)\n", i, result);
				return false;
			}
		}
		
		printf("[Vulkan] VulkanCommandBuffer::Create() - Success! Pool and synchronization objects created\n");
		return true;
	}
	
	void Destroy(VkDevice device)
	{
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			vkDestroySemaphore(device, image_available_semaphores[i], nullptr);
			vkDestroySemaphore(device, render_finished_semaphores[i], nullptr);
			vkDestroyFence(device, fences[i], nullptr);
		}
		
		if (pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, pool, nullptr);
			printf("[Vulkan] VulkanCommandBuffer::Destroy() - Command pool destroyed\n");
		}
		pool = VK_NULL_HANDLE;
	}
	
	VkCommandBuffer Begin_Frame(VkDevice device)
	{
		// Wait for fence
		vkWaitForFences(device, 1, &fences[current_frame], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &fences[current_frame]);
		
		// Reset command buffer
		vkResetCommandBuffer(buffers[current_frame], 0);
		
		// Begin recording
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(buffers[current_frame], &begin_info);
		return buffers[current_frame];
	}
	
	void End_Frame(VkDevice device, VkQueue queue)
	{
		vkEndCommandBuffer(buffers[current_frame]);
		
		// Submit
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &buffers[current_frame];
		
		vkQueueSubmit(queue, 1, &submit_info, fences[current_frame]);
		
		// Next frame
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
};

// Static member initialization
std::unique_ptr<VulkanInstance> VulkanGraphicsBackend::s_instance = nullptr;
std::unique_ptr<VulkanPhysicalDevice> VulkanGraphicsBackend::s_physical_device = nullptr;
std::unique_ptr<VulkanDevice> VulkanGraphicsBackend::s_device = nullptr;
std::unique_ptr<VulkanSwapchain> VulkanGraphicsBackend::s_swapchain = nullptr;
std::unique_ptr<VulkanMemoryAllocator> VulkanGraphicsBackend::s_memory_allocator = nullptr;
std::unique_ptr<VulkanRenderPass> VulkanGraphicsBackend::s_render_pass = nullptr;

// VulkanCommandBuffer is internal to cpp, managed separately
static std::unique_ptr<VulkanCommandBuffer> s_command_buffer = nullptr;

bool VulkanGraphicsBackend::s_initialized = false;
bool VulkanGraphicsBackend::s_in_scene = false;

/**
 * Initialization & Lifecycle
 */

bool VulkanGraphicsBackend::Init(void* window_handle, bool debug_mode)
{
	if (s_initialized) {
		printf("[Vulkan] Already initialized\n");
		return true;
	}
	
	printf("[Vulkan] Phase 39.3 Stage 1: Initializing Vulkan graphics backend...\n");
	
	// Phase 39.3 Stage 1: Create Vulkan instance
	s_instance = std::make_unique<VulkanInstance>();
	if (!s_instance->Create(debug_mode)) {
		printf("[Vulkan] ERROR: Failed to create Vulkan instance\n");
		return false;
	}
	
	// Phase 39.3 Stage 1: Select physical device
	s_physical_device = std::make_unique<VulkanPhysicalDevice>();
	if (!s_physical_device->Select(s_instance->handle)) {
		printf("[Vulkan] ERROR: Failed to select physical device\n");
		return false;
	}
	
	// Phase 39.3 Stage 1: Create logical device
	s_device = std::make_unique<VulkanDevice>();
	if (!s_device->Create(s_physical_device->handle)) {
		printf("[Vulkan] ERROR: Failed to create logical device\n");
		return false;
	}
	
	// Phase 39.3 Stage 1: Create swapchain
	s_swapchain = std::make_unique<VulkanSwapchain>();
	if (!s_swapchain->Create(s_device->handle, s_physical_device->handle, 
	                         s_instance->handle, window_handle, 1280, 1024)) {
		printf("[Vulkan] ERROR: Failed to create swapchain\n");
		return false;
	}
	
	// Phase 39.3 Stage 2: Create memory allocator
	s_memory_allocator = std::make_unique<VulkanMemoryAllocator>();
	if (!s_memory_allocator->Create(s_instance->handle, s_physical_device->handle, 
	                                s_device->handle)) {
		printf("[Vulkan] ERROR: Failed to create memory allocator\n");
		return false;
	}
	
	// Phase 39.3 Stage 2: Create render pass
	s_render_pass = std::make_unique<VulkanRenderPass>();
	if (!s_render_pass->Create(s_device->handle, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT)) {
		printf("[Vulkan] ERROR: Failed to create render pass\n");
		return false;
	}
	
	// Phase 39.3 Stage 2.5: Create framebuffers for swapchain images
	if (!s_swapchain->CreateFramebuffers(s_device->handle, VK_FORMAT_B8G8R8A8_UNORM, s_render_pass->handle, VK_NULL_HANDLE)) {
		printf("[Vulkan] ERROR: Failed to create framebuffers\n");
		return false;
	}
	s_swapchain->UpdateCurrentFramebuffer();
	
	// Phase 39.3 Stage 2: Create command buffers
	s_command_buffer = std::make_unique<VulkanCommandBuffer>();
	if (!s_command_buffer->Create(s_device->handle, s_device->graphics_queue_family)) {
		printf("[Vulkan] ERROR: Failed to create command buffers\n");
		return false;
	}
	
	s_initialized = true;
	printf("[Vulkan] Initialization complete\n");
	return true;
}

void VulkanGraphicsBackend::Shutdown()
{
	if (!s_initialized) {
		return;
	}
	
	printf("[Vulkan] Shutting down...\n");
	
	// Wait for GPU idle before destroying resources
	if (s_device && s_device->handle != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(s_device->handle);
	}
	
	// Destroy in reverse order of creation
	if (s_command_buffer) {
		s_command_buffer->Destroy(s_device ? s_device->handle : VK_NULL_HANDLE);
	}
	
	if (s_render_pass) {
		s_render_pass->Destroy(s_device ? s_device->handle : VK_NULL_HANDLE);
	}
	
	if (s_memory_allocator) {
		s_memory_allocator->Destroy();
	}
	
	if (s_swapchain) {
		s_swapchain->Destroy(s_device ? s_device->handle : VK_NULL_HANDLE,
		                     s_instance ? s_instance->handle : VK_NULL_HANDLE);
	}
	
	if (s_device) {
		s_device->Destroy();
	}
	
	if (s_physical_device) {
		// Physical device is not owned by us, no destroy needed
		s_physical_device.reset();
	}
	
	if (s_instance) {
		s_instance->Destroy();
	}
	
	s_initialized = false;
	s_in_scene = false;
	
	printf("[Vulkan] Shutdown complete\n");
}

bool VulkanGraphicsBackend::Is_Initialized()
{
	return s_initialized;
}

/**
 * Frame Rendering (Priority 1: CRITICAL)
 */

void VulkanGraphicsBackend::Begin_Scene()
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer || s_command_buffer->buffers.empty()) {
		printf("[Vulkan] Begin_Scene: No command buffer available\n");
		return;
	}
	
	s_in_scene = true;
	
	// Begin command buffer recording
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	VkResult result = vkBeginCommandBuffer(s_command_buffer->buffers[s_command_buffer->current_frame], &begin_info);
	if (result != VK_SUCCESS) {
		printf("[Vulkan] ERROR: vkBeginCommandBuffer failed (result=%d)\n", result);
		s_in_scene = false;
		return;
	}
	
	// Begin render pass
	if (!s_render_pass || !s_swapchain) {
		printf("[Vulkan] Begin_Scene: No render pass or swapchain available\n");
		s_in_scene = false;
		return;
	}
	
	VkRenderPassBeginInfo render_pass_info{};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = s_render_pass->handle;
	render_pass_info.framebuffer = s_swapchain->current_framebuffer;
	render_pass_info.renderArea.offset = {0, 0};
	render_pass_info.renderArea.extent = s_swapchain->extent;
	
	// Clear color (black)
	VkClearValue clear_values[2]{};
	clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clear_values[1].depthStencil = {1.0f, 0};
	render_pass_info.clearValueCount = 2;
	render_pass_info.pClearValues = clear_values;
	
	vkCmdBeginRenderPass(s_command_buffer->buffers[s_command_buffer->current_frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	
	printf("[Vulkan] Begin_Scene() - Scene rendering started\n");
}

void VulkanGraphicsBackend::End_Scene(bool flip_frame)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] End_Scene: No command buffer available\n");
		return;
	}
	
	s_in_scene = false;
	
	// End render pass
	vkCmdEndRenderPass(s_command_buffer->buffers[s_command_buffer->current_frame]);
	
	// End command buffer recording
	VkResult result = vkEndCommandBuffer(s_command_buffer->buffers[s_command_buffer->current_frame]);
	if (result != VK_SUCCESS) {
		printf("[Vulkan] ERROR: vkEndCommandBuffer failed (result=%d)\n", result);
		return;
	}
	
	// Submit command buffer if flip_frame requested
	if (flip_frame && s_device->graphics_queue != VK_NULL_HANDLE) {
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &s_command_buffer->buffers[s_command_buffer->current_frame];
		
		result = vkQueueSubmit(s_device->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		if (result != VK_SUCCESS) {
			printf("[Vulkan] ERROR: vkQueueSubmit failed (result=%d)\n", result);
			return;
		}
		
		// Wait for queue to finish (simple synchronization)
		vkQueueWaitIdle(s_device->graphics_queue);
	}
	
	printf("[Vulkan] End_Scene() - Scene rendering ended (flip_frame=%d)\n", flip_frame);
}

void VulkanGraphicsBackend::Clear(bool clear_color, bool clear_z_stencil, 
                                  const float* color, float dest_alpha, 
                                  float z, unsigned int stencil)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Clear: No command buffer available\n");
		return;
	}
	
	std::vector<VkClearAttachment> attachments;
	
	// Clear color attachment
	if (clear_color && color) {
		VkClearAttachment color_clear{};
		color_clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_clear.colorAttachment = 0;
		color_clear.clearValue.color.float32[0] = color[0];
		color_clear.clearValue.color.float32[1] = color[1];
		color_clear.clearValue.color.float32[2] = color[2];
		color_clear.clearValue.color.float32[3] = dest_alpha;
		attachments.push_back(color_clear);
	}
	
	// Clear depth/stencil attachment
	if (clear_z_stencil) {
		VkClearAttachment depth_clear{};
		depth_clear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (stencil != ~0u) {
			depth_clear.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		depth_clear.colorAttachment = 1;
		depth_clear.clearValue.depthStencil.depth = z;
		depth_clear.clearValue.depthStencil.stencil = stencil;
		attachments.push_back(depth_clear);
	}
	
	if (!attachments.empty()) {
		VkClearRect rect{};
		rect.rect.offset = {0, 0};
		rect.rect.extent = s_swapchain->extent;
		rect.baseArrayLayer = 0;
		rect.layerCount = 1;
		
		vkCmdClearAttachments(s_command_buffer->buffers[s_command_buffer->current_frame], attachments.size(), attachments.data(), 1, &rect);
	}
	
	printf("[Vulkan] Clear() - Cleared (color=%d, z=%d)\n", clear_color, clear_z_stencil);
}

void VulkanGraphicsBackend::Present()
{
	if (!s_initialized || s_in_scene) {
		return; // Can only present when NOT in scene
	}
	
	if (!s_device || !s_device->graphics_queue || !s_swapchain) {
		printf("[Vulkan] Present: Missing device, queue, or swapchain\n");
		return;
	}
	
	// Present the swapchain image
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 0; // Simplified: no wait semaphores
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &s_swapchain->handle;
	present_info.pImageIndices = &s_swapchain->current_image_index;
	
	VkResult result = vkQueuePresentKHR(s_device->graphics_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		printf("[Vulkan] Present: Swapchain needs recreation (result=%d)\n", result);
		// In full implementation, would trigger swapchain recreation
	} else if (result != VK_SUCCESS) {
		printf("[Vulkan] ERROR: vkQueuePresentKHR failed (result=%d)\n", result);
	}
	
	// Advance to next swapchain image
	s_swapchain->current_image_index = (s_swapchain->current_image_index + 1) % s_swapchain->images.size();
	s_swapchain->UpdateCurrentFramebuffer();
	
	printf("[Vulkan] Present() - Frame presented (image_index=%u)\n", s_swapchain->current_image_index);
}

/**
 * Draw Operations (Priority 1: CRITICAL)
 */

void VulkanGraphicsBackend::Draw_Primitive(unsigned primitive_type, 
                                           unsigned short start_vertex, 
                                           unsigned short vertex_count)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Draw_Primitive: No command buffer available\n");
		return;
	}
	
	// Map D3D primitive type to Vulkan topology
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	switch (primitive_type) {
		case 1: // D3DPT_POINTLIST
			topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case 2: // D3DPT_LINELIST
			topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case 3: // D3DPT_LINESTRIP
			topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case 4: // D3DPT_TRIANGLELIST
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case 5: // D3DPT_TRIANGLESTRIP
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		case 6: // D3DPT_TRIANGLEFAN
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			break;
	}
	
	// Draw vertices
	vkCmdDraw(s_command_buffer->buffers[s_command_buffer->current_frame], vertex_count, 1, start_vertex, 0);
	
	printf("[Vulkan] Draw_Primitive() - Drawn %u vertices (type=%u, start=%u)\n", vertex_count, primitive_type, start_vertex);
}

void VulkanGraphicsBackend::Draw_Indexed_Primitive(unsigned primitive_type,
                                                   unsigned short start_index,
                                                   unsigned short polygon_count,
                                                   unsigned short min_vertex_index,
                                                   unsigned short vertex_count)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Draw_Indexed_Primitive: No command buffer available\n");
		return;
	}
	
	// Map D3D primitive type to Vulkan topology
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	switch (primitive_type) {
		case 1: // D3DPT_POINTLIST
			topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case 2: // D3DPT_LINELIST
			topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case 3: // D3DPT_LINESTRIP
			topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case 4: // D3DPT_TRIANGLELIST
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case 5: // D3DPT_TRIANGLESTRIP
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		case 6: // D3DPT_TRIANGLEFAN
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			break;
	}
	
	// Convert polygon count to index count (3 indices per triangle for triangle list)
	unsigned index_count = polygon_count * 3;
	
	// Draw indexed vertices
	vkCmdDrawIndexed(s_command_buffer->buffers[s_command_buffer->current_frame], index_count, 1, start_index, min_vertex_index, 0);
	
	printf("[Vulkan] Draw_Indexed_Primitive() - Drawn %u polygons/%u indices (type=%u)\n", polygon_count, index_count, primitive_type);
}

/**
 * Render State Management (Priority 1: CRITICAL)
 */

void VulkanGraphicsBackend::Set_Render_State(unsigned state_type, unsigned value)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Render_State: No command buffer available\n");
		return;
	}
	
	// Map D3DRENDERSTATETYPE enum values to Vulkan commands
	// This is a simplified mapping - full implementation would handle all D3D render states
	switch (state_type) {
		case 7: // D3DRS_ZENABLE
			printf("[Vulkan] Set_Render_State: D3DRS_ZENABLE = %u\n", value);
			// Handled in pipeline creation
			break;
			
		case 14: // D3DRS_ZWRITEENABLE
			printf("[Vulkan] Set_Render_State: D3DRS_ZWRITEENABLE = %u\n", value);
			// Handled in pipeline creation
			break;
			
		case 15: // D3DRS_ALPHATESTENABLE
			printf("[Vulkan] Set_Render_State: D3DRS_ALPHATESTENABLE = %u\n", value);
			// Handled in pipeline creation
			break;
			
		case 16: // D3DRS_SRCBLEND
			printf("[Vulkan] Set_Render_State: D3DRS_SRCBLEND = %u\n", value);
			// Handled in pipeline creation
			break;
			
		case 17: // D3DRS_DESTBLEND
			printf("[Vulkan] Set_Render_State: D3DRS_DESTBLEND = %u\n", value);
			// Handled in pipeline creation
			break;
			
		case 25: // D3DRS_SHADEMODE
			printf("[Vulkan] Set_Render_State: D3DRS_SHADEMODE = %u\n", value);
			// Would affect rasterization state
			break;
			
		case 27: // D3DRS_AMBIENT
			printf("[Vulkan] Set_Render_State: D3DRS_AMBIENT = 0x%08x\n", value);
			// Would update shader constants
			break;
			
		case 28: // D3DRS_AMBIENTMATERIALSOURCE
			printf("[Vulkan] Set_Render_State: D3DRS_AMBIENTMATERIALSOURCE = %u\n", value);
			// Material binding
			break;
			
		case 131: // D3DRS_LIGHTING
			printf("[Vulkan] Set_Render_State: D3DRS_LIGHTING = %u\n", value);
			// Lighting shader enable
			break;
			
		default:
			printf("[Vulkan] Set_Render_State: Unknown state_type %u = %u\n", state_type, value);
			break;
	}
}

void VulkanGraphicsBackend::Set_Texture_Stage_State(unsigned stage, unsigned state_type, unsigned value)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Texture_Stage_State: No command buffer available\n");
		return;
	}
	
	// Map D3DTEXTURESTAGESTATETYPE enum values to Vulkan/shader commands
	// Simplified mapping - full implementation would handle all texture operations
	switch (state_type) {
		case 1: // D3DTSS_COLOROP
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_COLOROP = %u\n", stage, value);
			// Color blending operation - handled in shader or pipeline
			break;
			
		case 2: // D3DTSS_COLORARG1
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_COLORARG1 = %u\n", stage, value);
			// First color argument
			break;
			
		case 3: // D3DTSS_COLORARG2
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_COLORARG2 = %u\n", stage, value);
			// Second color argument
			break;
			
		case 4: // D3DTSS_ALPHAOP
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_ALPHAOP = %u\n", stage, value);
			// Alpha blending operation
			break;
			
		case 5: // D3DTSS_ALPHAARG1
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_ALPHAARG1 = %u\n", stage, value);
			// First alpha argument
			break;
			
		case 6: // D3DTSS_ALPHAARG2
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_ALPHAARG2 = %u\n", stage, value);
			// Second alpha argument
			break;
			
		case 7: // D3DTSS_BUMPENVMAT00
		case 8: // D3DTSS_BUMPENVMAT01
		case 9: // D3DTSS_BUMPENVMAT10
		case 10: // D3DTSS_BUMPENVMAT11
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u Bump map matrix element = %u\n", stage, value);
			// Bump mapping matrix
			break;
			
		case 13: // D3DTSS_TEXCOORDINDEX
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u D3DTSS_TEXCOORDINDEX = %u\n", stage, value);
			// Texture coordinate index
			break;
			
		case 16: // D3DTSS_ADDRESSU
		case 17: // D3DTSS_ADDRESSV
		case 18: // D3DTSS_ADDRESSW
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u Address mode = %u\n", stage, value);
			// Texture address mode (wrap, clamp, etc)
			break;
			
		case 20: // D3DTSS_MAGFILTER
		case 21: // D3DTSS_MINFILTER
		case 22: // D3DTSS_MIPFILTER
			printf("[Vulkan] Set_Texture_Stage_State: Stage %u Filter mode = %u\n", stage, value);
			// Texture filtering
			break;
			
		default:
			printf("[Vulkan] Set_Texture_Stage_State: Unknown stage_state %u (stage %u) = %u\n", state_type, stage, value);
			break;
	}
}

/**
 * Buffer & Texture Binding (Priority 2: HIGH)
 */

void VulkanGraphicsBackend::Set_Stream_Source(unsigned stream, const void* buffer, 
                                              unsigned stride)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Stream_Source: No command buffer available\n");
		return;
	}
	
	if (!buffer) {
		printf("[Vulkan] Set_Stream_Source: NULL buffer pointer\n");
		return;
	}
	
	// Bind vertex buffer to command buffer
	// In a full implementation, this would:
	// 1. Verify buffer is a valid VkBuffer
	// 2. Call vkCmdBindVertexBuffers with the buffer handle
	// For now, just log the binding
	VkBuffer vk_buffer = *reinterpret_cast<const VkBuffer*>(buffer);
	VkDeviceSize offset = 0;
	
	vkCmdBindVertexBuffers(s_command_buffer->buffers[s_command_buffer->current_frame], stream, 1, &vk_buffer, &offset);
	
	printf("[Vulkan] Set_Stream_Source() - Stream %u bound (stride=%u)\n", stream, stride);
}

void VulkanGraphicsBackend::Set_Indices(const void* buffer)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Indices: No command buffer available\n");
		return;
	}
	
	if (!buffer) {
		printf("[Vulkan] Set_Indices: NULL buffer pointer\n");
		return;
	}
	
	// Bind index buffer to command buffer
	// In a full implementation, this would verify buffer validity
	VkBuffer vk_buffer = *reinterpret_cast<const VkBuffer*>(buffer);
	VkDeviceSize offset = 0;
	
	// Assume 16-bit indices (D3DFMT_INDEX16) as default
	// Full implementation would allow specifying 32-bit indices
	vkCmdBindIndexBuffer(s_command_buffer->buffers[s_command_buffer->current_frame], vk_buffer, offset, VK_INDEX_TYPE_UINT16);
	
	printf("[Vulkan] Set_Indices() - Index buffer bound\n");
}

void VulkanGraphicsBackend::Set_Texture(unsigned stage, const void* texture)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Texture: No command buffer available\n");
		return;
	}
	
	if (!texture) {
		printf("[Vulkan] Set_Texture: NULL texture pointer (stage %u)\n", stage);
		return;
	}
	
	// Bind texture via descriptor sets
	// In a full implementation, this would:
	// 1. Lookup the descriptor set for the texture
	// 2. Call vkCmdBindDescriptorSets to bind it
	
	// For now, just log the binding and verify texture pointer validity
	printf("[Vulkan] Set_Texture() - Stage %u texture bound (ptr=%p)\n", stage, texture);
	
	// Store texture pointer for later reference
	if (s_device) {
		// In full implementation, would manage texture bindings in device state
	}
}

/**
 * Viewport & Scissor Management
 */

void VulkanGraphicsBackend::Set_Viewport(float x, float y, float width, float height, 
                                        float min_z, float max_z)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Viewport: No command buffer available\n");
		return;
	}
	
	// Set viewport
	VkViewport viewport{};
	viewport.x = x;
	viewport.y = y + height; // Vulkan has inverted Y compared to D3D
	viewport.width = width;
	viewport.height = -height; // Negative height to flip Y
	viewport.minDepth = min_z;
	viewport.maxDepth = max_z;
	
	vkCmdSetViewport(s_command_buffer->buffers[s_command_buffer->current_frame], 0, 1, &viewport);
	
	printf("[Vulkan] Set_Viewport() - Viewport set (%.0f,%.0f %.0fx%.0f, z: %.2f-%.2f)\n", 
		   x, y, width, height, min_z, max_z);
}

void VulkanGraphicsBackend::Set_Scissor(int x, int y, int width, int height)
{
	if (!s_initialized || !s_in_scene) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Scissor: No command buffer available\n");
		return;
	}
	
	// Set scissor rect
	VkRect2D scissor{};
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = width;
	scissor.extent.height = height;
	
	vkCmdSetScissor(s_command_buffer->buffers[s_command_buffer->current_frame], 0, 1, &scissor);
	
	printf("[Vulkan] Set_Scissor() - Scissor set (%d,%d %dx%d)\n", x, y, width, height);
}

/**
 * Transform Matrices (Priority 2: HIGH)
 */

void VulkanGraphicsBackend::Set_Transform(unsigned transform_type, const float* matrix)
{
	if (!s_initialized) {
		return;
	}
	
	if (!s_device || !s_command_buffer->buffers[s_command_buffer->current_frame]) {
		printf("[Vulkan] Set_Transform: No command buffer available\n");
		return;
	}
	
	if (!matrix) {
		printf("[Vulkan] Set_Transform: NULL matrix pointer\n");
		return;
	}
	
	// Map D3D transform types to shader constant updates
	// In full implementation, would:
	// 1. Create/update uniform buffer with matrix data
	// 2. Call vkCmdPushConstants or update descriptor set
	
	switch (transform_type) {
		case 2: // D3DTS_VIEW (0-3 for WORLD, VIEW, PROJECTION, TEXTURE0-TEXTURE7)
			printf("[Vulkan] Set_Transform() - View matrix set\n");
			break;
			
		case 3: // D3DTS_PROJECTION
			printf("[Vulkan] Set_Transform() - Projection matrix set\n");
			break;
			
		case 1: // D3DTS_WORLD
			printf("[Vulkan] Set_Transform() - World matrix set\n");
			break;
			
		default:
			printf("[Vulkan] Set_Transform() - Transform %u matrix set\n", transform_type);
			break;
	}
	
	// In full implementation, would push matrix via:
	// vkCmdPushConstants(cmd_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, offset, size, matrix);
}

/**
 * Device Capabilities & Queries
 */

void VulkanGraphicsBackend::Get_Device_Caps(void* caps_struct)
{
	if (!s_initialized) {
		return;
	}
	
	// Phase 39.3 Stage 3: Implement device capabilities
	// TODO: Query vkGetPhysicalDeviceProperties()
}

void VulkanGraphicsBackend::Get_Display_Mode(unsigned* width, unsigned* height)
{
	if (!s_initialized) {
		return;
	}
	
	// Phase 39.3 Stage 3: Implement display mode
	if (width) *width = 1280;
	if (height) *height = 1024;
}

/**
 * Static utility methods
 */

VkInstance VulkanGraphicsBackend::Get_VK_Instance()
{
	return s_instance ? s_instance->handle : VK_NULL_HANDLE;
}

VkPhysicalDevice VulkanGraphicsBackend::Get_VK_Physical_Device()
{
	return s_physical_device ? s_physical_device->handle : VK_NULL_HANDLE;
}

VkDevice VulkanGraphicsBackend::Get_VK_Device()
{
	return s_device ? s_device->handle : VK_NULL_HANDLE;
}

VkQueue VulkanGraphicsBackend::Get_Graphics_Queue()
{
	return s_device ? s_device->graphics_queue : VK_NULL_HANDLE;
}
