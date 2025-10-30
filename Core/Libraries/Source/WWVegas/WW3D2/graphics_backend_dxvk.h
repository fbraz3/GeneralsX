/**
 * @file graphics_backend_dxvk.h
 * 
 * Vulkan Graphics Backend Implementation
 * 
 * Provides a thin wrapper around Vulkan API that implements the IGraphicsBackend interface.
 * Enables cross-platform graphics rendering on macOS (via MoltenVK), Linux, and Windows.
 * 
 * This is NOT a port of DXVK (DXVK is Windows/Linux-only). Instead, it's a direct Vulkan
 * wrapper that maps DirectX 8 graphics calls to Vulkan equivalents.
 * 
 * Architecture:
 * - Game Code → IGraphicsBackend (abstraction layer from Phase 38)
 * - DXVKGraphicsBackend ← IGraphicsBackend (Vulkan implementation, this file)
 * - Vulkan SDK → MoltenVK (macOS) / native Vulkan (Linux/Windows) → GPU
 * 
 * Phase: 39.2 (Vulkan Wrapper Implementation)
 * Target Timeline: 14-20 hours for full Phase 39 completion
 * 
 * Dependencies:
 * - vulkan.h (Vulkan SDK 1.4.328.1+)
 * - graphics_backend.h (IGraphicsBackend interface from Phase 38)
 * - MoltenVK (macOS only, included with Vulkan SDK)
 * - win32_compat.h (Windows types and compatibility layer)
 * 
 * Created: October 30, 2025
 * Author: GeneralsX Development Team
 */

#ifndef GRAPHICS_BACKEND_DXVK_H
#define GRAPHICS_BACKEND_DXVK_H

#include "graphics_backend.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <map>

// Forward declarations
struct DXVKTextureHandle;
struct DXVKBufferHandle;

// ============================================================================
// Type Aliases for Vulkan Memory Management
// ============================================================================

using VulkanTexturePtr = std::shared_ptr<DXVKTextureHandle>;
using VulkanBufferPtr = std::shared_ptr<DXVKBufferHandle>;

// ============================================================================
// Vulkan Texture Handle
// ============================================================================

/**
 * Internal representation of a texture in Vulkan.
 * Wraps VkImage, VkImageView, and memory management.
 */
struct DXVKTextureHandle {
    VkImage image;                          ///< Vulkan image handle
    VkImageView imageView;                  ///< Vulkan image view
    VkDeviceMemory memory;                  ///< GPU memory allocation
    VkSampler sampler;                      ///< Texture sampler
    unsigned int width;                     ///< Texture width in pixels
    unsigned int height;                    ///< Texture height in pixels
    VkFormat format;                        ///< Vulkan pixel format (e.g., VK_FORMAT_R8G8B8A8_UNORM)
    D3DFORMAT originalFormat;               ///< Original DirectX format for reference
    bool isRenderTarget;                    ///< True if texture is used as render target
    bool isDynamic;                         ///< True if texture is dynamic (CPU-updatable)
    
    DXVKTextureHandle()
        : image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE), memory(VK_NULL_HANDLE),
          sampler(VK_NULL_HANDLE), width(0), height(0), format(VK_FORMAT_UNDEFINED),
          originalFormat(D3DFMT_UNKNOWN), isRenderTarget(false), isDynamic(false) {}
};

// ============================================================================
// Vulkan Buffer Handle
// ============================================================================

/**
 * Internal representation of a vertex or index buffer in Vulkan.
 */
struct DXVKBufferHandle {
    VkBuffer buffer;                        ///< Vulkan buffer handle
    VkDeviceMemory memory;                  ///< GPU memory allocation
    unsigned int size;                      ///< Buffer size in bytes
    VkBufferUsageFlags usage;               ///< Buffer usage flags (VERTEX, INDEX, etc)
    bool isDynamic;                         ///< True if buffer is dynamic (CPU-updatable)
    void* stagingData;                      ///< Staging buffer for CPU access during lock
    
    DXVKBufferHandle()
        : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), size(0),
          usage(0), isDynamic(false), stagingData(nullptr) {}
};

// ============================================================================
// Vulkan Graphics Backend
// ============================================================================

