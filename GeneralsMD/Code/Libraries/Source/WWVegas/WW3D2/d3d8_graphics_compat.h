// Synchronized with the canonical Core compatibility header to ensure
// GeneralsMD translation units see the same Direct3D8 tokens, types and
// helpers as Core builds. This file intentionally mirrors
// `Core/Libraries/Source/WWVegas/WW3D2/d3d8_graphics_compat.h` so PCH and
// wide-include scenarios remain consistent.

#pragma once

#include <string.h>

#ifndef D3D8_GRAPHICS_COMPAT_H_INCLUDED
#define D3D8_GRAPHICS_COMPAT_H_INCLUDED

// Ensure vertex-declaration (VSD) macros are available early so translation
// units that reference them during static declaration compilation can
// compile even when platform-specific headers are not present.
#ifndef D3DVSD_STREAM
#define D3DVSD_STREAM(_s)       (0x10000000u | (unsigned)(_s))
#endif
#ifndef D3DVSD_REG
#define D3DVSD_REG(_r,_t)      (((unsigned)(_r) & 0xffu) | (((unsigned)(_t) & 0xffu) << 8))
#endif

// NOTE: Earlier versions of this file attempted to force an alias of
// `D3DXMATRIX` to the project's `Matrix4x4` before including the canonical
// core compatibility types. That caused a typedef redefinition when the
// core `win32_compat_core.h` already defines `struct D3DXMATRIX` and led to
// initializer/union errors. The safe behavior is to include the canonical
// core header first and then provide fallbacks below (the later section in
// this file already provides robust, guarded helpers). Keep this block
// disabled to avoid conflicting typedefs and excess-initializer issues.
#ifndef D3DVSDT_FLOAT3
#define D3DVSDT_FLOAT3         3
#endif
#ifndef D3DVSDT_FLOAT2
#define D3DVSDT_FLOAT2         2
#endif
#ifndef D3DVSDT_FLOAT4
#define D3DVSDT_FLOAT4         4
#endif
#ifndef D3DVSDT_D3DCOLOR
#define D3DVSDT_D3DCOLOR       5
#endif
#ifndef D3DVSD_END
#undef D3DVSD_END
#define D3DVSD_END()           0x00000000u
#endif


#ifndef _WIN32

// Include core Windows types first
#include "win32_compat_core.h"

// Forward declarations for D3D interfaces (defined by game code)
struct IDirect3D8;
// NOTE: The full `D3DPRIMITIVETYPE` enum is defined later in this
// header. Do not forward-declare enums in C++ — the concrete enum
// definition appears below and will be visible to translation units
// that include this file.
// Ensure D3DPOOL is available for early references (some TUs include the
// device stub before the full enum below). Provide a minimal typed enum
// here so method signatures referencing `D3DPOOL` compile in single-TU
// (-j1) builds. This is reversible and matches the canonical values used
// elsewhere in the project.
#ifndef D3DPOOL_DEFINED
#define D3DPOOL_DEFINED
typedef enum D3DPOOL {
    D3DPOOL_DEFAULT = 0,
    D3DPOOL_MANAGED = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH = 3
} D3DPOOL;
#endif

// Forward-declaration for `D3DFORMAT` removed. Rely on the canonical
// definition provided by `win32_compat_core.h` (or this header's later
// `enum D3DFORMAT`) to avoid typedef/enum conflicts when multiple
// compatibility headers are included in a single translation unit.

// ============================================================================
// DirectX Primitive Type Enum
// Provide the primitive enum before the device stub so method signatures
// using `D3DPRIMITIVETYPE` are valid in C++ translation units that include
// this compatibility header early (single-TU deterministic builds).
// This is the same canonical definition used elsewhere in the project.
// ============================================================================

#ifndef D3DPRIMITIVETYPE_DEFINED
#define D3DPRIMITIVETYPE_DEFINED
enum D3DPRIMITIVETYPE {
    D3DPT_POINTLIST = 1,
    D3DPT_LINELIST = 2,
    D3DPT_LINESTRIP = 3,
    D3DPT_TRIANGLELIST = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN = 6
};
typedef enum D3DPRIMITIVETYPE D3DPRIMITIVETYPE;
#endif

// ============================================================================
// DirectX Format Enum (Texture Formats)
// Ensure the texture-format enum is available before the device stub so
// method signatures referencing `D3DFORMAT` are always well-formed in
// translation units that include this header early (single-TU builds).
// Guarded with `D3DFORMAT_DEFINED` so the canonical core header can
// provide the definition without conflict.
// ============================================================================

#ifndef D3DFORMAT_DEFINED
#define D3DFORMAT_DEFINED
enum D3DFORMAT {
    D3DFMT_UNKNOWN = 0,
    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 26,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 24,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8 = 28,
    D3DFMT_A8R3G3B2 = 29,
    D3DFMT_X4R4G4B4 = 30,
    D3DFMT_A8P8 = 40,
    D3DFMT_P8 = 41,
    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_A4L4 = 52,
    D3DFMT_V8U8 = 60,
    D3DFMT_L6V5U5 = 61,
    D3DFMT_X8L8V8U8 = 62,
    D3DFMT_DXT1 = 0x31545844,
    D3DFMT_DXT2 = 0x32545844,
    D3DFMT_DXT3 = 0x33545844,
    D3DFMT_DXT4 = 0x34545844,
    D3DFMT_DXT5 = 0x35545844,
    D3DFMT_D16_LOCKABLE = 70,
    D3DFMT_D32 = 71,
    D3DFMT_D15S1 = 73,
    D3DFMT_D24S8 = 75,
    D3DFMT_D24X8 = 77,
    D3DFMT_D16 = 80,
    D3DFMT_D24X4S4 = 79,
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102,
    D3DFMT_LIN_R8G8B8A8 = 0x100,
    D3DFMT_LIN_D24S8 = 0x101,
    D3DFMT_LIN_F24S8 = 0x102,
    D3DFMT_LIN_D16 = 0x103,
    D3DFMT_LIN_F16 = 0x104,
    D3DFMT_Q8W8V8U8 = 0x107,
    D3DFMT_UYVY = 0x4F425559,
    D3DFMT_YUY2 = 0x32595559
};
typedef enum D3DFORMAT D3DFORMAT;
#endif

