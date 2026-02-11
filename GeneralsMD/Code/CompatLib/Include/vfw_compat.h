#pragma once

// TheSuperHackers @build fbraz 10/02/2026 Bender
// Video for Windows (VFW) / AVI File API compatibility stubs
// Used by WW3D2/FramGrab.cpp (movie capture system)
// Phase 3 out-of-scope: Stubs return failure, disabling movie capture on Linux

#include "types_compat.h"

#ifndef _WIN32

// TheSuperHackers @build fbraz 10/02/2026 Bender
// Need full RECT definition (not just forward declaration) for SetRect() function
// DXVK windows_base.h provides complete RECT structure
#include <windows_base.h>

#include "gdi_compat.h"  // For BITMAPINFOHEADER (already has RECT forward-decl)

// Forward declarations
// NOTE: PAVIFILE/PAVISTREAM are TYPES. In FramGrab.cpp, "Stream" and "AVIFile" are VARIABLE NAMES.
// Do NOT #define them as type aliases - that breaks "PAVISTREAM Stream;" declarations!
typedef void* PAVIFILE;
typedef void* PAVISTREAM;

// AVI open flags
#define OF_READ         0x0000
#define OF_WRITE        0x0001
#define OF_CREATE       0x1000

// AVI stream types
#define streamtypeVIDEO mmioFOURCC('v','i','d','s')

// AVI stream flags
#define AVIIF_KEYFRAME  0x00000010

// Memory allocation flags
#define GMEM_MOVEABLE   0x0002

// AVI stream info structure
typedef struct _AVISTREAMINFO {
    DWORD fccType;
    DWORD fccHandler;
    DWORD dwFlags;
    DWORD dwCaps;
    WORD  wPriority;
    WORD  wLanguage;
    DWORD dwScale;
    DWORD dwRate;
    DWORD dwStart;
    DWORD dwLength;
    DWORD dwInitialFrames;
    DWORD dwSuggestedBufferSize;
    DWORD dwQuality;
    DWORD dwSampleSize;
    struct {
        short left;
        short top;
        short right;
        short bottom;
    } rcFrame;
    DWORD dwEditCount;
    DWORD dwFormatChangeCount;
    char  szName[64];
} AVIStreamInfo;

// BitmapInfoHeader alias (already defined in gdi_compat.h as BITMAPINFOHEADER)
#define BitmapInfoHeader BITMAPINFOHEADER

// Bitmap handle (already defined in gdi_compat.h as HBITMAP)
#define Bitmap HBITMAP

// mmioFOURCC macro - converts 4 characters to FOURCC code
#define mmioFOURCC(ch0, ch1, ch2, ch3) \
    ((DWORD)(uint8_t)(ch0) | ((DWORD)(uint8_t)(ch1) << 8) | \
     ((DWORD)(uint8_t)(ch2) << 16) | ((DWORD)(uint8_t)(ch3) << 24))

// VFW API stubs (all return failure/nullptr)
// TheSuperHackers @info These disable movie capture on Linux (Phase 3 feature)

static inline void AVIFileInit() {
    // Do nothing - no VFW initialization needed
}

static inline HRESULT AVIFileOpen(PAVIFILE* ppfile, const char* szFile, UINT uMode, void* lpHandler) {
    if (ppfile) *ppfile = nullptr;
    return 0x80004005; // E_FAIL
}

static inline HRESULT AVIFileCreateStream(PAVIFILE pfile, PAVISTREAM* ppavi, AVIStreamInfo* psi) {
    if (ppavi) *ppavi = nullptr;
    return 0x80004005; // E_FAIL
}

static inline HRESULT AVIStreamSetFormat(PAVISTREAM pavi, LONG lPos, void* lpFormat, LONG cbFormat) {
    return 0x80004005; // E_FAIL
}

static inline HRESULT AVIStreamWrite(PAVISTREAM pavi, LONG lStart, LONG lSamples, void* lpBuffer, LONG cbBuffer, DWORD dwFlags, LONG* plSampWritten, LONG* plBytesWritten) {
    return 0x80004005; // E_FAIL
}

static inline ULONG AVIStreamRelease(PAVISTREAM pavi) {
    return 0;
}

static inline ULONG AVIFileRelease(PAVIFILE pfile) {
    return 0;
}

static inline void AVIFileExit() {
    // Do nothing - no VFW cleanup needed
}

// GDI SetRect function (creates a rectangle)
static inline BOOL SetRect(RECT* lprc, int xLeft, int yTop, int xRight, int yBottom) {
    if (!lprc) return FALSE;
    lprc->left = xLeft;
    lprc->top = yTop;
    lprc->right = xRight;
    lprc->bottom = yBottom;
    return TRUE;
}

// Global memory allocation functions (return nullptr - movie capture disabled)
static inline void* GlobalAllocPtr(UINT uFlags, SIZE_T dwBytes) {
    return nullptr; // Fail allocation - disables movie capture
}

static inline void GlobalFreePtr(void* ptr) {
    // Do nothing - no memory was allocated
}

#endif // !_WIN32
