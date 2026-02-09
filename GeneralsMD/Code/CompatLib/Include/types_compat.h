#pragma once

#include <stdint.h>
#include <stddef.h>

typedef void *HANDLE;
typedef HANDLE HWND;
typedef HANDLE HINSTANCE;
typedef HANDLE HKEY;
typedef HANDLE HDC;
typedef int32_t HRESULT;

typedef int BOOL;
typedef unsigned char BYTE;

#define INVALID_HANDLE_VALUE ((HANDLE)-1)

typedef void *LPVOID;

typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint32_t DWORD;

typedef uint32_t UINT;
// 64-bit integer types
typedef int64_t _int64;
typedef uint64_t _uint64;
typedef int64_t int64;
typedef uint64_t uint64;
typedef int32_t *LPARAM;
typedef size_t WPARAM;

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