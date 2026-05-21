#pragma once

// GeneralsX @build fbraz 10/02/2026 Bender
// COM utility header wrapper - redirects to platform-specific implementation

#if defined(_WIN32) && defined(_MSC_VER)
// GeneralsX @bugfix Copilot 21/05/2026 Keep MinGW on the local COM compatibility stubs instead of the SDK COM utility header.
// Windows/MSVC: Use real COM utility header from Windows SDK.
#include_next <comutil.h>
#else
// MinGW and non-Windows: Use our compatibility layer.
#include "comutil_compat.h"
#endif
