#pragma once
#ifndef _WINDOWS_H_
#define _WINDOWS_H_

// Basic Windows compatibility for macOS port
#ifdef __APPLE__

// Compiler intrinsics - define early for debug compatibility
#define __forceinline inline __attribute__((always_inline))
#define __stdcall

#include <stdint.h>
#include <cstdint>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <wchar.h>
#include <stdarg.h>

// Basic Windows types - check if already defined
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#ifndef WORD
typedef uint16_t WORD;
#endif
#ifndef BYTE
typedef uint8_t BYTE;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef SIZE_T
typedef size_t SIZE_T;
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
#ifndef HBITMAP
typedef void* HBITMAP;
#endif
#ifndef HFONT
typedef void* HFONT;
#endif
#ifndef HBRUSH
typedef void* HBRUSH;
#endif
#ifndef HPEN
typedef void* HPEN;
#endif
#ifndef HMENU
typedef void* HMENU;
#endif
#ifndef HICON
typedef void* HICON;
#endif
#ifndef HCURSOR
typedef void* HCURSOR;
#endif
#ifndef HPALETTE
typedef void* HPALETTE;
#endif
#ifndef HMETAFILE
typedef void* HMETAFILE;
#endif
#ifndef HENHMETAFILE
typedef void* HENHMETAFILE;
#endif
#ifndef HKEY
typedef void* HKEY;
#endif
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef LONG
typedef long LONG;
#endif
#ifndef ULONG
typedef unsigned long ULONG;
#endif
#ifndef LONGLONG
typedef long long LONGLONG;
#endif
#ifndef CHAR
typedef char CHAR;
#endif
#ifndef WCHAR
typedef wchar_t WCHAR;
#endif
#ifndef LPSTR
typedef CHAR* LPSTR;
#endif
#ifndef LPCSTR
typedef const CHAR* LPCSTR;
#endif
#ifndef LPWSTR
typedef WCHAR* LPWSTR;
#endif
#ifndef LPCWSTR
typedef const WCHAR* LPCWSTR;
#endif
#ifndef LPVOID
typedef void* LPVOID;
#endif
#ifndef LPCVOID
typedef const void* LPCVOID;
#endif
#ifndef HRESULT
typedef long HRESULT;
#endif

// Integer types for compatibility
typedef long long _int64;
typedef long long __int64;
typedef unsigned long long __uint64;
typedef unsigned long long _uint64;

// Handle 'unsigned _int64' as a special case
#define unsigned__int64 unsigned long long

// Boolean constants
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Common constants
#define MAX_PATH 260
#define INFINITE 0xFFFFFFFF
#define INVALID_HANDLE_VALUE ((HANDLE)(long long)-1)

// Error codes
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_PATH_NOT_FOUND 3L
#define ERROR_ACCESS_DENIED 5L

// Registry stubs
#define HKEY_LOCAL_MACHINE ((void*)0x80000002)
#define HKEY_CURRENT_USER ((void*)0x80000001)
#define KEY_READ 0x20019
#define REG_DWORD 4

// Calling conventions
#define WINAPI
#define CALLBACK
#define STDMETHODCALLTYPE

// Memory management stubs
inline void* LocalAlloc(DWORD flags, size_t size) { return malloc(size); }
inline void* LocalFree(void* ptr) { free(ptr); return nullptr; }

// Threading stubs
typedef struct _CRITICAL_SECTION {
    pthread_mutex_t mutex;
} CRITICAL_SECTION, *LPCRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION* cs) {
    pthread_mutex_init(&cs->mutex, nullptr);
}

inline void DeleteCriticalSection(CRITICAL_SECTION* cs) {
    pthread_mutex_destroy(&cs->mutex);
}

inline void EnterCriticalSection(CRITICAL_SECTION* cs) {
    pthread_mutex_lock(&cs->mutex);
}

inline void LeaveCriticalSection(CRITICAL_SECTION* cs) {
    pthread_mutex_unlock(&cs->mutex);
}

