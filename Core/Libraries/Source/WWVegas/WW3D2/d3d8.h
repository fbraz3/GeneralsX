#pragma once

#ifndef _WIN32

// DirectX 8 compatibility header for non-Windows systems
#include "win32_compat.h"
#include <cstdint>

// Windows API types needed by DirectX
// Forward declarations for DirectX interfaces
struct IDirect3D8;
struct IDirect3DDevice8;
struct IDirect3DTexture8;
struct IDirect3DSurface8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DBaseTexture8;

// Basic DirectX types
typedef void* LPDIRECT3D8;
typedef void* LPDIRECT3DDEVICE8;
typedef void* LPDIRECT3DTEXTURE8;
typedef void* LPDIRECT3DSURFACE8;
typedef void* LPDIRECT3DVERTEXBUFFER8;
typedef void* LPDIRECT3DINDEXBUFFER8;
typedef void* LPDIRECT3DSWAPCHAIN8;
typedef void* IDirect3DSwapChain8;
typedef void* LPDISPATCH;

// DirectX color type
typedef uint32_t D3DCOLOR;

// Constants
#define D3D_OK 0x00000000L
#define CONST const

// DirectX enums and constants
typedef enum {
    D3DFMT_UNKNOWN = 0,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_DXT1 = 827611204,
    D3DFMT_DXT3 = 861165636,
    D3DFMT_DXT5 = 894720068,
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102
} D3DFORMAT;

// Additional format constants
#define D3DFMT_R8G8B8 20
#define D3DFMT_L8 50
#define D3DFMT_A8 28
#define D3DFMT_P8 41
#define D3DFMT_A8R3G3B2 29
#define D3DFMT_X4R4G4B4 30
#define D3DFMT_A8P8 40
#define D3DFMT_A8L8 51
#define D3DFMT_A4L4 52

// Bump mapping formats
#define D3DFMT_V8U8 60
#define D3DFMT_L6V5U5 61
#define D3DFMT_X8L8V8U8 62
#define D3DFMT_Q8W8V8U8 63
#define D3DFMT_V16U16 64
#define D3DFMT_W11V11U10 65

// Additional compression formats
#define D3DFMT_DXT2 827611205
#define D3DFMT_DXT4 894720069

// Video formats
#define D3DFMT_UYVY 1498831189
#define D3DFMT_YUY2 844715353

// Depth/stencil formats
#define D3DFMT_D16_LOCKABLE 70
#define D3DFMT_D32 71
#define D3DFMT_D15S1 73
#define D3DFMT_D24S8 75
#define D3DFMT_D16 80
#define D3DFMT_D24X8 77
#define D3DFMT_D24X4S4 79

// DirectX lock flags
#define D3DLOCK_READONLY 0x00000010L
#define D3DLOCK_DISCARD 0x00002000L
#define D3DLOCK_NOOVERWRITE 0x00001000L
#define D3DLOCK_NOSYSLOCK 0x00000800L

// DirectX usage constants
#define D3DUSAGE_NPATCHES 0x01000000L
#define D3DUSAGE_SOFTWAREPROCESSING 0x00000010L

// DirectX raster capabilities
#define D3DPRASTERCAPS_ZBIAS 0x00004000L

// DirectX texture operation capabilities
#define D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR 0x00020000L

// DirectX primitive texture capabilities
#define D3DPTEXTURECAPS_CUBEMAP 0x00000800L

// DirectX primitive texture filter capabilities
#define D3DPTFILTERCAPS_MAGFANISOTROPIC 0x04000000L
#define D3DPTFILTERCAPS_MINFANISOTROPIC 0x00000400L

// DirectX blend modes
#define D3DBLEND_ZERO 1
#define D3DBLEND_ONE 2
#define D3DBLEND_SRCCOLOR 3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA 5
#define D3DBLEND_INVSRCALPHA 6
#define D3DBLEND_DESTALPHA 7
#define D3DBLEND_INVDESTALPHA 8
#define D3DBLEND_DESTCOLOR 9
#define D3DBLEND_INVDESTCOLOR 10
#define D3DBLEND_SRCALPHASAT 11
#define D3DBLEND_BOTHSRCALPHA 12
#define D3DBLEND_BOTHINVSRCALPHA 13

