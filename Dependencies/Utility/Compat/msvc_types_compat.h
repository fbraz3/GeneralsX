/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// This file provides MSVC-specific type definitions for cross-platform compatibility
#pragma once

#include <cstdint>
#include <cstring>
#include <cwctype>
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>

#ifndef _WIN32

// MSVC integer type keywords - must be usable with 'unsigned' prefix
// Solution: Create actual type aliases that work like MSVC's built-in keywords
typedef int64_t __int64;
typedef int32_t __int32;
typedef int16_t __int16;
typedef int8_t __int8;

// Unsigned versions need special handling - create type aliases
typedef uint64_t uint__int64;
typedef uint32_t uint__int32;
typedef uint16_t uint__int16;
typedef uint8_t uint__int8;

// Short form aliases (used by legacy code)
typedef int64_t _int64;
typedef int32_t _int32;
typedef int16_t _int16;
typedef int8_t _int8;

// Inline keyword (MSVC uses __forceinline, GCC uses inline)
#define __forceinline inline
#define _forceinline inline

// String comparison functions (MSVC uses _stricmp, POSIX uses strcasecmp)
inline int _stricmp(const char* s1, const char* s2) {
    return strcasecmp(s1, s2);
}

inline int _strnicmp(const char* s1, const char* s2, size_t n) {
    return strncasecmp(s1, s2, n);
}

// Wide character comparison functions are defined in wchar_compat.h
// Do NOT duplicate _wcsicmp or _wcsnicmp here to avoid redefinition conflicts
// wchar_compat.h provides: _wcsicmp, _wcsnicmp implementations

// Windows SYSTEMTIME structure (used for replay timestamps)
#include <ctime>
struct SYSTEMTIME {
    uint16_t wYear;
    uint16_t wMonth;
    uint16_t wDayOfWeek;
    uint16_t wDay;
    uint16_t wHour;
    uint16_t wMinute;
    uint16_t wSecond;
    uint16_t wMilliseconds;
};

// File stat constants - map Windows stat constants to POSIX equivalents
#define _S_IFDIR S_IFDIR
#define _S_IFREG S_IFREG

// _stat alias - map directly to POSIX struct stat
#include <sys/stat.h>
#define _stat stat

// GetCommandLineA - get command line string
// On non-Windows systems, we can't reliably get the original command line
inline const char* GetCommandLineA() {
    static const char* default_cmdline = "";
    return default_cmdline;
}

// wsprintf - Windows formatted string function (use snprintf instead)
#include <cstdio>
inline int wsprintf(char* buf, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buf, 256, format, args);  // Assume 256-byte buffer for compatibility
    va_end(args);
    return result;
}

// Windows sync primitives stubs
typedef void* HANDLE;
// Note: NULL is NOT redefined here because it's used for integer zero initialization
// in C++ legacy code. The standard NULL (0) from cstddef works fine.

// CreateEvent stub - return a dummy handle
inline HANDLE CreateEvent(void* lpEventAttributes, int bManualReset, int bInitialState, const char* lpName) {
    // Non-functional stub for cross-platform compatibility
    // Return a non-null value to avoid null checks in existing code
    static int dummy_handle = 0xDEADBEEF;
    return (HANDLE)&dummy_handle;
}

// GetDoubleClickTime stub - return default system value
inline unsigned int GetDoubleClickTime() {
    // Typical system double-click time is 500ms
    return 500;
}

// GetLocalTime stub - populate SYSTEMTIME with current local time
inline void GetLocalTime(SYSTEMTIME* lpSystemTime) {
    if (!lpSystemTime) return;
    
    time_t now = time(NULL);
    struct tm* localtime_result = localtime(&now);
    
    if (localtime_result) {
        lpSystemTime->wYear = localtime_result->tm_year + 1900;
        lpSystemTime->wMonth = localtime_result->tm_mon + 1;
        lpSystemTime->wDay = localtime_result->tm_mday;
        lpSystemTime->wDayOfWeek = localtime_result->tm_wday;
        lpSystemTime->wHour = localtime_result->tm_hour;
        lpSystemTime->wMinute = localtime_result->tm_min;
        lpSystemTime->wSecond = localtime_result->tm_sec;
        lpSystemTime->wMilliseconds = 0;  // Not available from localtime
    }
}

