#pragma once

/**
 * @file d3d8_enums.h
 * @brief DirectX 8 Enumeration Definitions
 * 
 * This header provides DirectX 8 enumeration values needed by interfaces.
 * 
 * Phase 02.5: DirectX 8 Stub Interface Implementation
 */

#ifndef D3D8_ENUMS_H_INCLUDED
#define D3D8_ENUMS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// =====================================================================
// Enumeration Types
// =====================================================================

/**
 * @enum D3DPRIMITIVETYPE
 * @brief Primitive types for rendering
 */
enum D3DPRIMITIVETYPE {
    D3DPT_POINTLIST       = 1,
    D3DPT_LINELIST        = 2,
    D3DPT_LINESTRIP       = 3,
    D3DPT_TRIANGLELIST    = 4,
    D3DPT_TRIANGLESTRIP   = 5,
    D3DPT_TRIANGLEFAN     = 6,
};

/**
 * @enum D3DRENDERSTATETYPE
 * @brief Render state types (70+ states)
 * 
 * These control global rendering behavior like lighting, blending, etc.
 */
enum D3DRENDERSTATETYPE {
    D3DRS_ZENABLE                      = 7,
    D3DRS_FILLMODE                     = 8,
    D3DRS_SHADEMODE                    = 9,
    D3DRS_LINEPATTERN                  = 10,
    D3DRS_ZWRITEENABLE                 = 14,
    D3DRS_ALPHATESTENABLE              = 15,
    D3DRS_LASTPIXEL                    = 16,
    D3DRS_SRCBLEND                     = 19,
    D3DRS_DESTBLEND                    = 20,
    D3DRS_CULLMODE                     = 22,
    D3DRS_ZFUNC                        = 23,
    D3DRS_ALPHAREF                     = 24,
    D3DRS_ALPHAFUNC                    = 25,
    D3DRS_DITHERENABLE                 = 26,
    D3DRS_ALPHABLENDENABLE             = 27,
    D3DRS_FOGENABLE                    = 28,
    D3DRS_FOGTABLEMODE                 = 30,
    D3DRS_FOGSTART                     = 36,
    D3DRS_FOGEND                       = 37,
    D3DRS_FOGDENSITY                   = 38,
    D3DRS_RANGEFOGENABLE               = 48,
    D3DRS_STENCILENABLE                = 52,
    D3DRS_STENCILFAIL                  = 53,
    D3DRS_STENCILZFAIL                 = 54,
    D3DRS_STENCILPASS                  = 55,
    D3DRS_STENCILFUNC                  = 56,
    D3DRS_STENCILREF                   = 57,
    D3DRS_STENCILMASK                  = 58,
    D3DRS_STENCILWRITEMASK             = 59,
    D3DRS_TEXTUREFACTOR                = 60,
    D3DRS_WRAP0                        = 128,
    D3DRS_WRAP1                        = 129,
    D3DRS_WRAP2                        = 130,
    D3DRS_WRAP3                        = 131,
    D3DRS_LIGHTING                     = 137,
    D3DRS_AMBIENT                      = 139,
    D3DRS_FOGVERTEXMODE                = 140,
    D3DRS_COLORVERTEX                  = 141,
    D3DRS_LOCALVIEWER                  = 142,
    D3DRS_NORMALIZENORMALS             = 143,
    D3DRS_DIFFUSEMATERIALSOURCE        = 145,
    D3DRS_SPECULARMATERIALSOURCE       = 146,
    D3DRS_AMBIENTMATERIALSOURCE        = 147,
    D3DRS_EMISSIVEMATERIALSOURCE       = 148,
    D3DRS_VERTEXBLEND                  = 151,
    D3DRS_CLIPPLANEENABLE              = 152,
    D3DRS_POINTSIZE                    = 154,
    D3DRS_POINTSIZE_MIN                = 155,
    D3DRS_POINTSPRITEENABLE            = 156,
    D3DRS_POINTSCALEENABLE             = 157,
    D3DRS_POINTSCALE_A                 = 158,
    D3DRS_POINTSCALE_B                 = 159,
    D3DRS_POINTSCALE_C                 = 160,
    D3DRS_MULTISAMPLEANTIALIAS         = 161,
    D3DRS_MULTISAMPLEMASK              = 162,
    D3DRS_PATCHEDGESTYLE               = 163,
    D3DRS_DEBUGMONITORTOKEN            = 165,
    D3DRS_POINTSIZE_MAX                = 166,
    D3DRS_INDEXEDVERTEXBLENDENABLE    = 167,
    D3DRS_COLORWRITEENABLE             = 168,
    D3DRS_TWEENFACTOR                  = 170,
    D3DRS_BLENDOP                      = 171,
};

