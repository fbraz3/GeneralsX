/**
 * @file d3dx8fvf.h
 * @brief DirectX 8 Flexible Vertex Format (FVF) constants and macros
 * 
 * Phase 01: DirectX 8 Compatibility Layer
 * Date: November 11, 2025
 *
 * This header defines all D3DFVF_* constants and related macros used by the
 * game engine for mesh vertex format definitions. These constants are required
 * for cross-platform compilation on macOS, Linux, and Windows.
 */

#ifndef D3DX8FVF_H
#define D3DX8FVF_H

/**
 * Flexible Vertex Format (FVF) Flags
 * These flags define which components are present in a vertex
 */

/* Vertex coordinate types */
#define D3DFVF_XYZ              0x0002  /* Vertex has x, y, z coordinates */
#define D3DFVF_XYZRHW           0x0004  /* Vertex has x, y, z, rhw (transformed) */
#define D3DFVF_NORMAL           0x0010  /* Vertex has normal vector */
#define D3DFVF_PSIZE            0x0020  /* Vertex has point size */

/* Vertex color types */
#define D3DFVF_DIFFUSE          0x0040  /* Vertex has diffuse color */
#define D3DFVF_SPECULAR         0x0080  /* Vertex has specular color */

/* Texture coordinate types */
#define D3DFVF_TEX0             0x0000  /* No texture coordinates */
#define D3DFVF_TEX1             0x0100  /* 1 texture coordinate set */
#define D3DFVF_TEX2             0x0200  /* 2 texture coordinate sets */
#define D3DFVF_TEX3             0x0300  /* 3 texture coordinate sets */
#define D3DFVF_TEX4             0x0400  /* 4 texture coordinate sets */
#define D3DFVF_TEX5             0x0500  /* 5 texture coordinate sets */
#define D3DFVF_TEX6             0x0600  /* 6 texture coordinate sets */
#define D3DFVF_TEX7             0x0700  /* 7 texture coordinate sets */
#define D3DFVF_TEX8             0x0800  /* 8 texture coordinate sets */

/* Texture coordinate format */
#define D3DFVF_TEXTUREFORMAT2   0x0000  /* 2D texture coordinates (default) */
#define D3DFVF_TEXTUREFORMAT1   0x1000  /* 1D texture coordinates */
#define D3DFVF_TEXTUREFORMAT3   0x2000  /* 3D texture coordinates */
#define D3DFVF_TEXTUREFORMAT4   0x3000  /* 4D texture coordinates */

/**
 * Texture Coordinate Size Macros
 * These macros set the size of texture coordinates for each texture set
 * 
 * Parameters:
 *   coord_set - texture coordinate set index (0-7)
 * 
 * Usage:
 *   #define MY_FVF (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | \
 *                    D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1))
 */

#define D3DFVF_TEXCOORDSIZE1(coord_set)  (D3DFVF_TEXTUREFORMAT1 << (coord_set + 16))
#define D3DFVF_TEXCOORDSIZE2(coord_set)  (D3DFVF_TEXTUREFORMAT2 << (coord_set + 16))
#define D3DFVF_TEXCOORDSIZE3(coord_set)  (D3DFVF_TEXTUREFORMAT3 << (coord_set + 16))
#define D3DFVF_TEXCOORDSIZE4(coord_set)  (D3DFVF_TEXTUREFORMAT4 << (coord_set + 16))

/**
 * Reserved FVF format flags (internal use)
 */
#define D3DFVF_RESERVED0        0x0001
#define D3DFVF_RESERVED2        0x2000
#define D3DFVF_RESERVED3        0x4000
#define D3DFVF_RESERVED4        0x8000

/**
 * Drawing Primitive Constants
 */

/* Maximum texture coordinates per vertex */
#define D3DDP_MAXTEXCOORD       8

/* Vertex size calculations (in bytes) */
#define D3D_VERTEX_SIZE_XYZ                 12  /* 3 * sizeof(float) */
#define D3D_VERTEX_SIZE_NORMAL              12  /* 3 * sizeof(float) */
#define D3D_VERTEX_SIZE_DIFFUSE              4  /* sizeof(DWORD) */
#define D3D_VERTEX_SIZE_SPECULAR             4  /* sizeof(DWORD) */
#define D3D_VERTEX_SIZE_TEX_COORD_2D         8  /* 2 * sizeof(float) */
#define D3D_VERTEX_SIZE_TEX_COORD_3D        12  /* 3 * sizeof(float) */
#define D3D_VERTEX_SIZE_TEX_COORD_4D        16  /* 4 * sizeof(float) */

/**
 * Common Vertex Format Definitions
 * These are typical combinations used in game rendering
 */

/* Transformed vertices with lighting (typical for simple meshes) */
#define D3DFVF_TLVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

/* Lit vertices with texture (typical for game objects) */
#define D3DFVF_LVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

/* Unlit vertices with texture (typical for particle effects) */
#define D3DFVF_VERTEX (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)

/* Simple colored vertices */
#define D3DFVF_COLORVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

/**
 * Primitive Type Definitions (helper constants)
 * Note: These are defined in d3dtypes.h but included here for reference
 */
#define D3DPT_POINTLIST         1   /* List of isolated points */
#define D3DPT_LINELIST          2   /* List of isolated lines */
#define D3DPT_LINESTRIP         3   /* Connected line strip */
#define D3DPT_TRIANGLELIST      4   /* List of isolated triangles */
#define D3DPT_TRIANGLESTRIP     5   /* Connected triangle strip */
#define D3DPT_TRIANGLEFAN       6   /* Triangle fan */

#endif /* D3DX8FVF_H */
