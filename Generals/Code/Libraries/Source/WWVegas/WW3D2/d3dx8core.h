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

// D3DX utility functions - stub implementations
inline int D3DXMatrixIdentity(D3DXMATRIX* pOut) { return 0; }
inline int D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2) { return 0; }
inline int D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM) { return 0; }
inline int D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM) { return 0; }

// D3DX texture functions - stub implementations
inline int D3DXCreateTextureFromFile(IDirect3DDevice8* pDevice, const char* pSrcFile, IDirect3DTexture8** ppTexture) { return D3D_OK; }
inline int D3DXCreateTextureFromFileEx(IDirect3DDevice8* pDevice, const char* pSrcFile, DWORD Width, DWORD Height, DWORD MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, void* pSrcInfo, void* pPalette, IDirect3DTexture8** ppTexture) { return D3D_OK; }

// D3DX mesh functions - stub implementations
inline int D3DXCreateBox(IDirect3DDevice8* pDevice, float Width, float Height, float Depth, void** ppMesh, void** ppAdjacency) { return D3D_OK; }
inline int D3DXCreateSphere(IDirect3DDevice8* pDevice, float Radius, DWORD Slices, DWORD Stacks, void** ppMesh, void** ppAdjacency) { return D3D_OK; }

#else
// On Windows, include the real DirectX extensions headers
#include <d3dx8core.h>
#endif // !_WIN32