/**
 * @enum D3DTEXTURESTAGESTATETYPE
 * @brief Texture stage state types
 */
enum D3DTEXTURESTAGESTATETYPE {
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
    D3DTSS_BUMPENVLSCALE            = 22,
    D3DTSS_BUMPENVLOFFSET           = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS    = 24,
};

/**
 * @enum D3DSAMPLERSTATETYPE
 * @brief Sampler state types
 */
enum D3DSAMPLERSTATETYPE {
    D3DSAMP_ADDRESSU                = 1,
    D3DSAMP_ADDRESSV                = 2,
    D3DSAMP_ADDRESSW                = 3,
    D3DSAMP_BORDERCOLOR             = 4,
    D3DSAMP_MAGFILTER               = 5,
    D3DSAMP_MINFILTER               = 6,
    D3DSAMP_MIPFILTER               = 7,
    D3DSAMP_MIPMAPLODBIAS           = 8,
    D3DSAMP_MAXMIPLEVEL             = 9,
    D3DSAMP_MAXANISOTROPY           = 10,
    D3DSAMP_SRGBTEXTURE             = 11,
    D3DSAMP_ELEMENTINDEX            = 12,
    D3DSAMP_DMAP                    = 13,
};

/**
 * @enum D3DTRANSFORMSTATETYPE
 * @brief Transform matrix types
 */
enum D3DTRANSFORMSTATETYPE {
    D3DTS_VIEW              = 2,
    D3DTS_PROJECTION        = 3,
    D3DTS_TEXTURE0          = 16,
    D3DTS_TEXTURE1          = 17,
    D3DTS_TEXTURE2          = 18,
    D3DTS_TEXTURE3          = 19,
    D3DTS_TEXTURE4          = 20,
    D3DTS_TEXTURE5          = 21,
    D3DTS_TEXTURE6          = 22,
    D3DTS_TEXTURE7          = 23,
    D3DTS_WORLD             = 256,
    D3DTS_WORLD1            = 257,
    D3DTS_WORLD2            = 258,
    D3DTS_WORLD3            = 259,
};

/**
 * @enum D3DFORMAT
 * @brief Texture and surface format (60+ formats)
 */
