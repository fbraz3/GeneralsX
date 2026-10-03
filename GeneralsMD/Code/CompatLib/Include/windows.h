#pragma once

// Linux/Unix compatibility shim for Windows.h
// GeneralsX @build BenderAI 11/02/2026 Windows API compatibility layer
// STRATEGY: Provide our own complete Windows types/functions FIRST.
// DXVK headers (d3d8.h) will include their own windows_base.h later.
// Our types take precedence via PCH (PreRTS.h includes windows_compat.h early).

#ifdef _WIN32
// GeneralsX @bugfix GitHub Copilot 19/05/2026 On MinGW x64, forward to the system Windows SDK header to avoid CompatLib type collisions.
// Windows: always forward to the next windows.h in include search order.
#if defined(__GNUC__)
	#include_next <windows.h>
#else
	#include "windows_compat.h"
#endif
#else
// Linux: Our compatibility layer only (NO DXVK headers here!)
// DXVK's windows_base.h will be included by d3d8.h when needed
#include "windows_compat.h"
#endif
