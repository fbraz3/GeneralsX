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
//
//  FILE: SDL2Mouse.cpp
//
//  SDL2 Mouse Input Implementation (STUB FOR PHASE 02)
//  Full implementation deferred to Phase 03
//
//  Phase 02: Entry Point & Event Loop - Stub only for compilation
//  Phase 03: Full SDL mouse input handling
//
//  Author: Phase 02 (SDL2 Port)
//
///////////////////////////////////////////////////////////////////////////////

#include "SDL2Device/GameClient/SDL2Mouse.h"
#include "Common/Debug.h"

// Global instance
SDL2Mouse *TheSDL2Mouse = NULL;

//-------------------------------------------------------------------------------------------------
/** Constructor */
//-------------------------------------------------------------------------------------------------
SDL2Mouse::SDL2Mouse()
{
	DEBUG_LOG(("SDL2Mouse created (Phase 02 stub)"));
}

//-------------------------------------------------------------------------------------------------
/** Destructor */
//-------------------------------------------------------------------------------------------------
SDL2Mouse::~SDL2Mouse()
{
	DEBUG_LOG(("SDL2Mouse destroyed"));
}
