/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// This file contains the time functions for compatibility with non-windows platforms.
#pragma once
#include <time.h>
#include <stdint.h>

#define TIMERR_NOERROR 0
typedef int MMRESULT;
static inline MMRESULT timeBeginPeriod(int) { return TIMERR_NOERROR; }
static inline MMRESULT timeEndPeriod(int) { return TIMERR_NOERROR; }

// Convert timespec to milliseconds (64-bit intermediate to avoid overflow)
static inline long long timespec_to_ms(const struct timespec &ts) {
    return (long long)ts.tv_sec * 1000LL + (ts.tv_nsec / 1000000LL);
}

inline unsigned int timeGetTime()
{
  struct timespec ts;
#if defined(CLOCK_BOOTTIME)
  clock_gettime(CLOCK_BOOTTIME, &ts);
#elif defined(CLOCK_MONOTONIC)
  clock_gettime(CLOCK_MONOTONIC, &ts);
#else
  clock_gettime(CLOCK_REALTIME, &ts);
#endif
  return (unsigned int)timespec_to_ms(ts);
}

inline unsigned int GetTickCount()
{
  struct timespec ts;
#if defined(CLOCK_MONOTONIC)
  clock_gettime(CLOCK_MONOTONIC, &ts);
#else
  clock_gettime(CLOCK_REALTIME, &ts);
#endif
  // Return ms since monotonic base
  return (unsigned int)timespec_to_ms(ts);
}