// DirectX texture coordinate index constants
#define D3DTSS_TCI_PASSTHRU 0x00000000L
#define D3DTSS_TCI_CAMERASPACENORMAL 0x00010000L
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x00020000L
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR 0x00030000L

// DirectX texture transform flags
#define D3DTTFF_DISABLE 0
#define D3DTTFF_COUNT1 1
#define D3DTTFF_COUNT2 2
#define D3DTTFF_COUNT3 3
#define D3DTTFF_COUNT4 4
#define D3DTTFF_PROJECTED 256

// DirectX fog modes
#define D3DFOG_NONE 0
#define D3DFOG_EXP 1
#define D3DFOG_EXP2 2
#define D3DFOG_LINEAR 3

// DirectX material color source
#define D3DMCS_MATERIAL 0
#define D3DMCS_COLOR1 1
#define D3DMCS_COLOR2 2

// DirectX device types
#define D3DDEVTYPE_HAL 1
#define D3DDEVTYPE_REF 2
#define D3DDEVTYPE_SW 3

// DirectX creation flags
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x00000020L
#define D3DCREATE_HARDWARE_VERTEXPROCESSING 0x00000040L
#define D3DCREATE_MIXED_VERTEXPROCESSING 0x00000080L
#define D3DCREATE_FPU_PRESERVE 0x00000002L

// DirectX enumeration flags
#define D3DENUM_NO_WHQL_LEVEL 0x00000002L

// DirectX error codes
#define D3DERR_DEVICELOST 0x88760868L
#define D3DERR_DEVICENOTRESET 0x88760869L

// DirectX display mode structure
typedef struct {
    DWORD Width;
    DWORD Height; 
    DWORD RefreshRate;
    D3DFORMAT Format;
} D3DDISPLAYMODE;

typedef enum {
    D3DPOOL_DEFAULT = 0,
    D3DPOOL_MANAGED = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH = 3
} D3DPOOL;

typedef enum {
    D3DUSAGE_RENDERTARGET = 0x00000001L,
    D3DUSAGE_DEPTHSTENCIL = 0x00000002L,
    D3DUSAGE_DYNAMIC = 0x00000200L,
    D3DUSAGE_WRITEONLY = 0x00000008L
} D3DUSAGE;

// DirectX FVF constants
#define D3DFVF_RESERVED0        0x001
#define D3DFVF_POSITION_MASK    0x00E
#define D3DFVF_XYZ              0x002
#define D3DFVF_XYZRHW           0x004
#define D3DFVF_XYZB1            0x006
#define D3DFVF_XYZB2            0x008
#define D3DFVF_XYZB3            0x00A
#define D3DFVF_XYZB4            0x00C
#define D3DFVF_XYZB5            0x00E
#define D3DFVF_NORMAL           0x010
#define D3DFVF_PSIZE            0x020
#define D3DFVF_DIFFUSE          0x040
#define D3DFVF_SPECULAR         0x080
#define D3DFVF_TEXCOUNT_MASK    0xF00
#define D3DFVF_TEXCOUNT_SHIFT   8
#define D3DFVF_TEX0             0x000
#define D3DFVF_TEX1             0x100
#define D3DFVF_TEX2             0x200
#define D3DFVF_TEX3             0x300
#define D3DFVF_TEX4             0x400
#define D3DFVF_TEX5             0x500
#define D3DFVF_TEX6             0x600
#define D3DFVF_TEX7             0x700
#define D3DFVF_TEX8             0x800

// Additional FVF texture coordinate size macros
#define D3DFVF_TEXTUREFORMAT1 0
#define D3DFVF_TEXTUREFORMAT2 1
#define D3DFVF_TEXTUREFORMAT3 2
#define D3DFVF_TEXTUREFORMAT4 3
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))

// Additional FVF constants
#define D3DFVF_LASTBETA_UBYTE4 0x1000L

// Device capability constants
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT 0x00000001L
#define D3DDEVCAPS_NPATCHES 0x01000000L

// Texture operation capability constants
#define D3DTEXOPCAPS_DOTPRODUCT3 0x00800000L
#define D3DTEXOPCAPS_BUMPENVMAP 0x00200000L
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE 0x00400000L

