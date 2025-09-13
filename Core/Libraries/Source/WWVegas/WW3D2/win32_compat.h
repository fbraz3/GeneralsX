#pragma once
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstddef>
#include <unistd.h>

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
#ifndef __int64
typedef long long __int64;
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

#ifndef BITMAPFILEHEADER_DEFINED
#define BITMAPFILEHEADER_DEFINED
typedef struct {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#endif

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

// Platform version constants
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

// File attributes
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010

// Locale constants
#define LOCALE_SYSTEM_DEFAULT 0x0800

// Date/Time format constants
#define DATE_SHORTDATE 0x00000001
#define TIME_NOSECONDS 0x00000002
#define TIME_FORCE24HOURFORMAT 0x00000008
#define TIME_NOTIMEMARKER 0x00000004

// Virtual Key codes
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28

// FormatMessage constants
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000

// Utility macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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
typedef void* HICON;
typedef void* HPALETTE;
typedef void* HMETAFILE;
typedef void* HENHMETAFILE;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef uintptr_t DWORD_PTR;
typedef int BOOL;
typedef long LONG;
typedef unsigned long ULONG;  // Added ULONG type
typedef char CHAR;            // Added CHAR type
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

// IME types (stubs for non-Windows platforms)
typedef void* HIMC;
typedef struct {
    DWORD dwCount;
    DWORD dwOffset[1];
} CANDIDATELIST;

// IME function stubs
inline HIMC ImmAssociateContext(HWND hWnd, HIMC hIMC) {
    (void)hWnd; (void)hIMC;
    return NULL;
}

inline BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC) {
    (void)hWnd; (void)hIMC;
    return TRUE;
}

inline BOOL ImmDestroyContext(HIMC hIMC) {
    (void)hIMC;
    return TRUE;
}

inline HIMC ImmCreateContext(void) {
    return NULL;
}

inline HIMC ImmGetContext(HWND hWnd) {
    (void)hWnd;
    return NULL;
}

// IME message constants
#define WM_IME_CHAR 0x0286
#define WM_CHAR 0x0102

