/*
 * Phase 09: GraphicsDevice - Cross-Platform Graphics Abstraction
 * 
 * This abstraction layer provides a unified interface for all graphics operations,
 * replacing the legacy DirectX 8 rendering with Vulkan (phase 2) and enabling
 * cross-platform support through Wine/Proton.
 * 
 * Supported operations:
 * - 3D model rendering (static meshes, animated models, LOD)
 * - Terrain rendering (heightmap, texture blending, shadows)
 * - Particle effects (dynamic particles, trails, additive blending)
 * - UI rendering (2D text, buttons, HUD elements)
 * - Post-processing effects (fullscreen shaders, color grading)
 * - Texture management (loading, binding, format conversion)
 * - Shader compilation and management
 * - Render state management (blending, depth, culling, etc.)
 * 
 * Thread Safety: Safe for single-threaded access from main game thread
 * Platform Support: Windows (Vulkan), Wine (Vulkan), macOS/Linux (Vulkan via Wine)
 */

#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include <cstddef>

namespace GeneralsX {
namespace Graphics {

/// Invalid handle returned by resource creation operations
const int INVALID_GRAPHICS_HANDLE = -1;

/**
 * @brief Render state flags for graphics pipeline control
 */
enum RenderStateFlags {
    RS_DEPTH_TEST_ENABLED = 0x00000001,
    RS_DEPTH_WRITE_ENABLED = 0x00000002,
    RS_CULLING_ENABLED = 0x00000004,
    RS_CULLING_FRONT_FACE = 0x00000008,
    RS_BLEND_ENABLED = 0x00000010,
    RS_BLEND_ALPHA = 0x00000020,
    RS_BLEND_ADDITIVE = 0x00000040,
    RS_WIREFRAME_MODE = 0x00000080,
    RS_SCISSOR_TEST_ENABLED = 0x00000100,
};

/**
 * @brief Texture format specification
 */
enum TextureFormat {
    TEX_FORMAT_RGBA8,           // 32-bit RGBA
    TEX_FORMAT_BGRA8,           // 32-bit BGRA (DirectX legacy)
    TEX_FORMAT_RGB8,            // 24-bit RGB
    TEX_FORMAT_BC1,             // DXT1 compressed RGB
    TEX_FORMAT_BC2,             // DXT3 compressed RGBA
    TEX_FORMAT_BC3,             // DXT5 compressed RGBA
    TEX_FORMAT_BC4,             // RGTC compressed single channel
    TEX_FORMAT_BC5,             // RGTC compressed dual channel (normals)
    TEX_FORMAT_DEPTH24,         // 24-bit depth buffer
    TEX_FORMAT_DEPTH32F,        // 32-bit float depth buffer
};

/**
 * @brief Vertex attribute layout descriptor
 */
struct VertexAttribute {
    int location;               // Shader input location
    int offset;                 // Byte offset in vertex structure
    int components;             // Number of components (1-4)
    int typeSize;               // Size of single component in bytes
    bool normalized;            // Normalize integer values to [0,1]?
};

/**
 * @brief Render state configuration
 */
struct RenderState {
    unsigned int flags;         // RenderStateFlags bit field
    float lineWidth;            // For line drawing
    float depthBias;            // Polygon offset bias
    float depthSlope;           // Polygon offset slope
    unsigned int clearColor;    // Clear color (RGBA32)
    float clearDepth;           // Clear depth value [0.0, 1.0]
};

/**
 * @brief Viewport and scissor rectangle
 */
struct Viewport {
    int x, y;                   // Top-left corner
    int width, height;          // Dimensions
    float minDepth, maxDepth;   // Depth range [0.0, 1.0]
};

/**
 * @brief Abstract graphics device interface for all game rendering
 * 
 * All graphics subsystems in the game route through this interface.
 * Implementations provide actual backend (Vulkan, DirectX 8, OpenGL, etc.)
 */
class GraphicsDevice {
public:
    virtual ~GraphicsDevice() {}  // Empty destructor for pure virtual base class

    // ============================================================
    // Lifecycle
    // ============================================================

    /**
     * Initialize the graphics device and create rendering context
     * @param windowHandle Native window handle for rendering surface
     * @param windowWidth Initial window width in pixels
     * @param windowHeight Initial window height in pixels
     * @return true if initialization successful
     */
    virtual bool init(void* windowHandle, int windowWidth, int windowHeight) = 0;

    /**
     * Shutdown graphics device, release all resources
     * Safe to call multiple times
     */
    virtual void shutdown() = 0;

