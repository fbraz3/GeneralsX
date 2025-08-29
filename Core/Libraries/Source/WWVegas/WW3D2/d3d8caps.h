#pragma once

#ifndef _WIN32

// DirectX 8 caps compatibility header for non-Windows systems
#include "d3d8.h"

// Only define if not already defined
#ifndef D3DCAPS8_DEFINED
#define D3DCAPS8_DEFINED
// Already defined in d3d8.h
#endif

#else
// On Windows, include the real DirectX headers
#include <d3d8caps.h>
#endif // !_WIN32
