/*
 * Phase 09.3: VulkanDevice - Vulkan Graphics Backend Implementation
 *
 * Main Vulkan rendering device implementing the GraphicsDevice interface.
 * Manages:
 * - Vulkan instance, device, and queue creation
 * - Swapchain and framebuffer management
 * - Command buffer recording and submission
 * - Synchronization (fences, semaphores)
 * - Resource lifecycle (buffers, textures, pipelines)
 *
 * Architecture follows VULKAN_INTEGRATION_SPEC.md design.
 * VC6-compatible: Avoids STL, uses raw pointers and arrays.
 */

#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H

#include "GraphicsDevice/GraphicsDevice.h"
#include "GraphicsDevice/vulkan_stubs.h"

// Forward declarations
class VulkanBuffer;
class VulkanTexture;
class VulkanPipeline;
class VulkanSwapchain;
class VulkanRenderPass;
class VulkanMemoryAllocator;

namespace GeneralsX {
namespace Graphics {

/**
 * @brief Vulkan implementation of GraphicsDevice
 *
 * Provides hardware-accelerated graphics rendering via Vulkan API.
 * Supports Windows native, Wine, Linux, and macOS.
 *
 * Typical usage:
 *   GraphicsDeviceVulkan* device = new GraphicsDeviceVulkan();
 *   if (device->init(windowHandle, 1024, 768)) {
 *       // Use device...
 *       device->shutdown();
 *   }
 *   delete device;
 */
class GraphicsDeviceVulkan : public GraphicsDevice {
public:
    /**
     * Constructor
     */
    GraphicsDeviceVulkan();

    /**
     * Destructor
     */
    virtual ~GraphicsDeviceVulkan();

    // ============================================================
    // Lifecycle (GraphicsDevice interface)
    // ============================================================

    /**
     * Initialize Vulkan device and create rendering context
     * @param windowHandle SDL2 window pointer (SDL_Window*)
     * @param windowWidth Initial window width
     * @param windowHeight Initial window height
     * @return true if initialization successful
     */
    virtual bool init(void* windowHandle, int windowWidth, int windowHeight);

    /**
     * Shutdown graphics device and release all resources
     */
    virtual void shutdown();

    /**
     * Update graphics device each frame
     * @param deltaTime Frame delta time in seconds
     */
    virtual void update(float deltaTime);

    /**
     * Check if device is initialized
     */
    virtual bool isInitialized() const;

    /**
     * Get last error message
     */
    virtual const char* getLastError() const;

    /**
     * Handle window resize
     */
    virtual void handleWindowResize(int newWidth, int newHeight);

    // ============================================================
    // Frame Management
    // ============================================================

    virtual bool beginFrame();
    virtual bool endFrame();
    virtual void clear(unsigned int color, float depth, int clearMask);
    virtual void flushGPU();

    // ============================================================
    // Viewport & Scissor
    // ============================================================

    virtual void setViewport(const Viewport& viewport);
    virtual void getViewport(Viewport& outViewport) const;
    virtual void setScissor(int x, int y, int width, int height);

    // ============================================================
    // Buffer Management
    // ============================================================

    virtual int createVertexBuffer(const void* data, size_t size, int stride);
    virtual int createIndexBuffer(const void* data, size_t size, int indexSize);
    virtual int createUniformBuffer(const void* data, size_t size);
    virtual void updateBuffer(int bufferId, const void* data, size_t size, size_t offset = 0);
    virtual void destroyBuffer(int bufferId);
    virtual void bindVertexBuffer(int bufferId, int stride, int offset = 0);
    virtual void bindIndexBuffer(int bufferId, int offset = 0);
    virtual void bindUniformBuffer(int bufferId, int shaderSlot);

    // ============================================================
    // Texture Management
    // ============================================================

