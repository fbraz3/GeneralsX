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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/monodrvr.h                                         $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifdef _MSC_VER
#pragma warning (push,3)
#endif

#ifdef _WINDOWS
#include	<winioctl.h>
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif


/*
**	This is the identifier for the Monochrome Display Driver.
*/
#define FILE_DEVICE_MONO  0x00008000

/*
**	These are the IOCTL commands supported by the Monochrome Display Driver.
*/
#define IOCTL_MONO_HELP_SCREEN	CTL_CODE(FILE_DEVICE_MONO, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_CLEAR_SCREEN	CTL_CODE(FILE_DEVICE_MONO, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_PRINT_RAW		CTL_CODE(FILE_DEVICE_MONO, 0x802, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_SET_CURSOR		CTL_CODE(FILE_DEVICE_MONO, 0x803, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_SCROLL			CTL_CODE(FILE_DEVICE_MONO, 0x804, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_BRING_TO_TOP	CTL_CODE(FILE_DEVICE_MONO, 0x805, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_SET_ATTRIBUTE	CTL_CODE(FILE_DEVICE_MONO, 0x806, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_PAN				CTL_CODE(FILE_DEVICE_MONO, 0x807, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_LOCK				CTL_CODE(FILE_DEVICE_MONO, 0x808, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_UNLOCK			CTL_CODE(FILE_DEVICE_MONO, 0x809, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_SET_WINDOW		CTL_CODE(FILE_DEVICE_MONO, 0x80A, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_RESET_WINDOW	CTL_CODE(FILE_DEVICE_MONO, 0x80B, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_SET_FLAG		CTL_CODE(FILE_DEVICE_MONO, 0x80C, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_CLEAR_FLAG		CTL_CODE(FILE_DEVICE_MONO, 0x80D, METHOD_BUFFERED, FILE_WRITE_DATA)
#define IOCTL_MONO_FILL_ATTRIB	CTL_CODE(FILE_DEVICE_MONO, 0x80E, METHOD_BUFFERED, FILE_WRITE_DATA)

/*
**	These specify the various behavior flags available for the mono display driver. They 
**	default to being OFF (or FALSE).
*/
typedef enum MonoFlagType 
{
	MONOFLAG_WRAP,			// Text will wrap to the next line when right margin is reached.

	MONOFLAG_COUNT			// Used to indicate the number of mono flags available.
} MonoFlagType;


/*
	Here is a "C" example of how to use the Monochrome Display Driver.


int main(int argc, char *argv[])
{
	HANDLE handle;

	handle = CreateFile("\\\\.\\MONO", GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle != INVALID_HANDLE_VALUE)  {
		long retval;		// Return code from IoControl functions.
		unsigned short * pointer;	// Working pointer to mono RAM.
		union {
			int X,Y;
		} cursor;			// Cursor positioning parameter info.

		// Prints a line of text to the screen [length of string is 13].
		WriteFile(handle, "Test Message\n", 13, &retval, NULL);

		// Fetches a pointer to the mono memory.
		DeviceIoControl(handle, (DWORD)IOCTL_MONO_LOCK, NULL, 0, &pointer, sizeof(pointer), &retval, 0);
		if (pointer != NULL) {
			*pointer = 0x0721;		// '!' character appears in upper left corner (attribute 0x07).
		}
		DeviceIoControl(handle, (DWORD)IOCTL_MONO_UNLOCK, NULL, 0, NULL, 0, &retval, 0);

		// Set cursor to column 5, row 10.
		cursor.X = 5;
		cursor.Y = 10;
		DeviceIoControl(handle, (DWORD)IOCTL_MONO_SET_CURSOR, &cursor, sizeof(cursor), NULL, 0, &retval, 0);

		// Clear the screen.
		DeviceIoControl(handle, (DWORD)IOCTL_MONO_CLEAR_SCREEN, NULL, 0, NULL, 0, &retval, 0);

		CloseHandle(handle);
	}

	return 0;
}

*/


