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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// CriticalSection.h ///////////////////////////////////////////////////////
// Utility class to use critical sections in areas of code.
// Author: JohnM And MattC, August 13, 2002

#pragma once

#include "Common/PerfTimer.h"

// Phase 39.5 Week 2: SDL2 Threading Unification
// Unified critical section implementation using SDL2 cross-platform layer
#include "WWVegas/WW3D2/win32_thread_compat.h"

#ifdef PERF_TIMERS
extern PerfGather TheCritSecPerfGather;
#endif

class CriticalSection
{
	SDL2_CriticalSection m_criticalSection;

	public:
		CriticalSection()
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			// Phase 39.5: Use unified SDL2 critical section
			m_criticalSection = SDL2_CreateCriticalSection();
		}

		virtual ~CriticalSection()
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			// Phase 39.5: Destroy unified SDL2 critical section
			SDL2_DestroyCriticalSection(&m_criticalSection);
		}

	public:	// Use these when entering/exiting a critical section.
		void enter( void )
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			// Phase 39.5: Use unified SDL2 critical section lock
			SDL2_EnterCriticalSection(&m_criticalSection);
		}

		void exit( void )
		{
			#ifdef PERF_TIMERS
			AutoPerfGather a(TheCritSecPerfGather);
			#endif
			// Phase 39.5: Use unified SDL2 critical section unlock
			SDL2_LeaveCriticalSection(&m_criticalSection);
		}
};

class ScopedCriticalSection
{
	private:
		CriticalSection *m_cs;

	public:
		ScopedCriticalSection( CriticalSection *cs ) : m_cs(cs)
		{
			if (m_cs) {
				// Phase 39.5: Use unified SDL2 critical section via enter()
				m_cs->enter();
			}
		}

		virtual ~ScopedCriticalSection( )
		{
			if (m_cs) {
				// Phase 39.5: Automatic unlock via exit()
				m_cs->exit();
			}
		}

	private:
		// Phase 39.5: Make ScopedCriticalSection non-copyable
		ScopedCriticalSection(const ScopedCriticalSection&) = delete;
		ScopedCriticalSection& operator=(const ScopedCriticalSection&) = delete;
};

#include "mutex.h"

// These should be NULL on creation then non-NULL in WinMain or equivalent.
// This allows us to be silently non-threadsafe for WB and other single-threaded apps.
extern CriticalSection *TheAsciiStringCriticalSection;
extern CriticalSection *TheUnicodeStringCriticalSection;
extern CriticalSection *TheDmaCriticalSection;
extern CriticalSection *TheMemoryPoolCriticalSection;
extern CriticalSection *TheDebugLogCriticalSection;
