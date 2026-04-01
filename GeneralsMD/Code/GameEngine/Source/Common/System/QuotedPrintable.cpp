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

// FILE: QuotedPrintable.cpp /////////////////////////////////////////////////////////
// Author: Matt Campbell, February 2002
// Description: Quoted-printable encode/decode
////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/QuotedPrintable.h"

#define MAGIC_CHAR '_'

// takes an integer and returns an ASCII representation
static char intToHexDigit(int num)
{
	if (num<0 || num >15) return '\0';
	if (num<10)
	{
		return '0' + num;
	}
	return 'A' + (num-10);
}

// convert an ASCII representation of a hex digit into the digit itself
static int hexDigitToInt(char c)
{
	if (c <= '9' && c >= '0') return (c - '0');
	if (c <= 'f' && c >= 'a') return (c - 'a' + 10);
	if (c <= 'F' && c >= 'A') return (c - 'A' + 10);
	return 0;
}

// GeneralsX @bugfix copilot 01/04/2026 Keep wire format locale-independent by allowing only ASCII alnum bytes as literal.
static Bool isAsciiAlphaNumeric(unsigned char value)
{
	return (value >= '0' && value <= '9')
		|| (value >= 'A' && value <= 'Z')
		|| (value >= 'a' && value <= 'z');
}

static Bool isHexDigit(char c)
{
	return (c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'F')
		|| (c >= 'a' && c <= 'f');
}

// GeneralsX @bugfix copilot 31/03/2026 Encode/decode UTF-16 code units explicitly to avoid wchar_t size dependency.
static Bool appendQuotedPrintableByte(char *dest, int &index, unsigned char value)
{
	if (isAsciiAlphaNumeric(value))
	{
		if (index >= 1023)
		{
			return false;
		}
		dest[index++] = value;
		return true;
	}

	if (index > 1020)
	{
		return false;
	}

	dest[index++] = MAGIC_CHAR;
	dest[index++] = intToHexDigit(value >> 4);
	dest[index++] = intToHexDigit(value & 0xf);
	return true;
}

static Bool decodeQuotedPrintableByte(const unsigned char *&src, unsigned char &value)
{
	if (*src == MAGIC_CHAR)
	{
		if (src[1] == '\0' || src[2] == '\0')
		{
			return false;
		}

		if (!isHexDigit(src[1]) || !isHexDigit(src[2]))
		{
			return false;
		}

		value = static_cast<unsigned char>((hexDigitToInt(src[1]) << 4) | hexDigitToInt(src[2]));
		src += 3;
		return true;
	}

	value = *src;

	src++;
	return true;
}

// Convert unicode strings into ascii quoted-printable strings
AsciiString UnicodeStringToQuotedPrintable(UnicodeString original)
{
	static char dest[1024];
	const WideChar *src = original.str();
	int i = 0;

	while (*src)
	{
		const unsigned short codeUnit = static_cast<unsigned short>(*src);
		const unsigned char lowByte = static_cast<unsigned char>(codeUnit & 0xff);
		const unsigned char highByte = static_cast<unsigned char>((codeUnit >> 8) & 0xff);
		// GeneralsX @bugfix copilot 01/04/2026 Commit only full UTF-16 units to avoid partial trailing output.
		const int savedIndex = i;

		if (!appendQuotedPrintableByte(dest, i, lowByte) || !appendQuotedPrintableByte(dest, i, highByte))
		{
			i = savedIndex;
			break;
		}

		src++;
	}

	dest[i] = '\0';

	return dest;
}

// Convert ascii strings into ascii quoted-printable strings
AsciiString AsciiStringToQuotedPrintable(AsciiString original)
{
	static char dest[1024];
	const unsigned char *src = reinterpret_cast<const unsigned char *>(original.str());
	int i=0;
	while ( src[0]!='\0' && i<1021 )
	{
		if (!isalnum(*src))
		{
			dest[i++] = MAGIC_CHAR;
			dest[i++] = intToHexDigit((*src)>>4);
			dest[i++] = intToHexDigit((*src)&0xf);
		}
		else
		{
			dest[i++] = *src;
		}
		src ++;
	}
	dest[i] = '\0';

	return dest;
}

// Convert ascii quoted-printable strings into unicode strings
UnicodeString QuotedPrintableToUnicodeString(AsciiString original)
{
	static WideChar dest[1024];
	int i = 0;
	const unsigned char *src = reinterpret_cast<const unsigned char *>(original.str());

	while (*src && i < 1023)
	{
		unsigned char lowByte = 0;
		if (!decodeQuotedPrintableByte(src, lowByte))
		{
			break;
		}

		// GeneralsX @bugfix copilot 01/04/2026 Reject truncated odd-byte input instead of synthesizing a partial code unit.
		if (!*src)
		{
			break;
		}

		unsigned char highByte = 0;
		if (!decodeQuotedPrintableByte(src, highByte))
		{
			break;
		}

		const unsigned short codeUnit = static_cast<unsigned short>(lowByte | (highByte << 8));
		dest[i++] = static_cast<WideChar>(codeUnit);
	}

	dest[i] = 0;

	return dest;
}

// Convert ascii quoted-printable strings into ascii strings
AsciiString QuotedPrintableToAsciiString(AsciiString original)
{
	static char dest[1024];
	int i=0;

	unsigned char *c = reinterpret_cast<unsigned char *>(dest);
	const unsigned char *src = reinterpret_cast<const unsigned char *>(original.str());

	while (*src && i<1023)
	{
		if (*src == MAGIC_CHAR)
		{
			if (src[1] == '\0')
			{
				// string ends with MAGIC_CHAR
				break;
			}
			*c = hexDigitToInt(src[1]);
			src++;
			if (src[1] != '\0')
			{
				*c = *c<<4;
				*c = *c | hexDigitToInt(src[1]);
				src++;
			}
		}
		else
		{
			*c = *src;
		}
		src++;
		c++;
	}

	*c = 0;

	return dest;
}

