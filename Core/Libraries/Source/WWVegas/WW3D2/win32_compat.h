#pragma once
#include <cstring>
#include <cstdio>
#include <cstdint>

// Basic Windows typese

#ifndef WIN32_COMPAT_H_INCLUDED
#define WIN32_COMPAT_H_INCLUDED

#ifndef _WIN32
// Compatibility types for non-Windows systems
#include <cstring>
#include <cstdlib>

// Basic Windows types
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HKEY;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef uintptr_t DWORD_PTR;
typedef int BOOL;
typedef long LONG;
typedef void* LPVOID;
typedef float FLOAT;

// Basic Windows structures
typedef struct {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef struct {
    LONG x;
    LONG y;
} POINT;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef unsigned int UINT;
typedef long HRESULT;

// DirectX specific types
// D3DFORMAT will be defined in d3d8.h

typedef struct {
    DWORD BackBufferWidth;
    DWORD BackBufferHeight;
    DWORD BackBufferFormat;  // Will be cast to D3DFORMAT when needed
    DWORD BackBufferCount;
    DWORD MultiSampleType;
    DWORD SwapEffect;
    HWND hDeviceWindow;
    BOOL Windowed;
    BOOL EnableAutoDepthStencil;
    DWORD AutoDepthStencilFormat;  // Will be cast to D3DFORMAT when needed
    DWORD Flags;
    DWORD FullScreen_RefreshRateInHz;
    DWORD FullScreen_PresentationInterval;
} D3DPRESENT_PARAMETERS;

// Matrix type
#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct {
    float m[4][4];
} D3DMATRIX;
#endif

// DirectX constants
#define D3D_SDK_VERSION 220
#define D3DPRASTERCAPS_FOGRANGE 0x00010000L

// Device capability constants
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT 0x00000001L
#define D3DDEVCAPS_NPATCHES 0x01000000L

// Texture operation capability constants
#define D3DTEXOPCAPS_DOTPRODUCT3 0x00800000L
#define D3DTEXOPCAPS_BUMPENVMAP 0x00200000L
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE 0x00400000L

// Caps2 constants
#define D3DCAPS2_FULLSCREENGAMMA 0x00020000L

// Windows API compatibility macros
#define WINAPI
#define __cdecl
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)

// Constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Windows API constants
#define GWL_STYLE (-16)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define HWND_TOPMOST ((void*)-1)

// Function compatibility
#define _strdup strdup
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define _isnan isnan

// String function stubs
inline char* lstrcpyn(char* dest, const char* src, int max_len) {
    return strncpy(dest, src, max_len);
}

inline char* lstrcpy(char* dest, const char* src) {
    return strcpy(dest, src);
}

inline int lstrlen(const char* str) {
    return strlen(str);
}

inline char* lstrcat(char* dest, const char* src) {
    return strcat(dest, src);
}

// DirectX basic types
typedef struct {
    DWORD LowPart;
    LONG HighPart;
} LARGE_INTEGER;

typedef struct {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;

typedef struct {
    float x, y, z, w;
} D3DVECTOR4;

typedef struct {
    float x, y, z;
} D3DVECTOR;

// Stub functions that do nothing on non-Windows
inline void* GetDC(void*) { return nullptr; }
inline void ReleaseDC(void*, void*) {}
inline void* CreateCompatibleDC(void*) { return nullptr; }
inline void DeleteDC(void*) {}
inline void* CreateCompatibleBitmap(void*, int, int) { return nullptr; }
inline void* SelectObject(void*, void*) { return nullptr; }
inline void DeleteObject(void*) {}
inline void* GetDesktopWindow() { return nullptr; }

// DirectX stub functions
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char* buffer, DWORD size) { 
    return 0; 
}

// Windows API stub functions
inline void* LoadLibrary(const char*) { return nullptr; }
inline void* GetProcAddress(void*, const char*) { return nullptr; }
inline void FreeLibrary(void*) {}
inline void ZeroMemory(void* dest, size_t size) { memset(dest, 0, size); }

// Constants
#define MAX_PATH 260

// DirectX macros
#define SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define FAILED(hr) ((HRESULT)(hr) < 0)
#define HIWORD(l) ((WORD)((DWORD_PTR)(l) >> 16))
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))

// Windows API function stubs for non-Windows platforms
inline int GetClientRect(void* hwnd, RECT* rect) {
    if (rect) {
        rect->left = 0;
        rect->top = 0; 
        rect->right = 800;
        rect->bottom = 600;
    }
    return 1;
}

inline LONG GetWindowLong(void* hwnd, int index) {
    return 0;
}

inline int AdjustWindowRect(RECT* rect, DWORD style, BOOL menu) {
    // Stub implementation - just return success
    return 1;
}

inline int SetWindowPos(void* hwnd, void* insert_after, int x, int y, int cx, int cy, DWORD flags) {
    // Stub implementation - just return success  
    return 1;
}

// Windows-style structure for monitor info
typedef struct {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO;

// Monitor constants
#define MONITOR_DEFAULTTOPRIMARY 1

// SetWindowPos constants
#define SWP_NOZORDER 0x0004

// Monitor stub functions
inline void* MonitorFromWindow(void* hwnd, DWORD flags) {
    return (void*)1; // Return dummy monitor handle
}

inline int GetMonitorInfo(void* monitor, MONITORINFO* info) {
    if (info && info->cbSize >= sizeof(MONITORINFO)) {
        info->rcMonitor.left = 0;
        info->rcMonitor.top = 0;
        info->rcMonitor.right = 1920;
        info->rcMonitor.bottom = 1080;
        info->rcWork = info->rcMonitor;
        info->dwFlags = 1;
        return 1;
    }
    return 0;
}

#endif // !_WIN32

#endif // WIN32_COMPAT_H_INCLUDED
