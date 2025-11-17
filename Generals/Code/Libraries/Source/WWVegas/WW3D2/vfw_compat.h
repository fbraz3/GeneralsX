/*
** Phase 03: Video For Windows (VFW) API Compatibility Stub
** vfw.h - Video for Windows (DirectShow predecessor)
**
** Pattern: source_dest_type_compat
** Source: Windows vfw.h (Video for Windows API)
** Destination: POSIX/Linux/macOS platforms
** Type: Windows video API compatibility stubs
**
** vfw.h provides access to AVI file creation and video capture on Windows.
** On non-Windows platforms, this functionality is not available through VFW.
** We provide empty stubs to allow compilation.
*/

#pragma once

#ifndef VFW_COMPAT_H_INCLUDED
#define VFW_COMPAT_H_INCLUDED

#include <cstring>
#include <cstdlib>

/* Windows type definitions */
typedef long HRESULT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned short USHORT;
typedef int BOOL;
/* Note: RECT is already defined in windows.h, so we don't redefine it here */

/* Common memory functions */
static inline void ZeroMemory(void* ptr, size_t size)
{
    memset(ptr, 0, size);
}

/* Memory allocation flags */
#define GMEM_MOVEABLE 0x0002
#define GMEM_FIXED 0x0000
#define AVIIF_KEYFRAME 0x00000010

/* Return value constants */
#define S_OK 0
#define E_FAIL -1

/* File open modes */
#define OF_READ 0x0000
#define OF_WRITE 0x0001
#define OF_READWRITE 0x0002
#define OF_CREATE 0x1000

/* SetRect macro */
#define SetRect(prc, xLeft, yTop, xRight, yBottom) \
    do { (prc)->left = xLeft; (prc)->top = yTop; (prc)->right = xRight; (prc)->bottom = yBottom; } while(0)

/* Define dummy handles for AVI operations */
typedef void* PAVIFILE;
typedef void* PAVISTREAM;
typedef void* HDRAWDIB;

/* Common FOURCC values */
#define mmioFOURCC(ch0, ch1, ch2, ch3) \
    ((unsigned long)(unsigned char)(ch0) | ((unsigned long)(unsigned char)(ch1) << 8) | \
     ((unsigned long)(unsigned char)(ch2) << 16) | ((unsigned long)(unsigned char)(ch3) << 24))

/* Stream type codes */
#define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#define streamtypeAUDIO mmioFOURCC('a', 'u', 'd', 's')

/* BI_* compression constants - BI_RGB needed for VFW, others in win32_sdl_types_compat.h */
#ifndef BI_RGB
#define BI_RGB 0L
#endif

/* BITMAPINFOHEADER structure - defined here for VFW AVI operations */
#ifndef BITMAPINFOHEADER_DEFINED
#define BITMAPINFOHEADER_DEFINED

typedef struct {
    unsigned long biSize;
    long biWidth;
    long biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long biCompression;
    unsigned long biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    unsigned long biClrUsed;
    unsigned long biClrImportant;
} BITMAPINFOHEADER;

#endif /* BITMAPINFOHEADER_DEFINED */

/* AVISTREAMINFO structure stub */
typedef struct {
    unsigned int fccType;
    unsigned int fccHandler;
    unsigned long dwFlags;
    unsigned long dwCaps;
    unsigned short wPriority;
    unsigned short wLanguage;
    unsigned long dwScale;
    unsigned long dwRate;
    unsigned long dwStart;
    unsigned long dwLength;
    unsigned long dwInitialFrames;
    unsigned long dwSuggestedBufferSize;
    unsigned long dwQuality;
    unsigned long dwSampleSize;
    RECT rcFrame;
    unsigned long dwEditCount;
    unsigned long dwFormatChangeCount;
    char szName[64];
} AVISTREAMINFO;

/* Memory management stubs */
static inline void* GlobalAllocPtr(unsigned int flags, size_t size)
{
    return malloc(size);
}

static inline void GlobalFreePtr(void* ptr)
{
    free(ptr);
}

/* Empty stub functions that immediately fail or return NULL */
static inline HRESULT AVIFileInit(void)
{
    return S_OK;  /* Success - nothing to initialize */
}

static inline HRESULT AVIFileOpen(PAVIFILE* ppfile, const char* szFile, unsigned int uMode, void* lpHandler)
{
    if (ppfile) *ppfile = NULL;
    return E_FAIL;  /* Fail - no AVI support */
}

static inline HRESULT AVIFileCreateStream(PAVIFILE pfile, PAVISTREAM* ppavi, const AVISTREAMINFO* psi)
{
    if (ppavi) *ppavi = NULL;
    return E_FAIL;  /* Fail - no AVI support */
}

static inline HRESULT AVIStreamSetFormat(PAVISTREAM pavi, LONG lStart, const void* lpFormat, LONG cbFormat)
{
    return E_FAIL;  /* Fail - no AVI support */
}

static inline HRESULT AVIStreamWrite(PAVISTREAM pavi, LONG lStart, LONG lSamples, const void* lpBuffer, 
                                     LONG cbBuffer, DWORD dwFlags, LONG* plSampWritten, LONG* plBytesWritten)
{
    return E_FAIL;  /* Fail - no AVI support */
}

static inline ULONG AVIStreamRelease(PAVISTREAM pavi)
{
    return 0;
}

static inline ULONG AVIFileRelease(PAVIFILE pfile)
{
    return 0;
}

static inline void AVIFileExit(void)
{
    /* Nothing to clean up */
}

static inline HDRAWDIB DrawDibOpen(void)
{
    return NULL;
}

static inline BOOL DrawDibClose(HDRAWDIB hdd)
{
    return FALSE;
}

static inline BOOL DrawDibDraw(HDRAWDIB hdd, void* hdc, int xDst, int yDst, int dxDst, int dyDst, 
                               BITMAPINFOHEADER* lpbi, void* lpBits, int xSrc, int ySrc, int dxSrc, int dySrc, unsigned int wFlags)
{
    return FALSE;
}

static inline int DrawDibSetPalette(HDRAWDIB hdd, void* hpal)
{
    return 0;
}

#endif /* VFW_COMPAT_H_INCLUDED */
