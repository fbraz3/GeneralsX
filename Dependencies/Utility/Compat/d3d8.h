#pragma once

// Minimal Direct3D8 compatibility stubs to satisfy legacy includes on non-Windows builds.
// These are intentionally tiny placeholders and do not provide real Direct3D functionality.

#ifndef _D3D8_H_STUB
#define _D3D8_H_STUB

#include <stdint.h>

typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned int UINT;
typedef unsigned long HRESULT;

#define D3D_OK 0

// Basic types used in code

typedef struct D3DMATRIX { float m[4][4]; } D3DMATRIX;
typedef struct D3DLIGHT8 { int dummy; } D3DLIGHT8;
typedef struct D3DMATERIAL8 { int dummy; } D3DMATERIAL8;
typedef struct D3DVIEWPORT8 { int X; int Y; int Width; int Height; } D3DVIEWPORT8;

// Simple enums and flags (values chosen to be sensible for bitwise ops)
typedef enum D3DPOOL { D3DPOOL_DEFAULT = 0, D3DPOOL_MANAGED = 1 } D3DPOOL;
typedef enum D3DTRANSFORMSTATETYPE { D3DTS_WORLD = 0, D3DTS_VIEW = 1, D3DTS_PROJECTION = 2 } D3DTRANSFORMSTATETYPE;
typedef enum D3DRENDERSTATETYPE { D3DRS_ZBIAS = 0 } D3DRENDERSTATETYPE;
typedef enum D3DTEXTURESTAGESTATETYPE { D3DTSS_COLOROP = 0 } D3DTEXTURESTAGESTATETYPE;

// Fixed-function vertex format flags (FVF) used by legacy code
#ifndef D3DFVF_XYZ
#define D3DFVF_XYZ 0x002
#endif
#ifndef D3DFVF_NORMAL
#define D3DFVF_NORMAL 0x010
#endif
#ifndef D3DFVF_TEX1
#define D3DFVF_TEX1 0x0100
#endif
#ifndef D3DFVF_TEX2
#define D3DFVF_TEX2 0x0200
#endif

typedef struct D3DADAPTER_IDENTIFIER8 { char Driver[128]; char Description[128]; unsigned long VendorId; } D3DADAPTER_IDENTIFIER8;

typedef struct D3DDEVICE_CREATION_PARAMETERS { int dummy; } D3DDEVICE_CREATION_PARAMETERS;

// Minimal D3D caps struct used in some checks
typedef struct D3DCAPS8 { int DeviceType; int Caps; } D3DCAPS8;

// Forward declarations for interfaces
class IDirect3D8 { public: virtual ~IDirect3D8() {} };

/**
 * @class IDirect3DDevice8
 * @brief Minimal Direct3D 8 Device interface stub
 * 
 * Phase 01: DirectX 8 Compatibility Layer
 * This class provides the core Direct3D 8 device interface.
 * Methods are implemented as no-op stubs that return appropriate defaults.
 */
class IDirect3DDevice8 { 
public: 
    virtual ~IDirect3DDevice8() {} 
    
    // Core rendering methods
    virtual DWORD CreateImageSurface(unsigned int width, unsigned int height, int format, void** surface) {
        if (surface) *surface = nullptr;
        return D3D_OK;
    }
    
    virtual DWORD CopyRects(void* src_surface, void* src_rects, unsigned int num_rects, void* dst_surface, void* dst_points) {
        return D3D_OK;
    }
};

class IDirect3DBaseTexture8 { public: virtual ~IDirect3DBaseTexture8() {} };
class IDirect3DTexture8 : public IDirect3DBaseTexture8 { 
public: 
    virtual ~IDirect3DTexture8() {} 
    
    // Texture methods
    virtual int GetSurfaceLevel(unsigned int level, void** surface) {
        if (surface) *surface = nullptr;
        return 0;
    }
};

class IDirect3DSurface8 { public: virtual ~IDirect3DSurface8() {} };
class IDirect3DSwapChain8 { public: virtual ~IDirect3DSwapChain8() {} };

// Minimal macros used by code
#ifndef CONST
#define CONST const
#endif

#endif // _D3D8_H_STUB