/**
 * Vulkan graphics backend implementation.
 * 
 * Maps DirectX 8 graphics operations to Vulkan API calls.
 * Manages Vulkan instance, device, swapchain, command buffers, and frame resources.
 * 
 * Features:
 * - Multi-buffering with VkSwapchain
 * - Command buffer recording and submission
 * - Dynamic texture and buffer management
 * - Render state management (blend modes, depth testing, etc)
 * - Lighting and material support via shader uniforms
 * - Viewport and transform matrix management
 * 
 * Usage:
 * 1. Call Initialize() to set up Vulkan device and swapchain
 * 2. BeginScene() → draw calls → EndScene()
 * 3. Present() to display frame
 * 4. Call Shutdown() at application exit
 */
class DXVKGraphicsBackend : public IGraphicsBackend {
public:
    // ========================================================================
    // Lifecycle Management
    // ========================================================================
    
    DXVKGraphicsBackend();
    virtual ~DXVKGraphicsBackend();
    
    /**
     * Initialize Vulkan device and swapchain.
     * Called at application startup after window creation.
     */
    virtual HRESULT Initialize() override;
    
    /**
     * Clean up Vulkan resources.
     * Called at application shutdown.
     */
    virtual HRESULT Shutdown() override;
    
    /**
     * Reset device (e.g., after display mode change).
     */
    virtual HRESULT Reset() override;
    
    // ========================================================================
    // Scene Rendering (Frame Lifecycle)
    // ========================================================================
    
    /**
     * Begin a new frame.
     * Acquires swapchain image and records frame command buffer.
     */
    virtual HRESULT BeginScene() override;
    
    /**
     * End frame rendering (submit command buffer).
     */
    virtual HRESULT EndScene() override;
    
    /**
     * Present rendered frame to display.
     */
    virtual HRESULT Present() override;
    
    /**
     * Clear render target and depth buffer.
     */
    virtual HRESULT Clear(
        unsigned int count,
        const D3DRECT* rects,
        unsigned int flags,
        D3DCOLOR color,
        float z,
        unsigned int stencil
    ) override;
    
    // ========================================================================
    // Texture Management
    // ========================================================================
    
    /**
     * Create a texture.
     * Allocates VkImage and VkImageView.
     */
    virtual HRESULT CreateTexture(
        unsigned int width,
        unsigned int height,
        unsigned int levels,
        unsigned int usage,
        D3DFORMAT format,
        void** texture
    ) override;
    
    /**
     * Release a texture.
     * Frees VkImage and VkImageView resources.
     */
    virtual HRESULT ReleaseTexture(void* texture) override;
    
    /**
     * Set active texture for rendering.
     * Updates descriptor set with texture's VkImageView.
     */
    virtual HRESULT SetTexture(unsigned int stage, void* texture) override;
    
    /**
     * Get currently bound texture.
     */
    virtual HRESULT GetTexture(unsigned int stage, void** texture) override;
    
    /**
     * Lock texture for CPU access.
     * For dynamic textures, returns staging buffer pointer.
     */
    virtual HRESULT LockTexture(void* texture, void** data, unsigned int* pitch) override;
    
    /**
     * Unlock texture and upload CPU modifications to GPU.
     */
    virtual HRESULT UnlockTexture(void* texture) override;
    
    // ========================================================================
    // Render State Management
    // ========================================================================
    
    /**
     * Set render state (depth testing, blending, culling, etc).
     * Maps D3DRENDERSTATETYPE to Vulkan pipeline state.
     */
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override;
    
