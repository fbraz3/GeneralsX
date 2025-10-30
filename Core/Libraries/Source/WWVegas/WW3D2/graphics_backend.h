/**
 * GeneralsX Graphics Backend Interface
 * 
 * Abstract interface for graphics rendering operations.
 * Allows switching between different graphics implementations:
 * - Legacy: Metal/OpenGL (Phase 27-37)
 * - DXVK: Vulkan via DXVK (Phase 39+)
 * 
 * This layer provides complete abstraction of DirectX 8 operations
 * into a backend-agnostic interface. All graphics calls in game code
 * should go through this interface, not directly to D3D wrappers.
 */

#pragma once

// ============================================================================
// Forward Declarations - DirectX Types
// ============================================================================

// These are defined elsewhere in the codebase
struct D3DRECT;
struct D3DLIGHT8;
struct D3DMATERIAL8;
struct D3DVIEWPORT8;

// Enum types - forward declared
enum D3DRENDERSTATETYPE;
enum D3DTEXTUREOP;
enum D3DFORMAT;
enum D3DPRIMITIVETYPE;
enum D3DTRANSFORMSTATETYPE;

// DirectX types
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef int HRESULT;
typedef unsigned long D3DCOLOR;

// ============================================================================
// Graphics Backend Interface
// ============================================================================

