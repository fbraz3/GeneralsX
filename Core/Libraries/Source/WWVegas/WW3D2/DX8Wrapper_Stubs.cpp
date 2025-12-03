/*
 * DX8Wrapper_Stubs.cpp
 * 
 * Phase 39.4: Global stub definitions for DirectX 8 compatibility layer
 * Updated Phase 41 Week 3: Integrated Graphics::IGraphicsDriver factory
 * Updated Phase 56: Real buffer binding for vertex/index buffers
 * 
 * Provides initialization for global objects referenced by game code.
 * Bridge between legacy DX8Wrapper calls and modern Graphics::IGraphicsDriver abstraction.
 */

#include "DX8Wrapper_Stubs.h"
#include "surfaceclass.h"  // Phase 51: For SurfaceClass back buffer
#include "../../Graphics/Vulkan/d3d8_memory_texture.h"  // Phase 51: Real texture/surface implementations
#include "../../Graphics/IGraphicsDriver.h"
#include "../../Graphics/GraphicsDriverFactory.h"
#include "../../Graphics/dx8buffer_compat.h"  // Phase 56: For DX8VertexBufferClass, DX8IndexBufferClass

// Phase 41 Week 3: Global graphics driver instance (created via factory)
static Graphics::IGraphicsDriver* g_graphics_driver = nullptr;

// Global DX8Caps instance (Phase 39.4)
static DX8Caps g_dx8_caps;

// Global mesh renderer stub instance (Phase 39.4: Changed from pointer to static object)
static DX8MeshRenderer g_dx8_mesh_renderer;
DX8MeshRenderer TheDX8MeshRenderer = g_dx8_mesh_renderer;

// Global Renderer Debugger stub (Phase 39.4)
class DX8RendererDebugger {
public:
    static void Add_Mesh(void*) {}  // No-op stub
};

// ============================================================================
// Phase 41 Week 3: Graphics Driver Integration
// ============================================================================

/**
 * Get the current graphics driver instance
 * Returns the driver created via Graphics::GraphicsDriverFactory
 */
Graphics::IGraphicsDriver* GetCurrentGraphicsDriver() {
    return g_graphics_driver;
}

/**
 * DX8Wrapper::Init - Initialize graphics driver via factory
 * Phase 41 Week 3: Create driver using abstract factory pattern
 * 
 * @param hwnd Window handle (cast from SDL_Window* on POSIX platforms)
 * @param lite Lite mode flag (unused with modern abstraction)
 * @return true if initialization successful
 */
bool DX8Wrapper::Init(void* hwnd, bool lite) {
    // Phase 41 Week 3: Create graphics driver via factory
    // Uses environment variable GRAPHICS_DRIVER if set, otherwise defaults to Vulkan
    
    printf("[DX8Wrapper::Init] Starting, hwnd=%p, lite=%d\n", hwnd, lite);
    
    printf("[Phase 41 Week 3] DX8Wrapper::Init - Creating graphics driver via factory\n");
    
    
    // Destroy existing driver if any
    if (g_graphics_driver != nullptr) {
        printf("[DX8Wrapper::Init] Destroying existing driver\n");
        
        printf("[Phase 41 Week 3] DX8Wrapper::Init - Destroying existing driver\n");
        Graphics::GraphicsDriverFactory::DestroyDriver(g_graphics_driver);
        g_graphics_driver = nullptr;
    }
    
    // Create new driver via factory (uses environment variable + config file priority)
    // Window dimensions are set to default 1024x768 for initialization
    printf("[DX8Wrapper::Init] Calling GraphicsDriverFactory::CreateDriver\n");
    
    g_graphics_driver = Graphics::GraphicsDriverFactory::CreateDriver(
        Graphics::BackendType::Unknown,  // Use default selection priority
        hwnd,
        1024,  // width
        768,   // height
        false  // fullscreen
    );
    
    if (g_graphics_driver == nullptr) {
        printf("[DX8Wrapper::Init] ERROR: CreateDriver returned nullptr\n");
        
        printf("[Phase 41 Week 3 ERROR] DX8Wrapper::Init - Failed to create graphics driver\n");
        return false;
    }
    
    printf("[DX8Wrapper::Init] Success, backend=%s\n", g_graphics_driver->GetBackendName());
    
    printf("[Phase 41 Week 3] DX8Wrapper::Init - Graphics driver created successfully: %s\n",
           g_graphics_driver->GetBackendName());
    
    return true;
}

