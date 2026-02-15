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
** SDL3-based keyboard implementation for Linux builds.
**
** TheSuperHackers @feature CnC_Generals_Linux 10/02/2026 Bender
** Replaces Win32DIKeyboard with SDL3 keyboard APIs for Linux.
*/

#pragma once

#ifndef _WIN32

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
	
	// SDL3-specific methods
	void addSDL3KeyEvent(const SDL_KeyboardEvent& event);

protected:
	virtual void getKey(KeyboardIO *key);  // GeneralsX @build fbraz 12/02/2026 BenderAI - Get keyboard event
	virtual KeyVal translateScanCodeToKeyVal(unsigned char scan);

private:
	void translateKeyEvent(const SDL_KeyboardEvent& event);
	
	// SDL3 event buffer
	static const int MAX_SDL3_KEY_EVENTS = 64;
	
	struct SDL3KeyEvent {
		SDL_KeyboardEvent event;
	};
	
	SDL3KeyEvent m_eventBuffer[MAX_SDL3_KEY_EVENTS];
	int m_eventHead;
	int m_eventTail;
	
	const bool* m_SDLKeyState;  // Pointer to SDL3's internal key state array
	int m_NumKeys;
};

#endif // !_WIN32
