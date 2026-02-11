/*
** d3d8.h - DirectX 8 stub redirect
**
** TheSuperHackers @build 15/12/2024
** Compatibility wrapper for Linux builds using DXVK + OpenAL.
** Redirects to D3D8Stub.h which provides minimal type definitions.
*/

#pragma once

#if defined(SAGE_USE_OPENAL) && !defined(_WIN32)
    // Linux: Use local stub directly
    #include "D3D8Stub.h"
#else
    // Windows: Use real DirectX8 SDK
    #ifndef __MINGW32__
        #error "d3d8.h: Use min-dx8-sdk on Windows or DXVK on Linux"
    #endif
#endif
