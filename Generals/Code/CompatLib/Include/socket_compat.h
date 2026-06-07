/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

// GeneralsX @build fbraz 10/02/2026
// Socket compatibility layer - Win32 Sockets → POSIX BSD sockets
// Pattern: Own implementation (fighter19 excluded WWDownload entirely)

#pragma once

#include "types_compat.h"
#include "com_compat.h"

#ifndef _WIN32

// POSIX socket headers
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>    // mkdir()
#include <sys/ioctl.h>   // ioctl() for ioctlsocket()
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <filesystem>    // std::filesystem for CreateDirectory
#include <fcntl.h>
#include <errno.h>

// Win32 socket type mappings → POSIX
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

// Winsock data structures → BSD equivalents
typedef struct _WSADATA {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
} WSADATA, *LPWSADATA;

// Winsock error codes → POSIX errno mapping
#define WSAEWOULDBLOCK      EWOULDBLOCK
#define WSAEINPROGRESS      EINPROGRESS
#define WSAEALREADY         EALREADY
#define WSAENOTSOCK         ENOTSOCK
#define WSAEDESTADDRREQ     EDESTADDRREQ
#define WSAEMSGSIZE         EMSGSIZE
#define WSAEPROTOTYPE       EPROTOTYPE
#define WSAENOPROTOOPT      ENOPROTOOPT
#define WSAEPROTONOSUPPORT  EPROTONOSUPPORT
#define WSAEOPNOTSUPP       EOPNOTSUPP
#define WSAEAFNOSUPPORT     EAFNOSUPPORT
#define WSAEADDRINUSE       EADDRINUSE
#define WSAEADDRNOTAVAIL    EADDRNOTAVAIL
#define WSAENETDOWN         ENETDOWN
#define WSAENETUNREACH      ENETUNREACH
#define WSAENETRESET        ENETRESET
#define WSAECONNABORTED     ECONNABORTED
#define WSAECONNRESET       ECONNRESET
#define WSAENOBUFS          ENOBUFS
#define WSAEISCONN          EISCONN
#define WSAENOTCONN         ENOTCONN
#define WSAETIMEDOUT        ETIMEDOUT
#define WSAECONNREFUSED     ECONNREFUSED
#define WSAELOOP            ELOOP
#define WSAENAMETOOLONG     ENAMETOOLONG
#define WSAEHOSTUNREACH     EHOSTUNREACH
#define WSAENOTEMPTY        ENOTEMPTY
#define WSAEINVAL           EINVAL

// Winsock initialization/cleanup (POSIX: no-op, always available)
inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
    if (lpWSAData) {
        lpWSAData->wVersion = wVersionRequested;
        lpWSAData->wHighVersion = wVersionRequested;
    }
    return 0; // Success
}

inline int WSACleanup() {
    return 0; // Success (no-op on POSIX)
}

inline int WSAGetLastError() {
    return errno; // POSIX uses global errno
}

// Socket option manipulation
inline int ioctlsocket(SOCKET s, long cmd, unsigned long* argp) {
    return ioctl(s, cmd, argp);
}

// Win32 FIONBIO → POSIX O_NONBLOCK
#ifndef FIONBIO
#define FIONBIO 0x5421  // Linux ioctl number for non-blocking mode
#endif

// Win32 closesocket → POSIX close
inline int closesocket(SOCKET s) {
    return close(s);
}

// Helper macros for version packing (Winsock uses WORD version)
#ifndef MAKEWORD
#define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((w) & 0xFF))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

// Windows registry error codes (used in FTP.cpp registry checks)
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif
#ifndef REG_DWORD
#define REG_DWORD 4
#endif

// Windows registry types (stubs - Linux has no registry)
#ifndef HKEY_LOCAL_MACHINE
typedef void* HKEY;
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002)
#define HKEY_CURRENT_USER  ((HKEY)0x80000001)
#endif

#ifndef LPDWORD
typedef DWORD* LPDWORD;
#endif

#ifndef KEY_READ
#define KEY_READ 0x20019
#endif
#ifndef KEY_WRITE
#define KEY_WRITE 0x20006
#endif

// Registry value types
#ifndef REG_SZ
#define REG_SZ 1
#endif
#ifndef REG_OPTION_NON_VOLATILE
#define REG_OPTION_NON_VOLATILE 0x00000000
#endif

