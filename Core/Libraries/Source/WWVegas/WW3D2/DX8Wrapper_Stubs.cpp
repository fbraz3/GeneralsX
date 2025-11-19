/*
 * DX8Wrapper_Stubs.cpp
 * 
 * Phase 39.4: Global stub definitions for DirectX 8 compatibility layer
 * Provides initialization for global objects referenced by game code
 */

#include "DX8Wrapper_Stubs.h"

// Global DX8Caps instance (Phase 39.4)
static DX8Caps g_dx8_caps;

// Global mesh renderer stub instance (Phase 39.4: Changed from pointer to static object)
static DX8MeshRenderer g_dx8_mesh_renderer;
DX8MeshRenderer& TheDX8MeshRenderer = g_dx8_mesh_renderer;

// Global Renderer Debugger stub (Phase 39.4)
class DX8RendererDebugger {
public:
    static void Add_Mesh(void*) {}  // No-op stub
};

// Initialization function (called early in game startup)
void InitializeDX8Stubs() {
    // No-op for static global object
}

// Cleanup function (called during game shutdown)
void ShutdownDX8Stubs() {
    // No-op for static global object
}

// Get current capabilities (returns pointer to static global instance)
// Note: This is called via DX8Wrapper::Get_Current_Caps()
// Must define as extern to allow the static method to access it
extern DX8Caps* Get_DX8_Caps_Ptr() {
    return &g_dx8_caps;
}

// Phase 39.4: Define global FVF type constant for legacy render2d.cpp
const int dynamic_fvf_type = 0;
