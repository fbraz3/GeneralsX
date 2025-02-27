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

/* $Header: /Commando/Library/FIXED.H 1     7/22/97 12:00p Greg_h $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FIXED.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 06/19/96                                                     *
 *                                                                                             *
 *                  Last Update : June 19, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef FIXED_H
#define FIXED_H

#include "bool.h"
//#pragma warning 604 9
//#pragma warning 595 9

/*
**	This is a very simple fixed point class that functions like a regular integral type. However
**	it is under certain restrictions. The whole part must not exceed 255. The fractional part is
**	limited to an accuracy of 1/256. It cannot represent or properly handle negative values. It
**	really isn't all that fast (if an FPU is guaranteed to be present than using "float" might be
**	more efficient). It doesn't detect overflow or underflow in mathematical or bit-shift operations.
**
**	Take careful note that the normal mathematical operators return integers and not fixed point
**	values if either of the components is an integer. This is the normal C auto-upcasting rule
**	as it would apply presuming that integers are considered to be of higher precision than
**	fixed point numbers. This allows the result of these operators to generate values with greater
**	magnitude than is normally possible if the result were coerced into a fixed point number.
**	If the result should be fixed point, then ensure that both parameters are fixed point.
**
**	Note that although integers are used as the parameters in the mathematical operators, this
**	does not imply that negative parameters are supported. The use of integers is as a convenience
**	to the programmer -- constant integers are presumed signed. If unsigned parameters were
**	specified, then the compiler would have ambiguous conversion situation in the case of constant
** integers (e.g. 1, 10, 32, etc). This is most important for the constructor when dealing with the
**	"0" parameter case. In that situation the compiler might interpret the "0" as a null pointer rather
**	than an unsigned integer. There should be no adverse consequences of using signed integer parameters
**	since the precision/magnitude of these integers far exceeds the fixed point component counterparts.
**
**	Note that when integer values are returns from the arithmetic operators, the value is rounded
**	to the nearest whole integer value. This differs from normal integer math that always rounds down.
*/
class fixed
{
	public:
		// The default constructor must not touch the data members in any way.
		fixed(void) {}

		// Convenient constructor if numerator and denominator components are known.
		fixed(int numerator, int denominator);

		// Conversion constructor to get fixed point from integer.
		fixed(int value) {Data.Composite.Fraction = 0;Data.Composite.Whole = (unsigned char)value;}

		// Constructor if ASCII image of number is known.
		fixed(char const * ascii);

		// Convert to integer when implicitly required.
		operator unsigned (void) const {return(((unsigned)Data.Raw+(256/2)) / 256);}

		/*
		**	The standard operators as they apply to in-place operation.
		*/
		fixed & operator *= (fixed const & rvalue) {Data.Raw = (unsigned short)(((int)Data.Raw * rvalue.Data.Raw) / 256);return(*this);}
		fixed & operator *= (int rvalue) {Data.Raw = (unsigned short)(Data.Raw * rvalue);return(*this);}
		fixed & operator /= (fixed const & rvalue) {if (rvalue.Data.Raw != 0 && rvalue.Data.Raw != 256) Data.Raw = (unsigned short)(((int)Data.Raw * 256) / rvalue);return(*this);}
		fixed & operator /= (int rvalue) {if (rvalue) Data.Raw = (unsigned short)((unsigned)Data.Raw / rvalue);return(*this);}
		fixed & operator += (fixed const & rvalue) {Data.Raw += rvalue.Data.Raw;return(*this);}
		fixed & operator -= (fixed const & rvalue) {Data.Raw -= rvalue.Data.Raw;return(*this);}

