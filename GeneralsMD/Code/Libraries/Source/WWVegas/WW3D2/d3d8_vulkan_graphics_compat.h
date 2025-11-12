#pragma once

/**
 * @file d3d8_vulkan_graphics_compat.h
 * @brief DirectX 8.0 API definitions and interface stubs for cross-platform Vulkan backend
 * 
 * This header provides DirectX 8 interface definitions and constants needed
 * for cross-platform compilation without the DirectX SDK. 
 * 
 * On Windows, the real directx/d3d8.h is used.
 * On macOS/Linux, we provide mock interfaces backed by memory buffers that
 * can be later intercepted for Vulkan rendering via Metal/OpenGL backends.
 * 
 * Phase 01: DirectX 8 Compatibility Layer (renamed Phase 2.7)
 * Acceptance Criteria: AC3 - All DirectX 8 enumerations and types defined
 */

#ifndef D3D8_VULKAN_GRAPHICS_COMPAT_H_INCLUDED
#define D3D8_VULKAN_GRAPHICS_COMPAT_H_INCLUDED

#ifndef _WIN32
#include "win32_sdl_types_compat.h"
#include "d3d8_vulkan_types_compat.h"  // For IID and RGNDATA
#endif

// Note: d3d8_enums.h is not included here to avoid redefinition conflicts
// with existing enum definitions in this file. Enums are defined locally below.

// =====================================================================
// Forward Declarations
// =====================================================================

struct CORE_IDirect3D8;
struct CORE_IDirect3DDevice8;
struct CORE_IDirect3DTexture8;
struct CORE_IDirect3DSurface8;
struct CORE_IDirect3DVertexBuffer8;
struct CORE_IDirect3DIndexBuffer8;
struct CORE_IDirect3DBaseTexture8;
struct CORE_IDirect3DSwapChain8;
struct CORE_IDirect3DCubeTexture8;
struct CORE_IDirect3DVolumeTexture8;

// =====================================================================
// DirectX Basic Types
// =====================================================================

/** @brief DirectX color type (ARGB: 0xAARRGGBB) */
typedef uint32_t D3DCOLOR;

/** @brief DirectX rectangle type */
typedef struct {
    long x1;
    long y1;
    long x2;
    long y2;
} D3DRECT;

typedef D3DRECT* LPD3DRECT;

// =====================================================================
// DirectX Interface Pointer Types
// =====================================================================

typedef void* LPDIRECT3D8;
typedef void* LPDIRECT3DDEVICE8;
typedef void* LPDIRECT3DTEXTURE8;
typedef void* LPDIRECT3DSURFACE8;
typedef void* LPDIRECT3DVERTEXBUFFER8;
typedef void* LPDIRECT3DINDEXBUFFER8;
typedef void* LPDIRECT3DSWAPCHAIN8;
typedef void* LPDIRECT3DBASETEXTURE8;
typedef void* LPDIRECT3DCUBETEXTURE8;
typedef void* LPDIRECT3DVOLUMETEXTURE8;

// =====================================================================
// DirectX Constants
// =====================================================================

#define D3D_OK                        0x00000000L
#define D3DADAPTER_DEFAULT            0

// Resource types
#define D3DRTYPE_SURFACE              1
#define D3DRTYPE_VOLUME               2
#define D3DRTYPE_TEXTURE              3
#define D3DRTYPE_VOLUMETEXTURE        4
#define D3DRTYPE_CUBETEXTURE          5
#define D3DRTYPE_VERTEXBUFFER         6
#define D3DRTYPE_INDEXBUFFER          7

// Device types
#define D3DDEVTYPE_HAL                1
#define D3DDEVTYPE_REF                2
#define D3DDEVTYPE_SW                 3

// Adapter mode count (special value)
#define D3DPRESENT_RATE_DEFAULT       0

// =====================================================================
// DirectX Enumerations (moved to d3d8_enums.h - included above)
// =====================================================================
// All enum definitions are now in d3d8_enums.h for organization

// Keep only the most commonly used #defines for backward compatibility


// =====================================================================
// DirectX Texture Argument Constants
// =====================================================================

