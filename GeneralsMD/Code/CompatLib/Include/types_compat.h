#pragma once

#include <stdint.h>
#include <stddef.h>

// GeneralsX @build BenderAI 10/02/2026
// Basic Win32 types - CompatLib must be self-contained
// These MAY also be defined in bittype.h, but CompatLib compiles standalone
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#ifndef ULONG
typedef uint32_t ULONG;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef BYTE
typedef uint8_t BYTE;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef WORD
typedef uint16_t WORD;
#endif
#ifndef USHORT
typedef uint16_t USHORT;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef LPCWSTR
typedef const wchar_t* LPCWSTR;
#endif

// GeneralsX @build fbraz 11/02/2026 BenderAI - GDI color reference (0x00BBGGRR format)
#ifndef COLORREF
typedef DWORD COLORREF;
#endif

// GeneralsX @build fbraz 10/02/2026
// ARRAY_SIZE macro - get element count of static array
// Used by WWDownload/FTP.cpp
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// This header COMPLEMENTS bittype.h from WWLib, which already defines:
// DWORD, ULONG, UINT, BYTE, BOOL, WORD, USHORT, LPCSTR
// We only define Windows-specific types that bittype.h doesn't have

// DECLARE_HANDLE macro (from windef.h) - creates opaque handle types
#ifndef DECLARE_HANDLE
#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#else
typedef void *HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
#endif

#ifndef _HANDLE_TYPES_DEFINED
#define _HANDLE_TYPES_DEFINED
typedef void *HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HANDLE HKEY;
typedef HANDLE HDC;
typedef HANDLE HFONT;     // GeneralsX @build BenderAI 10/02/2026 - GDI font handle
typedef HANDLE HBITMAP;   // GeneralsX @build BenderAI 10/02/2026 - GDI bitmap handle
typedef HANDLE HGDIOBJ;   // GeneralsX @build fbraz 11/02/2026 BenderAI - Generic GDI object handle
typedef HANDLE HMONITOR;  // Monitor handle (multi-monitor APIs)
typedef int32_t HRESULT;
#endif /* _HANDLE_TYPES_DEFINED */

#define INVALID_HANDLE_VALUE ((HANDLE)-1)

typedef void *LPVOID;

typedef int32_t LONG;

// DirectX types not in bittype.h
typedef float FLOAT;

// Microsoft const specifier (DirectX uses it extensively)
#ifndef CONST
#define CONST const
#endif

// Note: RGNDATA provided by DXVK windows_base.h on Linux
// (included via d3d8.h → windows.h). Windows SDK provides on Windows builds.

// TheSuperHackers @build 10/02/2026 Bender
// 64-bit types now defined in bittype.h (_int64, __int64, _uint64, __uint64)
// Removed duplicate definitions to avoid conflicts
// typedef int64_t _int64;   // REMOVED - use bittype.h
// typedef uint64_t _uint64; // REMOVED - use bittype.h
typedef int64_t int64;
typedef uint64_t uint64;

typedef int32_t *LPARAM;
typedef size_t WPARAM;

// GeneralsX @build BenderAI 10/02/2026 - Pointer-sized unsigned integer for 64-bit safe pointer arithmetic
typedef size_t SIZE_T;

// GeneralsX @build fbraz 12/02/2026 BenderAI - COM/OLE types (for WOL browser support)
// LPDISPATCH is IDispatch* - COM interface pointer for runtime method dispatch
// On Linux, used only for dx8webbrowser CreateBrowser parameter stub
#ifndef LPDISPATCH
typedef void *LPDISPATCH;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define S_OK 0

// Microsoft direction specifiers, used purely for documentation purposes
#define IN
#define OUT

#define _isnan isnan
#define _finite isfinite