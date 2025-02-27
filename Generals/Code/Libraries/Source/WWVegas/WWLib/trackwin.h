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
 *                     $Archive:: /Commando/Code/wwlib/trackwin.h                             $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 11/28/00 2:44p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TRACKWIN_H
#define TRACKWIN_H


#ifdef NEVER

#include	"trect.h"

/*
**	This class is used to keep track of a subwindow within a larger window.
*/
class TrackWindow {
	public:
		TrackWindow(int width, int height) : Window(0, 0, width, height), FullWindow(0, 0, width, height) {}

		/*
		**	Simple sub-window manipulation.
		*/
		void Set(Rect const & rect) {Window = rect;if (FullWindow.Width==0) FullWindow = rect;}
		void Reset(void) {Window = Full_Rect();}

		/*
		**	Query the sub-window data.
		*/
		int Get_X(void) const {return(Window.X);}
		int Get_Y(void) const {return(Window.Y);}
		int Get_Width(void) const {return(Window.Width);}
		int Get_Height(void) const {return(Window.Height);}
		Rect Get_Rect(void) const {return(Window);}

		/*
		**	Query the full window data.
		*/
		int Full_Width(void) const {return(FullWindow.Width);}
		int Full_Height(void) const {return(FullWindow.Height);}
		Rect Full_Rect(void) const {return(FullWindow);}

	protected:

		/*
		**	This is the sub-window dimensions.
		*/
		Rect Window;

		/*
		**	This holds the full sized window. It is used for resetting the window
		**	and for maximum window dimension clipping.
		*/
		Rect FullWindow;
};
#endif

#endif
