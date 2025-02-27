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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					                  
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:    WSYS Library
//
// Module:		String
//
// File name:  wsys/string.h
//
// Created:    11/02/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __WSYS_STRING_H
#define __WSYS_STRING_H


//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


class WSYS_String
{
	protected:

		Char	*m_data;										///< actual string data

	public:

	explicit WSYS_String(const Char *string = NULL);
	~WSYS_String();

	// operators
	Bool								operator== (const char *rvalue) const;
	Bool								operator!= (const char *rvalue) const;
	const WSYS_String&	operator= (const WSYS_String &string);
	const WSYS_String&	operator= (const Char *string);
	const WSYS_String&	operator+= (const WSYS_String &string);
	const WSYS_String&	operator+= (const Char *string);
	friend WSYS_String	operator+ (const WSYS_String &string1, const WSYS_String &string2);
	friend WSYS_String	operator+ (const Char *string1, const WSYS_String &string2);
	friend WSYS_String	operator+ (const WSYS_String &string1, const Char *string2);
	const Char &				operator[] (Int index) const;
	Char &							operator[] (Int index);
											operator const Char * (void) const;
											operator Char * (void) const ;

	// methods
	void								makeUpperCase( void );
	void								makeLowerCase( void );
	Int									length(void) const;
	Bool								isEmpty(void) const;
	Int _cdecl					format(const Char *format, ...);
	void								set( const Char *string );
	Char*								get( void ) const;
};



//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------

inline Char* WSYS_String::get( void ) const { return m_data;};
inline const Char& WSYS_String::operator[] (Int index) const{ return m_data[index];};
inline Char& WSYS_String::operator[] (Int index) { return m_data[index];};
inline WSYS_String::operator const Char * (void) const { return m_data;};
inline WSYS_String::operator Char * (void) const {return m_data;};


#endif // __WSYS_STRING_H
