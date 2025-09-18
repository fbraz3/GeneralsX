#pragma once

#ifndef _WIN32

// DirectX 8 extensions compatibility header for non-Windows systems
#include "d3d8.h"

// DirectX 8 extensions types and functions - stub implementations

// Basic D3DX types
#ifndef D3DXMATRIX_DEFINED
#define D3DXMATRIX_DEFINED
typedef D3DMATRIX D3DXMATRIX;
#endif

#ifndef D3DXVECTOR3_DEFINED
#define D3DXVECTOR3_DEFINED
typedef struct {
    float x, y, z;
} D3DXVECTOR3;
#endif

#ifndef D3DXVECTOR4_DEFINED
#define D3DXVECTOR4_DEFINED
typedef struct {
    float x, y, z, w;
} D3DXVECTOR4;
#endif

// D3DX utility functions - use Core implementations instead
#ifndef D3DX_MATRIX_FUNCTIONS_DEFINED
#define D3DX_MATRIX_FUNCTIONS_DEFINED
// All matrix functions removed - use Core implementations
// Note: D3DXMatrixMultiply is defined in d3d8.h with proper signature
#endif

// D3DX texture functions - use Core implementations instead
#ifndef D3DX_TEXTURE_FUNCTIONS_DEFINED
#define D3DX_TEXTURE_FUNCTIONS_DEFINED
// All texture functions removed - use Core implementations
#endif

// D3DX mesh functions - stub implementations
#ifndef D3DX_MESH_FUNCTIONS_DEFINED
#define D3DX_MESH_FUNCTIONS_DEFINED
inline int D3DXCreateBox(IDirect3DDevice8* pDevice, float Width, float Height, float Depth, void** ppMesh, void** ppAdjacency) { return D3D_OK; }
inline int D3DXCreateSphere(IDirect3DDevice8* pDevice, float Radius, DWORD Slices, DWORD Stacks, void** ppMesh, void** ppAdjacency) { return D3D_OK; }
#endif

#else
// On Windows, include the real DirectX extensions headers
#include <d3dx8core.h>
#endif // !_WIN32