struct IDirect3DDevice8 {
    // Stub interface for cross-platform compilation
    // Implementation will be provided by Vulkan backend in Phase 52+
    virtual int SetVertexShader(unsigned long shader) { return 0; }
    virtual int SetPixelShader(unsigned long shader) { return 0; }
    virtual int SetVertexShaderConstant(int reg, const void* data, int count) { return 0; }
    virtual int SetPixelShaderConstant(int reg, const void* data, int count) { return 0; }
#ifdef D3DXVECTOR4_DEFINED
    // Convenience overloads accepting common D3DX types used by game code.
    // These simply forward to the generic pointer-based virtuals so TU code
    // that passes vectors by value/reference compiles without changes.
    virtual int SetVertexShaderConstant(int reg, const D3DXVECTOR4& v, int count) { return SetVertexShaderConstant(reg, &v, count); }
    virtual int SetPixelShaderConstant(int reg, const D3DXVECTOR4& v, int count) { return SetPixelShaderConstant(reg, &v, count); }
#endif
    // Some code calls DeletePixelShader() on the device object; provide a
    // member stub so those calls compile on non-Windows builds.
    virtual int DeletePixelShader(unsigned long shader) { (void)shader; return 0; }
    virtual int SetTransform(unsigned long state, const struct D3DMATRIX* matrix) { return 0; }
    virtual int GetTransform(unsigned long state, struct D3DMATRIX* matrix) { return 0; }
    virtual int SetMaterial(const struct D3DMATERIAL8* material) { return 0; }
    virtual int SetRenderState(unsigned long state, unsigned long value) { return 0; }
    virtual int GetRenderState(unsigned long state, unsigned long* value) { return 0; }
    // Provide GetRenderTarget stub used by some translation units.
    // Signature matches common usage: `GetRenderTarget(IDirect3DSurface8**)`.
    virtual int GetRenderTarget(struct IDirect3DSurface8** ppSurface) { (void)ppSurface; return 0; }
    virtual int SetTextureStageState(unsigned long stage, unsigned long state, unsigned long value) { return 0; }
    virtual int GetTextureStageState(unsigned long stage, unsigned long state, unsigned long* value) { return 0; }
    virtual int SetLight(unsigned long index, const struct D3DLIGHT8* light) { return 0; }
    virtual int GetLight(unsigned long index, struct D3DLIGHT8* light) { return 0; }
    virtual int LightEnable(unsigned long index, int enable) { return 0; }
    virtual int SetClipPlane(unsigned long index, const float* plane) { return 0; }
    virtual int GetClipPlane(unsigned long index, float* plane) { return 0; }
    virtual int SetTexture(unsigned long stage, struct IDirect3DBaseTexture8* texture) { return 0; }
    virtual int CopyRects(struct IDirect3DSurface8* src, const void* srcRects, unsigned long nRects, struct IDirect3DSurface8* dst, const void* dstPoints) { return 0; }
    virtual int TestCooperativeLevel() { return 0; }  // Phase 50 graphics stub
    virtual int Release() { return 0; }
    // Minimal buffer creation APIs used by game code (stubs for non-Windows builds)
    virtual int CreateIndexBuffer(unsigned long length, unsigned long usage, unsigned long format, D3DPOOL pool, struct IDirect3DIndexBuffer8** ppIndexBuffer) { (void)length; (void)usage; (void)format; (void)pool; (void)ppIndexBuffer; return 0; }
    virtual int CreateVertexBuffer(unsigned long length, unsigned long usage, unsigned long fvf, D3DPOOL pool, struct IDirect3DVertexBuffer8** ppVertexBuffer) { (void)length; (void)usage; (void)fvf; (void)pool; (void)ppVertexBuffer; return 0; }
    // Minimal CreateTexture stub used by some shader/render paths that
    // allocate a render-target texture at runtime. The stub returns
    // success but does not allocate a real texture on non-Windows builds.
    virtual int CreateTexture(unsigned int Width, unsigned int Height, unsigned int Levels, unsigned long Usage, D3DFORMAT Format, D3DPOOL Pool, struct IDirect3DTexture8** ppTexture) { (void)Width; (void)Height; (void)Levels; (void)Usage; (void)Format; (void)Pool; if (ppTexture) *ppTexture = nullptr; return 0; }
    virtual int SetIndices(struct IDirect3DIndexBuffer8* ib, unsigned long baseVertexIndex) { (void)ib; (void)baseVertexIndex; return 0; }
    // Minimal stream/draw APIs used by game code
    virtual int SetStreamSource(unsigned long StreamNumber, struct IDirect3DVertexBuffer8* pStreamData, unsigned long Stride) { (void)StreamNumber; (void)pStreamData; (void)Stride; return 0; }
    // Historically some translation units call a 5-argument variant of
    // DrawIndexedPrimitive. Provide a lightweight overload that forwards
    // to the canonical 6-argument virtual. This keeps caller code simple
    // while preserving a single implementation point for backends.
    virtual int DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, unsigned long BaseVertexIndex, unsigned long MinIndex, unsigned long NumVertices, unsigned long StartIndex, unsigned long PrimCount) { (void)Type; (void)BaseVertexIndex; (void)MinIndex; (void)NumVertices; (void)StartIndex; (void)PrimCount; return 0; }
    virtual int DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, unsigned long StartVertex, unsigned long NumVertices, unsigned long StartIndex, unsigned long PrimCount) { return DrawIndexedPrimitive(Type, 0u, StartVertex, NumVertices, StartIndex, PrimCount); }
    // Minimal DrawPrimitiveUP stub used by legacy TUs (provides same
    // signature as DirectX8). Implementations in backend will override
    // or replace this during the Vulkan backend integration.
    virtual int DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, unsigned long PrimitiveCount, const void* pVertexStreamZeroData, unsigned long VertexStreamZeroStride) { (void)PrimitiveType; (void)PrimitiveCount; (void)pVertexStreamZeroData; (void)VertexStreamZeroStride; return 0; }
};
struct IDirect3DResource8;
struct IDirect3DBaseTexture8 {
    virtual int AddRef() { return 0; }
    virtual int Release() { return 0; }
};
struct IDirect3DTexture8;
struct IDirect3DSurface8;
// Minimal index/vertex buffer interface stubs so code can call Lock/Unlock/Release
struct IDirect3DVertexBuffer8 {
    virtual int Lock(unsigned long OffsetToLock, unsigned long SizeToLock, unsigned char** ppbData, unsigned long Flags) { (void)OffsetToLock; (void)SizeToLock; (void)ppbData; (void)Flags; return 0; }
    virtual int Unlock() { return 0; }
    virtual int Release() { return 0; }
};
struct IDirect3DIndexBuffer8 {
    virtual int Lock(unsigned long OffsetToLock, unsigned long SizeToLock, unsigned char** ppbData, unsigned long Flags) { (void)OffsetToLock; (void)SizeToLock; (void)ppbData; (void)Flags; return 0; }
    virtual int Unlock() { return 0; }
    virtual int Release() { return 0; }
};
struct IDirect3DVolumeTexture8;
struct IDirect3DCubeTexture8;
struct IDirect3DVolume8;
struct IDirect3DSwapChain8;