enum D3DFORMAT {
    D3DFMT_UNKNOWN              = 0,
    D3DFMT_R8G8B8               = 20,
    D3DFMT_A8R8G8B8             = 21,
    D3DFMT_X8R8G8B8             = 22,
    D3DFMT_R5G6B5               = 23,
    D3DFMT_X1R5G5B5             = 24,
    D3DFMT_A1R5G5B5             = 25,
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8                   = 28,
    D3DFMT_A8R3G3B2             = 29,
    D3DFMT_X4R4G4B4             = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_A8B8G8R8             = 32,
    D3DFMT_X8B8G8R8             = 33,
    D3DFMT_G16R16               = 34,
    D3DFMT_A2R10G10B10          = 35,
    D3DFMT_A16B16G16R16         = 36,
    D3DFMT_A8P8                 = 40,
    D3DFMT_P8                   = 41,
    D3DFMT_L8                   = 50,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,
    D3DFMT_V8U8                 = 60,
    D3DFMT_L6V5U5               = 61,
    D3DFMT_X8L8V8U8             = 62,
    D3DFMT_Q8W8V8U8             = 63,
    D3DFMT_V16U16               = 64,
    D3DFMT_A2W10V10U10          = 67,
    D3DFMT_D16_LOCKABLE         = 70,
    D3DFMT_D32                  = 71,
    D3DFMT_D15S1                = 73,
    D3DFMT_D24S8                = 75,
    D3DFMT_D24X8                = 77,
    D3DFMT_D24X4S4              = 79,
    D3DFMT_D16                  = 80,
    D3DFMT_D32F_LOCKABLE        = 82,
    D3DFMT_D24FS8               = 83,
    D3DFMT_D32_LOCKABLE         = 84,
    D3DFMT_S8_LOCKABLE          = 85,
    D3DFMT_VERTEXDATA           = 100,
    D3DFMT_INDEX16              = 101,
    D3DFMT_INDEX32              = 102,
    D3DFMT_Q16W16V16U16         = 110,
    D3DFMT_MULTI2_ARGB8         = 0x3154414D,
    D3DFMT_R16F                 = 111,
    D3DFMT_G16R16F              = 112,
    D3DFMT_A16B16G16R16F        = 113,
    D3DFMT_R32F                 = 114,
    D3DFMT_G32R32F              = 115,
    D3DFMT_A32B32G32R32F        = 116,
    D3DFMT_CxV8U8               = 117,
    D3DFMT_DXT1                 = 0x31545844,
    D3DFMT_DXT2                 = 0x32545844,
    D3DFMT_DXT3                 = 0x33545844,
    D3DFMT_DXT4                 = 0x34545844,
    D3DFMT_DXT5                 = 0x35545844,
};

/**
 * @enum D3DPOOL
 * @brief Memory pool for resource allocation
 */
enum D3DPOOL {
    D3DPOOL_DEFAULT         = 0,
    D3DPOOL_MANAGED         = 1,
    D3DPOOL_SYSTEMMEM       = 2,
    D3DPOOL_SCRATCH         = 3,
};

/**
 * @enum D3DUSAGE
 * @brief Usage flags for resource creation
 */
enum D3DUSAGE {
    D3DUSAGE_RENDERTARGET       = 0x00000001,
    D3DUSAGE_DEPTHSTENCIL       = 0x00000002,
    D3DUSAGE_DYNAMIC            = 0x00000200,
    D3DUSAGE_NONSECURE         = 0x00800000,
    D3DUSAGE_NPATCHES          = 0x00100000,
    D3DUSAGE_POINTS            = 0x00000040,
    D3DUSAGE_RTPATCHES         = 0x00080000,
    D3DUSAGE_SOFTWAREPROCESSING = 0x00000020,
    D3DUSAGE_TEXTAPI           = 0x10000000,
};

/**
 * @enum D3DDEVTYPE
 * @brief Device type
 */
enum D3DDEVTYPE {
    D3DDEVTYPE_HAL                = 1,
    D3DDEVTYPE_REF                = 2,
    D3DDEVTYPE_SW                 = 3,
};

/**
 * @enum D3DMULTISAMPLE_TYPE
 * @brief Multi-sampling types
 */
enum D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE             = 0,
    D3DMULTISAMPLE_NONMASKABLE      = 1,
    D3DMULTISAMPLE_2_SAMPLES        = 2,
    D3DMULTISAMPLE_3_SAMPLES        = 3,
    D3DMULTISAMPLE_4_SAMPLES        = 4,
    D3DMULTISAMPLE_5_SAMPLES        = 5,
    D3DMULTISAMPLE_6_SAMPLES        = 6,
    D3DMULTISAMPLE_7_SAMPLES        = 7,
    D3DMULTISAMPLE_8_SAMPLES        = 8,
    D3DMULTISAMPLE_9_SAMPLES        = 9,
    D3DMULTISAMPLE_10_SAMPLES       = 10,
    D3DMULTISAMPLE_11_SAMPLES       = 11,
    D3DMULTISAMPLE_12_SAMPLES       = 12,
    D3DMULTISAMPLE_13_SAMPLES       = 13,
    D3DMULTISAMPLE_14_SAMPLES       = 14,
    D3DMULTISAMPLE_15_SAMPLES       = 15,
    D3DMULTISAMPLE_16_SAMPLES       = 16,
};

