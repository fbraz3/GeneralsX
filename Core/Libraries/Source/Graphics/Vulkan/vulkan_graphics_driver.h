#pragma once

#include "../IGraphicsDriver.h"
#include <memory>
#include <vector>

namespace Graphics {

// Forward declarations for Vulkan internals
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanDevice;
class VulkanSwapchain;
class VulkanMemoryAllocator;
class VulkanRenderPass;
// VulkanCommandPool and VulkanDescriptorPool defined in .cpp only

/**
 * Vulkan implementation of IGraphicsDriver.
 * 
 * This class provides the Vulkan backend for the GeneralsX graphics driver system.
 * It translates abstract graphics operations into Vulkan API calls, managing
 * GPU resources, memory allocation, and rendering pipelines.
 * 
 * All Vulkan types are kept private - the public interface exposes only abstract
 * types and opaque handles defined in IGraphicsDriver.
 */
class VulkanGraphicsDriver : public IGraphicsDriver {
public:
    /**
     * Constructor: Creates a new Vulkan graphics driver instance.
     */
    VulkanGraphicsDriver();
    
    /**
     * Destructor: Cleans up Vulkan resources.
     */
    virtual ~VulkanGraphicsDriver() override;
    
    // ===== INITIALIZATION & CLEANUP =====
    virtual bool Initialize(void* windowHandle, uint32_t width, uint32_t height, bool fullscreen) override;
    virtual void Shutdown() override;
    virtual bool IsInitialized() const override;
    virtual BackendType GetBackendType() const override;
    virtual const char* GetBackendName() const override;
    virtual const char* GetVersionString() const override;

    // ===== FRAME MANAGEMENT =====
    virtual bool BeginFrame() override;
    virtual void EndFrame() override;
    virtual bool Present() override;
    virtual void Clear(float r, float g, float b, float a, bool clearDepth) override;
    virtual void SetClearColor(float r, float g, float b, float a) override;

    // ===== DRAWING OPERATIONS =====
    virtual void DrawPrimitive(PrimitiveType primType, uint32_t vertexCount) override;
    virtual void DrawIndexedPrimitive(PrimitiveType primType, uint32_t indexCount,
                                     IndexBufferHandle ibHandle, uint32_t startIndex) override;
    virtual void DrawPrimitiveUP(PrimitiveType primType, uint32_t primCount,
                                const void* vertexData, uint32_t vertexStride) override;
    virtual void DrawIndexedPrimitiveUP(PrimitiveType primType, uint32_t minVertexIndex,
                                       uint32_t vertexCount, uint32_t primCount,
                                       const void* indexData, const void* vertexData,
                                       uint32_t vertexStride) override;

    // ===== RENDER STATE MANAGEMENT =====
    virtual bool SetRenderState(RenderState state, uint64_t value) override;
    virtual uint64_t GetRenderState(RenderState state) const override;
    virtual bool SetBlendState(const BlendStateDescriptor& desc) override;
    virtual bool SetDepthStencilState(const DepthStencilStateDescriptor& desc) override;
    virtual bool SetRasterizerState(const RasterizerStateDescriptor& desc) override;
    virtual bool SetScissorRect(const Rect& rect) override;
    virtual void SetViewport(const Viewport& vp) override;
    virtual Viewport GetViewport() const override;

    // ===== VERTEX & INDEX BUFFERS =====
    virtual VertexBufferHandle CreateVertexBuffer(uint32_t sizeInBytes, bool dynamic,
                                                 const void* initialData) override;
    virtual void DestroyVertexBuffer(VertexBufferHandle handle) override;
    virtual bool LockVertexBuffer(VertexBufferHandle handle, uint32_t offset,
                                 uint32_t size, void** lockedData, bool readOnly) override;
    virtual bool UnlockVertexBuffer(VertexBufferHandle handle) override;
    virtual uint32_t GetVertexBufferSize(VertexBufferHandle handle) const override;