// Thread creation stub
typedef DWORD (*LPTHREAD_START_ROUTINE)(void*);
inline HANDLE CreateThread(void* attr, size_t stack, LPTHREAD_START_ROUTINE start, void* param, DWORD flags, DWORD* threadId) {
    return nullptr; // Stub implementation
}

// Event functions
inline HANDLE CreateEvent(void* security, BOOL manual_reset, BOOL initial_state, LPCSTR name) {
    return nullptr; // Stub implementation
}

inline BOOL SetEvent(HANDLE event) {
    return TRUE; // Stub
}

inline BOOL ResetEvent(HANDLE event) {
    return TRUE; // Stub
}

inline DWORD WaitForSingleObject(HANDLE handle, DWORD milliseconds) {
    return 0; // Stub
}

// File handling stubs
#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_NORMAL 0x80

inline HANDLE CreateFileA(LPCSTR filename, DWORD access, DWORD share, void* security, DWORD creation, DWORD flags, HANDLE template_file) {
    return INVALID_HANDLE_VALUE; // Stub
}

inline BOOL CloseHandle(HANDLE handle) {
    return TRUE; // Stub
}

inline BOOL ReadFile(HANDLE file, LPVOID buffer, DWORD bytes_to_read, DWORD* bytes_read, void* overlapped) {
    return FALSE; // Stub
}

inline BOOL WriteFile(HANDLE file, LPCVOID buffer, DWORD bytes_to_write, DWORD* bytes_written, void* overlapped) {
    return FALSE; // Stub
}

// Registry stubs
inline LONG RegOpenKeyExA(HANDLE key, LPCSTR subkey, DWORD options, DWORD sam, HANDLE* result) {
    return ERROR_FILE_NOT_FOUND; // Stub
}

inline LONG RegQueryValueExA(HANDLE key, LPCSTR value, DWORD* reserved, DWORD* type, BYTE* data, DWORD* size) {
    return ERROR_FILE_NOT_FOUND; // Stub
}

inline LONG RegCloseKey(HANDLE key) {
    return ERROR_SUCCESS; // Stub
}

// String functions
inline int _stricmp(const char* str1, const char* str2) {
    return strcasecmp(str1, str2);
}

inline int _strnicmp(const char* str1, const char* str2, size_t count) {
    return strncasecmp(str1, str2, count);
}

// Windows sprintf functions
inline int wsprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}

// System info stubs - check if already defined
#ifndef GetTickCount
inline DWORD GetTickCount() {
    return (DWORD)(clock() * 1000 / CLOCKS_PER_SEC);
}
#endif

inline void Sleep(DWORD milliseconds) {
    usleep(milliseconds * 1000);
}

// Math function stub
inline LONG MulDiv(LONG number, LONG numerator, LONG denominator) {
    if (denominator == 0) return 0;
    return (LONG)((long long)number * numerator / denominator);
}

// Memory allocation types and functions
typedef void* HGLOBAL;
#define GMEM_FIXED 0x0000

inline HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
    return malloc(dwBytes);
}

inline HGLOBAL GlobalReAlloc(HGLOBAL hMem, SIZE_T dwBytes, UINT uFlags) {
    return realloc(hMem, dwBytes);
}

inline HGLOBAL GlobalFree(HGLOBAL hMem) {
    free(hMem);
    return NULL;
}

inline SIZE_T GlobalSize(HGLOBAL hMem) {
    // On macOS, we can't get the size of a malloc'd block
    // This is a limitation, but shouldn't affect most code
    return 0;
}

// Performance counter types and functions
#ifndef _LARGE_INTEGER_
#define _LARGE_INTEGER_
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
#endif

inline BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    if (!lpPerformanceCount) return FALSE;
    
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        lpPerformanceCount->QuadPart = ts.tv_sec * 1000000000LL + ts.tv_nsec;
        return TRUE;
    }
    return FALSE;
}

inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    if (!lpFrequency) return FALSE;
    
    // Return frequency in nanoseconds (1 billion per second)
    lpFrequency->QuadPart = 1000000000LL;
    return TRUE;
}

#endif // __APPLE__
#endif // _WINDOWS_H_
