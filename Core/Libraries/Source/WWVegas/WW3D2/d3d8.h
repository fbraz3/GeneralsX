#pragma once

#ifndef _WIN32
#include "win32_compat.h"
#endif

#define D3DX_DEFAULT 0xFFFFFFFF
#define D3DX_FILTER_NONE 0x00000001
#define D3DX_FILTER_POINT 0x00000002  
#define D3DX_FILTER_LINEAR 0x00000003
#define D3DX_FILTER_TRIANGLE 0x00000004
#define D3DX_FILTER_BOX 0x00000005

// D3DLIGHT types
#define D3DLIGHT_POINT 1
#define D3DLIGHT_SPOT 2
#define D3DLIGHT_DIRECTIONAL 3

// D3DBACKBUFFER types
#define D3DBACKBUFFER_TYPE_MONO 0

// D3DCULL constants
#define D3DCULL_NONE 1
#define D3DCULL_CW 2
#define D3DCULL_CCW 3

// D3DSHADE constants  
#define D3DSHADE_FLAT 1
#define D3DSHADE_GOURAUD 2
#define D3DSHADE_PHONG 3

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
typedef void* LPDISPATCH;

// DirectX color type
typedef uint32_t D3DCOLOR;

// DirectX texture filter types
typedef enum {
    D3DTEXF_NONE = 0,
    D3DTEXF_POINT = 1,
    D3DTEXF_LINEAR = 2,
    D3DTEXF_ANISOTROPIC = 3,
    D3DTEXF_FLATCUBIC = 4,
    D3DTEXF_GAUSSIANCUBIC = 5
} D3DTEXTUREFILTERTYPE;

// DirectX Z-buffer types
typedef enum {
    D3DZB_FALSE = 0,
    D3DZB_TRUE = 1,
    D3DZB_USEW = 2
} D3DZBUFFERTYPE;

// DirectX comparison functions - use macros instead of enum
#ifndef D3DCMPFUNC_DEFINED
#define D3DCMPFUNC_DEFINED
typedef DWORD D3DCMPFUNC;
#endif

// DirectX stencil operations - use macros instead of enum
#ifndef D3DSTENCILOP_DEFINED
#define D3DSTENCILOP_DEFINED
typedef DWORD D3DSTENCILOP;
#endif

// DirectX vertex blend flags
#ifndef D3DVERTEXBLENDFLAGS_DEFINED
#define D3DVERTEXBLENDFLAGS_DEFINED
typedef enum {
    D3DVBF_DISABLE = 0,
    D3DVBF_1WEIGHTS = 1,
    D3DVBF_2WEIGHTS = 2,
    D3DVBF_3WEIGHTS = 3,
    D3DVBF_TWEENING = 255,
    D3DVBF_0WEIGHTS = 256
} D3DVERTEXBLENDFLAGS;
#endif

// DirectX patch edge flags
typedef enum {
    D3DPATCHEDGE_DISCRETE = 0,
    D3DPATCHEDGE_CONTINUOUS = 1
} D3DPATCHEDGESTYLE;

// DirectX debug monitor tokens
typedef enum {
    D3DDMT_ENABLE = 0,
    D3DDMT_DISABLE = 1
} D3DDEBUGMONITORTOKENS;

// DirectX blend operations
typedef enum {
    D3DBLENDOP_ADD = 1,
    D3DBLENDOP_SUBTRACT = 2,
    D3DBLENDOP_REVSUBTRACT = 3,
    D3DBLENDOP_MIN = 4,
    D3DBLENDOP_MAX = 5
} D3DBLENDOP;

// DirectX wrap flags
#define D3DWRAP_U 0x00000001L
#define D3DWRAP_V 0x00000002L
#define D3DWRAP_W 0x00000004L

// DirectX color write enable flags
#define D3DCOLORWRITEENABLE_RED 0x00000001L
#define D3DCOLORWRITEENABLE_GREEN 0x00000002L
#define D3DCOLORWRITEENABLE_BLUE 0x00000004L
#define D3DCOLORWRITEENABLE_ALPHA 0x00000008L

