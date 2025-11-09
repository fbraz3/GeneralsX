#pragma once

#ifndef WIN32_COMPAT_CORE_H_INCLUDED
#define WIN32_COMPAT_CORE_H_INCLUDED

#ifndef _WIN32

#include <cstdint>
#include <cstring>
#include <string>

// ============================================================================
// Core Windows Type Definitions
// 
// These are fundamental Windows types required for cross-platform compilation
// ============================================================================

// ============================================================================
// Basic Integer Types
// ============================================================================

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

#ifndef USHORT
typedef unsigned short USHORT;
#endif

#ifndef UCHAR
typedef unsigned char UCHAR;
#endif

#ifndef SHORT
typedef short SHORT;
#endif

#ifndef CHAR
typedef char CHAR;
#endif

#ifndef INT
typedef int INT;
#endif

#ifndef FLOAT
typedef float FLOAT;
#endif

#ifndef DOUBLE
typedef double DOUBLE;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef __int64
typedef long long __int64;
#endif

#ifndef SIZE_T
typedef unsigned long SIZE_T;
#endif

#ifndef LPVOID
typedef void* LPVOID;
#endif

#ifndef LPCVOID
typedef const void* LPCVOID;
#endif

#ifndef LPSTR
typedef char* LPSTR;
#endif

#ifndef LPCSTR
typedef const char* LPCSTR;
#endif

#ifndef LPWSTR
typedef wchar_t* LPWSTR;
#endif

#ifndef LPCWSTR
typedef const wchar_t* LPCWSTR;
#endif

#ifndef HANDLE
typedef void* HANDLE;
#endif

#ifndef HMODULE
typedef void* HMODULE;
#endif

#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif

#ifndef HWND
typedef void* HWND;
#endif

#ifndef HDC
typedef void* HDC;
#endif

#ifndef HPEN
typedef void* HPEN;
#endif

#ifndef HBRUSH
typedef void* HBRUSH;
#endif

#ifndef HFONT
typedef void* HFONT;
#endif

#ifndef HBITMAP
typedef void* HBITMAP;
#endif

#ifndef HICON
typedef void* HICON;
#endif

#ifndef HCURSOR
typedef void* HCURSOR;
#endif

#ifndef HMENU
typedef void* HMENU;
#endif

#ifndef HKEY
typedef void* HKEY;
#endif

#ifndef HRESULT
typedef long HRESULT;
#endif

#ifndef WPARAM
typedef uintptr_t WPARAM;
#endif

#ifndef LPARAM
typedef uintptr_t LPARAM;
#endif

#ifndef LRESULT
typedef long LRESULT;
#endif

#ifndef FARPROC
typedef void (*FARPROC)();
#endif

#ifndef LARGE_INTEGER
typedef struct {
    long LowPart;
    long HighPart;
} LARGE_INTEGER;
#endif

#ifndef ULARGE_INTEGER
typedef struct {
    unsigned long LowPart;
    unsigned long HighPart;
} ULARGE_INTEGER;
#endif

#ifndef GUID
typedef struct {
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
#endif

// ============================================================================
// Boolean Constants (MUST be defined BEFORE TRUE/FALSE usage)
// ============================================================================

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// ============================================================================
// HRESULT Return Codes (DirectX)
// ============================================================================

#define S_OK 0x00000000L
#define S_FALSE 0x00000001L
#define E_NOTIMPL 0x80004001L
#define E_NOINTERFACE 0x80004002L
#define E_POINTER 0x80004003L
#define E_ABORT 0x80004004L
#define E_FAIL 0x80004005L
#define E_UNEXPECTED 0x8000FFFFL
#define E_ACCESSDENIED 0x80070005L
#define E_HANDLE 0x80070006L
#define E_INVALIDARG 0x80070057L
#define E_OUTOFMEMORY 0x8007000EL

// ============================================================================
// Compiler-Specific Attributes
// ============================================================================

#ifndef __declspec
#define __declspec(x)
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

// ============================================================================
// Calling Conventions (No-ops on non-Windows)
// ============================================================================

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef __fastcall
#define __fastcall
#endif

// ============================================================================
// COM-Related Macros
// ============================================================================

#define CoUninitialize() do {} while(0)
#define CoInitialize(x) S_OK

// ============================================================================
// NULL Pointer
// ============================================================================

#ifndef NULL
#define NULL 0
#endif

// ============================================================================
// D3DX Math Types (for Bezier and other math code)
// ============================================================================

#ifndef D3DXMATRIX_DEFINED
#define D3DXMATRIX_DEFINED
struct D3DXMATRIX {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
	// Constructor for C++ initialization with 16 floats
	D3DXMATRIX() {}
	D3DXMATRIX(float m11, float m12, float m13, float m14,
	           float m21, float m22, float m23, float m24,
	           float m31, float m32, float m33, float m34,
	           float m41, float m42, float m43, float m44) :
		_11(m11), _12(m12), _13(m13), _14(m14),
		_21(m21), _22(m22), _23(m23), _24(m24),
		_31(m31), _32(m32), _33(m33), _34(m34),
		_41(m41), _42(m42), _43(m43), _44(m44) {}
};
#endif

#ifndef D3DXVECTOR4_DEFINED
#define D3DXVECTOR4_DEFINED
typedef struct {
	float x, y, z, w;
} D3DXVECTOR4;
#endif

#ifndef D3DXVECTOR3_DEFINED
#define D3DXVECTOR3_DEFINED
typedef struct {
	float x, y, z;
} D3DXVECTOR3;
#endif

#endif // !_WIN32

#endif // WIN32_COMPAT_CORE_H_INCLUDED