// ============================================================================
// DirectX 8 Graphics Constants
// ============================================================================

#define D3D_OK 0x00000000L
#define D3DERR_NOTAVAILABLE 0x8876017CL
#define D3DERR_OUTOFVIDEOMEMORY 0x8876017EL
#define D3DERR_INVALIDCALL 0x8876086FL
#define D3DERR_DEVICELOST 0x88760868L

// DirectX Primitive Constants
#define D3DDP_MAXTEXCOORD 8  // Maximum texture coordinate sets in DirectX 8

// Duplicate D3DFORMAT enum removed: a guarded definition exists
// earlier in this header to satisfy early-include translation units.
// Keeping a single guarded definition prevents typedef/enum collisions
// with the canonical core compatibility header while ensuring method
// signatures using `D3DFORMAT` remain well-formed.

// ============================================================================
// DirectX Pool Type Enum
// ============================================================================

#ifndef D3DPOOL_DEFINED
#define D3DPOOL_DEFINED
enum D3DPOOL {
    D3DPOOL_DEFAULT = 0,
    D3DPOOL_MANAGED = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH = 3
};
typedef enum D3DPOOL D3DPOOL;
#endif

// ============================================================================
// DirectX Transform State Type
// ============================================================================

#ifndef D3DTRANSFORMSTATETYPE_DEFINED
#define D3DTRANSFORMSTATETYPE_DEFINED
enum D3DTRANSFORMSTATETYPE {
    D3DTS_VIEW = 2,
    D3DTS_PROJECTION = 3,
    D3DTS_TEXTURE0 = 16,
    D3DTS_TEXTURE1 = 17,
    D3DTS_TEXTURE2 = 18,
    D3DTS_TEXTURE3 = 19,
    D3DTS_TEXTURE4 = 20,
    D3DTS_TEXTURE5 = 21,
    D3DTS_TEXTURE6 = 22,
    D3DTS_TEXTURE7 = 23,
    D3DTS_WORLD = 256,
    D3DTS_WORLD1 = 257,
    D3DTS_WORLD2 = 258,
    D3DTS_WORLD3 = 259
};
typedef enum D3DTRANSFORMSTATETYPE D3DTRANSFORMSTATETYPE;
#endif

// ============================================================================
// DirectX Render State Type
// ============================================================================

#ifndef D3DRENDERSTATETYPE_DEFINED
#define D3DRENDERSTATETYPE_DEFINED
enum D3DRENDERSTATETYPE {
    D3DRS_AMBIENT = 0x0A,
    D3DRS_LIGHTING = 0x0B,
    D3DRS_CULLMODE = 0x16,
    D3DRS_ZENABLE = 0x07,
    D3DRS_ZWRITEENABLE = 0x14,
    D3DRS_ALPHATESTENABLE = 0x15,
    D3DRS_ALPHAREF = 0x18,
    D3DRS_ALPHAFUNC = 0x19,
    D3DRS_TEXTUREFACTOR = 0x1C,
    D3DRS_DITHERENABLE = 0x04,
    D3DRS_SHADEMODE = 0x09,
    D3DRS_ALPHABLENDENABLE = 0x0D,
    D3DRS_SRCBLEND = 0x0E,
    D3DRS_DESTBLEND = 0x0F,
    D3DRS_ZFUNC = 0x17,
    D3DRS_FILLMODE = 0x08,
    D3DRS_ZBIAS = 0x20,
    D3DRS_FOGSTART = 0x24,
    D3DRS_FOGEND = 0x25,
    D3DRS_FOGENABLE = 0x26,
    D3DRS_FOGCOLOR = 0x27,
    D3DRS_FOGMODE = 0x28,
    D3DRS_FOGDENSITY = 0x29,
    D3DRS_STENCILENABLE = 0x34,
    D3DRS_STENCILPASS = 0x37,
    D3DRS_STENCILFAIL = 0x35,
    D3DRS_STENCILZFAIL = 0x36,
    D3DRS_STENCILFUNC = 0x38,
    D3DRS_STENCILREF = 0x39,
    D3DRS_STENCILMASK = 0x3A,
    D3DRS_STENCILWRITEMASK = 0x3B,
    D3DRS_COLORWRITEENABLE = 0x57  // Enable color channel writes (Phase 50+)
};
typedef enum D3DRENDERSTATETYPE D3DRENDERSTATETYPE;
#endif

