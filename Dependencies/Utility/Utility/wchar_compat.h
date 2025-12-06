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

// This file contains WCHAR and related macros for compatibility with non-windows platforms.
#pragma once

#include <cwchar>
#include <cwctype>
#include <cstring>

// WCHAR
typedef wchar_t WCHAR;
typedef const WCHAR* LPCWSTR;
typedef WCHAR* LPWSTR;

// Wide character case-insensitive comparison
// On Windows: _wcsicmp, On Unix/macOS: wcscasecmp
#ifdef _WIN32
	#define _wcsicmp _wcsicmp
	#define wcsicmp _wcsicmp
	#define _wcsnicmp _wcsnicmp
#else
	// macOS and Linux use wcscasecmp instead of wcscasecmp
	// Implement a wrapper for _wcsicmp/wcsicmp
	inline int wcscasecmp_wrapper(const wchar_t* s1, const wchar_t* s2) {
		while (*s1 && *s2) {
			wchar_t c1 = std::towlower(*s1);
			wchar_t c2 = std::towlower(*s2);
			if (c1 != c2) return c1 - c2;
			s1++;
			s2++;
		}
		return std::towlower(*s1) - std::towlower(*s2);
	}
	
	inline int wcsnicmp_wrapper(const wchar_t* s1, const wchar_t* s2, size_t n) {
		for (size_t i = 0; i < n; i++) {
			if (!s1[i] || !s2[i]) {
				return (s1[i] != 0) - (s2[i] != 0);
			}
			wchar_t c1 = std::towlower(s1[i]);
			wchar_t c2 = std::towlower(s2[i]);
			if (c1 != c2) return c1 - c2;
		}
		return 0;
	}
	
	#define _wcsicmp wcscasecmp_wrapper
	#define wcsicmp wcscasecmp_wrapper
	#define _wcsnicmp wcsnicmp_wrapper
	#define wcscmp wcscmp  // Already exists in cwchar
#endif

// MultiByteToWideChar
#define CP_ACP 0
#define MultiByteToWideChar(cp, flags, mbstr, cb, wcstr, cch) mbstowcs(wcstr, mbstr, cch)
#define WideCharToMultiByte(cp, flags, wcstr, cch, mbstr, cb, defchar, used) wcstombs(mbstr, wcstr, cb)