// System time structure
typedef struct {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

// File system structures
typedef struct {
    DWORD dwFileAttributes;
    DWORD ftCreationTime_dwLowDateTime;
    DWORD ftCreationTime_dwHighDateTime;
    DWORD ftLastAccessTime_dwLowDateTime;
    DWORD ftLastAccessTime_dwHighDateTime;
    DWORD ftLastWriteTime_dwLowDateTime;
    DWORD ftLastWriteTime_dwHighDateTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    char cFileName[260];
    char cAlternateFileName[14];
} WIN32_FIND_DATA;

// Memory status structure
typedef struct {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS;

// File system compatibility
#include <sys/stat.h>
#include <time.h>
#define _stat stat
#define _S_IFDIR S_IFDIR

// File system stat structure (on macOS, use the system's stat)
#include <sys/stat.h>
#define _stat stat

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
#if !defined(_WIN32) && !defined(LARGE_INTEGER_DEFINED)
#define LARGE_INTEGER_DEFINED
typedef struct {
    DWORD LowPart;
    LONG HighPart;
} LARGE_INTEGER;
#endif

#if !defined(_WIN32) && !defined(GUID_DEFINED)
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

#ifndef D3DVECTOR_DEFINED
#define D3DVECTOR_DEFINED
typedef struct {
    float x, y, z;
} D3DVECTOR;
#endif

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

#ifndef WIN32_STRING_FUNCTIONS_DEFINED
#define WIN32_STRING_FUNCTIONS_DEFINED

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
    if (::getcwd(buffer, size)) {
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

inline BOOL SetCurrentDirectory(const char* path) {
    return (chdir(path) == 0) ? TRUE : FALSE;
}

inline HANDLE FindFirstFile(const char* pattern, WIN32_FIND_DATA* findData) {
    // Simplified implementation for cross-platform compatibility
    (void)pattern;
    (void)findData;
    return (HANDLE)-1; // INVALID_HANDLE_VALUE
}

inline BOOL FindNextFile(HANDLE hFindFile, WIN32_FIND_DATA* findData) {
    (void)hFindFile;
    (void)findData;
    return FALSE;
}

inline BOOL FindClose(HANDLE hFindFile) {
    (void)hFindFile;
    return TRUE;
}

inline BOOL CopyFile(const char* existingFileName, const char* newFileName, BOOL failIfExists) {
    (void)failIfExists;
    // Use POSIX file copy functionality
    FILE* src = fopen(existingFileName, "rb");
    if (!src) return FALSE;
    
    FILE* dst = fopen(newFileName, "wb");
    if (!dst) {
        fclose(src);
        return FALSE;
    }
    
    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            fclose(src);
            fclose(dst);
            return FALSE;
        }
    }
    
    fclose(src);
    fclose(dst);
    return TRUE;
}

// Shell folder constants
#define CSIDL_DESKTOP 0x0000
#define CSIDL_DESKTOPDIRECTORY 0x0010

// Shell folder types - forward declaration if not already defined
#ifndef _SHLOBJ_H_
struct _ITEMIDLIST;
typedef struct _ITEMIDLIST* LPITEMIDLIST;
#endif

inline long SHGetSpecialFolderLocation(void* hwnd, int csidl, LPITEMIDLIST* ppidl) {
    (void)hwnd; (void)csidl;
    // Stub implementation - return success
    if (ppidl) *ppidl = nullptr;
    return 0; // S_OK
}

inline BOOL SHGetPathFromIDList(LPITEMIDLIST pidl, char* pszPath) {
    (void)pidl;
    if (pszPath) {
        // Return user's Desktop directory path for macOS
        const char* home = getenv("HOME");
        if (home) {
            sprintf(pszPath, "%s/Desktop", home);
        } else {
            strcpy(pszPath, "/tmp");
        }
        return TRUE;
    }
    return FALSE;
}

inline void GlobalMemoryStatus(MEMORYSTATUS* memoryStatus) {
    if (memoryStatus) {
        memoryStatus->dwLength = sizeof(MEMORYSTATUS);
        memoryStatus->dwMemoryLoad = 50; // 50% memory load
        memoryStatus->dwTotalPhys = 8 * 1024 * 1024 * 1024UL; // 8GB
        memoryStatus->dwAvailPhys = 4 * 1024 * 1024 * 1024UL; // 4GB available
        memoryStatus->dwTotalPageFile = 16 * 1024 * 1024 * 1024UL; // 16GB
        memoryStatus->dwAvailPageFile = 8 * 1024 * 1024 * 1024UL; // 8GB available
        memoryStatus->dwTotalVirtual = SIZE_MAX;
        memoryStatus->dwAvailVirtual = SIZE_MAX / 2;
    }
}

inline int GetDateFormat(DWORD locale, DWORD flags, const void* date, const char* format, char* buffer, int size) {
    (void)locale; (void)flags; (void)date; (void)format;
    if (buffer && size > 0) {
        strncpy(buffer, "01/01/2025", size - 1);
        buffer[size - 1] = '\0';
        return strlen(buffer);
    }
    return 0;
}

inline int GetDateFormatW(DWORD locale, DWORD flags, const void* date, const wchar_t* format, wchar_t* buffer, int size) {
    (void)locale; (void)flags; (void)date; (void)format;
    if (buffer && size > 0) {
        wcscpy(buffer, L"01/01/2025");
        return wcslen(buffer);
    }
    return 0;
}

inline int GetTimeFormat(DWORD locale, DWORD flags, const void* time, const char* format, char* buffer, int size) {
    (void)locale; (void)flags; (void)time; (void)format;
    if (buffer && size > 0) {
        strncpy(buffer, "12:00", size - 1);
        buffer[size - 1] = '\0';
        return strlen(buffer);
    }
    return 0;
}

inline int GetTimeFormatW(DWORD locale, DWORD flags, const void* time, const wchar_t* format, wchar_t* buffer, int size) {
    (void)locale; (void)flags; (void)time; (void)format;
    if (buffer && size > 0) {
        wcscpy(buffer, L"12:00");
        return wcslen(buffer);
    }
    return 0;
}

inline DWORD FormatMessage(DWORD flags, const void* source, DWORD messageId, DWORD languageId, char* buffer, DWORD size, void* arguments) {
    (void)flags; (void)source; (void)messageId; (void)languageId; (void)arguments;
    if (buffer && size > 0) {
        strncpy(buffer, "Unknown error", size - 1);
        buffer[size - 1] = '\0';
        return strlen(buffer);
    }
    return 0;
}

inline DWORD FormatMessageW(DWORD flags, const void* source, DWORD messageId, DWORD languageId, wchar_t* buffer, DWORD size, void* arguments) {
    (void)flags; (void)source; (void)messageId; (void)languageId; (void)arguments;
    if (buffer && size > 0) {
        wcscpy(buffer, L"Unknown error");
        return wcslen(buffer);
    }
    return 0;
}

// Bitmap structures
#ifndef BITMAPFILEHEADER_DEFINED
#define BITMAPFILEHEADER_DEFINED
typedef struct {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;
#endif

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
#if !defined(_WIN32) && !defined(_CRITICAL_SECTION_)
typedef struct {
    void* DebugInfo;
    long LockCount;
    long RecursionCount;
    void* OwningThread;
    void* LockSemaphore;
    unsigned long SpinCount;
} CRITICAL_SECTION;

// Critical Section functions
inline void InitializeCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
    // For cross-platform compatibility, we'll use a simple approach
    // In a real implementation, this would use pthread_mutex_init
    memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
}

inline void DeleteCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
    // For cross-platform compatibility
    // In a real implementation, this would use pthread_mutex_destroy
    memset(lpCriticalSection, 0, sizeof(CRITICAL_SECTION));
}

