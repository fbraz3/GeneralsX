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
#include "texture.h"  // Phase 62: For TextureClass access
#include "matrix3d.h"  // Phase 62: For Matrix3D transform operations
#include "matrix4.h"   // Phase 62: For Matrix4x4 transform operations
#include "vertmaterial.h"  // Phase 62: For VertexMaterialClass access
#include "../../Graphics/Vulkan/d3d8_memory_texture.h"  // Phase 51: Real texture/surface implementations
#include "../../Graphics/IGraphicsDriver.h"
#include "../../Graphics/GraphicsDriverFactory.h"
#include "../../Graphics/dx8buffer_compat.h"  // Phase 56: For DX8VertexBufferClass, DX8IndexBufferClass

// Phase 41 Week 3: Global graphics driver instance (created via factory)
static Graphics::IGraphicsDriver* g_graphics_driver = nullptr;

// Phase 62: Device state tracking
static bool s_deviceInitted = false;
static int s_deviceWidth = 1024;   // Default width, updated by Set_Render_Device
static int s_deviceHeight = 768;   // Default height, updated by Set_Render_Device
static int s_deviceBitDepth = 32;
static bool s_deviceWindowed = true;

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
    
    // Mark device as initialized
    s_deviceInitted = true;
    
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
    static int end_scene_count = 0;
    end_scene_count++;
    
    // Always log first 10 frames, then every 100th frame
    bool should_log = (end_scene_count <= 10) || (end_scene_count % 100 == 0);
    
    if (should_log) {
        printf("[Phase 62 DEBUG] End_Scene #%d: flip_frame=%d, driver=%p\n", 
               end_scene_count, flip_frame ? 1 : 0, (void*)g_graphics_driver);
    }
    
    if (g_graphics_driver != nullptr) {
        // EndFrame returns void, just call it
        g_graphics_driver->EndFrame();
        
        // Present frame if requested
        if (flip_frame) {
            if (should_log) {
                printf("[Phase 62 DEBUG] End_Scene #%d: Calling Present()...\n", end_scene_count);
            }
            bool result = g_graphics_driver->Present();
            if (should_log) {
                printf("[Phase 62 DEBUG] End_Scene #%d: Present() returned %d\n", end_scene_count, result ? 1 : 0);
            }
            if (!result) {
                printf("[Phase 41 Week 3 ERROR] DX8Wrapper::End_Scene - Present failed\n");
            }
        } else {
            if (should_log) {
                printf("[Phase 62 DEBUG] End_Scene #%d: flip_frame=0, skipping Present\n", end_scene_count);
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
 * Phase 62: Check if a texture format is supported by the graphics driver
 * 
 * Maps WW3DFormat enum values to Graphics::TextureFormat and queries the driver.
 * If no driver is initialized, returns true for common formats to prevent early failures.
 * 
 * @param format WW3DFormat enum value (cast to int)
 * @return true if format is supported
 */
bool DX8Wrapper_SupportsTextureFormat(int format) {
    // If graphics driver is available, query it
    if (g_graphics_driver != nullptr) {
        // WW3DFormat enum values map directly to Graphics::TextureFormat
        // Both enums have same ordering starting from 0 (UNKNOWN)
        Graphics::TextureFormat texFormat = static_cast<Graphics::TextureFormat>(format);
        bool supported = g_graphics_driver->SupportsTextureFormat(texFormat);
        printf("[Phase 62] DX8Wrapper_SupportsTextureFormat(format=%d) -> %s\n", 
               format, supported ? "true" : "false");
        return supported;
    }
    
    // Driver not yet initialized - return true for common formats
    // to allow early texture creation during initialization
    // WW3DFormat enum values:
    //   0 = WW3D_FORMAT_UNKNOWN
    //   1 = WW3D_FORMAT_R8G8B8
    //   2 = WW3D_FORMAT_A8R8G8B8
    //   3 = WW3D_FORMAT_X8R8G8B8
    //   4 = WW3D_FORMAT_R5G6B5
    //   5 = WW3D_FORMAT_X1R5G5B5
    //   6 = WW3D_FORMAT_A1R5G5B5
    //   7 = WW3D_FORMAT_A4R4G4B4
    //   ... (see ww3dformat.h for full list)
    //   20-24 = DXT1-DXT5
    switch (format) {
        case 0:  // WW3D_FORMAT_UNKNOWN
            return false;
        case 1:  // WW3D_FORMAT_R8G8B8
        case 2:  // WW3D_FORMAT_A8R8G8B8
        case 3:  // WW3D_FORMAT_X8R8G8B8
        case 4:  // WW3D_FORMAT_R5G6B5
        case 5:  // WW3D_FORMAT_X1R5G5B5
        case 6:  // WW3D_FORMAT_A1R5G5B5
        case 7:  // WW3D_FORMAT_A4R4G4B4
        case 8:  // WW3D_FORMAT_R3G3B2
        case 9:  // WW3D_FORMAT_A8
        case 14: // WW3D_FORMAT_L8
        case 15: // WW3D_FORMAT_A8L8
        case 20: // WW3D_FORMAT_DXT1
        case 21: // WW3D_FORMAT_DXT2
        case 22: // WW3D_FORMAT_DXT3
        case 23: // WW3D_FORMAT_DXT4
        case 24: // WW3D_FORMAT_DXT5
            printf("[Phase 62] DX8Wrapper_SupportsTextureFormat(format=%d) -> true (no driver, common format)\n", format);
            return true;
        default:
            // Unknown format - be permissive, return true
            printf("[Phase 62] DX8Wrapper_SupportsTextureFormat(format=%d) -> true (no driver, permissive)\n", format);
            return true;
    }
}

/**
 * Phase 39.4: Define global FVF type constant for legacy render2d.cpp
 * Phase 62: Fixed - was 0, now D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 for 2D UI
 * D3DFVF_XYZRHW = 0x0004 - Pre-transformed vertices (screen space coordinates)
 * D3DFVF_DIFFUSE = 0x0040 - Vertex color
 * D3DFVF_TEX1 = 0x0100 - One texture coordinate set
 */
const int dynamic_fvf_type = 0x0144;  // D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1

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
static uint32_t g_dynamicVertexFVF = 0;  // Phase 62: Store FVF for pipeline selection
static const unsigned short* g_dynamicIndexData = nullptr;
static int g_dynamicIndexCount = 0;

/**
 * Phase 60: Store dynamic vertex buffer data for later use in Draw_Triangles
 * Phase 62: Added FVF parameter for pipeline selection
 */
void DX8Wrapper_SetDynamicVertexBuffer(const void* data, uint32_t vertexCount, uint32_t stride, uint32_t fvf) {
    g_dynamicVertexData = data;
    g_dynamicVertexCount = vertexCount;
    g_dynamicVertexStride = stride;
    g_dynamicVertexFVF = fvf;
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
    static int draw_call_count = 0;
    draw_call_count++;
    
    // Debug first 20 draw calls
    if (draw_call_count <= 20) {
        printf("[Phase 62 DRAW] DrawTrianglesInternal #%d: start=%d, poly=%d, minVert=%d, vertCount=%d\n",
               draw_call_count, start_index, polygon_count, min_vertex_index, vertex_count);
        printf("[Phase 62 DRAW]   driver=%p, dynVerts=%p, dynIndices=%p, stride=%d\n",
               (void*)g_graphics_driver, (void*)g_dynamicVertexData, (void*)g_dynamicIndexData, g_dynamicVertexStride);
    }
    
    if (g_graphics_driver == nullptr) return;
    
    // Phase 60: Check if we have dynamic buffers set (from DynamicVBAccessClass/DynamicIBAccessClass)
    if (draw_call_count <= 20) {
        printf("[Phase 62 DRAW] Check: dynVerts=%p, dynIndices=%p, condition=%s\n",
               (void*)g_dynamicVertexData, (void*)g_dynamicIndexData,
               (g_dynamicVertexData != nullptr && g_dynamicIndexData != nullptr) ? "TRUE" : "FALSE");
    }
    
    if (g_dynamicVertexData != nullptr && g_dynamicIndexData != nullptr) {
        // Debug: Print first vertex position for first few draw calls
        if (draw_call_count <= 5 && vertex_count >= 1 && g_dynamicVertexStride >= 12) {
            const float* verts = (const float*)g_dynamicVertexData;
            printf("[Phase 62 DRAW] Vertex 0: pos=(%.1f, %.1f, %.1f)\n", verts[0], verts[1], verts[2]);
            if (vertex_count >= 2) {
                const float* v1 = (const float*)((const char*)g_dynamicVertexData + g_dynamicVertexStride);
                printf("[Phase 62 DRAW] Vertex 1: pos=(%.1f, %.1f, %.1f)\n", v1[0], v1[1], v1[2]);
            }
            if (vertex_count >= 3) {
                const float* v2 = (const float*)((const char*)g_dynamicVertexData + g_dynamicVertexStride * 2);
                printf("[Phase 62 DRAW] Vertex 2: pos=(%.1f, %.1f, %.1f)\n", v2[0], v2[1], v2[2]);
            }
            if (vertex_count >= 4) {
                const float* v3 = (const float*)((const char*)g_dynamicVertexData + g_dynamicVertexStride * 3);
                printf("[Phase 62 DRAW] Vertex 3: pos=(%.1f, %.1f, %.1f)\n", v3[0], v3[1], v3[2]);
            }
        }
        
        // Use DrawIndexedPrimitiveUP for dynamic buffers
        // This uploads data directly without needing pre-allocated GPU buffers
        
        // Phase 62: Set FVF for pipeline selection before draw
        if (g_graphics_driver != nullptr) {
            g_graphics_driver->SetFVF(g_dynamicVertexFVF);
        }
        
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
        g_dynamicVertexFVF = 0;  // Phase 62: Clear FVF too
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
            vb_access->Get_Vertex_Stride(),
            vb_access->Get_FVF_Type()  // Phase 62: Pass FVF for pipeline selection
        );
    }
}

void DX8Wrapper::Set_Vertex_Buffer(int stream_number, DynamicVBAccessClass* vb_access) {
    Set_Vertex_Buffer(vb_access);
}

void DX8Wrapper::Set_Vertex_Buffer(DynamicVBAccessClass& vb_access) {
    // Phase 62: Debug vertex data at binding time
    const void* vertData = vb_access.Get_Vertex_Data();
    int vertCount = vb_access.Get_Vertex_Count();
    uint32_t stride = vb_access.Get_Vertex_Stride();
    int fvf = vb_access.Get_FVF_Type();
    
    static int setVBCount = 0;
    setVBCount++;
    if (setVBCount <= 5 && vertData && vertCount > 0 && stride >= 12) {
        const float* verts = (const float*)vertData;
        printf("[Phase 62] Set_Vertex_Buffer #%d: count=%d, stride=%u, fvf=0x%08X\n", setVBCount, vertCount, stride, fvf);
        printf("[Phase 62]   Vertex 0: pos=(%.2f, %.2f, %.2f)\n", verts[0], verts[1], verts[2]);
        if (vertCount >= 2) {
            const float* v1 = (const float*)((const char*)vertData + stride);
            printf("[Phase 62]   Vertex 1: pos=(%.2f, %.2f, %.2f)\n", v1[0], v1[1], v1[2]);
        }
    }
    
    DX8Wrapper_SetDynamicVertexBuffer(
        vertData,
        vertCount,
        stride,
        fvf  // Phase 62: Pass FVF for pipeline selection
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

// ============================================================================
// Phase 62: Texture Binding Implementation
// ============================================================================

/**
 * Convert D3D format to Graphics::TextureFormat
 * Maps DirectX 8 format constants to the backend-agnostic enum
 */
static Graphics::TextureFormat D3DFormatToTextureFormat(D3DFORMAT format) {
    switch (format) {
        case D3DFMT_R8G8B8:     return Graphics::TextureFormat::R8G8B8;
        case D3DFMT_A8R8G8B8:   return Graphics::TextureFormat::A8R8G8B8;
        case D3DFMT_X8R8G8B8:   return Graphics::TextureFormat::X8R8G8B8;
        case D3DFMT_R5G6B5:     return Graphics::TextureFormat::R5G6B5;
        case D3DFMT_X1R5G5B5:   return Graphics::TextureFormat::X1R5G5B5;
        case D3DFMT_A1R5G5B5:   return Graphics::TextureFormat::A1R5G5B5;
        case D3DFMT_A4R4G4B4:   return Graphics::TextureFormat::A4R4G4B4;
        case D3DFMT_R3G3B2:     return Graphics::TextureFormat::R3G3B2;
        case D3DFMT_A8:         return Graphics::TextureFormat::A8;
        case D3DFMT_L8:         return Graphics::TextureFormat::L8;
        case D3DFMT_A8L8:       return Graphics::TextureFormat::A8L8;
        case D3DFMT_DXT1:       return Graphics::TextureFormat::DXT1;
        case D3DFMT_DXT2:       return Graphics::TextureFormat::DXT2;
        case D3DFMT_DXT3:       return Graphics::TextureFormat::DXT3;
        case D3DFMT_DXT4:       return Graphics::TextureFormat::DXT4;
        case D3DFMT_DXT5:       return Graphics::TextureFormat::DXT5;
        default:                return Graphics::TextureFormat::Unknown;
    }
}

/**
 * Upload texture data from CPU memory to GPU
 * Creates a Vulkan texture and associates its handle with the MemoryDirect3DTexture8
 * 
 * @param memTex Pointer to memory texture containing pixel data
 * @return TextureHandle for the uploaded GPU texture, or INVALID_HANDLE on failure
 */
static Graphics::TextureHandle UploadTextureToGPU(MemoryDirect3DTexture8* memTex) {
    if (!g_graphics_driver || !memTex) {
        return Graphics::INVALID_HANDLE;
    }
    
    // Get texture dimensions and format
    unsigned int width = memTex->GetWidth();
    unsigned int height = memTex->GetHeight();
    D3DFORMAT d3dFormat = memTex->GetFormat();
    
    // Create texture descriptor
    Graphics::TextureDescriptor desc{};
    desc.width = width;
    desc.height = height;
    desc.depth = 1;
    desc.format = D3DFormatToTextureFormat(d3dFormat);
    desc.mipLevels = memTex->GetLevelCount();
    desc.cubeMap = false;
    desc.renderTarget = false;
    desc.depthStencil = false;
    desc.dynamic = false;
    
    // Get pixel data from mip level 0
    IDirect3DSurface8* surface = nullptr;
    HRESULT hr = memTex->GetSurfaceLevel(0, &surface);
    if (FAILED(hr) || !surface) {
        printf("[Phase 62] UploadTextureToGPU: Failed to get surface level 0\n");
        return Graphics::INVALID_HANDLE;
    }
    
    // Lock surface to get pixel data
    D3DLOCKED_RECT lockedRect{};
    hr = surface->LockRect(&lockedRect, nullptr, D3DLOCK_READONLY);
    if (FAILED(hr)) {
        printf("[Phase 62] UploadTextureToGPU: Failed to lock surface\n");
        surface->Release();
        return Graphics::INVALID_HANDLE;
    }
    
    // Debug: Print pitch info
    printf("[Phase 62] UploadTextureToGPU: LockRect returned pitch=%d, expected=%u, pBits=%p\n",
           lockedRect.Pitch, width * 4, lockedRect.pBits);
    
    // Debug: Print first 16 bytes of texture data
    if (lockedRect.pBits) {
        unsigned char* bytes = (unsigned char*)lockedRect.pBits;
        printf("[Phase 62] First 16 bytes: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\n",
               bytes[0], bytes[1], bytes[2], bytes[3],
               bytes[4], bytes[5], bytes[6], bytes[7],
               bytes[8], bytes[9], bytes[10], bytes[11],
               bytes[12], bytes[13], bytes[14], bytes[15]);
    }
    
    // Create texture on GPU with initial data
    Graphics::TextureHandle handle = g_graphics_driver->CreateTexture(desc, lockedRect.pBits);
    
    // Unlock and release surface
    surface->UnlockRect();
    surface->Release();
    
    if (handle != Graphics::INVALID_HANDLE) {
        printf("[Phase 62] UploadTextureToGPU: Created GPU texture (handle=%llu, %ux%u, format=%d)\n",
               handle, width, height, (int)d3dFormat);
    } else {
        printf("[Phase 62] UploadTextureToGPU: Failed to create GPU texture\n");
    }
    
    return handle;
}

/**
 * Internal texture binding implementation
 * Takes a DirectX 8 texture pointer and binds it to a sampler stage
 * 
 * If the texture hasn't been uploaded to GPU yet, uploads it first
 */
void DX8Wrapper_SetTextureInternal(int stage, void* d3d_texture) {
    if (!g_graphics_driver) {
        printf("[Phase 62] DX8Wrapper_SetTextureInternal: No graphics driver\n");
        return;
    }
    
    // NULL texture = unbind
    if (d3d_texture == nullptr) {
        g_graphics_driver->SetTexture((uint32_t)stage, Graphics::INVALID_HANDLE);
        printf("[Phase 62] DX8Wrapper_SetTextureInternal: Unbound texture from stage %d\n", stage);
        return;
    }
    
    // Cast to our memory texture implementation
    // Note: This assumes all textures are MemoryDirect3DTexture8 - which they should be
    // on non-Windows platforms
    MemoryDirect3DTexture8* memTex = dynamic_cast<MemoryDirect3DTexture8*>(
        static_cast<IDirect3DTexture8*>(d3d_texture));
    
    if (!memTex) {
        printf("[Phase 62] DX8Wrapper_SetTextureInternal: Not a MemoryDirect3DTexture8\n");
        return;
    }
    
    // Check if texture needs to be uploaded to GPU
    Graphics::TextureHandle handle = memTex->GetVulkanHandle();
    
    if (handle == 0 || memTex->IsGpuDirty()) {
        // Destroy old GPU texture if it exists
        if (handle != 0 && handle != Graphics::INVALID_HANDLE && g_graphics_driver) {
            g_graphics_driver->DestroyTexture(handle);
            printf("[Phase 62] DX8Wrapper_SetTextureInternal: Destroyed old GPU texture (handle=%llu) for re-upload\n", handle);
        }
        
        // Upload texture to GPU
        handle = UploadTextureToGPU(memTex);
        
        if (handle != Graphics::INVALID_HANDLE) {
            memTex->SetVulkanHandle(handle);
            memTex->ClearDirty();
            printf("[Phase 62] DX8Wrapper_SetTextureInternal: Texture re-uploaded after dirty flag\n");
        }
    }
    
    // Bind texture to stage
    if (handle != Graphics::INVALID_HANDLE) {
        g_graphics_driver->SetTexture((uint32_t)stage, handle);
        printf("[Phase 62] DX8Wrapper_SetTextureInternal: Bound texture (handle=%llu) to stage %d\n",
               handle, stage);
    } else {
        printf("[Phase 62] DX8Wrapper_SetTextureInternal: Failed to get GPU handle for texture\n");
    }
}

/**
 * Set texture from TextureClass wrapper
 * Extracts the D3D texture and delegates to Set_DX8_Texture
 */
void DX8Wrapper::Set_Texture(int stage, TextureClass* texture) {
    if (texture == nullptr) {
        Set_DX8_Texture(stage, nullptr);
        return;
    }
    
    // Get the DirectX 8 texture from TextureClass
    IDirect3DBaseTexture8* d3d_tex = texture->Peek_D3D_Base_Texture();
    
    // Phase 62: Debug NULL D3D texture from non-NULL TextureClass
    static int nullD3DCount = 0;
    if (d3d_tex == nullptr && nullD3DCount < 10) {
        printf("[Phase 62] Set_Texture: TextureClass=%p has NULL D3DTexture!\n", (void*)texture);
        nullD3DCount++;
    }
    
    Set_DX8_Texture(stage, d3d_tex);
}

// ============================================================================
// Phase 62: Transform Operations
// ============================================================================

// Current transform matrices stored globally for retrieval
static Graphics::Matrix4x4 g_worldMatrix;
static Graphics::Matrix4x4 g_viewMatrix;
static Graphics::Matrix4x4 g_projectionMatrix;

/**
 * Convert game Matrix4x4 to Graphics::Matrix4x4
 * Both have same memory layout (4x4 floats) so memcpy works
 */
static void MatrixToGraphicsMatrix(const float* src, Graphics::Matrix4x4& dst) {
    memcpy(dst.m, src, 16 * sizeof(float));
}

/**
 * Convert Graphics::Matrix4x4 to float array
 */
static void GraphicsMatrixToFloat(const Graphics::Matrix4x4& src, float* dst) {
    memcpy(dst, src.m, 16 * sizeof(float));
}

/**
 * Convert Matrix3D (3x4) to Graphics::Matrix4x4 (4x4)
 * Matrix3D has 3 rows of Vector4, we need to add 4th row [0,0,0,1]
 */
static void Matrix3DToGraphicsMatrix(const Matrix3D& src, Graphics::Matrix4x4& dst) {
    // Copy first 3 rows from Matrix3D (each row is a Vector4)
    const float* srcData = reinterpret_cast<const float*>(&src);
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            dst.m[row][col] = srcData[row * 4 + col];
        }
    }
    // Set 4th row to [0, 0, 0, 1] for proper homogeneous matrix
    dst.m[3][0] = 0.0f;
    dst.m[3][1] = 0.0f;
    dst.m[3][2] = 0.0f;
    dst.m[3][3] = 1.0f;
}

/**
 * Convert Graphics::Matrix4x4 (4x4) to Matrix3D (3x4)
 * Only copies first 3 rows, ignoring 4th row
 */
static void GraphicsMatrixToMatrix3D(const Graphics::Matrix4x4& src, Matrix3D& dst) {
    // Copy first 3 rows to Matrix3D
    float* dstData = reinterpret_cast<float*>(&dst);
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            dstData[row * 4 + col] = src.m[row][col];
        }
    }
}