// DirectX multisample types
typedef DWORD D3DMULTISAMPLE_TYPE;
#define D3DMULTISAMPLE_NONE 0
#define D3DMULTISAMPLE_2_SAMPLES 2
#define D3DMULTISAMPLE_3_SAMPLES 3
#define D3DMULTISAMPLE_4_SAMPLES 4
#define D3DMULTISAMPLE_5_SAMPLES 5
#define D3DMULTISAMPLE_6_SAMPLES 6
#define D3DMULTISAMPLE_7_SAMPLES 7
#define D3DMULTISAMPLE_8_SAMPLES 8
#define D3DMULTISAMPLE_9_SAMPLES 9
#define D3DMULTISAMPLE_10_SAMPLES 10
#define D3DMULTISAMPLE_11_SAMPLES 11
#define D3DMULTISAMPLE_12_SAMPLES 12
#define D3DMULTISAMPLE_13_SAMPLES 13
#define D3DMULTISAMPLE_14_SAMPLES 14
#define D3DMULTISAMPLE_15_SAMPLES 15
#define D3DMULTISAMPLE_16_SAMPLES 16

// DirectX swap effects
typedef DWORD D3DSWAPEFFECT;
#define D3DSWAPEFFECT_DISCARD 1
#define D3DSWAPEFFECT_FLIP 2
#define D3DSWAPEFFECT_COPY 3
#define D3DSWAPEFFECT_COPY_VSYNC 4

// D3DFORMAT is defined in win32_compat.h
// D3DPRESENT_PARAMETERS is defined in win32_compat.h

// Constants
#define D3D_OK 0x00000000L
#define CONST const

// Device types
#define D3DDEVTYPE_HAL 1
#define D3DDEVTYPE_REF 2
#define D3DDEVTYPE_SW 3

// Multisample types
#define D3DMULTISAMPLE_NONE 0
#define D3DMULTISAMPLE_2_SAMPLES 2
#define D3DMULTISAMPLE_4_SAMPLES 4
#define D3DMULTISAMPLE_8_SAMPLES 8
#define D3DMULTISAMPLE_16_SAMPLES 16

// Present intervals
#define D3DPRESENT_INTERVAL_DEFAULT 0
#define D3DPRESENT_INTERVAL_IMMEDIATE 0x80000000L
#define D3DPRESENT_INTERVAL_ONE 1
#define D3DPRESENT_INTERVAL_TWO 2
#define D3DPRESENT_INTERVAL_THREE 3
#define D3DPRESENT_INTERVAL_FOUR 4

// Present rates
#define D3DPRESENT_RATE_DEFAULT 0

// DirectX adapter constants
#define D3DADAPTER_DEFAULT 0

// DirectX resource types
#define D3DRTYPE_SURFACE 1
#define D3DRTYPE_VOLUME 2
#define D3DRTYPE_TEXTURE 3
#define D3DRTYPE_VOLUMETEXTURE 4
#define D3DRTYPE_CUBETEXTURE 5
#define D3DRTYPE_VERTEXBUFFER 6
#define D3DRTYPE_INDEXBUFFER 7

// DirectX clear flags
#define D3DCLEAR_TARGET 0x00000001L
#define D3DCLEAR_ZBUFFER 0x00000002L
#define D3DCLEAR_STENCIL 0x00000004L

// DirectX primitive types
#define D3DPT_POINTLIST 1
#define D3DPT_LINELIST 2
#define D3DPT_LINESTRIP 3
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN 6