/**
 * @enum D3DSWAPEFFECT
 * @brief Swap effect for presentation
 */
enum D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD          = 1,
    D3DSWAPEFFECT_FLIP             = 2,
    D3DSWAPEFFECT_COPY             = 3,
    D3DSWAPEFFECT_OVERLAY          = 4,
    D3DSWAPEFFECT_FLIPEX           = 5,
};

/**
 * @enum D3DPRESENT
 * @brief Present interval flags
 */
enum D3DPRESENT {
    D3DPRESENT_INTERVAL_DEFAULT     = 0,
    D3DPRESENT_INTERVAL_ONE         = 1,
    D3DPRESENT_INTERVAL_TWO         = 2,
    D3DPRESENT_INTERVAL_THREE       = 3,
    D3DPRESENT_INTERVAL_FOUR        = 4,
    D3DPRESENT_INTERVAL_IMMEDIATE   = 0x80000000,
};

/**
 * @enum D3DCUBEMAP_FACES
 * @brief Cube map face identifiers
 */
enum D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X      = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X      = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y      = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y      = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z      = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z      = 5,
};

/**
 * @enum D3DBACKBUFFER_TYPE
 * @brief Back buffer type
 */
enum D3DBACKBUFFER_TYPE {
    D3DBACKBUFFER_TYPE_MONO         = 0,
    D3DBACKBUFFER_TYPE_LEFT         = 1,
    D3DBACKBUFFER_TYPE_RIGHT        = 2,
};

/**
 * @enum D3DRESOURCETYPE
 * @brief Resource type
 */
enum D3DRESOURCETYPE {
    D3DRTYPE_SURFACE                = 1,
    D3DRTYPE_VOLUME                 = 2,
    D3DRTYPE_TEXTURE                = 3,
    D3DRTYPE_VOLUMETEXTURE          = 4,
    D3DRTYPE_CUBETEXTURE            = 5,
    D3DRTYPE_VERTEXBUFFER           = 6,
    D3DRTYPE_INDEXBUFFER            = 7,
};

/**
 * @enum D3DLIGHTTYPE
 * @brief Light type
 */
enum D3DLIGHTTYPE {
    D3DLIGHT_POINT                  = 1,
    D3DLIGHT_SPOT                   = 2,
    D3DLIGHT_DIRECTIONAL            = 3,
};

/**
 * @enum D3DBASISTYPE
 * @brief Basis type for patches
 */
enum D3DBASISTYPE {
    D3DBASIS_BEZIER                 = 0,
    D3DBASIS_BSPLINE                = 1,
    D3DBASIS_CATMULL_ROM            = 2,
};

/**
 * @enum D3DDEGREETYPE
 * @brief Degree type for patches
 */
enum D3DDEGREETYPE {
    D3DDEGREE_LINEAR                = 1,
    D3DDEGREE_QUADRATIC             = 2,
    D3DDEGREE_CUBIC                 = 3,
    D3DDEGREE_QUINTIC              = 5,
};

/**
 * @enum D3DTEXTUREADDRESS
 * @brief Texture addressing modes
 */
enum D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP                = 1,
    D3DTADDRESS_MIRROR              = 2,
    D3DTADDRESS_CLAMP               = 3,
    D3DTADDRESS_BORDER              = 4,
    D3DTADDRESS_MIRRORONCE          = 5,
};

/**
 * @enum D3DTEXTUREFILTERTYPE
 * @brief Texture filter types
 */
enum D3DTEXTUREFILTERTYPE {
    D3DTEXF_NONE                    = 0,
    D3DTEXF_POINT                   = 1,
    D3DTEXF_LINEAR                  = 2,
    D3DTEXF_ANISOTROPIC             = 3,
    D3DTEXF_PYRAMIDALQUAD           = 6,
    D3DTEXF_GAUSSIANQUAD            = 7,
};

#ifdef __cplusplus
}
#endif

#endif // D3D8_ENUMS_H_INCLUDED
