/*
**	Command & Conquer Generals(tm)
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

// FILE: W3DGameEngine.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Description:
//   Implementation of the Win32 game engine, this is the highest level of
//   the game application, it creates all the devices we will use for the game
///////////////////////////////////////////////////////////////////////////////////////////////////

// Phase 02: SDL2 cross-platform event handling
#include <SDL2/SDL.h>
// Phase 03: SDL2 input compatibility layer
#include "WW3D2/win32_sdl_api_compat.h"

#include "Win32Device/Common/Win32GameEngine.h"
#include "Common/PerfTimer.h"

#include "GameNetwork/LANAPICallbacks.h"

extern DWORD TheMessageTime;

// Phase 03: SDL2 input classes forward declarations
extern class Win32Mouse *TheWin32Mouse;

//-------------------------------------------------------------------------------------------------
/** Constructor for Win32GameEngine */
//-------------------------------------------------------------------------------------------------
Win32GameEngine::Win32GameEngine()
{
	// Stop blue screen
	m_previousErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
}

//-------------------------------------------------------------------------------------------------
/** Destructor for Win32GameEngine */
//-------------------------------------------------------------------------------------------------
Win32GameEngine::~Win32GameEngine()
{
	// restore it (this isn't really necessary, but feels good.)
	SetErrorMode( m_previousErrorMode );
}


//-------------------------------------------------------------------------------------------------
/** Initialize the game engine */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::init( void )
{

	// extending functionality
	GameEngine::init();

}

//-------------------------------------------------------------------------------------------------
/** Reset the system */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::reset( void )
{

	// extending functionality
	GameEngine::reset();

}

//-------------------------------------------------------------------------------------------------
/** Update the game engine by updating the GameClient and
	* GameLogic singletons. */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::update( void )
{


	// call the engine normal update
	GameEngine::update();

	extern HWND ApplicationHWnd;
	if (ApplicationHWnd && ::IsIconic(ApplicationHWnd)) {
		while (ApplicationHWnd && ::IsIconic(ApplicationHWnd)) {
			// We are alt-tabbed out here.  Sleep a bit, & process windows
			// so that we can become un-alt-tabbed out.
			Sleep(5);
			serviceWindowsOS();

			if (TheLAN != NULL) {
				// BGC - need to update TheLAN so we can process and respond to other
				// people's messages who may not be alt-tabbed out like we are.
				TheLAN->setIsActive(isActive());
				TheLAN->update();
			}

			// If we are running a multiplayer game, keep running the logic.
			// There is code in the client to skip client redraw if we are
			// iconic.  jba.
			if (TheGameEngine->getQuitting() || TheGameLogic->isInInternetGame() || TheGameLogic->isInLanGame()) {
				break; // keep running.
			}
		}
	}

	// allow windows to perform regular windows maintenance stuff like msgs
	serviceWindowsOS();

}

//-------------------------------------------------------------------------------------------------
/** This function may be called from within this application to let
  * Microsoft Windows do its message processing and dispatching.  Presumeably
	* we would call this at least once each time around the game loop to keep
	* Windows services from backing up */
//-------------------------------------------------------------------------------------------------
void Win32GameEngine::serviceWindowsOS( void )
{
	// Phase 02-03: SDL2 event loop implementation with keyboard/mouse support
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				// User clicked close button
				setQuitting(true);
				break;
				
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				// Window gained focus
				setIsActive(true);
				break;
				
			case SDL_WINDOWEVENT_FOCUS_LOST:
				// Window lost focus
				setIsActive(false);
				break;

			// Phase 03: Keyboard events
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			{
				// Translate SDL keyboard event to Windows format
				uint32_t msg, wparam, lparam;
				if (SDL2_ProcessKeyboardEvent(&event.key, &msg, &wparam, &lparam)) {
					// TODO: Pass to keyboard subsystem when available
					// For now, just log the event
					printf("Phase 03: Keyboard event - msg:0x%X wParam:0x%X lParam:0x%X\n",
					       msg, wparam, lparam);
				}
				break;
			}

			// Phase 03: Mouse button events
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				if (TheWin32Mouse) {
					// Translate SDL mouse button event to Windows format
					uint32_t msg = SDL2_TranslateMouseButton(event.button.button,
					                                         event.type == SDL_MOUSEBUTTONDOWN ? 1 : 0);
					uint32_t lparam = SDL2_EncodeMouseCoords(event.button.x, event.button.y);
					uint32_t wparam = 0;  // TODO: encode button state from SDL_GetMouseState()
					
					// Add to mouse event queue
					TheWin32Mouse->addWin32Event(msg, wparam, lparam, SDL_GetTicks());
				}
				break;
			}

			// Phase 03: Mouse motion events
			case SDL_MOUSEMOTION:
			{
				if (TheWin32Mouse) {
					// Queue mouse move event
					uint32_t lparam = SDL2_EncodeMouseCoords(event.motion.x, event.motion.y);
					uint32_t wparam = 0;  // TODO: encode button state
					
					// Add to mouse event queue
					TheWin32Mouse->addWin32Event(0x0200, wparam, lparam, SDL_GetTicks());  // WM_MOUSEMOVE
				}
				break;
			}

			// Phase 03: Mouse wheel events
			case SDL_MOUSEWHEEL:
			{
				if (TheWin32Mouse) {
					// Get current mouse position from the motion event data
					// For wheel events, we need to query current position
					int x, y;
					SDL_GetMouseState(&x, &y);
					
					uint32_t lparam = SDL2_EncodeMouseCoords(x, y);
					// Encode wheel delta: positive = scroll up, negative = scroll down
					// Windows uses 120 as one wheel click (MOUSE_WHEEL_DELTA)
					uint32_t wparam = (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL) ?
						(event.wheel.y > 0 ? 0x00780000 : 0xFF880000) :  // 120 or -120 in upper word
						(event.wheel.y < 0 ? 0x00780000 : 0xFF880000);   // Inverted for natural scrolling
					
					// Add to mouse event queue
					TheWin32Mouse->addWin32Event(0x020A, wparam, lparam, SDL_GetTicks());  // WM_MOUSEWHEEL
				}
				break;
			}
				
			default:
				// Other SDL2 events are handled elsewhere
				break;
		}
	}

}