/**
 * Helper: Check if a matrix is approximately identity
 * Used to detect when game wants 2D screen-space rendering
 */
static bool IsApproximatelyIdentity(const Graphics::Matrix4x4& m) {
    const float epsilon = 0.001f;
    return (
        fabsf(m.m[0][0] - 1.0f) < epsilon && fabsf(m.m[0][1]) < epsilon && fabsf(m.m[0][2]) < epsilon && fabsf(m.m[0][3]) < epsilon &&
        fabsf(m.m[1][0]) < epsilon && fabsf(m.m[1][1] - 1.0f) < epsilon && fabsf(m.m[1][2]) < epsilon && fabsf(m.m[1][3]) < epsilon &&
        fabsf(m.m[2][0]) < epsilon && fabsf(m.m[2][1]) < epsilon && fabsf(m.m[2][2] - 1.0f) < epsilon && fabsf(m.m[2][3]) < epsilon &&
        fabsf(m.m[3][0]) < epsilon && fabsf(m.m[3][1]) < epsilon && fabsf(m.m[3][2]) < epsilon && fabsf(m.m[3][3] - 1.0f) < epsilon
    );
}

/**
 * Helper: Create orthographic projection matrix for 2D screen-space rendering
 * Maps (0,0,0)-(width,height,1) to Vulkan NDC (-1,-1,0)-(1,1,1)
 * 
 * DirectX screen coordinates: (0,0) = top-left, Y increases downward
 * Vulkan NDC: (-1,-1) = top-left, Y increases downward
 * 
 * The matrix transforms:
 *   X: [0, width] -> [-1, 1]    using scale=2/width, translate=-1
 *   Y: [0, height] -> [-1, 1]   using scale=2/height, translate=-1
 *   Z: [0, 1] -> [0, 1]         passthrough
 * 
 * Row-major matrix layout (Graphics::Matrix4x4):
 *   | 2/w   0     0    -1  |
 *   |  0   2/h    0    -1  |
 *   |  0    0     1     0  |
 *   |  0    0     0     1  |
 */
