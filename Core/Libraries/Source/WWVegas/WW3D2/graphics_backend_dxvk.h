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
#include <array>
#include <chrono>

// Vulkan constants and extensions
#define DEVICE_EXTENSION_COUNT 1
const char* const DEVICE_EXTENSIONS[DEVICE_EXTENSION_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Forward declarations
struct DXVKTextureHandle;
struct DXVKBufferHandle;

// ============================================================================
// Vertex Format Enumeration (Phase 44.2)
// ============================================================================

enum VertexFormat {
    VERTEX_FORMAT_POSITION_ONLY = 0,    ///< 3x float32 (12 bytes) - position only
    VERTEX_FORMAT_POSITION_COLOR = 1,   ///< 3x float32 + uint32 (16 bytes) - position + RGBA color
    VERTEX_FORMAT_POSITION_UV = 2,      ///< 3x float32 + 2x float32 (20 bytes) - position + texture coordinates
    VERTEX_FORMAT_FULL = 3              ///< 3x float32 + 3x float32 + 2x float32 + uint32 (36 bytes) - full vertex with all attributes
};

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
    VkDeviceMemory imageMemory;             ///< GPU memory allocation for the image
    VkSampler sampler;                      ///< Texture sampler
    unsigned int width;                     ///< Texture width in pixels
    unsigned int height;                    ///< Texture height in pixels
    unsigned int mipLevels;                 ///< Number of mipmap levels
    VkFormat format;                        ///< Vulkan pixel format (e.g., VK_FORMAT_R8G8B8A8_UNORM)
    D3DFORMAT originalFormat;               ///< Original DirectX format for reference
    bool isRenderTarget;                    ///< True if texture is used as render target
    bool isDynamic;                         ///< True if texture is dynamic (CPU-updatable)
    
    // Lock-related fields
    void* lockedData;                       ///< Pointer to locked texture data (CPU-readable)
    VkBuffer lockedStagingBuffer;           ///< Staging buffer for texture data during lock
    VkDeviceMemory lockedStagingMemory;     ///< Memory for staging buffer
    unsigned int lockedOffset;              ///< Offset into texture where lock began
    size_t lockedSize;                      ///< Size of locked data in bytes
    
    DXVKTextureHandle()
        : image(VK_NULL_HANDLE), imageView(VK_NULL_HANDLE), memory(VK_NULL_HANDLE),
          imageMemory(VK_NULL_HANDLE), sampler(VK_NULL_HANDLE), width(0), height(0),
          mipLevels(1), format(VK_FORMAT_UNDEFINED), originalFormat(D3DFMT_UNKNOWN),
          isRenderTarget(false), isDynamic(false), lockedData(nullptr),
          lockedStagingBuffer(VK_NULL_HANDLE), lockedStagingMemory(VK_NULL_HANDLE),
          lockedOffset(0), lockedSize(0) {}
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
    VkDeviceMemory bufferMemory;            ///< GPU memory allocation for the buffer
    unsigned int size;                      ///< Buffer size in bytes
    VkBufferUsageFlags usage;               ///< Buffer usage flags (VERTEX, INDEX, etc)
    bool isDynamic;                         ///< True if buffer is dynamic (CPU-updatable)
    void* stagingData;                      ///< Staging buffer for CPU access during lock
    
    // Lock-related fields
    void* lockedData;                       ///< Pointer to locked buffer data (CPU-readable)
    VkBuffer lockedStagingBuffer;           ///< Staging buffer for buffer data during lock
    VkDeviceMemory lockedStagingMemory;     ///< Memory for staging buffer
    unsigned int lockedOffset;              ///< Offset into buffer where lock began
    size_t lockedSize;                      ///< Size of locked data in bytes
    
    DXVKBufferHandle()
        : buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), bufferMemory(VK_NULL_HANDLE),
          size(0), usage(0), isDynamic(false), stagingData(nullptr),
          lockedData(nullptr), lockedStagingBuffer(VK_NULL_HANDLE),
          lockedStagingMemory(VK_NULL_HANDLE), lockedOffset(0), lockedSize(0) {}
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
    
    // ========================================================================
    // Phase 43.2: Frame Synchronization & Timing
    // ========================================================================
    
    /**
     * Initialize frame synchronization structures.
     */
    HRESULT InitializeFrameSynchronization();
    
    /**
     * Shutdown frame synchronization.
     */
    HRESULT ShutdownFrameSynchronization();
    
    /**
     * Wait for a frame to complete GPU work (wait-before-overwrite pattern).
     */
    HRESULT WaitForFrame(uint32_t frameIndex, uint64_t timeoutNs);
    
    /**
     * Check if frame GPU work is complete (non-blocking).
     */
    bool IsFrameComplete(uint32_t frameIndex) const;
    
    /**
     * Pace rendering to target FPS (60).
     */
    uint64_t PaceFrameToTargetFPS();
    
    /**
     * Get time budget remaining in current frame.
     */
    int64_t GetFrameTimeBudgetRemaining() const;
    
    /**
     * Measure frame time from start.
     */
    float MeasureFrameTime() const;
    
    /**
     * Get current frame index for multi-frame management.
     */
    uint32_t GetCurrentFrameIndex() const;
    
    /**
     * Advance to next frame after Present().
     */
    uint32_t AdvanceToNextFrame();
    
    /**
     * Get total frame count since initialization.
     */
    uint32_t GetFrameCount() const;
    
    /**
     * Get max frames in flight.
     */
    uint32_t GetMaxFramesInFlight() const;
    
    /**
     * Report synchronization state for debugging.
     */
    void ReportSynchronizationState() const;
    
    /**
     * Set viewport for rendering.
     */
    void SetViewport(float x, float y, float width, float height,
                    float minDepth, float maxDepth);
    
    /**
     * Set scissor rectangle for rendering.
     */
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
    
    /**
     * Record clear operation in render pass.
     */
    HRESULT RecordClearOperation(bool clearColor, bool clearDepth,
                                const float* colorRGBA, float depthValue);
    
    /**
     * Enable or disable blending.
     */
    void SetBlendingEnabled(bool enabled);
    
    /**
     * Set blend mode.
     */
    void SetBlendMode(D3DBLEND srcFactor, D3DBLEND dstFactor);
    
    /**
     * Report render pass state for debugging.
     */
    void ReportRenderPassState() const;
    
    /**
     * Recover from device lost.
     */
    HRESULT RecoverFromDeviceLost();
    
    /**
     * Log comprehensive error information.
     */
    void LogError(HRESULT errorCode, const char* context);
    
    /**
     * Handle presentation errors.
     */
    HRESULT HandlePresentError(VkResult presentResult);
    
    /**
     * Handle acquisition errors.
     */
    HRESULT HandleAcquisitionError(VkResult acquireResult);
    
    /**
     * Validate render pipeline state.
     */
    HRESULT ValidateRenderState() const;
    
    /**
     * Perform comprehensive device diagnostics.
     */
    void PerformDiagnostics() const;
    
    // ========================================================================
    // Phase 43.5: Performance Monitoring & Profiling
    // ========================================================================
    
    /**
     * Get current FPS (frames per second).
     */
    float GetCurrentFPS() const;
    
    /**
     * Get rolling average FPS over recent frames.
     */
    float GetAverageFPS() const;
    
    /**
     * Get minimum frame time from history.
     */
    float GetMinFrameTime() const;
    
    /**
     * Get maximum frame time from history.
     */
    float GetMaxFrameTime() const;
    
    /**
     * Get average frame time over history.
     */
    float GetAverageFrameTime() const;
    
    /**
     * Record current frame time in history.
     */
    void RecordFrameTime(float frameTimeMs);
    
    /**
     * Record stage timing (acquire time).
     */
    void RecordAcquireTime(float timeMs);
    
    /**
     * Record stage timing (submit time).
     */
    void RecordSubmitTime(float timeMs);
    
    /**
     * Record stage timing (present time).
     */
    void RecordPresentTime(float timeMs);
    
    /**
     * Get frame timing breakdown for last frame.
     */
    struct FrameTimingBreakdown {
        float acquireTime;
        float submitTime;
        float presentTime;
        float totalTime;
    };
    
    FrameTimingBreakdown GetLastFrameBreakdown() const;
    
    /**
     * Get percentile frame time (e.g., 99th percentile).
     */
    float GetPercentileFrameTime(float percentile) const;
    
    /**
     * Get GPU frame time (Phase 45+).
     */
    float GetGPUFrameTime() const;
    
    /**
     * Get GPU utilization percentage (Phase 45+).
     */
    float GetGPUUtilization() const;
    
    /**
     * Generate performance report string.
     */
    const char* GeneratePerformanceReport(char* buffer, size_t bufferSize) const;
    
    /**
     * Print performance statistics to console.
     */
    void PrintPerformanceStats() const;
    
    /**
     * Count frames over budget (16.67ms for 60 FPS).
     */
    uint32_t CountFramesOverBudget() const;
    
    /**
     * Identify performance bottleneck (Acquire/Submit/Present).
     */
    const char* IdentifyBottleneck() const;
    
    /**
     * Get performance warning message.
     */
    const char* GetPerformanceWarning() const;
    
    /**
     * Reset performance statistics.
     */
    void ResetPerformanceStats();
    
    /**
     * Get performance history size.
     */
    uint32_t GetPerformanceHistorySize() const;
    
    // ========================================================================
    // Phase 44.1: Graphics Pipeline
    // ========================================================================
    
    /**
     * Create graphics pipeline layout.
     */
    HRESULT CreatePipelineLayout();
    
    /**
     * Destroy pipeline layout.
     */
    HRESULT DestroyPipelineLayout();
    
    /**
     * Create graphics pipeline (vertex + fragment shaders).
     */
    HRESULT CreateGraphicsPipeline();
    
    /**
     * Destroy graphics pipeline.
     */
    HRESULT DestroyGraphicsPipeline();
    
    /**
     * Bind graphics pipeline to command buffer.
     */
    HRESULT BindGraphicsPipeline();
    
    /**
     * Get graphics pipeline.
     */
    VkPipeline GetGraphicsPipeline() const;
    
    /**
     * Get pipeline layout.
     */
    VkPipelineLayout GetPipelineLayout() const;
    
    /**
     * Check if graphics pipeline is ready.
     */
    bool IsGraphicsPipelineReady() const;
    
    /**
     * Report graphics pipeline state.
     */
    void ReportPipelineState() const;

    // ========== Phase 44.2: Vertex Buffer Management ==========

    /**
     * Create vertex buffer and allocate device memory.
     * Allocates 16MB vertex buffer for geometry data.
     */
    HRESULT CreateVertexBuffer();
    
    /**
     * Destroy vertex buffer and free device memory.
     */
    void DestroyVertexBuffer();
    
    /**
     * Set vertex format (position-only, position+color, position+UV, full).
     */
    HRESULT SetVertexFormat(VertexFormat format);
    
    /**
     * Upload vertex data to GPU buffer.
     */
    HRESULT UpdateVertexBuffer(const void* vertexData, uint32_t vertexCount);
    
    /**
     * Bind vertex buffer to command buffer for rendering.
     */
    HRESULT BindVertexBuffer(VkCommandBuffer commandBuffer);
    
    /**
     * Query vertex buffer handle.
     */
    VkBuffer GetVertexBuffer() const;
    
    /**
     * Query vertex buffer memory handle.
     */
    VkDeviceMemory GetVertexBufferMemory() const;
    
    /**
     * Query current vertex count in buffer.
     */
    uint32_t GetVertexCount() const;
    
    /**
     * Query vertex stride (bytes per vertex).
     */
    uint32_t GetVertexStride() const;
    
    /**
     * Check if vertex buffer is ready for rendering.
     */
    bool IsVertexBufferReady() const;
    
    /**
     * Get vertex attribute descriptions for pipeline.
     */
    const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributeDescriptions() const;
    
    /**
     * Get vertex binding description for pipeline.
     */
    const VkVertexInputBindingDescription& GetVertexBindingDescription() const;
    
    /**
     * Report vertex buffer state and diagnostics.
     */
    void ReportVertexBufferState();
    
    // ========================================================================
    // Phase 44.3: Index Buffer Management
    // ========================================================================
    
    /**
     * Create index buffer (16MB GPU memory allocation).
     * Supports VK_INDEX_TYPE_UINT16 and VK_INDEX_TYPE_UINT32.
     */
    HRESULT CreateIndexBuffer();
    
    /**
     * Destroy index buffer and release GPU memory.
     */
    HRESULT DestroyIndexBuffer();
    
    /**
     * Set index format (UINT16 or UINT32).
     * Affects stride calculation for data upload.
     */
    HRESULT SetIndexFormat(VkIndexType format);
    
    /**
     * Upload index data to GPU buffer.
     * Parameters: data pointer, index count
     */
    HRESULT UpdateIndexBuffer(const void* data, uint32_t count);
    
    /**
     * Bind index buffer to current command buffer.
     * Called before DrawIndexed().
     */
    HRESULT BindIndexBuffer(VkCommandBuffer commandBuffer);
    
    /**
     * Get VkBuffer handle for index buffer.
     */
    VkBuffer GetIndexBuffer() const;
    
    /**
     * Get VkDeviceMemory handle for index buffer.
     */
    VkDeviceMemory GetIndexBufferMemory() const;
    
    /**
     * Get current index count.
     */
    uint32_t GetIndexCount() const;
    
    /**
     * Get current index format (UINT16 or UINT32).
     */
    VkIndexType GetIndexFormat() const;
    
    /**
     * Check if index buffer is ready for rendering.
     */
    bool IsIndexBufferReady() const;
    
    /**
     * Report index buffer state and diagnostics.
     */
    void ReportIndexBufferState() const;
    
    /**
     * Clear render target and depth buffer.
     */
    virtual HRESULT Clear(
        bool clear_color,
        bool clear_z_stencil,
        const void* color_vec3,
        float z = 1.0f,
        DWORD stencil = 0
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
    
    /**
     * Phase 42.2: Load DDS compressed texture from memory.
     * Supports DXT1/DXT3/DXT5 (BC1/2/3) compression.
     */
    HRESULT LoadDDSTexture(const uint8_t* data, uint32_t size, void** texture);
    
    /**
     * Phase 42.3: Load TGA texture from memory.
     * Supports 24-bit RGB and 32-bit RGBA TGA files.
     */
    HRESULT LoadTGATexture(const uint8_t* data, uint32_t size, void** texture);
    
    /**
     * Phase 42.4: Generate mipmap chain for texture.
     * Creates progressively smaller levels with box filter downsampling.
     */
    HRESULT GenerateMipmaps(void* texture, uint32_t levels = 0);
    
    // ========================================================================
    // Phase 42.5: Texture Cache Management
    // ========================================================================
    
    /**
     * Initialize texture cache system.
     * Called during backend initialization.
     */
    void InitializeTextureCache();
    
    /**
     * Shutdown texture cache system.
     * Releases all cached textures.
     */
    void ShutdownTextureCache();
    
    /**
     * Cache a loaded texture by filename.
     * Reduces redundant load operations.
     */
    void CacheTexture(const char* filename, void* textureHandle, 
                      uint32_t width, uint32_t height, D3DFORMAT format);
    
    /**
     * Retrieve texture from cache by filename.
     */
    bool GetCachedTexture(const char* filename, void** outTextureHandle);
    
    /**
     * Clear all cached textures.
     * Used for map transitions or explicit cache flush.
     */
    void ClearTextureCache();
    
    /**
     * Notify cache of frame completion for LRU tracking.
     */
    void NotifyFrameComplete();
    
    /**
     * Print cache statistics for profiling.
     */
    void PrintCacheStatistics();
    
    /**
     * Set maximum cache memory limit.
     */
    void SetMaxCacheMemory(uint32_t maxMemory);
    
    /**
     * Get current cache memory usage.
     */
    uint32_t GetCacheMemoryUsage() const;
    
    /**
     * Get cache hit rate for profiling.
     */
    float GetCacheHitRate() const;
    
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
    
    /**
     * Get Vulkan image usage flags for texture.
     */
    VkImageUsageFlags GetTextureImageUsageFlags(bool isRenderTarget);
    
    /**
     * Get texture memory properties.
     */
    VkMemoryPropertyFlags GetTextureMemoryProperties();
    
    // ========================================================================
    // Phase 42.5: Private Texture Cache Methods
    // ========================================================================
    
    /**
     * Evict least-recently-used textures when cache is full.
     */
    void EvictLRUTextures(uint32_t requiredMemory);
    
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
    std::vector<VkFramebuffer> m_swapchainFramebuffers;  ///< Framebuffers for each image
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

    // ========== Phase 44.2: Vertex Buffer Members ==========
    VkBuffer m_vertexBuffer;                ///< Vertex buffer handle
    VkDeviceMemory m_vertexBufferMemory;    ///< Vertex buffer device memory
    VkDeviceSize m_vertexBufferSize;        ///< Total vertex buffer size
    VkDeviceSize m_vertexBufferOffset;      ///< Current offset in vertex buffer
    uint32_t m_vertexCount;                 ///< Number of vertices in buffer
    uint32_t m_vertexStride;                ///< Bytes per vertex
    VertexFormat m_currentVertexFormat;     ///< Current vertex format
    std::vector<VkVertexInputAttributeDescription> m_vertexAttributeDescriptions;
    VkVertexInputBindingDescription m_vertexBindingDescription;

    // ========== Phase 44.3: Index Buffer Members ==========
    VkBuffer m_indexBuffer;                 ///< Index buffer handle
    VkDeviceMemory m_indexBufferMemory;     ///< Index buffer device memory
    VkDeviceSize m_indexBufferSize;         ///< Total index buffer size (16MB)
    VkDeviceSize m_indexBufferOffset;       ///< Current offset in index buffer
    uint32_t m_indexCount;                  ///< Number of indices in buffer
    uint32_t m_indexStride;                 ///< Bytes per index (2 or 4)
    VkIndexType m_currentIndexFormat;       ///< Current index format (UINT16 or UINT32)

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
    std::map<void*, VulkanTexturePtr> m_textureCache;       ///< Texture cache by handle pointer
    
    // ========================================================================
    // Phase 42.5: Texture Cache Members
    // ========================================================================
    
    bool m_textureCacheEnabled;             ///< Texture caching enabled
    std::map<uint32_t, void*> m_textureCacheMap;  ///< Cached textures by filename CRC
    uint32_t m_totalCacheMemory;            ///< Total memory used by cache
    uint32_t m_maxCacheMemory;              ///< Maximum cache memory limit (256MB default)
    uint32_t m_cacheHits;                   ///< Cache hit count for profiling
    uint32_t m_cacheMisses;                 ///< Cache miss count for profiling
    uint32_t m_currentFrameNumber;          ///< Current frame for LRU tracking
    
    // ========================================================================
    // Phase 43.2: Frame Synchronization & Timing Members
    // ========================================================================
    
    std::chrono::high_resolution_clock::time_point m_frameStartTime;  ///< Frame start timestamp
    uint32_t m_frameCount;                  ///< Total frames rendered
    
    std::map<unsigned int, VulkanBufferPtr> m_vertexBuffers;    ///< Vertex buffers by stream
    
    D3DVIEWPORT8 m_viewport;                ///< Current viewport
    float m_worldMatrix[16];                ///< World transformation matrix
    float m_viewMatrix[16];                 ///< View transformation matrix
    float m_projectionMatrix[16];           ///< Projection transformation matrix
    
    std::vector<D3DLIGHT8> m_lights;        ///< Active lights
    D3DMATERIAL8 m_material;                ///< Current material
    D3DCOLOR m_ambientColor;                ///< Ambient light color
    bool m_lightingEnabled;                 ///< Lighting state
    
    // Phase 41: Render State Tracking
    std::map<D3DRENDERSTATETYPE, DWORD> m_renderStates;    ///< Render states cache
    bool m_depthTestEnabled;                ///< Depth testing enabled state
    bool m_depthWriteEnabled;               ///< Depth write enabled state
    DWORD m_depthFunc;                      ///< Depth comparison function
    bool m_blendEnabled;                    ///< Blending enabled state
    DWORD m_srcBlend;                       ///< Source blend factor
    DWORD m_dstBlend;                       ///< Destination blend factor
    DWORD m_cullMode;                       ///< Face culling mode
    DWORD m_fillMode;                       ///< Fill mode (solid, wireframe, points)
    bool m_fogEnabled;                      ///< Fog enabled state
    DWORD m_fogMode;                        ///< Fog mode
    
    // Phase 41: Buffer Tracking
    DXVKBufferHandle* m_currentVertexBuffer;    ///< Currently bound vertex buffer
    DXVKBufferHandle* m_currentIndexBuffer;     ///< Currently bound index buffer
    
    // Phase 41: Light Management
    static constexpr uint32_t MAX_LIGHTS = 8;
    std::array<bool, MAX_LIGHTS> m_activeLights;   ///< Light enable/disable state

    
    // ========================================================================
    // State Tracking Members
    // ========================================================================
    
    bool m_initialized;                     ///< Device initialization state
    bool m_inScene;                         ///< True between BeginScene/EndScene
    bool m_debugOutput;                     ///< Debug output enabled
    HRESULT m_lastError;                    ///< Last error code
    
    // ========================================================================
    // Phase 43: Render Loop Members
    // ========================================================================
    
    uint32_t m_frameNumber;                 ///< Frame counter (incremented on BeginScene)
    uint32_t m_currentImageIndex;           ///< Current swapchain image index
    float m_clearColor[4];                  ///< Clear color (RGBA)
    
    // ========================================================================
    // Window and Display Members
    // ========================================================================
    
    HWND m_windowHandle;                    ///< Window handle (for surface creation)
    unsigned int m_displayWidth;            ///< Display width in pixels
    unsigned int m_displayHeight;           ///< Display height in pixels
};

#endif // GRAPHICS_BACKEND_DXVK_H
