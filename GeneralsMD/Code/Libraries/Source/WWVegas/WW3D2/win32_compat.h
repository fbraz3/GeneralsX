#pragma once

#ifndef WIN32_COMPAT_H_INCLUDED
#define WIN32_COMPAT_H_INCLUDED

#ifndef _WIN32

// ============================================================================
// Windows API Compatibility Layer - Master Header
// 
// This is the umbrella header that includes all modular Windows compatibility
// layers for cross-platform (macOS, Linux) compilation.
// 
// Architecture (Modular Components):
// 1. win32_compat_core.h - Core Windows types (DWORD, HRESULT, HWND, etc.)
// 2. win32_compat_file.h - File I/O stubs (CreateFile, ReadFile, etc.) [TODO]
// 3. win32_compat_memory.h - Memory management stubs [TODO]
// 4. win32_compat_registry.h - Registry stubs (HKEY, RegOpen, etc.) [TODO]
// 5. win32_compat_ui.h - UI/Window stubs (MessageBox, CreateWindow, etc.) [TODO]
// 6. d3d8.h - DirectX 8 graphics types (via d3d8_graphics_compat.h)
// 7. d3dx8math.h - DirectX math library
// 
// Phase 50: Graphics-only, includes core and d3d8 types
// Phase 51: Audio will add d3d8_audio_compat.h
// ============================================================================

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstddef>
#include <string>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <time.h>
#include <cstdlib>

// Include time compatibility early for MMRESULT and timing functions
#include <Utility/time_compat.h>

// Step 1: Core Windows types
#include "win32_compat_core.h"

// Step 1c: DirectX 8 math library (required by BezierSegment and other game code)
#include "../WWMath/d3dx8math.h"

// Step 2: Compatibility types for non-Windows systems
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mach-o/dyld.h>  // For _NSGetExecutablePath on macOS

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

// Boolean constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Microsoft compiler attributes compatibility
#ifndef __declspec
#define __declspec(x)
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

#ifndef WPARAM
typedef uintptr_t WPARAM;
#endif

#ifndef LPARAM
typedef uintptr_t LPARAM;
#endif

// Note: FARPROC and HRESULT are defined in win32_compat_core.h

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

// Pointer typedefs for BITMAP structures
typedef BITMAPINFOHEADER* PBITMAPINFOHEADER;
typedef BITMAPINFO* PBITMAPINFO;
typedef BYTE* LPBYTE;

// Memory allocation flags and types
#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#define LMEM_FIXED 0x0000
#define LMEM_ZEROINIT 0x0040

// GDI constants
#define DIB_RGB_COLORS 0

// Platform version constants
#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

// File attributes
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010

// Locale constants
#define LOCALE_SYSTEM_DEFAULT 0x0800

// Registry API compatibility - see windows.h for implementations

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

// Phase 36.7: Window constants (moved here for SDL2 window creation functions)
// Window style constants
#define WS_POPUP        0x80000000L
#define WS_VISIBLE      0x10000000L
#define WS_DLGFRAME     0x00400000L
#define WS_CAPTION      0x00C00000L
#define WS_SYSMENU      0x00080000L

// Window positioning constants
#define CW_USEDEFAULT   ((int)0x80000000)

// ShowWindow constants
#define SW_HIDE         0
#define SW_SHOWNORMAL   1
#define SW_SHOW         5
#define SW_RESTORE      9

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

// SAL annotations (placeholders for macOS)
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
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
typedef long LRESULT;
typedef wchar_t WCHAR;
typedef void* HBRUSH;
typedef void* HDC;
typedef WORD ATOM;

// GDI structures
typedef struct {
    HDC hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
} PAINTSTRUCT;

