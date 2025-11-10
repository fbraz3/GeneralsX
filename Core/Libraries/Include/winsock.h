// Minimal winsock.h shim for non-Windows platforms
// Redirects legacy <winsock.h> includes to the project's network compatibility layer.
#pragma once

#ifndef INCLUDED_WINSOCK_H_SHIM
#define INCLUDED_WINSOCK_H_SHIM

#if !defined(_WIN32)

// Provide the minimal Winsock compatibility used by the codebase.
// Prefer to include the project's canonical network compatibility header
// using flexible include checks so the shim works regardless of include
// search path ordering.

#if __has_include("network.h")
#include "network.h"
#elif __has_include(<WW3D2/network.h>)
#include <WW3D2/network.h>
#elif __has_include("../../GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/network.h")
#include "../../GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/network.h"
#else
#error "network.h not found - please ensure Core/Libraries/Source/WWVegas/WW3D2 is on the include path"
#endif

// Minimal local guards for symbols that might not be defined by the
// canonical header. Use #ifndef to avoid redefinition warnings.

#ifndef SOCKET
typedef int SOCKET;
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef MAKEWORD
#define MAKEWORD(a,b) ((unsigned short)((((unsigned short)(a)) & 0xff) | ((((unsigned short)(b)) & 0xff) << 8)))
#endif

#ifndef closesocket
#define closesocket(s) close(s)
#endif

#endif // !_WIN32

#endif // INCLUDED_WINSOCK_H_SHIM
