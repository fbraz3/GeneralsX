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

#pragma once

/**
 * Phase 39.3: VulkanGraphicsBackend
 * 
 * Replaces DirectX8Wrapper with native Vulkan implementation across all platforms.
 * This header defines the core Vulkan graphics backend interface that handles:
 * - Device creation and management
 * - Command buffer recording and submission
 * - Render pass and pipeline management
 * - Texture and buffer resource management
 * - Swapchain and presentation
 */

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <string>

// Forward declarations
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanDevice;
class VulkanSwapchain;
class VulkanMemoryAllocator;
class VulkanRenderPass;
class VulkanPipeline;

/**
 * VulkanGraphicsBackend
 * 
 * Main interface for Vulkan graphics operations. Replaces DX8Wrapper completely.
 * This class manages the entire Vulkan rendering pipeline:
 * 
 * 1. Instance & Device Management (Phase 39.3 Stage 1)
 *    - VkInstance creation with validation layers
 *    - Physical device selection and properties
 *    - Logical device creation with appropriate queues
 *    - Queue management (graphics, transfer, compute)
 * 
 * 2. Memory Management (Phase 39.3 Stage 2)
 *    - Vulkan Memory Allocator (VMA) integration
 *    - Buffer and image creation
 *    - Descriptor set management
 * 
 * 3. Rendering Pipeline (Phase 39.3 Stage 3)
 *    - Command pool and buffer management
 *    - Render pass setup
 *    - Pipeline cache management
 *    - Frame synchronization (semaphores, fences)
 * 
 * 4. D3D8 Method Implementation (Phase 39.3 Stage 4)
 *    - All 150+ D3D8 API calls mapped to Vulkan equivalents
 *    - Stateless rendering design
 *    - Material and texture binding
 */
class VulkanGraphicsBackend
{
public:
	/**
	 * Initialization & Lifecycle
	 */
	
	/**
	 * Initialize the Vulkan graphics backend
	 * 
	 * This replaces DX8Wrapper::Init() and performs:
	 * - Vulkan instance creation with validation layers
	 * - Physical device enumeration and selection
	 * - Logical device creation
	 * - Swapchain creation
	 * - Command pool setup
	 */
	static bool Init(void* window_handle, bool debug_mode = false);
	
	/**
	 * Shutdown the Vulkan graphics backend
	 * 
	 * Cleans up all Vulkan resources in proper order:
	 * - Wait for GPU idle
	 * - Destroy swapchain
	 * - Destroy device
	 * - Destroy instance
	 */
	static void Shutdown();
	
	/**
	 * Check if backend is initialized
	 */
	static bool Is_Initialized();
	
	/**
	 * Frame Rendering (Priority 1: CRITICAL)
	 */
	
	/**
	 * Begin scene rendering
	 * Replaces: IDirect3DDevice8::BeginScene()
	 * Maps to: vkBeginCommandBuffer() + vkCmdBeginRenderPass()
	 */
	static void Begin_Scene();
	
	/**
	 * End scene rendering
	 * Replaces: IDirect3DDevice8::EndScene()
	 * Maps to: vkCmdEndRenderPass() + vkEndCommandBuffer()
	 */
	static void End_Scene(bool flip_frame = true);
	
	/**
	 * Clear render targets
	 * Replaces: IDirect3DDevice8::Clear()
	 * Maps to: vkCmdClearAttachments()
	 */
	static void Clear(bool clear_color, bool clear_z_stencil, 
	                  const float* color, float dest_alpha = 0.0f, 
	                  float z = 1.0f, unsigned int stencil = 0);
	
	/**
	 * Present rendered frame to screen
	 * Replaces: IDirect3DDevice8::Present()
	 * Maps to: vkQueuePresentKHR()
	 */
	static void Present();
	
	/**
	 * Draw Operations (Priority 1: CRITICAL)
	 */
	
	/**
	 * Draw primitive (non-indexed)
	 * Replaces: IDirect3DDevice8::DrawPrimitive()
	 * Maps to: vkCmdDraw()
	 */
	static void Draw_Primitive(unsigned primitive_type, 
	                           unsigned short start_vertex, 
	                           unsigned short vertex_count);
	
	/**
	 * Draw indexed primitive
	 * Replaces: IDirect3DDevice8::DrawIndexedPrimitive()
	 * Maps to: vkCmdDrawIndexed()
	 */
	static void Draw_Indexed_Primitive(unsigned primitive_type,
	                                    unsigned short start_index,
	                                    unsigned short polygon_count,
	                                    unsigned short min_vertex_index = 0,
	                                    unsigned short vertex_count = 0);
	