    virtual IndexBufferHandle CreateIndexBuffer(uint32_t sizeInBytes, bool is32Bit,
                                               bool dynamic, const void* initialData) override;
    virtual void DestroyIndexBuffer(IndexBufferHandle handle) override;
    virtual bool LockIndexBuffer(IndexBufferHandle handle, uint32_t offset,
                                uint32_t size, void** lockedData, bool readOnly) override;
    virtual bool UnlockIndexBuffer(IndexBufferHandle handle) override;
    virtual uint32_t GetIndexBufferSize(IndexBufferHandle handle) const override;

    // ===== VERTEX FORMAT / DECLARATION =====
    virtual VertexFormatHandle CreateVertexFormat(const VertexElement* elements,
                                                 uint32_t elementCount) override;
    virtual void DestroyVertexFormat(VertexFormatHandle handle) override;
    virtual bool SetVertexFormat(VertexFormatHandle handle) override;
    virtual bool SetVertexStreamSource(uint32_t streamIndex, VertexBufferHandle vbHandle,
                                      uint32_t offset, uint32_t stride) override;
    virtual bool SetIndexBuffer(IndexBufferHandle ibHandle, uint32_t startIndex) override;

    // ===== TEXTURE MANAGEMENT =====
    virtual TextureHandle CreateTexture(const TextureDescriptor& desc, const void* initialData) override;
    virtual void DestroyTexture(TextureHandle handle) override;
    virtual bool SetTexture(uint32_t samplerIndex, TextureHandle handle) override;
    virtual TextureHandle GetTexture(uint32_t samplerIndex) const override;
    virtual bool LockTexture(TextureHandle handle, uint32_t level, void** lockedData,
                            uint32_t* pitch) override;
    virtual bool UnlockTexture(TextureHandle handle, uint32_t level) override;
    virtual TextureDescriptor GetTextureDescriptor(TextureHandle handle) const override;
    virtual bool UpdateTextureSubregion(TextureHandle handle, const void* data,
                                       uint32_t left, uint32_t top, uint32_t right,
                                       uint32_t bottom) override;

    // ===== RENDER TARGET & DEPTH STENCIL =====
    virtual RenderTargetHandle CreateRenderTarget(uint32_t width, uint32_t height,
                                                 TextureFormat format) override;
    virtual void DestroyRenderTarget(RenderTargetHandle handle) override;
    virtual bool SetRenderTarget(uint32_t targetIndex, RenderTargetHandle handle) override;
    virtual RenderTargetHandle GetRenderTarget(uint32_t targetIndex) const override;
    virtual bool SetDefaultRenderTarget() override;

    virtual DepthStencilHandle CreateDepthStencil(uint32_t width, uint32_t height,
                                                 TextureFormat format) override;
    virtual void DestroyDepthStencil(DepthStencilHandle handle) override;
    virtual bool SetDepthStencil(DepthStencilHandle handle) override;
    virtual DepthStencilHandle GetDepthStencil() const override;

    // ===== TRANSFORM MATRICES =====
    virtual void SetWorldMatrix(const Matrix4x4& matrix) override;
    virtual void SetViewMatrix(const Matrix4x4& matrix) override;
    virtual void SetProjectionMatrix(const Matrix4x4& matrix) override;
    virtual Matrix4x4 GetWorldMatrix() const override;
    virtual Matrix4x4 GetViewMatrix() const override;
    virtual Matrix4x4 GetProjectionMatrix() const override;

    // ===== LIGHTING =====
    virtual void SetAmbientLight(float r, float g, float b) override;
    virtual void SetLight(uint32_t lightIndex, const Light& light) override;
    virtual void DisableLight(uint32_t lightIndex) override;
    virtual bool IsLightEnabled(uint32_t lightIndex) const override;
    virtual uint32_t GetMaxLights() const override;
    virtual void SetMaterial(const Material& material) override;

    // ===== SAMPLER & TEXTURE STATES =====
    virtual bool SetSamplerState(uint32_t samplerIndex, uint32_t state, uint32_t value) override;
    virtual uint32_t GetSamplerState(uint32_t samplerIndex, uint32_t state) const override;
    virtual uint32_t GetMaxTextureSamplers() const override;

