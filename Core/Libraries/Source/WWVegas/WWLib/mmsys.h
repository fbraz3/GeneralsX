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
 *                     $Archive:: /Commando/Code/wwlib/mmsys.h                                $*
 *                                                                                             *
 *                      $Author:: Tom_s                                                       $*
 *                                                                                             *
 *                     $Modtime:: 6/12/00 2:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

/*
** This header just includes mmsystem.h with warning 4201 disabled.
** Also includes timeapi.h directly: Windows SDK 10.0.26100+ moved timeGetTime()
** to timeapi.h which is only conditionally included from mmsystem.h under
** #ifndef MMNOTIMER. Explicitly including it ensures timeGetTime is always available.
** GeneralsX @build Felipe 27/02/2026 Fix timeGetTime not found on Windows SDK 26100+
*/

#pragma warning(disable:4201)
#include <mmsystem.h>
#pragma warning(default:4201)

#ifdef _WIN32
#include <timeapi.h>  // timeGetTime() lives here in Windows SDK 10.0.26100.0+
#endif
