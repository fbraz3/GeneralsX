#ifndef DDRAW_COMPAT_H
#define DDRAW_COMPAT_H

#ifndef _WIN32

// DirectDraw compatibility layer for non-Windows platforms

// Include base Windows compatibility
#include "win32_compat.h"

// DirectDraw result codes
#define DD_OK 0
#define DDERR_GENERIC -1
#define DDERR_INVALIDPARAMS -2
#define DDERR_UNSUPPORTED -3

// DirectDraw pixel format flags
#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_ALPHA 0x00000002
#define DDPF_FOURCC 0x00000004
#define DDPF_PALETTEINDEXED4 0x00000008
#define DDPF_PALETTEINDEXEDTO8 0x00000010
#define DDPF_PALETTEINDEXED8 0x00000020
#define DDPF_RGB 0x00000040
#define DDPF_COMPRESSED 0x00000080
#define DDPF_RGBTOYUV 0x00000100
#define DDPF_YUV 0x00000200
#define DDPF_ZBUFFER 0x00000400
#define DDPF_PALETTEINDEXED1 0x00000800
#define DDPF_PALETTEINDEXED2 0x00001000
#define DDPF_ZPIXELS 0x00002000
#define DDPF_STENCILBUFFER 0x00004000
#define DDPF_ALPHAPREMULT 0x00008000
#define DDPF_LUMINANCE 0x00020000
#define DDPF_BUMPLUMINANCE 0x00040000
#define DDPF_BUMPDUDV 0x00080000

// DirectDraw surface capabilities
#define DDSCAPS2_CUBEMAP 0x00000200
#define DDSCAPS2_VOLUME 0x00200000

// DirectDraw structures
typedef struct {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
        DWORD dwLuminanceBitCount;
        DWORD dwBumpBitCount;
        DWORD dwPrivateFormatBitCount;
    };
    union {
        DWORD dwRBitMask;
        DWORD dwYBitMask;
        DWORD dwStencilBitDepth;
        DWORD dwLuminanceBitMask;
        DWORD dwBumpDuBitMask;
        DWORD dwOperations;
    };
    union {
        DWORD dwGBitMask;
        DWORD dwUBitMask;
        DWORD dwZBitMask;
        DWORD dwBumpDvBitMask;
        struct {
            WORD wFlipMSTypes;
            WORD wBltMSTypes;
        } MultiSampleCaps;
    };
    union {
        DWORD dwBBitMask;
        DWORD dwVBitMask;
        DWORD dwStencilBitMask;
        DWORD dwBumpLuminanceBitMask;
    };
    union {
        DWORD dwRGBAlphaBitMask;
        DWORD dwYUVAlphaBitMask;
        DWORD dwLuminanceAlphaBitMask;
        DWORD dwRGBZBitMask;
        DWORD dwYUVZBitMask;
    };
} DDPIXELFORMAT;

typedef DDPIXELFORMAT* LPDDPIXELFORMAT;

#endif // !_WIN32

#endif // DDRAW_COMPAT_H
