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

// FILE: INIWebpageURL.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, November 2001
// Desc:   Parsing Webpage URL INI entries
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/Registry.h"
#include <cstddef>
#include <Utility/compat.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class WebBrowserURL
{
public:
	WebBrowserURL() {}
	const FieldParse* getFieldParse(void) const { return m_URLFieldParseTable; }

	AsciiString m_tag;
	AsciiString m_url;

	static const FieldParse m_URLFieldParseTable[];
};

const FieldParse WebBrowserURL::m_URLFieldParseTable[] =
{
	{ "URL", INI::parseAsciiString, NULL, offsetof(WebBrowserURL, m_url) },
	{ NULL, NULL, NULL, 0 }
};

AsciiString encodeURL(AsciiString source)
{
	if (source.isEmpty())
	{
		return AsciiString::TheEmptyString;
	}

	AsciiString target;
	AsciiString allowedChars = "$-_.+!*'(),\\";
	const char* ptr = source.str();
	while (*ptr)
	{
		if (isalnum(*ptr) || allowedChars.find(*ptr))
		{
			target.concat(*ptr);
		}
		else
		{
			AsciiString tmp;
			target.concat('%');
			tmp.format("%2.2x", ((int)*ptr));
			target.concat(tmp);
		}
		++ptr;
	}

	return target;
}

//-------------------------------------------------------------------------------------------------
/** Parse Music entry */
//-------------------------------------------------------------------------------------------------

void INI::parseWebpageURLDefinition(INI* ini)
{
	AsciiString tag;
	const char* c = ini->getNextToken();
	tag.set(c);

	WebBrowserURL url;
	url.m_tag.set(tag);

	ini->initFromINI(&url, url.getFieldParse());

	if (url.m_url.startsWith("file://"))
	{
		char cwd[_MAX_PATH];
		getcwd(cwd, _MAX_PATH);

		url.m_url.format("file://%s%sData%s%s%s%s", encodeURL(cwd).str(), GET_PATH_SEPARATOR(), GET_PATH_SEPARATOR(), GetRegistryLanguage().str(), GET_PATH_SEPARATOR(), url.m_url.str() + 7);
		DEBUG_LOG(("INI::parseWebpageURLDefinition() - converted URL to [%s]", url.m_url.str()));
	}
}