    /**
     * Get current render state value.
     */
    virtual HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) override;
    
    /**
     * Set texture operation (blend mode).
     * Maps D3DTEXTUREOP to shader uniforms.
     */
    virtual HRESULT SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) override;
    
    // ========================================================================
    // Vertex Buffer Management
    // ========================================================================
    
    /**
     * Create a vertex buffer.
     * Allocates VkBuffer with VK_BUFFER_USAGE_VERTEX_BUFFER_BIT.
     */
    virtual HRESULT CreateVertexBuffer(
        unsigned int size,
        unsigned int usage,
        unsigned int format,
        void** buffer
    ) override;
    
    /**
     * Release a vertex buffer.
     */
    virtual HRESULT ReleaseVertexBuffer(void* buffer) override;
    
    /**
     * Lock vertex buffer for CPU access.
     */
    virtual HRESULT LockVertexBuffer(void* buffer, void** data, unsigned int flags) override;
    
    /**
     * Unlock vertex buffer.
     */
    virtual HRESULT UnlockVertexBuffer(void* buffer) override;
    
    /**
     * Set active vertex buffer stream.
     */
    virtual HRESULT SetStreamSource(
        unsigned int stream,
        void* buffer,
        unsigned int stride
    ) override;
    
    // ========================================================================
    // Index Buffer Management
    // ========================================================================
    
    /**
     * Create an index buffer.
     */
    virtual HRESULT CreateIndexBuffer(
        unsigned int size,
        D3DFORMAT format,
        void** buffer
    ) override;
    
    /**
     * Release an index buffer.
     */
    virtual HRESULT ReleaseIndexBuffer(void* buffer) override;
    
    /**
     * Lock index buffer for CPU access.
     */
    virtual HRESULT LockIndexBuffer(void* buffer, void** data, unsigned int flags) override;
    
    /**
     * Unlock index buffer.
     */
    virtual HRESULT UnlockIndexBuffer(void* buffer) override;
    
    /**
     * Set active index buffer.
     */
    virtual HRESULT SetIndices(void* buffer) override;
    
    // ========================================================================
    // Drawing Operations
    // ========================================================================
    
    /**
     * Draw vertices (non-indexed).
     * Records vkCmdDraw into current command buffer.
     */
    virtual HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int startVertex,
        unsigned int primitiveCount
    ) override;
    
    /**
     * Draw indexed vertices.
     * Records vkCmdDrawIndexed into current command buffer.
     */
    virtual HRESULT DrawIndexedPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int baseVertexIndex,
        unsigned int minVertexIndex,
        unsigned int numVertices,
        unsigned int startIndex,
        unsigned int primitiveCount
    ) override;
    
    // ========================================================================
    // Viewport and Transform
    // ========================================================================
    
    /**
     * Set viewport (vkCmdSetViewport).
     */
    virtual HRESULT SetViewport(
        unsigned int x,
        unsigned int y,
        unsigned int width,
        unsigned int height,
        float minZ,
        float maxZ
    ) override;
    
    /**
     * Set transformation matrix (world/view/projection).
     * Uploaded to shader via push constants or descriptor sets.
     */
    virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const float* matrix) override;
    
    /**
     * Get transformation matrix.
     */
    virtual HRESULT GetTransform(D3DTRANSFORMSTATETYPE state, float* matrix) override;
    
    // ========================================================================
    // Lighting
    // ========================================================================
    
    /**
     * Enable/disable fixed-function lighting.
     * In Vulkan, this is emulated via shader uniforms.
     */
    virtual HRESULT EnableLighting(bool enable) override;
    
    /**
     * Set light source.
     * Uploaded to shader via descriptor sets.
     */
    virtual HRESULT SetLight(unsigned int index, const D3DLIGHT8* light) override;
    
    /**
     * Enable/disable individual light.
     */
    virtual HRESULT LightEnable(unsigned int index, bool enable) override;
    
    /**
     * Set material properties.
     * Uploaded to shader via descriptor sets.
     */
    virtual HRESULT SetMaterial(const D3DMATERIAL8* material) override;
    
    /**
     * Set ambient light color.
     * Uploaded to shader via descriptor sets.
     */
    virtual HRESULT SetAmbient(D3DCOLOR color) override;
    
    // ========================================================================
    // Utility / Debug
    // ========================================================================
    
    /**
     * Get backend name ("DXVK/Vulkan" or "Vulkan/MoltenVK" on macOS).
     */
    virtual const char* GetBackendName() const override;
    
    /**
     * Get last error code.
     */
    virtual HRESULT GetLastError() const override;
    
    /**
     * Enable debug output (validation layers, debug callbacks).
     */
    virtual void SetDebugOutput(bool enable) override;