#define D3DTA_DIFFUSE                 0x00000000
#define D3DTA_CURRENT                 0x00000001
#define D3DTA_TEXTURE                 0x00000002
#define D3DTA_TFACTOR                 0x00000003
#define D3DTA_SPECULAR                0x00000004
#define D3DTA_TEMP                    0x00000005
#define D3DTA_COMPLEMENT              0x00000010
#define D3DTA_ALPHAREPLICATE          0x00000020
#define D3DTA_SELECTMASK              0x0000000f

// =====================================================================
// DirectX Texture Operations
// =====================================================================

/** @brief DirectX texture stage operations */
#define D3DTOP_DISABLE                1
#define D3DTOP_SELECTARG1             2
#define D3DTOP_SELECTARG2             3
#define D3DTOP_MODULATE               4
#define D3DTOP_MODULATE2X             5
#define D3DTOP_MODULATE4X             6
#define D3DTOP_ADD                    7
#define D3DTOP_ADDSIGNED              8
#define D3DTOP_ADDSIGNED2X            9
#define D3DTOP_SUBTRACT               10
#define D3DTOP_ADDSMOOTH              11
#define D3DTOP_BLENDDIFFUSEALPHA      12
#define D3DTOP_BLENDTEXTUREALPHA      13
#define D3DTOP_BLENDFACTORALPHA       14
#define D3DTOP_BLENDTEXTUREALPHAPM    15
#define D3DTOP_BLENDCURRENTALPHA      16
#define D3DTOP_PREMODULATE            17
#define D3DTOP_MODULATEALPHA_ADDCOLOR 18
#define D3DTOP_MODULATECOLOR_ADDALPHA 19
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR 20
#define D3DTOP_MODULATEINVCOLOR_ADDALPHA 21
#define D3DTOP_BUMPENVMAP             22
#define D3DTOP_BUMPENVMAPLUMINANCE    23
#define D3DTOP_DOTPRODUCT3            24
#define D3DTOP_MULTIPLYADD            25
#define D3DTOP_LERP                   26

// =====================================================================
// DirectX Clear Flags
// =====================================================================

#define D3DCLEAR_TARGET               0x00000001L
#define D3DCLEAR_ZBUFFER              0x00000002L
#define D3DCLEAR_STENCIL              0x00000004L

// =====================================================================
// DirectX Lock Flags
// =====================================================================

#define D3DLOCK_READONLY              0x00000010L
#define D3DLOCK_DISCARD               0x00002000L
#define D3DLOCK_NOOVERWRITE           0x00001000L
#define D3DLOCK_NOSYSLOCK             0x00000800L
#define D3DLOCK_NO_DIRTY_UPDATE       0x00008000L

// =====================================================================
// DirectX Usage Flags
// =====================================================================

#define D3DUSAGE_RENDERTARGET         0x00000001L
#define D3DUSAGE_DEPTHSTENCIL         0x00000002L
#define D3DUSAGE_DYNAMIC              0x00000200L
#define D3DUSAGE_WRITEONLY            0x00000008L
#define D3DUSAGE_SOFTWAREPROCESSING   0x00000010L
#define D3DUSAGE_NPATCHES             0x01000000L
#define D3DUSAGE_POINTS               0x00000040L

// =====================================================================
// DirectX Pool Types
// =====================================================================

#ifndef D3DPOOL_DEFINED
#define D3DPOOL_DEFINED
typedef enum {
    D3DPOOL_DEFAULT   = 0,
    D3DPOOL_MANAGED   = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH   = 3
} D3DPOOL;
#endif

// =====================================================================
// DirectX Color Write Enable Flags
// =====================================================================

#define D3DCOLORWRITEENABLE_RED       0x00000001L
#define D3DCOLORWRITEENABLE_GREEN     0x00000002L
#define D3DCOLORWRITEENABLE_BLUE      0x00000004L
#define D3DCOLORWRITEENABLE_ALPHA     0x00000008L

// =====================================================================
// DirectX Wrap Flags
// =====================================================================

#define D3DWRAP_U                     0x00000001L
#define D3DWRAP_V                     0x00000002L
#define D3DWRAP_W                     0x00000004L

// =====================================================================
// DirectX Multisample Types
// =====================================================================

