/*
** mmsystem.h - Windows Multimedia System stub
**
** TheSuperHackers @build 15/12/2024
** Minimal stub for Linux builds. Download.cpp includes this on Windows only.
** For Linux builds with OpenAL, this provides empty type definitions.
*/

#pragma once

#if defined(SAGE_USE_OPENAL) && !defined(_WIN32)
    #ifndef _MMSYSTEM_H_
    #define _MMSYSTEM_H_

    // Empty stub - multimedia functions are not used in Linux builds
    // Download.cpp only uses this header preparation, actual multimedia
    // operations are stubbed or not called in Linux builds.

    #endif
#endif
