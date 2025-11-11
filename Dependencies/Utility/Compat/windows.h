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

// Basic RECT/POINT types
typedef struct tagRECT { long left; long top; long right; long bottom; } RECT;
typedef struct tagPOINT { long x; long y; } POINT;

// Minimal HRESULT-like defines
typedef long HRESULT;
#define S_OK ((HRESULT)0)
#define D3D_OK S_OK

// Placeholder macros used by code
#ifndef CALLBACK
#define CALLBACK
#endif

#endif // _WINDOWS_H_STUB
