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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/win.h                                  $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/16/01 2:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*
**	This header file includes the Windows headers. If there are any special pragmas that need
**	to occur around this process, they are performed here. Typically, certain warnings will need
**	to be disabled since the Windows headers are repleat with illegal and dangerous constructs.
**
**	Within the windows headers themselves, Microsoft has disabled the warnings 4290, 4514,
**	4069, 4200, 4237, 4103, 4001, 4035, 4164. Makes you wonder, eh?
*/

#ifdef _WIN32

// When including windows, lets just bump the warning level back to 3...
#if (_MSC_VER >= 1200)
#pragma warning(push, 3)
#endif

// this define should also be in the DSP just in case someone includes windows stuff directly
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// GeneralsX @build BenderAI 28/02/2026
// Request multimedia components BEFORE windows.h when WIN32_LEAN_AND_MEAN is active
// These defines tell windows.h to include multimedia APIs even with LEAN_AND_MEAN
#ifndef NOMCI
#define NOMCI
#endif
#ifndef NOFIO
#define NOFIO
#endif

#include	<windows.h>
#include <timeapi.h>  // GeneralsX @build BenderAI 28/02/2026 - Modern location for timeBeginPeriod, timeEndPeriod
#include <mmsystem.h>  // GeneralsX @build BenderAI 10/02/2026 - Required for timeBeginPeriod, timeEndPeriod, MMRESULT
//#include	<windowsx.h>
//#include	<winnt.h>
//#include	<winuser.h>

// GeneralsX @build BenderAI 28/02/2026
// WINBOOL compatibility for Wine/DXVK native headers (e.g. d3d8types.h from DXVK include/native/directx)
// Wine uses WINBOOL as an alias for BOOL (int). MSVC/Windows SDK does not define WINBOOL.
// Define it here so any Wine-sourced header included under WIN32 compiles cleanly with MSVC.
#ifndef WINBOOL
#define WINBOOL BOOL
#endif

#if (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

extern HINSTANCE	ProgramInstance;
extern HWND			MainWindow;
extern bool GameInFocus;

#ifdef RTS_DEBUG

void __cdecl Print_Win32Error(unsigned long win32Error);

#else // RTS_DEBUG

#define Print_Win32Error

#endif // RTS_DEBUG

#else // _WIN32
// GeneralsX @build fbraz 03/02/2026 Include CompatLib headers for Linux Windows API compatibility
#include <windows.h>  // CompatLib wrapper that includes windows_base.h (DXVK) + windows_compat.h (stubs)

// GeneralsX @build fbraz 10/02/2026 Bender
// Explicit fallback mappings (belt & suspenders approach for WW3D2 compatibility)
#ifndef lstrlen
#define lstrlen strlen
#endif
#ifndef lstrcpy
#define lstrcpy strcpy
#endif
#ifndef lstrcat
#define lstrcat strcat
#endif
#endif // _WIN32
