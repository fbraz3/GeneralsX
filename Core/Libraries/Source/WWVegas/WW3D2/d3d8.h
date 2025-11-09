#pragma once

#ifndef D3D8_H_INCLUDED
#define D3D8_H_INCLUDED

#ifndef _WIN32

// ============================================================================
// DirectX 8 Master Header
// 
// This provides all DirectX 8 type definitions needed for cross-platform
// compilation via layered compatibility system:
//
// Layer 1 (Generic): d3d8_graphics_compat.h - Core types, enums, structures
//   - Contains: D3DFORMAT, D3DRENDERSTATETYPE, D3DCOLOR, etc.
//   - Backend-agnostic (used by all graphics backends)
//
// Layer 2 (Backend-Specific): d3d8_vulkan_graphics_compat.h - Vulkan implementation
//   - Contains: Vulkan-specific stubs, wrappers, dispatch functions
//   - Includes d3d8_graphics_compat.h and adds Vulkan layer on top
//   - Phase 50: Active (Vulkan-only architecture)
//
// Layer 3 (Audio): d3d8_audio_compat.h - Audio types, structures
//   - Phase 51+: Deferred implementation
// 
// Architecture Flow:
// d3d8.h (master) → d3d8_graphics_compat.h (generic) → d3d8_vulkan_graphics_compat.h (Phase 50 Vulkan)
// 
// ============================================================================

// Step 1: Include generic graphics types (backend-agnostic)
#include "d3d8_graphics_compat.h"

// Step 2: Include backend-specific implementation (Phase 50 = Vulkan)
#ifdef ENABLE_VULKAN
#include "d3d8_vulkan_graphics_compat.h"
#endif

// Step 3: Include audio compat for future Phase 51 implementation
// Currently disabled - audio implementation deferred to Phase 51
// #include "d3d8_audio_compat.h"

#endif // !_WIN32

#endif // D3D8_H_INCLUDED

