#pragma once
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cmath>

#ifndef WIN32_COMPAT_H_INCLUDED
#define WIN32_COMPAT_H_INCLUDED

#ifndef _WIN32
// Include DirectX math compatibility
#include "../WWMath/d3dx8math.h"
// Compatibility types for non-Windows systems
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

// Basic Windows types that might not be defined
#ifndef LONG
typedef long LONG;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

// Additional types we need
#ifndef FARPROC
typedef void* FARPROC;
#endif

#ifndef HRESULT
typedef long HRESULT;
#endif

// COM-related stubs
#define CoUninitialize() do {} while(0)

#ifndef RGBQUAD
// Windows bitmap and image types
typedef struct {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO;

// GDI constants
#define DIB_RGB_COLORS 0

#endif // RGBQUAD
#include <cctype>  // for toupper
#include <cwchar>  // for wchar_t

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
#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef struct {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;
#endif

#ifndef POINT_DEFINED
#define POINT_DEFINED
typedef struct {
    LONG x;
    LONG y;
} POINT;
#endif
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef unsigned int UINT;
typedef long HRESULT;
typedef wchar_t WCHAR;
typedef const WCHAR* LPCWSTR;
typedef WCHAR* LPWSTR;
typedef void* HANDLE;

// HRESULT success/error checking
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

// Common HRESULT values
#define S_OK                          ((HRESULT)0L)
#define S_FALSE                       ((HRESULT)1L)
#define E_FAIL                        ((HRESULT)0x80004005L)
#define E_INVALIDARG                  ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY                 ((HRESULT)0x8007000EL)

// Audio types
typedef DWORD U32;
typedef long S32;
typedef void* LPWAVEFORMAT;
typedef void* HTIMER;

// Calling conventions
#define _stdcall
#define __cdecl
#define AILCALLBACK

// DirectX specific types - only define if not already defined
#ifndef D3DFORMAT_DEFINED
#define D3DFORMAT_DEFINED
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
    D3DFMT_W11V11U10 = 65,
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
    D3DFMT_D24X4S4 = 79,
    D3DFMT_INDEX16 = 101,
    D3DFMT_INDEX32 = 102
} D3DFORMAT;
#endif // D3DFORMAT_DEFINED

// D3D Cube Map Face enum
typedef enum {
    D3DCUBEMAP_FACE_POSITIVE_X = 0,
    D3DCUBEMAP_FACE_NEGATIVE_X = 1,
    D3DCUBEMAP_FACE_POSITIVE_Y = 2,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
    D3DCUBEMAP_FACE_POSITIVE_Z = 4,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 5
} D3DCUBEMAP_FACES;

// D3D Locked Box structure
typedef struct {
    int RowPitch;
    int SlicePitch;
    void* pBits;
} D3DLOCKED_BOX;
// D3DFORMAT will be defined in d3d8.h

#ifndef D3DPRESENT_PARAMETERS_DEFINED
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
#define D3DPRESENT_PARAMETERS_DEFINED
#endif // D3DPRESENT_PARAMETERS_DEFINED

// Matrix type
#ifndef D3DMATRIX_DEFINED
#define D3DMATRIX_DEFINED
typedef struct {
    float _11, _12, _13, _14;
    float _21, _22, _23, _24;
    float _31, _32, _33, _34;
    float _41, _42, _43, _44;
} D3DMATRIX;
#endif

// DirectX constants
#define D3D_SDK_VERSION 220
#define D3DPRASTERCAPS_FOGRANGE 0x00010000L

// Device capability constants
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT 0x00000001L
#ifndef D3DDEVCAPS_NPATCHES_DEFINED
#define D3DDEVCAPS_NPATCHES_DEFINED
#define D3DDEVCAPS_NPATCHES 0x02000000L
#endif

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
#define HWND_TOPMOST ((HWND)-1)

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
#ifndef LARGE_INTEGER_DEFINED
#define LARGE_INTEGER_DEFINED
typedef struct {
    DWORD LowPart;
    LONG HighPart;
} LARGE_INTEGER;
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct {
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[8];
} GUID;
#endif

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

// GDI functions
#define RGB(r,g,b) ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define DIB_RGB_COLORS 0

// BITMAP function stubs
inline void* CreateDIBSection(void* hdc, const BITMAPINFO* pbmi, UINT usage, void** ppvBits, void* hSection, DWORD offset) {
    if (ppvBits) *ppvBits = nullptr;
    return nullptr;
}

