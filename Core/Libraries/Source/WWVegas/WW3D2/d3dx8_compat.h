/**
 * @file d3dx8_compat.h
 * @brief DirectX 8 Extended (D3DX8) Utility Functions and Constants
 * 
 * Phase 01: DirectX 8 Compatibility Layer
 * Date: November 11, 2025
 *
 * This header provides D3DX8 utility function stubs for cross-platform compilation.
 * These functions are used by the game engine for texture operations, matrix math,
 * and other DirectX utilities. On non-Windows platforms, they are no-op stubs
 * that return D3D_OK.
 */

#pragma once

#ifndef D3DX8_COMPAT_H
#define D3DX8_COMPAT_H

#include <cmath>
#include <cstring>

/* =====================================================================
 * DirectX Math Constants
 * ===================================================================== */

#define D3DX_PI         3.14159265358979323846f
#define D3DX_2PI        6.28318530717958647692f
#define D3DX_PI_2       1.57079632679489661923f
#define D3DX_PI_4       0.78539816339744830962f

/* =====================================================================
 * Memory Utilities
 * ===================================================================== */

/**
 * @brief Zero out a memory region
 * 
 * Fills a block of memory with zeros. This is commonly used for
 * initializing structures to zero before use.
 * 
 * @param ptr       Pointer to memory to zero
 * @param size      Number of bytes to zero
 */
inline void ZeroMemory(void* ptr, size_t size) {
    std::memset(ptr, 0, size);
}

/* =====================================================================
 * D3DX Filter Flags
 * ===================================================================== */

#define D3DX_FILTER_NONE         0x00000000
#define D3DX_FILTER_POINT        0x00000001
#define D3DX_FILTER_LINEAR       0x00000002
#define D3DX_FILTER_TRIANGLE     0x00000003
#define D3DX_FILTER_BOX          0x00000004
#define D3DX_FILTER_MIRROR_U     0x00010000
#define D3DX_FILTER_MIRROR_V     0x00020000
#define D3DX_FILTER_MIRROR_W     0x00040000
#define D3DX_FILTER_DITHER       0x00080000

/* =====================================================================
 * D3DX Color Key
 * ===================================================================== */

#define D3DX_DEFAULT_NONPOW2_COL_KEY 0xFF000000

/* =====================================================================
 * D3DX Utility Function Stubs
 * ===================================================================== */

/**
 * @brief Load one surface into another
 * 
 * Copies and optionally converts pixel data from one surface to another.
 * This is commonly used for texture format conversion and blitting.
 * 
 * @param pDestSurface      Destination surface pointer
 * @param pDestPalette      Destination palette (usually NULL)
 * @param pDestRect         Destination rectangle (NULL = entire surface)
 * @param pSrcSurface       Source surface pointer
 * @param pSrcPalette       Source palette (usually NULL)
 * @param pSrcRect          Source rectangle (NULL = entire surface)
 * @param dwFilter          Filter type (D3DX_FILTER_*)
 * @param ColorKey          Color key for transparency (usually 0)
 * @return HRESULT (D3D_OK on success)
 */
inline int CORE_D3DXLoadSurfaceFromSurface(
    void* pDestSurface,
    void* pDestPalette,
    void* pDestRect,
    void* pSrcSurface,
    void* pSrcPalette,
    void* pSrcRect,
    unsigned long dwFilter,
    unsigned long ColorKey) {
    return 0; /* D3D_OK */
}

/**
 * @brief Filter a texture
 * 
 * Generates mipmaps or applies other filters to a texture.
 * 
 * @param pTexture          Texture to filter
 * @param pPalette          Palette (usually NULL)
 * @param SrcLevel          Source level
 * @param dwFilter          Filter type (D3DX_FILTER_*)
 * @return HRESULT (D3D_OK on success)
 */
inline int CORE_D3DXFilterTexture(
    void* pTexture,
    void* pPalette,
    unsigned long SrcLevel,
    unsigned long dwFilter) {
    return 0; /* D3D_OK */
}

/**
 * @brief Compatibility Mappings
 * 
 * Map legacy D3DX function calls to our CORE_ prefixed versions
 */
#ifndef D3DXLoadSurfaceFromSurface
#define D3DXLoadSurfaceFromSurface CORE_D3DXLoadSurfaceFromSurface
#endif

#ifndef D3DXFilterTexture
#define D3DXFilterTexture CORE_D3DXFilterTexture
#endif

/**
 * @brief Create a texture from a file
 * 
 * Loads a texture from a file (TGA, DDS, JPG, etc).
 * 
 * @param pDevice           Direct3D device
 * @param pSrcFile          Source filename
 * @param ppTexture         Output texture pointer
 * @return HRESULT (D3D_OK on success, E_FAIL on file not found)
 */
