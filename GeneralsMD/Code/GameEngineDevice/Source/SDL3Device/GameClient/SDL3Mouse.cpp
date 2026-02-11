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

/*
** SDL3Mouse.cpp
**
** SDL3-based mouse implementation for Linux builds.
**
** TheSuperHackers @feature CnC_Generals_Linux 10/02/2026 Bender
** Replaces Win32Mouse/Win32DIMouse with SDL3 mouse APIs for Linux.
*/

#ifndef _WIN32

#include "GameEngineDevice/SDL3Device/GameClient/SDL3Mouse.h"
#include <cstdio>
#include <cstring>

/**
 * Constructor
 *
 * @param window SDL3 window handle (required for mouse capture)
 */
SDL3Mouse::SDL3Mouse(SDL_Window* window)
	: Mouse(),
	  m_Window(window),
	  m_IsCaptured(false),
	  m_IsVisible(true),
	  m_eventHead(0),
	  m_eventTail(0),
	  m_LeftButtonDownTime(0),
	  m_RightButtonDownTime(0),
	  m_MiddleButtonDownTime(0)
{
	fprintf(stderr, "DEBUG: SDL3Mouse::SDL3Mouse() created\n");
	
	m_LeftButtonDownPos.x = 0;
	m_LeftButtonDownPos.y = 0;
	m_RightButtonDownPos.x = 0;
	m_RightButtonDownPos.y = 0;
	m_MiddleButtonDownPos.x = 0;
	m_MiddleButtonDownPos.y = 0;
}

/**
 * Destructor
 */
SDL3Mouse::~SDL3Mouse(void)
{
	releaseCapture();
	fprintf(stderr, "DEBUG: SDL3Mouse::~SDL3Mouse() destroyed\n");
}

/**
 * Initialize mouse subsystem
 */
void SDL3Mouse::init(void)
{
	fprintf(stderr, "INFO: SDL3Mouse::init()\n");
	
	// Call parent init (loads cursor info from INI, etc.)
	Mouse::init();
	
	// Show cursor by default
	SDL_ShowCursor();
	m_IsVisible = true;
	
	// Clear event buffer
	m_eventHead = 0;
	m_eventTail = 0;
	
	fprintf(stderr, "INFO: SDL3Mouse::init() complete\n");
}

/**
 * Reset mouse to default state
 */
void SDL3Mouse::reset(void)
{
	fprintf(stderr, "DEBUG: SDL3Mouse::reset()\n");
	
	Mouse::reset();
	
	releaseCapture();
	SDL_ShowCursor();
	m_IsVisible = true;
	
	// Clear event buffer
	m_eventHead = 0;
	m_eventTail = 0;
}

/**
 * Update mouse state (called per-frame)
 */
void SDL3Mouse::update(void)
{
	// Call parent update (processes events, updates m_currMouse)
	Mouse::update();
}

/**
 * Initialize cursor resources (load cursor images, etc.)
 * For Phase 1.5, stub this - cursor resources loaded later
 */
void SDL3Mouse::initCursorResources(void)
{
	fprintf(stderr, "DEBUG: SDL3Mouse::initCursorResources() - stub (Phase 2)\n");
	// TODO: Phase 2 - Load SDL3 cursor images from files
}

/**
 * Set mouse cursor type
 * For Phase 1.5, just use system arrow cursor
 */
void SDL3Mouse::setCursor(MouseCursor cursor)
{
	// TODO: Phase 2 - Map MouseCursor enum to SDL3 system cursors
	// For now, just use default arrow
	SDL_Cursor* sdlCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	if (sdlCursor) {
		SDL_SetCursor(sdlCursor);
	}
}

/**
 * Set cursor visibility
 */
void SDL3Mouse::setVisibility(Bool visible)
{
	m_IsVisible = visible;
	
	if (visible) {
		SDL_ShowCursor();
	} else {
		SDL_HideCursor();
	}
}

/**
 * Handle window losing focus
 */
void SDL3Mouse::loseFocus()
{
	releaseCapture();
}

/**
 * Handle window regaining focus
 */
