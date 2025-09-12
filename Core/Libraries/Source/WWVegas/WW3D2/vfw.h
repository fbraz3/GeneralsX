#pragma once

// Video for Windows compatibility header
// This header provides VFW functionality for non-Windows systems

#ifndef _WIN32

#include "win32_compat.h"

// Video for Windows types
typedef void* HVIDEO;
typedef void* HIC;
typedef void* HDRAWDIB;
typedef void* PAVIFILE;
typedef void* PAVISTREAM;

// AVI Stream Info structure
typedef struct {
    DWORD fccType;
    DWORD fccHandler;
    DWORD dwFlags;
    DWORD dwCaps;
    WORD wPriority;
    WORD wLanguage;
    DWORD dwScale;
    DWORD dwRate;
    DWORD dwStart;
    DWORD dwLength;
    DWORD dwInitialFrames;
    DWORD dwSuggestedBufferSize;
    DWORD dwQuality;
    DWORD dwSampleSize;
    RECT rcFrame;
    DWORD dwEditCount;
    DWORD dwFormatChangeCount;
    char szName[64];
} AVISTREAMINFO;

// Video for Windows constants
#define OF_WRITE 1
#define OF_CREATE 0x1000
#define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#define GMEM_MOVEABLE 0x0002
#define AVIIF_KEYFRAME 0x00000010L

// COM error codes if not already defined
#ifndef E_FAIL
#define E_FAIL 0x80004005L
#endif

// Additional types
typedef unsigned long ULONG;

// Global memory functions
#ifndef GLOBALALLOCPTR_DEFINED
#define GLOBALALLOCPTR_DEFINED
inline void* GlobalAllocPtr(UINT flags, DWORD size) { return malloc(size); }
#endif
inline void GlobalFreePtr(void* ptr) { free(ptr); }

// Rectangle function
inline void SetRect(RECT* rect, int left, int top, int right, int bottom) {
    if (rect) {
        rect->left = left;
        rect->top = top;
        rect->right = right;
        rect->bottom = bottom;
    }
}

// AVI functions
inline void AVIFileInit() {}
inline void AVIFileExit() {}
inline HRESULT AVIFileOpen(PAVIFILE* ppfile, const char* szFile, UINT uMode, void* lpHandler) {
    return E_FAIL;
}
inline HRESULT AVIFileCreateStream(PAVIFILE pfile, PAVISTREAM* ppavi, AVISTREAMINFO* psi) {
    return E_FAIL;
}
inline HRESULT AVIStreamSetFormat(PAVISTREAM pavi, LONG lPos, void* lpFormat, LONG cbFormat) {
    return E_FAIL;
}
inline HRESULT AVIStreamWrite(PAVISTREAM pavi, LONG lStart, LONG lSamples, void* lpBuffer, LONG cbBuffer, DWORD dwFlags, LONG* plSampWritten, LONG* plBytesWritten) {
    return E_FAIL;
}
inline ULONG AVIStreamRelease(PAVISTREAM pavi) { return 0; }
inline ULONG AVIFileRelease(PAVIFILE pfile) { return 0; }

// Compression/decompression constants
#define ICTYPE_VIDEO            mmioFOURCC('v', 'i', 'd', 'c')
#define ICTYPE_AUDIO            mmioFOURCC('a', 'u', 'd', 'c')

// Video compression modes
#define ICMODE_COMPRESS         1
#define ICMODE_DECOMPRESS       2
#define ICMODE_FASTDECOMPRESS   3
#define ICMODE_QUERY            4
#define ICMODE_FASTCOMPRESS     5

// Error codes
#define ICERR_OK                0
#define ICERR_DONTDRAW          1
#define ICERR_NEWPALETTE        2
#define ICERR_GOTOKEYFRAME      3
#define ICERR_STOPDRAWING       4
#define ICERR_UNSUPPORTED       -1
#define ICERR_BADFORMAT         -2
#define ICERR_MEMORY            -3
#define ICERR_INTERNAL          -4
#define ICERR_BADFLAGS          -5
#define ICERR_BADPARAM          -6
#define ICERR_BADSIZE           -7
#define ICERR_BADHANDLE         -8
#define ICERR_CANTUPDATE        -9
#define ICERR_ABORT             -10
#define ICERR_ERROR             -100
#define ICERR_BADBITDEPTH       -200
#define ICERR_BADIMAGESIZE      -201

// FOURCC macro
#define mmioFOURCC(ch0, ch1, ch2, ch3) \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
     ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))

// Stub functions
inline HIC ICOpen(DWORD fccType, DWORD fccHandler, UINT wMode) { return nullptr; }
inline DWORD ICClose(HIC hic) { return ICERR_OK; }
inline DWORD ICCompress(HIC hic, DWORD dwFlags, 
                       BITMAPINFOHEADER* lpbiOutput, void* lpData,
                       BITMAPINFOHEADER* lpbiInput, void* lpBits,
                       DWORD* lpckid, DWORD* lpdwFlags, 
                       LONG lFrameNum, DWORD dwFrameSize, 
                       DWORD dwQuality, 
                       BITMAPINFOHEADER* lpbiPrev, void* lpPrev) {
    return ICERR_UNSUPPORTED;
}

inline DWORD ICDecompress(HIC hic, DWORD dwFlags,
                         BITMAPINFOHEADER* lpbiFormat, void* lpData,
                         BITMAPINFOHEADER* lpbi, void* lpBits) {
    return ICERR_UNSUPPORTED;
}

#else
// On Windows, include the real vfw.h
#include <vfw.h>
#endif