inline void EnterCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
    // For cross-platform compatibility, this is a simple stub
    // In a real implementation, this would use pthread_mutex_lock
    (void)lpCriticalSection; // Suppress unused parameter warning
}

inline void LeaveCriticalSection(CRITICAL_SECTION* lpCriticalSection) {
    // For cross-platform compatibility, this is a simple stub
    // In a real implementation, this would use pthread_mutex_unlock
    (void)lpCriticalSection; // Suppress unused parameter warning
}
#endif

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
#if !defined(_WIN32) && !defined(CREATETHREAD_DEFINED)
#define CREATETHREAD_DEFINED
inline HANDLE CreateThread(void* lpThreadAttributes, unsigned long dwStackSize, 
                          void* lpStartAddress, void* lpParameter, 
                          DWORD dwCreationFlags, DWORD* lpThreadId) {
    return nullptr; // Stub implementation
}
#endif

inline HANDLE CreateEvent(void* lpEventAttributes, BOOL bManualReset, BOOL bInitialState, const char* lpName) {
    (void)lpEventAttributes; (void)bManualReset; (void)bInitialState; (void)lpName;
    // Return a dummy handle for compatibility
    static int dummy_event = 1;
    return (HANDLE)&dummy_event;
}

// Memory management APIs
#ifndef MEMORY_MANAGEMENT_DEFINED
#define MEMORY_MANAGEMENT_DEFINED

// Memory allocation flags
#define HEAP_ZERO_MEMORY     0x00000008
#define GMEM_FIXED           0x0000
#define GMEM_MOVEABLE        0x0002
#define GHND                 0x0042

// Process heap handle type
typedef void* HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)-1)

// Get process heap (returns a dummy handle)
inline HANDLE GetProcessHeap() {
    static int dummy_heap = 1;
    return (HANDLE)&dummy_heap;
}

// Heap allocation functions
inline void* HeapAlloc(HANDLE hHeap, DWORD dwFlags, size_t dwBytes) {
    (void)hHeap; // Unused parameter
    void* ptr = malloc(dwBytes);
    if (ptr && (dwFlags & HEAP_ZERO_MEMORY)) {
        memset(ptr, 0, dwBytes);
    }
    return ptr;
}

inline BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, void* lpMem) {
    (void)hHeap; (void)dwFlags; // Unused parameters
    if (lpMem) {
        free(lpMem);
        return TRUE;
    }
    return FALSE;
}

// Global memory allocation functions
typedef void* HGLOBAL;

inline HGLOBAL GlobalAlloc(UINT uFlags, size_t dwBytes) {
    void* ptr = malloc(dwBytes);
    if (ptr && (uFlags & GHND)) {
        memset(ptr, 0, dwBytes);
    }
    return (HGLOBAL)ptr;
}

