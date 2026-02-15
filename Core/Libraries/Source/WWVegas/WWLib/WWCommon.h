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

#pragma once

#include "refcount.h"
#include "STLUtils.h"
#include "stringex.h"
#include <Utility/stdio_adapter.h>

// TheSuperHackers @build 10/02/2026 Bender
// Linux port: Include platform compatibility headers BEFORE any DirectX/Windows headers are parsed
// This ensures Win32 types (DWORD, HANDLE, etc.) and COM types (IUnknown, GUID, etc.) are defined
#if !defined(_WIN32)
    // Define guard to tell old Utility/compat.h that new compat is already included
    #define DEPENDENCIES_UTILITY_COMPAT_H 1
    
    // Order matters: bittype.h defines basic types, types_compat adds Windows types, com_compat adds COM
    #include "bittype.h"           // Basic types: uint32, DWORD, BYTE, etc.
    #include "types_compat.h"      // Windows types: HANDLE, HWND, HMONITOR, RGNDATA, etc.
    #include "windows_compat.h"    // Windows API compatibility: GetDoubleClickTime, HIWORD, etc.
    #include "com_compat.h"        // COM/DirectX: IUnknown, GUID, REFIID, DECLARE_INTERFACE_, etc.
    
    // GeneralsX @build fbraz 10/02/2026 Bender
    // CRITICAL: Include time_compat.h and gdi_compat.h DIRECTLY here because windows_compat.h
    // skips them when DEPENDENCIES_UTILITY_COMPAT_H is defined (see windows_compat.h line 138)
    // ww3d.cpp needs MMRESULT/timeBeginPeriod (time_compat.h) and BITMAP structures (gdi_compat.h)
    #include "time_compat.h"       // MMRESULT, timeBeginPeriod, timeEndPeriod stubs
    #include "gdi_compat.h"        // BITMAPFILEHEADER, BITMAPINFOHEADER, BI_RGB
    #include "string_compat.h"     // lstrlen, lstrcpy, lstrcmpi mappings
    #include "file_compat.h"       // GetCurrentDirectory, GetFileAttributes stubs
    #include "vfw_compat.h"        // VFW/AVI API stubs (FramGrab.cpp movie capture - Phase 3 out-of-scope)
#endif


// This macro serves as a general way to determine the number of elements within an array.
#ifndef ARRAY_SIZE
#if defined(_MSC_VER) && _MSC_VER < 1300
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#else
template <typename Type, size_t Size> char (*ArraySizeHelper(Type(&)[Size]))[Size];
#define ARRAY_SIZE(arr) sizeof(*ArraySizeHelper(arr))
#endif
#endif // ARRAY_SIZE

enum
{
	// TheSuperHackers @info The original WWSync was 33 ms, ~30 fps, integer.
	// Changing this will require tweaking all Drawable code that concerns the ww3d time step, including locomotion physics.
	WWSyncPerSecond = 30,
	WWSyncMilliseconds = 1000 / WWSyncPerSecond,
};

#if defined(_MSC_VER) && _MSC_VER < 1300
typedef unsigned MemValueType;
typedef long Interlocked32; // To use with Interlocked functions
#else
typedef unsigned long long MemValueType;
typedef volatile long Interlocked32; // To use with Interlocked functions
#endif