// Window class structure
typedef struct {
    UINT style;
    LRESULT (*lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    void* hIcon;
    void* hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASS;

// Exception handling structures
typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    DWORD NumberParameters;
    void* ExceptionInformation[15];
} EXCEPTION_RECORD;

typedef struct _CONTEXT {
    // Simplified context structure
    DWORD ContextFlags;
    DWORD Eax, Ebx, Ecx, Edx, Esi, Edi;
    DWORD Esp, Ebp, Eip;
} CONTEXT;

typedef struct _EXCEPTION_POINTERS {
    EXCEPTION_RECORD* ExceptionRecord;
    CONTEXT* ContextRecord;
} EXCEPTION_POINTERS;
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
#define E_NOTIMPL                     ((HRESULT)0x80004001L)

// Audio types (avoid redefinition if Miles Sound System already defined them)
// Skip these definitions if Miles Sound System header was already included
#ifndef MILES_SOUND_SYSTEM_TYPES_DEFINED
typedef DWORD U32;
typedef long S32;
typedef void* LPWAVEFORMAT;
typedef void* HTIMER;
#endif

// Miles Sound System compatibility stubs
inline void AIL_lock() {
    // Miles Sound System lock - stub implementation for cross-platform
}

inline void AIL_unlock() {
    // Miles Sound System unlock - stub implementation for cross-platform
}

inline void AIL_set_3D_position(void* sample, float x, float y, float z) {
    // Miles Sound System 3D position - stub implementation for cross-platform
}

inline void AIL_set_3D_orientation(void* sample, float x_face, float y_face, float z_face, 
                                  float x_up, float y_up, float z_up) {
    // Miles Sound System 3D orientation - stub implementation for cross-platform
}

inline void AIL_set_sample_processor(void* sample, int pipeline_stage, void* provider) {
    // Miles Sound System sample processor - stub implementation for cross-platform
}

inline void AIL_set_filter_sample_preference(void* filter, const char* preference, void* value) {
    // Miles Sound System filter preference - stub implementation for cross-platform
}

// Miles Sound System constants
#define DP_FILTER 0x1000

// Miles Sound System types
typedef unsigned long MILES_HANDLE;

// Miles Sound System invalid handle for HPROVIDER
#define INVALID_HPROVIDER ((HPROVIDER)(MILES_HANDLE)-1)

// IME types (stubs for non-Windows platforms)
typedef void* HIMC;
typedef struct {
    DWORD dwStyle;
    RECT  rcArea;
    // Simplified structure for cross-platform compatibility
} COMPOSITIONFORM, *LPCOMPOSITIONFORM;

typedef struct {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
    // Simplified structure for cross-platform compatibility
} CANDIDATEFORM, *LPCANDIDATEFORM;

typedef DWORD* LPDWORD;
typedef void* LPSECURITY_ATTRIBUTES;  // Security attributes placeholder

typedef struct {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *LPCANDIDATELIST;

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

inline BOOL ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm) {
    (void)hIMC; (void)lpCompForm;
    return TRUE;
}

inline BOOL ImmSetCandidateWindow(HIMC hIMC, LPCANDIDATEFORM lpCandidate) {
    (void)hIMC; (void)lpCandidate;
    return TRUE;
}

inline LONG ImmGetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen;
    return 0;
}

inline LONG ImmGetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen;
    return 0;
}

inline LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen;
    return 0;
}

inline DWORD ImmGetCandidateListCountW(HIMC hIMC, LPDWORD lpdwListCount) {
    (void)hIMC; (void)lpdwListCount;
    return 0;
}

inline DWORD ImmGetCandidateListCountA(HIMC hIMC, LPDWORD lpdwListCount) {
    (void)hIMC; (void)lpdwListCount;
    return 0;
}

inline DWORD ImmGetCandidateListW(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen) {
    (void)hIMC; (void)deIndex; (void)lpCandList; (void)dwBufLen;
    return 0;
}

inline DWORD ImmGetCandidateListA(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen) {
    (void)hIMC; (void)deIndex; (void)lpCandList; (void)dwBufLen;
    return 0;
}

// IME message constants
#define WM_IME_CHAR 0x0286
#define WM_CHAR 0x0102
#define WM_IME_SELECT 0x0285
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define WM_IME_SETCONTEXT       0x0281
#define WM_IME_NOTIFY           0x0282
#define WM_IME_COMPOSITIONFULL  0x0284

// IME notification constants
#define GCS_RESULTSTR 0x0800
#define GCS_COMPSTR            0x0008
#define GCS_CURSORPOS          0x0080
#define CS_INSERTCHAR 0x2000
#define CS_NOMOVECARET         0x4000
#define IMN_OPENCANDIDATE      0x0005
#define IMN_CLOSECANDIDATE     0x0004
#define IMN_CHANGECANDIDATE    0x0003
#define IMN_GUIDELINE          0x000D
#define IMN_SETCONVERSIONMODE  0x0006
#define IMN_SETSENTENCEMODE    0x0008

