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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/framgrab.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"

#if defined (_MSC_VER)
#pragma warning (push, 3)	// (gth) system headers complain at warning level 4...
#endif

// GeneralsX @refactor BenderAI 10/02/2026 - guard Windows-only headers for Linux compatibility
#ifdef _WIN32
#ifndef _WINDOWS_
#include "windows.h"
#endif

#ifndef _INC_WINDOWSX
#include "windowsx.h"
#endif

// GeneralsX @build BenderAI 28/02/2026
// FRAME_GRABBER feature requires VFW (Video for Windows)
// VFW headers have missing DRV_USER constants in Windows SDK 10.0.26100.0
// Phase 3 out-of-scope feature - disable for Phase 1 graphics focus
#if defined(ENABLE_FRAME_GRABBER) && !defined(DISABLE_VFW)
  #ifndef _INC_VFW
  #include "vfw.h"
  #endif
#else
  // Frame grabber disabled (out-of-scope for Phase 1)
  // Provide minimal typedefs for FramGrab class structure
  typedef void* PAVIFILE;
  typedef void* PAVISTREAM;
  typedef struct { DWORD cbSize; } AVISTREAMINFO;
#endif
#else
// GeneralsX @refactor BenderAI 10/02/2026 - fallback for Linux (stub types)
#include "windows_compat.h"
// Linux stubs for AVI types
typedef void* PAVIFILE;
typedef void* PAVISTREAM;
typedef struct { DWORD cbSize; } AVISTREAMINFO;
#endif // _WIN32

#if defined (_MSC_VER)
#pragma warning (pop)
#endif

// FramGrab.h: interface for the FrameGrabClass class.
//
//////////////////////////////////////////////////////////////////////

class FrameGrabClass
{
public:
	enum MODE {
		RAW,
		AVI
	};

	// depending on which mode you select, it will produce either frames or an AVI.
	FrameGrabClass(const char *filename, MODE mode, int width, int height, int bitdepth, float framerate );

	virtual ~FrameGrabClass();

	void ConvertGrab(void *BitmapPointer);
	void Grab(void *BitmapPointer);

	// GeneralsX @refactor BenderAI 10/02/2026 - conditional GetBuffer for Linux stub
#ifdef _WIN32
	long * GetBuffer()			{ return Bitmap; }
#else
	void * GetBuffer()			{ return NULL; }
#endif
	float	GetFrameRate()			{ return FrameRate; }

protected:
	const char *Filename;
	float			FrameRate;

	MODE Mode;
	long Counter; // used for incrementing filename cunter, etc.

	void GrabAVI(void *BitmapPointer);
	void GrabRawFrame(void *BitmapPointer);

	// GeneralsX @refactor BenderAI 10/02/2026 - guard Windows AVI members for Linux compatibility
#ifdef _WIN32
	// avi settings
	PAVIFILE				AVIFile;
	long					*Bitmap;
	PAVISTREAM			Stream;
	AVISTREAMINFO		AVIStreamInfo;
	BITMAPINFOHEADER	BitmapInfoHeader;
#endif

	// general purpose cleanup routine
	void CleanupAVI();

	// convert the SR image into AVI byte ordering
	void ConvertFrame(void *BitmapPointer);

};