// Local minimal Direct3D fixed-function tokens required by some GeneralsMD
// translation units. Values are chosen to be stable compile-time tokens
// and are safe to change later if a backend requires different values.
#ifndef D3DFVF_XYZRHW
#define D3DFVF_XYZRHW 0x00000040
#endif
#ifndef D3DFVF_DIFFUSE
#define D3DFVF_DIFFUSE 0x00004000
#endif

// Shade modes
#ifndef D3DSHADE_FLAT
#define D3DSHADE_FLAT 1
#endif
#ifndef D3DSHADE_GOURAUD
#define D3DSHADE_GOURAUD 2
#endif

// Primitive misc caps (compile-time presence only)
#ifndef D3DPMISCCAPS_COLORWRITEENABLE
#define D3DPMISCCAPS_COLORWRITEENABLE 0x00000040
#endif

// Stencil operation symbolic token used by game code (compile-time presence)
#ifndef D3DSTENCILOP_DECRSAT
#define D3DSTENCILOP_DECRSAT 7
#endif

// ============================================================================
// DirectX Texture Stage State Type
// ============================================================================

#ifndef D3DTEXTURESTAGESTATETYPE_DEFINED
#define D3DTEXTURESTAGESTATETYPE_DEFINED
enum D3DTEXTURESTAGESTATETYPE {
    D3DTSS_COLOROP = 1,
    D3DTSS_COLORARG1 = 2,
    D3DTSS_COLORARG2 = 3,
    D3DTSS_ALPHAOP = 4,
    D3DTSS_ALPHAARG1 = 5,
    D3DTSS_ALPHAARG2 = 6,
    D3DTSS_BUMPENVMAT00 = 7,
    D3DTSS_BUMPENVMAT01 = 8,
    D3DTSS_BUMPENVMAT10 = 9,
    D3DTSS_BUMPENVMAT11 = 10,
    D3DTSS_TEXCOORDINDEX = 11,
    D3DTSS_BUMPENVSCALE = 12,
    D3DTSS_BUMPENVOFFSET = 13,
    D3DTSS_TEXTURETRANSFORMFLAGS = 14,
    D3DTSS_COLORARG0 = 15,
    D3DTSS_ALPHAARG0 = 16,
    D3DTSS_RESULTARG = 17,
    D3DTSS_CONSTANT = 18,
    D3DTSS_ADDRESSU = 19,
    D3DTSS_ADDRESSV = 20,
    D3DTSS_ADDRESSW = 21,
    D3DTSS_MAGFILTER = 22,
    D3DTSS_MINFILTER = 23,
    D3DTSS_MIPFILTER = 24
};
typedef enum D3DTEXTURESTAGESTATETYPE D3DTEXTURESTAGESTATETYPE;
#endif

// ============================================================================
// DirectX Texture Argument Constants (D3DTA_* )
// Values chosen to match common DirectX headers so the DX8 wrapper
// and game code using these tokens behaves consistently across platforms.
// ============================================================================
#ifndef D3DTA_DIFFUSE
#define D3DTA_SELECTMASK        0x0000000f
#define D3DTA_DIFFUSE           0x00000000
#define D3DTA_CURRENT           0x00000001
#define D3DTA_TEXTURE           0x00000002
#define D3DTA_TFACTOR           0x00000003
#define D3DTA_SPECULAR          0x00000004
#define D3DTA_COMPLEMENT        0x00000010
#define D3DTA_ALPHAREPLICATE    0x00000020
#endif

// ============================================================================
// DirectX Texture Address Mode (D3DTADDRESS_*)
// ============================================================================
#ifndef D3DTADDRESS_WRAP
#define D3DTADDRESS_WRAP    1
#define D3DTADDRESS_MIRROR  2
#define D3DTADDRESS_CLAMP   3
#define D3DTADDRESS_BORDER  4
#endif

// ============================================================================
// DirectX Texture Filter (D3DTEXF_*) - commonly used symbolic names
// ============================================================================
#ifndef D3DTEXF_POINT
#define D3DTEXF_POINT   1
#define D3DTEXF_LINEAR  2
#define D3DTEXF_ANISOTROPIC 3
#endif

// Provide a compile-time symbolic 'none' texture filter for code paths
// that reference `D3DTEXF_NONE`. Set to zero so it behaves as a no-op
// when used in texture stage state macros.
#ifndef D3DTEXF_NONE
#define D3DTEXF_NONE 0
#endif

// ============================================================================
// DirectX Buffer Usage and Lock flags (minimal set)
// ==========================================================================
#ifndef D3DUSAGE_WRITEONLY
#define D3DUSAGE_WRITEONLY 0x00000008L
#endif
#ifndef D3DUSAGE_DYNAMIC
#define D3DUSAGE_DYNAMIC   0x00000200L
#endif

#ifndef D3DLOCK_DISCARD
#define D3DLOCK_DISCARD        0x00002000L
#endif
#ifndef D3DLOCK_NOOVERWRITE
#define D3DLOCK_NOOVERWRITE    0x00001000L
#endif
#ifndef D3DLOCK_READONLY
#define D3DLOCK_READONLY       0x00000010L
#endif

