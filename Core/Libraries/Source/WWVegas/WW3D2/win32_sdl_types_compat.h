#pragma once

/**
 * @file win32_types.h
 * @brief Windows type definitions and compatibility layer for cross-platform support
 * 
 * This header provides minimal Windows type definitions needed for non-Windows
 * platforms (macOS, Linux) to compile code that uses Windows API types.
 * 
 * On Windows, these types are provided by windows.h and windef.h.
 * On macOS/Linux, we provide compatible stubs to avoid Windows SDK dependency.
 * 
 * Phase 01: DirectX 8 Compatibility Layer
 * Acceptance Criteria: AC2 - All Win32 types defined and functional
 */

#ifndef WIN32_TYPES_H_INCLUDED
#define WIN32_TYPES_H_INCLUDED

#include <cstdint>
#include <cstring>

// =====================================================================
// Compiler Compatibility Macros
// =====================================================================

#ifndef CONST
#define CONST const
#endif


/** @brief Signed long (32-bit) */
#ifndef LONG
typedef long LONG;
#endif

/** @brief Unsigned long (32-bit) */
#ifndef DWORD
typedef unsigned long DWORD;
#endif

/** @brief Unsigned short (16-bit) */
#ifndef WORD
typedef unsigned short WORD;
#endif

/** @brief Unsigned char (8-bit) */
#ifndef BYTE
typedef unsigned char BYTE;
#endif

/** @brief Signed 64-bit integer */
#ifndef __int64
typedef long long __int64;
#endif

/** @brief Unsigned integer (32-bit) */
#ifndef UINT
typedef unsigned int UINT;
#endif

/** @brief Float type (32-bit) */
#ifndef FLOAT
typedef float FLOAT;
#endif

/** @brief Boolean type (32-bit DWORD) */
#ifndef BOOL
#ifdef __GNUC__
// On GCC/Clang, bittype.h may define BOOL as int; use compatible guard
typedef int BOOL;
#else
typedef DWORD BOOL;
#endif

/** @brief Signed char (8-bit) */
#ifndef CHAR
typedef char CHAR;
#endif

/** @brief Unsigned long long (64-bit) */
#ifndef DWORD64
typedef unsigned long long DWORD64;
#endif

/** @brief Signed long long (64-bit) */
#ifndef LONGLONG
typedef long long LONGLONG;
#endif

#endif // !_WIN32

// =====================================================================
// Windows Pointer Types
// =====================================================================


/** @brief Window handle pointer */
#ifndef HWND
typedef void* HWND;
#endif

/** @brief Instance handle pointer */
#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif

/** @brief Module handle pointer */
#ifndef HMODULE
typedef HINSTANCE HMODULE;
#endif

/** @brief Device context handle pointer */
#ifndef HDC
typedef void* HDC;
#endif

/** @brief Menu handle pointer */
#ifndef HMENU
typedef void* HMENU;
#endif

/** @brief Icon handle pointer */
#ifndef HICON
typedef void* HICON;
#endif

/** @brief Cursor handle pointer */
#ifndef HCURSOR
typedef void* HCURSOR;
#endif

/** @brief Font handle pointer */
#ifndef HFONT
typedef void* HFONT;
#endif

/** @brief Bitmap handle pointer */
#ifndef HBITMAP
typedef void* HBITMAP;
#endif

/** @brief Far procedure pointer (legacy 16-bit) */
#ifndef FARPROC
typedef void* FARPROC;
#endif

/** @brief Generic pointer to unsigned char */
#ifndef LPBYTE
typedef BYTE* LPBYTE;
#endif


// =====================================================================
// Windows Message Types
// =====================================================================


/* Message types: WPARAM and LPARAM for message handlers */
typedef intptr_t WPARAM;                       ///< Unsigned parameter (use as integer or pointer)
typedef intptr_t LPARAM;                       ///< Signed parameter (use as integer or pointer)


// =====================================================================
// Boolean Constants
// =====================================================================

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// =====================================================================
// HRESULT - COM Error Handling
// =====================================================================


