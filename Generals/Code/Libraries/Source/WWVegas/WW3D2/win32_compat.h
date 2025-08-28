#pragma once

#ifndef WIN32_COMPAT_H_INCLUDED
#define WIN32_COMPAT_H_INCLUDED

#ifndef _WIN32
// Compatibility types for non-Windows systems

// Basic Windows types
typedef void* HDC;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef int BOOL;
typedef long LONG;
typedef void* LPVOID;
typedef const char* LPCSTR;
typedef char* LPSTR;

// Constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Function compatibility
#define _strdup strdup
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

// DirectX basic types
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

#endif // !_WIN32

#endif // WIN32_COMPAT_H_INCLUDED