/**
 * DX8Wrapper::Shutdown - Cleanup graphics driver
 * Phase 41 Week 3: Destroy driver created by factory
 */
void DX8Wrapper::Shutdown() {
    printf("[Phase 41 Week 3] DX8Wrapper::Shutdown - Destroying graphics driver\n");
    
    if (g_graphics_driver != nullptr) {
        Graphics::GraphicsDriverFactory::DestroyDriver(g_graphics_driver);
        g_graphics_driver = nullptr;
    }
}

/**
 * DX8Wrapper::Begin_Scene - Start frame rendering
 * Phase 41 Week 3: Delegate to Graphics::IGraphicsDriver::BeginFrame
 * 
 * Maps legacy DirectX 8 Begin_Scene call to abstract graphics driver interface.
 */
void DX8Wrapper::Begin_Scene() {
    printf("[Phase 54] DX8Wrapper::Begin_Scene - Called, g_graphics_driver=%p\n", (void*)g_graphics_driver);
    
    if (g_graphics_driver != nullptr) {
        printf("[Phase 54] DX8Wrapper::Begin_Scene - Calling g_graphics_driver->BeginFrame()\n");
        
        if (!g_graphics_driver->BeginFrame()) {
            printf("[Phase 41 Week 3 ERROR] DX8Wrapper::Begin_Scene - BeginFrame failed\n");
        }
        printf("[Phase 54] DX8Wrapper::Begin_Scene - BeginFrame returned\n");
        
    } else {
        printf("[Phase 41 Week 3 WARNING] DX8Wrapper::Begin_Scene - No graphics driver initialized\n");
    }
}

/**
 * DX8Wrapper::End_Scene - End frame rendering and present
 * Phase 41 Week 3: Delegate to Graphics::IGraphicsDriver::EndFrame + Present
 * 
 * Maps legacy DirectX 8 End_Scene call to abstract graphics driver interface.
 * 
 * @param flip_frame Whether to present the frame to display
 */
void DX8Wrapper::End_Scene(bool flip_frame) {
    if (g_graphics_driver != nullptr) {
        // EndFrame returns void, just call it
        g_graphics_driver->EndFrame();
        
        // Present frame if requested
        if (flip_frame) {
            if (!g_graphics_driver->Present()) {
                printf("[Phase 41 Week 3 ERROR] DX8Wrapper::End_Scene - Present failed\n");
            }
        }
    } else {
        printf("[Phase 41 Week 3 WARNING] DX8Wrapper::End_Scene - No graphics driver initialized\n");
    }
}

// ============================================================================
// Legacy Initialization Functions (for compatibility)
// ============================================================================

/**
 * Initialization function (called early in game startup)
 * Phase 41 Week 3: Maintains compatibility with existing code
 */
void InitializeDX8Stubs() {
    printf("[Phase 41 Week 3] InitializeDX8Stubs - DX8 stub system initialized\n");
}

/**
 * Cleanup function (called during game shutdown)
 * Phase 41 Week 3: Ensures graphics driver is properly cleaned up
 */
void ShutdownDX8Stubs() {
    printf("[Phase 41 Week 3] ShutdownDX8Stubs - Cleaning up DX8 stub system\n");
    DX8Wrapper::Shutdown();  // Ensure graphics driver is destroyed
}

/**
 * Get current capabilities (returns pointer to static global instance)
 * Note: This is called via DX8Wrapper::Get_Current_Caps()
 * Must define as extern to allow the static method to access it
 */
extern DX8Caps* Get_DX8_Caps_Ptr() {
    return &g_dx8_caps;
}

/**
 * Phase 39.4: Define global FVF type constant for legacy render2d.cpp
 */
const int dynamic_fvf_type = 0;

