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
** SDL3Keyboard.h
**
** SDL3-based keyboard implementation (cross-platform).
**
** TheSuperHackers @feature CnC_Generals_Linux 10/02/2026 Bender
** Replaces Win32DIKeyboard with SDL3 keyboard APIs.
** Enabled when SAGE_USE_SDL3 is ON (cross-platform: Linux and Windows Modern).
*/

#pragma once

#ifdef SAGE_USE_SDL3

// SYSTEM INCLUDES
#include <SDL3/SDL.h>

// USER INCLUDES
#include "GameClient/Keyboard.h"
#include "GameClient/KeyDefs.h"

// TYPE DEFINES ----------------------------------------------------------------
// KeyVal is an alias for KeyDefType (DirectInput key code enum)
typedef KeyDefType KeyVal;

// SDL3Keyboard ---------------------------------------------------------------
/** Keyboard interface using SDL3 APIs for Linux */
//-----------------------------------------------------------------------------
class SDL3Keyboard : public Keyboard
{
public:
	SDL3Keyboard(void);
	virtual ~SDL3Keyboard(void);

	// SubsystemInterface
	virtual void init(void);
	virtual void reset(void);
	virtual void update(void);

	// Keyboard interface
	virtual KeyboardIO *getKeyboard(void);
	virtual Bool getCapsState(void);  // GeneralsX @build fbraz 12/02/2026 BenderAI - Caps Lock state
	
	// SDL3-specific methods - Fighter19 pattern
	// GeneralsX @refactor felipebraz 16/02/2026 Add direct addSDLEvent() method
	void addSDLEvent(SDL_Event *event);
	
	// Legacy method (kept for compatibility)
	void addSDL3KeyEvent(const SDL_KeyboardEvent& event);

protected:
	virtual void getKey(KeyboardIO *key);  // GeneralsX @build fbraz 12/02/2026 BenderAI - Get keyboard event
	virtual KeyVal translateScanCodeToKeyVal(unsigned char scan);

private:
	void translateKeyEvent(const SDL_KeyboardEvent& event);
	
	// SDL3 event buffer - Fighter19 pattern
	// GeneralsX @refactor felipebraz 16/02/2026 Use raw SDL_Event buffer with sentinel
	// GeneralsX @bugfix felipebraz 18/02/2026 Increase to 256 to match Mouse::NUM_MOUSE_EVENTS
	static const UnsignedInt MAX_SDL3_KEY_EVENTS = 256;
	
	SDL_Event m_eventBuffer[MAX_SDL3_KEY_EVENTS];
	UnsignedInt m_nextFreeIndex;  // Write position
	UnsignedInt m_nextGetIndex;   // Read position
};

#endif // SAGE_USE_SDL3