// IME candidate list style constants
#define IME_CAND_UNKNOWN       0x0000
#define IME_CAND_READ          0x0001
#define IME_CAND_CODE          0x0002
#define IME_CAND_MEANING       0x0003

// IME property constants
#define IGP_PROPERTY           0x0004
#define IME_PROP_CANDLIST_START_FROM_1  0x00080000
#define IME_PROP_UNICODE       0x20000000

// Keyboard layout type
typedef DWORD HKL;

// Additional IME functions
inline HKL GetKeyboardLayout(DWORD idThread) {
    (void)idThread;
    return 0;
}

inline DWORD ImmGetProperty(HKL hKL, DWORD fdwIndex) {
    (void)hKL; (void)fdwIndex;
    return 0;
}

// Additional string functions for Windows compatibility
inline size_t _mbsnccnt(const unsigned char* str, size_t count) {
    (void)str; (void)count;
    return count; // Simple fallback for cross-platform
}
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

// User and computer name constants
#define UNLEN 256
#define MAX_COMPUTERNAME_LENGTH 15

// User and computer name functions
inline BOOL GetUserName(char* lpBuffer, DWORD* pcbBuffer) {
    const char* username = getenv("USER");
    if (!username) username = getenv("USERNAME");
    if (!username) username = "unknown";
    
    size_t len = strlen(username);
    if (len >= *pcbBuffer) {
        *pcbBuffer = len + 1;
        return FALSE;
    }
    
    strcpy(lpBuffer, username);
    *pcbBuffer = len;
    return TRUE;
}

inline BOOL GetComputerName(char* lpBuffer, DWORD* nSize) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }
    
    size_t len = strlen(hostname);
    if (len >= *nSize) {
        *nSize = len + 1;
        return FALSE;
    }
    
    strcpy(lpBuffer, hostname);
    *nSize = len;
    return TRUE;
}

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

// D3DLOCKED_BOX is defined in d3d8_graphics_compat.h
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
typedef struct D3DMATRIX {
    union {
        struct {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };
} D3DMATRIX, _D3DMATRIX;
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
#define _wcsicmp wcscasecmp

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

// Define LONGLONG type
typedef long long LONGLONG;

// LARGE_INTEGER is defined in win32_compat_core.h

// GUID is defined in win32_compat_core.h

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
inline void* CreateCompatibleDC(void*) { 
    static char dummy_dc = 1; // Dummy non-NULL pointer for macOS compatibility
    return &dummy_dc; 
}
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
    // Return a non-null opaque handle to indicate success on non-Windows
    static char dummy_font;
    (void)nHeight; (void)nWidth; (void)nEscapement; (void)nOrientation;
    (void)fnWeight; (void)fdwItalic; (void)fdwUnderline; (void)fdwStrikeOut;
    (void)fdwCharSet; (void)fdwOutputPrecision; (void)fdwClipPrecision;
    (void)fdwQuality; (void)fdwPitchAndFamily; (void)lpszFace;
    return (HFONT)&dummy_font;
}

inline BOOL ExtTextOutW(HDC hdc, int x, int y, UINT options, const RECT* rect,
                       const wchar_t* string, UINT count, const int* dx) {
    // macOS stub - simulate successful text rendering
    return TRUE; // Return success instead of failure
}

inline BOOL GetTextExtentPoint32W(HDC hdc, const wchar_t* string, int count, SIZE* size) {
    (void)hdc;
    if (size && count > 0) {
        // Use a conservative average width, tuned later if needed
        size->cx = count * 10; // average width
        size->cy = 16;         // line height
        return TRUE;
    }
    if (size) { size->cx = 0; size->cy = 0; }
    return FALSE;
}