typedef DWORD D3DMULTISAMPLE_TYPE;
#define D3DMULTISAMPLE_NONE           0
#define D3DMULTISAMPLE_2_SAMPLES      2
#define D3DMULTISAMPLE_4_SAMPLES      4
#define D3DMULTISAMPLE_8_SAMPLES      8
#define D3DMULTISAMPLE_16_SAMPLES     16

// =====================================================================
// DirectX Swap Effects
// =====================================================================

typedef DWORD D3DSWAPEFFECT;
#define D3DSWAPEFFECT_DISCARD         1
#define D3DSWAPEFFECT_FLIP            2
#define D3DSWAPEFFECT_COPY            3
#define D3DSWAPEFFECT_COPY_VSYNC      4

// =====================================================================
// DirectX Present Intervals
// =====================================================================

#define D3DPRESENT_INTERVAL_DEFAULT   0
#define D3DPRESENT_INTERVAL_IMMEDIATE 0x80000000L
#define D3DPRESENT_INTERVAL_ONE       1
#define D3DPRESENT_INTERVAL_TWO       2
#define D3DPRESENT_INTERVAL_THREE     3
#define D3DPRESENT_INTERVAL_FOUR      4

// =====================================================================
// DirectX Render State Types
// =====================================================================

/** @brief DirectX render state enumeration */
typedef enum {
    D3DRS_ZENABLE                   = 7,
    D3DRS_FILLMODE                  = 8,
    D3DRS_SHADEMODE                 = 9,
    D3DRS_LINEPATTERN               = 10,
    D3DRS_ZWRITEENABLE              = 14,
    D3DRS_ALPHATESTENABLE           = 15,
    D3DRS_LASTPIXEL                 = 16,
    D3DRS_SRCBLEND                  = 19,
    D3DRS_DESTBLEND                 = 20,
    D3DRS_CULLMODE                  = 22,
    D3DRS_ZFUNC                     = 23,
    D3DRS_ALPHAREF                  = 24,
    D3DRS_ALPHAFUNC                 = 25,
    D3DRS_DITHERENABLE              = 26,
    D3DRS_ALPHABLENDENABLE          = 27,
    D3DRS_FOGENABLE                 = 28,
    D3DRS_SPECULARENABLE            = 29,
    D3DRS_ZVISIBLE                  = 30,
    D3DRS_FOGCOLOR                  = 34,
    D3DRS_FOGTABLEMODE              = 35,
    D3DRS_FOGSTART                  = 36,
    D3DRS_FOGEND                    = 37,
    D3DRS_FOGDENSITY                = 38,
    D3DRS_EDGEANTIALIAS             = 40,
    D3DRS_ZBIAS                     = 47,
    D3DRS_RANGEFOGENABLE            = 48,
    D3DRS_STENCILENABLE             = 52,
    D3DRS_STENCILFAIL               = 53,
    D3DRS_STENCILZFAIL              = 54,
    D3DRS_STENCILPASS               = 55,
    D3DRS_STENCILFUNC               = 56,
    D3DRS_STENCILREF                = 57,
    D3DRS_STENCILMASK               = 58,
    D3DRS_STENCILWRITEMASK          = 59,
    D3DRS_TEXTUREFACTOR             = 60,
    D3DRS_WRAP0                     = 128,
    D3DRS_WRAP1                     = 130,
    D3DRS_WRAP2                     = 131,
    D3DRS_WRAP3                     = 132,
    D3DRS_WRAP4                     = 133,
    D3DRS_WRAP5                     = 134,
    D3DRS_WRAP6                     = 135,
    D3DRS_WRAP7                     = 138,
    D3DRS_CLIPPING                  = 136,
    D3DRS_LIGHTING                  = 137,
    D3DRS_AMBIENT                   = 139,
    D3DRS_FOGVERTEXMODE             = 140,
    D3DRS_COLORVERTEX               = 141,
    D3DRS_LOCALVIEWER               = 142,
    D3DRS_NORMALIZENORMALS          = 143,
    D3DRS_DIFFUSEMATERIALSOURCE     = 145,
    D3DRS_SPECULARMATERIALSOURCE    = 146,
    D3DRS_AMBIENTMATERIALSOURCE     = 147,
    D3DRS_EMISSIVEMATERIALSOURCE    = 148,
    D3DRS_VERTEXBLEND               = 151,
    D3DRS_CLIPPLANEENABLE           = 152,
    D3DRS_SOFTWAREVERTEXPROCESSING  = 153,
    D3DRS_POINTSIZE                 = 154,
    D3DRS_POINTSIZE_MIN             = 155,
    D3DRS_POINTSPRITEENABLE         = 156,
    D3DRS_POINTSCALEENABLE          = 157,
    D3DRS_POINTSCALE_A              = 158,
    D3DRS_POINTSCALE_B              = 159,
    D3DRS_POINTSCALE_C              = 160,
    D3DRS_MULTISAMPLEANTIALIAS      = 161,
    D3DRS_MULTISAMPLEMASK           = 162,
    D3DRS_PATCHEDGESTYLE            = 163,
    D3DRS_PATCHSEGMENTS             = 164,
    D3DRS_DEBUGMONITORTOKEN         = 165,
    D3DRS_POINTSIZE_MAX             = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE  = 167,
    D3DRS_COLORWRITEENABLE          = 168,
    D3DRS_TWEENFACTOR               = 170,
    D3DRS_BLENDOP                   = 171
} D3DRENDERSTATETYPE;

