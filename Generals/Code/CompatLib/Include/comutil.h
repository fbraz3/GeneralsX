#pragma once

// GeneralsX @build fbraz 10/02/2026 Bender
// COM utility header wrapper - redirects to platform-specific implementation

#ifdef _WIN32
// Windows: Use real COM utility header from Windows SDK
#include_next <comutil.h>
#else
// Linux: Use our compatibility layer
#include "comutil_compat.h"
#endif
