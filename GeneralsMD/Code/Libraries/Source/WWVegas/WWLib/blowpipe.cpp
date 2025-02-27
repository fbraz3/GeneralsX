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
 *                     $Archive:: /Commando/Library/BLOWPIPE.CPP                              $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   BlowPipe::Flush -- Flushes any pending data out the pipe.                                 *
 *   BlowPipe::Key -- Submit a key to the blowfish pipe handler.                               *
 *   BlowPipe::Put -- Submit a block of data for encrypt/decrypt.                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blowpipe.h"
#include	<string.h>
#include	<assert.h>


/***********************************************************************************************
 * BlowPipe::Flush -- Flushes any pending data out the pipe.                                   *
 *                                                                                             *
 *    If there is any pending data in the holding buffer, then this routine will force it to   *
 *    be flushed out the end of the pipe.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the actual number of bytes output at the end final distant pipe       *
 *          segment in the chain.                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int BlowPipe::Flush(void)
{
	int total = 0;
	if (Counter > 0 && BF != NULL) {
		total += Pipe::Put(Buffer, Counter);
	}
	Counter = 0;
	total += Pipe::Flush();
	return(total);
}


/***********************************************************************************************
 * BlowPipe::Put -- Submit a block of data for encrypt/decrypt.                                *
 *                                                                                             *
 *    This will take the data block specified and process it before passing it on to the next  *
 *    link in the pipe chain. A key must be submitted before this routine will actually perform*
 *    any processing. Prior to key submission, the data is passed through unchanged.           *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the buffer that contains the data to pass through.          *
 *                                                                                             *
 *          length   -- The length of the data in the buffer.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with then actual number of bytes output at the final distant end link in   *
 *          the pipe chain.                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int BlowPipe::Put(void const * source, int slen)
{
	if (source == NULL || slen < 1) {
		return(Pipe::Put(source, slen));
	}

	/*
	**	If there is no blowfish engine present, then merely pass the data through
	**	unchanged in any way.
	*/
	if (BF == NULL) {
		return(Pipe::Put(source, slen));
	}

	int total = 0;

	/*
	**	If there is a partial block accumulated, then tag on the new data to
	**	this block and process it if the block is full. Proceed with the bulk
	**	processing if there are any left over bytes from this step. This step
	**	can be skipped if there are no pending bytes in the buffer.
	*/
	if (Counter) {
		int sublen = ((int)sizeof(Buffer)-Counter < slen) ? (sizeof(Buffer)-Counter) : slen;
		memmove(&Buffer[Counter], source, sublen);
		Counter += sublen;
		source = ((char *)source) + sublen;
		slen -= sublen;

		if (Counter == sizeof(Buffer)) {
			if (Control == DECRYPT) {
				BF->Decrypt(Buffer, sizeof(Buffer), Buffer);
			} else {
				BF->Encrypt(Buffer, sizeof(Buffer), Buffer);
			}
			total += Pipe::Put(Buffer, sizeof(Buffer));
			Counter = 0;
		}
	}

	/*
	**	Process the input data in blocks until there is not enough
	**	source data to fill a full block of data.
	*/
	while (slen >= sizeof(Buffer)) {
		if (Control == DECRYPT) {
			BF->Decrypt(source, sizeof(Buffer), Buffer);
		} else {
			BF->Encrypt(source, sizeof(Buffer), Buffer);
		}
		total += Pipe::Put(Buffer, sizeof(Buffer));
		source = ((char *)source) + sizeof(Buffer);
		slen -= sizeof(Buffer);
	}

	/*
	**	If there are any left over bytes, then they must be less than the size of
	**	the staging buffer. Store the bytes in the staging buffer for later
	**	processing.
	*/
	if (slen > 0) {
		memmove(Buffer, source, slen);
		Counter = slen;
	}

	/*
	**	Return with the total number of bytes flushed out to the final end of the
	**	pipe chain.
	*/
	return(total);
}


/***********************************************************************************************
 * BlowPipe::Key -- Submit a key to the blowfish pipe handler.                                 *
 *                                                                                             *
 *    This routine will take the key provided and use it to process the data that passes       *
 *    through this pipe. Prior to a key being submitted, the data passes through the pipe      *
 *    unchanged.                                                                               *
 *                                                                                             *
 * INPUT:   key   -- Pointer to the key data to use.                                           *
 *                                                                                             *
 *          length-- The length of the key. The key length must not be greater than 56 bytes.  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void BlowPipe::Key(void const * key, int length)
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