    /**
     * Update graphics device (process pending commands, synchronization)
     * Should be called every frame from main game loop
     * @param deltaTime Frame delta time in seconds
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Check if device is currently initialized and ready
     * @return true if device is initialized
     */
    virtual bool isInitialized() const = 0;

    /**
     * Get last error message (for debugging)
     * @return Error string, empty if no error
     */
    virtual const char* getLastError() const = 0;

    /**
     * Handle window resize event
     * Resizes render targets and swapchain as needed
     * @param newWidth New window width in pixels
     * @param newHeight New window height in pixels
     */
    virtual void handleWindowResize(int newWidth, int newHeight) = 0;

    // ============================================================
    // Frame Management
    // ============================================================

    /**
     * Begin rendering frame
     * Prepares device for drawing operations
     * @return true if frame began successfully
     */
    virtual bool beginFrame() = 0;

    /**
     * End rendering frame and present to screen
     * Submits all pending draw calls and displays framebuffer
     * @return true if frame ended successfully
     */
    virtual bool endFrame() = 0;

    /**
     * Clear framebuffer with specified color and depth
     * @param color RGBA32 clear color (0xAABBGGRR format)
     * @param depth Clear depth value [0.0, 1.0]
     * @param clearMask Bitmask: (1 << COLOR_BUFFER) | (1 << DEPTH_BUFFER)
     */
    virtual void clear(unsigned int color, float depth, int clearMask) = 0;

    /**
     * Wait for GPU to complete pending operations
     * Used for synchronization and debugging
     */
    virtual void flushGPU() = 0;

    // ============================================================
    // Viewport and Scissor Control
    // ============================================================

    /**
     * Set rendering viewport (region of framebuffer being rendered to)
     * @param viewport Viewport specification
     */
    virtual void setViewport(const Viewport& viewport) = 0;

    /**
     * Get current viewport
     * @param outViewport [out] Current viewport
     */
    virtual void getViewport(Viewport& outViewport) const = 0;

    /**
     * Set scissor rectangle (region of framebuffer being visible)
     * @param x Top-left X coordinate
     * @param y Top-left Y coordinate
     * @param width Rectangle width
     * @param height Rectangle height
     */
    virtual void setScissor(int x, int y, int width, int height) = 0;

    // ============================================================
    // Buffer Management (Vertex/Index/Uniform)
    // ============================================================

    /**
     * Create a vertex buffer
     * @param data Pointer to vertex data (can be nullptr for GPU-allocated buffer)
     * @param size Size of vertex data in bytes
     * @param stride Bytes per vertex (for vertex format validation)
     * @return Buffer handle for later binding, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createVertexBuffer(const void* data, size_t size, int stride) = 0;

    /**
     * Create an index buffer
     * @param data Pointer to index data
     * @param size Size of index data in bytes (indices * sizeof(index))
     * @param indexSize Bytes per index (2 for uint16, 4 for uint32)
     * @return Buffer handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createIndexBuffer(const void* data, size_t size, int indexSize) = 0;

    /**
     * Create a uniform (constant) buffer
     * @param data Pointer to constant buffer data
     * @param size Size in bytes
     * @return Buffer handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createUniformBuffer(const void* data, size_t size) = 0;

    /**
     * Update buffer contents
     * @param bufferId Handle returned by create*Buffer
     * @param data New buffer data
     * @param size Data size in bytes
     * @param offset Byte offset into buffer (for partial updates)
     */
    virtual void updateBuffer(int bufferId, const void* data, size_t size, size_t offset = 0) = 0;

    /**
     * Destroy buffer and release resources
     * @param bufferId Handle returned by create*Buffer
     */
    virtual void destroyBuffer(int bufferId) = 0;

    /**
     * Bind vertex buffer for rendering
     * @param bufferId Handle from createVertexBuffer
     * @param stride Bytes per vertex
     * @param offset Byte offset into buffer
     */
    virtual void bindVertexBuffer(int bufferId, int stride, int offset = 0) = 0;

    /**
     * Bind index buffer for indexed drawing
     * @param bufferId Handle from createIndexBuffer
     * @param offset Byte offset into buffer
     */
    virtual void bindIndexBuffer(int bufferId, int offset = 0) = 0;

    /**
     * Bind uniform buffer for shader access
     * @param bufferId Handle from createUniformBuffer
     * @param shaderSlot Constant buffer slot in shader (b0, b1, etc.)
     */
    virtual void bindUniformBuffer(int bufferId, int shaderSlot) = 0;