private:
    // ========================================================================
    // Private Vulkan Instance and Device Management
    // ========================================================================
    
    /**
     * Create Vulkan instance (VkInstance).
     * Enables validation layers in debug builds.
     */
    HRESULT CreateInstance();
    
    /**
     * Destroy Vulkan instance.
     */
    void DestroyInstance();
    
    /**
     * Select physical device and create logical device (VkDevice).
     * Queries device capabilities and selects optimal GPU.
     */
    HRESULT CreateDevice();
    
    /**
     * Destroy logical device.
     */
    void DestroyDevice();
    
    /**
     * Create window surface (VkSurfaceKHR).
     * Platform-specific: uses vkCreateMetalSurfaceEXT on macOS.
     */
    HRESULT CreateSurface();
    
    /**
     * Destroy surface.
     */
    void DestroySurface();
    
    /**
     * Create swapchain (VkSwapchainKHR) for presentation.
     * Handles image format selection and buffer creation.
     */
    HRESULT CreateSwapchain();
    
    /**
     * Recreate swapchain (on resize or reset).
     */
    HRESULT RecreateSwapchain();
    
    /**
     * Destroy swapchain.
     */
    void DestroySwapchain();
    
    /**
     * Create render pass (VkRenderPass).
     * Defines attachment layout and subpass structure.
     */
    HRESULT CreateRenderPass();
    
    /**
     * Destroy render pass.
     */
    void DestroyRenderPass();
    
    /**
     * Create framebuffers (VkFramebuffer) for each swapchain image.
     */
    HRESULT CreateFramebuffers();
    
    /**
     * Destroy framebuffers.
     */
    void DestroyFramebuffers();
    
    /**
     * Create command pool (VkCommandPool) for command buffer allocation.
     */
    HRESULT CreateCommandPool();
    
    /**
     * Destroy command pool.
     */
    void DestroyCommandPool();
    
    /**
     * Allocate command buffers (one per swapchain image).
     */
    HRESULT AllocateCommandBuffers();
    
    /**
     * Free command buffers.
     */
    void FreeCommandBuffers();
    
    /**
     * Create synchronization objects (semaphores and fences).
     * Used for frame pacing and GPU-CPU synchronization.
     */
    HRESULT CreateSyncObjects();
    
    /**
     * Destroy synchronization objects.
     */
    void DestroySyncObjects();
    
    /**
     * Create pipeline cache for shader compilation optimization.
     */
    HRESULT CreatePipelineCache();
    
    /**
     * Destroy pipeline cache.
     */
    void DestroyPipelineCache();
    
    // ========================================================================
    // Private Shader and Pipeline Management
    // ========================================================================
    
    /**
     * Create shader modules from SPIR-V bytecode.
     * Loads default shaders for fixed-function emulation.
     */
    HRESULT CreateShaderModules();
    
    /**
     * Destroy shader modules.
     */
    void DestroyShaderModules();
    
    /**
     * Create graphics pipeline (VkPipeline).
     * Defines vertex input, rasterization, depth-stencil, and color blending.
     */
    HRESULT CreateGraphicsPipeline();
    
    /**
     * Recreate graphics pipeline (on render state changes).
     */
    HRESULT RecreateGraphicsPipeline();
    
    /**
     * Destroy graphics pipeline.
     */
    void DestroyGraphicsPipeline();
    
    // ========================================================================
    // Private Memory Allocation Helpers
    // ========================================================================
    
    /**
     * Allocate GPU memory for images/buffers.
     * Selects appropriate memory type based on requirements.
     */
    HRESULT AllocateMemory(
        VkMemoryRequirements requirements,
        VkMemoryPropertyFlags properties,
        VkDeviceMemory& memory
    );
    
    /**
     * Find suitable memory type index.
     */
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    // ========================================================================
    // Private Platform-Specific Surface Creation
    // ========================================================================
    
    /**
     * Create Metal surface (macOS).
     */
    #ifdef __APPLE__
        HRESULT CreateSurfaceMacOS();
    #endif
    
    /**
     * Create Win32 surface (Windows).
     */
    #ifdef _WIN32
        HRESULT CreateSurfaceWindows();
    #endif
    
    /**
     * Create X11 surface (Linux).
     */
    #ifdef __linux__
        HRESULT CreateSurfaceLinux();
    #endif
    
    // ========================================================================
    // Private Command Buffer Recording Helpers
    // ========================================================================
    
    /**
     * Record frame commands into current command buffer.
     * Called between BeginScene() and EndScene().
     */
    void RecordFrameCommands();
    
    /**
     * Submit recorded commands to queue.
     * Called from EndScene().
     */
    HRESULT SubmitCommands();
    
    /**
     * Convert DirectX format to Vulkan format.
     */
    VkFormat ConvertD3DFormatToVulkan(D3DFORMAT format);
    
    /**
     * Convert DirectX primitive type to Vulkan topology.
     */
    VkPrimitiveTopology ConvertPrimitiveType(D3DPRIMITIVETYPE type);
    
    // ========================================================================
    // Vulkan Instance and Device Members
    // ========================================================================
    
    VkInstance m_instance;                  ///< Vulkan instance
    VkPhysicalDevice m_physicalDevice;      ///< Selected GPU
    VkDevice m_device;                      ///< Logical device
    VkQueue m_graphicsQueue;                ///< Graphics queue
    uint32_t m_graphicsQueueFamily;         ///< Graphics queue family index
    
    // ========================================================================
    // Vulkan Swapchain and Presentation Members
    // ========================================================================
    
    VkSurfaceKHR m_surface;                 ///< Window surface
    VkSwapchainKHR m_swapchain;             ///< Swapchain for presentation
    std::vector<VkImage> m_swapchainImages; ///< Swapchain images
    std::vector<VkImageView> m_swapchainImageViews;  ///< Image views
    std::vector<VkFramebuffer> m_framebuffers;       ///< Framebuffers
    VkFormat m_swapchainFormat;             ///< Chosen swapchain format
    VkExtent2D m_swapchainExtent;           ///< Swapchain dimensions
    uint32_t m_currentSwapchainIndex;       ///< Current frame swapchain image
    
    // ========================================================================
    // Vulkan Rendering Members
    // ========================================================================
    
    VkRenderPass m_renderPass;              ///< Main render pass
    std::vector<VkCommandBuffer> m_commandBuffers;  ///< Per-frame command buffers
    VkCommandPool m_commandPool;            ///< Command buffer pool
    VkPipeline m_graphicsPipeline;          ///< Graphics pipeline
    VkPipelineLayout m_pipelineLayout;      ///< Pipeline layout
    VkPipelineCache m_pipelineCache;        ///< Pipeline cache for optimization
    
    // ========================================================================
    // Vulkan Synchronization Members
    // ========================================================================
    
    std::vector<VkSemaphore> m_imageAvailableSemaphores;   ///< Image available semaphores
    std::vector<VkSemaphore> m_renderFinishedSemaphores;   ///< Render finished semaphores
    std::vector<VkFence> m_inFlightFences;                 ///< In-flight fences
    size_t m_currentFrame;                  ///< Current frame index (for multi-buffering)
    
    // ========================================================================
    // Vulkan Shader Members
    // ========================================================================
    
    VkShaderModule m_vertexShader;          ///< Vertex shader module
    VkShaderModule m_fragmentShader;        ///< Fragment shader module
    
    // ========================================================================
    // Graphics State Members
    // ========================================================================
    
    std::map<unsigned int, VulkanTexturePtr> m_textures;    ///< Active textures by stage
    std::map<unsigned int, VulkanBufferPtr> m_vertexBuffers;    ///< Vertex buffers by stream
    VulkanBufferPtr m_indexBuffer;          ///< Current index buffer
    
    D3DVIEWPORT8 m_viewport;                ///< Current viewport
    float m_worldMatrix[16];                ///< World transformation matrix
    float m_viewMatrix[16];                 ///< View transformation matrix
    float m_projectionMatrix[16];           ///< Projection transformation matrix
    
    std::vector<D3DLIGHT8> m_lights;        ///< Active lights
    D3DMATERIAL8 m_material;                ///< Current material
    D3DCOLOR m_ambientColor;                ///< Ambient light color
    bool m_lightingEnabled;                 ///< Lighting state
    
    std::map<D3DRENDERSTATETYPE, DWORD> m_renderStates;    ///< Render states cache
    
    // ========================================================================
    // State Tracking Members
    // ========================================================================
    
    bool m_initialized;                     ///< Device initialization state
    bool m_inScene;                         ///< True between BeginScene/EndScene
    bool m_debugOutput;                     ///< Debug output enabled
    HRESULT m_lastError;                    ///< Last error code
    
    // ========================================================================
    // Window and Display Members
    // ========================================================================
    
    HWND m_windowHandle;                    ///< Window handle (for surface creation)
    unsigned int m_displayWidth;            ///< Display width in pixels
    unsigned int m_displayHeight;           ///< Display height in pixels
};

#endif // GRAPHICS_BACKEND_DXVK_H