void SDL3Mouse::regainFocus()
{
	// Capture may be re-enabled by game logic
}

/**
 * Capture mouse (confine to window)
 */
void SDL3Mouse::capture(void)
{
	if (!m_Window || m_IsCaptured) {
		return;
	}
	
	// SDL3: Capture mouse to window
	SDL_CaptureMouse(true);
	
	// SDL3: Grab mouse (confine to window)
	SDL_SetWindowMouseGrab(m_Window, true);
	
	m_IsCaptured = true;
	
	fprintf(stderr, "DEBUG: SDL3Mouse::capture() - mouse captured\n");
}

/**
 * Release mouse capture
 */
void SDL3Mouse::releaseCapture(void)
{
	if (!m_IsCaptured) {
		return;
	}
	
	SDL_CaptureMouse(false);
	if (m_Window) {
		SDL_SetWindowMouseGrab(m_Window, false);
	}
	
	m_IsCaptured = false;
	
	fprintf(stderr, "DEBUG: SDL3Mouse::releaseCapture() - mouse released\n");
}

/**
 * Get next mouse event from buffer
 * Called by Mouse::update() in parent class
 *
 * @param result Output MouseIO structure
 * @param flush If true, flush all events
 * @return MOUSE_OK if event retrieved, MOUSE_NONE if buffer empty
 */
UnsignedByte SDL3Mouse::getMouseEvent(MouseIO *result, Bool flush)
{
	// Check if buffer is empty
	if (m_eventTail == m_eventHead) {
		return MOUSE_NONE;
	}
	
	// Get event from buffer
	SDL3MouseEvent& evt = m_eventBuffer[m_eventTail];
	
	// Translate SDL3 event to MouseIO
	switch (evt.type) {
		case SDL3MouseEvent::MOTION:
			translateMotionEvent(evt.motion, result);
			break;
		case SDL3MouseEvent::BUTTON:
			translateButtonEvent(evt.button, result);
			break;
		case SDL3MouseEvent::WHEEL:
			translateWheelEvent(evt.wheel, result);
			break;
	}
	
	// Advance tail (circular buffer)
	m_eventTail = (m_eventTail + 1) % MAX_SDL3_MOUSE_EVENTS;
	
	return MOUSE_OK;
}

/**
 * Add SDL3 mouse motion event to buffer
 * Called by SDL3GameEngine::handleMouseMotionEvent()
 */
void SDL3Mouse::addSDL3MouseMotionEvent(const SDL_MouseMotionEvent& event)
{
	int nextHead = (m_eventHead + 1) % MAX_SDL3_MOUSE_EVENTS;
	
	// Check if buffer is full
	if (nextHead == m_eventTail) {
		fprintf(stderr, "WARNING: SDL3Mouse event buffer full (dropped motion event)\n");
		return;
	}
	
	// Add to buffer
	SDL3MouseEvent& evt = m_eventBuffer[m_eventHead];
	evt.type = SDL3MouseEvent::MOTION;
	evt.motion = event;
	
	m_eventHead = nextHead;
}

/**
 * Add SDL3 mouse button event to buffer
 * Called by SDL3GameEngine::handleMouseButtonEvent()
 */
void SDL3Mouse::addSDL3MouseButtonEvent(const SDL_MouseButtonEvent& event)
{
	int nextHead = (m_eventHead + 1) % MAX_SDL3_MOUSE_EVENTS;
	
	if (nextHead == m_eventTail) {
		fprintf(stderr, "WARNING: SDL3Mouse event buffer full (dropped button event)\n");
		return;
	}
	
	SDL3MouseEvent& evt = m_eventBuffer[m_eventHead];
	evt.type = SDL3MouseEvent::BUTTON;
	evt.button = event;
	
	m_eventHead = nextHead;
}

/**
 * Add SDL3 mouse wheel event to buffer
 * Called by SDL3GameEngine::handleMouseWheelEvent()
 */