/** @brief COM result/error code type */
#ifndef HRESULT
typedef long HRESULT;
#endif

// Standard HRESULT macros and error codes
#define S_OK                          ((HRESULT)0x00000000L)
#define S_FALSE                       ((HRESULT)0x00000001L)
#define E_FAIL                        ((HRESULT)0x80004005L)
#define E_NOTIMPL                     ((HRESULT)0x80004001L)
#define E_NOINTERFACE                 ((HRESULT)0x80004002L)
#define E_POINTER                     ((HRESULT)0x80004003L)
#define E_ABORT                       ((HRESULT)0x80004004L)
#define E_INVALIDARG                  ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY                 ((HRESULT)0x8007000EL)
#define E_ACCESSDENIED                ((HRESULT)0x80070005L)

// HRESULT helper macros
#define SUCCEEDED(hr)                 (((HRESULT)(hr)) >= 0)
#define FAILED(hr)                    (((HRESULT)(hr)) < 0)
#define HRESULT_CODE(hr)              ((hr) & 0xFFFF)
#define HRESULT_FACILITY(hr)          (((hr) >> 16) & 0x1FFF)
#define HRESULT_SEVERITY(hr)          (((hr) >> 31) & 0x1)


// =====================================================================
// RECT - Rectangle Structure (Geometry)
// =====================================================================

#ifndef RECT_DEFINED
#define RECT_DEFINED

/** @brief Rectangle structure for screen coordinates */
typedef struct {
    LONG left;      /**< Left edge coordinate */
    LONG top;       /**< Top edge coordinate */
    LONG right;     /**< Right edge coordinate */
    LONG bottom;    /**< Bottom edge coordinate */
} RECT;

/** @brief Pointer to RECT structure */
typedef RECT* PRECT;
typedef RECT* LPRECT;

#endif

// =====================================================================
// POINT - Point Structure (Geometry)
// =====================================================================

#ifndef POINT_DEFINED
#define POINT_DEFINED

/** @brief Point structure for 2D coordinates */
typedef struct {
    LONG x;         /**< X coordinate */
    LONG y;         /**< Y coordinate */
} POINT;

/** @brief Pointer to POINT structure */
typedef POINT* PPOINT;
typedef POINT* LPPOINT;

#endif

// =====================================================================
// SIZE - Size Structure (Dimensions)
// =====================================================================

#ifndef SIZE_DEFINED
#define SIZE_DEFINED

/** @brief Size structure for width and height */
typedef struct {
    LONG cx;        /**< Width/horizontal extent */
    LONG cy;        /**< Height/vertical extent */
} SIZE;

/** @brief Pointer to SIZE structure */
typedef SIZE* PSIZE;
typedef SIZE* LPSIZE;

#endif

// =====================================================================
// Color Type
// =====================================================================

#ifndef COLORREF_DEFINED
#define COLORREF_DEFINED

/** @brief Color reference (ARGB format on Windows: 0x00BBGGRR) */
typedef DWORD COLORREF;

/** @brief Create RGB color from components: 0x00BBGGRR format */
#define RGB(r,g,b)    ((COLORREF)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))

/** @brief Get red component from COLORREF */
#define GetRValue(rgb) ((BYTE)(rgb))

/** @brief Get green component from COLORREF */
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))

/** @brief Get blue component from COLORREF */
#define GetBValue(rgb) ((BYTE)((rgb)>>16))

#endif

// =====================================================================
// Compiler Attribute Compatibility
// =====================================================================


/** @brief __declspec compatibility macro for non-Windows platforms */
#ifndef __declspec
#define __declspec(x)
#endif

/** @brief STDCALL convention (Windows x86 calling convention) - no-op on other platforms */
#ifndef STDCALL
#define STDCALL
#endif

/** @brief CDECL convention (C default calling convention) - no-op on other platforms */
#ifndef CDECL
#define CDECL
#endif

/** @brief WINAPI convention (typically STDCALL) - no-op on other platforms */
#ifndef WINAPI
#define WINAPI
#endif

