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
 *                     $Archive:: /Commando/Library/_xmouse.h                                 $* 
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

#ifndef _XMOUSE_H
#define _XMOUSE_H

#include	"xmouse.h"

extern Mouse * MouseCursor;



inline void Hide_Mouse(void) {MouseCursor->Hide_Mouse();}
inline void Show_Mouse(void) {MouseCursor->Show_Mouse();}
inline void Conditional_Hide_Mouse(Rect rect) {MouseCursor->Conditional_Hide_Mouse(rect);}
inline void Conditional_Show_Mouse(void) {MouseCursor->Conditional_Show_Mouse();}
inline int Get_Mouse_State(void) {return(MouseCursor->Get_Mouse_State());}
inline void Set_Mouse_Cursor(int hotx, int hoty, ShapeSet const * cursor, int shape) {MouseCursor->Set_Cursor(hotx, hoty, cursor, shape);}
inline int Get_Mouse_X(void) {return(MouseCursor->Get_Mouse_X());}
inline int Get_Mouse_Y(void) {return(MouseCursor->Get_Mouse_Y());}


#endif
