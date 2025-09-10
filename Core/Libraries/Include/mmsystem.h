#pragma once
#ifndef _MMSYSTEM_H_
#define _MMSYSTEM_H_

// Multimedia system compatibility for macOS port
#ifdef __APPLE__

#include "windows.h"

// Multimedia types - check if already defined
#ifndef MMRESULT
typedef DWORD MMRESULT;
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

// Timer capabilities structure
typedef struct tagTIMECAPS {
    DWORD wPeriodMin;
    DWORD wPeriodMax;
} TIMECAPS, *LPTIMECAPS;

// Wave format structure
typedef struct tWAVEFORMATEX {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
} WAVEFORMATEX;

// Multimedia function stubs - check if already defined
#ifndef timeGetTime
inline DWORD timeGetTime() {
    return GetTickCount(); // Use our Windows compatibility function
}
#endif

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

// Wave functions stubs
inline DWORD waveOutGetNumDevs() {
    return 0; // No devices
}

inline MMRESULT waveOutGetDevCaps(DWORD_PTR uDeviceID, void* pwoc, DWORD cbwoc) {
    return MMSYSERR_BADDEVICEID; // Stub
}

#endif // __APPLE__
#endif // _MMSYSTEM_H_