// =====================================================================
// DirectX Texture Stage State Types
// =====================================================================

/** @brief DirectX texture stage state enumeration */
typedef enum {
    D3DTSS_COLOROP                  = 1,
    D3DTSS_COLORARG1                = 2,
    D3DTSS_COLORARG2                = 3,
    D3DTSS_ALPHAOP                  = 4,
    D3DTSS_ALPHAARG1                = 5,
    D3DTSS_ALPHAARG2                = 6,
    D3DTSS_BUMPENVMAT00             = 7,
    D3DTSS_BUMPENVMAT01             = 8,
    D3DTSS_BUMPENVMAT10             = 9,
    D3DTSS_BUMPENVMAT11             = 10,
    D3DTSS_TEXCOORDINDEX            = 11,
    D3DTSS_ADDRESSU                 = 13,
    D3DTSS_ADDRESSV                 = 14,
    D3DTSS_BORDERCOLOR              = 15,
    D3DTSS_MAGFILTER                = 16,
    D3DTSS_MINFILTER                = 17,
    D3DTSS_MIPFILTER                = 18,
    D3DTSS_MIPMAPLODBIAS            = 19,
    D3DTSS_MAXMIPLEVEL              = 20,
    D3DTSS_MAXANISOTROPY            = 21,
    D3DTSS_BUMPENVLSCALE            = 22,
    D3DTSS_BUMPENVLOFFSET           = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS    = 24,
    D3DTSS_ADDRESSW                 = 25,
    D3DTSS_COLORARG0                = 26,
    D3DTSS_ALPHAARG0                = 27,
    D3DTSS_RESULTARG                = 28,
    D3DTSS_CONSTANT                 = 32
} D3DTEXTURESTAGESTATETYPE;

// =====================================================================
// DirectX Transform State Types
// =====================================================================

/** @brief DirectX transform state enumeration */
typedef enum {
    D3DTS_WORLD       = 256,
    D3DTS_VIEW        = 2,
    D3DTS_PROJECTION  = 3,
    D3DTS_TEXTURE0    = 16,
    D3DTS_TEXTURE1    = 17,
    D3DTS_TEXTURE2    = 18,
    D3DTS_TEXTURE3    = 19,
    D3DTS_TEXTURE4    = 20,
    D3DTS_TEXTURE5    = 21,
    D3DTS_TEXTURE6    = 22,
    D3DTS_TEXTURE7    = 23
} D3DTRANSFORMSTATETYPE;

// =====================================================================
// DirectX Vertex Blend Flags
// =====================================================================

typedef enum {
    D3DVBF_DISABLE        = 0,
    D3DVBF_1WEIGHTS       = 1,
    D3DVBF_2WEIGHTS       = 2,
    D3DVBF_3WEIGHTS       = 3,
    D3DVBF_TWEENING       = 255,
    D3DVBF_0WEIGHTS       = 256
} D3DVERTEXBLENDFLAGS;