// ============================================================================
// DirectX Texture Operation Enum (D3DTOP_*)
// ============================================================================
#ifndef D3DTEXTUREOP_DEFINED
#define D3DTEXTUREOP_DEFINED
typedef enum {
    D3DTOP_DISABLE    = 1,
    D3DTOP_SELECTARG1 = 2,
    D3DTOP_SELECTARG2 = 3,

    D3DTOP_MODULATE   = 4,
    D3DTOP_MODULATE2X = 5,
    D3DTOP_MODULATE4X = 6,

    D3DTOP_ADD        = 7,
    D3DTOP_ADDSIGNED  = 8,
    D3DTOP_ADDSIGNED2X= 9,
    D3DTOP_SUBTRACT   = 10,
    D3DTOP_ADDSMOOTH  = 11,
    D3DTOP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREOP;
#endif

// ============================================================================
// Texture stage transform / texcoord index flags (D3DTSS_TCI_*)
// Values chosen to match reference headers used elsewhere in the repo.
// ============================================================================
#ifndef D3DTSS_TCI_CAMERASPACEPOSITION
#define D3DTSS_TCI_PASSTHRU                             0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL                    0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION                  0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR          0x00030000
#endif

// ============================================================================
// Texture transform flags (D3DTTFF_*) used with D3DTSS_TEXTURETRANSFORMFLAGS
// ============================================================================
#ifndef D3DTEXTURETRANSFORMFLAGS_DEFINED
#define D3DTEXTURETRANSFORMFLAGS_DEFINED
// Avoid collisions with win32 compatibility macros that define the same
// symbolic names (these are simple numeric macros in some headers). Undef
// them locally so we can declare a typed enum with the same identifiers.
#if defined(D3DTTFF_COUNT1) || defined(D3DTTFF_COUNT2) || defined(D3DTTFF_COUNT3) || defined(D3DTTFF_COUNT4) || defined(D3DTTFF_PROJECTED)
#undef D3DTTFF_COUNT1
#undef D3DTTFF_COUNT2
#undef D3DTTFF_COUNT3
#undef D3DTTFF_COUNT4
#undef D3DTTFF_PROJECTED
#endif
typedef enum _D3DTEXTURETRANSFORMFLAGS {
    D3DTTFF_DISABLE         = 0,
    D3DTTFF_COUNT1          = 1,
    D3DTTFF_COUNT2          = 2,
    D3DTTFF_COUNT3          = 3,
    D3DTTFF_COUNT4          = 4,
    D3DTTFF_PROJECTED       = 256,
    D3DTTFF_FORCE_DWORD     = 0x7fffffff
} D3DTEXTURETRANSFORMFLAGS;
#endif

// ============================================================================
// DirectX Texture Argument Constants (D3DTA_* )
// Provide D3DTA_TFACTOR which is used by some game code to select the
// texture-factor argument in texture stage operations. Value chosen to
// match a small integer used as an argument for platform compatibility.
// This is a compile-time convenience; runtime correctness is handled by
// the DX8 wrapper / Vulkan backend.
// ============================================================================
#ifndef D3DTA_TFACTOR
#define D3DTA_TFACTOR 0x00000003
#endif

// ============================================================================
// DirectX Light Type Enum
// ============================================================================

#ifndef D3DLIGHTTYPE_DEFINED
#define D3DLIGHTTYPE_DEFINED
enum D3DLIGHTTYPE {
    D3DLIGHT_POINT = 1,
    D3DLIGHT_SPOT = 2,
    D3DLIGHT_DIRECTIONAL = 3
};
typedef enum D3DLIGHTTYPE D3DLIGHTTYPE;
#endif

// ============================================================================
// (Primitive enum defined earlier to satisfy early TU usage)
// ============================================================================
// DirectX Cull Mode Enum
// ============================================================================

#ifndef D3DCULL_DEFINED
#define D3DCULL_DEFINED
enum D3DCULL {
    D3DCULL_NONE = 1,
    D3DCULL_CW = 2,
    D3DCULL_CCW = 3
};
typedef enum D3DCULL D3DCULL;
#endif

// ============================================================================
// DirectX Comparison Function Enum
// ============================================================================

#ifndef D3DCMPFUNC_DEFINED
#define D3DCMPFUNC_DEFINED
enum D3DCMPFUNC {
    D3DCMP_NEVER = 1,
    D3DCMP_LESS = 2,
    D3DCMP_EQUAL = 3,
    D3DCMP_LESSEQUAL = 4,
    D3DCMP_GREATER = 5,
    D3DCMP_NOTEQUAL = 6,
    D3DCMP_GREATEREQUAL = 7,
    D3DCMP_ALWAYS = 8
};
typedef enum D3DCMPFUNC D3DCMPFUNC;
#endif

// ============================================================================
// DirectX Blend Type Enum
// ============================================================================

#ifndef D3DBLEND_DEFINED
#define D3DBLEND_DEFINED
enum D3DBLEND {
    D3DBLEND_ZERO = 1,
    D3DBLEND_ONE = 2,
    D3DBLEND_SRCCOLOR = 3,
    D3DBLEND_INVSRCCOLOR = 4,
    D3DBLEND_SRCALPHA = 5,
    D3DBLEND_INVSRCALPHA = 6,
    D3DBLEND_DESTALPHA = 7,
    D3DBLEND_INVDESTALPHA = 8,
    D3DBLEND_DESTCOLOR = 9,
    D3DBLEND_INVDESTCOLOR = 10,
    D3DBLEND_SRCALPHASAT = 11,
    D3DBLEND_BOTHSRCALPHA = 12,
    D3DBLEND_BOTHINVSRCALPHA = 13,
    D3DBLEND_BLENDFACTOR = 14
};
typedef enum D3DBLEND D3DBLEND;
#endif

// ============================================================================
// DirectX Color (32-bit ARGB)
// ============================================================================

#ifndef D3DCOLOR_DEFINED
#define D3DCOLOR_DEFINED
typedef unsigned long D3DCOLOR;
#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)
#endif

// ============================================================================
// DirectX Matrix Structure
// Provide both a struct tag and a typedef so code can use either
// `struct D3DMATRIX` or `D3DMATRIX` uniformly across the codebase.
// ============================================================================

#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct D3DMATRIX {
    float m[4][4];
} D3DMATRIX;
#endif

