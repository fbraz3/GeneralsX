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
 *                     $Archive:: /Commando/Library/BLWSTRAW.CPP                              $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   BlowStraw::Get -- Fetch a block of data from the straw.                                   *
 *   BlowStraw::Key -- Submit a key to the Blowfish straw.                                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blwstraw.h"
#include	<string.h>
#include	<assert.h>


/***********************************************************************************************
 * BlowStraw::Get -- Fetch a block of data from the straw.                                     *
 *                                                                                             *
 *    This routine will take a block of data from the straw and process it according to the    *
 *    encrypt/decrypt flag and the key supplied. Prior to a key be supplied, the data passes   *
 *    through this straw unchanged.                                                            *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the buffer to hold the data being requested.                *
 *                                                                                             *
 *          length   -- The length of the data being requested.                                *
 *                                                                                             *
 * OUTPUT:  Returns with the actual number of bytes stored into the buffer. If the number      *
 *          returned is less than the number requested, then this indicates that the data      *
 *          source has been exhausted.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int BlowStraw::Get(void * source, int slen)
{
	/*
	**	Verify the parameter for legality.
	*/
	if (source == NULL || slen <= 0) {
		return(0);
	}

	/*
	**	If there is no blowfish engine present, then merely pass the data through
	**	unchanged.
	*/
	if (BF == NULL) {
		return(Straw::Get(source, slen));
	}

	int total = 0;

	while (slen > 0) {

		/*
		**	If there are any left over bytes in the buffer, pass them
		**	through first.
		*/
		if (Counter > 0) {
			int sublen = (slen < Counter) ? slen : Counter;
			memmove(source, &Buffer[sizeof(Buffer)-Counter], sublen);
			Counter -= sublen;
			source = ((char *)source) + sublen;
			slen -= sublen;
			total += sublen;
		}
		if (slen == 0) break;

		/*
		**	Fetch and encrypt/decrypt the next block.
		*/
		int incount = Straw::Get(Buffer, sizeof(Buffer));
		if (incount == 0) break;

		/*
		**	Only full blocks are processed. Partial blocks are
		**	merely passed through unchanged.
		*/
		if (incount == sizeof(Buffer)) {
			if (Control == DECRYPT) {
				BF->Decrypt(Buffer, incount, Buffer);
			} else {
				BF->Encrypt(Buffer, incount, Buffer);
			}
		} else {
			memmove(&Buffer[sizeof(Buffer)-incount], Buffer, incount);
		}
		Counter = incount;
	}

	/*
	**	Return with the total number of bytes placed into the buffer.
	*/
	return(total);
}


/***********************************************************************************************
 * BlowStraw::Key -- Submit a key to the Blowfish straw.                                       *
 *                                                                                             *
 *    This will take the key specified and use it to process the data that flows through this  *
 *    straw segment. Prior to a key being submitted, the data will flow through unchanged.     *
 *                                                                                             *
 * INPUT:   key   -- Pointer to the key to submit.                                             *
 *                                                                                             *
 *          length-- The length of the key. The length must not exceed 56 bytes.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void BlowStraw::Key(void const * key, int length)
{
	/*
	**	Create the blowfish engine if one isn't already present.
	*/
	if (BF == NULL) {
		BF = new BlowfishEngine;
	}

	assert(BF != NULL);

	if (BF != NULL) {
		BF->Submit_Key(key, length);
	}
}