    // ============================================================
    // Texture Management
    // ============================================================

    /**
     * Create a 2D texture from image data
     * @param data Pointer to pixel data
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format Pixel format (see TextureFormat enum)
     * @param mipLevels Number of mipmap levels (1 = no mipmaps)
     * @return Texture handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createTexture(const void* data, int width, int height,
                             TextureFormat format, int mipLevels = 1) = 0;

    /**
     * Create a render target texture (for post-processing, shadows, etc.)
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format Pixel format
     * @return Texture handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createRenderTargetTexture(int width, int height, TextureFormat format) = 0;

    /**
     * Create a depth buffer texture (for rendering)
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format Depth format (TEX_FORMAT_DEPTH24 or TEX_FORMAT_DEPTH32F)
     * @return Texture handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createDepthTexture(int width, int height, TextureFormat format) = 0;

    /**
     * Update texture contents
     * @param textureId Handle from createTexture
     * @param data New pixel data
     * @param x Destination X offset in texture
     * @param y Destination Y offset in texture
     * @param width Region width
     * @param height Region height
     */
    virtual void updateTexture(int textureId, const void* data,
                              int x, int y, int width, int height) = 0;

    /**
     * Bind texture for rendering
     * @param textureId Handle from createTexture
     * @param slot Texture slot in shader (t0, t1, etc.)
     */
    virtual void bindTexture(int textureId, int slot) = 0;

    /**
     * Destroy texture and release resources
     * @param textureId Handle from createTexture
     */
    virtual void destroyTexture(int textureId) = 0;

    /**
     * Get texture dimensions
     * @param textureId Texture handle
     * @param outWidth [out] Texture width
     * @param outHeight [out] Texture height
     */
    virtual void getTextureSize(int textureId, int& outWidth, int& outHeight) const = 0;

    // ============================================================
    // Shader Management
    // ============================================================

    /**
     * Create a graphics pipeline with vertex and fragment shaders
     * @param vertexShaderCode GLSL vertex shader source code
     * @param fragmentShaderCode GLSL fragment shader source code
     * @param vertexAttributes Array of vertex attribute descriptors
     * @param attributeCount Number of vertex attributes
     * @return Shader/pipeline handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createShaderProgram(const char* vertexShaderCode,
                                   const char* fragmentShaderCode,
                                   const VertexAttribute* vertexAttributes,
                                   int attributeCount) = 0;

    /**
     * Create compute shader program
     * @param computeShaderCode GLSL compute shader source code
     * @return Shader handle, INVALID_GRAPHICS_HANDLE on failure
     */
    virtual int createComputeShader(const char* computeShaderCode) = 0;

    /**
     * Bind shader program for rendering
     * @param shaderId Handle from createShaderProgram
     */
    virtual void bindShaderProgram(int shaderId) = 0;

    /**
     * Bind compute shader for execution
     * @param shaderId Handle from createComputeShader
     */
    virtual void bindComputeShader(int shaderId) = 0;

    /**
     * Dispatch compute shader execution
     * @param groupCountX Number of work groups in X dimension
     * @param groupCountY Number of work groups in Y dimension
     * @param groupCountZ Number of work groups in Z dimension
     */
    virtual void dispatchCompute(int groupCountX, int groupCountY, int groupCountZ) = 0;

    /**
     * Destroy shader program and release resources
     * @param shaderId Handle from createShaderProgram
     */
    virtual void destroyShaderProgram(int shaderId) = 0;

    /**
     * Get shader compilation error log (for debugging)
     * @return Error log string, empty if no errors
     */
    virtual const char* getShaderErrors() const = 0;

    // ============================================================
    // Render State Management
    // ============================================================

    /**
     * Set render state configuration
     * @param state RenderState structure with flags and settings
     */
    virtual void setRenderState(const RenderState& state) = 0;

    /**
     * Get current render state
     * @param outState [out] Current render state
     */
    virtual void getRenderState(RenderState& outState) const = 0;

    /**
     * Set specific render state flag
     * @param flag Flag from RenderStateFlags
     * @param enabled true to enable, false to disable
     */
    virtual void setRenderStateFlag(unsigned int flag, bool enabled) = 0;

    /**
     * Set blend factor for blending operations
     * @param srcFactor Source blend factor
     * @param dstFactor Destination blend factor
     */
    virtual void setBlendFactor(int srcFactor, int dstFactor) = 0;

    // ============================================================
    // Drawing Operations
    // ============================================================