inline BOOL GetTextMetrics(HDC hdc, TEXTMETRIC* tm) {
    (void)hdc;
    if (tm) {
        memset(tm, 0, sizeof(TEXTMETRIC));
        tm->tmHeight = 16;
        tm->tmAscent = 13;
        tm->tmOverhang = 0;
        tm->tmAveCharWidth = 8;
        return TRUE;
    }
    return FALSE;
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

inline void D3DXMatrixRotationZ(D3DMATRIX* out, float angle) {
    // Simple 2D rotation matrix around Z axis
    if (out) {
        float cosAngle = cosf(angle);
        float sinAngle = sinf(angle);
        
        // Initialize to identity matrix
        memset(out, 0, sizeof(D3DMATRIX));
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
#define HKEY_CURRENT_USER ((HKEY)0x80000001)
#define KEY_READ (0x20019)
#define KEY_WRITE (0x20006)
#define ERROR_SUCCESS 0L
#define REG_DWORD 4
#define REG_SZ 1
#define REG_OPTION_NON_VOLATILE 0

// File system compatibility
// TODO: Implement _stat compatibility properly
// #include <sys/stat.h>

// String functions for compatibility
#define _strnicmp strncasecmp

// Function stubs for file system compatibility
// inline int _stat(const char* path, struct stat* buffer) {
//     return stat(path, buffer);
// }

// Registry function stubs - using definitions from windows.h to avoid conflicts

// Threading function stubs
#if !defined(_WIN32) && !defined(CREATETHREAD_DEFINED)
#define CREATETHREAD_DEFINED
inline HANDLE CreateThread(void* lpThreadAttributes, unsigned long dwStackSize, 
                          void* lpStartAddress, void* lpParameter, 
                          DWORD dwCreationFlags, DWORD* lpThreadId) {
    return nullptr; // Stub implementation
}

inline BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) {
    (void)hThread; (void)dwExitCode;
    // Stub implementation - in real cross-platform code, would use pthread_cancel
    // For now, just return success to maintain compatibility
    return TRUE;
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
typedef HGLOBAL HLOCAL;  // Local memory handle (redirected to global)

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

// Local memory API (redirected to global memory)
inline HLOCAL LocalAlloc(UINT uFlags, size_t dwBytes) {
    return (HLOCAL)GlobalAlloc(uFlags, dwBytes);
}

inline HLOCAL LocalFree(HLOCAL hMem) {
    return (HLOCAL)GlobalFree((HGLOBAL)hMem);
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

// Mutex and synchronization APIs - using definitions from threading.h and windows.h
#ifndef MUTEX_SYNCHRONIZATION_DEFINED
#define MUTEX_SYNCHRONIZATION_DEFINED

// Error codes
#define ERROR_ALREADY_EXISTS 183L

#endif // MUTEX_SYNCHRONIZATION_DEFINED

//=============================================================================
// PHASE 5: AUDIO & MULTIMEDIA APIs
//=============================================================================

#ifndef AUDIO_MULTIMEDIA_DEFINED
#define AUDIO_MULTIMEDIA_DEFINED

// Include DirectSound compatibility layer (audio - Phase 51+)
// Phase 50: Graphics only (Vulkan), audio deferred to Phase 51
#if 0 // Disabled for Phase 50 - audio implementation deferred
#include "dsound_compat.h"
#endif

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
// Skip if Miles Sound System header was already included
#ifndef MILES_SOUND_SYSTEM_TYPES_DEFINED
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

#endif // MILES_SOUND_SYSTEM_TYPES_DEFINED

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
    // Phase 36.7: SDL2 window visibility control
    if (!hWnd) return FALSE;
    SDL_Window* window = (SDL_Window*)hWnd;
    
    if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_RESTORE) {
        SDL_ShowWindow(window);
        SDL_RaiseWindow(window);
    } else if (nCmdShow == SW_HIDE) {
        SDL_HideWindow(window);
    }
    return TRUE;
}

inline BOOL UpdateWindow(HWND hWnd) {
    // Phase 36.7: SDL2 window update (pump events)
    if (!hWnd) return FALSE;
    SDL_PumpEvents(); // Process pending events
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
#ifndef _WIN32
extern char* GetCommandLineA_Global();
#endif

inline char* GetCommandLineA(void) {
#ifdef _WIN32
    // Windows implementation
    return ::GetCommandLineA();
#else
    // macOS/Unix: Use global variable set by main()
    return GetCommandLineA_Global();
#endif
}

inline UINT GetDoubleClickTime(void) {
    // Stub implementation for double-click time (in milliseconds)
    return 500;
}

inline DWORD GetModuleFileName(HMODULE hModule, char* lpFilename, DWORD nSize) {
    // Cross-platform implementation for getting module filename
    if (lpFilename && nSize > 0) {
#ifdef _WIN32
        // Windows implementation would go here
        return ::GetModuleFileName(hModule, lpFilename, nSize);
#else
        // macOS/Unix implementation using _NSGetExecutablePath
        uint32_t size = nSize;
        if (_NSGetExecutablePath(lpFilename, &size) == 0) {
            return strlen(lpFilename);
        }
#endif
    }
    return 0;
}

inline DWORD GetModuleFileNameW(HMODULE hModule, wchar_t* lpFilename, DWORD nSize) {
    // Stub implementation for getting module filename (wide char version)
    if (lpFilename && nSize > 0) {
        const wchar_t* path = L"/Applications/Game.app/Contents/MacOS/game";
        wcsncpy(lpFilename, path, nSize - 1);
        lpFilename[nSize - 1] = L'\0';
        return wcslen(lpFilename);
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

// Windows Sockets API compatibility
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define LOBYTE(w)           ((BYTE)(((DWORD)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD)(w)) >> 8) & 0xff))

// Windows Socket Error Constants
#define WSAEWOULDBLOCK     EAGAIN
#define WSAEINVAL          EINVAL  
#define WSAEALREADY        EALREADY
#define WSAEISCONN         EISCONN
#define WSAENOTCONN        ENOTCONN
#define WSAECONNRESET      ECONNRESET
#define WSAECONNABORTED    ECONNABORTED
#define WSAEINPROGRESS     EINPROGRESS
#define WSAETIMEDOUT       ETIMEDOUT
#define WSAENETDOWN        ENETDOWN
#define WSAENETUNREACH     ENETUNREACH
#define WSAEHOSTDOWN       EHOSTDOWN
#define WSAEHOSTUNREACH    EHOSTUNREACH

typedef struct {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA;

// Socket types and functions
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent HOSTENT;

// Note: getsockname and listen are defined in network.h with proper compat layer
// Note: closesocket is defined in WWDownload/winsock.h
// Define closesocket for GameSpy compatibility
#ifndef closesocket
#define closesocket(s) close(s)
#endif

// Note: WSAStartup, WSACleanup, WSAGetLastError and other network functions are defined in network.h
// Note: gethostbyname and gethostname are provided by system headers

// Windows compatibility for cross-platform build

inline void PostQuitMessage(int nExitCode) {
    // On non-Windows platforms, this would typically signal the main loop to exit
    (void)nExitCode; // Stub implementation
}

// GDI functions
inline HDC BeginPaint(HWND hWnd, PAINTSTRUCT* lpPaint) {
    (void)hWnd;
    if (lpPaint) {
        memset(lpPaint, 0, sizeof(PAINTSTRUCT));
        lpPaint->hdc = (HDC)1; // Dummy HDC
    }
    return (HDC)1;
}

inline BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint) {
    (void)hWnd; (void)lpPaint;
    return TRUE;
}

inline int SaveDC(HDC hdc) {
    (void)hdc;
    return 1;
}

inline BOOL RestoreDC(HDC hdc, int nSavedDC) {
    (void)hdc; (void)nSavedDC;
    return TRUE;
}

inline BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) {
    (void)hdc; (void)x; (void)y; (void)cx; (void)cy; (void)hdcSrc; (void)x1; (void)y1; (void)rop;
    return TRUE;
}

inline LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    (void)hWnd; (void)Msg; (void)wParam; (void)lParam;
    return 0;
}

inline void* LoadIcon(HINSTANCE hInstance, LPCSTR lpIconName) {
    (void)hInstance; (void)lpIconName;
    return (void*)1; // Dummy icon handle
}

inline void* GetStockObject(int i) {
    (void)i;
    return (void*)1; // Dummy object handle
}

inline ATOM RegisterClass(const WNDCLASS* lpWndClass) {
    (void)lpWndClass;
    return 1; // Dummy class atom
}

inline int GetSystemMetrics(int nIndex) {
    switch (nIndex) {
        case 0: return 1920; // SM_CXSCREEN - Default screen width
        case 1: return 1080; // SM_CYSCREEN - Default screen height
        default: return 0;
    }
}

inline HWND SetFocus(HWND hWnd) {
    // Phase 36.7: SDL2 focus control
    if (!hWnd) return NULL;
    SDL_Window* window = (SDL_Window*)hWnd;
    SDL_RaiseWindow(window);
    return hWnd; // Return same window as previous focus
}

inline BOOL SetForegroundWindow(HWND hWnd) {
    // Phase 36.7: SDL2 foreground window control
    if (!hWnd) return FALSE;
    SDL_Window* window = (SDL_Window*)hWnd;
    SDL_RaiseWindow(window);
    return TRUE;
}

inline HWND CreateWindow(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle,
                        int x, int y, int nWidth, int nHeight,
                        HWND hWndParent, void* hMenu, HINSTANCE hInstance, void* lpParam) {
    (void)lpClassName; (void)hWndParent; (void)hMenu; (void)hInstance; (void)lpParam;
    
    // Phase 36.7: SDL2 window creation for real window visibility
    // Initialize SDL if not already initialized
    static bool sdl_initialized = false;
    if (!sdl_initialized) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("CreateWindow - SDL_Init failed: %s\n", SDL_GetError());
            return NULL;
        }
        sdl_initialized = true;
        printf("CreateWindow - SDL2 initialized successfully\n");
    }
    
    // Phase 36.8: Always use macOS window decorations (ignore WS_POPUP)
    // Convert Windows style to SDL flags - force decorated window on macOS
    Uint32 sdl_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    // Note: dwStyle is intentionally ignored to keep macOS native decorations

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow(
        lpWindowName,
        x == CW_USEDEFAULT ? SDL_WINDOWPOS_CENTERED : x,
        y == CW_USEDEFAULT ? SDL_WINDOWPOS_CENTERED : y,
        nWidth,
        nHeight,
        sdl_flags
    );
    
    if (!window) {
        printf("CreateWindow - SDL_CreateWindow failed: %s\n", SDL_GetError());
        return NULL;
    }
    
    printf("CreateWindow - SDL2 window created: %s (%dx%d at %d,%d)\n", 
           lpWindowName, nWidth, nHeight, x, y);
    
    return (HWND)window; // Return SDL_Window* as HWND
}

