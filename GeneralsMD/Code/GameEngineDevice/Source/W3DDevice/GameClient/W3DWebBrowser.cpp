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

///// W3DWebBrowser.cpp ///////////////////
// July 2002, Bryan Cleveland

#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "WW3D2/texture.h"
#include "WW3D2/textureloader.h"
#include "WW3D2/surfaceclass.h"
#include "GameClient/Image.h"
#include "GameClient/GameWindow.h"
#include "vector2i.h"
#include "WW3D2/dx8wrapper.h"
// #include "WW3D2/dx8webbrowser.h // Phase 39.4: Removed with DirectX 8 cleanup"

#ifdef __GNUC__
#else
#endif

// Non-Windows: Web browser functionality not available (Windows COM API)
// No implementation needed - class is not defined on non-Windows platforms