// ============================================================================
// Phase 43.1 -> Phase 51: Surface Creation (real implementations)
// ============================================================================

/**
 * Create an off-screen plain surface for texture data
 * Phase 51: Provides real memory-backed surface creation
 * 
 * Parameter format is expected to be a WW3DFormat enum value cast to int
 */
IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(int width, int height, int format)
{
    // Phase 51: Create real memory-backed surface
    printf("[Phase 51] DX8Wrapper::_Create_DX8_Surface(width=%d, height=%d, format=%d)\n", 
           width, height, format);
    
    return CreateMemorySurface(width, height, (D3DFORMAT)format);
}

/**
 * Load a surface from a file
 * Phase 51: Returns nullptr for now - file loading requires texture system
 */
IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(const char* filename)
{
    // Phase 51: File loading is complex - return nullptr for now
    // This would need integration with the texture loading system
    printf("[Phase 51] DX8Wrapper::_Create_DX8_Surface(filename='%s') - NOT IMPLEMENTED\n", filename);
    
    // TODO: Implement file loading when needed
    return nullptr;
}

// Static back buffer surface for smudge rendering
static SurfaceClass* g_back_buffer_surface = nullptr;

/**
 * Get the back buffer surface
 * Phase 51: Return a valid SurfaceClass for smudge rendering
 * 
 * The back buffer represents the current render target. For smudge rendering,
 * we need to return surface dimensions so the smudge system knows the render area.
 * 
 * @return SurfaceClass* Pointer to back buffer surface (caller should REF_PTR_RELEASE)
 */
SurfaceClass* DX8Wrapper::_Get_DX8_Back_Buffer()
{
    // Get current resolution from the graphics driver or use defaults
    int width = Peek_Device_Resolution_Width();
    int height = Peek_Device_Resolution_Height();
    
    // Create a surface that represents the back buffer
    // This is used primarily for querying dimensions, not actual pixel access
    IDirect3DSurface8* d3d_surface = CreateMemorySurface(
        (unsigned int)width, 
        (unsigned int)height, 
        (D3DFORMAT)D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT);
    if (!d3d_surface) {
        printf("[Phase 51] _Get_DX8_Back_Buffer: Failed to create D3D surface\n");
        return nullptr;
    }
    
    // Create SurfaceClass wrapper
    // Note: SurfaceClass takes ownership of the D3D surface
    SurfaceClass* surface = new SurfaceClass(d3d_surface);
    
    printf("[Phase 51] _Get_DX8_Back_Buffer: Created %dx%d surface=%p\n", 
            width, height, (void*)surface);
    
    return surface;
}

// ============================================================================
// Phase 56: Buffer Binding Functions
// ============================================================================

// ============================================================================
// Phase 60: Dynamic Buffer Support for DrawIndexedPrimitiveUP
// ============================================================================

// Storage for dynamic buffer data
static const void* g_dynamicVertexData = nullptr;
static uint32_t g_dynamicVertexCount = 0;
static uint32_t g_dynamicVertexStride = 0;
static const unsigned short* g_dynamicIndexData = nullptr;
static int g_dynamicIndexCount = 0;

/**
 * Phase 60: Store dynamic vertex buffer data for later use in Draw_Triangles
 */
void DX8Wrapper_SetDynamicVertexBuffer(const void* data, uint32_t vertexCount, uint32_t stride) {
    g_dynamicVertexData = data;
    g_dynamicVertexCount = vertexCount;
    g_dynamicVertexStride = stride;
}

/**
 * Phase 60: Store dynamic index buffer data for later use in Draw_Triangles
 */
void DX8Wrapper_SetDynamicIndexBuffer(const unsigned short* data, int indexCount) {
    g_dynamicIndexData = data;
    g_dynamicIndexCount = indexCount;
}

/**
 * Phase 60: Check if dynamic buffers are set (for DrawIndexedPrimitiveUP path)
 */
bool DX8Wrapper_HasDynamicBuffers() {
    return g_dynamicVertexData != nullptr && g_dynamicIndexData != nullptr;
}

// ============================================================================
// Phase 56: Internal Buffer Binding Functions
// ============================================================================