// =====================================================================
// DirectX Texture Coordinate Index Flags
// =====================================================================

#define D3DTSS_TCI_PASSTHRU           0x00000000L
#define D3DTSS_TCI_CAMERASPACENORMAL  0x00010000L
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x00020000L
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR 0x00030000L

// =====================================================================
// DirectX Texture Transform Flags
// =====================================================================

#define D3DTTFF_DISABLE               0
#define D3DTTFF_COUNT1                1
#define D3DTTFF_COUNT2                2
#define D3DTTFF_COUNT3                3
#define D3DTTFF_COUNT4                4
#define D3DTTFF_PROJECTED             256

// =====================================================================
// DirectX Filter Constants
// =====================================================================

#define D3DX_FILTER_NONE              0x00000001L
#define D3DX_FILTER_POINT             0x00000002L
#define D3DX_FILTER_LINEAR            0x00000003L
#define D3DX_FILTER_TRIANGLE          0x00000004L
#define D3DX_FILTER_BOX               1

// =====================================================================
// DirectX Special Constants
// =====================================================================

#define D3DX_DEFAULT                  ((UINT)-1)
#define D3DDP_MAXTEXCOORD             8
#define D3D_MATRIX                    D3DMATRIX

// =====================================================================
// DirectX Error Codes
// =====================================================================

#define D3DERR_NOTAVAILABLE           0x88760866L
#define D3DERR_OUTOFVIDEOMEMORY       0x8876017CL
#define D3DERR_DEVICELOST             0x88760868L
#define D3DERR_DEVICENOTRESET         0x88760869L
#define D3DERR_CONFLICTINGTEXTUREFILTER 0x88760826L
#define D3DERR_CONFLICTINGTEXTUREPALETTE 0x88760827L
#define D3DERR_UNSUPPORTEDTEXTUREFILTER 0x88760828L

// =====================================================================
// DirectX Format Types
// =====================================================================

/** @brief DirectX pixel format enumeration */
typedef enum {
    D3DFMT_UNKNOWN             = 0,
    D3DFMT_R8G8B8              = 20,
    D3DFMT_A8R8G8B8            = 21,
    D3DFMT_X8R8G8B8            = 22,
    D3DFMT_R5G6B5              = 23,
    D3DFMT_X1R5G5B5            = 24,
    D3DFMT_A1R5G5B5            = 25,
    D3DFMT_A4R4G4B4            = 26,
    D3DFMT_R3G3B2              = 27,
    D3DFMT_A8                  = 28,
    D3DFMT_A8R3G3B2            = 29,
    D3DFMT_X4R4G4B4            = 30,
    D3DFMT_A2B10G10R10         = 31,
    D3DFMT_A8B8G8R8            = 32,
    D3DFMT_X8B8G8R8            = 33,
    D3DFMT_G16R16              = 34,
    D3DFMT_A2R10G10B10         = 35,
    D3DFMT_A16B16G16R16        = 36,
    D3DFMT_A8P8                = 40,
    D3DFMT_P8                  = 41,
    D3DFMT_L8                  = 50,
    D3DFMT_A8L8                = 51,
    D3DFMT_A4L4                = 52,
    D3DFMT_V8U8                = 60,
    D3DFMT_L6V5U5              = 61,
    D3DFMT_X8L8V8U8            = 62,
    D3DFMT_Q8W8V8U8            = 63,
    D3DFMT_V16U16              = 64,
    D3DFMT_W11V11U10           = 65,  /**< 11-bit W, V, U components (bumpmapping) */
    D3DFMT_A2W10V10U10         = 67,
    D3DFMT_D16_LOCKABLE        = 70,
    D3DFMT_D32                 = 71,
    D3DFMT_D15S1               = 73,
    D3DFMT_D24S8               = 75,
    D3DFMT_D24X8               = 77,
    D3DFMT_D24X4S4             = 79,
    D3DFMT_D16                 = 80,
    D3DFMT_D32F_LOCKABLE       = 82,
    D3DFMT_D24FS8              = 83,
    D3DFMT_D32_LOCKABLE        = 84,
    D3DFMT_S8_LOCKABLE         = 85,
    D3DFMT_VERTEXDATA          = 100,
    D3DFMT_INDEX16             = 101,
    D3DFMT_INDEX32             = 102,
    D3DFMT_Q16W16V16U16        = 110,
    D3DFMT_MULTI2_ARGB8        = 827611204,
    D3DFMT_R16F                = 111,
    D3DFMT_G16R16F             = 112,
    D3DFMT_A16B16G16R16F       = 113,
    D3DFMT_R32F                = 114,
    D3DFMT_G32R32F             = 115,
    D3DFMT_A32B32G32R32F       = 116,
    D3DFMT_CxV8U8              = 117,
    D3DFMT_DXT1                = 827611204,
    D3DFMT_DXT2                = 844716353,
    D3DFMT_DXT3                = 844720673,
    D3DFMT_DXT4                = 860949553,
    D3DFMT_DXT5                = 861165617,
    D3DFMT_UYVY                = 1498831189,
    D3DFMT_YUY2                = 844715353
} D3DFORMAT;

