#pragma once

#ifndef _WIN32

// DirectX 8 extensions compatibility header for non-Windows systems
#include "d3d8.h"

// Include the math compatibility layer which defines the types
#include "../WWMath/d3dx8math.h"

// DirectX 8 extensions types and functions - stub implementations
// Note: D3DXMATRIX, D3DXVECTOR3, D3DXVECTOR4 are defined in d3dx8math.h

// D3DX utility functions - stub implementations
#ifndef D3DX_MATRIX_FUNCTIONS_DEFINED
#define D3DX_MATRIX_FUNCTIONS_DEFINED
inline int D3DXMatrixIdentity(D3DXMATRIX* pOut) { return 0; }
inline int D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2) { return 0; }
inline int D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM) { return 0; }
// Note: D3DXMatrixInverse is defined in d3dx8math.h
#endif

// D3DX texture functions - stub implementations
#ifndef D3DX_TEXTURE_FUNCTIONS_DEFINED
#define D3DX_TEXTURE_FUNCTIONS_DEFINED
inline int D3DXCreateTextureFromFile(IDirect3DDevice8* pDevice, const char* pSrcFile, IDirect3DTexture8** ppTexture) { return D3D_OK; }
inline int D3DXCreateTextureFromFileEx(IDirect3DDevice8* pDevice, const char* pSrcFile, DWORD Width, DWORD Height, DWORD MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, void* pSrcInfo, void* pPalette, IDirect3DTexture8** ppTexture) { return D3D_OK; }
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