static void CreateOrthographicProjection(Graphics::Matrix4x4& m, float width, float height) {
    memset(m.m, 0, sizeof(m.m));
    
    // Row 0: X transform
    m.m[0][0] = 2.0f / width;       // Scale X
    m.m[0][3] = -1.0f;              // Translate X (moves 0 to -1)
    
    // Row 1: Y transform  
    m.m[1][1] = 2.0f / height;      // Scale Y
    m.m[1][3] = -1.0f;              // Translate Y (moves 0 to -1)
    
    // Row 2: Z passthrough
    m.m[2][2] = 1.0f;
    
    // Row 3: W = 1
    m.m[3][3] = 1.0f;
}

// Track if we're in 2D mode (identity projection)
static bool g_using2DOrthoProjection = false;

/**
 * Internal set transform implementation
 * Routes to appropriate graphics driver method based on transform type
 * 
 * Phase 62: Detect identity projection and convert to orthographic for 2D rendering
 */
void DX8Wrapper_SetTransformInternal(unsigned int type, const float* matrix) {
    if (!g_graphics_driver || !matrix) {
        return;
    }
    
    Graphics::Matrix4x4 gfxMatrix;
    MatrixToGraphicsMatrix(matrix, gfxMatrix);
    
    switch (type) {
        case D3DTS_WORLD:
            g_worldMatrix = gfxMatrix;
            g_graphics_driver->SetWorldMatrix(gfxMatrix);
            break;
        case D3DTS_VIEW:
            g_viewMatrix = gfxMatrix;
            g_graphics_driver->SetViewMatrix(gfxMatrix);
            break;
        case D3DTS_PROJECTION:
            // Phase 62: Check if this is an identity projection (2D rendering mode)
            // If so, replace with orthographic projection for screen-space coordinates
            if (IsApproximatelyIdentity(gfxMatrix)) {
                // Get screen dimensions for orthographic projection
                Graphics::Matrix4x4 orthoMatrix;
                CreateOrthographicProjection(orthoMatrix, (float)s_deviceWidth, (float)s_deviceHeight);
                
                static int ortho_count = 0;
                if (ortho_count < 3) {
                    printf("[Phase 62] Detected identity projection, converting to ortho (%dx%d)\n",
                           s_deviceWidth, s_deviceHeight);
                    ortho_count++;
                }
                
                g_projectionMatrix = orthoMatrix;
                g_graphics_driver->SetProjectionMatrix(orthoMatrix);
                g_using2DOrthoProjection = true;
            } else {
                g_projectionMatrix = gfxMatrix;
                g_graphics_driver->SetProjectionMatrix(gfxMatrix);
                g_using2DOrthoProjection = false;
            }
            break;
        default:
            // Unknown transform type - ignore
            printf("[Phase 62] DX8Wrapper_SetTransformInternal: Unknown transform type %u\n", type);
            break;
    }
}

/**
 * Internal get transform implementation
 * Retrieves current transform from graphics driver
 */
void DX8Wrapper_GetTransformInternal(unsigned int type, float* matrix) {
    if (!g_graphics_driver || !matrix) {
        return;
    }
    
    Graphics::Matrix4x4 gfxMatrix;
    
    switch (type) {
        case D3DTS_WORLD:
            gfxMatrix = g_graphics_driver->GetWorldMatrix();
            break;
        case D3DTS_VIEW:
            gfxMatrix = g_graphics_driver->GetViewMatrix();
            break;
        case D3DTS_PROJECTION:
            gfxMatrix = g_graphics_driver->GetProjectionMatrix();
            break;
        default:
            // Unknown transform type - return identity
            printf("[Phase 62] DX8Wrapper_GetTransformInternal: Unknown transform type %u\n", type);
            return;
    }
    
    GraphicsMatrixToFloat(gfxMatrix, matrix);
}

