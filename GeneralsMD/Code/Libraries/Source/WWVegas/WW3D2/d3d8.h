#pragma once

#ifndef D3D8_H_INCLUDED
#define D3D8_H_INCLUDED

#ifndef _WIN32

// ============================================================================
// DirectX 8 Master Header
// 
// This provides all DirectX 8 type definitions needed for cross-platform
// compilation. It includes both graphics and audio compatibility layers.
// 
// Architecture:
// - d3d8_graphics_compat.h: Graphics types, enums, structures (Phase 50)
// - d3d8_audio_compat.h: Audio types, structures (Phase 51 - deferred)
// 
// Phase 50: Only graphics layer is active
// ============================================================================

#include "d3d8_graphics_compat.h"

// Include audio compat for future Phase 51 implementation
// Currently disabled - audio implementation deferred to Phase 51
// #include "d3d8_audio_compat.h"

#endif // !_WIN32

#endif // D3D8_H_INCLUDED

