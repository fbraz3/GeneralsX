#pragma once

// GeneralsX @build fbraz 10/02/2026 Bender
// COM utility header wrapper - redirects to platform-specific implementation

#ifdef _WIN32
// GeneralsX @bugfix BenderAI 01/04/2026 MSVC does not support include_next in this shim.
// Use comdef + compatibility helpers to provide _bstr_t/_variant_t and _com_util helpers.
#if defined(_MSC_VER)
#include <comdef.h>
#include "Utility/comsupp_compat.h"
#else
#include_next <comutil.h>
#endif
#else
// Linux: Use our compatibility layer
#include "comutil_compat.h"
#endif
