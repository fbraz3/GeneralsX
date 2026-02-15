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

// This file contains string macros and alias functions to help compiling on non-windows platforms
#pragma once
#include <ctype.h>
#include <string.h>
#include <wchar.h>

typedef const char* LPCSTR;
typedef char* LPSTR;

// String functions
// GeneralsX @build BenderAI 11/02/2026 - Use extern "C" linkage to match GameSpy SDK declaration
#ifndef __GSPLATFORM_H__
#ifdef __cplusplus
extern "C" {
#endif
inline char *_strlwr(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = tolower(str[i]);
  }
  return str;
}
#ifdef __cplusplus
}
#endif
#endif

#define strlwr _strlwr
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp

// Wide character string mapping
#define wcscmp wcscmp
#define wcslen wcslen

