#pragma once

#ifndef CALLBACK
#define CALLBACK
#endif

#if !defined(__stdcall)
#define __stdcall
#endif

#ifndef WINAPI
#define WINAPI
#endif

#if !defined _MSC_VER
#if !defined(__cdecl)
#if defined __has_attribute && __has_attribute(cdecl) && defined(__i386__)
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif /* !defined __cdecl */
#endif

#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif

static unsigned int GetDoubleClickTime()
{
  return 500;
}

// CRITICAL: bittype.h from WWLib defines basic Win32 types (DWORD, BYTE, UINT, etc.)
// Must be included BEFORE d3d8.h or any DirectX headers
#include "bittype.h"

// types_compat.h now complements bittype.h instead of conflicting
// bittype.h defines basic types (DWORD, ULONG, etc.)
// types_compat.h adds Windows-specific types (HANDLE, HWND, etc.)
#include "types_compat.h"

// TheSuperHackers @build fighter19 10/02/2026 Bender - Win32 window management API stubs
#include "wnd_compat.h"

// const void* type (Windows API)
#ifndef LPCVOID
typedef const void *LPCVOID;
#endif

// FAILED macro (DirectX HRESULT checking)
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif

// Note: PALETTEENTRY, LARGE_INTEGER, RGNDATA provided by DXVK windows_base.h on Linux
// (included via d3d8.h). These types are Windows SDK on Windows builds.

#define HIWORD(value) ((((uint32_t)(value) >> 16) & 0xFFFF))
#define LOWORD(value) (((uint32_t)(value) & 0xFFFF))

// Copied from windows_base.h from DXVK-Native
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev, fac, code) \
	((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif

#ifndef _MAX_DIR
#define _MAX_DIR _MAX_PATH
#endif

#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif

// TheSuperHackers @build 10/02/2026 Bender
// Threading functions: GetCurrentThreadId() and Sleep() for Linux
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

static inline int GetCurrentThreadId()
{
    return (int)pthread_self();
}

static inline void Sleep(int milliseconds)
{
    usleep(milliseconds * 1000);
}

// TheSuperHackers @build 10/02/2026 Bender
// Timing functions: timeGetTime(), timeBeginPeriod(), timeEndPeriod() for Linux
static inline DWORD timeGetTime(void)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (DWORD)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

static inline DWORD timeBeginPeriod(DWORD period)
{
    // NOOP on Linux (timer resolution is kernel controlled)
    return 0;
}

static inline DWORD timeEndPeriod(DWORD period)
{
    // NOOP on Linux
    return 0;
}

// Only include compat headers if old Dependencies/Utility/Utility/compat.h hasn't been included
// to avoid conflicts between old and new compat systems
#ifndef DEPENDENCIES_UTILITY_COMPAT_H
// Threading functions now defined above, no need for thread_compat.h
// #include "thread_compat.h"  // TheSuperHackers @build 10/02/2026 Bender - Commented out (functions now inline above)
#include "tchar_compat.h"
#include "time_compat.h"
#include "string_compat.h"
#include "keyboard_compat.h"
#include "memory_compat.h"
#include "module_compat.h"
#include "wchar_compat.h"
#include "gdi_compat.h"
#include "wnd_compat.h"
#include "file_compat.h"
#include "socket_compat.h"  // TheSuperHackers @build fbraz 10/02/2026 - Win32 Sockets → POSIX BSD sockets (WWDownload)
//#include "intrin_compat.h"
#endif