/**
 * Internal function to bind a vertex buffer to the graphics driver
 * Called by DX8Wrapper::Set_Vertex_Buffer static functions
 */
void DX8Wrapper_SetVertexBufferInternal(Graphics::VertexBufferHandle handle, uint32_t stride) {
    // Phase 60: Clear dynamic buffers when using static buffers
    g_dynamicVertexData = nullptr;
    g_dynamicVertexCount = 0;
    g_dynamicVertexStride = 0;
    
    if (g_graphics_driver != nullptr) {
        g_graphics_driver->SetVertexStreamSource(0, handle, 0, stride);
    }
}

/**
 * Internal function to bind an index buffer to the graphics driver
 * Called by DX8Wrapper::Set_Index_Buffer static functions
 */
void DX8Wrapper_SetIndexBufferInternal(Graphics::IndexBufferHandle handle, uint32_t startIndex) {
    // Phase 60: Clear dynamic buffers when using static buffers
    g_dynamicIndexData = nullptr;
    g_dynamicIndexCount = 0;
    
    if (g_graphics_driver != nullptr) {
        g_graphics_driver->SetIndexBuffer(handle, startIndex);
    }
}

/**
 * Phase 56/60: Draw triangles using current bound buffers
 * This is the real implementation that replaces the empty stub
 * 
 * Phase 60: Now detects dynamic buffers and uses DrawIndexedPrimitiveUP
 */
void DX8Wrapper_DrawTrianglesInternal(int start_index, int polygon_count, int min_vertex_index, int vertex_count) {
    if (g_graphics_driver == nullptr) return;
    
    // Phase 60: Check if we have dynamic buffers set (from DynamicVBAccessClass/DynamicIBAccessClass)
    if (g_dynamicVertexData != nullptr && g_dynamicIndexData != nullptr) {
        // Use DrawIndexedPrimitiveUP for dynamic buffers
        // This uploads data directly without needing pre-allocated GPU buffers
        
        g_graphics_driver->DrawIndexedPrimitiveUP(
            Graphics::PrimitiveType::TriangleList,
            (uint32_t)min_vertex_index,     // minVertexIndex
            (uint32_t)vertex_count,          // vertexCount
            (uint32_t)polygon_count,         // primCount (triangles)
            g_dynamicIndexData,              // indexData
            g_dynamicVertexData,             // vertexData
            g_dynamicVertexStride            // vertexStride
        );
        
        // Clear dynamic buffers after use to avoid stale data
        g_dynamicVertexData = nullptr;
        g_dynamicVertexCount = 0;
        g_dynamicVertexStride = 0;
        g_dynamicIndexData = nullptr;
        g_dynamicIndexCount = 0;
        return;
    }
    
    // For indexed drawing with static buffers: polygon_count * 3 = index count for triangles
    uint32_t index_count = (uint32_t)(polygon_count * 3);
    
    // Note: We use DrawIndexedPrimitive here as Draw_Triangles typically uses index buffers
    // The index buffer handle was set previously via Set_Index_Buffer
    // We pass INVALID_HANDLE and let the driver use the currently bound buffer
    g_graphics_driver->DrawIndexedPrimitive(
        Graphics::PrimitiveType::TriangleList,
        index_count,
        Graphics::INVALID_HANDLE,  // Use currently bound index buffer
        (uint32_t)start_index
    );
}

/**
 * Phase 56: Draw triangle strip using current bound buffers
 */
void DX8Wrapper_DrawStripInternal(int start_index, int primitive_count, int min_vertex_index, int vertex_count) {
    if (g_graphics_driver != nullptr) {
        // For triangle strip: primitive_count + 2 = vertex count
        uint32_t vertex_cnt = (uint32_t)(primitive_count + 2);
        
        g_graphics_driver->DrawPrimitive(
            Graphics::PrimitiveType::TriangleStrip,
            vertex_cnt
        );
    }
}

// ============================================================================
// Phase 56: DX8Wrapper Buffer Binding Methods
// ============================================================================

