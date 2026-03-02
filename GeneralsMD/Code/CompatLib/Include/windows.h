#pragma once

// Linux/Unix compatibility shim for <windows.h>
// GeneralsX @build BenderAI 11/02/2026 (updated 26/05/2026)
//
// PURPOSE: Intercepts #include <windows.h> on Linux so that POSIX compat types
// are loaded instead of a missing Windows SDK.
//
// ON WINDOWS: This file is intentionally a NO-OP.
//   The real Windows SDK windows.h is found via the INCLUDE environment variable
//   (set by build_win64_modern.ps1 / MSVC environment setup).
//   If this file is reached on Windows it means CompatLib/Include is on the
//   project include path BEFORE the SDK — fix the CMakeLists target_include_directories.
//
// ON LINUX: Include our compatibility layer.
//   DXVK's windows_base.h will be included by d3d8.h when needed.
//   Do NOT pull in DXVK headers here; order matters.
#ifndef _WIN32
#include "windows_compat.h"
#endif