// DirectX error codes
#define D3DERR_CONFLICTINGTEXTUREFILTER 0x88760826L
#define D3DERR_CONFLICTINGTEXTUREPALETTE 0x88760827L
#define D3DERR_UNSUPPORTEDTEXTUREFILTER 0x88760828L
#define D3DERR_TOOMANYOPERATIONS 0x88760829L
#define D3DERR_UNSUPPORTEDALPHAARG 0x8876082AL
#define D3DERR_UNSUPPORTEDALPHAOPERATION 0x8876082BL
#define D3DERR_UNSUPPORTEDCOLORARG 0x8876082CL
#define D3DERR_UNSUPPORTEDCOLOROPERATION 0x8876082DL
#define D3DERR_UNSUPPORTEDFACTORVALUE 0x8876082EL
#define D3DERR_WRONGTEXTUREFORMAT 0x8876082FL
#define D3DERR_NOTAVAILABLE 0x88760866L
#define D3DERR_OUTOFVIDEOMEMORY 0x8876017CL

// DirectX primitive type
typedef DWORD D3DPRIMITIVETYPE;

// DirectX enums and constants

// Conversion helpers for DWORD <-> D3DFORMAT compatibility
#define DWORD_TO_D3DFORMAT(x) ((D3DFORMAT)(x))
#define D3DFORMAT_TO_DWORD(x) ((DWORD)(x))

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

// Fill modes
#define D3DFILL_POINT 1
#define D3DFILL_WIREFRAME 2
#define D3DFILL_SOLID 3

// Texture arguments (D3DTA_*)
#define D3DTA_DIFFUSE 0x00000000
#define D3DTA_CURRENT 0x00000001
#define D3DTA_TEXTURE 0x00000002
#define D3DTA_TFACTOR 0x00000003
#define D3DTA_SPECULAR 0x00000004
#define D3DTA_TEMP 0x00000005
#define D3DTA_COMPLEMENT 0x00000010
#define D3DTA_ALPHAREPLICATE 0x00000020
#define D3DTA_SELECTMASK 0x0000000f

// Texture address modes
#define D3DTADDRESS_WRAP 1
#define D3DTADDRESS_MIRROR 2
#define D3DTADDRESS_CLAMP 3
#define D3DTADDRESS_BORDER 4
#define D3DTADDRESS_MIRRORONCE 5

// Set Gamma Ramp flags
#define D3DSGR_NO_CALIBRATION 0x00000000
#define D3DSGR_CALIBRATE 0x00000001

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
#define D3DPTFILTERCAPS_MAGFLINEAR 0x02000000L
#define D3DPTFILTERCAPS_MINFLINEAR 0x00000200L
#define D3DPTFILTERCAPS_MIPFLINEAR 0x00020000L

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

#ifndef D3DPOOL_DEFINED
#define D3DPOOL_DEFINED
typedef enum {
    D3DPOOL_DEFAULT = 0,
    D3DPOOL_MANAGED = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH = 3
} D3DPOOL;
#endif

// D3DBLEND constants
typedef DWORD D3DBLEND;
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

// D3DTEXTUREOP constants
typedef DWORD D3DTEXTUREOP;
#define D3DTOP_DISABLE 1
#define D3DTOP_SELECTARG1 2
#define D3DTOP_SELECTARG2 3
#define D3DTOP_MODULATE 4
#define D3DTOP_MODULATE2X 5
#define D3DTOP_MODULATE4X 6
#define D3DTOP_ADD 7
#define D3DTOP_ADDSIGNED 8
#define D3DTOP_ADDSIGNED2X 9
#define D3DTOP_SUBTRACT 10
#define D3DTOP_ADDSMOOTH 11
#define D3DTOP_BLENDDIFFUSEALPHA 12
#define D3DTOP_BLENDTEXTUREALPHA 13
#define D3DTOP_BLENDFACTORALPHA 14
#define D3DTOP_BLENDTEXTUREALPHAPM 15
#define D3DTOP_BLENDCURRENTALPHA 16
#define D3DTOP_PREMODULATE 17
#define D3DTOP_MODULATEALPHA_ADDCOLOR 18
#define D3DTOP_MODULATECOLOR_ADDALPHA 19
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR 20
#define D3DTOP_MODULATEINVCOLOR_ADDALPHA 21
#define D3DTOP_BUMPENVMAP 22
#define D3DTOP_BUMPENVMAPLUMINANCE 23
#define D3DTOP_DOTPRODUCT3 24
#define D3DTOP_MULTIPLYADD 25
#define D3DTOP_LERP 26

