#pragma once

// Linux/Unix compatibility shim for <windows.h>
// GeneralsX @build BenderAI 11/02/2026 (updated 26/05/2026)
// See windows.h for full explanation of this shim's purpose.
// ON WINDOWS: NO-OP. Real SDK provides all types.
// ON LINUX:   Includes our POSIX compatibility layer.
#ifndef _WIN32
#include "windows_compat.h"
#endif