// Older headers may refer to `_D3DMATRIX` (leading underscore). The
// canonical `win32_compat.h` already defines both `D3DMATRIX` and
// `_D3DMATRIX` on some platforms. Provide a local alias when missing so
// translation units that cast to `_D3DMATRIX *` compile in single-TU
// deterministic builds. This is reversible and safe when the canonical
// header already defines the symbol.
#ifndef _D3DMATRIX_DEFINED
#define _D3DMATRIX_DEFINED
typedef D3DMATRIX _D3DMATRIX;
#endif

// Minimal D3DX matrix compatibility: some shader helper code expects
// `D3DXMATRIX` and helper functions `D3DXMatrixTranslation` and
// `D3DXMatrixScaling`. Provide a conservative, self-contained D3DX
// matrix type (aliasing the D3DMATRIX memory layout) and always export
// inline callable helpers so translation units get real symbols even
// when macros are hidden by PCH/include-order. This keeps changes local
// and reversible: if the canonical headers provide real D3DX types these
// guards will prevent redefinition.

#ifndef D3DXMATRIX_DEFINED
#define D3DXMATRIX_DEFINED

// Prefer an explicit POD compatible with D3DMATRIX layout so we can
// provide reliable memcpy-based helpers. This avoids aliasing to
// project-specific `Matrix4x4` (which may be incomplete in some TUs)
// and prevents operator/initializer issues during -j1 builds.
typedef D3DMATRIX D3DXMATRIX;

// Unconditionally provide inline, real functions so callers always have
// symbol-level access to the helpers (macros may be invisible due to
// PCH/include-order). Use the D3DMATRIX layout and memcpy into the
// destination pointer to remain layout-compatible with existing code.
static inline void D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z) {
    if (!pOut) return;
    D3DMATRIX _tmp = {{{1.0f,0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f,0.0f},{0.0f,0.0f,1.0f,0.0f},{(x),(y),(z),1.0f}}};
    memcpy((void*)pOut, &_tmp, sizeof(D3DMATRIX));
}
static inline void D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz) {
    if (!pOut) return;
    D3DMATRIX _tmp = {{{(sx),0.0f,0.0f,0.0f},{0.0f,(sy),0.0f,0.0f},{0.0f,0.0f,(sz),0.0f},{0.0f,0.0f,0.0f,1.0f}}};
    memcpy((void*)pOut, &_tmp, sizeof(D3DMATRIX));
}

// Provide macro convenience for older code paths that expect macros.
#ifndef D3DXMatrixTranslation_MACRO
#define D3DXMatrixTranslation_MACRO(pOut,x,y,z) D3DXMatrixTranslation((pOut),(x),(y),(z))
#endif
#ifndef D3DXMatrixScaling_MACRO
#define D3DXMatrixScaling_MACRO(pOut,sx,sy,sz) D3DXMatrixScaling((pOut),(sx),(sy),(sz))
#endif

// Provide basic operator overloads for D3DXMATRIX so mixed-matrix
// arithmetic used by shader helpers compiles. These operate on the
// 4x4 float layout and return a concrete D3DXMATRIX value.
#ifndef D3DXMATRIX_OPS_DEFINED
#define D3DXMATRIX_OPS_DEFINED
static inline D3DXMATRIX operator*(const D3DXMATRIX &A, const D3DXMATRIX &B) {
    float a[4][4]; float b[4][4]; float r[4][4];
    memcpy(a, &A, sizeof(a));
    memcpy(b, &B, sizeof(b));
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) sum += a[i][k] * b[k][j];
            r[i][j] = sum;
        }
    }
    D3DXMATRIX out;
    memcpy(&out, r, sizeof(r));
    return out;
}
static inline D3DXMATRIX &operator*=(D3DXMATRIX &A, const D3DXMATRIX &B) { A = A * B; return A; }
#endif

#endif

// Minimal stencil operation constants used by game rendering paths. The
// exact numeric values are not critical for compile-time; these tokens
// allow the code to reference symbolic names and be implemented by the
// backend at runtime.
#ifndef D3DSTENCILOP_KEEP
#define D3DSTENCILOP_KEEP    1
#define D3DSTENCILOP_ZERO    2
#define D3DSTENCILOP_REPLACE 3
#define D3DSTENCILOP_INCR    4
#define D3DSTENCILOP_DECR    5
#endif

// ============================================================================
// DirectX Color Value (float RGBA)
// ============================================================================

#ifndef D3DCOLORVALUE_DEFINED
#define D3DCOLORVALUE_DEFINED
struct D3DCOLORVALUE {
    float r, g, b, a;
};
#endif

// ============================================================================
// DirectX Vector (3D float)
// ============================================================================

#ifndef D3DVECTOR_DEFINED
#define D3DVECTOR_DEFINED
struct D3DVECTOR {
    float x, y, z;
};
#endif

// ============================================================================
// DirectX Light Structure
// ============================================================================

#ifndef D3DLIGHT8_DEFINED
#define D3DLIGHT8_DEFINED
struct D3DLIGHT8 {
    D3DLIGHTTYPE Type;
    D3DCOLORVALUE Diffuse;
    D3DCOLORVALUE Specular;
    D3DCOLORVALUE Ambient;
    D3DVECTOR Position;
    D3DVECTOR Direction;
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
};
#endif

// ============================================================================
// DirectX Device Capabilities Structure
// ============================================================================