/** @brief CALLBACK convention for callbacks */
#ifndef CALLBACK
#define CALLBACK
#endif

/** @brief APIENTRY for API function declarations */
#ifndef APIENTRY
#define APIENTRY
#endif


// =====================================================================
// Bitmap and Image Structures
// =====================================================================

#ifndef BITMAPINFOHEADER_DEFINED
#define BITMAPINFOHEADER_DEFINED

/** @brief Bitmap information header (DIB format) */
typedef struct {
    DWORD biSize;               /**< Structure size in bytes */
    LONG biWidth;               /**< Image width in pixels */
    LONG biHeight;              /**< Image height in pixels (positive = bottom-up, negative = top-down) */
    WORD biPlanes;              /**< Number of planes (must be 1) */
    WORD biBitCount;            /**< Bits per pixel (1, 4, 8, 16, 24, or 32) */
    DWORD biCompression;        /**< Compression type (BI_RGB=0, BI_RLE8, BI_RLE4, BI_BITFIELDS) */
    DWORD biSizeImage;          /**< Image size in bytes (can be 0 for uncompressed) */
    LONG biXPelsPerMeter;       /**< Horizontal resolution in pixels per meter */
    LONG biYPelsPerMeter;       /**< Vertical resolution in pixels per meter */
    DWORD biClrUsed;            /**< Number of color indices used (0 = all) */
    DWORD biClrImportant;       /**< Number of important colors (0 = all) */
} BITMAPINFOHEADER;

/** @brief Pointer to BITMAPINFOHEADER */
typedef BITMAPINFOHEADER* PBITMAPINFOHEADER;
typedef BITMAPINFOHEADER* LPBITMAPINFOHEADER;

#endif

#ifndef BITMAPFILEHEADER_DEFINED
#define BITMAPFILEHEADER_DEFINED

/** @brief Bitmap file header (DIB file format) */
typedef struct {
    WORD bfType;                /**< File type ("BM" = 0x4D42) */
    DWORD bfSize;               /**< File size in bytes */
    WORD bfReserved1;           /**< Reserved (must be 0) */
    WORD bfReserved2;           /**< Reserved (must be 0) */
    DWORD bfOffBits;            /**< Offset to pixel data from start of file */
} BITMAPFILEHEADER;

/** @brief Pointer to BITMAPFILEHEADER */
typedef BITMAPFILEHEADER* PBITMAPFILEHEADER;
typedef BITMAPFILEHEADER* LPBITMAPFILEHEADER;

#endif

#ifndef RGBQUAD_DEFINED
#define RGBQUAD_DEFINED

/** @brief RGB quad structure (palette entry) */
typedef struct {
    BYTE rgbBlue;               /**< Blue component (0-255) */
    BYTE rgbGreen;              /**< Green component (0-255) */
    BYTE rgbRed;                /**< Red component (0-255) */
    BYTE rgbReserved;           /**< Reserved (usually 0) */
} RGBQUAD;

/** @brief Pointer to RGBQUAD */
typedef RGBQUAD* PRGBQUAD;
typedef RGBQUAD* LPRGBQUAD;

#endif

#ifndef BITMAPINFO_DEFINED
#define BITMAPINFO_DEFINED

/** @brief Complete bitmap information structure */
typedef struct {
    BITMAPINFOHEADER bmiHeader; /**< Bitmap header information */
    RGBQUAD bmiColors[1];       /**< Palette entries (flexible array) */
} BITMAPINFO;

/** @brief Pointer to BITMAPINFO */
typedef BITMAPINFO* PBITMAPINFO;
typedef BITMAPINFO* LPBITMAPINFO;

#endif

// =====================================================================
// File and Directory Constants
// =====================================================================


/** @brief File attributes - directory */
#define FILE_ATTRIBUTE_DIRECTORY      0x00000010

/** @brief File attributes - normal file */
#define FILE_ATTRIBUTE_NORMAL         0x00000080

