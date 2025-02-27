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
 *                     $Archive:: /Commando/Library/RLE.H                                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef RLE_H
#define RLE_H

/*
**	This class will RLE compress and decompress arbitrary blocks of data. This RLE compression
**	is geared to compressing only runs of 0 bytes. This makes it useful for sprite encoding.
*/
class RLEEngine 
{
	public:
	
		/*
		**	Codec for arbitrary blocks.
		*/
		int Compress(void const * source, void * dest, int length) const;
		int Decompress(void const * source, void * dest, int length) const;

		/*
		**	Codec for length encoded blocks. This is useful for sprite storage.
		*/
		int Line_Compress(void const * source, void * dest, int length) const;
		int Line_Decompress(void const * source, void * dest) const;
};

#endif