#ifndef D3DUSAGE_DEFINED
#define D3DUSAGE_DEFINED
typedef enum {
    D3DUSAGE_RENDERTARGET = 0x00000001L,
    D3DUSAGE_DEPTHSTENCIL = 0x00000002L,
    D3DUSAGE_DYNAMIC = 0x00000200L,
    D3DUSAGE_WRITEONLY = 0x00000008L
} D3DUSAGE;
#endif

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
#ifndef D3DDEVCAPS_NPATCHES_DEFINED
#define D3DDEVCAPS_NPATCHES_DEFINED
#define D3DDEVCAPS_NPATCHES 0x02000000L
#endif

// Caps2 constants
#define D3DCAPS2_FULLSCREENGAMMA 0x00020000L

// Resource type constants
#define D3DRTYPE_TEXTURE 3

// DirectX constants
#define D3DDP_MAXTEXCOORD 8

// Transform state types
#ifndef D3DTRANSFORMSTATETYPE_DEFINED
#define D3DTRANSFORMSTATETYPE_DEFINED
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
#endif

// Render state types
#ifndef D3DRENDERSTATETYPE_DEFINED
#define D3DRENDERSTATETYPE_DEFINED
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
    D3DRS_ZBIAS = 47,
    D3DRS_STENCILPASS = 55,
    D3DRS_STENCILFUNC = 56,
    D3DRS_STENCILREF = 57,
    D3DRS_STENCILMASK = 58,
    D3DRS_STENCILWRITEMASK = 59,
    D3DRS_TEXTUREFACTOR = 60,
    D3DRS_WRAP0 = 128,
    D3DRS_CLIPPING = 136,
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
    
    // Additional DirectX 8 constants
    D3DRS_LINEPATTERN = 10,
    D3DRS_ZVISIBLE = 30,
    D3DRS_EDGEANTIALIAS = 40,
    D3DRS_WRAP1 = 130,
    D3DRS_WRAP2 = 131,
    D3DRS_WRAP3 = 132,
    D3DRS_WRAP4 = 133,
    D3DRS_WRAP5 = 134,
    D3DRS_WRAP6 = 135,
    D3DRS_WRAP7 = 138,
    D3DRS_POINTSIZE = 154,
    D3DRS_POINTSIZE_MIN = 155,
    D3DRS_POINTSPRITEENABLE = 156,
    D3DRS_POINTSCALEENABLE = 157,
    D3DRS_POINTSCALE_A = 158,
    D3DRS_POINTSCALE_B = 159,
    D3DRS_POINTSCALE_C = 160,
    D3DRS_MULTISAMPLEANTIALIAS = 161,
    D3DRS_MULTISAMPLEMASK = 162,
    D3DRS_PATCHEDGESTYLE = 163,
    D3DRS_PATCHSEGMENTS = 164,
    D3DRS_DEBUGMONITORTOKEN = 165,
    D3DRS_POINTSIZE_MAX = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE = 167,
    D3DRS_COLORWRITEENABLE = 168,
    D3DRS_TWEENFACTOR = 170,
    D3DRS_BLENDOP = 171
} D3DRENDERSTATETYPE;
#endif

// Texture stage state types
#ifndef D3DTEXTURESTAGESTATETYPE_DEFINED
#define D3DTEXTURESTAGESTATETYPE_DEFINED
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
    D3DTSS_CONSTANT = 32,
    
    // Additional texture stage states
    D3DTSS_ADDRESSU = 13,
    D3DTSS_ADDRESSV = 14,
    D3DTSS_BORDERCOLOR = 15,
    D3DTSS_MAGFILTER = 16,
    D3DTSS_MINFILTER = 17,
    D3DTSS_MIPFILTER = 18,
    D3DTSS_MIPMAPLODBIAS = 19,
    D3DTSS_MAXMIPLEVEL = 20,
    D3DTSS_MAXANISOTROPY = 21,
    D3DTSS_ADDRESSW = 25
} D3DTEXTURESTAGESTATETYPE;
#endif

