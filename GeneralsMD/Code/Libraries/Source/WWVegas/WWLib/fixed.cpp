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

/* $Header: /Commando/Code/Library/FIXED.CPP 2     10/05/98 3:30p Greg_h $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FIXED.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/20/96                                                     *
 *                                                                                             *
 *                  Last Update : July 3, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   fixed::As_ASCII -- Returns a pointer (static) of this number as an ASCII string.          *
 *   fixed::To_ASCII -- Convert a fixed point number into an ASCII string.                     *
 *   fixed::fixed -- Constructor for fixed integral from ASCII initializer.                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"fixed.h"
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<ctype.h>


/*
**	These are some handy fixed point constants. Using these constants instead of manually
**	constructing them is not only faster, but more readable.
*/
const fixed fixed::_1_2(1, 2);		// 1/2
const fixed fixed::_1_3(1, 3);		// 1/3
const fixed fixed::_1_4(1, 4);		// 1/4
const fixed fixed::_3_4(3, 4);		// 3/4
const fixed fixed::_2_3(2, 3);		// 2/3


fixed::fixed(int numerator, int denominator)
{
	if (denominator == 0) {
		Data.Raw = 0;
	} else {
		Data.Raw = (unsigned short)((unsigned)(numerator * 256) / (unsigned)denominator);
	}
}


/***********************************************************************************************
 * fixed::fixed -- Constructor for fixed integral from ASCII initializer.                      *
 *                                                                                             *
 *    This will parse the ASCII initialization string into a fixed point number.               *
 *    The source string can be a conventional fixed point representation (e.g., "1.0", ".25")  *
 *    or a percent value (e.g. "100%", "25%", "150%"). For percent values, the trailing "%"    *
 *    is required.                                                                             *
 *                                                                                             *
 * INPUT:   ascii -- Pointer to the ascii source to translate into a fixed point number.       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   It is possible to specify an ASCII string that has more precision and           *
 *             magnitude than can be represented by the fixed point number. In such a case,    *
 *             the resulting value is undefined.                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
fixed::fixed(char const * ascii)
{
	/*
	**	If there is no valid pointer, then default to zero value. This takes care of any
	**	compiler confusion that would call this routine when the programmer wanted the
	**	integer parameter constructor to be called.
	*/
	if (ascii == NULL) {
		Data.Raw = 0;
		return;
	}

	/*
	**	The whole part (if any) always starts with the first legal characters.
	*/
	char const * wholepart = ascii;

	/*
	**	Skip any leading white space.
	*/
	while (isspace(*ascii)) {
		ascii++;
	}

	/*
	**	Determine if the number is expressed as a percentage. Detect this by
	**	seeing if there is a trailing "%" character.
	*/
	char const * tptr = ascii;
	while (isdigit(*tptr)) {
		tptr++;
	}

	/*
	**	Percentage value is specified as a whole number but is presumed to be
	**	divided by 100 to get mathematical fixed point percentage value.
	*/
	if (*tptr == '%') {
		Data.Raw = (unsigned short)((atoi(ascii) * 256) / 100);
	} else {

		Data.Composite.Whole = Data.Composite.Fraction = 0;
		if (wholepart && *wholepart != '.') {
			Data.Composite.Whole = (unsigned char)atoi(wholepart);
		}

		char const * fracpart = strchr(ascii, '.');
		if (fracpart) fracpart++;
		if (fracpart) {
			int frac = atoi(fracpart);

//			int len = 0;
			int base = 1;
			char const * fptr = fracpart;
			while (isdigit(*fptr)) {
				fptr++;
//				len++;
				base *= 10;
			}

			Data.Composite.Fraction = (unsigned char)((256 * frac) / base);
		}
	}
}


/***********************************************************************************************
 * fixed::To_ASCII -- Convert a fixed point number into an ASCII string.                       *
 *                                                                                             *
 *    Use this routine to convert this fixed point number into an ASCII null terminated        *
 *    string. This is the counterpart to the fixed point constructor that takes an ASCII       *
 *    string.                                                                                  *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the buffer to hold the fixed point ASCII string.            *
 *                                                                                             *
 *          maxlen   -- The length of the buffer.                                              *
 *                                                                                             *
 * OUTPUT:  Returns with the number of characters placed in the buffer. The trailing null is   *
 *          not counted in this total.                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int fixed::To_ASCII(char * buffer, int maxlen) const
{
	if (buffer == NULL) return(0);

	/*
	**	Determine the whole and fractional parts of the number. The fractional
	**	part number is the value in 1000ths.
	*/
	int whole = Data.Composite.Whole;
	int frac = ((int)Data.Composite.Fraction * 1000) / 256;
	char tbuffer[32];

	/*
	**	If there number consists only of a whole part, then the number is simply
	**	printed into the buffer. If there is a fractional part, then there
	**	will be a decimal place followed by up to three digits of accuracy for the
	**	fractional component.
	*/
	if (frac == 0) {
		sprintf(tbuffer, "%d", whole);
	} else {
		sprintf(tbuffer, "%d.%02d", whole, frac);

		char * ptr = &tbuffer[strlen(tbuffer)-1];
		while (*ptr == '0') {
			*ptr = '\0';
			ptr--;
		}
	}

	/*
	**	If no maximum length to the output buffer was specified, then presume the
	**	output buffer is just long enough to store the number and the trailing
	**	zero.
	*/
	if (maxlen == -1) {
		maxlen = strlen(tbuffer)+1;
	}

	/*
	**	Fill the output buffer with the ASCII number.
	*/
	strncpy(buffer, tbuffer, maxlen);

	/*
	**	Return with the number of ASCII characters placed into the output buffer.
	*/
	int len = strlen(tbuffer);
	if (len < maxlen-1) return(len);
	return(maxlen-1);
}


/***********************************************************************************************
 * fixed::As_ASCII -- Returns a pointer (static) of this number as an ASCII string.            *
 *                                                                                             *
 *    This number will be converted into an ASCII string (using a static buffer) and the       *
 *    string pointer will be returned.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the ASCII representation of this fixed point number.     *
 *                                                                                             *
 * WARNINGS:   As with all static return pointers, the pointer is valid only until such time   *
 *             as this routine is called again.                                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * fixed::As_ASCII(void) const
{
	static char buffer[32];

	To_ASCII(buffer, sizeof(buffer));
	return(buffer);
}