inline DWORD SetBkColor(void* hdc, DWORD color) {
    return 0;
}

inline DWORD SetTextColor(void* hdc, DWORD color) {
    return 0;
}

// DirectX stub functions
#ifndef D3DXGETERRORSTRINGA_DEFINED
inline HRESULT D3DXGetErrorStringA(HRESULT hr, char* buffer, DWORD size) { 
    return 0; 
}
#define D3DXGETERRORSTRINGA_DEFINED
#endif // D3DXGETERRORSTRINGA_DEFINED

#ifndef WIN32_COMPAT_FUNCTIONS_DEFINED
#define WIN32_COMPAT_FUNCTIONS_DEFINED

// Windows API stub functions
inline void* LoadLibrary(const char*) { return nullptr; }
inline void* GetProcAddress(void*, const char*) { return nullptr; }
inline void FreeLibrary(void*) {}
inline void ZeroMemory(void* dest, size_t size) { memset(dest, 0, size); }

#endif // WIN32_COMPAT_FUNCTIONS_DEFINED

// Constants
#define MAX_PATH 260

// DirectX macros
#define HIWORD(l) ((WORD)((DWORD_PTR)(l) >> 16))
#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))

// Additional window flags (not previously defined)
#define SWP_NOZORDER 0x0004
#define MONITOR_DEFAULTTOPRIMARY 0x00000001

// Monitor info structure
typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT rcMonitor;
    RECT rcWork;
    DWORD dwFlags;
} MONITORINFO;

// Windows API function stubs for non-Windows platforms
#ifndef WIN32_API_STUBS_DEFINED
#define WIN32_API_STUBS_DEFINED

inline int GetClientRect(void* hwnd, RECT* rect) {
    if (rect) {
        rect->left = 0;
        rect->top = 0;
        rect->right = 800;
        rect->bottom = 600;
    }
    return 1;
}

inline DWORD GetWindowLong(HWND hwnd, int index) {
    return 0;
}

inline void AdjustWindowRect(RECT* rect, DWORD style, BOOL menu) {
    // Stub implementation - no adjustment
}

inline BOOL SetWindowPos(HWND hwnd, HWND insertAfter, int x, int y, int cx, int cy, UINT flags) {
    return TRUE;
}

inline void* MonitorFromWindow(HWND hwnd, DWORD flags) {
    return (void*)1; // Fake monitor handle
}

inline BOOL GetMonitorInfo(void* monitor, MONITORINFO* info) {
    if (info && info->cbSize >= sizeof(MONITORINFO)) {
        info->rcMonitor = {0, 0, 1920, 1080};
        info->rcWork = {0, 0, 1920, 1040};
        info->dwFlags = 0;
        return TRUE;
    }
    return FALSE;
}

#endif // WIN32_API_STUBS_DEFINED

#ifndef WIN32_COMPAT_FUNCTIONS_DEFINED

// String manipulation functions
inline char* strupr(char* str) {
    if (str) {
        for (char* p = str; *p; ++p) {
            *p = toupper(*p);
        }
    }
    return str;
}

// Case-insensitive string comparison
inline int lstrcmpi(const char* str1, const char* str2) {
    return strcasecmp(str1, str2);
}

// File system functions
inline DWORD GetCurrentDirectory(DWORD size, char* buffer) {
    if (getcwd(buffer, size)) {
        return strlen(buffer);
    }
    return 0;
}

inline DWORD GetFileAttributes(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return 0; // FILE_ATTRIBUTE_NORMAL
    }
    return 0xFFFFFFFF; // INVALID_FILE_ATTRIBUTES
}

