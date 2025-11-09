#pragma once

// Cross-platform windows.h compatibility shim
// On Windows: includes real windows.h
// On non-Windows: includes win32_compat.h

#ifdef _WIN32
// Real Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
// Non-Windows: use compatibility layer
#include "win32_compat.h"
#endif // _WIN32
