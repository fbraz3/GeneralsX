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
 * @file dx8fvf_vertex_formats.h
 * @brief DirectX 8 Vertex Format Structs for Cross-Platform Compatibility
 * 
 * Phase 42: Provides vertex format structure definitions needed by W3D rendering
 * components like W3DRoadBuffer, W3DBridgeBuffer, etc.
 * 
 * These structs define the memory layout for vertex data passed to graphics drivers.
 * Each struct corresponds to a specific DirectX 8 FVF (Flexible Vertex Format).
 */

#pragma once

#ifndef DX8FVF_VERTEX_FORMATS_H
#define DX8FVF_VERTEX_FORMATS_H

// ============================================================================
// Simple Vertex Formats
// ============================================================================

/**
 * @struct VertexFormatXYZ
 * Position only vertex format (3 floats)
 */
struct VertexFormatXYZ
{
	float x;
	float y;
	float z;
};

/**
 * @struct VertexFormatXYZN
 * Position + Normal vertex format (6 floats)
 */
struct VertexFormatXYZN
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
};

// ============================================================================
// Vertex Formats with Texture Coordinates
// ============================================================================

/**
 * @struct VertexFormatXYZUV1
 * Position + 1 texture coordinate (5 floats)
 * NOTE: This struct is ALSO defined in streakRender.h - using that version
 * to avoid duplication.
 */

/**
 * @struct VertexFormatXYZUV2
 * Position + 2 texture coordinates (7 floats)
 * NOTE: This struct is ALSO defined elsewhere - not redefined here
 * to avoid duplication.
 */

/**
 * @struct VertexFormatXYZNUV1
 * Position + Normal + 1 texture coordinate (8 floats)
 */
struct VertexFormatXYZNUV1
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u1;
	float v1;
};

/**
 * @struct VertexFormatXYZNUV2
 * Position + Normal + 2 texture coordinates (10 floats)
 */
struct VertexFormatXYZNUV2
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u1;
	float v1;
	float u2;
	float v2;
};

// ============================================================================
// Vertex Formats with Diffuse Color
// ============================================================================

/**
 * @struct VertexFormatXYZDUV1
 * Position + Diffuse Color + 1 texture coordinate (6 floats + color)
 * NOTE: This struct is ALSO defined in seglinerenderer.h - using that version
 * to avoid duplication. This comment is kept for documentation purposes.
 */

/**
 * @struct VertexFormatXYZDUV2
 * Position + Diffuse Color + 2 texture coordinates (8 floats + color)
 * This struct needs to be defined and is used in W3DWater.cpp and W3DCustomEdging.cpp
 */
struct VertexFormatXYZDUV2
{
	float x;
	float y;
	float z;
	unsigned int diffuse;
	float u1;
	float v1;
	float u2;
	float v2;
};

// ============================================================================
// Vertex Formats with Normal + Diffuse Color
// ============================================================================

/**
 * @struct VertexFormatXYZNDUV1
 * Position + Normal + Diffuse Color + 1 texture coordinate (9 floats + color)
 */
struct VertexFormatXYZNDUV1
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned int diffuse;
	float u1;
	float v1;
};

/**
 * @struct VertexFormatXYZNDUV2
 * Position + Normal + Diffuse Color + 2 texture coordinates (11 floats + color)
 */
struct VertexFormatXYZNDUV2
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned int diffuse;
	float u1;
	float v1;
	float u2;
	float v2;
};

// ============================================================================
// Complex Vertex Formats
// ============================================================================

/**
 * @struct VertexFormatXYZNDUV1TG3
 * Complex format: Position + Normal + Diffuse + 4 texture coordinate sets
 * Used for advanced terrain/object rendering with tangent space mapping
 */
struct VertexFormatXYZNDUV1TG3
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	unsigned int diffuse;
	
	// 4 texture coordinate sets (various dimensions)
	float u0, v0;               // 2D texture coords (normal map)
	float u1, v1, w1;           // 3D texture coords (tangent space)
	float u2, v2, w2;           // 3D texture coords (binormal space)
	float u3, v3, w3;           // 3D texture coords (normal space)
};

/**
 * @struct VertexFormatXYZNUV2DMAP
 * Format for displacement mapping: Position + Normal + 3 texture coordinate sets
 * Set 0: 1D (displacement)
 * Set 1: 4D (displacement params)
 * Set 2: 2D (normal map coords)
 */
struct VertexFormatXYZNUV2DMAP
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	
	float disp;                 // 1D displacement value
	float disp_u, disp_v, disp_w, disp_q;  // 4D displacement parameters
	float nm_u, nm_v;           // 2D normal map coordinates
};

#endif // DX8FVF_VERTEX_FORMATS_H