#ifndef GLOBALALLOCPTR_DEFINED
#define GLOBALALLOCPTR_DEFINED
inline void* GlobalAllocPtr(UINT uFlags, size_t dwBytes) {
    return GlobalAlloc(uFlags, dwBytes);
}
#endif

inline HGLOBAL GlobalFree(HGLOBAL hMem) {
    if (hMem) {
        free(hMem);
    }
    return nullptr;
}

#endif // MEMORY_MANAGEMENT_DEFINED

// Performance timing APIs
#ifndef PERFORMANCE_TIMING_DEFINED
#define PERFORMANCE_TIMING_DEFINED

#include <chrono>
#include <sys/time.h>

// High-resolution performance counter
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    if (!lpPerformanceCount) return FALSE;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto ticks = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    
    lpPerformanceCount->LowPart = (DWORD)(ticks & 0xFFFFFFFF);
    lpPerformanceCount->HighPart = (LONG)(ticks >> 32);
    return TRUE;
}

// Performance counter frequency (nanoseconds per second)
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    if (!lpFrequency) return FALSE;
    
    // High-resolution clock frequency is 1 nanosecond = 1,000,000,000 per second
    lpFrequency->LowPart = 1000000000;
    lpFrequency->HighPart = 0;
    return TRUE;
}

// System tick count in milliseconds
// Note: GetTickCount is provided by Dependencies/Utility/Utility/time_compat.h
// #ifndef GETTICKCOUNT_DEFINED
// #define GETTICKCOUNT_DEFINED
// inline DWORD GetTickCount() {
//     auto now = std::chrono::steady_clock::now();
//     auto duration = now.time_since_epoch();
//     auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
//     return (DWORD)millis;
// }
// #endif

#endif // PERFORMANCE_TIMING_DEFINED

//=============================================================================
// PHASE 5: AUDIO & MULTIMEDIA APIs
//=============================================================================

#ifndef AUDIO_MULTIMEDIA_DEFINED
#define AUDIO_MULTIMEDIA_DEFINED

// Include DirectSound compatibility layer
#include "dsound_compat.h"

// Multimedia timer functions
#ifndef MULTIMEDIA_TIMERS_DEFINED
#define MULTIMEDIA_TIMERS_DEFINED

#ifndef CALLBACK
#define CALLBACK
#endif

#include <thread>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>

// MMRESULT is already defined in Dependencies/Utility/time_compat.h as 'int'
// We should not redefine it - use the existing definition

typedef UINT MMIOID;

