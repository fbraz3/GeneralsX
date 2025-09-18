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

// FILE: ScopedMutex.h ////////////////////////////////////////////////////////////////////////////
// Author: John McDonald, November 2002
// Desc:   A scoped mutex class to easily lock a scope with a pre-existing mutex object.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SCOPEDMUTEX_H__
#define __SCOPEDMUTEX_H__

#ifdef _WIN32
#include <windows.h>
#else
// Cross-platform mutex compatibility
#include <pthread.h>
typedef pthread_mutex_t* MUTEX_HANDLE;
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#define WAIT_OBJECT_0 0

inline DWORD WaitForSingleObject(MUTEX_HANDLE handle, DWORD timeout) {
    // Simple pthread mutex lock (ignoring timeout for simplicity)
    if (pthread_mutex_lock(handle) == 0) {
        return WAIT_OBJECT_0;
    }
    return 1; // Error
}

inline void ReleaseMutex(MUTEX_HANDLE handle) {
    pthread_mutex_unlock(handle);
}
#endif

class ScopedMutex
{
	private:
#ifdef _WIN32
		HANDLE m_mutex;
#else
		pthread_mutex_t* m_mutex;
#endif

	public:
#ifdef _WIN32
		ScopedMutex(HANDLE mutex) : m_mutex(mutex)
#else
		ScopedMutex(HANDLE mutex) : m_mutex((pthread_mutex_t*)mutex)
#endif
		{
#ifdef _WIN32
			DWORD status = WaitForSingleObject(m_mutex, 500);
			if (status != WAIT_OBJECT_0) {
				DEBUG_LOG(("ScopedMutex WaitForSingleObject timed out - status %d", status));
			}
#else
			if (m_mutex && pthread_mutex_lock(m_mutex) != 0) {
				DEBUG_LOG(("ScopedMutex pthread_mutex_lock failed"));
			}
#endif
		}

		~ScopedMutex()
		{
#ifdef _WIN32
			ReleaseMutex(m_mutex);
#else
			if (m_mutex) {
				pthread_mutex_unlock(m_mutex);
			}
#endif
		}
};

#endif /* __SCOPEDMUTEX_H__ */
