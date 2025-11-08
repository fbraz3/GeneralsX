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

#include	<windows.h>
//#include <mmsystem.h>
//#include	<windowsx.h>
//#include	<winnt.h>
//#include	<winuser.h>

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
//#include <unistd.h>	// file does not exist
#include "../WW3D2/win32_compat.h"
#endif // _WIN32

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

#include	<windows.h>
//#include <mmsystem.h>
//#include	<windowsx.h>
//#include	<winnt.h>
//#include	<winuser.h>

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
//#include <unistd.h>	// file does not exist
#endif // _WIN32