/**
 * Bind a DX8VertexBufferClass to the graphics driver
 * Extracts the driver handle and passes it to the Vulkan driver
 */
void DX8Wrapper::Set_Vertex_Buffer(DX8VertexBufferClass* vb) {
    if (vb != nullptr) {
        Graphics::VertexBufferHandle handle = vb->GetDriverHandle();
        // Get stride from FVF - need to calculate vertex size
        uint32_t stride = vb->GetSize() / vb->GetVertexCount();
        DX8Wrapper_SetVertexBufferInternal(handle, stride);
    } else {
        // Unbind vertex buffer
        DX8Wrapper_SetVertexBufferInternal(Graphics::INVALID_HANDLE, 0);
    }
}

void DX8Wrapper::Set_Vertex_Buffer(int stream_number, DX8VertexBufferClass* vb) {
    // Stream number is ignored for now - we only support stream 0
    Set_Vertex_Buffer(vb);
}

/**
 * Bind a DX8IndexBufferClass to the graphics driver
 * Extracts the driver handle and passes it to the Vulkan driver
 */
void DX8Wrapper::Set_Index_Buffer(DX8IndexBufferClass* ib, int start_index) {
    if (ib != nullptr) {
        Graphics::IndexBufferHandle handle = ib->GetDriverHandle();
        DX8Wrapper_SetIndexBufferInternal(handle, (uint32_t)start_index);
    } else {
        // Unbind index buffer
        DX8Wrapper_SetIndexBufferInternal(Graphics::INVALID_HANDLE, 0);
    }
}

void DX8Wrapper::Set_Index_Buffer(int stream_number, DX8IndexBufferClass* ib, int start_index) {
    // Stream number is ignored for now
    Set_Index_Buffer(ib, start_index);
}
// ============================================================================
// Phase 60: DynamicVBAccessClass and DynamicIBAccessClass Buffer Binding
// ============================================================================

/**
 * Bind a DynamicVBAccessClass to store vertex data for DrawIndexedPrimitiveUP
 */
void DX8Wrapper::Set_Vertex_Buffer(DynamicVBAccessClass* vb_access) {
    if (vb_access != nullptr) {
        DX8Wrapper_SetDynamicVertexBuffer(
            vb_access->Get_Vertex_Data(),
            vb_access->Get_Vertex_Count(),
            vb_access->Get_Vertex_Stride()
        );
    }
}

void DX8Wrapper::Set_Vertex_Buffer(int stream_number, DynamicVBAccessClass* vb_access) {
    Set_Vertex_Buffer(vb_access);
}

void DX8Wrapper::Set_Vertex_Buffer(DynamicVBAccessClass& vb_access) {
    DX8Wrapper_SetDynamicVertexBuffer(
        vb_access.Get_Vertex_Data(),
        vb_access.Get_Vertex_Count(),
        vb_access.Get_Vertex_Stride()
    );
}

void DX8Wrapper::Set_Vertex_Buffer(int stream_number, DynamicVBAccessClass& vb_access) {
    Set_Vertex_Buffer(vb_access);
}

/**
 * Bind a DynamicIBAccessClass to store index data for DrawIndexedPrimitiveUP
 */
void DX8Wrapper::Set_Index_Buffer(DynamicIBAccessClass* ib_access, int start_index) {
    if (ib_access != nullptr) {
        DX8Wrapper_SetDynamicIndexBuffer(
            ib_access->Get_Index_Data(),
            ib_access->Get_Index_Count()
        );
    }
}

void DX8Wrapper::Set_Index_Buffer(int stream_number, DynamicIBAccessClass* ib_access, int start_index) {
    Set_Index_Buffer(ib_access, start_index);
}

void DX8Wrapper::Set_Index_Buffer(DynamicIBAccessClass& ib_access, int start_index) {
    DX8Wrapper_SetDynamicIndexBuffer(
        ib_access.Get_Index_Data(),
        ib_access.Get_Index_Count()
    );
}

void DX8Wrapper::Set_Index_Buffer(int stream_number, DynamicIBAccessClass& ib_access, int start_index) {
    Set_Index_Buffer(ib_access, start_index);
}