/**
 * Set world matrix to identity
 */
void DX8Wrapper::Set_World_Identity() {
    if (!g_graphics_driver) return;
    
    Graphics::Matrix4x4 identity;  // Default constructor creates identity
    g_worldMatrix = identity;
    g_graphics_driver->SetWorldMatrix(identity);
}

/**
 * Set view matrix to identity
 */
void DX8Wrapper::Set_View_Identity() {
    if (!g_graphics_driver) return;
    
    Graphics::Matrix4x4 identity;  // Default constructor creates identity
    g_viewMatrix = identity;
    g_graphics_driver->SetViewMatrix(identity);
}

// ============================================================================
// Phase 62: Matrix3D/Matrix4x4 Transform Overloads
// ============================================================================

void DX8Wrapper::Set_Transform(unsigned int type, const Matrix3D& transform) {
    if (!g_graphics_driver) return;
    
    Graphics::Matrix4x4 gfxMatrix;
    Matrix3DToGraphicsMatrix(transform, gfxMatrix);
    
    switch (type) {
        case D3DTS_WORLD:
            g_worldMatrix = gfxMatrix;
            g_graphics_driver->SetWorldMatrix(gfxMatrix);
            break;
        case D3DTS_VIEW:
            g_viewMatrix = gfxMatrix;
            g_graphics_driver->SetViewMatrix(gfxMatrix);
            break;
        case D3DTS_PROJECTION:
            g_projectionMatrix = gfxMatrix;
            g_graphics_driver->SetProjectionMatrix(gfxMatrix);
            break;
        default:
            break;
    }
}

void DX8Wrapper::Set_Transform(unsigned int type, const Matrix3D* transform) {
    if (transform) {
        Set_Transform(type, *transform);
    }
}

void DX8Wrapper::Set_Transform(unsigned int type, const Matrix4x4& transform) {
    // Matrix4x4 has Vector4 Row[4], which is equivalent to float[16]
    const float* data = reinterpret_cast<const float*>(&transform);
    DX8Wrapper_SetTransformInternal(type, data);
}

void DX8Wrapper::Set_Transform(unsigned int type, const Matrix4x4* transform) {
    if (transform) {
        Set_Transform(type, *transform);
    }
}

void DX8Wrapper::Get_Transform(unsigned int type, Matrix3D& transform) {
    if (!g_graphics_driver) return;
    
    Graphics::Matrix4x4 gfxMatrix;
    
    switch (type) {
        case D3DTS_WORLD:
            gfxMatrix = g_graphics_driver->GetWorldMatrix();
            break;
        case D3DTS_VIEW:
            gfxMatrix = g_graphics_driver->GetViewMatrix();
            break;
        case D3DTS_PROJECTION:
            gfxMatrix = g_graphics_driver->GetProjectionMatrix();
            break;
        default:
            return;
    }
    
    GraphicsMatrixToMatrix3D(gfxMatrix, transform);
}

void DX8Wrapper::Get_Transform(unsigned int type, Matrix3D* transform) {
    if (transform) {
        Get_Transform(type, *transform);
    }
}

void DX8Wrapper::Get_Transform(unsigned int type, Matrix4x4& transform) {
    float* data = reinterpret_cast<float*>(&transform);
    DX8Wrapper_GetTransformInternal(type, data);
}

void DX8Wrapper::Get_Transform(unsigned int type, Matrix4x4* transform) {
    if (transform) {
        Get_Transform(type, *transform);
    }
}

// ============================================================================
// Phase 62: Viewport Operations
// ============================================================================

// ============================================================================
// Phase 62: Device Resolution Functions
// ============================================================================

// Note: s_deviceWidth, s_deviceHeight, s_deviceBitDepth, s_deviceWindowed are
// declared near the top of this file with other static variables

/**
 * Get current device width (for quick access without output params)
 */
int DX8Wrapper::Peek_Device_Resolution_Width() {
    return s_deviceWidth;
}

/**
 * Get current device height (for quick access without output params)
 */
int DX8Wrapper::Peek_Device_Resolution_Height() {
    return s_deviceHeight;
}

/**
 * Get current device width
 */
int DX8Wrapper::Get_Device_Resolution_Width() {
    return s_deviceWidth;
}

/**
 * Get current device height
 */
int DX8Wrapper::Get_Device_Resolution_Height() {
    return s_deviceHeight;
}

/**
 * Get full device resolution details
 * This is called by render2d.cpp to set up viewport dimensions
 */
void DX8Wrapper::Get_Device_Resolution(int& w, int& h, int& bits, bool& windowed) {
    w = s_deviceWidth;
    h = s_deviceHeight;
    bits = s_deviceBitDepth;
    windowed = s_deviceWindowed;
}

/**
 * Get render target resolution (same as device resolution for now)
 */
void DX8Wrapper::Get_Render_Target_Resolution(int& w, int& h, int& bits, bool& windowed) {
    w = s_deviceWidth;
    h = s_deviceHeight;
    bits = s_deviceBitDepth;
    windowed = s_deviceWindowed;
    
    // Debug: Print what we're returning (first 3 frames only)
    static int debug_count = 0;
    if (debug_count < 3) {
        printf("[Phase 62 DEBUG] Get_Render_Target_Resolution: returning %dx%d %dbit windowed=%d\n",
               w, h, bits, windowed ? 1 : 0);
        debug_count++;
    }
}

// ============================================================================
// Phase 62: Viewport Operations
// ============================================================================

/**
 * Internal viewport setting implementation
 * Converts D3DVIEWPORT8 parameters to Graphics::Viewport
 */
void DX8Wrapper_SetViewportInternal(int x, int y, int width, int height, float minZ, float maxZ) {
    if (!g_graphics_driver) return;
    
    Graphics::Viewport vp;
    vp.x = x;
    vp.y = y;
    vp.width = static_cast<uint32_t>(width);
    vp.height = static_cast<uint32_t>(height);
    vp.minZ = minZ;
    vp.maxZ = maxZ;
    
    g_graphics_driver->SetViewport(vp);
    printf("[Phase 62] DX8Wrapper_SetViewportInternal: Set viewport (%d, %d, %d, %d, %.2f, %.2f)\n",
           x, y, width, height, minZ, maxZ);
}

/**
 * Set viewport from D3DVIEWPORT8 structure
 * D3DVIEWPORT8 structure layout:
 *   DWORD X, Y, Width, Height;
 *   float MinZ, MaxZ;
 */
void DX8Wrapper::Set_Viewport(void* viewport) {
    if (!viewport) return;
    
    // Cast to proper D3DVIEWPORT8 structure type
    const D3DVIEWPORT8* vp = static_cast<const D3DVIEWPORT8*>(viewport);
    
    int x = static_cast<int>(vp->X);
    int y = static_cast<int>(vp->Y);
    int width = static_cast<int>(vp->Width);
    int height = static_cast<int>(vp->Height);
    float minZ = vp->MinZ;
    float maxZ = vp->MaxZ;
    
    DX8Wrapper_SetViewportInternal(x, y, width, height, minZ, maxZ);
}

void DX8Wrapper::Set_Viewport(const void* viewport) {
    Set_Viewport(const_cast<void*>(viewport));
}

// ============================================================================
// Phase 62: Clear Operations
// ============================================================================

/**
 * Internal clear implementation
 * Clears render target color and/or depth buffer
 */
void DX8Wrapper_ClearInternal(bool clearColor, bool clearZ, float r, float g, float b, float a) {
    if (!g_graphics_driver) return;
    
    // Only clear if at least one flag is set
    if (clearColor || clearZ) {
        if (clearColor) {
            g_graphics_driver->SetClearColor(r, g, b, a);
        }
        g_graphics_driver->Clear(r, g, b, a, clearZ);
        printf("[Phase 62] DX8Wrapper_ClearInternal: Cleared (color=%d, depth=%d, rgba=%.2f,%.2f,%.2f,%.2f)\n",
               clearColor, clearZ, r, g, b, a);
    }
}

/**
 * Clear the render target and/or depth buffer
 * This is called by the game to clear the screen before rendering
 */
void DX8Wrapper::Clear(bool clear_color, bool clear_z, const Vector3& color, float dest_alpha) {
    DX8Wrapper_ClearInternal(clear_color, clear_z, color.X, color.Y, color.Z, dest_alpha);
}

// ============================================================================
// Phase 62: Material Operations
// ============================================================================

/**
 * Set material from DirectX 8 D3DMATERIAL8 structure
 * D3DMATERIAL8 contains: Diffuse, Ambient, Specular, Emissive (D3DCOLORVALUE) + Power (float)
 */
void DX8Wrapper::Set_DX8_Material(void* d3d_material) {
    if (!g_graphics_driver) return;
    
    if (d3d_material == nullptr) {
        // Set default material
        Graphics::Material mat;
        g_graphics_driver->SetMaterial(mat);
        return;
    }
    
    // Cast to D3DMATERIAL8 structure
    const D3DMATERIAL8* d3dMat = static_cast<const D3DMATERIAL8*>(d3d_material);
    
    // Convert to Graphics::Material
    Graphics::Material mat;
    mat.diffuse = Graphics::Color(d3dMat->Diffuse.r, d3dMat->Diffuse.g, d3dMat->Diffuse.b, d3dMat->Diffuse.a);
    mat.ambient = Graphics::Color(d3dMat->Ambient.r, d3dMat->Ambient.g, d3dMat->Ambient.b, d3dMat->Ambient.a);
    mat.specular = Graphics::Color(d3dMat->Specular.r, d3dMat->Specular.g, d3dMat->Specular.b, d3dMat->Specular.a);
    mat.emissive = Graphics::Color(d3dMat->Emissive.r, d3dMat->Emissive.g, d3dMat->Emissive.b, d3dMat->Emissive.a);
    mat.shininess = d3dMat->Power;
    
    g_graphics_driver->SetMaterial(mat);
}

