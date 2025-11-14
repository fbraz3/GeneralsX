#pragma once

// Minimal windows.h stub for non-Windows builds.
// Provides a few basic types used by legacy code so compilation can proceed.

#ifndef _WINDOWS_H_STUB
#define _WINDOWS_H_STUB

typedef unsigned long DWORD;
typedef int BOOL;
typedef void* HANDLE;
#define TRUE 1
#define FALSE 0

#ifndef WINAPI
#define WINAPI
#endif

// Basic RECT/POINT types (only if not already defined by win32_types.h or win32_sdl_types_compat.h)
#if !defined(RECT_DEFINED) && !defined(RECT)
typedef struct tagRECT { long left; long top; long right; long bottom; } RECT;
#define RECT_DEFINED
#endif

#if !defined(POINT_DEFINED) && !defined(POINT)
typedef struct tagPOINT { long x; long y; } POINT;
#define POINT_DEFINED
#endif

// Minimal HRESULT-like defines
typedef long HRESULT;
#ifndef S_OK
#define S_OK ((HRESULT)0)
#endif
#ifndef D3D_OK
#define D3D_OK S_OK
#endif

// Placeholder macros used by code
#ifndef CALLBACK
#define CALLBACK
#endif

#endif // _WINDOWS_H_STUB
