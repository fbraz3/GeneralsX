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
 *                     $Archive:: /Commando/Library/trackxy.h                                 $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef TRACKXY_H
#define TRACKXY_H

#ifdef NEVER
/*
**	This class is used to keep track of a "current XY location". The Surface class uses this, but
**	it can be used for any such purpose.
*/
class TrackXY {
	public:
		TrackXY(void) : X(0), Y(0) {}

		/*
		**	It is often convenient to have a "current location" for a surface. The
		**	use of this location is arbitrary and outside the scope of this class.
		*/
		void Set(int x, int y) {X = x;Y = y;}
		int Get_X(void) const {return(X);}
		int Get_Y(void) const {return(Y);}

	protected:

		/*
		**	Keeps track of the current location on this surface. The use of this
		**	current location is outside the scope of this class, but it can be quite
		**	useful for other support functions.
		*/
		int X;
		int Y;
};
#endif

#endif
