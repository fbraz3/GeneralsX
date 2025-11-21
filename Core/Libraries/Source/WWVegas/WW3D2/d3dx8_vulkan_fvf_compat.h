/**
 * @file d3dx8fvf.h
 * @brief DirectX 8 Flexible Vertex Format (FVF) compatibility layer
 *
 * This header provides cross-platform definitions for DirectX 8 vertex format constants.
 * These constants are used to define the structure of vertex data in mesh rendering.
 *
 * Phase 01: DirectX 8 Compatibility Layer
 * Cross-platform support for macOS, Linux, Windows
 */

#ifndef D3DX8FVF_H
#define D3DX8FVF_H

#pragma once

/* ============================================================================
 * D3DFVF (Flexible Vertex Format) Position Format Constants
 * ============================================================================
 * These define the position component of vertex data
 */

#define D3DFVF_XYZ                0x0002      /**< Vertex has x, y, z coordinates (float * 3) */
#define D3DFVF_XYZRHW             0x0004      /**< Vertex has x, y, z, rhw (float * 4) */
#define D3DFVF_XYZB1              0x0006      /**< Vertex has x, y, z, beta weight (float * 4) */
#define D3DFVF_XYZB2              0x0008      /**< Vertex has x, y, z, beta1, beta2 (float * 5) */
#define D3DFVF_XYZB3              0x000A      /**< Vertex has x, y, z, beta1, beta2, beta3 (float * 6) */
#define D3DFVF_XYZB4              0x000C      /**< Vertex has x, y, z, beta1-4 (float * 7) */
#define D3DFVF_XYZB5              0x000E      /**< Vertex has x, y, z, beta1-5 (float * 8) */

/* ============================================================================
 * D3DFVF Component Constants
 * ============================================================================
 * These define optional components that can be added to position data
 */

#define D3DFVF_NORMAL             0x0010      /**< Vertex has x, y, z normal (float * 3) */
#define D3DFVF_PSIZE              0x0020      /**< Vertex has point size (float) */
#define D3DFVF_DIFFUSE            0x0040      /**< Vertex has diffuse color (D3DCOLOR) */
#define D3DFVF_SPECULAR           0x0080      /**< Vertex has specular color (D3DCOLOR) */

/* ============================================================================
 * D3DFVF Texture Coordinate Constants
 * ============================================================================
 * These define the number of texture coordinate sets
 */

#define D3DFVF_TEX0               0x0000      /**< No texture coordinates */
#define D3DFVF_TEX1               0x0100      /**< 1 texture coordinate set */
#define D3DFVF_TEX2               0x0200      /**< 2 texture coordinate sets */
#define D3DFVF_TEX3               0x0300      /**< 3 texture coordinate sets */
#define D3DFVF_TEX4               0x0400      /**< 4 texture coordinate sets */
#define D3DFVF_TEX5               0x0500      /**< 5 texture coordinate sets */
#define D3DFVF_TEX6               0x0600      /**< 6 texture coordinate sets */
#define D3DFVF_TEX7               0x0700      /**< 7 texture coordinate sets */
#define D3DFVF_TEX8               0x0800      /**< 8 texture coordinate sets */

/* ============================================================================
 * D3DFVF Texture Coordinate Size Macros
 * ============================================================================
 * These macros specify the size of each texture coordinate
 * Usage: D3DFVF_TEXCOORDSIZE2(0) for 2D texture coords in set 0
 */

#define D3DFVF_TEXCOORDSIZE1(n)   (1 << (n + 16))   /**< 1D texture coordinates (u) */
#define D3DFVF_TEXCOORDSIZE2(n)   (2 << (n + 16))   /**< 2D texture coordinates (u, v) */
#define D3DFVF_TEXCOORDSIZE3(n)   (3 << (n + 16))   /**< 3D texture coordinates (u, v, w) */
#define D3DFVF_TEXCOORDSIZE4(n)   (4 << (n + 16))   /**< 4D texture coordinates (u, v, w, q) */

#define D3DFVF_TEXTUREFORMAT2     0x00000000        /**< Two-dimensional texture coordinates */
#define D3DFVF_TEXTUREFORMAT1     0x00010000        /**< One-dimensional texture coordinates */
#define D3DFVF_TEXTUREFORMAT3     0x00020000        /**< Three-dimensional texture coordinates */
#define D3DFVF_TEXTUREFORMAT4     0x00030000        /**< Four-dimensional texture coordinates */

/* ============================================================================
 * D3DFVF Common Vertex Formats
 * ============================================================================
 * Pre-defined vertex format combinations for common use cases
 */

/** Basic vertex: position + normal + diffuse + 1 texture set */
#define D3DFVF_VERTEX             (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1)

/** Transformed vertex: pre-transformed position + diffuse + 1 texture set */
#define D3DFVF_TLVERTEX           (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

/** Unlit vertex: position + diffuse + 1 texture set */
#define D3DFVF_UNLITVERTEX        (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

/* ============================================================================
 * Game-Specific Vertex Format Constants (DX8_FVF_* naming convention)
 * ============================================================================
 * These define vertex formats commonly used in Generals game code
 */

/** Position + Diffuse + 1 Texture Coordinate (used in terrain, roads, scorches) */
#define DX8_FVF_XYZDUV1           (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)   /* 0x0142 */

/** Position + Diffuse + 2 Texture Coordinates */
#define DX8_FVF_XYZDUV2           (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)   /* 0x0242 */

/** Position + Normal + Diffuse + 1 Texture Coordinate */
#define DX8_FVF_XYZN DUV1         (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1) /* 0x0152 */

/** Position + Normal + Diffuse + 2 Texture Coordinates */
#define DX8_FVF_XYZNDUV2          (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2) /* 0x0252 */

/* ============================================================================
 * D3DDP (Device Dependent Primitive) Constants
 * ============================================================================
 * These define device-dependent primitive limitations
 */

#define D3DDP_MAXTEXCOORD         8           /**< Maximum number of texture coordinate sets */

/* ============================================================================
 * Additional Vertex Format Flags
 * ============================================================================
 */

#define D3DFVF_RESERVED0          0x001       /**< Reserved - must be 0 */
#define D3DFVF_TEXTUREFORMAT      0x00030000  /**< Texture coordinate format mask */
#define D3DFVF_TEXCOUNT           0x00F00000  /**< Texture coordinate count mask */

/**
 * Macro to extract texture coordinate count from FVF
 * @param fvf The FVF value
 * @return Number of texture coordinate sets (0-8)
 */
#define D3DFVF_GETTEXCOORDCOUNT(fvf)   (((fvf) & D3DFVF_TEXCOUNT) >> 20)

/**
 * Macro to extract texture format from FVF
 * @param fvf The FVF value
 * @return Texture format (1D, 2D, 3D, or 4D)
 */
#define D3DFVF_GETTEXTUREFORMAT(fvf)   (((fvf) & D3DFVF_TEXTUREFORMAT) >> 16)

/* ============================================================================
 * Type Definitions for FVF Sizes
 * ============================================================================
 */

/** Size in bytes of different vertex components */
#define D3DFVF_POSITION_SIZE      12    /**< Size of XYZ position (3 * float) */
#define D3DFVF_NORMAL_SIZE        12    /**< Size of normal (3 * float) */
#define D3DFVF_COLOR_SIZE         4     /**< Size of diffuse/specular color (DWORD) */
#define D3DFVF_TEXCOORD_SIZE      8     /**< Size of 2D texture coordinate (2 * float) */

#endif /* D3DX8FVF_H */
