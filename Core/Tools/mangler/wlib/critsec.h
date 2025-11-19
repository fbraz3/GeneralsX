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

#pragma once

#include "wstypes.h"

// Windows headers have a tendency to redefine IN
#ifdef IN
#undef IN
#endif
#define IN const

//
// Critical Section built either on a POSIX Mutex, or a Win32 Critical Section
//
// POSIX version is done by keeping a thread_id and a reference count.  Win32 version
//  just calls the built in functions.
//
class CritSec
{
 public:
                CritSec();
               ~CritSec();

  sint32		lock(int *refcount=NULL) RO;
  sint32		unlock(void) RO;

 protected:
    mutable pthread_mutex_t	Mutex_;         // Mutex lock
    mutable pthread_t		ThreadId_;	// Owner of mutex
    mutable int                 RefCount_;      // Reference count
};
