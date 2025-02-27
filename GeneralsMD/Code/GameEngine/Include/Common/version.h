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

// FILE: version.h //////////////////////////////////////////////////////
// Generals version number class
// Author: Matthew D. Campbell, November 2001

#pragma once

#ifndef __VERSION_H__
#define __VERSION_H__

/** 
 * The Version class formats the version number into integer and string
 * values for different parts of the game.
 * @todo: increment build number on compile, and stamp exe with username
 */
class Version
{
public:
	Version();
	UnsignedInt getVersionNumber( void );						///< Return a 4-byte integer suitable for WOLAPI
	AsciiString getAsciiVersion( void );						///< Return a human-readable version number
	UnicodeString getUnicodeVersion( void );				///< Return a human-readable version number
	UnicodeString getFullUnicodeVersion( void );		///< Return a human-readable version number
	AsciiString getAsciiBuildTime( void );					///< Return a formated date/time string for build time
	UnicodeString getUnicodeBuildTime( void );			///< Return a formated date/time string for build time
	AsciiString getAsciiBuildLocation( void );			///< Return a string with the build location
	UnicodeString getUnicodeBuildLocation( void );	///< Return a string with the build location
	AsciiString getAsciiBuildUser( void );					///< Return a string with the build user
	UnicodeString getUnicodeBuildUser( void );			///< Return a string with the build user

	Bool showFullVersion( void ) { return m_showFullVersion; }
	void setShowFullVersion( Bool val ) { m_showFullVersion = val; }

	void setVersion(Int major, Int minor, Int buildNum,
		Int localBuildNum, AsciiString user, AsciiString location,
		AsciiString buildTime, AsciiString buildDate); ///< Set version info

private:
	Int m_major;
	Int m_minor;
	Int m_buildNum;
	Int m_localBuildNum;
	AsciiString m_buildLocation;
	AsciiString m_buildUser;
	AsciiString m_buildTime;
	AsciiString m_buildDate;
	Bool m_showFullVersion;
};

extern Version *TheVersion;	///< The Version singleton

#endif // __VERSION_H__
