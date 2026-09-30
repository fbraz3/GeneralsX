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

// FILE: XferCRC.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, February 2002
// Desc:   Xfer CRC implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/XferCRC.h"
#include "Common/XferDeepCRC.h"
#include "Common/crc.h"
#include "Common/Snapshot.h"
#include "Utility/endian_compat.h"

#if DEEP_CRC_TO_MEMORY
#include "GameLogic/GameLogic.h"
#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferCRC::XferCRC()
{

	m_xferMode = XFER_CRC;
	m_crc = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferCRC::~XferCRC()
{

}

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for writing */
//-------------------------------------------------------------------------------------------------
void XferCRC::open( AsciiString identifier )
{

	// call base class
	Xfer::open( identifier );

	// initialize CRC to brand new one at zero
	m_crc = 0;

}

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferCRC::close()
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int XferCRC::beginBlock()
{

	return 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::endBlock()
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::addCRC( UnsignedInt val )
{

	m_crc = (m_crc << 1) + htobe(val) + ((m_crc >> 31) & 0x01);

}

// ------------------------------------------------------------------------------------------------
/** Entry point for xfering a snapshot */
// ------------------------------------------------------------------------------------------------
void XferCRC::xferSnapshot( Snapshot *snapshot )
{

	if( snapshot == nullptr )
	{

		return;

	}

	// run the crc function of the snapshot
	snapshot->crc( this );

}

//-------------------------------------------------------------------------------------------------
/** Perform a single CRC operation on the data passed in */
//-------------------------------------------------------------------------------------------------
void XferCRC::xferImplementation( void *data, Int dataSize )
{
	const UnsignedInt *uintPtr = (const UnsignedInt *) (data);
	dataSize *= (data != nullptr);

	int dataBytes = (dataSize / 4);

	for (Int i=0 ; i<dataBytes; ++i)
	{
		addCRC (*uintPtr++);
	}

	UnsignedInt val = 0;
	const unsigned char *c = (const unsigned char *)uintPtr;

	switch(dataSize & 3)
	{
	case 3:
		val += (c[2] << 16);
		FALLTHROUGH;
	case 2:
		val += (c[1] << 8);
		FALLTHROUGH;
	case 1:
		val += c[0];
		m_crc = (m_crc << 1) + val + ((m_crc >> 31) & 0x01);
		FALLTHROUGH;
	default:
		break;
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::skip( Int dataSize )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt XferCRC::getCRC()
{

	return htobe(m_crc);

}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferDeepCRC::XferDeepCRC()
{

#if DEEP_CRC_TO_MEMORY
	m_xferMode = XFER_CRC;
	m_buffer = nullptr;
	m_bufferIndex = 0;
#else
	m_xferMode = XFER_SAVE;
#endif

	m_fileFP = nullptr;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferDeepCRC::~XferDeepCRC()
{

	// warn the user if a file was left open
	if( m_fileFP != nullptr )
	{

		DEBUG_CRASH(( "Warning: Xfer file '%s' was left open", m_identifier.str() ));
		close();

	}

}

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for writing */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::open( AsciiString identifier )
{

#if DEEP_CRC_TO_MEMORY
	m_xferMode = XFER_CRC;
#else
	m_xferMode = XFER_SAVE;
#endif

#if !DEEP_CRC_TO_MEMORY
	// sanity, check to see if we're already open
	if( m_fileFP != nullptr )
	{

		DEBUG_CRASH(( "Cannot open file '%s' cause we've already got '%s' open",
									identifier.str(), m_identifier.str() ));
		throw XFER_FILE_ALREADY_OPEN;

	}
#endif

	// call base class
	Xfer::open( identifier );

#if DEEP_CRC_TO_MEMORY
	m_buffer = &TheGameLogic->getCRCBuffer();
	m_bufferIndex = 0;

	AsciiString str;
	str.format("[ START OF DEEP CRC FRAME %d ]", TheGameLogic->getFrame());
	const UnsignedInt length = str.getLength();

	while (m_bufferIndex + length >= m_buffer->size())
	{
		m_buffer->resize(m_buffer->size() * 2);
	}

	memcpy(&(*m_buffer)[m_bufferIndex], str.str(), length);
	m_bufferIndex += length;
#else
	// open the file
	m_fileFP = fopen( identifier.str(), "w+b" );
	if( m_fileFP == nullptr )
	{

		DEBUG_CRASH(( "File '%s' not found", identifier.str() ));
		throw XFER_FILE_NOT_FOUND;

	}
#endif

	// initialize CRC to brand new one at zero
	m_crc = 0;

}

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::close()
{

#if DEEP_CRC_TO_MEMORY
	AsciiString str;
	str.format("[ END OF DEEP CRC FRAME %d ]", TheGameLogic->getFrame());
	const UnsignedInt length = str.getLength();

	while (m_bufferIndex + length >= m_buffer->size())
	{
		m_buffer->resize(m_buffer->size() * 2);
	}

	memcpy(&(*m_buffer)[m_bufferIndex], str.str(), length);
	m_bufferIndex += length;

	TheGameLogic->storeCRCBuffer(m_bufferIndex);
#else
	// sanity, if we don't have an open file we can do nothing
	if( m_fileFP == nullptr )
	{

		DEBUG_CRASH(( "Xfer close called, but no file was open" ));
		throw XFER_FILE_NOT_OPEN;

	}

	// close the file
	fclose( m_fileFP );
	m_fileFP = nullptr;
#endif

#if DEEP_CRC_TO_MEMORY
	m_buffer = nullptr;
#endif

	// erase the filename
	m_identifier.clear();

}

//-------------------------------------------------------------------------------------------------
/** Perform a single CRC operation on the data passed in */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::xferImplementation( void *data, Int dataSize )
{

	if (!data || dataSize < 1)
	{
		return;
	}

#if DEEP_CRC_TO_MEMORY
	while (m_bufferIndex + dataSize >= m_buffer->size())
	{
		m_buffer->resize(m_buffer->size() * 2);
	}

	memcpy(&(*m_buffer)[m_bufferIndex], data, dataSize);
	m_bufferIndex += dataSize;
#else
	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != nullptr, ("XferSave - file pointer for '%s' is null",
										 m_identifier.str()) );

	// write data to file
	if( fwrite( data, dataSize, 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferSave - Error writing to file '%s'", m_identifier.str() ));
		throw XFER_WRITE_ERROR;

	}
#endif

	XferCRC::xferImplementation( data, dataSize );

}

// ------------------------------------------------------------------------------------------------
/** Save ascii string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferMarkerLabel( AsciiString asciiStringData )
{

}

// ------------------------------------------------------------------------------------------------
/** Save ascii string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferAsciiString( AsciiString *asciiStringData )
{

	// sanity
	if( asciiStringData->getLength() > 16385 )
	{

		DEBUG_CRASH(( "XferSave cannot save this ascii string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}

	// save length of string to follow
	UnsignedShort len = asciiStringData->getLength();
	xferUnsignedShort( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)asciiStringData->str(), sizeof( Byte ) * len );

}

// ------------------------------------------------------------------------------------------------
/** Save unicodee string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferUnicodeString( UnicodeString *unicodeStringData )
{

	// sanity
	if( unicodeStringData->getLength() > 255 )
	{

		DEBUG_CRASH(( "XferSave cannot save this unicode string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}

	// save length of string to follow
	Byte len = unicodeStringData->getLength();
	xferByte( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)unicodeStringData->str(), sizeof( WideChar ) * len );

}

#if DEEP_CRC_TO_MEMORY
void XferDeepCRC::changeXferMode(XferMode xferMode)
{
	m_xferMode = xferMode;
}

void XferDeepCRC::xferLogString(const AsciiString& str)
{
	if (m_buffer)
	{
		xferUser(const_cast<char*>(str.str()), str.getLength());
	}
}
#endif