/** @brief File attributes - read-only */
#define FILE_ATTRIBUTE_READONLY       0x00000001

/** @brief File attributes - hidden */
#define FILE_ATTRIBUTE_HIDDEN         0x00000002

/** @brief File attributes - system */
#define FILE_ATTRIBUTE_SYSTEM         0x00000004


// =====================================================================
// Format and Locale Constants
// =====================================================================


/** @brief Short date format (MM/DD/YY) */
#define DATE_SHORTDATE                0x00000001

/** @brief Time format without seconds */
#define TIME_NOSECONDS                0x00000002

/** @brief Force 24-hour time format */
#define TIME_FORCE24HOURFORMAT        0x00000008

/** @brief Don't include time marker (AM/PM) */
#define TIME_NOTIMEMARKER             0x00000004

/** @brief System default locale ID */
#define LOCALE_SYSTEM_DEFAULT         0x0800

/** @brief User default locale ID */
#define LOCALE_USER_DEFAULT           0x0400


// =====================================================================
// COM Interface Support (Stub)
// =====================================================================


/** @brief COM uninitialization stub - no-op on non-Windows platforms */
#define CoUninitialize()              do { } while(0)

/** @brief COM initialization stub - no-op on non-Windows platforms */
#define CoInitialize(pvReserved)      S_OK

/** @brief COM multithread initialization stub - no-op on non-Windows platforms */
#define CoInitializeEx(pvReserved, dwCoInit) S_OK


// =====================================================================
// Memory Allocation Flags (Compatibility)
// =====================================================================


/** @brief Local memory - fixed allocation */
#define LMEM_FIXED                    0x0000

/** @brief Local memory - movable allocation */
#define LMEM_MOVEABLE                 0x0002

/** @brief Local memory - initialize to zero */
#define LMEM_ZEROINIT                 0x0040

/** @brief Combined fixed + zero init flags */
#define LPTR                          (LMEM_FIXED | LMEM_ZEROINIT)


// =====================================================================
// DIB (Device Independent Bitmap) Constants
// =====================================================================


/** @brief DIB uses RGB color table */
#define DIB_RGB_COLORS                0

/** @brief DIB uses palette indices */
#define DIB_PAL_COLORS                1

/** @brief Compression type: no compression */
#define BI_RGB                        0

/** @brief Compression type: RLE 8-bit */
#define BI_RLE8                       1

/** @brief Compression type: RLE 4-bit */
#define BI_RLE4                       2

/** @brief Compression type: bitfield compression */
#define BI_BITFIELDS                  3

// =====================================================================
// Windows Path and String Limits
// =====================================================================

/** @brief Maximum path length for Windows paths */
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// =====================================================================
// String functions (ANSI versions mapped to standard C functions)
// =====================================================================

#ifndef lstrcpyn
#define lstrcpyn(dest, src, n) strncpy((dest), (src), (n))
#endif

#ifndef lstrcat
#define lstrcat(dest, src) strcat((dest), (src))
#endif

#ifndef _strdup
#define _strdup(s) strdup((s))
#endif

// Phase 02: String function wrappers (lstrcpy, lstrlen, lstrcmpi)
#ifndef lstrcpy
#define lstrcpy(dest, src) strcpy((dest), (src))
#endif

#ifndef lstrlen
#define lstrlen(s) strlen((s))
#endif

#ifndef lstrcmpi
#define lstrcmpi(s1, s2) strcasecmp((s1), (s2))
#endif

#ifndef _isnan
#define _isnan(x) isnan((x))
#endif

// =====================================================================
// DirectDraw Surface (DDS) Capability Flags
// =====================================================================

#ifndef DDSCAPS2_CUBEMAP
#define DDSCAPS2_CUBEMAP           0x00000200  ///< Surface is a cube map
#endif

#ifndef DDSCAPS2_VOLUME
#define DDSCAPS2_VOLUME            0x00200000  ///< Surface is a volume texture
#endif


#endif // WIN32_TYPES_H_INCLUDED