// Registry function stubs (return failure - no registry on Linux)
inline long RegOpenKeyEx(HKEY hKey, const char* lpSubKey, DWORD ulOptions, 
                         DWORD samDesired, HKEY* phkResult) {
    (void)hKey; (void)lpSubKey; (void)ulOptions; (void)samDesired; (void)phkResult;
    return 1; // ERROR_FILE_NOT_FOUND equivalent
}

inline long RegQueryValueEx(HKEY hKey, const char* lpValueName, LPDWORD lpReserved,
                            LPDWORD lpType, BYTE* lpData, LPDWORD lpcbData) {
    (void)hKey; (void)lpValueName; (void)lpReserved; (void)lpType; (void)lpData; (void)lpcbData;
    return 1; // ERROR_FILE_NOT_FOUND equivalent
}

inline long RegCloseKey(HKEY hKey) {
    (void)hKey;
    return 0; // Success (no-op)
}

inline long RegCreateKeyEx(HKEY hKey, const char* lpSubKey, DWORD Reserved,
                           char* lpClass, DWORD dwOptions, DWORD samDesired,
                           void* lpSecurityAttributes, HKEY* phkResult, DWORD* lpdwDisposition) {
    (void)hKey; (void)lpSubKey; (void)Reserved; (void)lpClass; (void)dwOptions;
    (void)samDesired; (void)lpSecurityAttributes; (void)phkResult; (void)lpdwDisposition;
    return 1; // ERROR_FILE_NOT_FOUND equivalent
}

inline long RegSetValueEx(HKEY hKey, const char* lpValueName, DWORD Reserved,
                          DWORD dwType, const BYTE* lpData, DWORD cbData) {
    (void)hKey; (void)lpValueName; (void)Reserved; (void)dwType; (void)lpData; (void)cbData;
    return 1; // ERROR_FILE_NOT_FOUND equivalent
}

// CreateDirectory stub (Linux: uses C++17 std::filesystem for recursive creation)
// FTP.cpp and GameState.cpp use this for download/save directories
// GeneralsX @build BenderAI 11/02/2026 - Upgraded to recursive mkdir
inline int CreateDirectory(const char* lpPathName, void* lpSecurityAttributes) {
    (void)lpSecurityAttributes;
    try {
        return std::filesystem::create_directories(lpPathName) ? 1 : 0;
    } catch (...) {
        return 0; // FALSE
    }
}

// GeneralsX @build BenderAI 13/02/2026 Removed CreateThread stub - now provided by thread_compat.h
// (thread_compat.h included via windows_compat.h since Session 31)
// LPTHREAD_START_ROUTINE typedef removed - use start_routine from thread_compat.h instead

// OutputDebugString stub (FTP.cpp debug logging)
// GeneralsX @build BenderAI 11/02/2026 - Guard against old compat layer macro collision
#ifndef DEPENDENCIES_UTILITY_COMPAT_H
inline void OutputDebugString(const char* lpOutputString) {
    // On Linux: write to stderr (equivalent to Windows debug output)
    if (lpOutputString) {
        fprintf(stderr, "[DEBUG] %s", lpOutputString);
    }
}
#endif

// strlcpy weak symbol (FTP.cpp uses for safe string copy)
// Declared weak to avoid conflicts with BSD libc or Dependencies/Utility version
extern "C" __attribute__((weak))
size_t strlcpy(char *dst, const char *src, size_t dsize) {
    const char *osrc = src;
    size_t nleft = dsize;
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') break;
        }
    }
    if (nleft == 0) {
        if (dsize != 0) *dst = '\0';
        while (*src++) ;
    }
    return(src - osrc - 1);
}

// GeneralsX @TheSuperHackers @build BenderAI 11/02/2026 strlcat weak symbol (Download.cpp, FTP.cpp)
extern "C" __attribute__((weak))
size_t strlcat(char *dst, const char *src, size_t dsize) {
   const char *odst = dst;
    const char *osrc = src;
    size_t n = dsize;
    size_t dlen;

    // Find end of dst and adjust n
    while (n-- != 0 && *dst != '\0')
        dst++;
    dlen = dst - odst;
    n = dsize - dlen;

    if (n-- == 0)
        return(dlen + strlen(src));
    while (*src != '\0') {
        if (n != 0) {
            *dst++ = *src;
            n--;
        }
        src++;
    }
    *dst = '\0';

    return(dlen + (src - osrc)); // Total length that would have been created
}

#endif // !_WIN32
