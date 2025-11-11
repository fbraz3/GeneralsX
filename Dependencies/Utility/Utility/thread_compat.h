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

// This file contains thread related functions for compatibility with non-windows platforms.
#pragma once

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

// Return a stable integer thread id. On macOS use pthread_threadid_np to
// retrieve a uint64_t id. On other UNIX platforms fall back to casting
// pthread_self() to uintptr_t and truncating to int (best-effort).
inline int GetCurrentThreadId()
{
#if defined(__APPLE__)
    uint64_t tid = 0;
    pthread_threadid_np(NULL, &tid);
    return (int)(tid & 0xFFFFFFFF);
#else
    return (int)(uintptr_t)pthread_self();
#endif
}

inline void Sleep(int ms)
{
  usleep((useconds_t)ms * 1000);
}

