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

// Phase 40: macOS system header for SDL2_GetModuleFilePath
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

// Cross-platform event system using SDL2 (threading primitives: CreateEvent, SetEvent, WaitForSingleObject)
// Note: SDL2_Events.h is included AFTER HANDLE/DWORD/BOOL typedefs below to avoid circular dependency

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
typedef int BOOL;                   // Boolean type (int for Win32 compatibility)
typedef unsigned long DWORD;        // 32-bit unsigned integer
// Note: NULL is NOT redefined here because it's used for integer zero initialization
// in C++ legacy code. The standard NULL (0) from cstddef works fine.

// CreateEvent, SetEvent, ResetEvent, WaitForSingleObject - SDL2 implementations
// Implementation: SDL2_Events.h (included above for non-Windows platforms)
// On Windows: Native Win32 HANDLE-based implementations from windows.h
// Cross-platform: Use SDL2 event system with SDL_mutex and SDL_cond

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

// GetAsyncKeyState - get keyboard key state via SDL2
// Returns SHORT: high bit (0x8000) set if key is pressed, low bit (0x0001) set if key was toggled
// Used by: W3DWaterTracks.cpp (debug features: F5-F8, DELETE, INSERT), WorldBuilder tools (SHIFT, CONTROL)
#include <SDL2/SDL.h>
inline short GetAsyncKeyState(int vKey) {
    // Get SDL2 keyboard state
    const uint8_t* keyboard_state = SDL_GetKeyboardState(NULL);
    if (!keyboard_state) return 0;
    
    // Map virtual key to SDL scancode (based on win32_sdl_api_compat.h VK_* definitions)
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    
    switch (vKey) {
        case 0x08: scancode = SDL_SCANCODE_BACKSPACE; break;  // VK_BACK
        case 0x09: scancode = SDL_SCANCODE_TAB; break;        // VK_TAB
        case 0x0D: scancode = SDL_SCANCODE_RETURN; break;     // VK_RETURN
        case 0x10: scancode = SDL_SCANCODE_LSHIFT; break;     // VK_SHIFT (maps to left shift)
        case 0x11: scancode = SDL_SCANCODE_LCTRL; break;      // VK_CONTROL (maps to left ctrl)
        case 0x12: scancode = SDL_SCANCODE_LALT; break;       // VK_MENU (ALT key)
        case 0x20: scancode = SDL_SCANCODE_SPACE; break;      // VK_SPACE
        case 0x21: scancode = SDL_SCANCODE_PAGEUP; break;     // VK_PRIOR
        case 0x22: scancode = SDL_SCANCODE_PAGEDOWN; break;   // VK_NEXT
        case 0x23: scancode = SDL_SCANCODE_END; break;        // VK_END
        case 0x24: scancode = SDL_SCANCODE_HOME; break;       // VK_HOME
        case 0x25: scancode = SDL_SCANCODE_LEFT; break;       // VK_LEFT
        case 0x26: scancode = SDL_SCANCODE_UP; break;         // VK_UP
        case 0x27: scancode = SDL_SCANCODE_RIGHT; break;      // VK_RIGHT
        case 0x28: scancode = SDL_SCANCODE_DOWN; break;       // VK_DOWN
        case 0x2D: scancode = SDL_SCANCODE_INSERT; break;     // VK_INSERT
        case 0x2E: scancode = SDL_SCANCODE_DELETE; break;     // VK_DELETE
        case 0x70: scancode = SDL_SCANCODE_F1; break;         // VK_F1
        case 0x71: scancode = SDL_SCANCODE_F2; break;         // VK_F2
        case 0x72: scancode = SDL_SCANCODE_F3; break;         // VK_F3
        case 0x73: scancode = SDL_SCANCODE_F4; break;         // VK_F4
        case 0x74: scancode = SDL_SCANCODE_F5; break;         // VK_F5
        case 0x75: scancode = SDL_SCANCODE_F6; break;         // VK_F6
        case 0x76: scancode = SDL_SCANCODE_F7; break;         // VK_F7
        case 0x77: scancode = SDL_SCANCODE_F8; break;         // VK_F8
        case 0x78: scancode = SDL_SCANCODE_F9; break;         // VK_F9
        case 0x79: scancode = SDL_SCANCODE_F10; break;        // VK_F10
        case 0x7A: scancode = SDL_SCANCODE_F11; break;        // VK_F11
        case 0x7B: scancode = SDL_SCANCODE_F12; break;        // VK_F12
        default: return 0;  // Unmapped key
    }
    
    // Return high bit set (0x8000) if key is currently pressed
    // Low bit (0x0001) is not implemented (toggle state requires event tracking)
    if (keyboard_state[scancode]) {
        return 0x8000;  // High bit set = key pressed
    }
    return 0;  // Key not pressed
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
// Phase 40: Now using SDL2_GetModuleFilePath instead of this stub
// Keeping this for backwards compatibility but no longer used in new code
inline unsigned int GetModuleFileName(void* hModule, char* lpFilename, unsigned int nSize) {
    // This stub is deprecated - use SDL2_GetModuleFilePath() instead
    // See Core/GameEngine/Source/Common/System/SDL2_AppWindow.cpp
    
    #ifdef __APPLE__
    // macOS: Use system call directly (declaration is in <mach-o/dyld.h> included at top of file)
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

// MessageBox constants - Windows API compatibility
// Phase 40: SDL2 migration requires these constants for MessageBoxWrapper
#define MB_OK                   0x00000000
#define MB_OKCANCEL             0x00000001
#define MB_ABORTRETRYIGNORE     0x00000002
#define MB_YESNOCANCEL          0x00000003
#define MB_YESNO                0x00000004
#define MB_RETRYCANCEL          0x00000005
#define MB_CANCELTRYCONTINUE    0x00000006

// MessageBox icons
#define MB_ICONHAND             0x00000010  // ICONERROR
#define MB_ICONERROR            0x00000010
#define MB_ICONQUESTION         0x00000020
#define MB_ICONWARNING          0x00000030  // ICONEXCLAMATION
#define MB_ICONEXCLAMATION      0x00000030
#define MB_ICONASTERISK         0x00000040  // ICONINFORMATION
#define MB_ICONINFORMATION      0x00000040

// MessageBox modality
#define MB_APPLMODAL            0x00000000
#define MB_SYSTEMMODAL          0x00001000
#define MB_TASKMODAL            0x00002000

// MessageBox default buttons
#define MB_DEFBUTTON1           0x00000000
#define MB_DEFBUTTON2           0x00000100
#define MB_DEFBUTTON3           0x00000200
#define MB_DEFBUTTON4           0x00000300

// MessageBox misc
#define MB_SETFOREGROUND        0x00010000
#define MB_TOPMOST              0x00040000
#define MB_RIGHT                0x00080000
#define MB_RTLREADING           0x00100000
#define MB_NOFOCUS              0x00008000

// MessageBox return codes
#define IDOK                    1
#define IDCANCEL                2
#define IDABORT                 3
#define IDRETRY                 4
#define IDIGNORE                5
#define IDYES                   6
#define IDNO                    7
#define IDTRYAGAIN              10
#define IDCONTINUE              11

// Include SDL2 event system AFTER HANDLE/DWORD/BOOL typedefs are defined
#include "SDL2_Events.h"

#endif // !_WIN32

