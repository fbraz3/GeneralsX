/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file dx8buffer_compat.h
 * @brief DirectX 8 Buffer Type Definitions for Cross-Platform Compatibility
 * 
 * Phase 42: Provides DirectX 8 buffer-related type definitions
 * (LPDIRECT3DDEVICE8, LPDIRECT3DVERTEXBUFFER8, etc.) 
 * needed for game code that was originally written for DirectX.
 * 
 * These are essentially void pointers in the Vulkan/SDL2 port, 
 * as the actual buffer management is handled by the graphics driver abstraction layer.
 * 
 * Includes:
 * - Device pointer types (LPDIRECT3DDEVICE8)
 * - Buffer pointer types (vertex, index buffers)
 * - Texture pointer types
 * - Surface pointer types
 * - Swapchain pointer types
 */

#pragma once

#ifndef WW3D2_DX8_BUFFER_COMPAT_H
#define WW3D2_DX8_BUFFER_COMPAT_H

#include "d3d8_vulkan_graphics_compat.h"
#include "d3dx8_vulkan_fvf_compat.h"  // Phase 42: FVF constants (DX8_FVF_*)

// Include Graphics-level buffer compatibility layer
// This provides DX8VertexBufferClass and DX8IndexBufferClass definitions
#include "../../Graphics/dx8buffer_compat.h"

#endif // WW3D2_DX8_BUFFER_COMPAT_H
