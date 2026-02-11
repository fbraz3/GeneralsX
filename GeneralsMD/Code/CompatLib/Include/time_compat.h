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