// DirectX structures
#ifndef D3DTLVERTEX_DEFINED
#define D3DTLVERTEX_DEFINED
typedef struct {
    float x, y, z, rhw;
    DWORD color;
    float tu, tv;
} D3DTLVERTEX;
#endif

#ifndef D3DSURFACE_DESC_DEFINED
#define D3DSURFACE_DESC_DEFINED
typedef struct {
    DWORD Width;
    DWORD Height;
    DWORD Levels;
    DWORD Usage;
    D3DFORMAT Format;
    D3DPOOL Pool;
    DWORD Size;
} D3DSURFACE_DESC;
#endif

// DirectX volume description structure
#ifndef D3DVOLUME_DESC_DEFINED
#define D3DVOLUME_DESC_DEFINED
typedef struct {
    D3DFORMAT Format;
    DWORD Type;
    DWORD Usage;
    D3DPOOL Pool;
    DWORD Size;
    DWORD Width;
    DWORD Height;
    DWORD Depth;
} D3DVOLUME_DESC;
#endif

#ifndef D3DLOCKED_RECT_DEFINED
#define D3DLOCKED_RECT_DEFINED
typedef struct {
    void* pBits;
    int Pitch;
} D3DLOCKED_RECT;
#endif

// Matrix structure from d3dx8math.h
// Basic matrix type
#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct {
    float m[4][4];
} D3DMATRIX;
#endif

// DirectX gamma ramp structure
#ifndef D3DGAMMARAMP_DEFINED
#define D3DGAMMARAMP_DEFINED
typedef struct {
    WORD red[256];
    WORD green[256];
    WORD blue[256];
} D3DGAMMARAMP;
#endif

// Color structure for D3D
#ifndef D3DCOLORVALUE_DEFINED
#define D3DCOLORVALUE_DEFINED
typedef struct {
    float r, g, b, a;
} D3DCOLORVALUE;
#endif

// DirectX light structure
#ifndef D3DLIGHT8_DEFINED
#define D3DLIGHT8_DEFINED
typedef struct {
    DWORD Type;
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
} D3DLIGHT8;
#endif

// DirectX viewport structure
#ifndef D3DVIEWPORT8_DEFINED
#define D3DVIEWPORT8_DEFINED
typedef struct {
    DWORD X;
    DWORD Y;
    DWORD Width;
    DWORD Height;
    float MinZ;
    float MaxZ;
} D3DVIEWPORT8;
#endif

// DirectX material structure
#ifndef D3DMATERIAL8_DEFINED
#define D3DMATERIAL8_DEFINED
typedef struct {
    float Diffuse[4];
    float Ambient[4];
    float Specular[4];
    float Emissive[4];
    float Power;
} D3DMATERIAL8;
#endif

// DirectX caps structure (simplified)
#ifndef D3DCAPS8_DEFINED
#define D3DCAPS8_DEFINED
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
    DWORD MaxVolumeExtent;
    DWORD MaxSimultaneousTextures;
    float MaxPointSize;
    DWORD VertexShaderVersion;
    DWORD PixelShaderVersion;
} D3DCAPS8;
#endif

// DirectX adapter identifier
#ifndef D3DADAPTER_IDENTIFIER8_DEFINED
#define D3DADAPTER_IDENTIFIER8_DEFINED
typedef struct {
    char Driver[512];
    char Description[512];
    DWORD VendorId;
    DWORD DeviceId;
    DWORD SubSysId;
    DWORD Revision;
    DWORD WHQLLevel;
} D3DADAPTER_IDENTIFIER8;
#endif