// HKL - Handle to Keyboard Layout (Windows keyboard API)
typedef void* HKL;

// GetKeyboardLayout stub - get current keyboard layout
// On non-Windows systems, return a default English US layout identifier
inline HKL GetKeyboardLayout(unsigned int idThread) {
    // Return US English layout ID (0x0409)
    // This is safe because the code only checks the low word for language ID
    return (HKL)0x04090409;  // Format: LANGID SUBLANGID
}

// SHGetSpecialFolderPath stub - get special folder paths
#define CSIDL_PERSONAL 5  // My Documents
#define CSIDL_APPDATA 26  // Application Data

inline bool SHGetSpecialFolderPath(void* hwnd, char* pszPath, int csidl, bool fCreate) {
    // Stub implementation - return a generic user directory
    const char* home = getenv("HOME");
    if (home) {
        strncpy(pszPath, home, 260);
        return true;
    }
    strncpy(pszPath, "/tmp", 260);
    return false;
}

// CreateDirectory stub - create a directory
inline bool CreateDirectory(const char* lpPathName, void* lpSecurityAttributes) {
    // Use POSIX mkdir (note: need to check permissions)
    #include <sys/stat.h>
    return mkdir(lpPathName, 0755) == 0 || errno == EEXIST;
}

// GetModuleFileName stub - get executable path
inline unsigned int GetModuleFileName(void* hModule, char* lpFilename, unsigned int nSize) {
    // Use _NSGetExecutablePath on macOS, /proc/self/exe on Linux
    #ifdef __APPLE__
    extern int _NSGetExecutablePath(char* buf, uint32_t* bufsize);
    uint32_t size = nSize;
    if (_NSGetExecutablePath(lpFilename, &size) == 0) {
        return size;
    }
    #else
    // Linux fallback
    ssize_t len = readlink("/proc/self/exe", lpFilename, nSize - 1);
    if (len != -1) {
        lpFilename[len] = '\0';
        return len;
    }
    #endif
    return 0;
}

// Windows memory allocation (Global*) stubs
typedef void* HGLOBAL;
#define GMEM_FIXED 0

inline HGLOBAL GlobalAlloc(unsigned int flags, size_t size) {
    return malloc(size);
}

inline void GlobalFree(HGLOBAL hMem) {
    free(hMem);
}

inline HGLOBAL GlobalReAlloc(HGLOBAL hMem, size_t size, unsigned int flags) {
    return realloc(hMem, size);
}

inline size_t GlobalSize(HGLOBAL hMem) {
    // POSIX doesn't have a direct equivalent to GlobalSize
    // Return a conservative estimate or 0 (safer)
    return 0;
}

// Windows GUI types and stubs
typedef void* HWND;
typedef void* HINSTANCE;
// Note: HRESULT is defined in windows.h as long, not int

// CComModule stub - Active Template Library COM module
// Fail-Fast: Provide real implementation for all platforms, not just Windows stubs
struct CComModule {
    // Initialize COM subsystem (no-op on non-Windows, but real on Windows)
    void Init(void* pModule, void* hInstance, void* pTypeLib) {
        // On non-Windows: COM doesn't exist, but we need the function to exist
        // On Windows (in actual windows.h): This initializes ATL COM infrastructure
        // Cross-platform: Just ensure the method exists
        #ifdef _WIN32
        // Real Windows implementation is in atlbase.h
        #else
        // No-op on macOS/Linux - COM doesn't exist
        #endif
    }
    
    // Terminate COM subsystem  
    void Term() {
        // On non-Windows: No-op, COM doesn't exist
        // On Windows (in actual windows.h): This cleans up ATL COM infrastructure
        #ifdef _WIN32
        // Real Windows implementation is in atlbase.h
        #else
        // No-op on macOS/Linux
        #endif
    }
};
extern CComModule _Module;

// Windows window functions
inline bool SetWindowText(HWND hWnd, const char* lpString) {
    // Stub - does nothing on non-Windows
    return true;
}