// Caps2 constants
#define D3DCAPS2_FULLSCREENGAMMA 0x00020000L

// Resource type constants
#define D3DRTYPE_TEXTURE 3

// DirectX constants
#define D3DDP_MAXTEXCOORD 8

// Transform state types
typedef enum {
    D3DTS_WORLD = 256,
    D3DTS_VIEW = 2,
    D3DTS_PROJECTION = 3,
    D3DTS_TEXTURE0 = 16,
    D3DTS_TEXTURE1 = 17,
    D3DTS_TEXTURE2 = 18,
    D3DTS_TEXTURE3 = 19,
    D3DTS_TEXTURE4 = 20,
    D3DTS_TEXTURE5 = 21,
    D3DTS_TEXTURE6 = 22,
    D3DTS_TEXTURE7 = 23
} D3DTRANSFORMSTATETYPE;

// Render state types
typedef enum {
    D3DRS_ZENABLE = 7,
    D3DRS_FILLMODE = 8,
    D3DRS_SHADEMODE = 9,
    D3DRS_ZWRITEENABLE = 14,
    D3DRS_ALPHATESTENABLE = 15,
    D3DRS_LASTPIXEL = 16,
    D3DRS_SRCBLEND = 19,
    D3DRS_DESTBLEND = 20,
    D3DRS_CULLMODE = 22,
    D3DRS_ZFUNC = 23,
    D3DRS_ALPHAREF = 24,
    D3DRS_ALPHAFUNC = 25,
    D3DRS_DITHERENABLE = 26,
    D3DRS_ALPHABLENDENABLE = 27,
    D3DRS_FOGENABLE = 28,
    D3DRS_SPECULARENABLE = 29,
    D3DRS_FOGCOLOR = 34,
    D3DRS_FOGTABLEMODE = 35,
    D3DRS_FOGSTART = 36,
    D3DRS_FOGEND = 37,
    D3DRS_FOGDENSITY = 38,
    D3DRS_RANGEFOGENABLE = 48,
    D3DRS_STENCILENABLE = 52,
    D3DRS_STENCILFAIL = 53,
    D3DRS_STENCILZFAIL = 54,
    D3DRS_STENCILPASS = 55,
    D3DRS_STENCILFUNC = 56,
    D3DRS_STENCILREF = 57,
    D3DRS_STENCILMASK = 58,
    D3DRS_STENCILWRITEMASK = 59,
    D3DRS_TEXTUREFACTOR = 60,
    D3DRS_WRAP0 = 128,
    D3DRS_LIGHTING = 137,
    D3DRS_AMBIENT = 139,
    D3DRS_FOGVERTEXMODE = 140,
    D3DRS_COLORVERTEX = 141,
    D3DRS_LOCALVIEWER = 142,
    D3DRS_NORMALIZENORMALS = 143,
    D3DRS_DIFFUSEMATERIALSOURCE = 145,
    D3DRS_SPECULARMATERIALSOURCE = 146,
    D3DRS_AMBIENTMATERIALSOURCE = 147,
    D3DRS_EMISSIVEMATERIALSOURCE = 148,
    D3DRS_VERTEXBLEND = 151,
    D3DRS_CLIPPLANEENABLE = 152,
    D3DRS_SOFTWAREVERTEXPROCESSING = 153,
    D3DRS_ZBIAS = 154
} D3DRENDERSTATETYPE;

// Texture stage state types
typedef enum {
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
    D3DTSS_BUMPENVLSCALE = 22,
    D3DTSS_BUMPENVLOFFSET = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,
    D3DTSS_COLORARG0 = 26,
    D3DTSS_ALPHAARG0 = 27,
    D3DTSS_RESULTARG = 28,
    D3DTSS_CONSTANT = 32
} D3DTEXTURESTAGESTATETYPE;

// DirectX structures
typedef struct {
    float x, y, z, rhw;
    DWORD color;
    float tu, tv;
} D3DTLVERTEX;

typedef struct {
    DWORD Width;
    DWORD Height;
    DWORD Levels;
    DWORD Usage;
    D3DFORMAT Format;
    D3DPOOL Pool;
} D3DSURFACE_DESC;

