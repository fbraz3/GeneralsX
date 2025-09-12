#ifndef NETWORK_COMPAT_H
#define NETWORK_COMPAT_H

#ifdef __APPLE__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

// Windows network types
typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct in_addr IN_ADDR;
typedef unsigned short u_short;
typedef unsigned long u_long;

// Network constants
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define INADDR_ANY 0x00000000
#define INADDR_LOOPBACK 0x7f000001

// Redefine conflicting functions with unique names to avoid symbol conflicts
namespace Win32Net {
    inline int compat_getsockname(SOCKET s, struct sockaddr* name, int* namelen) {
        socklen_t len = (socklen_t)*namelen;
        int result = ::getsockname(s, name, &len);
        *namelen = (int)len;
        return result;
    }

    inline int compat_bind(SOCKET s, const struct sockaddr* addr, int namelen) {
        return ::bind(s, addr, (socklen_t)namelen);
    }

    inline int compat_connect(SOCKET s, const struct sockaddr* name, int namelen) {
        return ::connect(s, name, (socklen_t)namelen);
    }

    inline int compat_listen(SOCKET s, int backlog) {
        return ::listen(s, backlog);
    }

    inline SOCKET compat_accept(SOCKET s, struct sockaddr* addr, int* addrlen) {
        socklen_t len = addrlen ? (socklen_t)*addrlen : 0;
        SOCKET result = ::accept(s, addr, addrlen ? &len : NULL);
        if (addrlen) *addrlen = (int)len;
        return result;
    }

    inline int compat_send(SOCKET s, const char* buf, int len, int flags) {
        return (int)::send(s, buf, (size_t)len, flags);
    }

    inline int compat_recv(SOCKET s, char* buf, int len, int flags) {
        return (int)::recv(s, buf, (size_t)len, flags);
    }

    inline int compat_sendto(SOCKET s, const char* buf, int len, int flags, 
                            const struct sockaddr* to, int tolen) {
        return (int)::sendto(s, buf, (size_t)len, flags, to, (socklen_t)tolen);
    }

    inline int compat_recvfrom(SOCKET s, char* buf, int len, int flags,
                              struct sockaddr* from, int* fromlen) {
        socklen_t flen = fromlen ? (socklen_t)*fromlen : 0;
        int result = (int)::recvfrom(s, buf, (size_t)len, flags, from, fromlen ? &flen : NULL);
        if (fromlen) *fromlen = (int)flen;
        return result;
    }

    inline int closesocket(SOCKET s) {
        return close(s);
    }

    inline SOCKET compat_socket(int af, int type, int protocol) {
        return ::socket(af, type, protocol);
    }

    inline int compat_setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen) {
        return ::setsockopt(s, level, optname, optval, (socklen_t)optlen);
    }

    inline int compat_getsockopt(SOCKET s, int level, int optname, char* optval, int* optlen) {
        socklen_t len = (socklen_t)*optlen;
        int result = ::getsockopt(s, level, optname, optval, &len);
        *optlen = (int)len;
        return result;
    }

    inline unsigned long compat_inet_addr(const char* cp) {
        return (unsigned long)::inet_addr(cp);
    }

    inline char* compat_inet_ntoa(struct in_addr in) {
        return ::inet_ntoa(in);
    }

    inline u_short compat_htons(u_short hostshort) {
        return ::htons(hostshort);
    }

    inline u_short compat_ntohs(u_short netshort) {
        return ::ntohs(netshort);
    }

    inline u_long compat_htonl(u_long hostlong) {
        return ::htonl(hostlong);
    }

    inline u_long compat_ntohl(u_long netlong) {
        return ::ntohl(netlong);
    }
}

// Map Windows function names to our compatibility functions
#define getsockname Win32Net::compat_getsockname
#define bind Win32Net::compat_bind
#define connect Win32Net::compat_connect
#define listen Win32Net::compat_listen
#define accept Win32Net::compat_accept
#define send Win32Net::compat_send
#define recv Win32Net::compat_recv
#define sendto Win32Net::compat_sendto
#define recvfrom Win32Net::compat_recvfrom
#define socket Win32Net::compat_socket
#define setsockopt Win32Net::compat_setsockopt
#define getsockopt Win32Net::compat_getsockopt
#define inet_addr Win32Net::compat_inet_addr
#define inet_ntoa Win32Net::compat_inet_ntoa
#define htons Win32Net::compat_htons
#define ntohs Win32Net::compat_ntohs
#define htonl Win32Net::compat_htonl
#define ntohl Win32Net::compat_ntohl

// WSA functions (Windows Socket API)
inline int WSAStartup(u_short wVersionRequested, void* lpWSAData) {
    (void)wVersionRequested; (void)lpWSAData;
    return 0;
}

inline int WSACleanup(void) {
    return 0;
}

inline int WSAGetLastError(void) {
    return errno;
}

#endif // __APPLE__

#endif // NETWORK_COMPAT_H