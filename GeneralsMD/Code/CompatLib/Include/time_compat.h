#pragma once

// TheSuperHackers @build fighter19 10/02/2026 Bender - Need DWORD type
#include "types_compat.h"
#include <stdint.h>

struct SYSTEMTIME
{
	uint16_t wYear;
	uint16_t wMonth;
	uint16_t wDayOfWeek;
	uint16_t wDay;
	uint16_t wHour;
	uint16_t wMinute;
	uint16_t wSecond;
	uint16_t wMilliseconds;
};

typedef int MMRESULT;

#define TIMERR_NOERROR 0

// Avoid conflict with old Dependencies/Utility/Utility/time_compat.h
// Old header defines inline implementations, we define declarations
// Only define if old header hasn't been included yet
#ifndef __TIME_COMPAT_OLD_INCLUDED
#ifndef timeBeginPeriod
static MMRESULT timeBeginPeriod(int) { return TIMERR_NOERROR; }
#endif
#ifndef timeEndPeriod
static MMRESULT timeEndPeriod(int) { return TIMERR_NOERROR; }
#endif
#ifndef timeGetTime
DWORD timeGetTime(void);
#endif
#ifndef GetTickCount
DWORD GetTickCount(void);
#endif
#endif

void Sleep(DWORD ms);

void GetLocalTime(SYSTEMTIME* st);

// TheSuperHackers @build BenderAI 13/02/2026 Add QueryPerformance stubs for cross-platform portability
// Windows API for high-resolution performance counters (used in W3DDisplay.cpp)
#ifdef _WIN32
// Windows has native implementations in kernel32.lib
extern "C" {
    BOOL __stdcall QueryPerformanceCounter(void* lpPerformanceCount);
    BOOL __stdcall QueryPerformanceFrequency(void* lpFrequency);
}
#else
// Linux: Stub implementation using clock_gettime(CLOCK_MONOTONIC)
#include <time.h>
inline BOOL QueryPerformanceCounter(void* lpPerformanceCount) {
    if (!lpPerformanceCount) return 0;
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    // Convert to 100-nanosecond intervals for Windows compatibility
    int64_t* pCount = (int64_t*)lpPerformanceCount;
    *pCount = ts.tv_sec * 10000000LL + ts.tv_nsec / 100;
    return 1;
}

inline BOOL QueryPerformanceFrequency(void* lpFrequency) {
    if (!lpFrequency) return 0;
    // CLOCK_MONOTONIC resolution is 1 nanosecond => frequency is 1 billion counts per second
    // But we report 10 million (for 100-nanosecond intervals) for Windows compatibility
    int64_t* pFreq = (int64_t*)lpFrequency;
    *pFreq = 10000000LL;  // 100-nanosecond intervals
    return 1;
}
#endif