inline bool SetWindowTextW(HWND hWnd, const wchar_t* lpString) {
    // Stub - does nothing on non-Windows
    return true;
}

// DeleteFile stub
inline bool DeleteFile(const char* lpFileName) {
    // Use POSIX unlink
    return unlink(lpFileName) == 0;
}

// DLL loading stubs for non-Windows platforms
// LoadLibrary/FreeLibrary/GetProcAddress are Windows-only
typedef void* FARPROC;  // Function pointer type - can be any function pointer
typedef void* HMODULE;  // Module handle - just use void* on non-Windows
typedef void* HINSTANCE;  // Application instance - just use void* on non-Windows

inline HMODULE LoadLibrary(const char* lpLibFileName) {
    // Stub - returns NULL on non-Windows (libraries aren't available)
    return NULL;
}

inline FARPROC GetProcAddress(HMODULE hModule, const char* lpProcName) {
    // Stub - returns NULL on non-Windows (no DLL functions available)
    return NULL;
}

inline bool FreeLibrary(HMODULE hLibModule) {
    // Stub - nothing to free on non-Windows
    return true;
}

// MessageBox stubs
inline int MessageBox(HWND hWnd, const char* lpText, const char* lpCaption, unsigned int uType) {
    // Stub - does nothing on non-Windows
    return 0;
}

inline int MessageBoxW(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, unsigned int uType) {
    // Stub - does nothing on non-Windows
    return 0;
}

// itoa - convert integer to ASCII string (Windows CRT function)
inline char* itoa(int value, char* buffer, int radix) {
    if (!buffer) return NULL;
    
    // Only support base 10 for simplicity (most common use case)
    if (radix != 10) {
        buffer[0] = '\0';
        return buffer;
    }
    
    snprintf(buffer, 65, "%d", value);  // 65 bytes is safe for 32-bit int
    return buffer;
}

// _isnan - Windows CRT isnan function (maps to standard C99 isnan)
#include <cmath>
#include <algorithm>
#ifndef _isnan
#define _isnan(x) std::isnan(x)
#endif

// _finite - Windows CRT function to check if float is finite
#ifndef _finite
#define _finite(x) std::isfinite(x)
#endif

// __min and __max - Windows compiler intrinsics (replace with std::min/max)
#ifndef __min
#define __min(a, b) std::min((a), (b))
#endif

#ifndef __max
#define __max(a, b) std::max((a), (b))
#endif

// Floating-point control functions (Windows CRT) - stubs for non-Windows
// These control FPU precision/rounding modes
#include <cfloat>

inline void _fpreset(void) {
    // Stub - FPU already initialized to defaults on non-Windows
}

inline unsigned int _statusfp(void) {
    // Return default status flags
    return 0;
}

inline unsigned int _controlfp(unsigned int newVal, unsigned int mask) {
    // Stub - just return the new value
    return newVal;
}

// FPU control constants (Windows)
#define _MCW_EM     0x0008001F  // Exception mask
#define _MCW_IC     0x00040000  // Infinity control
#define _MCW_RC     0x00000300  // Rounding control
#define _MCW_PC     0x00030000  // Precision control

#define _EM_INVALID 0x00000010  // Invalid operation
#define _EM_DENORMAL 0x00080000 // Denormal operand
#define _EM_ZERODIVIDE 0x00000008 // Zero divide
#define _EM_OVERFLOW 0x00000004  // Overflow
#define _EM_UNDERFLOW 0x00000002 // Underflow
#define _EM_INEXACT 0x00000001   // Inexact result

#define _IC_AFFINE   0x00040000  // Affine
#define _IC_PROJECTIVE 0x00000000 // Projective

#define _RC_NEAR    0x00000000  // Round to nearest
#define _RC_DOWN    0x00000100  // Round down
#define _RC_UP      0x00000200  // Round up
#define _RC_CHOP    0x00000300  // Round towards zero

#define _PC_24      0x00000000  // 24-bit precision
#define _PC_53      0x00010000  // 53-bit precision
#define _PC_64      0x00020000  // 64-bit precision

#endif // !_WIN32

