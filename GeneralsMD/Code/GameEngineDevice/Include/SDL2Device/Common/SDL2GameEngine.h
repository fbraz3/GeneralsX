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
//  SDL2GameEngine.h - SDL2 Platform Game Engine
//
//  Implements GameEngine for SDL2-based platforms (macOS, Linux, etc.)
//  Handles SDL2 event polling and window lifecycle management
//
//  Phase 02: SDL2 Application Event Loop
//
//  Author: Phase 02 (SDL2 Port)
////////////////////////////////////////////////////////////////////////////////

#include "Common/GameEngine.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

// CONSTANTS //////////////////////////////////////////////////////////////////

// TYPES / CLASSES ////////////////////////////////////////////////////////////

/**
 * SDL2GameEngine - SDL2-based implementation of GameEngine
 *
 * This class replaces Win32GameEngine for non-Windows platforms.
 * It handles SDL2 event polling instead of Windows message pump.
 *
 * Key differences from Win32GameEngine:
 * - Uses SDL_PollEvent() instead of PeekMessage/GetMessage
 * - Maps SDL_WindowEvent to engine lifecycle events
 * - Handles minimized window via SDL_GetWindowFlags()
 */
class SDL2GameEngine : public GameEngine
{
public:

	/** Constructor */
	SDL2GameEngine();

	/** Destructor */
	~SDL2GameEngine();

	/** Initialize the game engine */
	void init(void);

	/** Reset the system */
	void reset(void);

	/**
	 * Update the game engine
	 * Called once per frame from GameMain()
	 */
	void update(void);

	/**
	 * Service SDL2 OS events
	 * Processes pending SDL events and translates them to internal engine events
	 */
	void serviceSDL2OS(void);

protected:

	/** Handle SDL window events */
	void handleWindowEvent(const SDL_WindowEvent &event);

	/** Handle SDL quit event */
	void handleQuitEvent(void);

private:

	// No additional state needed for basic event handling
};

#endif // SDL2GAMEENGINE_H
