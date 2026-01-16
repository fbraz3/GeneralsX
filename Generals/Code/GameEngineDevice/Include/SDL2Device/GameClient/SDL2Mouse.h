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

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#pragma pack(push, 1)
#include <SDL2/SDL.h>
#pragma pack(pop)

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "GameClient/Mouse.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class Mouse;

/**
 * SDL2Mouse - SDL2-based mouse input handler
 *
 * Handles translation of SDL2 mouse events to engine-level MouseIO structure
 * Used by SDL2GameEngine to inject mouse input into TheMouse system
 */
class SDL2Mouse
{
public:
	SDL2Mouse();
	virtual ~SDL2Mouse();

	// Event handlers called from SDL2GameEngine::serviceSDL2OS()
	void onMouseButtonDown(const SDL_MouseButtonEvent &event);
	void onMouseButtonUp(const SDL_MouseButtonEvent &event);
	void onMouseMotion(const SDL_MouseMotionEvent &event);
	void onMouseWheel(const SDL_MouseWheelEvent &event);

	// Mouse state query methods
	Int getMouseX() const { return m_currentX; }
	Int getMouseY() const { return m_currentY; }
	Bool isLeftButtonDown() const { return m_leftButtonDown; }
	Bool isRightButtonDown() const { return m_rightButtonDown; }
	Bool isMiddleButtonDown() const { return m_middleButtonDown; }

	// Get the accumulated mouse data for engine consumption
	void getMouseData(MouseIO *outMouse);

private:
	// Current mouse position
	Int m_currentX;
	Int m_currentY;
	Int m_previousX;
	Int m_previousY;

	// Button state tracking (Up, Down, or DoubleClick)
	MouseButtonState m_leftState;
	MouseButtonState m_rightState;
	MouseButtonState m_middleState;

	// Raw button tracking for double-click detection
	Bool m_leftButtonDown;
	Bool m_rightButtonDown;
	Bool m_middleButtonDown;

	// Double-click detection variables
	UnsignedInt m_leftClickTime;
	UnsignedInt m_rightClickTime;
	UnsignedInt m_middleClickTime;

	Int m_leftClickX;
	Int m_leftClickY;
	Int m_rightClickX;
	Int m_rightClickY;
	Int m_middleClickX;
	Int m_middleClickY;

	// Wheel state (accumulated delta)
	Int m_wheelDelta;

	// Helper methods
	MouseButtonState checkForDoubleClick(UnsignedInt currentTime, UnsignedInt &lastClickTime, 
	                                      Int x, Int y, Int &lastX, Int &lastY);
	void clearMouseState();
};

// EXTERNALS //////////////////////////////////////////////////////////////////
extern SDL2Mouse *TheSDL2Mouse;

#endif // SDL2MOUSE_H