typedef struct {
    void* pBits;
    int Pitch;
} D3DLOCKED_RECT;

// Matrix structure from d3dx8math.h
// Basic matrix type
#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct {
    float m[4][4];
} D3DMATRIX;
#endif

// DirectX light structure
typedef struct {
    DWORD Type;
    float Diffuse[4];
    float Specular[4];
    float Ambient[4];
    float Position[3];
    float Direction[3];
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
} D3DLIGHT8;

// DirectX viewport structure
typedef struct {
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
} D3DVIEWPORT8;

// DirectX material structure
typedef struct {
    float Diffuse[4];
    float Ambient[4];
    float Specular[4];
    float Emissive[4];
    float Power;
} D3DMATERIAL8;

// DirectX caps structure (simplified)
typedef struct {
    DWORD DeviceType;
    DWORD AdapterOrdinal;
    DWORD Caps;
    DWORD Caps2;
    DWORD DevCaps;
    DWORD RasterCaps;
    DWORD TextureOpCaps;
    DWORD TextureCaps;
    DWORD TextureFilterCaps;
    DWORD MaxTextureWidth;
    DWORD MaxTextureHeight;
    DWORD MaxSimultaneousTextures;
    float MaxPointSize;
    DWORD VertexShaderVersion;
    DWORD PixelShaderVersion;
} D3DCAPS8;

// DirectX adapter identifier
typedef struct {
    char Driver[512];
    char Description[512];
    LARGE_INTEGER DriverVersion;
    DWORD VendorId;
    DWORD DeviceId;
    DWORD SubSysId;
    DWORD Revision;
    GUID DeviceIdentifier;
    DWORD WHQLLevel;
} D3DADAPTER_IDENTIFIER8;

// Forward declarations
struct IDirect3DBaseTexture8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DTexture8;

// Surface interface (declared early for use in IDirect3DDevice8)
struct IDirect3DSurface8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int LockRect(void* locked_rect, const RECT* rect, DWORD flags) { return D3D_OK; }
    virtual int UnlockRect() { return D3D_OK; }
    virtual int GetDesc(void* desc) { return D3D_OK; }
};

// DirectX interface types
struct IDirect3D8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int CheckDeviceFormat(DWORD adapter, DWORD device_type, DWORD adapter_format, DWORD usage, DWORD resource_type, DWORD check_format) { return D3D_OK; }
    virtual int GetDeviceCaps(DWORD adapter, DWORD device_type, D3DCAPS8* caps) { return D3D_OK; }
    virtual int GetAdapterIdentifier(DWORD adapter, DWORD flags, void* identifier) { return D3D_OK; }
    virtual int CreateDevice(DWORD adapter, DWORD device_type, void* focus_window, DWORD behavior_flags, void* presentation_parameters, IDirect3DDevice8** returned_device_interface) { return D3D_OK; }
    virtual DWORD GetAdapterCount() { return 1; }
    virtual DWORD GetAdapterModeCount(DWORD adapter) { return 1; }
    virtual int EnumAdapterModes(DWORD adapter, DWORD mode, D3DDISPLAYMODE* mode_desc) { return D3D_OK; }
};