void SDL3Mouse::addSDL3MouseWheelEvent(const SDL_MouseWheelEvent& event)
{
	int nextHead = (m_eventHead + 1) % MAX_SDL3_MOUSE_EVENTS;
	
	if (nextHead == m_eventTail) {
		fprintf(stderr, "WARNING: SDL3Mouse event buffer full (dropped wheel event)\n");
		return;
	}
	
	SDL3MouseEvent& evt = m_eventBuffer[m_eventHead];
	evt.type = SDL3MouseEvent::WHEEL;
	evt.wheel = event;
	
	m_eventHead = nextHead;
}

/**
 * Translate SDL3 motion event to MouseIO
 */
void SDL3Mouse::translateMotionEvent(const SDL_MouseMotionEvent& event, MouseIO *result)
{
	result->pos.x = (Int)event.x;
	result->pos.y = (Int)event.y;
	result->deltaPos.x = (Int)event.xrel;
	result->deltaPos.y = (Int)event.yrel;
	result->time = event.timestamp;
	
	// No button state change on motion
	result->leftState = MBS_None;
	result->rightState = MBS_None;
	result->middleState = MBS_None;
	result->wheelPos = 0;
}

/**
 * Translate SDL3 button event to MouseIO
 */
void SDL3Mouse::translateButtonEvent(const SDL_MouseButtonEvent& event, MouseIO *result)
{
	result->pos.x = (Int)event.x;
	result->pos.y = (Int)event.y;
	result->deltaPos.x = 0;
	result->deltaPos.y = 0;
	result->time = event.timestamp;
	result->wheelPos = 0;
	
	// Initialize all button states to None
	result->leftState = MBS_None;
	result->rightState = MBS_None;
	result->middleState = MBS_None;
	
	MouseButtonState state = event.down ? MBS_Down : MBS_Up;
	
	// Map SDL3 button to MouseIO button
	switch (event.button) {
		case SDL_BUTTON_LEFT:
			result->leftState = state;
			if (event.down) {
				m_LeftButtonDownTime = event.timestamp;
				m_LeftButtonDownPos.x = (Int)event.x;
				m_LeftButtonDownPos.y = (Int)event.y;
			} else {
				// Check for double-click (within CLICK_SENSITIVITY frames)
				Uint32 deltaTime = event.timestamp - m_LeftButtonDownTime;
				Int deltaX = (Int)event.x - m_LeftButtonDownPos.x;
				Int deltaY = (Int)event.y - m_LeftButtonDownPos.y;
				Int distSq = deltaX * deltaX + deltaY * deltaY;
				
				// TODO: CLICK_SENSITIVITY is in frames, not milliseconds - need conversion
				if (deltaTime < 500 && distSq < CLICK_DISTANCE_DELTA_SQUARED) {
					result->leftState = MBS_DoubleClick;
				}
			}
			break;
			
		case SDL_BUTTON_RIGHT:
			result->rightState = state;
			if (event.down) {
				m_RightButtonDownTime = event.timestamp;
				m_RightButtonDownPos.x = (Int)event.x;
				m_RightButtonDownPos.y = (Int)event.y;
			}
			break;
			
		case SDL_BUTTON_MIDDLE:
			result->middleState = state;
			if (event.down) {
				m_MiddleButtonDownTime = event.timestamp;
				m_MiddleButtonDownPos.x = (Int)event.x;
				m_MiddleButtonDownPos.y = (Int)event.y;
			}
			break;
	}
}

/**
 * Translate SDL3 wheel event to MouseIO
 */
void SDL3Mouse::translateWheelEvent(const SDL_MouseWheelEvent& event, MouseIO *result)
{
	// SDL3 mouse position not provided in wheel event, get current position
	float mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	
	result->pos.x = (Int)mouseX;
	result->pos.y = (Int)mouseY;
	result->deltaPos.x = 0;
	result->deltaPos.y = 0;
	result->time = event.timestamp;
	
	// SDL3 wheel: positive = up/away, negative = down/toward user
	// Multiply by MOUSE_WHEEL_DELTA (120) to match Windows behavior
	result->wheelPos = (Int)(event.y * MOUSE_WHEEL_DELTA);
	
	result->leftState = MBS_None;
	result->rightState = MBS_None;
	result->middleState = MBS_None;
}

#endif // !_WIN32