#ifndef D3DCAPS8_DEFINED
#define D3DCAPS8_DEFINED
struct D3DCAPS8 {
    DWORD DeviceType;
    DWORD Caps;
    DWORD Caps2;
    DWORD Caps3;
    DWORD CursorCaps;
    DWORD DevCaps;
    DWORD PrimitiveMiscCaps;
    DWORD RasterCaps;
    DWORD ZCmpCaps;
    DWORD SrcBlendCaps;
    DWORD DestBlendCaps;
    DWORD AlphaCmpCaps;
    DWORD ShadeCaps;
    DWORD TextureCaps;
    DWORD TextureFilterCaps;
    DWORD CubeTextureFilterCaps;
    DWORD VolumeTextureFilterCaps;
    DWORD TextureAddressCaps;
    DWORD VolumeTextureAddressCaps;
    DWORD LineCaps;
    DWORD MaxTextureWidth;
    DWORD MaxTextureHeight;
    DWORD MaxVolumeExtent;
    DWORD MaxTextureRepeat;
    DWORD MaxTextureAspectRatio;
    DWORD MaxAnisotropy;
    float MaxVertexW;
    float GuardBandLeft;
    float GuardBandTop;
    float GuardBandRight;
    float GuardBandBottom;
    float ExtentsAdjust;
    DWORD StencilCaps;
    DWORD FVFCaps;
    DWORD TextureOpCaps;
    DWORD MaxTextureBlendStages;
    DWORD MaxSimultaneousTextures;
    DWORD VertexProcessingCaps;
    DWORD MaxActiveLights;
    DWORD MaxUserClipPlanes;
    DWORD MaxVertexBlendMatrices;
    DWORD MaxVertexBlendMatrixIndex;
    float MaxPointSize;
    DWORD MaxPrimitiveCount;
    DWORD MaxVertexIndex;
    DWORD MaxStreams;
    DWORD MaxStreamStride;
    DWORD VertexShaderVersion;
    DWORD PixelShaderVersion;
};
#endif

// ============================================================================
// DirectX Adapter Identifier Structure
// ============================================================================

#ifndef D3DADAPTER_IDENTIFIER8_DEFINED
#define D3DADAPTER_IDENTIFIER8_DEFINED
struct D3DADAPTER_IDENTIFIER8 {
    char Driver[512];
    char Description[512];
    LARGE_INTEGER DriverVersion;
    DWORD VendorId;
    DWORD DeviceId;
    DWORD SubSysId;
    DWORD Revision;
    GUID DeviceIdentifier;
    DWORD WHQLLevel;
};
#endif

// ============================================================================
// DirectX Display Mode Structure
// ============================================================================

#ifndef D3DDISPLAYMODE_DEFINED
#define D3DDISPLAYMODE_DEFINED
struct D3DDISPLAYMODE {
    DWORD Width;
    DWORD Height;
    DWORD RefreshRate;
    D3DFORMAT Format;
};
#endif

// ============================================================================
// DirectX Viewport Structure
// ============================================================================

#ifndef D3DVIEWPORT8_DEFINED
#define D3DVIEWPORT8_DEFINED
struct D3DVIEWPORT8 {
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
};
#endif

// ============================================================================
// DirectX Material Structure
// ============================================================================

#ifndef D3DMATERIAL8_DEFINED
#define D3DMATERIAL8_DEFINED
struct D3DMATERIAL8 {
    float Diffuse[4];
    float Ambient[4];
    float Specular[4];
    float Emissive[4];
    float Power;
};
#endif

// ============================================================================
// DirectX FVF (Flexible Vertex Format) Macros
// ============================================================================

#ifndef D3DFVF_XYZ
#define D3DFVF_XYZ 0x002
#define D3DFVF_NORMAL 0x010
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX0 0x000
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200
#define D3DFVF_TEX3 0x300
#define D3DFVF_TEX4 0x400
#define D3DFVF_TEXCOORDSIZE1(n) (3 << (n*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(n) (0 << (n*2 + 16))
#define D3DFVF_TEXCOORDSIZE3(n) (1 << (n*2 + 16))
#define D3DFVF_TEXCOORDSIZE4(n) (2 << (n*2 + 16))
#endif

// ============================================================================
// DirectX Locked Rectangle (for texture surface locking)
// ============================================================================

#ifndef D3DLOCKED_RECT_DEFINED
#define D3DLOCKED_RECT_DEFINED
struct D3DLOCKED_RECT {
    INT Pitch;
    void* pBits;
};
#endif

// ============================================================================
// DirectX Locked Box (for volume texture locking)
// ============================================================================

#ifndef D3DLOCKED_BOX_DEFINED
#define D3DLOCKED_BOX_DEFINED
struct D3DLOCKED_BOX {
    INT RowPitch;
    INT SlicePitch;
    void* pBits;
};
#endif

// ============================================================================
// DirectX Fill Mode Enum
// ============================================================================

#ifndef D3DFILLMODE_DEFINED
#define D3DFILLMODE_DEFINED
enum D3DFILLMODE {
    D3DFILL_POINT = 1,
    D3DFILL_WIREFRAME = 2,
    D3DFILL_SOLID = 3
};
typedef enum D3DFILLMODE D3DFILLMODE;
#endif

// ============================================================================
// DirectX 8 Interface Stubs (void* implementations for compatibility)
// ============================================================================

// Forward declare interface structs (opaque pointers)
struct IDirect3D8;
struct IDirect3DDevice8;
struct IDirect3DTexture8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DSwapChain8;
struct IDirect3DSurface8;
struct IDirect3DVolumeTexture8;
struct IDirect3DCubeTexture8;
struct IDirect3DVolume8;
struct IDirect3DVertexShader8;
struct IDirect3DPixelShader8;