struct IDirect3DDevice8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    
    // DirectX 8 Device methods - stub implementations
    virtual int LightEnable(DWORD index, BOOL enable) { return D3D_OK; }
    virtual int SetRenderState(D3DRENDERSTATETYPE state, DWORD value) { return D3D_OK; }
    virtual int SetTexture(DWORD stage, IDirect3DBaseTexture8* texture) { return D3D_OK; }
    virtual int SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) { return D3D_OK; }
    virtual int SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) { return D3D_OK; }
    virtual int GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX* matrix) { return D3D_OK; }
    virtual int SetMaterial(const D3DMATERIAL8* material) { return D3D_OK; }
    virtual int GetMaterial(D3DMATERIAL8* material) { return D3D_OK; }
    virtual int SetLight(DWORD index, const D3DLIGHT8* light) { return D3D_OK; }
    virtual int GetLight(DWORD index, D3DLIGHT8* light) { return D3D_OK; }
    virtual int SetViewport(const D3DVIEWPORT8* viewport) { return D3D_OK; }
    virtual int GetViewport(D3DVIEWPORT8* viewport) { return D3D_OK; }
    virtual int Clear(DWORD count, const void* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil) { return D3D_OK; }
    virtual int BeginScene() { return D3D_OK; }
    virtual int EndScene() { return D3D_OK; }
    virtual int Present(const void* src_rect, const void* dest_rect, void* dest_window_override, const void* dirty_region) { return D3D_OK; }
    virtual int SetVertexShader(DWORD handle) { return D3D_OK; }
    virtual int GetVertexShader(DWORD* handle) { return D3D_OK; }
    virtual int SetPixelShader(DWORD handle) { return D3D_OK; }
    virtual int GetPixelShader(DWORD* handle) { return D3D_OK; }
    virtual int DrawPrimitive(DWORD primitive_type, DWORD start_vertex, DWORD primitive_count) { return D3D_OK; }
    virtual int DrawIndexedPrimitive(DWORD type, DWORD min_index, DWORD num_vertices, DWORD start_index, DWORD primitive_count) { return D3D_OK; }
    virtual int CreateVertexBuffer(DWORD length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer8** vertex_buffer) { return D3D_OK; }
    virtual int CreateIndexBuffer(DWORD length, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer8** index_buffer) { return D3D_OK; }
    virtual int CreateTexture(DWORD width, DWORD height, DWORD levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8** texture) { return D3D_OK; }
    virtual int SetStreamSource(DWORD stream_number, IDirect3DVertexBuffer8* stream_data, DWORD stride) { return D3D_OK; }
    virtual int SetIndices(IDirect3DIndexBuffer8* index_data, DWORD base_vertex_index) { return D3D_OK; }
    virtual int GetDeviceCaps(D3DCAPS8* caps) { return D3D_OK; }
    
    // Additional methods for compatibility
    virtual int SetVertexShaderConstant(DWORD reg, const void* data, DWORD count) { return D3D_OK; }
    virtual int SetPixelShaderConstant(DWORD reg, const void* data, DWORD count) { return D3D_OK; }
    virtual int SetClipPlane(DWORD index, const float* plane) { return D3D_OK; }
    virtual int CopyRects(IDirect3DSurface8* source_surface, const RECT* source_rects, DWORD num_rects, IDirect3DSurface8* dest_surface, const POINT* dest_points) { return D3D_OK; }
    virtual int ValidateDevice(DWORD* num_passes) { return D3D_OK; }
    virtual int TestCooperativeLevel() { return D3D_OK; }
    virtual int Reset(void* presentation_parameters) { return D3D_OK; }
    virtual int ResourceManagerDiscardBytes(DWORD bytes) { return D3D_OK; }
};

struct IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
};

struct IDirect3DTexture8 : public IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int LockRect(DWORD level, void* locked_rect, const RECT* rect, DWORD flags) { return D3D_OK; }
    virtual int UnlockRect(DWORD level) { return D3D_OK; }
    virtual int GetLevelDesc(DWORD level, void* desc) { return D3D_OK; }
    virtual int GetSurfaceLevel(DWORD level, IDirect3DSurface8** surface) { return D3D_OK; }
};

struct IDirect3DVertexBuffer8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int Lock(DWORD offset, DWORD size, void** data, DWORD flags) { return D3D_OK; }
    virtual int Unlock() { return D3D_OK; }
    virtual int GetDesc(void* desc) { return D3D_OK; }
};

struct IDirect3DIndexBuffer8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int Lock(DWORD offset, DWORD size, void** data, DWORD flags) { return D3D_OK; }
    virtual int Unlock() { return D3D_OK; }
    virtual int GetDesc(void* desc) { return D3D_OK; }
};

// Stub functions
inline void* Direct3DCreate8(unsigned int) { return nullptr; }
inline DWORD D3DXGetFVFVertexSize(DWORD fvf) { 
    // Simple approximation - return a reasonable size
    return 64;  // Size for a typical vertex with position, normal, and texture coords
}

#else
// On Windows, include the real DirectX headers
#include <d3d8.h>
#endif // !_WIN32