// =====================================================================
// DirectX Vector Types
// =====================================================================

/** @brief 3D vector structure for geometry */
typedef struct _D3DVECTOR {
    float x;  /**< X component */
    float y;  /**< Y component */
    float z;  /**< Z component */
} D3DVECTOR;

typedef D3DVECTOR* LPD3DVECTOR;

// =====================================================================
// DirectX Structures
// =====================================================================

/** @brief Locked rectangle for texture/surface access */
#ifndef D3DLOCKED_RECT_DEFINED
#define D3DLOCKED_RECT_DEFINED
typedef struct {
    void* pBits;      /**< Pointer to pixel data */
    int Pitch;        /**< Row pitch in bytes */
} D3DLOCKED_RECT;
#endif

/** @brief Surface description structure */
#ifndef D3DSURFACE_DESC_DEFINED
#define D3DSURFACE_DESC_DEFINED
typedef struct {
    D3DFORMAT Format; /**< Surface pixel format */
    DWORD Type;       /**< Resource type */
    DWORD Usage;      /**< Usage flags */
    D3DPOOL Pool;     /**< Memory pool */
    DWORD Width;      /**< Width in pixels */
    DWORD Height;     /**< Height in pixels */
    DWORD Size;       /**< Total size in bytes */
} D3DSURFACE_DESC;
#endif

/** @brief Capabilities structure for device capabilities */
#ifndef D3DCAPS8_DEFINED
#define D3DCAPS8_DEFINED
typedef struct {
    DWORD DeviceType;              /**< Device type (HAL/REF/SW) */
    DWORD AdapterOrdinal;          /**< Adapter ordinal */
    DWORD Caps;                    /**< Capabilities flags */
    DWORD Caps2;                   /**< Extended capabilities */
    DWORD DevCaps;                 /**< Device capabilities */
    DWORD PrimitiveMiscCaps;       /**< Primitive misc capabilities */
    DWORD RasterCaps;              /**< Raster capabilities */
    DWORD ZCmpCaps;                /**< Z-comparison capabilities */
    DWORD SrcBlendCaps;            /**< Source blend capabilities */
    DWORD DestBlendCaps;           /**< Destination blend capabilities */
    DWORD AlphaCmpCaps;            /**< Alpha comparison capabilities */
    DWORD ShadeCaps;               /**< Shading capabilities */
    DWORD TextureCaps;             /**< Texture capabilities */
    DWORD TextureFilterCaps;       /**< Texture filter capabilities */
    DWORD CubeTextureFilterCaps;   /**< Cube texture filter capabilities */
    DWORD VolumeTextureFilterCaps; /**< Volume texture filter capabilities */
    DWORD TextureAddressCaps;      /**< Texture address capabilities */
    DWORD VolumeTextureAddressCaps;/**< Volume texture address capabilities */
    DWORD LineCaps;                /**< Line drawing capabilities */
    DWORD MaxTextureWidth;         /**< Maximum texture width */
    DWORD MaxTextureHeight;        /**< Maximum texture height */
    DWORD MaxVolumeExtent;         /**< Maximum volume extent */
    DWORD MaxTextureRepeat;        /**< Maximum texture repeat */
    DWORD MaxAnisotropy;           /**< Maximum anisotropy */
    float MaxVertexW;              /**< Maximum vertex w */
    float GuardBandLeft;           /**< Guard band left */
    float GuardBandTop;            /**< Guard band top */
    float GuardBandRight;          /**< Guard band right */
    float GuardBandBottom;         /**< Guard band bottom */
    float PerspectiveScaleThreshold;  /**< Perspective scale threshold */
    float MaxPointSize;            /**< Maximum point size */
    DWORD MaxPrimitiveCount;       /**< Maximum primitive count */
    DWORD MaxVertexIndex;          /**< Maximum vertex index */
    DWORD MaxStreams;              /**< Maximum streams */
    DWORD MaxStreamStride;         /**< Maximum stream stride */
    DWORD VertexShaderVersion;     /**< Vertex shader version */
    DWORD MaxVertexShaderConst;    /**< Maximum vertex shader constants */
    DWORD PixelShaderVersion;      /**< Pixel shader version */
    DWORD MaxPixelShaderValue;     /**< Maximum pixel shader value */
    DWORD MaxSimultaneousTextures; /**< Maximum simultaneous textures */
    DWORD TextureOpCaps;           /**< Texture operation capabilities */
    DWORD DeclTypes;               /**< Declared types */
    DWORD FVFCaps;                 /**< FVF capabilities */
    DWORD RasterCaps2;             /**< Extended raster capabilities */
} D3DCAPS8;
#endif