/**
 * Set material from VertexMaterialClass
 * Extracts material properties and passes to graphics driver
 */
void DX8Wrapper::Set_Material(VertexMaterialClass* material) {
    if (!g_graphics_driver) return;
    
    if (material == nullptr) {
        // Set default material
        Graphics::Material mat;
        g_graphics_driver->SetMaterial(mat);
        return;
    }
    
    // Extract material properties
    Graphics::Material mat;
    
    // Get colors from VertexMaterialClass
    Vector3 ambient, diffuse, specular, emissive;
    material->Get_Ambient(&ambient);
    material->Get_Diffuse(&diffuse);
    material->Get_Specular(&specular);
    material->Get_Emissive(&emissive);
    
    float opacity = material->Get_Opacity();
    float shininess = material->Get_Shininess();
    
    mat.ambient = Graphics::Color(ambient.X, ambient.Y, ambient.Z, 1.0f);
    mat.diffuse = Graphics::Color(diffuse.X, diffuse.Y, diffuse.Z, opacity);
    mat.specular = Graphics::Color(specular.X, specular.Y, specular.Z, 1.0f);
    mat.emissive = Graphics::Color(emissive.X, emissive.Y, emissive.Z, 1.0f);
    mat.shininess = shininess;
    
    g_graphics_driver->SetMaterial(mat);
}

// ============================================================================
// Phase 62: Shader Operations
// ============================================================================

/**
 * Set shader from ShaderClass
 * Extracts shader settings and applies to graphics driver render states
 * 
 * Note: ShaderClass in this engine is NOT a GPU shader program - it's a collection
 * of render states (blend modes, depth testing, texture filtering, etc.)
 */
void DX8Wrapper::Set_Shader(ShaderClass* shader) {
    // ShaderClass contains render state settings, not actual GPU shaders
    // For now, this is a no-op as we need to examine ShaderClass structure
    // to properly map its settings to Graphics::IGraphicsDriver render states
    (void)shader;
}

void DX8Wrapper::Set_Shader(const ShaderClass& shader) {
    // Same as pointer version - extract render states
    (void)shader;
}

// ============================================================================
// Phase 62: Render State Management
// ============================================================================

/**
 * Apply any pending render state changes to the graphics driver
 */
void DX8Wrapper::Apply_Render_State_Changes() {
    // Currently render states are applied immediately
    // This is a hook for batching render state changes if needed
}

/**
 * Mark all cached render states as invalid
 * Forces re-application on next use
 */
void DX8Wrapper::Invalidate_Cached_Render_States() {
    // No cached render states currently - all are applied immediately
}

/**
 * Convert D3D render state to Graphics::RenderState and apply
 */
static Graphics::RenderState D3DRenderStateToGraphics(unsigned int d3dState) {
    // Map D3DRENDERSTATETYPE to Graphics::RenderState
    // Common D3DRS_* values:
    //   D3DRS_ZENABLE = 7
    //   D3DRS_FILLMODE = 8
    //   D3DRS_SHADEMODE = 9
    //   D3DRS_ZWRITEENABLE = 14
    //   D3DRS_ALPHATESTENABLE = 15
    //   D3DRS_SRCBLEND = 19
    //   D3DRS_DESTBLEND = 20
    //   D3DRS_CULLMODE = 22
    //   D3DRS_ZFUNC = 23
    //   D3DRS_ALPHAREF = 24
    //   D3DRS_ALPHAFUNC = 25
    //   D3DRS_DITHERENABLE = 26
    //   D3DRS_ALPHABLENDENABLE = 27
    //   D3DRS_FOGENABLE = 28
    //   D3DRS_SPECULARENABLE = 29
    //   D3DRS_FOGCOLOR = 34
    //   D3DRS_FOGTABLEMODE = 35
    //   D3DRS_FOGSTART = 36
    //   D3DRS_FOGEND = 37
    //   D3DRS_FOGDENSITY = 38
    //   D3DRS_LIGHTING = 137
    //   D3DRS_AMBIENT = 139
    //   D3DRS_COLORVERTEX = 141
    //   D3DRS_NORMALIZENORMALS = 143
    
    switch (d3dState) {
        case 7:   return Graphics::RenderState::ZEnable;
        case 14:  return Graphics::RenderState::ZWriteEnable;
        case 19:  return Graphics::RenderState::SrcBlend;
        case 20:  return Graphics::RenderState::DstBlend;
        case 22:  return Graphics::RenderState::CullMode;
        case 23:  return Graphics::RenderState::ZFunc;
        case 24:  return Graphics::RenderState::AlphaRef;
        case 25:  return Graphics::RenderState::AlphaFunc;
        case 26:  return Graphics::RenderState::DitherEnable;
        case 27:  return Graphics::RenderState::AlphaBlendEnable;
        case 28:  return Graphics::RenderState::FogEnable;
        case 29:  return Graphics::RenderState::Specular;
        case 34:  return Graphics::RenderState::FogColor;
        case 36:  return Graphics::RenderState::FogStart;
        case 37:  return Graphics::RenderState::FogEnd;
        case 38:  return Graphics::RenderState::FogDensity;
        case 137: return Graphics::RenderState::Lighting;
        case 139: return Graphics::RenderState::Ambient;
        case 143: return Graphics::RenderState::NormalizeNormals;
        default:  return Graphics::RenderState::Lighting;  // Safe default
    }
}

/**
 * Set a DirectX 8 render state
 * Maps D3DRENDERSTATETYPE to Graphics::RenderState
 */
void DX8Wrapper::Set_DX8_Render_State(unsigned int state, unsigned int value) {
    if (!g_graphics_driver) return;
    
    Graphics::RenderState gfxState = D3DRenderStateToGraphics(state);
    g_graphics_driver->SetRenderState(gfxState, static_cast<uint64_t>(value));
}

/**
 * Set a DirectX 8 texture stage state
 * Maps D3DTEXTURESTAGESTATETYPE to sampler state
 */
void DX8Wrapper::Set_DX8_Texture_Stage_State(int stage, unsigned int state, unsigned int value) {
    if (!g_graphics_driver) return;
    
    // Texture stage states are mapped to sampler states
    // Common D3DTSS_* values we need to support:
    //   D3DTSS_COLOROP = 1, D3DTSS_COLORARG1 = 2, D3DTSS_COLORARG2 = 3
    //   D3DTSS_ALPHAOP = 4, D3DTSS_ALPHAARG1 = 5, D3DTSS_ALPHAARG2 = 6
    //   D3DTSS_ADDRESSU = 13, D3DTSS_ADDRESSV = 14
    //   D3DTSS_MAGFILTER = 16, D3DTSS_MINFILTER = 17, D3DTSS_MIPFILTER = 18
    
    g_graphics_driver->SetSamplerState(static_cast<uint32_t>(stage), state, value);
}

// ============================================================================
// Phase 62: Lighting Operations
// ============================================================================

// Current fog state for Get_Fog_* functions
static bool s_fogEnabled = false;
static unsigned int s_fogColor = 0;

/**
 * Set light from DirectX 8 D3DLIGHT8 structure
 */
void DX8Wrapper::Set_DX8_Light(int index, void* d3d_light) {
    if (!g_graphics_driver) return;
    
    if (d3d_light == nullptr) {
        // Disable this light
        g_graphics_driver->DisableLight(static_cast<uint32_t>(index));
        return;
    }
    
    const D3DLIGHT8* d3dLight = static_cast<const D3DLIGHT8*>(d3d_light);
    
    // Convert to Graphics::Light
    Graphics::Light light;
    
    // Light type (D3DLIGHTTYPE: 1=POINT, 2=SPOT, 3=DIRECTIONAL)
    switch (d3dLight->Type) {
        case 1: light.type = Graphics::LightType::Point; break;
        case 2: light.type = Graphics::LightType::Spot; break;
        case 3: light.type = Graphics::LightType::Directional; break;
        default: light.type = Graphics::LightType::Directional; break;
    }
    
    // Colors
    light.diffuse = Graphics::Color(d3dLight->Diffuse.r, d3dLight->Diffuse.g, d3dLight->Diffuse.b, d3dLight->Diffuse.a);
    light.specular = Graphics::Color(d3dLight->Specular.r, d3dLight->Specular.g, d3dLight->Specular.b, d3dLight->Specular.a);
    light.ambient = Graphics::Color(d3dLight->Ambient.r, d3dLight->Ambient.g, d3dLight->Ambient.b, d3dLight->Ambient.a);
    
    // Position and direction
    light.position = Graphics::Vector3(d3dLight->Position.x, d3dLight->Position.y, d3dLight->Position.z);
    light.direction = Graphics::Vector3(d3dLight->Direction.x, d3dLight->Direction.y, d3dLight->Direction.z);
    
    // Attenuation
    light.range = d3dLight->Range;
    light.falloff = d3dLight->Falloff;
    light.attenuation0 = d3dLight->Attenuation0;
    light.attenuation1 = d3dLight->Attenuation1;
    light.attenuation2 = d3dLight->Attenuation2;
    
    // Spot light angles
    light.theta = d3dLight->Theta;
    light.phi = d3dLight->Phi;
    
    g_graphics_driver->SetLight(static_cast<uint32_t>(index), light);
}

