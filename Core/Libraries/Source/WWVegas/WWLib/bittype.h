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

/* $Header: /G/wwlib/bittype.h 4     4/02/99 1:37p Eric_c $ */
/***************************************************************************
 ***                  Confidential - Westwood Studios                    ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Voxel Technology                         *
 *                                                                         *
 *                    File Name : BITTYPE.h                                *
 *                                                                         *
 *                   Programmer : Greg Hjelstrom                           *
 *                                                                         *
 *                   Start Date : 02/24/97                                 *
 *                                                                         *
 *                  Last Update : February 24, 1997 [GH]                   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <stdint.h>

// TheSuperHackers @build 10/02/2026 Bender
// Basic integer types: match platform word sizes correctly
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef signed char		sint8;
typedef signed short	sint16;

// TheSuperHackers @build 10/02/2026 BenderAI  
// uint32/sint32: Use fixed-width types on 64-bit platforms (long is 64-bit on Linux/macOS x64)
#if defined(__linux__) || defined(__APPLE__)
    typedef uint32_t uint32;
    typedef int32_t  sint32;
#else
    // Windows: long is always 32-bit
    typedef unsigned long uint32;
    typedef signed long   sint32;
#endif

typedef unsigned int    uint;
typedef signed int      sint;

// TheSuperHackers @build 10/02/2026 BenderAI + @bugfix fbraz 03/02/2026
// 64-bit integer types: On Linux, these are provided by debug_debug.h/profile_funclevel.h to avoid conflicts
// Only define on MSVC where they're native compiler extensions
#ifdef _MSC_VER
    // MSVC provides __int64 natively, just define unsigned variants
    typedef unsigned __int64 __uint64;
    typedef unsigned __int64 _uint64;
#endif
// Note: Linux GCC/Clang use int64_t typedefs from debug_debug.h or profile_f unclevel.h

typedef float				float32;
typedef double				float64;

// TheSuperHackers @build 09/02/2025 Bender
// Win32 types: use uint32_t on Linux 64-bit to match DXVK, unsigned long on Windows 32-bit
#if defined(__linux__) || defined(__APPLE__)
    // Linux/macOS: long is 64-bit on x86_64, use uint32_t to match Win32 behavior
    typedef uint32_t DWORD;
    typedef uint32_t ULONG;
#else
    // Windows: long is always 32-bit
    typedef unsigned long DWORD;
    typedef unsigned long ULONG;
#endif

typedef unsigned short	WORD;
typedef unsigned char   BYTE;
typedef int             BOOL;
typedef unsigned short	USHORT;
typedef const char *		LPCSTR;
typedef int             INT;
typedef unsigned int    UINT;

#if defined(_MSC_VER) && _MSC_VER < 1300
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif
