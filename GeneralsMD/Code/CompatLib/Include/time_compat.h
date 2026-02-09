#pragma once

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
static MMRESULT timeBeginPeriod(int) { return TIMERR_NOERROR; }
static MMRESULT timeEndPeriod(int) { return TIMERR_NOERROR; }

// Same as in windows_base.h of DXVK-Native
// Consider using that instead, but for now, we don't want this as a constant dependency
DWORD timeGetTime(void);
DWORD GetTickCount(void);

void Sleep(DWORD ms);

void GetLocalTime(SYSTEMTIME* st);