#pragma once

#ifndef _WIN32

// DirectX 8 types compatibility header for non-Windows systems
#include "d3d8.h"

// Additional DirectX types that might be needed - only define if not already defined

#else
// On Windows, include the real DirectX headers
#include <d3d8types.h>
#endif // !_WIN32
