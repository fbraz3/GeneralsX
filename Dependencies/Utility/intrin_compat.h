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

/////////////////////////////////////////////////////////////////////////
// Intrinsics Compatibility Header - Windows/Unix
// Maps Windows-specific compiler intrinsics and types to POSIX equivalents
/////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>

#ifdef _WIN32
	// Windows: Use native MSVC intrinsics
	#include <intrin.h>
#else
	// Linux/Unix: Provide POSIX equivalents
	
	// Note: Do NOT typedef _int64/__int64 here - causes conflicts with type modifiers
	// Instead, individual headers (profile_funclevel.h, debug_debug.h) handle 64-bit types
	
	// Intrinsic function compatibility
	#define __forceinline inline
	
	// CPU cycle counter (ARM64 safe)
	#ifdef __aarch64__
		// ARM64: Use timer register via inline asm
		inline uint64_t _rdtsc(void)
		{
			uint64_t val;
			asm volatile ("mrs %0, cntvct_el0" : "=r" (val));
			return val;
		}
	#else
		// x86/x86_64: Use RDTSC instruction
		#ifdef __GNUC__
			#define _rdtsc() __builtin_ia32_rdtsc()
		#else
			inline uint64_t _rdtsc(void)
			{
				uint32_t lo, hi;
				asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
				return ((uint64_t)hi << 32) | lo;
			}
		#endif
	#endif
	
	// Math function compatibility (C++11 std::isnan vs Windows _isnan)
	#include <cmath>
	#define _isnan(x) std::isnan(x)
	#define _isfinite(x) std::isfinite(x)
	#define _isinf(x) std::isinf(x)
	
#endif // _WIN32