// Only define LPTIMECALLBACK if not already defined
#ifndef LPTIMECALLBACK
typedef void (CALLBACK *LPTIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
#endif

// Multimedia timer constants
#define TIMERR_NOERROR      0
#define TIMERR_NOCANDO      97
#define TIMERR_BADDEVICEID  98
#define TIME_ONESHOT        0x0000
#define TIME_PERIODIC       0x0001
#define TIME_CALLBACK_FUNCTION 0x0000

// Multimedia timer structures
typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *PTIMECAPS, *LPTIMECAPS;

// Timer management class
class MultimediaTimerManager {
public:
    static MultimediaTimerManager& getInstance();
    
    MMRESULT timeSetEvent(UINT delay, UINT resolution, LPTIMECALLBACK callback, 
                         DWORD_PTR user, UINT flags);
    MMRESULT timeKillEvent(UINT timerId);
    MMRESULT timeGetDevCaps(LPTIMECAPS caps, UINT cbcaps);
    MMRESULT timeBeginPeriod(UINT period);
    MMRESULT timeEndPeriod(UINT period);
    
private:
    struct TimerInfo {
        std::thread thread;
        std::atomic<bool> active;
        UINT delay;
        LPTIMECALLBACK callback;
        DWORD_PTR user;
        UINT flags;
    };
    
    std::map<UINT, std::unique_ptr<TimerInfo>> m_timers;
    std::mutex m_timersMutex;
    UINT m_nextTimerId;
    
    MultimediaTimerManager() : m_nextTimerId(1) {}
    ~MultimediaTimerManager();
    
    void timerThread(UINT timerId);
};

// Multimedia timer API functions
inline MMRESULT timeSetEvent(UINT delay, UINT resolution, LPTIMECALLBACK callback, 
                            DWORD_PTR user, UINT flags) {
    return MultimediaTimerManager::getInstance().timeSetEvent(delay, resolution, callback, user, flags);
}

inline MMRESULT timeKillEvent(UINT timerId) {
    return MultimediaTimerManager::getInstance().timeKillEvent(timerId);
}

inline MMRESULT timeGetDevCaps(LPTIMECAPS caps, UINT cbcaps) {
    return MultimediaTimerManager::getInstance().timeGetDevCaps(caps, cbcaps);
}

inline MMRESULT timeBeginPeriod(UINT period) {
    return MultimediaTimerManager::getInstance().timeBeginPeriod(period);
}

inline MMRESULT timeEndPeriod(UINT period) {
    return MultimediaTimerManager::getInstance().timeEndPeriod(period);
}

#endif // MULTIMEDIA_TIMERS_DEFINED

// WAVE format structures for audio compatibility
#ifndef WAVE_FORMAT_DEFINED
#define WAVE_FORMAT_DEFINED

#define WAVE_FORMAT_PCM         1
#define WAVE_FORMAT_ADPCM       2
#define WAVE_FORMAT_IEEE_FLOAT  3

typedef struct waveformat_tag {
    WORD    wFormatTag;
    WORD    nChannels;
    DWORD   nSamplesPerSec;
    DWORD   nAvgBytesPerSec;
    WORD    nBlockAlign;
} WAVEFORMAT;

typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;

typedef struct waveformatex_tag {
    WORD        wFormatTag;
    WORD        nChannels;
    DWORD       nSamplesPerSec;
    DWORD       nAvgBytesPerSec;
    WORD        nBlockAlign;
    WORD        wBitsPerSample;
    WORD        cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

#endif // WAVE_FORMAT_DEFINED

// Audio device management
#ifndef AUDIO_DEVICE_DEFINED
#define AUDIO_DEVICE_DEFINED

// Audio device types
#define WAVE_MAPPER             ((UINT)-1)
#define CALLBACK_NULL           0x00000000
#define CALLBACK_WINDOW         0x00010000
#define CALLBACK_TASK           0x00020000
#define CALLBACK_FUNCTION       0x00030000

// Audio device capabilities
typedef struct waveoutcaps_tag {
    WORD    wMid;
    WORD    wPid;
    DWORD   vDriverVersion;
    CHAR    szPname[32];
    DWORD   dwFormats;
    WORD    wChannels;
    WORD    wReserved1;
    DWORD   dwSupport;
} WAVEOUTCAPS, *PWAVEOUTCAPS, *LPWAVEOUTCAPS;

// Audio device functions (stubs for compatibility)
inline MMRESULT waveOutGetNumDevs() {
    return 1; // Always report one device available
}

inline MMRESULT waveOutGetDevCaps(UINT deviceId, LPWAVEOUTCAPS caps, UINT cbcaps) {
    if (caps && cbcaps >= sizeof(WAVEOUTCAPS)) {
        memset(caps, 0, sizeof(WAVEOUTCAPS));
        strcpy(caps->szPname, "OpenAL Audio Device");
        caps->dwFormats = 0xFFFFFFFF; // Support all formats
        caps->wChannels = 2; // Stereo
        return TIMERR_NOERROR;
    }
    return TIMERR_BADDEVICEID;
}

#endif // AUDIO_DEVICE_DEFINED

#endif // AUDIO_MULTIMEDIA_DEFINED

#ifndef WINDOW_FUNCTIONS_DEFINED
#define WINDOW_FUNCTIONS_DEFINED

// Window management functions stubs
inline BOOL SetWindowText(HWND hWnd, const char* lpString) {
    // Stub implementation for setting window text
    return TRUE;
}

inline BOOL SetWindowTextW(HWND hWnd, const wchar_t* lpString) {
    // Stub implementation for setting window text (wide char)
    return TRUE;
}

inline BOOL ShowWindow(HWND hWnd, int nCmdShow) {
    // Stub implementation for showing/hiding window
    return TRUE;
}

inline BOOL UpdateWindow(HWND hWnd) {
    // Stub implementation for updating window
    return TRUE;
}

inline HWND GetActiveWindow(void) {
    // Stub implementation for getting active window
    return (HWND)0;
}

inline HWND SetActiveWindow(HWND hWnd) {
    // Stub implementation for setting active window
    return hWnd;
}

inline BOOL GetWindowRect(HWND hWnd, RECT* lpRect) {
    // Stub implementation for getting window rectangle
    if (lpRect) {
        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = 800;
        lpRect->bottom = 600;
    }
    return TRUE;
}

// Additional Windows API functions
inline char* GetCommandLineA(void) {
    // Stub implementation for getting command line
    static char cmdline[] = "game.exe";
    return cmdline;
}

inline UINT GetDoubleClickTime(void) {
    // Stub implementation for double-click time (in milliseconds)
    return 500;
}

inline DWORD GetModuleFileName(HMODULE hModule, char* lpFilename, DWORD nSize) {
    // Stub implementation for getting module filename
    if (lpFilename && nSize > 0) {
        strncpy(lpFilename, "/Applications/Game.app/Contents/MacOS/game", nSize - 1);
        lpFilename[nSize - 1] = '\0';
        return strlen(lpFilename);
    }
    return 0;
}

inline void GetLocalTime(SYSTEMTIME* lpSystemTime) {
    // Stub implementation for getting local time
    if (lpSystemTime) {
        time_t now = time(0);
        struct tm* t = localtime(&now);
        lpSystemTime->wYear = t->tm_year + 1900;
        lpSystemTime->wMonth = t->tm_mon + 1;
        lpSystemTime->wDayOfWeek = t->tm_wday;
        lpSystemTime->wDay = t->tm_mday;
        lpSystemTime->wHour = t->tm_hour;
        lpSystemTime->wMinute = t->tm_min;
        lpSystemTime->wSecond = t->tm_sec;
        lpSystemTime->wMilliseconds = 0;
    }
}

#endif // WINDOW_FUNCTIONS_DEFINED

#endif // WIN32_STRING_FUNCTIONS_DEFINED

// Microsoft-specific max/min macros
#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// String functions for compatibility
// GameSpy provides _strlwr but with different linkage, so we work around this

// Helper function with a different name to avoid conflicts
static inline char *ww_strlwr_impl(char *str) {
    if (str) {
        for (char *p = str; *p; ++p) {
            *p = tolower(*p);
        }
    }
    return str;
}

// Only define the macro if we haven't seen GameSpy headers yet
#if !defined(__GSPLATFORM_H__) && !defined(_strlwr)
#define _strlwr ww_strlwr_impl
#endif

// Additional Windows APIs
// itoa function for integer to string conversion
inline char *itoa(int value, char *str, int base) {
    if (base == 10) {
        sprintf(str, "%d", value);
    } else if (base == 16) {
        sprintf(str, "%x", value);
    } else {
        // Simple base-10 fallback for other bases
        sprintf(str, "%d", value);
    }
    return str;
}

// Wide string to integer conversion
inline int _wtoi(const wchar_t* wstr) {
    if (!wstr) return 0;
    
    // Convert wchar_t to char for atoi
    char buffer[64];
    size_t len = wcslen(wstr);
    if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
    
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (char)wstr[i];  // Simple conversion for ASCII numbers
    }
    buffer[len] = '\0';
    
    return atoi(buffer);
}

// Font resource management stubs
inline int AddFontResource(const char* filename) {
    (void)filename;
    return 1; // Success stub
}

inline BOOL RemoveFontResource(const char* filename) {
    (void)filename;
    return TRUE; // Success stub
}

// OS Version information
typedef struct {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    char szCSDVersion[128];
} OSVERSIONINFO;

inline BOOL GetVersionEx(OSVERSIONINFO* lpVersionInfo) {
    if (lpVersionInfo) {
        lpVersionInfo->dwMajorVersion = 10;  // macOS major version
        lpVersionInfo->dwMinorVersion = 15;  // macOS minor version  
        lpVersionInfo->dwBuildNumber = 0;
        lpVersionInfo->dwPlatformId = 2;     // VER_PLATFORM_WIN32_NT equivalent
        strcpy(lpVersionInfo->szCSDVersion, "");
        return TRUE;
    }
    return FALSE;
}

#endif // !_WIN32

#endif // WIN32_COMPAT_H_INCLUDED