// Bitmap structures
typedef struct {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

#define BI_RGB 0

//Text and Font types and functions
typedef struct tagSIZE {
    long cx;
    long cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagTEXTMETRIC {
    long tmHeight;
    long tmAscent;
    long tmDescent;
    long tmInternalLeading;
    long tmExternalLeading;
    long tmAveCharWidth;
    long tmMaxCharWidth;
    long tmWeight;
    long tmOverhang;
    long tmDigitizedAspectX;
    long tmDigitizedAspectY;
    char tmFirstChar;
    char tmLastChar;
    char tmDefaultChar;
    char tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRIC, *PTEXTMETRIC, *LPTEXTMETRIC;

// Font weight constants
#define FW_NORMAL           400
#define FW_BOLD             700

// Charset constants
#define DEFAULT_CHARSET     1

// Font precision constants
#define OUT_DEFAULT_PRECIS  0
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define VARIABLE_PITCH      2

// Text output constants
#define ETO_OPAQUE          2

// Text/Font functions (stubs for non-Windows)
inline HFONT CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation,
                       int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut,
                       DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision,
                       DWORD fdwQuality, DWORD fdwPitchAndFamily, const char* lpszFace) {
    return nullptr; // Stub implementation
}

inline BOOL ExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* rect,
                       const wchar_t* string, UINT count, const int* dx) {
    return FALSE; // Stub implementation
}

inline BOOL GetTextExtentPoint32W(HDC hdc, const wchar_t* string, int count, SIZE* size) {
    if (size) {
        size->cx = count * 8; // Rough estimate
        size->cy = 16;
    }
    return TRUE;
}

inline BOOL GetTextMetrics(HDC hdc, TEXTMETRIC* tm) {
    if (tm) {
        memset(tm, 0, sizeof(TEXTMETRIC));
        tm->tmHeight = 16;
        tm->tmAveCharWidth = 8;
    }
    return TRUE;
}

inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
    if (nDenominator == 0) return 0;
    return (int)(((long long)nNumber * nNumerator) / nDenominator);
}

// DirectX 8 compatibility stubs
// Note: D3DXMATRIX is defined in d3dx8math.h
#ifndef D3DX_PI
#define D3DX_PI 3.14159265f
#endif

// DirectX Texture Stage States
#define D3DTSS_TCI_CAMERASPACEPOSITION     1
#define D3DTSS_TCI_CAMERASPACENORMAL       2

// DirectX Texture Transform Flags
#define D3DTTFF_COUNT1                     1
#define D3DTTFF_COUNT2                     2
#define D3DTTFF_COUNT3                     3
#define D3DTTFF_COUNT4                     4
#define D3DTTFF_PROJECTED                  256

inline void D3DXMatrixRotationZ(D3DXMATRIX* out, float angle) {
    // Simple 2D rotation matrix around Z axis
    if (out) {
        float cosAngle = cosf(angle);
        float sinAngle = sinf(angle);
        
        // Initialize to identity matrix
        memset(out, 0, sizeof(D3DXMATRIX));
        out->m[0][0] = cosAngle;  out->m[0][1] = sinAngle;   out->m[0][2] = 0; out->m[0][3] = 0;
        out->m[1][0] = -sinAngle; out->m[1][1] = cosAngle;   out->m[1][2] = 0; out->m[1][3] = 0;
        out->m[2][0] = 0;         out->m[2][1] = 0;          out->m[2][2] = 1; out->m[2][3] = 0;
        out->m[3][0] = 0;         out->m[3][1] = 0;          out->m[3][2] = 0; out->m[3][3] = 1;
    }
}

// Additional Windows types needed for compilation
#ifndef F32
typedef float F32;
#endif

// Threading types
typedef struct {
    void* DebugInfo;
    long LockCount;
    long RecursionCount;
    void* OwningThread;
    void* LockSemaphore;
    unsigned long SpinCount;
} CRITICAL_SECTION;

// Registry constants
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002)
#define KEY_READ (0x20019)
#define ERROR_SUCCESS 0L
#define REG_DWORD 4

// File system compatibility
// TODO: Implement _stat compatibility properly
// #include <sys/stat.h>

// String functions for compatibility
#define _strnicmp strncasecmp

// Function stubs for file system compatibility
// inline int _stat(const char* path, struct stat* buffer) {
//     return stat(path, buffer);
// }

// Registry function stubs - commented out to avoid redefinition conflicts
// Using definitions from Core/Libraries/Include/windows.h instead
/*
inline long RegOpenKeyEx(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult) {
    return ERROR_SUCCESS; // Stub implementation
}

inline long RegQueryValueEx(HKEY hKey, const char* lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData) {
    return ERROR_SUCCESS; // Stub implementation
}
*/

// Threading function stubs
inline HANDLE CreateThread(void* lpThreadAttributes, unsigned long dwStackSize, 
                          void* lpStartAddress, void* lpParameter, 
                          DWORD dwCreationFlags, DWORD* lpThreadId) {
    return nullptr; // Stub implementation
}

#endif // WIN32_COMPAT_FUNCTIONS_DEFINED

#endif // !_WIN32

#endif // WIN32_COMPAT_H_INCLUDED