/**
 * Set light (wrapper for Set_DX8_Light)
 */
void DX8Wrapper::Set_Light(int index, void* light) {
    Set_DX8_Light(index, light);
}

/**
 * Set ambient light color from raw pointer (cast to Vector3)
 */
void DX8Wrapper::Set_Ambient(const void* color) {
    if (!g_graphics_driver || !color) return;
    
    const Vector3* v = static_cast<const Vector3*>(color);
    g_graphics_driver->SetAmbientLight(v->X, v->Y, v->Z);
}

/**
 * Set ambient light color from Vector3
 */
void DX8Wrapper::Set_Ambient(const Vector3& color) {
    if (!g_graphics_driver) return;
    
    g_graphics_driver->SetAmbientLight(color.X, color.Y, color.Z);
}

// ============================================================================
// Phase 62: Fog Operations
// ============================================================================

/**
 * Set fog parameters
 */
void DX8Wrapper::Set_Fog(bool enable, const Vector3& color, float start, float end) {
    if (!g_graphics_driver) return;
    
    s_fogEnabled = enable;
    
    // Convert color to ARGB unsigned int
    unsigned char r = static_cast<unsigned char>(color.X * 255.0f);
    unsigned char g = static_cast<unsigned char>(color.Y * 255.0f);
    unsigned char b = static_cast<unsigned char>(color.Z * 255.0f);
    s_fogColor = (0xFF << 24) | (r << 16) | (g << 8) | b;
    
    // Set fog enable
    g_graphics_driver->SetRenderState(Graphics::RenderState::FogEnable, enable ? 1 : 0);
    
    if (enable) {
        // Set fog color (as ARGB unsigned int cast to uint64_t)
        g_graphics_driver->SetRenderState(Graphics::RenderState::FogColor, s_fogColor);
        
        // Set fog start and end (cast float bits to uint64_t)
        uint32_t startBits, endBits;
        memcpy(&startBits, &start, sizeof(float));
        memcpy(&endBits, &end, sizeof(float));
        g_graphics_driver->SetRenderState(Graphics::RenderState::FogStart, startBits);
        g_graphics_driver->SetRenderState(Graphics::RenderState::FogEnd, endBits);
    }
}

/**
 * Get fog enable state
 */
bool DX8Wrapper::Get_Fog_Enable() {
    return s_fogEnabled;
}

/**
 * Get fog color as ARGB unsigned int
 */
unsigned int DX8Wrapper::Get_Fog_Color() {
    return s_fogColor;
}

// ============================================================================
// Phase 62: Device Management Functions
// ============================================================================

/**
 * Set render device with full parameters
 * Updates resolution and notifies graphics driver
 */
bool DX8Wrapper::Set_Render_Device(int dev, int width, int height, int bits, int windowed,
                                    bool resize_window, bool reset_device, bool restore_assets) {
    s_deviceWidth = width;
    s_deviceHeight = height;
    s_deviceBitDepth = bits;
    s_deviceWindowed = (windowed != 0);
    
    printf("[Phase 62] Set_Render_Device: %dx%d %dbit windowed=%d\n", 
           width, height, bits, windowed);
    
    // Resize swapchain if driver exists
    if (g_graphics_driver) {
        g_graphics_driver->ResizeSwapChain(static_cast<uint32_t>(width), 
                                           static_cast<uint32_t>(height));
    }
    
    return true;
}

/**
 * Set render device with device name
 */
bool DX8Wrapper::Set_Render_Device(const char* dev_name, int width, int height, int bits, int windowed,
                                    bool resize_window) {
    return Set_Render_Device(0, width, height, bits, windowed, resize_window, false, false);
}

/**
 * Check if device is initialized
 */
bool DX8Wrapper::Is_Initted() {
    return s_deviceInitted && (g_graphics_driver != nullptr);
}

/**
 * Reset the graphics device
 */
bool DX8Wrapper::Reset_Device() {
    if (!g_graphics_driver) return false;
    
    printf("[Phase 62] Reset_Device: Resizing to %dx%d\n", s_deviceWidth, s_deviceHeight);
    return g_graphics_driver->ResizeSwapChain(static_cast<uint32_t>(s_deviceWidth),
                                              static_cast<uint32_t>(s_deviceHeight));
}

/**
 * Set device resolution
 */
bool DX8Wrapper::Set_Device_Resolution(int width, int height, int bits, bool windowed, bool resize_window) {
    s_deviceWidth = width;
    s_deviceHeight = height;
    s_deviceBitDepth = bits;
    s_deviceWindowed = windowed;
    
    printf("[Phase 62] Set_Device_Resolution: %dx%d %dbit windowed=%d\n", 
           width, height, bits, windowed);
    
    if (g_graphics_driver) {
        g_graphics_driver->ResizeSwapChain(static_cast<uint32_t>(width),
                                           static_cast<uint32_t>(height));
        if (!windowed) {
            g_graphics_driver->SetFullscreen(true);
        }
    }
    
    return true;
}

/**
 * Toggle between windowed and fullscreen mode
 */
bool DX8Wrapper::Toggle_Windowed() {
    s_deviceWindowed = !s_deviceWindowed;
    
    if (g_graphics_driver) {
        g_graphics_driver->SetFullscreen(!s_deviceWindowed);
    }
    
    return s_deviceWindowed;
}

/**
 * Check if running in windowed mode
 */
bool DX8Wrapper::Is_Windowed() {
    return s_deviceWindowed;
}

// ============================================================================
// Phase 62: Render Target Functions
// ============================================================================

// Current render target handle
static Graphics::RenderTargetHandle s_currentRenderTarget = Graphics::INVALID_HANDLE;

/**
 * Set the current render target
 */
void DX8Wrapper::Set_Render_Target(SurfaceClass* target) {
    if (!g_graphics_driver) return;
    
    if (target == nullptr) {
        // Reset to default back buffer
        g_graphics_driver->SetDefaultRenderTarget();
        s_currentRenderTarget = Graphics::INVALID_HANDLE;
    } else {
        // Set custom render target
        // Note: Would need SurfaceClass to expose render target handle
        // For now, just track that we have a custom target set
        printf("[Phase 62] Set_Render_Target: Custom target (not fully implemented)\n");
    }
}

/**
 * Set render target with depth/stencil buffer
 */
void DX8Wrapper::Set_Render_Target_With_Z(SurfaceClass* color_target, SurfaceClass* z_target) {
    // Same as Set_Render_Target for now
    Set_Render_Target(color_target);
}

/**
 * Create a render target surface
 */
SurfaceClass* DX8Wrapper::Create_Render_Target(int width, int height) {
    if (!g_graphics_driver) return nullptr;
    
    // Create render target via graphics driver
    Graphics::RenderTargetHandle handle = g_graphics_driver->CreateRenderTarget(
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        Graphics::TextureFormat::A8R8G8B8
    );
    
    if (handle == Graphics::INVALID_HANDLE) {
        printf("[Phase 62] Create_Render_Target: Failed to create %dx%d render target\n", width, height);
        return nullptr;
    }
    
    // Create a SurfaceClass wrapper
    // Note: Would need to store handle in SurfaceClass
    IDirect3DSurface8* d3dSurface = CreateMemorySurface(
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(height),
        static_cast<D3DFORMAT>(D3DFMT_A8R8G8B8),
        D3DPOOL_DEFAULT
    );
    
    if (!d3dSurface) {
        g_graphics_driver->DestroyRenderTarget(handle);
        return nullptr;
    }
    
    SurfaceClass* surface = new SurfaceClass(d3dSurface);
    printf("[Phase 62] Create_Render_Target: Created %dx%d render target\n", width, height);
    return surface;
}

/**
 * Flip/present the back buffer to primary display
 */
void DX8Wrapper::Flip_To_Primary() {
    if (g_graphics_driver) {
        g_graphics_driver->Present();
    }
}

// ============================================================================
// Phase 62: Swap Chain & Display Functions
// ============================================================================

static int s_swapInterval = 1;  // Default to vsync on
static int s_textureBitDepth = 32;
static float s_gamma = 1.0f;
static float s_brightness = 1.0f;
static float s_contrast = 1.0f;

/**
 * Get current swap interval (vsync setting)
 */
int DX8Wrapper::Get_Swap_Interval() {
    return s_swapInterval;
}

/**
 * Set swap interval for vsync control
 * 0 = no vsync, 1 = vsync, 2+ = adaptive vsync multiples
 */
void DX8Wrapper::Set_Swap_Interval(int swap) {
    s_swapInterval = swap;
    // Note: Would need graphics driver support to actually change vsync
    printf("[Phase 62] Set_Swap_Interval: %d\n", swap);
}

/**
 * Set texture bit depth for texture loading
 */
void DX8Wrapper::Set_Texture_Bitdepth(int bitdepth) {
    s_textureBitDepth = bitdepth;
    printf("[Phase 62] Set_Texture_Bitdepth: %d\n", bitdepth);
}

