#pragma once

#include <stdint.h>
#include <stddef.h>

// GeneralsX @build BenderAI 28/02/2026
// NOTE: bittype.h from WWLib is the authoritative source for basic Win32 types
// (DWORD, ULONG, UINT, BYTE, BOOL, WORD, USHORT, LPCSTR).
// DO NOT redefine here to avoid MSVC "redefinition; types basic differ" errors.
// types_compat.h ONLY defines Windows-specific types that bittype.h doesn't have.

// GeneralsX @build BenderAI 10/02/2026
// Removed duplicate DWORD/ULONG/UINT/BYTE/BOOL/WORD/USHORT/LPCSTR typedefs
// All of these are already in bittype.h which is included first in windows_compat.h

// GeneralsX @bugfix fbraz3 04/03/2026 Define basic Win32 types for non-Windows platforms.
// On Windows these come from bittype.h (via windows_compat.h), but headers such as
// time_compat.h are included directly without windows_compat.h, so DWORD/BOOL/etc.
// would be undefined. Use #ifndef guards to avoid conflicts when bittype.h is
// included later in the same translation unit.
#ifndef _WIN32
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#ifndef WORD
typedef uint16_t WORD;
#endif
#ifndef BYTE
typedef uint8_t BYTE;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef ULONG
typedef unsigned long ULONG;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef USHORT
typedef uint16_t USHORT;
#endif

// GDI color reference (0x00BBGGRR format) - not in bittype.h
#ifndef COLORREF
typedef uint32_t COLORREF;
#endif

// Unicode string types - not in bittype.h
#ifndef LPCWSTR
typedef const wchar_t* LPCWSTR;
#endif
#endif // !_WIN32

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
// GeneralsX @build BenderAI 26/05/2026 - Guard with #ifndef _WIN32:
// On Windows: windef.h (via windows.h) provides DECLARE_HANDLE, HANDLE, and all H* types.
// Redefining them here causes C2371 "redefinition; different basic types" conflicts.
#ifndef _WIN32
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

typedef int32_t *LPARAM;
typedef size_t WPARAM;

// Pointer-sized unsigned integer for 64-bit safe pointer arithmetic
typedef size_t SIZE_T;

// COM/OLE types (for WOL browser support)
// LPDISPATCH is IDispatch* - COM interface pointer for runtime method dispatch
#ifndef LPDISPATCH
typedef void *LPDISPATCH;
#endif

#ifndef S_OK
#define S_OK 0
#endif

// Microsoft direction specifiers, used purely for documentation purposes
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#endif // !_WIN32

// TheSuperHackers @build 10/02/2026 Bender
// 64-bit convenience aliases (not in Windows SDK or bittype.h by these names)
// Used by game code on both Windows and Linux builds.
typedef int64_t int64;
typedef uint64_t uint64;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef _WIN32
#define _isnan isnan
#define _finite isfinite
#endif