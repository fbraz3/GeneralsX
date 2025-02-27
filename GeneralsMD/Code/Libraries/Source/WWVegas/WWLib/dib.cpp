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
 *                     $Archive:: /G/wwlib/dib.cpp                                            $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 3/29/99 5:11p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   DIB8C::DIB8Class -- constructor                                                           * 
 *   DIB8C::~DIB8Class -- destructor                                                           * 
 *   DIB8C::Clear -- clears the DIB                                                            * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "dib.h"
#include <math.h>

/*********************************************************************************************** 
 * DIB8C::DIB8Class -- constructor                                                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/18/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
DIB8Class::DIB8Class(HWND hwnd,int width,int height,PaletteClass & pal):
	IsZombie(false),
	Info(NULL),
	Handle(0),
	Pixels(NULL),
	Width(width),
	Height(height),
	PixelBase(NULL),
	Pitch(NULL),
	Surface(NULL)
{
	// Allocate a BITMAPINFO structure
	Info = (BITMAPINFO *) new char [sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD)];

	if (Info == NULL) {
		IsZombie = true;
		return;
	}

	// Describe the type of DIB we want.
	Info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Info->bmiHeader.biWidth = width;
	Info->bmiHeader.biHeight = -height;		//ask for a top-down dib.
	Info->bmiHeader.biPlanes = 1;
	Info->bmiHeader.biBitCount = 8;
	Info->bmiHeader.biCompression = BI_RGB;
	Info->bmiHeader.biSizeImage = 0;
	Info->bmiHeader.biXPelsPerMeter = 0;
	Info->bmiHeader.biYPelsPerMeter = 0;
	Info->bmiHeader.biClrUsed = 256;
	Info->bmiHeader.biClrImportant = 256;

	// Fill in the DIB's palette.
	for (int i=0; i<256; i++) {
		Info->bmiColors[i].rgbBlue =		(unsigned char)pal[i].Get_Blue();
		Info->bmiColors[i].rgbGreen =		(unsigned char)pal[i].Get_Green();
		Info->bmiColors[i].rgbRed =		(unsigned char)pal[i].Get_Red();
		Info->bmiColors[i].rgbReserved =	0;
	}
	
	// Create the DIB.
	HDC hdc = GetDC(hwnd);
	Handle = CreateDIBSection(hdc, Info, DIB_RGB_COLORS,(void**)&Pixels, NULL, 0);
	ReleaseDC(hwnd, hdc);

	if (!Handle) {
		IsZombie = true;
		return;
    }

	Width = Info->bmiHeader.biWidth;
	Height = abs(Info->bmiHeader.biHeight);
	Pitch = (Width + 3) & 0xfffffffC;

	// Check if the DIB is bottom-up or top-down.
	// (it better be top-down, thats what I'm asking for!!!)
	if (Info->bmiHeader.biHeight > 0) {

		// bottom-up DIB
        PixelBase = (Pixels + (Height - 1) * Width);
        Pitch = -Pitch; 

    } else {

		// top-down DIB
        PixelBase = Pixels;
        Pitch = Pitch;    
    }

	// Now, wrap a BSurface around the DIB.
	int surfwid = abs(Pitch);
	Surface = new BSurface(surfwid,Height,1,Pixels);

	if (Surface == NULL) {
		IsZombie = true;
		return;
	}
}


/*********************************************************************************************** 
 * DIB8C::~DIB8Class -- destructor                                                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/18/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
DIB8Class::~DIB8Class(void)
{
	if (Info) delete [] Info;
	if (Handle) DeleteObject(Handle);
	if (Surface) delete Surface;
}


/*********************************************************************************************** 
 * DIB8C::Clear -- clears the DIB                                                              * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/18/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void DIB8Class::Clear(unsigned char color)
{
	if (Pixels) {
		memset(Pixels, color, Width*Height);
	}
}