typedef LONG (*PTOP_LEVEL_EXCEPTION_FILTER)(EXCEPTION_POINTERS* ExceptionInfo);

inline PTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(PTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
    (void)lpTopLevelExceptionFilter;
    return NULL;
}

inline HWND FindWindow(LPCSTR lpClassName, LPCSTR lpWindowName) {
    (void)lpClassName; (void)lpWindowName;
    return NULL; // Window not found
}

inline void* LoadImage(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    (void)hInst; (void)name; (void)type; (void)cx; (void)cy; (void)fuLoad;
    return (void*)1; // Dummy image handle
}
#ifndef _WIN32
// Windows calling convention stubs
#define WINAPI
#define __stdcall

// Windows far pointer compatibility
#define FAR

// Windows types needed for ICMP
typedef void* LPVOID;
#endif

// Additional Windows APIs
// Floating-point control functions
inline void _fpreset() {
    // Microsoft floating-point reset - stub for cross-platform
}

inline unsigned int _statusfp() {
    // Microsoft floating-point status - stub for cross-platform
    return 0;
}

inline unsigned int _controlfp(unsigned int newval, unsigned int mask) {
    // Microsoft floating-point control - stub for cross-platform
    return 0;
}

// Floating-point control constants
#define _MCW_RC     0x00000300  /* Rounding Control */
#define _MCW_PC     0x00030000  /* Precision Control */
#define _RC_NEAR    0x00000000  /* Near */
#define _RC_DOWN    0x00000100  /* Down */
#define _RC_UP      0x00000200  /* Up */
#define _RC_CHOP    0x00000300  /* Chop */
#define _PC_24      0x00020000  /* 24 bits */
#define _PC_53      0x00010000  /* 53 bits */
#define _PC_64      0x00000000  /* 64 bits */

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