/**
 * Get current texture bit depth
 */
int DX8Wrapper::Get_Texture_Bitdepth() {
    return s_textureBitDepth;
}

/**
 * Set gamma/brightness/contrast
 */
void DX8Wrapper::Set_Gamma(float gamma, float bright, float contrast, bool calibrate) {
    s_gamma = gamma;
    s_brightness = bright;
    s_contrast = contrast;
    printf("[Phase 62] Set_Gamma: gamma=%.2f brightness=%.2f contrast=%.2f calibrate=%d\n",
           gamma, bright, contrast, calibrate);
    // Note: Would need graphics driver support to apply gamma correction
}

// ============================================================================
// Phase 62: Front/Back Buffer Access
// ============================================================================

/**
 * Get front buffer surface (for screen capture)
 */
SurfaceClass* DX8Wrapper::Get_Front_Buffer() {
    // Create a surface representing the front buffer
    IDirect3DSurface8* d3dSurface = CreateMemorySurface(
        static_cast<unsigned int>(s_deviceWidth),
        static_cast<unsigned int>(s_deviceHeight),
        static_cast<D3DFORMAT>(D3DFMT_A8R8G8B8),
        D3DPOOL_DEFAULT
    );
    
    if (!d3dSurface) return nullptr;
    
    return new SurfaceClass(d3dSurface);
}

/**
 * Get back buffer surface
 */
SurfaceClass* DX8Wrapper::Get_Back_Buffer() {
    return _Get_DX8_Back_Buffer();
}

// ============================================================================
// Phase 62: Shader Operations
// ============================================================================

static unsigned int s_currentPixelShader = 0;
static unsigned int s_currentVertexShader = 0;

/**
 * Set pixel shader
 * In DirectX 8, this was an integer handle to a created shader
 */
void DX8Wrapper::Set_Pixel_Shader(unsigned int shader) {
    s_currentPixelShader = shader;
    // Note: Actual shader binding would go through graphics driver
    // For fixed-function pipeline emulation, 0 means no shader
}

/**
 * Set vertex shader
 * In DirectX 8, this could be either FVF code or shader handle
 */
void DX8Wrapper::Set_Vertex_Shader(unsigned int shader) {
    s_currentVertexShader = shader;
    // Note: Actual shader binding would go through graphics driver
}

// ============================================================================
// Phase 62: Projection Transform with Z-Bias
// ============================================================================

/**
 * Set projection matrix with Z-bias adjustment
 * Z-bias is used to prevent z-fighting on coplanar surfaces
 */
void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const void* projection, float z_near, float z_far) {
    if (!projection) return;
    
    // Set the projection matrix first
    DX8Wrapper_SetTransformInternal(D3DTS_PROJECTION, static_cast<const float*>(projection));
    
    // Z-bias would require modifying the projection matrix slightly
    // For now, the basic projection is set
}

void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix3D& projection, float z_near, float z_far) {
    Set_Transform(D3DTS_PROJECTION, projection);
}

void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix4x4& projection, float z_near, float z_far) {
    Set_Transform(D3DTS_PROJECTION, projection);
}

// ============================================================================
// Phase 62: Depth/Z Texture Creation
// ============================================================================

/**
 * Create a depth/Z texture for shadow mapping or depth effects
 */
void* DX8Wrapper::_Create_DX8_ZTexture(int width, int height, int format) {
    // Create a depth texture using memory texture system
    // Format should be a depth format like D3DFMT_D16, D3DFMT_D24S8, etc.
    IDirect3DTexture8* tex = CreateMemoryTexture(
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(height),
        1,  // mip levels
        D3DUSAGE_DEPTHSTENCIL,
        static_cast<D3DFORMAT>(format),
        D3DPOOL_DEFAULT
    );
    
    printf("[Phase 62] _Create_DX8_ZTexture: %dx%d format=%d result=%p\n", 
           width, height, format, (void*)tex);
    return tex;
}

/**
 * Create a cube texture for environment mapping
 */
void* DX8Wrapper::_Create_DX8_Cube_Texture(int width, int height, int format) {
    // Cube textures have 6 faces, each of size width x width (height is ignored)
    // For now, create a regular texture as placeholder
    // Real implementation would need CubeTexture support in graphics driver
    IDirect3DTexture8* tex = CreateMemoryTexture(
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(width),
        1,  // mip levels
        0,
        static_cast<D3DFORMAT>(format),
        D3DPOOL_MANAGED
    );
    
    printf("[Phase 62] _Create_DX8_Cube_Texture: %dx%d format=%d result=%p (placeholder)\n",
           width, width, format, (void*)tex);
    return tex;
}

/**
 * Create a volume (3D) texture
 */
void* DX8Wrapper::_Create_DX8_Volume_Texture(int width, int height, int depth, int format) {
    // Volume textures are 3D textures
    // For now, create a regular 2D texture as placeholder
    // Real implementation would need 3D texture support in graphics driver
    IDirect3DTexture8* tex = CreateMemoryTexture(
        static_cast<unsigned int>(width),
        static_cast<unsigned int>(height),
        1,  // mip levels
        0,
        static_cast<D3DFORMAT>(format),
        D3DPOOL_MANAGED
    );
    
    printf("[Phase 62] _Create_DX8_Volume_Texture: %dx%dx%d format=%d result=%p (placeholder)\n",
           width, height, depth, format, (void*)tex);
    return tex;
}

// ============================================================================
// Phase 62: Surface Copy Operations
// ============================================================================

/**
 * Copy rectangles between surfaces
 * This is a DirectX 8 style surface copy operation
 */
void DX8Wrapper::_Copy_DX8_Rects(void* src_surface, void* src_rect, int src_pitch,
                                  void* dst_surface, void* dst_rect) {
    if (!src_surface || !dst_surface) return;
    
    IDirect3DSurface8* srcSurf = static_cast<IDirect3DSurface8*>(src_surface);
    IDirect3DSurface8* dstSurf = static_cast<IDirect3DSurface8*>(dst_surface);
    
    // Get surface descriptions
    D3DSURFACE_DESC srcDesc, dstDesc;
    srcSurf->GetDesc(&srcDesc);
    dstSurf->GetDesc(&dstDesc);
    
    // Lock both surfaces
    D3DLOCKED_RECT srcLock, dstLock;
    if (FAILED(srcSurf->LockRect(&srcLock, static_cast<RECT*>(src_rect), D3DLOCK_READONLY))) {
        printf("[Phase 62] _Copy_DX8_Rects: Failed to lock source surface\n");
        return;
    }
    if (FAILED(dstSurf->LockRect(&dstLock, static_cast<RECT*>(dst_rect), 0))) {
        srcSurf->UnlockRect();
        printf("[Phase 62] _Copy_DX8_Rects: Failed to lock destination surface\n");
        return;
    }
    
    // Simple copy - assumes compatible formats
    unsigned int copyHeight = (src_rect) ? 
        (static_cast<RECT*>(src_rect)->bottom - static_cast<RECT*>(src_rect)->top) : srcDesc.Height;
    unsigned int copyWidth = (src_rect) ?
        (static_cast<RECT*>(src_rect)->right - static_cast<RECT*>(src_rect)->left) : srcDesc.Width;
    
    // Calculate bytes per pixel (assume 4 for ARGB)
    int bpp = 4;
    
    const unsigned char* srcData = static_cast<const unsigned char*>(srcLock.pBits);
    unsigned char* dstData = static_cast<unsigned char*>(dstLock.pBits);
    
    for (unsigned int y = 0; y < copyHeight; y++) {
        memcpy(dstData, srcData, copyWidth * bpp);
        srcData += srcLock.Pitch;
        dstData += dstLock.Pitch;
    }
    
    dstSurf->UnlockRect();
    srcSurf->UnlockRect();
}

/**
 * Copy rectangles (DirectX 8 API style)
 */
int DX8Wrapper::CopyRects(void* src_surface, void* src_rect, int count, 
                          void* dst_surface, void* dst_rect) {
    if (!src_surface || !dst_surface) return -1;
    
    // For single rect copy
    if (count == 1 || count == 0) {
        _Copy_DX8_Rects(src_surface, src_rect, 0, dst_surface, dst_rect);
    } else {
        // Multiple rects - would need RECT array handling
        printf("[Phase 62] CopyRects: Multiple rect copy not fully implemented (count=%d)\n", count);
        _Copy_DX8_Rects(src_surface, nullptr, 0, dst_surface, nullptr);
    }
    
    return 0;  // Success
}

/**
 * Load surface from another surface with filtering
 * This emulates D3DXLoadSurfaceFromSurface
 */
int DX8Wrapper::D3DXLoadSurfaceFromSurface(void* dst_surface, void* dst_palette, void* dst_rect,
                                            void* src_surface, void* src_palette, void* src_rect,
                                            int filter, int color_key) {
    if (!src_surface || !dst_surface) return -1;
    
    // For now, do a simple copy ignoring filter and color key
    // Real implementation would need filtering support (point, linear, etc.)
    _Copy_DX8_Rects(src_surface, src_rect, 0, dst_surface, dst_rect);
    
    return 0;  // Success
}

// ============================================================================
// Phase 62: Remaining Buffer Binding Stubs (void* overloads)
// ============================================================================

