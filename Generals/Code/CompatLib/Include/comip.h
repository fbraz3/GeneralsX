#pragma once

// GeneralsX @build fbraz 11/02/2026 Bender
// COM Interface Pointer header wrapper - redirects to platform-specific implementation
// Used by dx8webbrowser.cpp (DirectX web browser embedding)

#if defined(_WIN32) && defined(_MSC_VER)
// GeneralsX @bugfix Copilot 21/05/2026 Keep MinGW on the local COM compatibility stubs instead of the SDK COM pointer header.
// Windows/MSVC: Use real COM IP header from Windows SDK.
#include_next <comip.h>
#else
// MinGW and non-Windows: Use our compatibility layer (minimal stubs for compilation).
#include "comip_compat.h"
#endif
