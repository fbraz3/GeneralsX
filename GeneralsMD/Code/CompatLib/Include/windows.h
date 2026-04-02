#pragma once

// Linux/Unix compatibility shim for Windows.h
// GeneralsX @build BenderAI 11/02/2026 Windows API compatibility layer
// STRATEGY: Provide our own complete Windows types/functions FIRST.
// DXVK headers (d3d8.h) will include their own windows_base.h later.
// Our types take precedence via PCH (PreRTS.h includes windows_compat.h early).

#ifdef _WIN32
// GeneralsX @bugfix BenderAI 01/04/2026 Windows modern builds must use SDK windows.h, not linux compatibility shim.
// This header name shadows <windows.h> in project include paths; force SDK include explicitly.
#if defined(_MSC_VER)
	#if defined(__has_include)
		#if __has_include(<../um/windows.h>)
			#include <../um/windows.h>
		#else
			#include "windows_compat.h"
		#endif
	#else
		#include "windows_compat.h"
	#endif
#else
	#include "windows_compat.h"
#endif
#else
// Linux: Our compatibility layer only (NO DXVK headers here!)
// DXVK's windows_base.h will be included by d3d8.h when needed
#include "windows_compat.h"
#endif
