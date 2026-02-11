#pragma once

// TheSuperHackers @build fbraz 11/02/2026 Bender
// COM Interface Pointer header wrapper - redirects to platform-specific implementation
// Used by dx8webbrowser.cpp (DirectX web browser embedding)

#ifdef _WIN32
// Windows: Use real COM IP header from Windows SDK
#include_next <comip.h>
#else
// Linux: Use our compatibility layer (minimal stubs for compilation)
#include "comip_compat.h"
#endif