inline DWORD GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (DWORD)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

// PeekMessage constants
#define PM_NOREMOVE     0x0000
#define PM_REMOVE       0x0001

// Windows Message constants
#define WM_SIZE         0x0005
#define WM_ACTIVATE     0x0006
#define WM_SETFOCUS     0x0007
#define WM_KILLFOCUS    0x0008
#define WM_ACTIVATEAPP  0x001C
#define WM_PAINT        0x000F
#define WM_CLOSE        0x0010
#define WM_QUIT         0x0012
#define WM_SYSCOMMAND   0x0112
#define WM_POWERBROADCAST 0x0218
#define WM_QUERYENDSESSION 0x0011
#define WM_MOVE         0x0003
#define WM_SETCURSOR    0x0020
#define WM_ERASEBKGND   0x0014
#define WM_KEYDOWN      0x0100
#define WM_KEYUP        0x0101
#define WM_CHAR         0x0102
#define WM_NCHITTEST    0x0084
#define WM_MOUSEMOVE    0x0200
#define WM_LBUTTONDOWN  0x0201
#define WM_LBUTTONUP    0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN  0x0204
#define WM_RBUTTONUP    0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN  0x0207
#define WM_MBUTTONUP    0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL   0x020A

// Hit test constants for WM_NCHITTEST
#define HTCLIENT        1

