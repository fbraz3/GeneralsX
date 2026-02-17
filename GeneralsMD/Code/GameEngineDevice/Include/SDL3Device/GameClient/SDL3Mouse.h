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
** SDL3Mouse.h
**
** SDL3-based mouse implementation for Linux builds.
**
** TheSuperHackers @feature CnC_Generals_Linux 10/02/2026 Bender
** Replaces Win32Mouse/Win32DIMouse with SDL3 mouse APIs for Linux.
*/

#pragma once

#ifndef _WIN32

// SYSTEM INCLUDES
#include <SDL3/SDL.h>

// USER INCLUDES
#include "GameClient/Mouse.h"

// SDL3Mouse ------------------------------------------------------------------
/** Mouse interface using SDL3 APIs for Linux */
//-----------------------------------------------------------------------------
class SDL3Mouse : public Mouse
{
public:
	SDL3Mouse(SDL_Window* window);
	virtual ~SDL3Mouse(void);

	// SubsystemInterface
	virtual void init(void);
	virtual void reset(void);
	virtual void update(void);
	virtual void initCursorResources(void);

	// Mouse interface
	virtual void setCursor(MouseCursor cursor);
	virtual void setVisibility(Bool visible);
	virtual void loseFocus();
	virtual void regainFocus();

	// SDL3-specific methods
	// Fighter19 pattern: addSDLEvent() accepts raw SDL_Event directly
	// GeneralsX @refactor felipebraz 16/02/2026 Switch to fighter19 event model
	void addSDLEvent(SDL_Event *event);
	
	// Legacy methods (kept for compatibility, will be removed after validation)
	void addSDL3MouseMotionEvent(const SDL_MouseMotionEvent& event);
	void addSDL3MouseButtonEvent(const SDL_MouseButtonEvent& event);
	void addSDL3MouseWheelEvent(const SDL_MouseWheelEvent& event);

protected:
	virtual void capture(void);
	virtual void releaseCapture(void);
	virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush);

private:
	// Event translation from SDL_Event (raw format)
	// GeneralsX @refactor felipebraz 16/02/2026 Unified translation method
	void translateEvent(UnsignedInt eventIndex, MouseIO *result);

	// Legacy translation methods (kept for compatibility)
	void translateMotionEvent(const SDL_MouseMotionEvent& event, MouseIO *result);
	void translateButtonEvent(const SDL_MouseButtonEvent& event, MouseIO *result);
	void translateWheelEvent(const SDL_MouseWheelEvent& event, MouseIO *result);

	// SDL3 event buffer - Fighter19 pattern: raw SDL_Event array with sentinels
	// GeneralsX @refactor felipebraz 16/02/2026 Use raw SDL_Event buffer like fighter19
	static const UnsignedInt MAX_SDL3_MOUSE_EVENTS = 128;
	
	SDL_Event m_eventBuffer[MAX_SDL3_MOUSE_EVENTS];
	UnsignedInt m_nextFreeIndex;  // Write position (insert new events here)
	UnsignedInt m_nextGetIndex;   // Read position (retrieve events from here)

	SDL_Window* m_Window;
	Bool m_IsCaptured;
	Bool m_IsVisible;
	
	// Track button states for click detection
	Uint32 m_LeftButtonDownTime;
	Uint32 m_RightButtonDownTime;
	Uint32 m_MiddleButtonDownTime;
	
	ICoord2D m_LeftButtonDownPos;
	ICoord2D m_RightButtonDownPos;
	ICoord2D m_MiddleButtonDownPos;
};

#endif // !_WIN32
