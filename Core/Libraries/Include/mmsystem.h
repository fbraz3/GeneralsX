#pragma once
#ifndef _MMSYSTEM_H_
#define _MMSYSTEM_H_

// Multimedia system compatibility for macOS port
#ifdef __APPLE__

#ifdef _WIN32
#include "windows.h"
#else
#include "../Source/WWVegas/WW3D2/win32_compat.h"
#endif

// Include time compatibility first to avoid conflicts
#include "../../../Dependencies/Utility/./Utility/time_compat.h"

// Multimedia types - check if already defined
#ifndef MMRESULT
// Use same type as time_compat.h to avoid conflicts
typedef int MMRESULT;
#endif
#ifndef DWORD_PTR
typedef DWORD DWORD_PTR;
#endif
#ifndef HMIDI
typedef void* HMIDI;
#endif
#ifndef HWAVEOUT
typedef void* HWAVEOUT;
#endif
#ifndef HWAVEIN
typedef void* HWAVEIN;
#endif

// Multimedia constants
#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR 1
#define MMSYSERR_BADDEVICEID 2
#define MMSYSERR_NOTENABLED 3
#define MMSYSERR_ALLOCATED 4
#define MMSYSERR_INVALHANDLE 5
#define MMSYSERR_NODRIVER 6
#define MMSYSERR_NOMEM 7

// Time functions
#define TIME_MS 0x0001
#define TIME_SAMPLES 0x0002
#define TIME_BYTES 0x0004

// Wave format
#define WAVE_FORMAT_PCM 1

// Timer capabilities structure - skip to avoid conflicts with win32_compat.h
/*
typedef struct tagTIMECAPS {
    DWORD wPeriodMin;
    DWORD wPeriodMax;
} TIMECAPS, *LPTIMECAPS;
*/

// Timer callback function type
typedef void (CALLBACK *LPTIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

// Timer constants
#define TIME_ONESHOT    0x0000
#define TIME_PERIODIC   0x0001
#define TIME_CALLBACK_FUNCTION 0x0000
#define TIMERR_NOERROR  0
#define TIMERR_NOCANDO  97

// Wave format structure - skip to avoid conflicts with win32_compat.h
/*
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX;
*/

// Multimedia function stubs - check if already defined
// timeGetTime is already defined in time_compat.h, so skip it here

#ifndef timeGetDevCaps
inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, DWORD cbtc) {
    if (ptc && cbtc >= sizeof(TIMECAPS)) {
        ptc->wPeriodMin = 1;
        ptc->wPeriodMax = 1000;
        return MMSYSERR_NOERROR;
    }
    return MMSYSERR_ERROR;
}
#endif

#ifndef timeBeginPeriod
inline MMRESULT timeBeginPeriod(DWORD uPeriod) {
    return MMSYSERR_NOERROR; // Stub
}
#endif

#ifndef timeEndPeriod
inline MMRESULT timeEndPeriod(DWORD uPeriod) {
    return MMSYSERR_NOERROR; // Stub
}
#endif

// Enhanced multimedia timer functions - skip to avoid conflicts with win32_compat.h
/*
#ifndef timeSetEvent
inline MMRESULT timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD_PTR dwUser, UINT fuEvent) {
    // Use our compatibility layer implementation
    return ::timeSetEvent(uDelay, uResolution, fptc, dwUser, fuEvent);
}
#endif

#ifndef timeKillEvent
inline MMRESULT timeKillEvent(UINT uTimerID) {
    // Use our compatibility layer implementation
    return ::timeKillEvent(uTimerID);
}
#endif
*/

// Wave functions stubs - skip to avoid conflicts with win32_compat.h
/*
inline DWORD waveOutGetNumDevs() {
    return 0; // No devices
}
*/

inline MMRESULT waveOutGetDevCaps(DWORD_PTR uDeviceID, void* pwoc, DWORD cbwoc) {
    return MMSYSERR_BADDEVICEID; // Stub
}

#endif // __APPLE__
#endif // _MMSYSTEM_H_