/**
 * Abstract graphics backend interface.
 * 
 * Implement this interface to support a new graphics backend.
 * All methods should return HRESULT (DirectX convention).
 * 
 * S_OK (0) = Success
 * Any other value = Failure
 */
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() {}
    
    // ========================================================================
    // Device Management
    // ========================================================================
    
    /**
     * Initialize the graphics device.
     * Called once at startup.
     */
    virtual HRESULT Initialize() = 0;
    
    /**
     * Shutdown the graphics device.
     * Called at application exit.
     */
    virtual HRESULT Shutdown() = 0;
    
    /**
     * Reset the graphics device.
     * Called when device is lost (e.g., window resize).
     */
    virtual HRESULT Reset() = 0;
    
    // ========================================================================
    // Scene Operations
    // ========================================================================
    
    /**
     * Begin rendering a new frame.
     */
    virtual HRESULT BeginScene() = 0;
    
    /**
     * End rendering the current frame.
     */
    virtual HRESULT EndScene() = 0;
    
    /**
     * Present the rendered frame to the screen.
     */
    virtual HRESULT Present() = 0;
    
    /**
     * Clear the render target and/or depth buffer.
     * 
     * @param count Number of rectangles in rects array
     * @param rects Array of rectangles to clear (NULL = entire surface)
     * @param flags D3DCLEAR_* flags (TARGET, ZBUFFER, STENCIL)
     * @param color Color to clear to
     * @param z Depth value to clear to
     * @param stencil Stencil value to clear to
     */
    virtual HRESULT Clear(
        DWORD count,
        const D3DRECT* rects,
        DWORD flags,
        D3DCOLOR color,
        float z,
        DWORD stencil
    ) = 0;
    
    // ========================================================================
    // Texture Management
    // ========================================================================
    
    /**
     * Set the texture for a texture stage.
     * 
     * @param stage Texture stage index (0-7 typically)
     * @param texture Texture handle (void* to backend-specific data)
     */
    virtual HRESULT SetTexture(unsigned int stage, void* texture) = 0;
    
    /**
     * Get the current texture for a stage.
     * 
     * @param stage Texture stage index
     * @param texture Output pointer to texture handle
     */
    virtual HRESULT GetTexture(unsigned int stage, void** texture) = 0;
    
    /**
     * Create a new texture.
     * 
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     * @param format D3DFORMAT (e.g., D3DFMT_A8R8G8B8)
     * @param texture Output pointer to new texture handle
     */
    virtual HRESULT CreateTexture(
        unsigned int width,
        unsigned int height,
        D3DFORMAT format,
        void** texture
    ) = 0;
    
    /**
     * Release a texture resource.
     */
    virtual HRESULT ReleaseTexture(void* texture) = 0;
    
    /**
     * Lock a texture for CPU access.
     * 
     * @param texture Texture handle
     * @param data Output pointer to pixel data
     * @param pitch Output pitch (bytes per scanline)
     */
    virtual HRESULT LockTexture(void* texture, void** data, unsigned int* pitch) = 0;
    
    /**
     * Unlock a texture after CPU modifications.
     */
    virtual HRESULT UnlockTexture(void* texture) = 0;
    
    // ========================================================================
    // Render State Management
    // ========================================================================
    
    /**
     * Set a render state.
     * 
     * @param state D3DRENDERSTATETYPE (e.g., D3DRS_LIGHTING, D3DRS_ZENABLE)
     * @param value State value (meaning depends on state)
     */
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) = 0;
    
    /**
     * Get a render state value.
     */
    virtual HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) = 0;
    
    /**
     * Set texture operation (blend mode, etc).
     * 
     * @param stage Texture stage index
     * @param operation D3DTEXTUREOP value
     */
    virtual HRESULT SetTextureOp(unsigned int stage, D3DTEXTUREOP operation) = 0;
    
    // ========================================================================
    // Vertex/Index Buffers
    // ========================================================================
    
    /**
     * Create a vertex buffer.
     * 
     * @param size Size in bytes
     * @param usage D3DUSAGE_* flags
     * @param format Vertex format (FVF)
     * @param buffer Output pointer to buffer handle
     */
    virtual HRESULT CreateVertexBuffer(
        unsigned int size,
        unsigned int usage,
        unsigned int format,
        void** buffer
    ) = 0;
    
    /**
     * Release a vertex buffer.
     */
    virtual HRESULT ReleaseVertexBuffer(void* buffer) = 0;
    
    /**
     * Lock a vertex buffer for CPU access.
     * 
     * @param buffer Buffer handle
     * @param data Output pointer to vertex data
     * @param flags D3DLOCK_* flags
     */
    virtual HRESULT LockVertexBuffer(void* buffer, void** data, unsigned int flags) = 0;
    
    /**
     * Unlock a vertex buffer.
     */
    virtual HRESULT UnlockVertexBuffer(void* buffer) = 0;
    
    /**
     * Create an index buffer.
     * 
     * @param size Size in bytes
     * @param format Index format (16-bit or 32-bit)
     * @param buffer Output pointer to buffer handle
     */
    virtual HRESULT CreateIndexBuffer(
        unsigned int size,
        D3DFORMAT format,
        void** buffer
    ) = 0;
    
    /**
     * Release an index buffer.
     */
    virtual HRESULT ReleaseIndexBuffer(void* buffer) = 0;
    
    /**
     * Lock an index buffer for CPU access.
     */
    virtual HRESULT LockIndexBuffer(void* buffer, void** data, unsigned int flags) = 0;
    
    /**
     * Unlock an index buffer.
     */
    virtual HRESULT UnlockIndexBuffer(void* buffer) = 0;
    
    /**
     * Set the active vertex buffer stream.
     * 
     * @param stream Stream index
     * @param buffer Buffer handle
     * @param stride Bytes between vertices
     */
    virtual HRESULT SetStreamSource(
        unsigned int stream,
        void* buffer,
        unsigned int stride
    ) = 0;
    
    /**
     * Set the active index buffer.
     */
    virtual HRESULT SetIndices(void* buffer) = 0;
    
    // ========================================================================
    // Drawing Operations
    // ========================================================================
    
    /**
     * Draw vertices (non-indexed).
     * 
     * @param primitiveType D3DPRIMITIVETYPE (e.g., D3DPT_TRIANGLELIST)
     * @param startVertex Starting vertex index
     * @param primitiveCount Number of primitives to draw
     */
    virtual HRESULT DrawPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int startVertex,
        unsigned int primitiveCount
    ) = 0;
    
    /**
     * Draw indexed vertices.
     * 
     * @param primitiveType D3DPRIMITIVETYPE
     * @param baseVertexIndex Base vertex index offset
     * @param minVertexIndex Minimum vertex index
     * @param numVertices Number of vertices
     * @param startIndex Starting index in index buffer
     * @param primitiveCount Number of primitives to draw
     */
    virtual HRESULT DrawIndexedPrimitive(
        D3DPRIMITIVETYPE primitiveType,
        unsigned int baseVertexIndex,
        unsigned int minVertexIndex,
        unsigned int numVertices,
        unsigned int startIndex,
        unsigned int primitiveCount
    ) = 0;
    
    // ========================================================================
    // Viewport and Transform
    // ========================================================================
    
    /**
     * Set the viewport.
     * 
     * @param x Viewport left
     * @param y Viewport top
     * @param width Viewport width
     * @param height Viewport height
     * @param minZ Near plane
     * @param maxZ Far plane
     */
    virtual HRESULT SetViewport(
        unsigned int x,
        unsigned int y,
        unsigned int width,
        unsigned int height,
        float minZ,
        float maxZ
    ) = 0;
    
    /**
     * Set a transformation matrix.
     * 
     * @param state D3DTRANSFORMSTATETYPE (WORLD, VIEW, PROJECTION)
     * @param matrix 4x4 transformation matrix (16 floats)
     */
    virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const float* matrix) = 0;
    
    /**
     * Get a transformation matrix.
     */
    virtual HRESULT GetTransform(D3DTRANSFORMSTATETYPE state, float* matrix) = 0;
    
    // ========================================================================
    // Lighting
    // ========================================================================
    
    /**
     * Enable or disable lighting.
     * 
     * @param enable True to enable, false to disable
     */
    virtual HRESULT EnableLighting(bool enable) = 0;
    
    /**
     * Set a light source.
     * 
     * @param index Light index
     * @param light Light structure (D3DLIGHT8)
     */
    virtual HRESULT SetLight(unsigned int index, const D3DLIGHT8* light) = 0;
    
    /**
     * Enable/disable a light.
     */
    virtual HRESULT LightEnable(unsigned int index, bool enable) = 0;
    
    /**
     * Set material properties.
     */
    virtual HRESULT SetMaterial(const D3DMATERIAL8* material) = 0;
    
    /**
     * Set ambient light color.
     */
    virtual HRESULT SetAmbient(D3DCOLOR color) = 0;
    
    // ========================================================================
    // Utility / Debug
    // ========================================================================
    
    /**
     * Get the backend name (e.g., "Legacy Metal", "DXVK/Vulkan").
     */
    virtual const char* GetBackendName() const = 0;
    
    /**
     * Get the last error that occurred.
     */
    virtual HRESULT GetLastError() const = 0;
    
    /**
     * Enable/disable debug output.
     */
    virtual void SetDebugOutput(bool enable) = 0;
};

// ============================================================================
// Global Graphics Backend Instance
// ============================================================================

/**
 * Global graphics backend instance.
 * 
 * Set during application initialization based on USE_DXVK flag:
 * - USE_DXVK=OFF: LegacyGraphicsBackend (Metal/OpenGL)
 * - USE_DXVK=ON: DVKGraphicsBackend (DXVK/Vulkan)
 * 
 * All graphics operations should go through this pointer.
 */
extern IGraphicsBackend* g_graphicsBackend;

#endif // GRAPHICS_BACKEND_H