	/**
	 * Render State Management (Priority 1: CRITICAL)
	 */
	
	/**
	 * Set render state
	 * Replaces: IDirect3DDevice8::SetRenderState()
	 * Maps to: VkPipeline state or shader uniform updates
	 */
	static void Set_Render_State(unsigned state_type, unsigned value);
	
	/**
	 * Set texture stage state
	 * Replaces: IDirect3DDevice8::SetTextureStageState()
	 * Maps to: Descriptor binding or shader constants
	 */
	static void Set_Texture_Stage_State(unsigned stage, unsigned state_type, unsigned value);
	
	/**
	 * Buffer & Texture Binding (Priority 2: HIGH)
	 */
	
	/**
	 * Set vertex buffer stream source
	 * Replaces: IDirect3DDevice8::SetStreamSource()
	 * Maps to: vkCmdBindVertexBuffers()
	 */
	static void Set_Stream_Source(unsigned stream, const void* buffer, 
	                               unsigned stride);
	
	/**
	 * Set index buffer
	 * Replaces: IDirect3DDevice8::SetIndices()
	 * Maps to: vkCmdBindIndexBuffer()
	 */
	static void Set_Indices(const void* buffer);
	
	/**
	 * Set texture for rendering
	 * Replaces: IDirect3DDevice8::SetTexture()
	 * Maps to: vkCmdBindDescriptorSets()
	 */
	static void Set_Texture(unsigned stage, const void* texture);
	
	/**
	 * Viewport & Scissor Management
	 */
	
	/**
	 * Set viewport
	 * Replaces: IDirect3DDevice8::SetViewport()
	 * Maps to: vkCmdSetViewport()
	 */
	static void Set_Viewport(float x, float y, float width, float height, 
	                         float min_z, float max_z);
	
	/**
	 * Set scissor rectangle
	 * Replaces: IDirect3DDevice8::SetScissorRect()
	 * Maps to: vkCmdSetScissor()
	 */
	static void Set_Scissor(int x, int y, int width, int height);
	
	/**
	 * Transform Matrices (Priority 2: HIGH)
	 */
	
	/**
	 * Set transform matrix
	 * Replaces: IDirect3DDevice8::SetTransform()
	 * Maps to: Shader uniform buffer updates
	 */
	static void Set_Transform(unsigned transform_type, const float* matrix);
	
	/**
	 * Device Capabilities & Queries
	 */
	
	/**
	 * Get device capabilities
	 * Replaces: IDirect3DDevice8::GetDeviceCaps()
	 * Maps to: vkGetPhysicalDeviceProperties()
	 */
	static void Get_Device_Caps(void* caps_struct);
	
	/**
	 * Get display mode
	 * Replaces: IDirect3DDevice8::GetDisplayMode()
	 * Maps to: VkSwapchainCreateInfoKHR extent
	 */
	static void Get_Display_Mode(unsigned* width, unsigned* height);
	
	/**
	 * Static utility methods for accessing backend components
	 */
	
	/**
	 * Get the current Vulkan instance
	 */
	static VkInstance Get_VK_Instance();
	
	/**
	 * Get the current Vulkan physical device
	 */
	static VkPhysicalDevice Get_VK_Physical_Device();
	
	/**
	 * Get the current Vulkan logical device
	 */
	static VkDevice Get_VK_Device();
	
	/**
	 * Get the graphics command queue
	 */
	static VkQueue Get_Graphics_Queue();
	
private:
	/**
	 * Internal state
	 */
	static std::unique_ptr<VulkanInstance> s_instance;
	static std::unique_ptr<VulkanPhysicalDevice> s_physical_device;
	static std::unique_ptr<VulkanDevice> s_device;
	static std::unique_ptr<VulkanSwapchain> s_swapchain;
	static std::unique_ptr<VulkanMemoryAllocator> s_memory_allocator;
	static std::unique_ptr<VulkanRenderPass> s_render_pass;
	
	static bool s_initialized;
	static bool s_in_scene;
	
	// Disable copy/move
	VulkanGraphicsBackend() = delete;
	VulkanGraphicsBackend(const VulkanGraphicsBackend&) = delete;
	VulkanGraphicsBackend& operator=(const VulkanGraphicsBackend&) = delete;
};
