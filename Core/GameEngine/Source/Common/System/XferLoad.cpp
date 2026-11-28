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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: XferLoad.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer implementation for loading from disk
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Debug.h"
#include "Common/GameState.h"
#include "Common/Snapshot.h"
#include "Common/XferLoad.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferLoad::XferLoad()
{

	m_xferMode = XFER_LOAD;
	m_fileFP = nullptr;
	m_unicodeEncoding = UNICODE_ENCODING_UNKNOWN;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferLoad::~XferLoad()
{

	// warn the user if a file was left open
	if( m_fileFP != nullptr )
	{

		DEBUG_CRASH(( "Warning: Xfer file '%s' was left open", m_identifier.str() ));
		close();

	}

}

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for reading */
//-------------------------------------------------------------------------------------------------
void XferLoad::open( AsciiString identifier )
{

	// sanity, check to see if we're already open
	if( m_fileFP != nullptr )
	{

		DEBUG_CRASH(( "Cannot open file '%s' cause we've already got '%s' open",
									identifier.str(), m_identifier.str() ));
		throw XFER_FILE_ALREADY_OPEN;

	}

	// call base class
	Xfer::open( identifier );
	m_unicodeEncoding = UNICODE_ENCODING_UNKNOWN;

	// open the file
	m_fileFP = fopen( identifier.str(), "rb" );
	if( m_fileFP == nullptr )
	{

		DEBUG_CRASH(( "File '%s' not found", identifier.str() ));
		throw XFER_FILE_NOT_FOUND;

	}

}

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferLoad::close()
{

	// sanity, if we don't have an open file we can do nothing
	if( m_fileFP == nullptr )
	{

		DEBUG_CRASH(( "Xfer close called, but no file was open" ));
		throw XFER_FILE_NOT_OPEN;

	}

	// close the file
	fclose( m_fileFP );
	m_fileFP = nullptr;

	// erase the filename
	m_identifier.clear();
	m_unicodeEncoding = UNICODE_ENCODING_UNKNOWN;

}

//-------------------------------------------------------------------------------------------------
/** Read a block size descriptor from the file at the current position */
//-------------------------------------------------------------------------------------------------
Int XferLoad::beginBlock()
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != nullptr, ("Xfer begin block - file pointer for '%s' is null",
										 m_identifier.str()) );

	// read block size
	XferBlockSize blockSize;
	if( fread( &blockSize, sizeof( XferBlockSize ), 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "Xfer - Error reading block size for '%s'", m_identifier.str() ));
		return 0;

	}

	// return the block size
	return blockSize;

}

// ------------------------------------------------------------------------------------------------
/** End block ... this does nothing when reading */
// ------------------------------------------------------------------------------------------------
void XferLoad::endBlock()
{

}

//-------------------------------------------------------------------------------------------------
/** Skip forward 'dataSize' bytes in the file */
//-------------------------------------------------------------------------------------------------
void XferLoad::skip( Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != nullptr, ("XferLoad::skip - file pointer for '%s' is null",
										 m_identifier.str()) );

	// sanity
	DEBUG_ASSERTCRASH( dataSize >=0, ("XferLoad::skip - dataSize '%d' must be greater than 0",
										 dataSize) );

	// skip datasize in the file from the current position
	if( fseek( m_fileFP, dataSize, SEEK_CUR ) != 0 )
		throw XFER_SKIP_ERROR;

}

// ------------------------------------------------------------------------------------------------
/** Entry point for xfering a snapshot */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferSnapshot( Snapshot *snapshot )
{

	if( snapshot == nullptr )
	{

		DEBUG_CRASH(( "XferLoad::xferSnapshot - Invalid parameters" ));
		throw XFER_INVALID_PARAMETERS;

	}

	// run the xfer function of the snapshot
	snapshot->xfer( this );

	// add this snapshot to the game state for later post processing if not restricted
	if( BitIsSet( getOptions(), XO_NO_POST_PROCESSING ) == FALSE )
		TheGameState->addPostProcessSnapshot( snapshot );

}

// ------------------------------------------------------------------------------------------------
/** Read string from file and store in ascii string */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferAsciiString( AsciiString *asciiStringData )
{

	// read bytes of string length to follow
	UnsignedByte len;
	xferUnsignedByte( &len );

	// read all the string data
	const Int MAX_XFER_LOAD_STRING_BUFFER = 1024;
	static Char buffer[ MAX_XFER_LOAD_STRING_BUFFER ];

	if( len > 0 )
		xferUser( buffer, sizeof( Byte ) * len );
	buffer[ len ] = 0;  // terminate

	// save into ascii string
	asciiStringData->set( buffer );

}

// ------------------------------------------------------------------------------------------------
/** Append a Unicode code point to a native WideChar buffer */
// ------------------------------------------------------------------------------------------------
static void appendCodePoint( WideChar *buffer, Int &charCount, UnsignedInt codePoint )
{
	if( sizeof( WideChar ) <= 2 && codePoint > 0xFFFF )
	{
		if( codePoint > 0x10FFFF )
		{
			buffer[ charCount++ ] = (WideChar)0xFFFD;
		}
		else
		{
			const UnsignedInt offset = codePoint - 0x10000;
			buffer[ charCount++ ] = (WideChar)(0xD800 + (offset >> 10));
			buffer[ charCount++ ] = (WideChar)(0xDC00 + (offset & 0x3FF));
		}
		return;
	}

	buffer[ charCount++ ] = (WideChar)codePoint;
}