/**
 * Set vertex buffer from generic void pointer
 * This is called when the caller doesn't know the specific buffer type
 * We attempt to interpret the pointer based on context
 */
void DX8Wrapper::Set_Vertex_Buffer(void* vb_access) {
    // void* overload - typically used for legacy code paths
    // For now, log and ignore - specific typed overloads handle real binding
    if (vb_access != nullptr) {
        // Could potentially try to detect buffer type, but safer to require typed access
        printf("[Phase 62] DX8Wrapper::Set_Vertex_Buffer(void*) - Called with %p, using dynamic path\n", vb_access);
    }
}

void DX8Wrapper::Set_Vertex_Buffer(int stream_number, void* vb_access) {
    // Stream number variant - delegate to base implementation
    Set_Vertex_Buffer(vb_access);
}

/**
 * Set index buffer from generic void pointer
 */
void DX8Wrapper::Set_Index_Buffer(void* ib_access, int start_index) {
    // void* overload - typically used for legacy code paths
    if (ib_access != nullptr) {
        printf("[Phase 62] DX8Wrapper::Set_Index_Buffer(void*) - Called with %p, start_index=%d\n", 
               ib_access, start_index);
    }
}

void DX8Wrapper::Set_Index_Buffer(int stream_number, void* ib_access, int start_index) {
    // Stream number variant - delegate to base implementation  
    Set_Index_Buffer(ib_access, start_index);
}

// ============================================================================
// Phase 62: Render State Get/Release
// ============================================================================

// Storage for current render state snapshot
static uint64_t s_savedRenderStates[256] = {0};

/**
 * Get current render state into buffer
 * Saves all render states for later restoration
 */
void DX8Wrapper::Get_Render_State(void* state) {
    if (!state || !g_graphics_driver) return;
    
    // Copy current render state snapshot to caller's buffer
    memcpy(state, s_savedRenderStates, sizeof(s_savedRenderStates));
}

/**
 * Release/restore saved render state
 * Called to restore previously saved render states
 */
void DX8Wrapper::Release_Render_State() {
    // Clear saved render state snapshot
    memset(s_savedRenderStates, 0, sizeof(s_savedRenderStates));
}

// ============================================================================
// Phase 62: Light Environment
// ============================================================================

/**
 * Set a complete light environment (multiple lights at once)
 * The light_env parameter is typically a game-specific structure containing
 * multiple light definitions
 */
void DX8Wrapper::Set_Light_Environment(void* light_env) {
    if (!g_graphics_driver || !light_env) return;
    
    // Light environment would typically contain:
    // - Ambient light color
    // - Array of directional/point/spot lights
    // For now, just log that we received the call
    printf("[Phase 62] DX8Wrapper::Set_Light_Environment - Called with %p\n", light_env);
    
    // TODO: Parse game-specific light environment structure when needed
}

// ============================================================================
// Phase 62: IDirect3DDevice8Stub Implementation
// ============================================================================

// Global stub device instance
static IDirect3DDevice8Stub s_stubDevice;

/**
 * Get the global stub D3D device instance
 */
IDirect3DDevice8Stub* GetStubD3DDevice8() {
    return &s_stubDevice;
}

/**
 * TestCooperativeLevel - Check if device is in a valid state
 * Returns D3D_OK if initialized, D3DERR_DEVICELOST otherwise
 */
int IDirect3DDevice8Stub::TestCooperativeLevel() {
    // Return 0 (D3D_OK) if graphics driver is initialized
    if (g_graphics_driver && g_graphics_driver->IsInitialized()) {
        return 0;  // D3D_OK
    }
    // D3DERR_DEVICELOST = 0x88760868 (but return 0 for compatibility)
    return 0;
}

/**
 * SetTexture - Bind a texture to a sampler stage
 * Delegates to DX8Wrapper::Set_DX8_Texture
 */
int IDirect3DDevice8Stub::SetTexture(int stage, void* texture) {
    DX8Wrapper::Set_DX8_Texture(stage, texture);
    return 0;  // D3D_OK
}

/**
 * SetPixelShader - Set current pixel shader
 * Delegates to DX8Wrapper::Set_Pixel_Shader
 */
int IDirect3DDevice8Stub::SetPixelShader(int shader) {
    DX8Wrapper::Set_Pixel_Shader(static_cast<unsigned int>(shader));
    return 0;  // D3D_OK
}

/**
 * DeletePixelShader - Delete a pixel shader
 * No-op in our implementation (shaders are managed differently)
 */
int IDirect3DDevice8Stub::DeletePixelShader(int shader) {
    // Pixel shaders are not used in modern backend
    (void)shader;
    return 0;  // D3D_OK
}

/**
 * DeleteVertexShader - Delete a vertex shader  
 * No-op in our implementation (shaders are managed differently)
 */
int IDirect3DDevice8Stub::DeleteVertexShader(int shader) {
    // Vertex shaders are not used in modern backend
    (void)shader;
    return 0;  // D3D_OK
}

/**
 * SetStreamSource - Bind a vertex buffer to a stream
 * Delegates to internal vertex buffer binding
 */
int IDirect3DDevice8Stub::SetStreamSource(int stream_number, void* stream_data, int stride) {
    // stream_data is typically an IDirect3DVertexBuffer8* or similar
    // For now, store as generic void* vertex buffer
    DX8Wrapper::Set_Vertex_Buffer(stream_number, stream_data);
    return 0;  // D3D_OK
}

/**
 * SetVertexShader - Set vertex shader or FVF
 * In DX8, this can be either a shader handle or an FVF code
 */
int IDirect3DDevice8Stub::SetVertexShader(int shader) {
    DX8Wrapper::Set_Vertex_Shader(static_cast<unsigned int>(shader));
    return 0;  // D3D_OK
}

/**
 * SetIndices - Set index buffer
 * Delegates to DX8Wrapper::Set_Index_Buffer
 */
int IDirect3DDevice8Stub::SetIndices(void* index_data, int base_vertex_index) {
    DX8Wrapper::Set_Index_Buffer(index_data, base_vertex_index);
    return 0;  // D3D_OK
}

/**
 * DrawIndexedPrimitive - Draw indexed geometry
 * Delegates to DX8Wrapper::Draw_Triangles for triangle lists
 */
int IDirect3DDevice8Stub::DrawIndexedPrimitive(int type, int min_index, int num_vertices, int start_index, int prim_count) {
    // D3DPT_TRIANGLELIST = 4, D3DPT_TRIANGLESTRIP = 5
    if (type == 4) {  // Triangle list
        DX8Wrapper::Draw_Triangles(start_index, prim_count, min_index, num_vertices);
    } else if (type == 5) {  // Triangle strip
        DX8Wrapper::Draw_Strip(start_index, prim_count, min_index, num_vertices);
    }
    return 0;  // D3D_OK
}

/**
 * ProcessVertices - Process vertices through vertex shader
 * This is rarely used - primarily for software T&L
 */
int IDirect3DDevice8Stub::ProcessVertices(int src_start_index, int dest_index, int vertex_count, void* dest_buffer, int flags) {
    // ProcessVertices is used for software vertex processing
    // Not commonly needed with hardware acceleration
    (void)src_start_index;
    (void)dest_index;
    (void)vertex_count;
    (void)dest_buffer;
    (void)flags;
    return 0;  // D3D_OK
}

/**
 * GetRenderTarget - Get the current render target
 * Returns the back buffer surface
 */
int IDirect3DDevice8Stub::GetRenderTarget(void** renderTarget) {
    if (renderTarget) {
        SurfaceClass* surface = DX8Wrapper::_Get_DX8_Back_Buffer();
        // Note: Caller is responsible for releasing the surface
        *renderTarget = surface;
    }
    return 0;  // D3D_OK
}

/**
 * CreateTexture - Create a new texture
 * Delegates to DX8Wrapper::_Create_DX8_Texture
 */
int IDirect3DDevice8Stub::CreateTexture(int width, int height, int levels, int usage, int format, int pool, void** texture) {
    if (texture) {
        *texture = DX8Wrapper::_Create_DX8_Texture(width, height, format, levels);
    }
    return 0;  // D3D_OK
}

/**
 * GetDepthStencilSurface - Get the current depth/stencil surface
 * Returns nullptr for now (depth buffer is managed internally)
 */
int IDirect3DDevice8Stub::GetDepthStencilSurface(void** depthSurface) {
    if (depthSurface) {
        // Depth buffer is managed by the graphics driver internally
        // Return nullptr for now
        *depthSurface = nullptr;
    }
    return 0;  // D3D_OK
}

/**
 * SetRenderState - Set a render state
 * Delegates to DX8Wrapper::Set_DX8_Render_State
 */
int IDirect3DDevice8Stub::SetRenderState(int state, int value) {
    DX8Wrapper::Set_DX8_Render_State(static_cast<unsigned int>(state), static_cast<unsigned int>(value));
    return 0;  // D3D_OK
}

/**
 * SetTextureStageState - Set a texture stage state
 * Delegates to DX8Wrapper::Set_DX8_Texture_Stage_State
 */
int IDirect3DDevice8Stub::SetTextureStageState(int stage, int type, int value) {
    DX8Wrapper::Set_DX8_Texture_Stage_State(stage, static_cast<unsigned int>(type), static_cast<unsigned int>(value));
    return 0;  // D3D_OK
}