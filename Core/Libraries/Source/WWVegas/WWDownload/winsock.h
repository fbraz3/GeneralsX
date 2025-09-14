#ifndef WINSOCK_H
#define WINSOCK_H

#ifdef _WIN32
    #include <winsock2.h>
#else
    // Unix/macOS socket compatibility
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <sys/ioctl.h>
    
    // Include our network compatibility layer
    #include "../WW3D2/network.h"
    
    // Windows socket types
    typedef int SOCKET;
    typedef struct sockaddr SOCKADDR;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct hostent HOSTENT;
    
    // Windows socket constants
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define WSAEWOULDBLOCK EAGAIN
    #define WSAECONNRESET ECONNRESET
    #define WSAECONNABORTED ECONNABORTED
    #define WSAEINPROGRESS EINPROGRESS
    #define WSAEALREADY EALREADY
    #define WSAEINVAL EINVAL
    #define WSAEISCONN EISCONN
    #define WSAENOTCONN ENOTCONN
    #define WSAETIMEDOUT ETIMEDOUT
    
    // Windows in_addr compatibility - map S_un.S_addr to s_addr
    #define S_un_S_addr s_addr
    
    // Windows socket functions defined in network.h
    // (WSAStartup, WSACleanup, WSAGetLastError)
    #ifndef closesocket
    inline int closesocket(SOCKET s) { return close(s); }
    #endif
    inline int ioctlsocket(SOCKET s, long cmd, unsigned long* argp) { 
        return ioctl(s, cmd, argp); 
    }
    
    // Socket option constants (don't redefine)
    #ifndef SO_REUSEADDR
    #define SO_REUSEADDR 2
    #endif
    
    #ifndef SOMAXCONN
    #define SOMAXCONN 128
    #endif
    
    // Windows COM types
    typedef long HRESULT;
    typedef const char* LPCSTR;
    
    // Common HRESULT values
    #define S_OK 0
    #define S_FALSE 1
    #define E_FAIL 0x80004005L
    
    // HRESULT macros
    #define MAKE_HRESULT(sev,fac,code) ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))))
    #define SEVERITY_ERROR 1
    #define FACILITY_ITF 4
    
#endif

#endif // WINSOCK_H
