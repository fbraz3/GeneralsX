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
 *                     $Archive:: /Commando/Code/wwlib/blitter.h                              $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 5/04/01 7:48p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef BLITTER_H
#define BLITTER_H

/*
**	This is the interface class to the blitter object. The blitter object handles moving
**	pixels. That's all it does. For every type of pixel translation, there should be a
**	blitter object created that supports this interface. The blit blitting routines will
**	call the appropriate method as the pixel are being processed.
*/
class Blitter {
	public:

		/*
		**	Blits from source to dest (starts at first pixel). This is the preferred
		**	method of pixel blitting and this routine will be called 99% of the time under
		**	normal circumstances.
		*/
		virtual void BlitForward(void * dest, void const * source, int length) const = 0;

		/*
		**	Copies the pixel in reverse order. This only required if the source and dest
		**	pixel regions overlap in a certain way. This routine will rarely be called.
		*/
		virtual void BlitBackward(void * dest, void const * source, int length) const = 0;

		/*
		**	This routine calls the appropriate blit routine. A proper overlap check cannot
		**	be performed by this routine because the pixel size information is not present.
		**	as such, you should call the appropriate blit routine rather than letting this
		**	routine perform the check and call.
		*/
		void Blit(void * dest, void const * source, int length) const {if (dest < source) BlitBackward(dest, source, length); else BlitForward(dest, source, length);}
};


/*
**	This is the blitter object interface for dealing with RLE compressed pixel data. For
**	every type of RLE compressed blitter operation desired, there would be an object created
**	that supports this interface.
*/
class RLEBlitter {
	public:

		/*
		**	Blits from the RLE compressed source to the destination buffer. An optional
		**	leading pixel skip value can be supplied when a sub-section of an RLE
		**	compressed pixel sequence is desired. This is necessary because RLE decompression
		**	must begin at the start of the compressed data sequence.
		*/
		virtual void Blit(void * dest, void const * source, int length, int leadskip=0) const = 0;
};



#endif
