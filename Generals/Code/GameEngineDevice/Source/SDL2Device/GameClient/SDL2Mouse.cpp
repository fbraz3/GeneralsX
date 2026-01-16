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
#include "GameClient/Mouse.h"
#include "Common/Debug.h"
#include <cmath>

// Global instance
SDL2Mouse *TheSDL2Mouse = NULL;

#define DOUBLE_CLICK_TIMEOUT_MS 300
#define DOUBLE_CLICK_MAX_DISTANCE 10

//-------------------------------------------------------------------------------------------------
/** Constructor - Initialize mouse state */
//-------------------------------------------------------------------------------------------------
SDL2Mouse::SDL2Mouse()
	: m_currentX(0)
	, m_currentY(0)
	, m_previousX(0)
	, m_previousY(0)
	, m_leftState(MBS_None)
	, m_rightState(MBS_None)
	, m_middleState(MBS_None)
	, m_leftButtonDown(false)
	, m_rightButtonDown(false)
	, m_middleButtonDown(false)
	, m_leftClickTime(0)
	, m_rightClickTime(0)
	, m_middleClickTime(0)
	, m_leftClickX(0)
	, m_leftClickY(0)
	, m_rightClickX(0)
	, m_rightClickY(0)
	, m_middleClickX(0)
	, m_middleClickY(0)
	, m_wheelDelta(0)
{
	DEBUG_LOG(("SDL2Mouse initialized"));
}