// ------------------------------------------------------------------------------------------------
/** Decode a buffer of 2-byte UTF-16 code units into a native WideChar buffer */
// ------------------------------------------------------------------------------------------------
static void decodeUtf16Buffer( const UnsignedShort *fileBuffer, Int len, WideChar *buffer, Int &charCount )
{
	for( Int i = 0; i < len; ++i )
	{
		UnsignedInt codePoint = fileBuffer[ i ];

		if( sizeof( WideChar ) > 2 &&
				codePoint >= 0xD800 && codePoint <= 0xDBFF && i + 1 < len &&
				fileBuffer[ i + 1 ] >= 0xDC00 && fileBuffer[ i + 1 ] <= 0xDFFF )
		{
			codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (fileBuffer[ i + 1 ] - 0xDC00);
			++i;
		}

		buffer[ charCount++ ] = (WideChar)codePoint;
	}
}

// ------------------------------------------------------------------------------------------------
/** Read string from file and store in unicode string */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferUnicodeString( UnicodeString *unicodeStringData )
{

	// read bytes of string length to follow
	UnsignedByte len;
	xferUnsignedByte( &len );

	// read all the string data
	const Int MAX_XFER_LOAD_STRING_BUFFER = 1024;
	static WideChar buffer[ MAX_XFER_LOAD_STRING_BUFFER ];

	// GeneralsX @bugfix alviss7 22/11/2026 Reads unicode strings as 2 byte UTF-16 code units
	// instead of the native WideChar.
	// GeneralsX @bugfix fbraz3 26/11/2026 Auto-detect and decode legacy 4-byte UTF-32LE
	// strings written by pre-1.0.9 Linux/macOS builds so existing savegames remain readable.
	Int charCount = 0;
	if( len > 0 )
	{

		if( m_unicodeEncoding == UNICODE_ENCODING_UTF32 )
		{

			UnsignedInt legacyBuffer[ MAX_XFER_UNICODE_STRING_LEN ];
			xferUser( legacyBuffer, sizeof( UnsignedInt ) * len );
			for( Int i = 0; i < len; ++i )
				appendCodePoint( buffer, charCount, legacyBuffer[ i ] );

		}
		else if( m_unicodeEncoding == UNICODE_ENCODING_UTF16 )
		{

			UnsignedShort fileBuffer[ MAX_XFER_UNICODE_STRING_LEN ];
			xferUser( fileBuffer, sizeof( UnsignedShort ) * len );
			decodeUtf16Buffer( fileBuffer, len, buffer, charCount );

		}
		else
		{

			UnsignedShort fileBuffer[ MAX_XFER_UNICODE_STRING_LEN ];
			xferUser( fileBuffer, sizeof( UnsignedShort ) * len );

			Bool isLegacyUtf32 = FALSE;
			if( len >= 2 )
			{

				// Valid UTF-16 strings never contain 0x0000; in 4-byte UTF-32LE, the high 16 bits
				// of BMP characters are 0x0000. Restricting detection to 0x0000 avoids misclassifying
				// UTF-16 control characters (e.g. \b, \t, \n) followed by binary fields.
				isLegacyUtf32 = (fileBuffer[ 1 ] == 0x0000) || (len >= 4 && fileBuffer[ 3 ] == 0x0000);
				if( !isLegacyUtf32 )
					m_unicodeEncoding = UNICODE_ENCODING_UTF16;

			}
			else
			{

				const long probePos = ftell( m_fileFP );
				UnsignedShort nextWord = 0;
				if( fread( &nextWord, sizeof( UnsignedShort ), 1, m_fileFP ) == 1 && nextWord == 0x0000 )
				{

					isLegacyUtf32 = TRUE;
					m_unicodeEncoding = UNICODE_ENCODING_UTF32;
					appendCodePoint( buffer, charCount, (UnsignedInt)fileBuffer[ 0 ] );

				}
				else
				{

					clearerr( m_fileFP );
					if( probePos >= 0 )
						fseek( m_fileFP, probePos, SEEK_SET );
					m_unicodeEncoding = UNICODE_ENCODING_UTF16;

				}

			}

			if( isLegacyUtf32 && len >= 2 )
			{

				m_unicodeEncoding = UNICODE_ENCODING_UTF32;
				UnsignedShort tailBuffer[ MAX_XFER_UNICODE_STRING_LEN ];
				xferUser( tailBuffer, sizeof( UnsignedShort ) * len );
				for( Int i = 0; i < len; ++i )
				{

					const Int loIndex = 2 * i;
					const Int hiIndex = loIndex + 1;
					const UnsignedShort lo = (loIndex < len) ? fileBuffer[ loIndex ] : tailBuffer[ loIndex - len ];
					const UnsignedShort hi = (hiIndex < len) ? fileBuffer[ hiIndex ] : tailBuffer[ hiIndex - len ];
					appendCodePoint( buffer, charCount, (UnsignedInt)lo | ((UnsignedInt)hi << 16) );

				}

			}
			else if( !isLegacyUtf32 )
			{

				decodeUtf16Buffer( fileBuffer, len, buffer, charCount );

			}

		}

	}
	buffer[ charCount ] = 0;  // terminate

	// save into unicode string
	unicodeStringData->set( buffer );

}

//-------------------------------------------------------------------------------------------------
/** Perform the read operation */
//-------------------------------------------------------------------------------------------------
void XferLoad::xferImplementation( void *data, Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != nullptr, ("XferLoad - file pointer for '%s' is null",
										 m_identifier.str()) );

	// read data from file
	if( fread( data, dataSize, 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferLoad - Error reading from file '%s'", m_identifier.str() ));
		throw XFER_READ_ERROR;

	}

}

