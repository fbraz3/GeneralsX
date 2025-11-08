/*
**	Windows API compatibility stub for non-Windows platforms
**	Provides minimal type definitions and constants from windows.h
*/

#ifndef WINDOWS_H
#define WINDOWS_H

#ifdef _WIN32
#error "This stub header should only be used on non-Windows platforms"
#endif

#include <cstdint>
#include <cstring>
#include <climits>
#include <cerrno>

// Define TRUE/FALSE first (before use)
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Basic types
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef int INT;
typedef unsigned long DWORD;
typedef long LONG;
typedef float FLOAT;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef void* LPVOID;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HKEY;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef unsigned long ULONG;
typedef unsigned char UCHAR;
typedef size_t SIZE_T;
typedef struct { int x, y; } POINT;
typedef struct { long left, top, right, bottom; } RECT;
typedef struct { unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds; } SYSTEMTIME;

// Handle constants
#define NULL 0
#define INVALID_HANDLE_VALUE ((void*)-1)

// Registry-related (stubbed for non-Windows)
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002L)
#define HKEY_CURRENT_USER ((HKEY)0x80000001L)
#define REG_SZ 1
#define REG_DWORD 4
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2

// File constants
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF

// Message box
#define MB_OK 0x00000000
#define MB_OKCANCEL 0x00000001
#define MB_YESNO 0x00000004
#define MB_ICONERROR 0x00000010
#define MB_ICONQUESTION 0x00000020
#define MB_ICONWARNING 0x00000030
#define MB_ICONINFORMATION 0x00000040
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

// Math constants
#define INFINITE 0xFFFFFFFF
#define MAX_PATH 260

// Module functions (stubbed)
inline HMODULE GetModuleHandleA(LPCSTR lpModuleName) { return nullptr; }
inline HMODULE GetModuleHandleW(LPCWSTR lpModuleName) { return nullptr; }
inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) { 
    if (lpFilename && nSize > 0) lpFilename[0] = '\0';
    return 0;
}
inline DWORD GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize) {
    if (lpFilename && nSize > 0) lpFilename[0] = L'\0';
    return 0;
}

// File I/O stubs
inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                         void* lpSecurityAttributes, DWORD dwCreationDisposition,
                         DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return INVALID_HANDLE_VALUE;
}
inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                    DWORD* lpNumberOfBytesRead, void* lpOverlapped) {
    return FALSE;
}
inline BOOL WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite,
                     DWORD* lpNumberOfBytesWritten, void* lpOverlapped) {
    return FALSE;
}
inline BOOL CloseHandle(HANDLE hObject) { 
    return TRUE; 
}
inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh) { 
    return INVALID_SET_FILE_POINTER; 
}

// Message boxes (stubbed)
inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return IDOK; }
inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) { return IDOK; }

// Registry stubs (all no-ops)
inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
                         DWORD samDesired, HKEY* phkResult) {
    *phkResult = nullptr;
    return ERROR_FILE_NOT_FOUND;
}
inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, DWORD* lpReserved,
                            DWORD* lpType, BYTE* lpData, DWORD* lpcbData) {
    return ERROR_FILE_NOT_FOUND;
}
inline LONG RegCloseKey(HKEY hKey) { return ERROR_SUCCESS; }

// Thread/synchronization stubs
inline DWORD GetCurrentProcessId() { return 0; }
// Note: GetCurrentThreadId() is defined in thread_compat.h - don't duplicate it
inline void Sleep(DWORD dwMilliseconds) { /* no-op */ }

// System info
inline void GetSystemTime(SYSTEMTIME* lpSystemTime) {
    if (lpSystemTime) std::memset(lpSystemTime, 0, sizeof(*lpSystemTime));
}

// Memory functions
inline LPVOID LocalAlloc(UINT uFlags, SIZE_T uBytes) { return nullptr; }
inline LPVOID LocalFree(LPVOID hMem) { return hMem; }

// Misc
inline BOOL IsDebuggerPresent() { return FALSE; }
inline void OutputDebugStringA(LPCSTR lpOutputString) { /* no-op */ }
inline void OutputDebugStringW(LPCWSTR lpOutputString) { /* no-op */ }

