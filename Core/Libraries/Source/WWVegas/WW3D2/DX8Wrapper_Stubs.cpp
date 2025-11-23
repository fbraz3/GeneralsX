/*
 * DX8Wrapper_Stubs.cpp
 * 
 * Phase 39.4: Global stub definitions for DirectX 8 compatibility layer
 * Updated Phase 41 Week 3: Integrated Graphics::IGraphicsDriver factory
 * 
 * Provides initialization for global objects referenced by game code.
 * Bridge between legacy DX8Wrapper calls and modern Graphics::IGraphicsDriver abstraction.
 */

#include "DX8Wrapper_Stubs.h"
#include "../../Graphics/IGraphicsDriver.h"
#include "../../Graphics/GraphicsDriverFactory.h"

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
    
    printf("[Phase 41 Week 3] DX8Wrapper::Init - Creating graphics driver via factory\n");
    
    // Destroy existing driver if any
    if (g_graphics_driver != nullptr) {
        printf("[Phase 41 Week 3] DX8Wrapper::Init - Destroying existing driver\n");
        Graphics::GraphicsDriverFactory::DestroyDriver(g_graphics_driver);
        g_graphics_driver = nullptr;
    }
    
    // Create new driver via factory (uses environment variable + config file priority)
    // Window dimensions are set to default 1024x768 for initialization
    g_graphics_driver = Graphics::GraphicsDriverFactory::CreateDriver(
        Graphics::BackendType::Unknown,  // Use default selection priority
        hwnd,
        1024,  // width
        768,   // height
        false  // fullscreen
    );
    
    if (g_graphics_driver == nullptr) {
        printf("[Phase 41 Week 3 ERROR] DX8Wrapper::Init - Failed to create graphics driver\n");
        return false;
    }
    
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
    if (g_graphics_driver != nullptr) {
        if (!g_graphics_driver->BeginFrame()) {
            printf("[Phase 41 Week 3 ERROR] DX8Wrapper::Begin_Scene - BeginFrame failed\n");
        }
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
// Phase 43.1: Surface Creation (for texture system foundation)
// ============================================================================

/**
 * Create an off-screen plain surface for texture data
 * Phase 43.1: Provides cross-platform surface creation
 * 
 * Parameter format is expected to be a WW3DFormat enum value cast to int
 */
IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(int width, int height, int format)
{
    // Phase 43.1: This method creates a surface using the graphics backend
    // For now, we return nullptr as a placeholder
    // The actual implementation will be in the graphics backend layer
    printf("[Phase 43.1] DX8Wrapper::_Create_DX8_Surface(width=%d, height=%d, format=%d)\n", 
           width, height, format);
    
    // TODO: Replace with actual graphics backend surface creation
    // return g_graphics_driver->CreateSurface(width, height, (WW3DFormat)format);
    return nullptr;
}

/**
 * Load a surface from a file
 * Phase 43.1: Provides cross-platform surface loading from disk
 */
IDirect3DSurface8* DX8Wrapper::_Create_DX8_Surface(const char* filename)
{
    // Phase 43.1: This method loads a surface from a file
    printf("[Phase 43.1] DX8Wrapper::_Create_DX8_Surface(filename='%s')\n", filename);
    
    // TODO: Replace with actual file loading implementation
    // return TextureLoader::Load_Surface_Immediate(filename, WW3D_FORMAT_UNKNOWN, true);
    return nullptr;
}

