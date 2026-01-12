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

#pragma once

////////////////////////////////////////////////////////////////////////////////
//  SDL2Mouse.h - SDL2 Mouse Input Implementation
//
//  Stub for Phase 02 - Full implementation in Phase 03
//  Handles SDL2 mouse events and input injection
//
//  Phase 02: Entry Point (stub only)
//  Phase 03: Full implementation with input injection
//
//  Author: Phase 02 (SDL2 Port)
////////////////////////////////////////////////////////////////////////////////

// Forward declaration
class Mouse;

/**
 * SDL2Mouse - SDL2-based mouse input handler
 *
 * PHASE 02: Stub class for compilation
 * PHASE 03: Full implementation with SDL_MOUSEBUTTONDOWN/UP injection
 */
class SDL2Mouse
{
public:
	SDL2Mouse();
	~SDL2Mouse();

	// Stub methods for Phase 02 compatibility
	// Full implementation in Phase 03
};

// Global instance (will be created in Phase 03)
extern SDL2Mouse *TheSDL2Mouse;

#endif // SDL2MOUSE_H