#endif // WINDOWS_H


#ifndef WINDOWS_H
#define WINDOWS_H

#ifdef _WIN32
#error "This stub header should only be used on non-Windows platforms"
#endif

#include <cstdint>
#include <cstring>
#include <climits>
#include <cerrno>

// Define TRUE/FALSE first
#define TRUE 1
#define FALSE 0

// Basic types
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef int INT;
typedef unsigned long DWORD;
typedef long LONG;
typedef float FLOAT;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;
typedef void* LPVOID;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HKEY;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef unsigned long ULONG;
typedef unsigned char UCHAR;
typedef size_t SIZE_T;
typedef struct { int x, y; } POINT;
typedef struct { long left, top, right, bottom; } RECT;
typedef struct { unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds; } SYSTEMTIME;

// Handle constants
#define NULL 0
#define INVALID_HANDLE_VALUE ((void*)-1)

// Registry-related (stubbed for non-Windows)
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002L)
#define HKEY_CURRENT_USER ((HKEY)0x80000001L)
#define REG_SZ 1
#define REG_DWORD 4
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2

// File constants
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF

// Message box
#define MB_OK 0x00000000
#define MB_OKCANCEL 0x00000001
#define MB_YESNO 0x00000004
#define MB_ICONERROR 0x00000010
#define MB_ICONQUESTION 0x00000020
#define MB_ICONWARNING 0x00000030
#define MB_ICONINFORMATION 0x00000040
#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

// Math constants
#define INFINITE 0xFFFFFFFF
#define MAX_PATH 260

// Module functions (stubbed)
inline HMODULE GetModuleHandleA(LPCSTR lpModuleName) { return nullptr; }
inline HMODULE GetModuleHandleW(LPCWSTR lpModuleName) { return nullptr; }
inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) { 
    if (lpFilename && nSize > 0) lpFilename[0] = '\0';
    return 0;
}
inline DWORD GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize) {
    if (lpFilename && nSize > 0) lpFilename[0] = L'\0';
    return 0;
}

// File I/O stubs
inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                         void* lpSecurityAttributes, DWORD dwCreationDisposition,
                         DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return INVALID_HANDLE_VALUE;
}
inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                    DWORD* lpNumberOfBytesRead, void* lpOverlapped) {
    return FALSE;
}
inline BOOL WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite,
                     DWORD* lpNumberOfBytesWritten, void* lpOverlapped) {
    return FALSE;
}
inline BOOL CloseHandle(HANDLE hObject) { return TRUE; }
inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh) { return INVALID_SET_FILE_POINTER; }

// Message boxes (stubbed)
inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) { return IDOK; }
inline int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) { return IDOK; }

// Registry stubs (all no-ops)
inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
                         DWORD samDesired, HKEY* phkResult) {
    *phkResult = nullptr;
    return ERROR_FILE_NOT_FOUND;
}
inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, DWORD* lpReserved,
                            DWORD* lpType, BYTE* lpData, DWORD* lpcbData) {
    return ERROR_FILE_NOT_FOUND;
}
inline LONG RegCloseKey(HKEY hKey) { return ERROR_SUCCESS; }

// Thread/synchronization stubs
inline DWORD GetCurrentProcessId() { return 0; }
inline DWORD GetCurrentThreadId() { return 0; }
inline void Sleep(DWORD dwMilliseconds) { /* no-op */ }

// System info
inline void GetSystemTime(SYSTEMTIME* lpSystemTime) {
    if (lpSystemTime) std::memset(lpSystemTime, 0, sizeof(*lpSystemTime));
}

// Memory functions
inline LPVOID LocalAlloc(UINT uFlags, SIZE_T uBytes) { return nullptr; }
inline LPVOID LocalFree(LPVOID hMem) { return hMem; }

// Misc
inline BOOL IsDebuggerPresent() { return FALSE; }
inline void OutputDebugStringA(LPCSTR lpOutputString) { /* no-op */ }
inline void OutputDebugStringW(LPCWSTR lpOutputString) { /* no-op */ }

#define TRUE 1
#define FALSE 0

#endif // WINDOWS_H