    virtual int createTexture(const void* data, int width, int height,
                             TextureFormat format, int mipLevels = 1);
    virtual int createRenderTargetTexture(int width, int height, TextureFormat format);
    virtual int createDepthTexture(int width, int height, TextureFormat format);
    virtual void updateTexture(int textureId, const void* data,
                              int x, int y, int width, int height);
    virtual void bindTexture(int textureId, int slot);
    virtual void destroyTexture(int textureId);
    virtual void getTextureSize(int textureId, int& outWidth, int& outHeight) const;

    // ============================================================
    // Shader Management
    // ============================================================

    virtual int createShaderProgram(const char* vertexShaderCode,
                                   const char* fragmentShaderCode,
                                   const VertexAttribute* vertexAttributes,
                                   int attributeCount);
    virtual int createComputeShader(const char* computeShaderCode);
    virtual void bindShaderProgram(int shaderId);
    virtual void bindComputeShader(int shaderId);
    virtual void dispatchCompute(int groupCountX, int groupCountY, int groupCountZ);
    virtual void destroyShaderProgram(int shaderId);
    virtual const char* getShaderErrors() const;

    // ============================================================
    // Render State Management
    // ============================================================

    virtual void setRenderState(const RenderState& state);
    virtual void getRenderState(RenderState& outState) const;
    virtual void setRenderStateFlag(unsigned int flag, bool enabled);
    virtual void setBlendFactor(int srcFactor, int dstFactor);

    // ============================================================
    // Drawing Operations
    // ============================================================

    virtual void drawTriangleList(int vertexCount, int startVertex = 0);
    virtual void drawIndexedTriangleList(int indexCount, int startIndex = 0, int baseVertex = 0);
    virtual void drawLineList(int lineCount, int startVertex = 0);
    virtual void drawPointList(int pointCount, int startVertex = 0);

    // ============================================================
    // Framebuffer / Render Target Management
    // ============================================================

    virtual void setRenderTarget(int colorTextureId, int depthTextureId);
    virtual void setMultipleRenderTargets(const int* colorTextureIds, int colorTextureCount,
                                         int depthTextureId);
    virtual void resetRenderTarget();
    virtual void getRenderTargetSize(int& outWidth, int& outHeight) const;

    // ============================================================
    // Feature Queries & Capabilities
    // ============================================================

    virtual bool supportsFeature(const char* featureName) const;
    virtual int getMaxTextureSize() const;
    virtual int getMaxTextureUnits() const;
    virtual int getMaxRenderTargets() const;
    virtual const char* getDeviceName() const;
    virtual const char* getAPIName() const;
    virtual size_t getTotalGPUMemory() const;
    virtual size_t getAvailableGPUMemory() const;

    // ============================================================
    // Debugging & Profiling
    // ============================================================

    virtual void setDebugMode(bool enabled);
    virtual void pushDebugMarker(const char* markerName);
    virtual void popDebugMarker();
    virtual float getFrameTime() const;
    virtual float getGPUUtilization() const;

    // ============================================================
    // Vulkan-Specific Public Methods
    // ============================================================

    /**
     * Get Vulkan device handle (for advanced usage)
     */
    VkDevice getVkDevice() const { return device; }

    /**
     * Get Vulkan physical device
     */
    VkPhysicalDevice getVkPhysicalDevice() const { return physicalDevice; }

    /**
     * Get Vulkan graphics queue
     */
    VkQueue getVkGraphicsQueue() const { return graphicsQueue; }

    /**
     * Get current Vulkan command buffer
     */
    VkCommandBuffer getVkCommandBuffer() const { return currentCommandBuffer; }

private:
    // ============================================================
    // Vulkan Core Objects
    // ============================================================

    VkInstance instance;                    ///< Vulkan instance
    VkPhysicalDevice physicalDevice;        ///< GPU device
    VkDevice device;                        ///< Logical device
    VkQueue graphicsQueue;                  ///< Graphics command queue
    uint32_t graphicsQueueFamily;           ///< Queue family index
    VkCommandPool commandPool;              ///< Command buffer pool
    VkCommandBuffer currentCommandBuffer;   ///< Current recording command buffer
    VkCommandBuffer* commandBuffers;        ///< Per-frame command buffers
    int commandBufferCount;                 ///< Number of command buffers

