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

// This file contains all the header files that shouldn't change frequently.
// Be careful what you stick in here, because putting files that change often in here will
// tend to cheese people's goats.

#pragma once

//-----------------------------------------------------------------------------
// srj sez: this must come first, first, first.
#ifdef _WIN32
#define _STLP_USE_NEWALLOC					1
//#define _STLP_USE_CUSTOM_NEWALLOC		STLSpecialAlloc
class STLSpecialAlloc;


// We actually don't use Windows for much other than timeGetTime, but it was included in 40
// different .cpp files, so I bit the bullet and included it here.
// PLEASE DO NOT ABUSE WINDOWS OR IT WILL BE REMOVED ENTIRELY. :-)
//--------------------------------------------------------------------------------- System Includes
#define WIN32_LEAN_AND_MEAN
#include <atlbase.h>
#include <windows.h>

#include <assert.h>
#include <ctype.h>
#include <direct.h>
#include <excpt.h>
#include <float.h>
#include <Utility/fstream_adapter.h>
#include <imagehlp.h>
#include <io.h>
#include <limits.h>
#include <lmcons.h>
#if defined(_MSC_VER) && _MSC_VER < 1300
#include <mapicode.h>
#endif
#include <math.h>
#include <memory.h>
#include <mmsystem.h>
#include <objbase.h>
#include <ocidl.h>
#include <process.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>
#include <snmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <tchar.h>
#include <time.h>
#include <vfw.h>
#include <winerror.h>
#include <wininet.h>
#include <winreg.h>

#ifndef DIRECTINPUT_VERSION
#	define DIRECTINPUT_VERSION	0x800
#endif

#ifdef _WIN32
#if defined(_WIN32)
#if 0 // Don't include platform-specific DirectInput headers on non-Windows platforms
#endif
#endif
#endif

//------------------------------------------------------------------------------------ STL Includes
// srj sez: no, include STLTypesdefs below, instead, thanks
//#include <algorithm>
//#include <bitset>
//#include <hash_map>
//#include <list>
//#include <map>
//#include <queue>
//#include <set>
//#include <stack>
//#include <string>
//#include <vector>

//------------------------------------------------------------------------------------ RTS Includes
// Icky. These have to be in this order.
#include "Lib/BaseType.h"
#include "Common/STLTypedefs.h"
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/AsciiString.h"
#include "Common/SubsystemInterface.h"

#include "Common/GameCommon.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"

// You might not want Kindof in here because it seems like it changes frequently, but the problem
// is that Kindof is included EVERYWHERE, so it might as well be precompiled.
#include "Common/INI.h"
#include "Common/KindOf.h"
#include "Common/DisabledTypes.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/LogicRandomValue.h"
#include "Common/ObjectStatusTypes.h"

#include "Common/Thing.h"
#include "Common/UnicodeString.h"

#else // !_WIN32
// Non-Windows platform includes
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstddef>

// Windows compatibility layer for non-Windows platforms
// Include via core library path (set up by CMake include directories)
#include <win32_compat.h>

// Ensure D3D8 Vulkan compatibility definitions (device stubs, buffer interfaces,
// and common D3D flags) are available in the precompiled header for Phase 50
// (Vulkan-only) builds on non-Windows platforms.
#include "WW3D2/d3d8_vulkan_graphics_compat.h"

// Do not include platform-specific DirectInput headers on non-Windows builds.
// They can pull in Windows typedefs (like LARGE_INTEGER) that conflict
// with our canonical compatibility definitions. If DirectInput APIs are
// needed they should be wrapped in a Windows-only source file.
// #include <dinput.h>

#include "Lib/BaseType.h"

// Ensure basic MessageBox macros and prototypes are available in the PCH
// for non-Windows builds. These are minimal, reversible fallbacks that
// avoid surprising undeclared identifier errors in many translation units.
#ifndef MB_OK
#define MB_OK            0x00000000L
#endif
#ifndef MB_ICONERROR
#define MB_ICONERROR     0x00000010L
#endif
#ifndef MB_ICONINFORMATION
#define MB_ICONINFORMATION 0x00000040L
#endif
#ifndef MB_SYSTEMMODAL
#define MB_SYSTEMMODAL   0x00001000L
#endif
#ifndef MB_TASKMODAL
#define MB_TASKMODAL     0x00002000L
#endif
#ifndef MB_APPLMODAL
#define MB_APPLMODAL     MB_SYSTEMMODAL
#endif
#ifndef HWND_NOTOPMOST
#define HWND_NOTOPMOST ((HWND)-2)
#endif

// Minimal MessageBox prototypes -- compatibility stubs are provided by
// the umbrella compat header, but ensure declarations exist for all TUs.
#ifndef MESSAGEBOX_PROTOTYPES_INCLUDED
#define MESSAGEBOX_PROTOTYPES_INCLUDED
extern int MessageBoxA(HWND hWnd, const char* lpText, const char* lpCaption, unsigned int uType);
extern int MessageBoxW(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, unsigned int uType);
#endif

// Some legacy headers define `max`/`min` as macros which break uses of
// `std::max`/`std::min` in portable code. Undefine them here after
// including the project's base types so the standard functions work.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include "Common/STLTypedefs.h"
#include "Common/Errors.h"
#include "Common/Debug.h"
#include "Common/AsciiString.h"
#include "Common/SubsystemInterface.h"
#include "Common/GameCommon.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "Common/INI.h"
#include "Common/KindOf.h"
#include "Common/DisabledTypes.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/ClientRandomValue.h"
#include "GameLogic/LogicRandomValue.h"
#include "Common/ObjectStatusTypes.h"
#include "Common/Thing.h"
#include "Common/UnicodeString.h"

#endif // _WIN32