/** @brief Large integer (64-bit) structure */
#ifndef LARGE_INTEGER_DEFINED
#define LARGE_INTEGER_DEFINED
typedef union {
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    };
    int64_t QuadPart;
} LARGE_INTEGER;
#endif

/** @brief GUID (Globally Unique Identifier) structure */
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} GUID;
#endif

/** @brief Adapter identifier structure */
#ifndef D3DADAPTER_IDENTIFIER8_DEFINED
#define D3DADAPTER_IDENTIFIER8_DEFINED
typedef struct {
    char Driver[512];              /**< Driver name */
    char Description[512];         /**< Driver description */
    LARGE_INTEGER DriverVersion;   /**< Driver version */
    DWORD VendorId;                /**< Vendor ID */
    DWORD DeviceId;                /**< Device ID */
    DWORD SubSysId;                /**< Subsystem ID */
    DWORD Revision;                /**< Device revision */
    GUID DeviceIdentifier;         /**< Device GUID */
    DWORD WHQLLevel;               /**< WHQL level */
} D3DADAPTER_IDENTIFIER8;
#endif

/** @brief Color value structure (normalized float) */
#ifndef D3DCOLORVALUE_DEFINED
#define D3DCOLORVALUE_DEFINED
typedef struct {
    float r;  /**< Red component (0.0-1.0) */
    float g;  /**< Green component (0.0-1.0) */
    float b;  /**< Blue component (0.0-1.0) */
    float a;  /**< Alpha component (0.0-1.0) */
} D3DCOLORVALUE;
#endif

/** @brief Light structure */
#ifndef D3DLIGHT8_DEFINED
#define D3DLIGHT8_DEFINED
typedef struct {
    DWORD Type;                    /**< Light type (POINT/SPOT/DIRECTIONAL) */
    D3DCOLORVALUE Diffuse;         /**< Diffuse color */
    D3DCOLORVALUE Specular;        /**< Specular color */
    D3DCOLORVALUE Ambient;         /**< Ambient color */
    D3DVECTOR Position;            /**< Light position */
    D3DVECTOR Direction;           /**< Light direction */
    float Range;                   /**< Light range */
    float Falloff;                 /**< Falloff */
    float Attenuation0;            /**< Attenuation factor 0 */
    float Attenuation1;            /**< Attenuation factor 1 */
    float Attenuation2;            /**< Attenuation factor 2 */
    float Theta;                   /**< Theta angle */
    float Phi;                     /**< Phi angle */
} D3DLIGHT8;
#endif

/** @brief Material structure */
#ifndef D3DMATERIAL8_DEFINED
#define D3DMATERIAL8_DEFINED
typedef struct {
    D3DCOLORVALUE Diffuse;         /**< Diffuse color */
    D3DCOLORVALUE Ambient;         /**< Ambient color */
    D3DCOLORVALUE Specular;        /**< Specular color */
    D3DCOLORVALUE Emissive;        /**< Emissive color */
    float Power;                   /**< Specular power */
} D3DMATERIAL8;
#endif

