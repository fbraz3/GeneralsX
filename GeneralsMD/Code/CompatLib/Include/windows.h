#pragma once

// Linux/Unix compatibility shim for Windows.h
// On Linux: Include DXVK windows_base.h FIRST, then compatibility layer
// On Windows: Just use real Windows SDK

#ifdef _WIN32
// Windows: use real Windows.h from SDK (will be found in system paths first)
// This header is only reached if no SDK windows.h exists
#include "windows_compat.h"
#else
// Linux: DXVK windows_base.h provides core Windows types
// CRITICAL: Include windows_base.h BEFORE any D3D headers
#include <windows_base.h>

// Then add our additional compatibility types/macros
#include "windows_compat.h"
#endif