// System command constants for WM_SYSCOMMAND
#define SC_KEYMENU      0xF100
#define SC_MOVE         0xF010
#define SC_SIZE         0xF000
#define SC_MAXIMIZE     0xF030
#define SC_MONITORPOWER 0xF170

// Window activation constants
#define WA_INACTIVE     0

// Note: WS_* and SW_* constants moved to top of file for SDL2 window creation

// Window class styles
#define CS_HREDRAW      0x0002
#define CS_VREDRAW      0x0001
#define CS_DBLCLKS      0x0008

// GDI constants
#define SRCCOPY         0x00CC0020
#define BLACK_BRUSH     4

// System metrics constants
#define SM_CXSCREEN     0
#define SM_CYSCREEN     1

// Image loading constants
#define IMAGE_BITMAP    0
#define LR_SHARED       0x8000
#define LR_LOADFROMFILE 0x0010

// Exception handling constants
#define EXCEPTION_EXECUTE_HANDLER 1

// Note: ShowWindow constants (SW_*) moved to top of file for SDL2 window creation

// Resource macros
#define MAKEINTRESOURCE(i) ((LPCSTR)((ULONG_PTR)((WORD)(i))))

// Text macro for Unicode/ANSI compatibility
#define TEXT(quote) quote

// Extended window styles
#define WS_EX_TOPMOST   0x00000008L

// Window positioning constants
#define HWND_TOP        ((HWND)0)

// Windows message structure
typedef struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
} MSG, *PMSG, *LPMSG;

// Additional Windows functions
inline BOOL IsIconic(HWND hWnd) {
    (void)hWnd;
    return FALSE; // Stub: assume window is not minimized
}

inline BOOL PeekMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    (void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax; (void)wRemoveMsg;
    return FALSE; // No messages pending
}

inline BOOL GetMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    (void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax;
    return FALSE; // No messages
}

inline BOOL TranslateMessage(const MSG* lpMsg) {
    (void)lpMsg;
    return TRUE;
}

inline LRESULT DispatchMessage(const MSG* lpMsg) {
    (void)lpMsg;
    return 0;
}

inline UINT SetErrorMode(UINT uMode) {
    (void)uMode;
    return 0; // Previous error mode
}

// Error mode constants
#define SEM_FAILCRITICALERRORS 0x0001

// MessageBox constants
#define IDOK     1
#define IDCANCEL 2
#define IDABORT  3
#define IDRETRY  4
#define IDIGNORE 5
#define IDYES    6
#define IDNO     7

// Additional Windows API functions for SaveGame and GameEngine (only if not already defined)
#ifndef MESSAGEBOX_DEFINED
#define MESSAGEBOX_DEFINED
inline int MessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    printf("MessageBox: %s - %s\n", lpCaption ? lpCaption : "Alert", lpText ? lpText : "");
    return IDOK;
}

inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
    printf("MessageBoxW: Alert - (wide string message)\n");
    return IDOK;
}
#endif