// Forward declarations
struct IDirect3DBaseTexture8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DTexture8;

// Surface interface (declared early for use in IDirect3DDevice8)
#ifndef IDIRECT3DSURFACE8_DEFINED
#define IDIRECT3DSURFACE8_DEFINED
struct IDirect3DSurface8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int LockRect(void* locked_rect, const RECT* rect, DWORD flags) { return D3D_OK; }
    virtual int UnlockRect() { return D3D_OK; }
    virtual int GetDesc(void* desc) { return D3D_OK; }
};
#endif

// DirectX interface types
#ifndef IDIRECT3D8_DEFINED
#define IDIRECT3D8_DEFINED
struct IDirect3D8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int CheckDeviceFormat(DWORD adapter, DWORD device_type, D3DFORMAT adapter_format, DWORD usage, DWORD resource_type, D3DFORMAT check_format) { return D3D_OK; }
    virtual int GetDeviceCaps(DWORD adapter, DWORD device_type, D3DCAPS8* caps) { return D3D_OK; }
    virtual int GetAdapterIdentifier(DWORD adapter, DWORD flags, void* identifier) { return D3D_OK; }
    virtual int CreateDevice(DWORD adapter, DWORD device_type, void* focus_window, DWORD behavior_flags, void* presentation_parameters, IDirect3DDevice8** returned_device_interface) { return D3D_OK; }
    virtual DWORD GetAdapterCount() { return 1; }
    virtual DWORD GetAdapterModeCount(DWORD adapter) { return 1; }
    virtual int EnumAdapterModes(DWORD adapter, DWORD mode, D3DDISPLAYMODE* mode_desc) { return D3D_OK; }
    virtual int GetAdapterDisplayMode(DWORD adapter, D3DDISPLAYMODE* mode) { 
        if (mode) {
            mode->Width = 1920;
            mode->Height = 1080;
            mode->RefreshRate = 60;
            mode->Format = D3DFMT_X8R8G8B8;
        }
        return D3D_OK; 
    }
    virtual int CheckDeviceType(DWORD adapter, DWORD device_type, D3DFORMAT adapter_format, D3DFORMAT back_buffer_format, BOOL windowed) { return D3D_OK; }
    virtual int CheckDepthStencilMatch(DWORD adapter, DWORD device_type, D3DFORMAT adapter_format, D3DFORMAT render_target_format, D3DFORMAT depth_stencil_format) { return D3D_OK; }
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
    virtual int CreateImageSurface(DWORD width, DWORD height, D3DFORMAT format, IDirect3DSurface8** surface) { return D3D_OK; }
    virtual int UpdateTexture(IDirect3DBaseTexture8* source_texture, IDirect3DBaseTexture8* dest_texture) { return D3D_OK; }
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
    virtual int GetDepthStencilSurface(IDirect3DSurface8** depth_stencil_surface) { 
        if (depth_stencil_surface) *depth_stencil_surface = nullptr;
        return D3D_OK; 
    }
    virtual int GetDisplayMode(void* mode) { return D3D_OK; }
    virtual int GetFrontBuffer(IDirect3DSurface8* dest_surface) { return D3D_OK; }
    virtual int GetBackBuffer(DWORD back_buffer, DWORD type, IDirect3DSurface8** back_buffer_surface) { return D3D_OK; }
    virtual int SetRenderTarget(IDirect3DSurface8* render_target, IDirect3DSurface8* new_z_stencil) { return D3D_OK; }
    virtual int GetRenderTarget(IDirect3DSurface8** render_target) { return D3D_OK; }
    virtual int CreateAdditionalSwapChain(void* presentation_parameters, void** swap_chain) { return D3D_OK; }
    virtual DWORD GetAvailableTextureMem() { return 0; }
    virtual int SetGammaRamp(DWORD flags, const D3DGAMMARAMP* ramp) { return D3D_OK; }
};

