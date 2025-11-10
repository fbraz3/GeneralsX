// Minimal, well-guarded stub of d3dx8tex.h used only to satisfy compile
// time references during the Phase 50 triage. This file avoids
// redefining symbols provided by the central compat header and only
// provides the smallest set of types/functions required by game code.

#ifndef GENERALS_D3DX8TEX_H
#define GENERALS_D3DX8TEX_H

// Basic image info
typedef struct D3DXIMAGE_INFO {
    unsigned int Width;
    unsigned int Height;
    unsigned int Format;
} D3DXIMAGE_INFO;

// Provide D3DSURFACE_DESC if not already provided elsewhere.
#ifndef D3DSURFACE_DESC_DEFINED
#define D3DSURFACE_DESC_DEFINED
typedef struct D3DSURFACE_DESC {
    unsigned int Width;
    unsigned int Height;
    unsigned int Format;
} D3DSURFACE_DESC;
#endif

// The central compat header provides forward-declarations for interface
// structs such as IDirect3DTexture8 and IDirect3DSurface8. If a full
// definition is needed (for member methods the code calls), define a
// lightweight struct only when not already defined.
#ifndef IDIRECT3DTEXTURE8_DEFINED
#define IDIRECT3DTEXTURE8_DEFINED
struct IDirect3DSurface8 {
    virtual int GetDesc(D3DSURFACE_DESC* desc) { if (desc) { desc->Width = 0; desc->Height = 0; desc->Format = 0; } return 0; }
    virtual int LockRect(void* /*pLockedRect*/, void* /*pRect*/, unsigned long /*Flags*/) { return 0; }
    virtual int UnlockRect() { return 0; }
    virtual unsigned long Release() { return 0; }
    virtual ~IDirect3DSurface8() {}
};

struct IDirect3DTexture8 : public IDirect3DBaseTexture8 {
    virtual int GetSurfaceLevel(unsigned int /*level*/, IDirect3DSurface8** out) { if (out) *out = nullptr; return 0; }
    // SetLOD is used by TerrainTex.cpp
    virtual int SetLOD(unsigned int /*LOD*/) { return 0; }
    virtual ~IDirect3DTexture8() {}
};
#endif // IDIRECT3DTEXTURE8_DEFINED

// D3DX filter constant referenced by terrain code
#ifndef D3DX_FILTER_BOX
#define D3DX_FILTER_BOX 0
#endif

// Texture filter enums — guard to avoid redefinition warnings
#ifndef D3DTEXF_POINT
#define D3DTEXF_POINT   1
#define D3DTEXF_LINEAR  2
#define D3DTEXF_ANISOTROPIC 3
#endif

// Single unambiguous prototype — match usage in TerrainTex.cpp (palette pointer
// may be NULL). Return type is int for compatibility with DX8_ErrorCode wrappers.
int D3DXFilterTexture(IDirect3DTexture8* pTexture, IDirect3DSurface8* pPalette, unsigned int dwFlags, unsigned int Filter);
int D3DXCreateTextureFromFileA(void* device, const char* file, IDirect3DTexture8** tex);

#endif // GENERALS_D3DX8TEX_H