		/*
		**	The standard "My Dear Aunt Sally" operators. The integer versions of multiply
		**	and divide are more efficient than using the fixed point counterparts.
		*/
//		const fixed operator * (fixed const & rvalue) const {return(fixed(*this) *= rvalue);}
		const fixed operator * (fixed const & rvalue) const {fixed temp = *this;temp.Data.Raw = (unsigned short)(((int)temp.Data.Raw * (int)rvalue.Data.Raw) / 256);return(temp);}
		const int operator * (int rvalue) const {return ((((unsigned)Data.Raw * rvalue) + (256/2)) / 256);}
//		const fixed operator / (fixed const & rvalue) const {return(fixed(*this) /= rvalue);}
		const fixed operator / (fixed const & rvalue) const {fixed temp = *this;if (rvalue.Data.Raw != 0 && rvalue.Data.Raw != 256) temp.Data.Raw = (unsigned short)(((int)temp.Data.Raw * 256) / rvalue.Data.Raw);return(temp);}
		const int operator / (int rvalue) const {if (rvalue) return(((unsigned)Data.Raw+(256/2)) / ((unsigned)rvalue*256));return(*this);}
//		const fixed operator + (fixed const & rvalue) const {return(fixed(*this) += rvalue);}
		const fixed operator + (fixed const & rvalue) const {fixed temp = *this;temp += rvalue;return(temp);}
		const int operator + (int rvalue) const {return((((unsigned)Data.Raw+(256/2))/256) + rvalue);}
//		const fixed operator - (fixed const & rvalue) const {return(fixed(*this) -= rvalue);}
		const fixed operator - (fixed const & rvalue) const {fixed temp = *this;temp -= rvalue;return(temp);}
		const int operator - (int rvalue) const {return((((unsigned)Data.Raw+(256/2))/256) - rvalue);}

		/*
		**	The Shift operators are more efficient than using multiplies or divides by power-of-2 numbers.
		*/
		fixed & operator >>= (unsigned rvalue) {Data.Raw >>= rvalue;return(*this);}
		fixed & operator <<= (unsigned rvalue) {Data.Raw <<= rvalue;return(*this);}
		const fixed operator >> (unsigned rvalue) const {fixed temp = *this;temp >>= rvalue;return(temp);}
		const fixed operator << (unsigned rvalue) const {fixed temp = *this;temp <<= rvalue;return(temp);}

		/*
		**	The full set of comparison operators.
		*/
		bool operator == (fixed const & rvalue) const {return(Data.Raw == rvalue.Data.Raw);}
		bool operator != (fixed const & rvalue) const {return(Data.Raw != rvalue.Data.Raw);}
		bool operator < (fixed const & rvalue) const {return(Data.Raw < rvalue.Data.Raw);}
		bool operator > (fixed const & rvalue) const {return(Data.Raw > rvalue.Data.Raw);}
		bool operator <= (fixed const & rvalue) const {return(Data.Raw <= rvalue.Data.Raw);}
		bool operator >= (fixed const & rvalue) const {return(Data.Raw >= rvalue.Data.Raw);}
		bool operator ! (void) const {return(Data.Raw == 0);}

		/*
		**	Comparison to integers requires consideration of fractional component.
		*/
		bool operator < (int rvalue) const {return(Data.Raw < (rvalue*256));}
		bool operator > (int rvalue) const {return(Data.Raw > (rvalue*256));}
		bool operator <= (int rvalue) const {return(Data.Raw <= (rvalue*256));}
		bool operator >= (int rvalue) const {return(Data.Raw >= (rvalue*256));}
		bool operator == (int rvalue) const {return(Data.Raw == (rvalue*256));}
		bool operator != (int rvalue) const {return(Data.Raw != (rvalue*256));}