/** @brief Viewport structure */
#ifndef D3DVIEWPORT8_DEFINED
#define D3DVIEWPORT8_DEFINED
typedef struct {
    DWORD X;           /**< Viewport X position */
    DWORD Y;           /**< Viewport Y position */
    DWORD Width;       /**< Viewport width */
    DWORD Height;      /**< Viewport height */
    float MinZ;        /**< Minimum Z value */
    float MaxZ;        /**< Maximum Z value */
} D3DVIEWPORT8;
#endif

/** @brief Transform-locked vertex structure */
#ifndef D3DTLVERTEX_DEFINED
#define D3DTLVERTEX_DEFINED
typedef struct {
    float x, y, z, rhw;  /**< Position and RHW */
    DWORD color;         /**< Diffuse color */
    float tu, tv;        /**< Texture coordinates */
} D3DTLVERTEX;
#endif

/** @brief Display mode structure */
#ifndef D3DDISPLAYMODE_DEFINED
#define D3DDISPLAYMODE_DEFINED
typedef struct {
    DWORD Width;           /**< Display width */
    DWORD Height;          /**< Display height */
    DWORD RefreshRate;     /**< Refresh rate */
    D3DFORMAT Format;      /**< Display format */
} D3DDISPLAYMODE;
#endif

/** @brief Matrix 4x4 structure */
#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
} D3DMATRIX;

typedef D3DMATRIX* LPD3DMATRIX;
#endif

// =====================================================================
// Vertex Format Constants (from d3dx8fvf.h)
// =====================================================================

#define D3DFVF_RESERVED0              0x001
#define D3DFVF_POSITION_MASK          0x00E
#define D3DFVF_XYZ                    0x002
#define D3DFVF_XYZRHW                 0x004
#define D3DFVF_XYZB1                  0x006
#define D3DFVF_XYZB2                  0x008
#define D3DFVF_XYZB3                  0x00A
#define D3DFVF_XYZB4                  0x00C
#define D3DFVF_XYZB5                  0x00E
#define D3DFVF_NORMAL                 0x010
#define D3DFVF_PSIZE                  0x020
#define D3DFVF_DIFFUSE                0x040
#define D3DFVF_SPECULAR               0x080
#define D3DFVF_TEXCOUNT_MASK          0xF00
#define D3DFVF_TEXCOUNT_SHIFT         8
#define D3DFVF_TEX0                   0x000
#define D3DFVF_TEX1                   0x100
#define D3DFVF_TEX2                   0x200
#define D3DFVF_TEX3                   0x300
#define D3DFVF_TEX4                   0x400
#define D3DFVF_TEX5                   0x500
#define D3DFVF_TEX6                   0x600
#define D3DFVF_TEX7                   0x700
#define D3DFVF_TEX8                   0x800

/* D3DFVF texture coordinate size macros for vertex formats */
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (0x0000 << (CoordIndex*2+16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (0x0001 << (CoordIndex*2+16))
#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (0x0002 << (CoordIndex*2+16))
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (0x0003 << (CoordIndex*2+16))

/* DirectX 8 Interface Forward Declarations (stubs for compatibility) */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct IDirect3D8 IDirect3D8;
typedef struct IDirect3DDevice8 IDirect3DDevice8;
typedef struct IDirect3DTexture8 IDirect3DTexture8;
typedef struct IDirect3DSurface8 IDirect3DSurface8;
typedef struct IDirect3DVertexBuffer8 IDirect3DVertexBuffer8;
typedef struct IDirect3DIndexBuffer8 IDirect3DIndexBuffer8;
typedef struct IDirect3DCubeTexture8 IDirect3DCubeTexture8;
typedef struct IDirect3DVolumeTexture8 IDirect3DVolumeTexture8;
typedef struct IDirect3DSwapChain8 IDirect3DSwapChain8;

#ifdef __cplusplus
}
#endif

// Include COM interface stub definitions after all enums are defined
#include "d3d8_vulkan_interfaces_compat.h"

#endif // D3D8_H_INCLUDED

