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

// FILE: XferDeepCRC.h ////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer hard disk read implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/Xfer.h"
#include "Common/XferCRC.h"

#if DEEP_CRC_TO_MEMORY
#include <vector>
#endif

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Snapshot;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class XferDeepCRC : public XferCRC
{

public:

	XferDeepCRC();
	virtual ~XferDeepCRC() override;

	// Xfer methods
	virtual void open( AsciiString identifier ) override;		///< start a CRC session with this xfer instance
	virtual void close() override;											///< stop CRC session

	// xfer methods
	virtual void xferMarkerLabel( AsciiString asciiStringData ) override;  ///< xfer ascii string (need our own)
	virtual void xferAsciiString( AsciiString *asciiStringData ) override;  ///< xfer ascii string (need our own)
	virtual void xferUnicodeString( UnicodeString *unicodeStringData ) override;	///< xfer unicode string (need our own);

#if DEEP_CRC_TO_MEMORY
	virtual void xferLogString(const AsciiString& str) override;
	void changeXferMode(XferMode xferMode);
#endif

protected:

	virtual void xferImplementation( void *data, Int dataSize ) override;

#if DEEP_CRC_TO_MEMORY
	std::vector<UnsignedByte>* m_buffer;									///< pointer to buffer
	size_t m_bufferIndex;																	///< current index in buffer
#endif
	FILE * m_fileFP;																			///< pointer to file
};