// Define pointer types
typedef struct IDirect3D8 *LPDIRECT3D8, *PDIRECT3D8;
typedef struct IDirect3DDevice8 *LPDIRECT3DDEVICE8, *PDIRECT3DDEVICE8;
typedef struct IDirect3DTexture8 *LPDIRECT3DTEXTURE8, *PDIRECT3DTEXTURE8;
typedef struct IDirect3DVertexBuffer8 *LPDIRECT3DVERTEXBUFFER8, *PDIRECT3DVERTEXBUFFER8;
typedef struct IDirect3DIndexBuffer8 *LPDIRECT3DINDEXBUFFER8, *PDIRECT3DINDEXBUFFER8;
typedef struct IDirect3DSwapChain8 *LPDIRECT3DSWAPCHAIN8, *PDIRECT3DSWAPCHAIN8;
typedef struct IDirect3DSurface8 *LPDIRECT3DSURFACE8, *PDIRECT3DSURFACE8;
typedef struct IDirect3DVolumeTexture8 *LPDIRECT3DVOLUMETEXTURE8, *PDIRECT3DVOLUMETEXTURE8;
typedef struct IDirect3DCubeTexture8 *LPDIRECT3DCUBETEXTURE8, *PDIRECT3DCUBETEXTURE8;
typedef struct IDirect3DVolume8 *LPDIRECT3DVOLUME8, *PDIRECT3DVOLUME8;
typedef struct IDirect3DVertexShader8 *LPDIRECT3DVERTEXSHADER8, *PDIRECT3DVERTEXSHADER8;
typedef struct IDirect3DPixelShader8 *LPDIRECT3DPIXELSHADER8, *PDIRECT3DPIXELSHADER8;

// ============================================================================
// DirectX Color Write Constants
// ============================================================================

#ifndef D3DCOLORWRITEENABLE_DEFINED
#define D3DCOLORWRITEENABLE_DEFINED
#define D3DCOLORWRITEENABLE_RED   0x00000001
#define D3DCOLORWRITEENABLE_GREEN 0x00000002
#define D3DCOLORWRITEENABLE_BLUE  0x00000004
#define D3DCOLORWRITEENABLE_ALPHA 0x00000008
#define D3DCOLORWRITEENABLE_ALL   (D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA)
#endif

#endif // !_WIN32

// ============================================================================
// Vertex shader declaration (VSD) helper macros used by DirectX8 shader
// declaration arrays. These macros are only required at compile-time so the
// original arrays in the game code can be present on non-Windows builds.
// Values are chosen as lightweight tokens and are not used at runtime by
// the Vulkan backend; they simply allow the code to compile.
// ============================================================================
#ifndef D3DVSD_STREAM
#define D3DVSD_STREAM(_s)       (0x10000000u | (unsigned)(_s))
#endif
#ifndef D3DVSD_REG
#define D3DVSD_REG(_r,_t)      (((unsigned)(_r) & 0xffu) | (((unsigned)(_t) & 0xffu) << 8))
#endif
#ifndef D3DVSDT_FLOAT3
#define D3DVSDT_FLOAT3         3
#endif
#ifndef D3DVSDT_FLOAT2
#define D3DVSDT_FLOAT2         2
#endif
#ifndef D3DVSDT_FLOAT4
#define D3DVSDT_FLOAT4         4
#endif
#ifndef D3DVSDT_D3DCOLOR
#define D3DVSDT_D3DCOLOR       5
#endif
#ifndef D3DVSD_END
#define D3DVSD_END             0x00000000u
#endif

// ============================================================================
// Common D3DTOP extensions used by game shaders (compile-time tokens)
// ============================================================================
#ifndef D3DTOP_MULTIPLYADD
#define D3DTOP_MULTIPLYADD     0x0000000C
#endif
#ifndef D3DTOP_DOTPRODUCT3
#define D3DTOP_DOTPRODUCT3     0x0000000D
#endif
// Some shader code references D3DTOP_BLENDCURRENTALPHA; define a compile-time
// token here to satisfy references. Numeric value chosen adjacent to other
// extension tokens and safe for compile-time use; actual runtime behavior
// is implemented by the backend.
#ifndef D3DTOP_BLENDCURRENTALPHA
#define D3DTOP_BLENDCURRENTALPHA 0x0000000E
#endif

// ============================================================================
// Minimal D3DX types used by shader constant setups in the game. These
// lightweight POD types allow passing vectors/matrices to Set*Constant
// wrapper overloads below so existing code compiles without invasive edits.
// ============================================================================
#ifndef D3DXVECTOR4_DEFINED
#define D3DXVECTOR4_DEFINED
struct D3DXVECTOR4 { float x, y, z, w; D3DXVECTOR4() = default; D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {} };
#endif
#ifndef D3DXVECTOR3_DEFINED
#define D3DXVECTOR3_DEFINED
struct D3DXVECTOR3 { float x, y, z; D3DXVECTOR3() = default; D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {} };
#endif

// ============================================================================
// Extend the IDirect3DDevice8 compatibility stub with small overloads used by
// game code when setting shader constants. These overloads forward to the
// generic void* variant so the stub remains simple while enabling type-safe
// calls from existing code.
// ============================================================================
#ifndef _WIN32
inline int IDirect3DDevice8_SetPixelShaderConstant_Fallback(IDirect3DDevice8* dev, int reg, const void* data, int count) { return dev->SetPixelShaderConstant(reg, data, count); }
inline int IDirect3DDevice8_SetVertexShaderConstant_Fallback(IDirect3DDevice8* dev, int reg, const void* data, int count) { return dev->SetVertexShaderConstant(reg, data, count); }

// Provide overload-style helpers (not virtual members) to be used by C++ code
// that previously relied on DirectX helper overloads. These are simple free
// functions to avoid modifying the minimal stub class layout above.
static inline int SetPixelShaderConstant(IDirect3DDevice8* dev, int reg, const D3DXVECTOR4& v, int count = 1) { return IDirect3DDevice8_SetPixelShaderConstant_Fallback(dev, reg, &v, count); }
static inline int SetVertexShaderConstant(IDirect3DDevice8* dev, int reg, const D3DXVECTOR4& v, int count = 1) { return IDirect3DDevice8_SetVertexShaderConstant_Fallback(dev, reg, &v, count); }

// Add DeletePixelShader stub on the compatibility type via a free function
// so code calling device->DeletePixelShader(...) still compiles when the
// compatibility wrapper is used. Some code expects a device-level delete call.
static inline int DeletePixelShader(IDirect3DDevice8* dev, unsigned long shader) { (void)shader; return 0; }
#endif

#endif // D3D8_GRAPHICS_COMPAT_H_INCLUDED