		/*
		**	Friend functions to handle the alternate positioning of fixed and integer parameters.
		*/
		friend const int operator * (int lvalue, fixed const & rvalue) {return(rvalue * lvalue);}
		friend const int operator / (int lvalue, fixed const & rvalue) {if (rvalue.Data.Raw == 0 || rvalue.Data.Raw == 256) return (lvalue); return(((unsigned)(lvalue * 256)+(256/2)) / rvalue.Data.Raw);}
		friend const int operator + (int lvalue, fixed const & rvalue) {return(rvalue + lvalue);}
		friend const int operator - (int lvalue, fixed const & rvalue) {return((((lvalue*256) - rvalue.Data.Raw) + (256/2)) / 256);}
		friend bool operator < (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) < rvalue.Data.Raw);}
		friend bool operator > (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) > rvalue.Data.Raw);}
		friend bool operator <= (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) <= rvalue.Data.Raw);}
		friend bool operator >= (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) >= rvalue.Data.Raw);}
		friend bool operator == (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) == rvalue.Data.Raw);}
		friend bool operator != (unsigned lvalue, fixed const & rvalue) {return((lvalue*256) != rvalue.Data.Raw);}
		friend int operator *= (int & lvalue, fixed const & rvalue) {lvalue = lvalue * rvalue;return(lvalue);}
		friend int operator /= (int & lvalue, fixed const & rvalue) {lvalue = lvalue / rvalue;return(lvalue);}
		friend int operator += (int & lvalue, fixed const & rvalue) {lvalue = lvalue + rvalue;return(lvalue);}
		friend int operator -= (int & lvalue, fixed const & rvalue) {lvalue = lvalue - rvalue;return(lvalue);}

		/*
		**	Helper functions to handle simple and common operations on fixed point numbers.
		*/
		void Round_Up(void) {Data.Raw += (unsigned short)(256-1);Data.Composite.Fraction = 0;}
		void Round_Down(void) {Data.Composite.Fraction = 0;}
		void Round(void) {if (Data.Composite.Fraction >= 256/2) Round_Up();Round_Down();}
		void Saturate(unsigned capvalue) {if (Data.Raw > (capvalue*256)) Data.Raw = (unsigned short)(capvalue*256);}
		void Saturate(fixed const & capvalue) {if (*this > capvalue) *this = capvalue;}
		void Sub_Saturate(unsigned capvalue) {if (Data.Raw >= (capvalue*256)) Data.Raw = (unsigned short)((capvalue*256)-1);}
		void Sub_Saturate(fixed const & capvalue) {if (*this >= capvalue) Data.Raw = (unsigned short)(capvalue.Data.Raw-1);}
		void Inverse(void) {*this = fixed(1) / *this;}

		/*
		**	Friend helper functions that work in the typical C fashion of passing the object to
		**	be processed as a parameter to the function.
		*/
		friend const fixed Round_Up(fixed const & value) {fixed temp = value; temp.Round_Up();return(temp);}
		friend const fixed Round_Down(fixed const & value) {fixed temp = value; temp.Round_Down();return(temp);}
		friend const fixed Round(fixed const & value) {fixed temp = value; temp.Round();return(temp);}
		friend const fixed Saturate(fixed const & value, unsigned capvalue) {fixed temp = value;temp.Saturate(capvalue);return(temp);}
		friend const fixed Saturate(fixed const & value, fixed const & capvalue) {fixed temp = value;temp.Saturate(capvalue);return(temp);}
		friend const fixed Sub_Saturate(fixed const & value, unsigned capvalue) {fixed temp = value;temp.Sub_Saturate(capvalue);return(temp);}
		friend const fixed Sub_Saturate(fixed const & value, fixed const & capvalue) {fixed temp = value;temp.Sub_Saturate(capvalue);return(temp);}
		friend const fixed Inverse(fixed const & value) {fixed temp = value;temp.Inverse();return(temp);}

		/*
		**	Conversion of the fixed point number into an ASCII string.
		*/
		int To_ASCII(char * buffer, int maxlen=-1) const;
		char const * As_ASCII(void) const;

		/*
		**	Helper constants that provide some convenient fixed point values.
		*/
		static const fixed _1_2;
		static const fixed _1_3;
		static const fixed _1_4;
		static const fixed _3_4;
		static const fixed _2_3;

	private:
		union {
			struct {
#ifdef BIG_ENDIAN
				unsigned char Whole;
				unsigned char Fraction;
#else
				unsigned char Fraction;
				unsigned char Whole;
#endif
			} Composite;
			unsigned short Raw;
		} Data;
};


#endif