//-------------------------------------------------------------------------------------------------
/** Destructor */
//-------------------------------------------------------------------------------------------------
SDL2Mouse::~SDL2Mouse()
{
	DEBUG_LOG(("SDL2Mouse destroyed"));
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL mouse button down event */
//-------------------------------------------------------------------------------------------------
void SDL2Mouse::onMouseButtonDown(const SDL_MouseButtonEvent &event)
{
	UnsignedInt currentTime = event.timestamp;

	switch (event.button)
	{
		case SDL_BUTTON_LEFT:
			{
				// Check for double-click
				MouseButtonState newState = checkForDoubleClick(currentTime, m_leftClickTime,
				                                                 event.x, event.y, m_leftClickX, m_leftClickY);
				m_leftState = (newState == MBS_DoubleClick) ? MBS_DoubleClick : MBS_Down;
				m_leftButtonDown = true;
				break;
			}

		case SDL_BUTTON_RIGHT:
			{
				MouseButtonState newState = checkForDoubleClick(currentTime, m_rightClickTime,
				                                                 event.x, event.y, m_rightClickX, m_rightClickY);
				m_rightState = (newState == MBS_DoubleClick) ? MBS_DoubleClick : MBS_Down;
				m_rightButtonDown = true;
				break;
			}

		case SDL_BUTTON_MIDDLE:
			{
				MouseButtonState newState = checkForDoubleClick(currentTime, m_middleClickTime,
				                                                 event.x, event.y, m_middleClickX, m_middleClickY);
				m_middleState = (newState == MBS_DoubleClick) ? MBS_DoubleClick : MBS_Down;
				m_middleButtonDown = true;
				break;
			}

		default:
			break;
	}

	// Update mouse position
	m_previousX = m_currentX;
	m_previousY = m_currentY;
	m_currentX = event.x;
	m_currentY = event.y;
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL mouse button up event */
//-------------------------------------------------------------------------------------------------
void SDL2Mouse::onMouseButtonUp(const SDL_MouseButtonEvent &event)
{
	switch (event.button)
	{
		case SDL_BUTTON_LEFT:
			m_leftButtonDown = false;
			if (m_leftState != MBS_DoubleClick)  // Keep double-click state for one frame
				m_leftState = MBS_Up;
			break;

		case SDL_BUTTON_RIGHT:
			m_rightButtonDown = false;
			if (m_rightState != MBS_DoubleClick)
				m_rightState = MBS_Up;
			break;

		case SDL_BUTTON_MIDDLE:
			m_middleButtonDown = false;
			if (m_middleState != MBS_DoubleClick)
				m_middleState = MBS_Up;
			break;

		default:
			break;
	}

	// Update mouse position
	m_previousX = m_currentX;
	m_previousY = m_currentY;
	m_currentX = event.x;
	m_currentY = event.y;
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL mouse motion event */
//-------------------------------------------------------------------------------------------------
void SDL2Mouse::onMouseMotion(const SDL_MouseMotionEvent &event)
{
	m_previousX = m_currentX;
	m_previousY = m_currentY;
	m_currentX = event.x;
	m_currentY = event.y;
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL mouse wheel event */
//-------------------------------------------------------------------------------------------------
void SDL2Mouse::onMouseWheel(const SDL_MouseWheelEvent &event)
{
	// SDL_MOUSEWHEEL: positive Y is scroll up (away), negative is scroll down (toward)
	// Engine convention: positive is up/away, negative is down/toward
	// Each SDL wheel click typically represents 120 units (matching Windows WHEEL_DELTA)
	m_wheelDelta = event.y * MOUSE_WHEEL_DELTA;
}

//-------------------------------------------------------------------------------------------------
/** Check if this is a double-click event */
//-------------------------------------------------------------------------------------------------
MouseButtonState SDL2Mouse::checkForDoubleClick(UnsignedInt currentTime, UnsignedInt &lastClickTime,
                                                 Int x, Int y, Int &lastX, Int &lastY)
{
	// Calculate distance from last click
	Int deltaX = x - lastX;
	Int deltaY = y - lastY;
	Int distanceSquared = deltaX * deltaX + deltaY * deltaY;

	// Check if within double-click timeout and distance
	UnsignedInt timeDelta = currentTime - lastClickTime;
	if (timeDelta <= DOUBLE_CLICK_TIMEOUT_MS && distanceSquared <= (DOUBLE_CLICK_MAX_DISTANCE * DOUBLE_CLICK_MAX_DISTANCE))
	{
		lastClickTime = 0;  // Reset to prevent triple-clicks
		return MBS_DoubleClick;
	}

	// Record this click
	lastClickTime = currentTime;
	lastX = x;
	lastY = y;
	return MBS_Down;
}

//-------------------------------------------------------------------------------------------------
/** Get mouse data in MouseIO format for engine consumption */
//-------------------------------------------------------------------------------------------------
void SDL2Mouse::getMouseData(MouseIO *outMouse)
{
	if (!outMouse)
		return;

	// Position
	outMouse->pos.x = m_currentX;
	outMouse->pos.y = m_currentY;
	outMouse->deltaPos.x = m_currentX - m_previousX;
	outMouse->deltaPos.y = m_currentY - m_previousY;

	// Timestamp (SDL uses milliseconds since SDL_Init)
	outMouse->time = SDL_GetTicks();

	// Button states
	outMouse->leftState = m_leftState;
	outMouse->rightState = m_rightState;
	outMouse->middleState = m_middleState;

	// Wheel
	outMouse->wheelPos = m_wheelDelta;

	// Events (simplified: state is the event for this frame)
	outMouse->leftEvent = (m_leftState != MBS_None) ? 1 : 0;
	outMouse->rightEvent = (m_rightState != MBS_None) ? 1 : 0;
	outMouse->middleEvent = (m_middleState != MBS_None) ? 1 : 0;

	// Reset wheel delta after reading (it's transient)
	m_wheelDelta = 0;

	// Clear Up states after one frame
	if (m_leftState == MBS_Up)
		m_leftState = MBS_None;
	if (m_rightState == MBS_Up)
		m_rightState = MBS_None;
	if (m_middleState == MBS_Up)
		m_middleState = MBS_None;
	if (m_leftState == MBS_DoubleClick && !m_leftButtonDown)
		m_leftState = MBS_None;
	if (m_rightState == MBS_DoubleClick && !m_rightButtonDown)
		m_rightState = MBS_None;
	if (m_middleState == MBS_DoubleClick && !m_middleButtonDown)
		m_middleState = MBS_None;
}
