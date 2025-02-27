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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/mono.h                                 $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 5/04/01 7:36p                                               $*
 *                                                                                             * 
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef MONO_H
#define MONO_H

#include	"win.h"

class MonoClass {
	public:
      typedef enum MonoAttribute {
			INVISIBLE=0x00,				// Black on black.
			UNDERLINE=0x01,				// Underline.
			BLINKING=0x90,					// Blinking white on black.
			NORMAL=0x02,					// White on black.
			INVERSE=0x70					// Black on white.
      } MonoAttribute;

		MonoClass(void);
		~MonoClass(void);

		static void Enable(void) {Enabled = true;};
		static void Disable(void) {Enabled = false;};
		static bool Is_Enabled(void) {return Enabled;};

		void Sub_Window(int x=0, int y=0, int w=80, int h=25);
		void Fill_Attrib(int x, int y, int w, int h, MonoAttribute attrib);
		void Clear(void);
		void Set_Cursor(int x, int y);
		void Print(char const *text);
		void Print(int text);
		void Printf(char const *text, ...);
		void Printf(int text, ...);
		void Text_Print(char const *text, int x, int y, MonoAttribute attrib=NORMAL);
		void Text_Print(int text, int x, int y, MonoAttribute attrib=NORMAL);
		void View(void);
		void Scroll(int lines=1);
		void Pan(int cols=1);
		void Set_Default_Attribute(MonoAttribute attrib);

		/*
		**	This merely makes a duplicate of the mono object into a newly created mono
		**	object.
		*/
		MonoClass (MonoClass const &);

		static MonoClass * Current;

	private:

		/*
		**	Handle of the mono page.
		*/
		HANDLE Handle;

		/*
		**	If this is true, then monochrome output is allowed. It defaults to false
		**	so that monochrome output must be explicitly enabled.
		*/
		static bool Enabled;

		MonoClass & operator = (MonoClass const & );
};


#endif