inline int D3DXCreateTextureFromFile(
    void* pDevice,
    const char* pSrcFile,
    void** ppTexture) {
    if (ppTexture) *ppTexture = nullptr;
    return 0x80004005; /* E_FAIL */
}

/**
 * @brief Create a texture from memory
 * 
 * Loads a texture from memory buffer.
 * 
 * @param pDevice           Direct3D device
 * @param pSrcData          Source data buffer
 * @param SrcDataSize       Size of source data
 * @param ppTexture         Output texture pointer
 * @return HRESULT (D3D_OK on success)
 */
inline int D3DXCreateTextureFromFileInMemory(
    void* pDevice,
    const void* pSrcData,
    unsigned long SrcDataSize,
    void** ppTexture) {
    if (ppTexture) *ppTexture = nullptr;
    return 0; /* D3D_OK */
}

/**
 * @brief Load surface from file
 * 
 * Loads surface data from a file directly.
 * 
 * @param pDestSurface      Destination surface
 * @param pDestPalette      Destination palette (NULL)
 * @param pDestRect         Destination rectangle (NULL = entire surface)
 * @param pSrcFile          Source filename
 * @param pSrcRect          Source rectangle (NULL = entire surface)
 * @param dwFilter          Filter type
 * @param ColorKey          Color key
 * @param pSrcInfo          Optional: receives source image info
 * @return HRESULT
 */
inline int D3DXLoadSurfaceFromFile(
    void* pDestSurface,
    void* pDestPalette,
    void* pDestRect,
    const char* pSrcFile,
    void* pSrcRect,
    unsigned long dwFilter,
    unsigned long ColorKey,
    void* pSrcInfo) {
    return 0x80004005; /* E_FAIL */
}

/**
 * @brief Create a cube texture from a file
 * 
 * Loads a cube texture (all 6 faces) from a single file.
 * 
 * @param pDevice           Direct3D device
 * @param pSrcFile          Source filename
 * @param ppCubeTexture     Output cube texture pointer
 * @return HRESULT
 */
inline int D3DXCreateCubeTextureFromFile(
    void* pDevice,
    const char* pSrcFile,
    void** ppCubeTexture) {
    if (ppCubeTexture) *ppCubeTexture = nullptr;
    return 0x80004005; /* E_FAIL */
}

/**
 * @brief Create a volume texture from a file
 * 
 * Loads a 3D volume texture from a file.
 * 
 * @param pDevice           Direct3D device
 * @param pSrcFile          Source filename
 * @param ppVolumeTexture   Output volume texture pointer
 * @return HRESULT
 */
inline int D3DXCreateVolumeTextureFromFile(
    void* pDevice,
    const char* pSrcFile,
    void** ppVolumeTexture) {
    if (ppVolumeTexture) *ppVolumeTexture = nullptr;
    return 0x80004005; /* E_FAIL */
}

/**
 * @brief Create texture from resource
 * 
 * Loads a texture from Windows resource (WINDOWS ONLY - returns E_FAIL on other platforms)
 * 
 * @param pDevice           Direct3D device
 * @param hModule           Module handle
 * @param pResource         Resource name
 * @param ppTexture         Output texture pointer
 * @return HRESULT (always E_FAIL on non-Windows)
 */
inline int D3DXCreateTextureFromResource(
    void* pDevice,
    void* hModule,
    const char* pResource,
    void** ppTexture) {
    if (ppTexture) *ppTexture = nullptr;
    return 0x80004005; /* E_FAIL - not supported on non-Windows */
}

/**
 * @brief Get image info from file
 * 
 * Reads image dimensions and format from a file without loading full data.
 * 
 * @param pSrcFile          Source filename
 * @param pSrcInfo          Output structure (receives width, height, depth, mipmap count, format)
 * @return HRESULT
 */
inline int D3DXGetImageInfoFromFile(
    const char* pSrcFile,
    void* pSrcInfo) {
    return 0x80004005; /* E_FAIL */
}

/**
 * @brief Utility: compute normal map from height map
 * 
 * Generates normal map from a height map (used for texture effects).
 * 
 * @param pSrcTexture       Height map texture
 * @param Flags             Processing flags
 * @param pHeightToNormalConstantScale  Scaling factor
 * @return HRESULT
 */
inline int D3DXComputeNormalMap(
    void* pSrcTexture,
    unsigned long Flags,
    unsigned long Channel,
    float Amplitude) {
    return 0; /* D3D_OK */
}

#endif /* D3DX8_COMPAT_H */