#ifndef DELETEFILE_DEFINED
#define DELETEFILE_DEFINED
inline BOOL DeleteFile(LPCSTR lpFileName) {
    return unlink(lpFileName) == 0 ? TRUE : FALSE;
}
#endif

#ifndef CREATEDIRECTORY_DEFINED
#define CREATEDIRECTORY_DEFINED
inline BOOL CreateDirectory(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    return mkdir(lpPathName, 0755) == 0 ? TRUE : FALSE;
}
#endif

inline int _access(const char* path, int mode) {
    return access(path, mode);
}

// COM module stub for non-Windows platforms
class CComModule {
public:
    int Init(void* obj_map, HINSTANCE instance) { return 0; }
    void Term() {}
    HRESULT RegisterServer(BOOL bRegTypeLib = FALSE) { return S_OK; }
    HRESULT UnregisterServer(BOOL bUnRegTypeLib = FALSE) { return S_OK; }
};

// COM object stub for non-Windows platforms
template<typename T>
class CComObject : public T {
public:
    CComObject() {}
    virtual ~CComObject() {}
};

#endif // !_WIN32

// Cross-platform symbol implementations for undefined symbols
#ifndef _WIN32

// Global variables needed for linking
// Note: g_LastErrorDump is declared in GeneralsMD/Code/GameEngine/Include/Common/StackDump.h
// as AsciiString, not char array
// char g_LastErrorDump[4096];

// Forward declaration for WebBrowser class
// Note: TheWebBrowser declaration moved to WebBrowser.h to avoid conflicts with CComObject<WebBrowser>
// class WebBrowser;
// extern WebBrowser* TheWebBrowser;

// Forward declaration for StringClass (defined in wwstring.h)
class StringClass;

// Registry functions - for StringClass-dependent functions, include wwstring.h first
// Note: Real implementations are in WWDownload/Registry.h

// Remove stub functions that conflict with real implementations
// GetStringFromRegistry and SetStringInRegistry are implemented in Registry.h

// Registry helper functions not covered by Registry.h
inline bool GetRegistryKeyExists(const std::string& path, const std::string& key) {
    // Stub for checking if key exists
    return false;
}

// Note: SetUnsignedIntInRegistry and GetUnsignedIntFromRegistry are implemented in Registry.h

inline bool GetStringFromGeneralsRegistry(const std::string& path, const std::string& key, std::string& val) {
    // This function should use the real Registry.h implementation
    extern bool GetStringFromRegistry(std::string path, std::string key, std::string& val);
    return GetStringFromRegistry(path, key, val);
}

// Debug/Stack trace functions
inline void FillStackAddresses(void** addresses, unsigned int maxDepth, unsigned int skipFrames) {
    for (unsigned int i = 0; i < maxDepth; i++) {
        addresses[i] = nullptr;
    }
}

inline void StackDumpFromAddresses(void** addresses, unsigned int count, void (*output)(const char*)) {
    if (output) {
        output("Stack dump not available on this platform\n");
    }
}

// Stub class implementations  
// Note: RegistryClass is defined in registry.h - no need to redefine here

// Note: W3DRendererAdapter is a real class in Core/Libraries/Include/GraphicsAPI/W3DRendererAdapter.h
// Removed stub namespace to avoid conflicts

// Note: All stub class implementations removed - using real implementations from GameEngine

// Cross-platform function stubs for functions referenced but not found

// Global error dump variable (used in Debug.cpp)
// extern char g_LastErrorDump[4096];  // Commented out to avoid type conflict with AsciiString version

// Registry language function stub - forward declaration
// Implementation should be in GeneralsMD/Code/GameEngine/Source/Common/System/registry.cpp
// DEFERRED: These functions require AsciiString type, deferred to Phase 51+
// AsciiString GetRegistryLanguage(void);

// Generals-specific registry function stub  
// DEFERRED: bool GetStringFromGeneralsRegistry(AsciiString section, AsciiString key, AsciiString& value);

#endif // !_WIN32

#endif // WIN32_COMPAT_H_INCLUDED

// D3D Color macros
#define D3DCOLOR_RGBA(r, g, b, a) ((unsigned int)((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF))
#define D3DCOLOR_ARGB(a, r, g, b) ((unsigned int)((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF))

#endif // WIN32_COMPAT_H_INCLUDED
