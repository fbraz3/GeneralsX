#pragma once

// Linux/Unix compatibility shim for Windows.h
// GeneralsX @build BenderAI 11/02/2026 Windows API compatibility layer
// STRATEGY: Provide our own complete Windows types/functions FIRST.
// DXVK headers (d3d8.h) will include their own windows_base.h later.
// Our types take precedence via PCH (PreRTS.h includes windows_compat.h early).

#ifdef _WIN32
#include <windows.h>
#else
// Linux/macOS: Our compatibility layer only (prevents DXVK unknwn.h redefinition)
#include "windows_compat.h"
#endif
