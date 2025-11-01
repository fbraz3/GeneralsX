/**
 * Graphics Backend Global Instance
 * 
 * Singleton pattern for graphics backend selection.
 * Backend is chosen at initialization based on USE_DXVK CMake flag.
 */

#include "graphics_backend.h"
#include "graphics_backend_legacy.h"
#ifdef USE_DXVK
#include "graphics_backend_dxvk.h"
#endif

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
 * Create the graphics backend instance (without initializing).
 * Allows SetWindowHandle() to be called BEFORE Initialize().
 * 
 * @return HRESULT (S_OK on success, error code on failure)
 */
HRESULT CreateGraphicsBackend() {
    if (g_graphicsBackend != nullptr) {
        printf("WARNING: Graphics backend already created!\n");
        return 0; // S_OK (already done)
    }
    
    #ifdef USE_DXVK
        printf("Graphics Backend: Creating DXVK/Vulkan backend (USE_DXVK enabled)\n");
        fflush(stdout);
        
        g_graphicsBackend = new DXVKGraphicsBackend();
        
        printf("CreateGraphicsBackend - DXVKGraphicsBackend created: %p\n", (void*)g_graphicsBackend);
        fflush(stdout);
        
        if (g_graphicsBackend == nullptr) {
            printf("ERROR: Failed to create DXVKGraphicsBackend\n");
            fflush(stdout);
            return -1; // E_FAIL
        }
    #else
        printf("Graphics Backend: Creating Legacy Metal/OpenGL backend (USE_DXVK disabled)\n");
        fflush(stdout);
        
        g_graphicsBackend = new LegacyGraphicsBackend();
        
        printf("CreateGraphicsBackend - LegacyGraphicsBackend created: %p\n", (void*)g_graphicsBackend);
        fflush(stdout);
        
        if (g_graphicsBackend == nullptr) {
            printf("ERROR: Failed to create LegacyGraphicsBackend\n");
            fflush(stdout);
            return -1; // E_FAIL
        }
    #endif
    
    return 0; // S_OK
}

/**
 * Initialize the graphics backend (must be called AFTER CreateGraphicsBackend and SetWindowHandle).
 * 
 * @return HRESULT (S_OK on success, error code on failure)
 */
HRESULT InitializeGraphicsBackendNow() {
    if (g_graphicsBackend == nullptr) {
        printf("ERROR: Graphics backend not created! Call CreateGraphicsBackend first.\n");
        fflush(stdout);
        return -1; // E_FAIL
    }
    
    printf("InitializeGraphicsBackendNow - Calling backend->Initialize()...\n");
    fflush(stdout);
    
    HRESULT hr = g_graphicsBackend->Initialize();
    
    if (hr != 0) { // S_OK = 0
        printf("ERROR: Failed to initialize graphics backend (0x%08lx)\n", (unsigned long)hr);
        fflush(stdout);
        return hr;
    }
    
    printf("Graphics backend initialized successfully\n");
    printf("Backend: %s\n", g_graphicsBackend->GetBackendName());
    fflush(stdout);
    
    return 0; // S_OK
}

/**
 * LEGACY: Initialize the graphics backend (all-in-one).
 * Kept for compatibility. New code should use CreateGraphicsBackend + SetWindowHandle + InitializeGraphicsBackendNow.
 * 
 * @return HRESULT (S_OK on success, error code on failure)
 */
HRESULT InitializeGraphicsBackend() {
    HRESULT hr = CreateGraphicsBackend();
    if (hr != 0) {
        return hr;
    }
    
    return InitializeGraphicsBackendNow();
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