    /**
     * Draw non-indexed triangle list
     * @param vertexCount Number of vertices to draw
     * @param startVertex Starting vertex index
     */
    virtual void drawTriangleList(int vertexCount, int startVertex = 0) = 0;

    /**
     * Draw indexed triangle list
     * @param indexCount Number of indices to draw
     * @param startIndex Starting index
     * @param baseVertex Base vertex index
     */
    virtual void drawIndexedTriangleList(int indexCount, int startIndex = 0, int baseVertex = 0) = 0;

    /**
     * Draw non-indexed line list
     * @param lineCount Number of lines
     * @param startVertex Starting vertex index
     */
    virtual void drawLineList(int lineCount, int startVertex = 0) = 0;

    /**
     * Draw point list (for particle effects, etc.)
     * @param pointCount Number of points
     * @param startVertex Starting vertex index
     */
    virtual void drawPointList(int pointCount, int startVertex = 0) = 0;

    // ============================================================
    // Framebuffer/RenderTarget Management
    // ============================================================

    /**
     * Set render target texture for off-screen rendering
     * @param colorTextureId Texture handle from createRenderTargetTexture (or -1 for backbuffer)
     * @param depthTextureId Depth texture handle (or -1 if no depth)
     */
    virtual void setRenderTarget(int colorTextureId, int depthTextureId) = 0;

    /**
     * Set multiple render targets for MRT rendering
     * @param colorTextureIds Array of color texture handles
     * @param colorTextureCount Number of color targets
     * @param depthTextureId Depth texture handle (or -1 if no depth)
     */
    virtual void setMultipleRenderTargets(const int* colorTextureIds, int colorTextureCount,
                                         int depthTextureId) = 0;

    /**
     * Reset render target to backbuffer
     */
    virtual void resetRenderTarget() = 0;

    /**
     * Get current render target dimensions
     * @param outWidth [out] Width in pixels
     * @param outHeight [out] Height in pixels
     */
    virtual void getRenderTargetSize(int& outWidth, int& outHeight) const = 0;

    // ============================================================
    // Feature Queries and Capabilities
    // ============================================================

    /**
     * Check if graphics device supports a feature
     * @param featureName Feature name string (e.g., "geometry_shaders", "compute_shaders")
     * @return true if feature is supported
     */
    virtual bool supportsFeature(const char* featureName) const = 0;

    /**
     * Get maximum texture size supported
     * @return Maximum texture width/height in pixels
     */
    virtual int getMaxTextureSize() const = 0;

    /**
     * Get maximum number of texture units (simultaneous texture bindings)
     * @return Maximum texture units
     */
    virtual int getMaxTextureUnits() const = 0;

    /**
     * Get maximum number of simultaneous render targets
     * @return Maximum render targets
     */
    virtual int getMaxRenderTargets() const = 0;

    /**
     * Get device name string (e.g., "Vulkan 1.2 on NVIDIA RTX 3080")
     * @return Device name
     */
    virtual const char* getDeviceName() const = 0;

    /**
     * Get API name string (e.g., "Vulkan", "DirectX 8", "OpenGL")
     * @return API name
     */
    virtual const char* getAPIName() const = 0;

    /**
     * Get total GPU memory available (in bytes)
     * @return Memory size, 0 if unknown
     */
    virtual size_t getTotalGPUMemory() const = 0;

    /**
     * Get currently available GPU memory (in bytes)
     * @return Available memory size, 0 if unknown
     */
    virtual size_t getAvailableGPUMemory() const = 0;

    // ============================================================
    // Debugging and Profiling
    // ============================================================

    /**
     * Enable/disable graphics debugging
     * When enabled, provides detailed logging of graphics operations
     * @param enabled true to enable debugging
     */
    virtual void setDebugMode(bool enabled) = 0;

    /**
     * Insert a debug marker into GPU command stream
     * Helps correlate GPU profiling data with CPU code
     * @param markerName Name of the marker region
     */
    virtual void pushDebugMarker(const char* markerName) = 0;

    /**
     * Pop debug marker from GPU command stream
     */
    virtual void popDebugMarker() = 0;

    /**
     * Get current frame performance statistics
     * @return Frame time in milliseconds
     */
    virtual float getFrameTime() const = 0;

    /**
     * Get current GPU utilization percentage (0-100)
     * @return GPU utilization, -1 if unknown
     */
    virtual float getGPUUtilization() const = 0;
};

} // namespace Graphics
} // namespace GeneralsX

#endif // GRAPHICSDEVICE_H
