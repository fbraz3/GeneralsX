#pragma once

#ifndef D3D8_GRAPHICS_COMPAT_H_INCLUDED
#define D3D8_GRAPHICS_COMPAT_H_INCLUDED

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
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_X1R5G5B5 = 26,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_DXT1 = 0x31545844,  // BC1 compression
    D3DFMT_DXT3 = 0x33545844,  // BC2 compression
    D3DFMT_DXT5 = 0x35545844,  // BC3 compression
    D3DFMT_D32 = 80,
    D3DFMT_D24S8 = 75,
    D3DFMT_D24X8 = 77,
    D3DFMT_D16 = 80,
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102,
    D3DFMT_L8 = 50,             // Luminance 8-bit
    D3DFMT_L16 = 81,            // Luminance 16-bit
    D3DFMT_A8L8 = 51,           // Alpha-Luminance
    D3DFMT_A4L4 = 52            // Alpha-Luminance 4:4 bits
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
    D3DRS_STENCILWRITEMASK = 0x3B
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
// ============================================================================

#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
struct D3DMATRIX {
    float m[4][4];
};
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

#endif // !_WIN32

#endif // D3D8_GRAPHICS_COMPAT_H_INCLUDED
