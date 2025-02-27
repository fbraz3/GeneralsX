/*
**	Command & Conquer Generals(tm)
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

/*****************************************************************************
**                                                                          **
**                       Westwood Studios Pacific.                          **
**                                                                          **
**                       Confidential Information					                  **
**                Copyright (C) 2000 - All Rights Reserved                  **
**                                                                          **
******************************************************************************
**                                                                          **
** Project:  Dune Emperor                                                   **
**                                                                          **
** Module:  <module> (<prefix>_)                                            **
**                                                                          **
** Version:  $ID$                                                           **
**                                                                          **
** File name:  audassrt.cpp                                                 **
**                                                                          **
** Created by:  12/??/00 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <assert.h>
//#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include <wpaudio/altypes.h>


/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/

extern void	WindowsDebugPrint( const char * lpOutputString );

/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/



/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/



/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/

char					*DBG_type_struct_is_dead = "Invalid structure";
char					assert_msg_buf[10*1024];
static FILE		*err_file = NULL;
static int		total_errors;
static char		_msg_buf[sizeof(assert_msg_buf)*2];

/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/



/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/

#ifdef _DEBUG
/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void __cdecl _assert_printf ( const char *format, ...)
{

   va_list args;
   va_start( args, format );     /* Initialize variable arguments. */

   vsprintf ( assert_msg_buf, format, args );
   va_end( args );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void __cdecl _aud_debug_printf ( const char *format, ...)
{
   va_list args;
   va_start( args, format );     /* Initialize variable arguments. */

   vsprintf ( _msg_buf, format, args );
   va_end( args );

	WindowsDebugPrint ( _msg_buf );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void _aud_assert ( const char *, const char *file, int line, const char *reason )
{
	sprintf ( _msg_buf, "%s(%d) : Error : ASSERT - %s\n", file, line, reason );
	WindowsDebugPrint ( _msg_buf );
	_assert ( _msg_buf, (void *) file, line );
}

#endif

/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