    // ============================================================
    // Swapchain & Presentation
    // ============================================================

    VulkanSwapchain* swapchain;             ///< Swapchain manager
    VkSurfaceKHR surface;                   ///< Window surface

    // ============================================================
    // Render Pass & Framebuffers
    // ============================================================

    VulkanRenderPass* renderPass;           ///< Main render pass
    VkFramebuffer* framebuffers;            ///< Framebuffers per swapchain image
    int framebufferCount;                   ///< Number of framebuffers

    // ============================================================
    // Synchronization
    // ============================================================

    VkSemaphore* imageAvailableSemaphores;  ///< Per-frame semaphores
    VkSemaphore* renderFinishedSemaphores;  ///< Per-frame semaphores
    VkFence* inFlightFences;                ///< Per-frame fences
    int synchronizationObjectCount;         ///< Number of sync objects
    int currentFrameIndex;                  ///< Current frame for sync objects

    // ============================================================
    // Resource Management
    // ============================================================

    VulkanMemoryAllocator* memoryAllocator; ///< GPU memory allocator

    // Simple handle-based resource tracking
    // (VC6 compatible - no STL containers)
    // Using enum instead of static const for VC6 compatibility
    enum {
        MAX_BUFFERS = 256,
        MAX_TEXTURES = 512,
        MAX_PIPELINES = 128
    };

    VulkanBuffer* buffers[256];
    VulkanTexture* textures[512];
    VulkanPipeline* pipelines[128];

    int nextBufferHandle;
    int nextTextureHandle;
    int nextPipelineHandle;

    // ============================================================
    // State Management
    // ============================================================

    RenderState currentRenderState;         ///< Current render state
    Viewport currentViewport;               ///< Current viewport
    bool isInitialized_;                    ///< Initialization flag
    bool isFrameRecording;                  ///< Currently recording frame?

    int windowWidth, windowHeight;          ///< Window dimensions
    float deltaTime;                        ///< Last frame delta time

    char lastErrorMessage[256];             ///< Error message buffer

    // ============================================================
    // Private Initialization Methods
    // ============================================================

    /**
     * Create Vulkan instance
     */
    bool createInstance();

    /**
     * Select physical device (GPU)
     */
    bool selectPhysicalDevice();

    /**
     * Create logical device
     */
    bool createLogicalDevice();

    /**
     * Create command pool and buffers
     */
    bool createCommandPool();

    /**
     * Create swapchain from SDL2 window
     */
    bool createSwapchain(void* windowHandle);

    /**
     * Create render pass
     */
    bool createRenderPass();

    /**
     * Create framebuffers
     */
    bool createFramebuffers();

    /**
     * Create synchronization objects
     */
    bool createSynchronizationObjects();

    /**
     * Create memory allocator
     */
    bool createMemoryAllocator();

    // ============================================================
    // Cleanup Methods
    // ============================================================

    void destroySwapchain();
    void destroySynchronizationObjects();
    void destroyFramebuffers();
    void destroyRenderPass();
    void destroyCommandPool();
    void destroyLogicalDevice();
    void destroyInstance();

    // ============================================================
    // Utility Methods
    // ============================================================

    /**
     * Convert DirectX TextureFormat to Vulkan VkFormat
     */
    VkFormat textureFormatToVulkan(TextureFormat format) const;

    /**
     * Find queue family supporting graphics
     */
    bool findQueueFamilies(VkPhysicalDevice device);

    /**
     * Check if physical device is suitable
     */
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    /**
     * Set error message for debugging
     */
    void setError(const char* format, ...);

    /**
     * Handle frame index for synchronization
     */
    void advanceFrame();
};

} // namespace Graphics
} // namespace GeneralsX

#endif // VULKANDEVICE_H
