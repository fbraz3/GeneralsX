// Minimal network compatibility header for non-Windows platforms
// Provides lightweight stubs/mappings for Winsock-style APIs used by legacy code.
#pragma once

#ifndef NETWORK_COMPAT_H
#define NETWORK_COMPAT_H

#if !defined(_WIN32)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// Basic Winsock types for compatibility
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct in_addr IN_ADDR;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

// Map closesocket to close
#ifndef closesocket
#define closesocket(s) close(s)
#endif

// Map common Winsock error codes to POSIX equivalents if not already defined
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif
#ifndef WSAEINVAL
#define WSAEINVAL EINVAL
#endif
#ifndef WSAEALREADY
#define WSAEALREADY EALREADY
#endif
#ifndef WSAEISCONN
#define WSAEISCONN EISCONN
#endif

// MAKEWORD / HOSTENT compatibility
#ifndef MAKEWORD
#define MAKEWORD(a,b) ((unsigned short)((((unsigned short)(a)) & 0xff) | ((((unsigned short)(b)) & 0xff) << 8)))
#endif

// Alias POSIX hostent to Windows-style HOSTENT used by legacy code
typedef struct hostent HOSTENT;

// Simple sockaddr helpers that mirror Windows behavior where needed
namespace Win32Net {
    inline int compat_getsockname(SOCKET s, struct sockaddr* name, int* namelen) {
        socklen_t len = (socklen_t)*namelen;
        int result = ::getsockname(s, name, &len);
        *namelen = (int)len;
        return result;
    }
}

// Lightweight WSA stubs
// These exist so legacy code can call WSAStartup/WSACleanup/WSAGetLastError
// without requiring the full Winsock API on POSIX platforms.
inline int WSAStartup(unsigned short /*wVersionRequested*/, void* /*lpWSAData*/) {
    return 0; // no-op on POSIX
}

inline int WSACleanup(void) {
    return 0; // no-op on POSIX
}

inline int WSAGetLastError(void) {
    return errno;
}

#endif // !_WIN32

#endif // NETWORK_COMPAT_H
