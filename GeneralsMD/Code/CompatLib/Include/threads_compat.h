/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// TheSuperHackers @build felipebraz 10/02/2026 Phase 1.5
// Cross-platform threading primitives (Win32 critical sections for Linux)

#pragma once

#ifndef THREADS_COMPAT_H
#define THREADS_COMPAT_H

#ifdef _UNIX

// POSIX threads support
#include <pthread.h>

// Emulate Windows CRITICAL_SECTION with pthread_mutex_t
// Based on fighter19's CritSec implementation
typedef struct _CRITICAL_SECTION_COMPAT {
    pthread_mutex_t mutex;
    pthread_t owner;     // Thread ID of current owner (for recursive locking)
    int ref_count;       // Recursion depth counter
} CRITICAL_SECTION;

// Windows-style critical section API wrappers
#ifdef __cplusplus
extern "C" {
#endif

// Initialize a critical section (maps to pthread_mutex_init)
inline void InitializeCriticalSection(CRITICAL_SECTION *cs) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    
    // Enable recursive locking (Windows CRITICAL_SECTION is recursive by default)
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    
    pthread_mutex_init(&cs->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    cs->owner = 0;
    cs->ref_count = 0;
}

// Destroy a critical section (maps to pthread_mutex_destroy)
inline void DeleteCriticalSection(CRITICAL_SECTION *cs) {
    pthread_mutex_destroy(&cs->mutex);
    cs->owner = 0;
    cs->ref_count = 0;
}

// Enter a critical section (blocking lock)
inline void EnterCriticalSection(CRITICAL_SECTION *cs) {
    pthread_mutex_lock(&cs->mutex);
    cs->owner = pthread_self();
    cs->ref_count++;
}

// Leave a critical section (unlock)
inline void LeaveCriticalSection(CRITICAL_SECTION *cs) {
    cs->ref_count--;
    
    if (cs->ref_count == 0) {
        cs->owner = 0;
        pthread_mutex_unlock(&cs->mutex);
    }
}

#ifdef __cplusplus
}
#endif

#endif // _UNIX

#endif // THREADS_COMPAT_H
