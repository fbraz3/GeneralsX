/* SPDX-License-Identifier: MIT OR GPL-2.0-or-later */
#include "types_compat.h"
#include "time_compat.h"

#include <time.h>

// GeneralsX @feature BenderAI 24/02/2026 Phase 5 macOS timer support (CLOCK_BOOTTIME -> CLOCK_UPTIME)
#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

DWORD timeGetTime(void)
{
  // Boost could be used but is slow
#ifdef __APPLE__
  // macOS: Use mach_absolute_time() for uptime
  static mach_timebase_info_data_t tb = { 0, 0 };
  if (tb.denom == 0) {
    mach_timebase_info(&tb);
  }
  uint64_t elapsed = mach_absolute_time();
  uint64_t nanoseconds = elapsed * tb.numer / tb.denom;
  DWORD diff = (DWORD)(nanoseconds / 1000000);
#elif defined(__linux__)
  // Linux: CLOCK_BOOTTIME is Linux-specific (system uptime)
  struct timespec ts;
  clock_gettime(CLOCK_BOOTTIME, &ts);
  DWORD diff = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
  // Other POSIX: Use CLOCK_MONOTONIC (fallback for BSD, etc.)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  DWORD diff = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
  return diff;
}

DWORD GetTickCount(void)
{
  return timeGetTime();
}

void Sleep(DWORD ms)
{
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

void GetLocalTime(SYSTEMTIME* st)
{
	time_t t = time(NULL);
	struct tm* tm = localtime(&t);
	st->wYear = tm->tm_year + 1900;
	st->wMonth = tm->tm_mon + 1;
	st->wDayOfWeek = tm->tm_wday;
	st->wDay = tm->tm_mday;
	st->wHour = tm->tm_hour;
	st->wMinute = tm->tm_min;
	st->wSecond = tm->tm_sec;
	st->wMilliseconds = 0;
}