/* Minimal stub of d3d8types.h for non-Windows platforms.
   Provide only what is missing on POSIX builds and avoid
   redefining types already declared by win32 compatibility headers.
*/
#ifndef D3D8TYPES_STUB_H
#define D3D8TYPES_STUB_H

#include <stdint.h>

// Only define integer aliases if they are not already provided
#ifndef DWORD
typedef unsigned long DWORD;
#endif

#ifndef LONG
typedef long LONG;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef UINT
typedef unsigned int UINT;
#endif

// Colour and handle types
#ifndef D3DCOLOR
typedef DWORD D3DCOLOR;
#endif

// COM/Automation pointer stub used by some browser helpers
#ifndef LPDISPATCH
typedef void* LPDISPATCH;
#endif

// HRESULT-like type
#ifndef HRESULT
typedef LONG HRESULT;
#endif

#endif /* D3D8TYPES_STUB_H */
