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
 *                     $Archive:: /Commando/Library/SHAPIPE.CPP                               $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   SHAPipe::Result -- Fetches the current SHA value.                                         *
 *   SHAPipe::Put -- Pass data through the pipe, but use it to build a SHA digest.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include	"shapipe.h"


/***********************************************************************************************
 * SHAPipe::Put -- Pass data through the pipe, but use it to build a SHA digest.               *
 *                                                                                             *
 *    This pipe segment will not modify the data, but it will examine the data and use it when *
 *    building a SHA digest.                                                                   *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the data to flow through the pipe.                          *
 *                                                                                             *
 *          length   -- The number of bytes to submit.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the actual number of bytes output at the distant final end of the     *
 *          pipe chain.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int SHAPipe::Put(void const * source, int slen)
{
	SHA.Hash(source, slen);
	return(Pipe::Put(source, slen));
}


/***********************************************************************************************
 * SHAPipe::Result -- Fetches the current SHA value.                                           *
 *                                                                                             *
 *    This routine will return the SHA digest for the data that has passed through this        *
 *    link in the pipe chain. It is a non-destructive read.                                    *
 *                                                                                             *
 * INPUT:   result   -- Pointer to the buffer to hold the SHA digest. This buffer must be      *
 *                      20 bytes long.                                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes copied into the buffer. This will be 20.          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int SHAPipe::Result(void * result) const
{
	return(SHA.Result(result));
}


