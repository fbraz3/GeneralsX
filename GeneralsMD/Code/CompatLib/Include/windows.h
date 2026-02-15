#pragma once

// Linux/Unix compatibility shim for Windows.h
// GeneralsX @build BenderAI 11/02/2026 Windows API compatibility layer
// STRATEGY: Provide our own complete Windows types/functions FIRST.
// DXVK headers (d3d8.h) will include their own windows_base.h later.
// Our types take precedence via PCH (PreRTS.h includes windows_compat.h early).

#ifdef _WIN32
// Windows: use real Windows.h from SDK (will be found in system paths first)
// This header is only reached if no SDK windows.h exists
#include "windows_compat.h"
#else
// Linux: Our compatibility layer only (NO DXVK headers here!)
// DXVK's windows_base.h will be included by d3d8.h when needed
#include "windows_compat.h"
#endif
