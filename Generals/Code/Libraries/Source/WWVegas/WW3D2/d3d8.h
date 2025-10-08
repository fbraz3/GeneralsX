#pragma once

#ifdef _WIN32
// On Windows, include the real DirectX headers
#include <d3d8.h>
#else

// For GeneralsMD, we want to use the DirectX names directly,
// so we include the Core d3d8.h which has all the implementations
// and add macros to alias the CORE_ types to normal names

#include "../../../../../../Core/Libraries/Source/WWVegas/WW3D2/d3d8.h"

// Use macros to avoid typedef conflicts with forward declarations
#ifndef IDirect3D8
#define IDirect3D8 CORE_IDirect3D8
#endif

#ifndef IDirect3DDevice8  
#define IDirect3DDevice8 CORE_IDirect3DDevice8
#endif

#ifndef IDirect3DBaseTexture8
#define IDirect3DBaseTexture8 CORE_IDirect3DBaseTexture8
#endif

#ifndef IDirect3DTexture8
#define IDirect3DTexture8 CORE_IDirect3DTexture8
#endif

#ifndef IDirect3DSurface8
#define IDirect3DSurface8 CORE_IDirect3DSurface8
#endif

#ifndef IDirect3DVertexBuffer8
#define IDirect3DVertexBuffer8 CORE_IDirect3DVertexBuffer8
#endif

#ifndef IDirect3DIndexBuffer8
#define IDirect3DIndexBuffer8 CORE_IDirect3DIndexBuffer8
#endif

#ifndef IDirect3DSwapChain8
#define IDirect3DSwapChain8 CORE_IDirect3DSwapChain8
#endif

#ifndef IDirect3DCubeTexture8
#define IDirect3DCubeTexture8 CORE_IDirect3DCubeTexture8
#endif

#ifndef IDirect3DVolumeTexture8
#define IDirect3DVolumeTexture8 CORE_IDirect3DVolumeTexture8
#endif

// Define pointer types
#ifndef LPDIRECT3D8
#define LPDIRECT3D8 CORE_IDirect3D8*
#endif

#ifndef LPDIRECT3DDEVICE8
#define LPDIRECT3DDEVICE8 CORE_IDirect3DDevice8*
#endif

#ifndef LPDIRECT3DBASETEXTURE8
#define LPDIRECT3DBASETEXTURE8 CORE_IDirect3DBaseTexture8*
#endif

#ifndef LPDIRECT3DTEXTURE8
#define LPDIRECT3DTEXTURE8 CORE_IDirect3DTexture8*
#endif

#ifndef LPDIRECT3DSURFACE8
#define LPDIRECT3DSURFACE8 CORE_IDirect3DSurface8*
#endif

#ifndef LPDIRECT3DVERTEXBUFFER8
#define LPDIRECT3DVERTEXBUFFER8 CORE_IDirect3DVertexBuffer8*
#endif

#ifndef LPDIRECT3DINDEXBUFFER8
#define LPDIRECT3DINDEXBUFFER8 CORE_IDirect3DIndexBuffer8*
#endif

#ifndef LPDIRECT3DSWAPCHAIN8
#define LPDIRECT3DSWAPCHAIN8 CORE_IDirect3DSwapChain8*
#endif

#endif // _WIN32