    // ===== CAPABILITIES & QUERIES =====
    virtual bool SupportsTextureFormat(TextureFormat format) const override;
    virtual uint32_t GetMaxTextureWidth() const override;
    virtual uint32_t GetMaxTextureHeight() const override;
    virtual uint32_t GetMaxVertexBlendMatrices() const override;
    virtual uint32_t GetMaxClipPlanes() const override;
    virtual bool SupportsHardwareTransformAndLight() const override;
    virtual bool SupportsPixelShaders() const override;
    virtual bool SupportsVertexShaders() const override;
    virtual bool SupportsCompressedTextures() const override;
    virtual uint32_t GetMaxPrimitiveCount() const override;
    virtual uint32_t GetMaxVertexIndex() const override;
    virtual uint32_t GetMaxStreamStride() const override;

    // ===== UTILITY =====
    virtual bool ResizeSwapChain(uint32_t width, uint32_t height) override;
    virtual void GetDisplaySize(uint32_t& width, uint32_t& height) const override;
    virtual bool SetFullscreen(bool fullscreen) override;
    virtual bool IsFullscreen() const override;
    virtual void Flush() override;
    virtual void WaitForGPU() override;
    virtual const char* GetLastError() const override;
    
    // ===== INTERNAL ACCESSORS (for d3d8_vulkan_*.cpp files) =====
    /**
     * Get the Vulkan instance (for internal use by Vulkan implementation files).
     * @return Pointer to internal VulkanInstance, or nullptr if not initialized.
     */
    VulkanInstance* GetVulkanInstance();
    const VulkanInstance* GetVulkanInstance() const;
    
    /**
     * Get the Vulkan device (for internal use by Vulkan implementation files).
     * @return Pointer to internal VulkanDevice, or nullptr if not initialized.
     */
    VulkanDevice* GetVulkanDevice();
    const VulkanDevice* GetVulkanDevice() const;
    
    /**
     * Get the graphics queue (for internal use by Vulkan implementation files).
     * @return Vulkan queue handle, or VK_NULL_HANDLE if not initialized.
     */
    void* GetGraphicsQueue() const;  // Returns VkQueue

private:
    // Initialization state
    bool m_initialized;
    bool m_in_frame;
    uint32_t m_display_width;
    uint32_t m_display_height;
    bool m_fullscreen;
    
    // Vulkan components (opaque - implementation details hidden)
    std::unique_ptr<VulkanInstance> m_instance;
    std::unique_ptr<VulkanPhysicalDevice> m_physical_device;
    std::unique_ptr<VulkanDevice> m_device;
    std::unique_ptr<VulkanSwapchain> m_swapchain;
    std::unique_ptr<VulkanMemoryAllocator> m_memory_allocator;
    std::unique_ptr<VulkanRenderPass> m_render_pass;
    // Command pool and descriptor pool defined in .cpp - not exposed in header
    
    // Phase 54: Frame rendering infrastructure
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_current_frame;
    uint32_t m_current_image_index;
    bool m_frame_started;
    
    // State caching
    Color m_clear_color;  // Moved before frame vars to match initializer order
    std::vector<uint64_t> m_render_state_cache;
    std::vector<TextureHandle> m_bound_textures;
    Viewport m_viewport;
    
    // Phase 54: Pipeline and rendering helpers (implemented in .cpp)
    bool CreateSyncObjects();
    void DestroySyncObjects();
    bool CreateCommandBuffers();
    void DestroyCommandBuffers();
    bool CreateGraphicsPipeline();
    void DestroyGraphicsPipeline();
    bool RecreatePipelineIfNeeded();  // Phase 59: Recreate pipeline when render state changes
    bool RecordClearCommand();
};

/**
 * Factory function to create a Vulkan graphics driver instance.
 * @return Newly created IGraphicsDriver* instance (Vulkan implementation).
 */
extern IGraphicsDriver* CreateVulkanGraphicsDriver();

}  // namespace Graphics
