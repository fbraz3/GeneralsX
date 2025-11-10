#pragma once

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


#ifndef _WIN32

// Include core Windows types first
#include "win32_compat_core.h"

// Forward declarations for D3D interfaces (defined by game code)
struct IDirect3D8;

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
};
struct IDirect3DResource8;
struct IDirect3DBaseTexture8 {
    virtual int AddRef() { return 0; }
    virtual int Release() { return 0; }
};
struct IDirect3DTexture8;
struct IDirect3DSurface8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
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

// ============================================================================
// DirectX Format Enum (Texture Formats)
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
    D3DFMT_A4R4G4B4 = 24,       // Alpha 4:4:4:4
    D3DFMT_R3G3B2 = 27,         // 8-bit RGB
    D3DFMT_A8 = 28,             // Alpha only
    D3DFMT_A8R3G3B2 = 29,       // Alpha + RGB
    D3DFMT_X4R4G4B4 = 30,       // 16-bit RGB no alpha
    D3DFMT_A8P8 = 40,           // 8-bit palette with alpha
    D3DFMT_P8 = 41,             // 8-bit palette
    D3DFMT_L8 = 50,             // Luminance 8-bit
    D3DFMT_A8L8 = 51,           // Alpha-Luminance
    D3DFMT_A4L4 = 52,           // Alpha-Luminance 4:4 bits
    D3DFMT_V8U8 = 60,           // Bump map format
    D3DFMT_L6V5U5 = 61,         // Luminance + bump map
    D3DFMT_X8L8V8U8 = 62,       // Bump map format
    D3DFMT_DXT1 = 0x31545844,   // BC1 compression
    D3DFMT_DXT2 = 0x32545844,   // BC2 compression (alternative)
    D3DFMT_DXT3 = 0x33545844,   // BC2 compression
    D3DFMT_DXT4 = 0x34545844,   // BC3 compression (alternative)
    D3DFMT_DXT5 = 0x35545844,   // BC3 compression
    D3DFMT_D16_LOCKABLE = 70,   // 16-bit lockable z-buffer
    D3DFMT_D32 = 71,            // 32-bit z-buffer
    D3DFMT_D15S1 = 73,          // 15-bit z-buffer with 1-bit stencil
    D3DFMT_D24S8 = 75,          // 24-bit z-buffer with 8-bit stencil
    D3DFMT_D24X8 = 77,          // 24-bit z-buffer
    D3DFMT_D16 = 80,            // 16-bit z-buffer
    D3DFMT_D24X4S4 = 79,        // 24-bit z-buffer with 4-bit stencil
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102,
    // Xbox specific formats (for compatibility)
    D3DFMT_LIN_R8G8B8A8 = 0x100,
    D3DFMT_LIN_D24S8 = 0x101,
    D3DFMT_LIN_F24S8 = 0x102,
    D3DFMT_LIN_D16 = 0x103,
    D3DFMT_LIN_F16 = 0x104,
    D3DFMT_Q8W8V8U8 = 0x107,    // Bump map format with signed components
    D3DFMT_UYVY = 0x4F425559,   // UYVY format
    D3DFMT_YUY2 = 0x32595559    // YUY2 format
};
typedef enum D3DFORMAT D3DFORMAT;
#endif

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
// DirectX Primitive Type Enum
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