struct IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual DWORD GetPriority() { return 0; }
    virtual DWORD SetPriority(DWORD priority) { return 0; }
    virtual DWORD GetLevelCount() { return 1; }
};

struct IDirect3DTexture8 : public IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int LockRect(DWORD level, void* locked_rect, const RECT* rect, DWORD flags) { return D3D_OK; }
    virtual int UnlockRect(DWORD level) { return D3D_OK; }
    virtual int GetLevelDesc(DWORD level, void* desc) { return D3D_OK; }
    virtual int GetSurfaceLevel(DWORD level, IDirect3DSurface8** surface) { return D3D_OK; }
};

struct IDirect3DCubeTexture8 : public IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int LockRect(DWORD face, DWORD level, void* locked_rect, const RECT* rect, DWORD flags) { return D3D_OK; }
    virtual int UnlockRect(DWORD face, DWORD level) { return D3D_OK; }
    virtual int GetLevelDesc(DWORD level, void* desc) { return D3D_OK; }
    virtual int GetCubeMapSurface(DWORD face, DWORD level, IDirect3DSurface8** surface) { return D3D_OK; }
};

struct IDirect3DVolumeTexture8 : public IDirect3DBaseTexture8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int LockBox(DWORD level, void* locked_box, const void* box, DWORD flags) { return D3D_OK; }
    virtual int UnlockBox(DWORD level) { return D3D_OK; }
    virtual int GetLevelDesc(DWORD level, void* desc) { return D3D_OK; }
    virtual int GetVolumeLevel(DWORD level, void** volume) { return D3D_OK; }
};

struct IDirect3DSwapChain8 {
    virtual int AddRef() { return 1; }
    virtual int Release() { return 0; }
    virtual int QueryInterface(void*, void**) { return 0; }
    virtual int Present(const RECT* source_rect, const RECT* dest_rect, void* dest_window_override, const void* dirty_region) { return D3D_OK; }
    virtual int GetBackBuffer(DWORD back_buffer, DWORD type, IDirect3DSurface8** back_buffer_surface) { 
        if (back_buffer_surface) *back_buffer_surface = nullptr;
        return D3D_OK; 
    }
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
inline int D3DXCreateTexture(void* device, DWORD width, DWORD height, DWORD miplevels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8** texture) {
    if (texture) *texture = nullptr;
    return D3D_OK;
}
inline int D3DXCreateCubeTexture(void* device, DWORD edgelength, DWORD miplevels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DCubeTexture8** cube_texture) {
    if (cube_texture) *cube_texture = nullptr;
    return D3D_OK;
}
inline int D3DXCreateVolumeTexture(void* device, DWORD width, DWORD height, DWORD depth, DWORD miplevels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DVolumeTexture8** volume_texture) {
    if (volume_texture) *volume_texture = nullptr;
    return D3D_OK;
}
inline int D3DXLoadSurfaceFromSurface(void* dest_surface, void* dest_palette, void* dest_rect, void* src_surface, void* src_palette, void* src_rect, DWORD filter, DWORD color_key) {
    return D3D_OK;
}
inline int D3DXFilterTexture(void* texture, void* palette, DWORD src_level, DWORD filter) {
    return D3D_OK;
}
inline int D3DXCreateTextureFromFileEx(void* device, const char* src_file, DWORD width, DWORD height, DWORD mip_levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, DWORD filter, DWORD mip_filter, DWORD color_key, void* src_info, void* palette, IDirect3DTexture8** texture) {
    if (texture) *texture = nullptr;
    return D3D_OK;
}
inline int D3DXCreateTextureFromFileExA(void* device, const char* src_file, DWORD width, DWORD height, DWORD mip_levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, DWORD filter, DWORD mip_filter, DWORD color_key, void* src_info, void* palette, IDirect3DTexture8** texture) {
    if (texture) *texture = nullptr;
    return D3D_OK;
}

#else
// On Windows, include the real DirectX headers
#include <d3d8.h>
#endif // !_WIN32
