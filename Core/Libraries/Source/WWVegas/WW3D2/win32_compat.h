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
typedef enum {
    D3DFMT_UNKNOWN = 0,
    D3DFMT_R8G8B8 = 20,
    D3DFMT_A8R8G8B8 = 21,
    D3DFMT_X8R8G8B8 = 22,
    D3DFMT_R5G6B5 = 23,
    D3DFMT_X1R5G5B5 = 24,
    D3DFMT_A1R5G5B5 = 25,
    D3DFMT_A4R4G4B4 = 26,
    D3DFMT_R3G3B2 = 27,
    D3DFMT_A8 = 28,
    D3DFMT_A8R3G3B2 = 29,
    D3DFMT_X4R4G4B4 = 30,
    D3DFMT_A2B10G10R10 = 31,
    D3DFMT_G16R16 = 34,
    D3DFMT_A8P8 = 40,
    D3DFMT_P8 = 41,
    D3DFMT_L8 = 50,
    D3DFMT_A8L8 = 51,
    D3DFMT_A4L4 = 52,
    D3DFMT_V8U8 = 60,
    D3DFMT_L6V5U5 = 61,
    D3DFMT_X8L8V8U8 = 62,
    D3DFMT_Q8W8V8U8 = 63,
    D3DFMT_V16U16 = 64,
    D3DFMT_A2W10V10U10 = 67,
    D3DFMT_UYVY = 0x59565955,
    D3DFMT_YUY2 = 0x32595559,
    D3DFMT_DXT1 = 0x31545844,
    D3DFMT_DXT2 = 0x32545844,
    D3DFMT_DXT3 = 0x33545844,
    D3DFMT_DXT4 = 0x34545844,
    D3DFMT_DXT5 = 0x35545844,
    D3DFMT_D16_LOCKABLE = 70,
    D3DFMT_D32 = 71,
    D3DFMT_D15S1 = 73,
    D3DFMT_D24S8 = 75,
    D3DFMT_D16 = 80,
    D3DFMT_D24X8 = 77,
    D3DFMT_D24X4S4 = 79
} D3DFORMAT;
// D3DFORMAT will be defined in d3d8.h

typedef struct {
    DWORD BackBufferWidth;
    DWORD BackBufferHeight;
    D3DFORMAT BackBufferFormat;
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
// DirectX texture operation capabilities
#define D3DTEXOPCAPS_DISABLE 0x00000001L
#define D3DTEXOPCAPS_SELECTARG1 0x00000002L
#define D3DTEXOPCAPS_SELECTARG2 0x00000004L
#define D3DTEXOPCAPS_MODULATE 0x00000008L
#define D3DTEXOPCAPS_MODULATE2X 0x00000010L
#define D3DTEXOPCAPS_MODULATE4X 0x00000020L
#define D3DTEXOPCAPS_ADD 0x00000040L
#define D3DTEXOPCAPS_ADDSIGNED 0x00000080L
#define D3DTEXOPCAPS_ADDSIGNED2X 0x00000100L
#define D3DTEXOPCAPS_SUBTRACT 0x00000200L
#define D3DTEXOPCAPS_ADDSMOOTH 0x00000400L
#define D3DTEXOPCAPS_BLENDDIFFUSEALPHA 0x00000800L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHA 0x00001000L
#define D3DTEXOPCAPS_BLENDFACTORALPHA 0x00002000L
#define D3DTEXOPCAPS_BLENDTEXTUREALPHAPM 0x00004000L
#define D3DTEXOPCAPS_BLENDCURRENTALPHA 0x00008000L
#define D3DTEXOPCAPS_PREMODULATE 0x00010000L
#define D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR 0x00020000L
#define D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA 0x00040000L
#define D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR 0x00080000L
#define D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA 0x00100000L
#define D3DTEXOPCAPS_BUMPENVMAP 0x00200000L
#define D3DTEXOPCAPS_BUMPENVMAPLUMINANCE 0x00400000L
#define D3DTEXOPCAPS_DOTPRODUCT3 0x00800000L

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

class D3DVECTOR4 {
public:
    float x, y, z, w;
    
    // Array access operator
    float& operator[](int index) {
        return (&x)[index];
    }
    
    const float& operator[](int index) const {
        return (&x)[index];
    }
};

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
