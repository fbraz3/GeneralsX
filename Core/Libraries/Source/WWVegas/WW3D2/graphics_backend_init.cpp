/**
 * Graphics Backend Global Instance
 * 
 * Singleton pattern for graphics backend selection.
 * Backend is chosen at initialization based on USE_DXVK CMake flag.
 */

#include "graphics_backend.h"
#include "graphics_backend_legacy.h"

// ============================================================================
// Global Graphics Backend Instance
// ============================================================================

/**
 * Global graphics backend instance.
 * 
 * Initialized at application startup with the appropriate backend:
 * - USE_DXVK=OFF (default): LegacyGraphicsBackend (Metal/OpenGL)
 * - USE_DXVK=ON: DVKGraphicsBackend (DXVK/Vulkan) - Phase 39+
 * 
 * All game code should access graphics through this pointer.
 */
IGraphicsBackend* g_graphicsBackend = nullptr;

// ============================================================================
// Backend Initialization
// ============================================================================

/**
 * Initialize the graphics backend.
 * Called from WinMain() or main() at application startup.
 * 
 * @return HRESULT (S_OK on success, error code on failure)
 */
HRESULT InitializeGraphicsBackend() {
    if (g_graphicsBackend != nullptr) {
        printf("WARNING: Graphics backend already initialized!\n");
        return 0; // S_OK (already done)
    }
    
    #ifdef USE_DXVK
        // DXVK/Vulkan backend (Phase 39+)
        printf("Graphics Backend: DXVK/Vulkan (USE_DXVK enabled)\n");
        
        // TODO: Implement DVKGraphicsBackend in Phase 39
        // g_graphicsBackend = new DVKGraphicsBackend();
        
        printf("ERROR: DXVK backend not yet implemented (Phase 39)\n");
        return -1; // E_FAIL
    #else
        // Legacy Metal/OpenGL backend (Phase 27-37)
        printf("Graphics Backend: Legacy Metal/OpenGL (USE_DXVK disabled)\n");
        
        g_graphicsBackend = new LegacyGraphicsBackend();
        
        if (g_graphicsBackend == nullptr) {
            printf("ERROR: Failed to create LegacyGraphicsBackend\n");
            return -1; // E_FAIL
        }
        
        HRESULT hr = g_graphicsBackend->Initialize();
        if (hr != 0) { // S_OK = 0
            printf("ERROR: Failed to initialize graphics backend (0x%08lx)\n", hr);
            delete g_graphicsBackend;
            g_graphicsBackend = nullptr;
            return hr;
        }
        
        printf("Graphics backend initialized successfully\n");
        printf("Backend: %s\n", g_graphicsBackend->GetBackendName());
        
        return 0; // S_OK
    #endif
}

/**
 * Shutdown the graphics backend.
 * Called from main() or WinMain() at application exit.
 */
HRESULT ShutdownGraphicsBackend() {
    if (g_graphicsBackend == nullptr) {
        return 0; // S_OK (not initialized)
    }
    
    printf("Graphics Backend: Shutting down...\n");
    
    HRESULT hr = g_graphicsBackend->Shutdown();
    
    delete g_graphicsBackend;
    g_graphicsBackend = nullptr;
    
    printf("Graphics backend shut down\n");
    
    return hr;
}
