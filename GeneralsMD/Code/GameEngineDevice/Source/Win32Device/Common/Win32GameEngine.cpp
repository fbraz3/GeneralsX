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

#ifndef _WIN32
#include <glad/glad.h>  // CRITICAL: GLAD must be included BEFORE SDL2
#include <SDL2/SDL.h>
#endif

#include <windows.h>
#include "Win32Device/Common/Win32GameEngine.h"
#include "Common/PerfTimer.h"

#include "GameNetwork/LANAPICallbacks.h"

extern DWORD TheMessageTime;

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

    // When we are alt-tabbed out... the MilesAudioManager seems to go into a coma sometimes
    // and not regain focus properly when we come back. This seems to wake it up nicely.
    AudioAffect aa = (AudioAffect)0x10;
		TheAudio->setVolume(TheAudio->getVolume( aa ), aa );

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
#ifdef _WIN32
	// Windows platform: Use traditional Windows message loop
	MSG msg;
	Int returnValue;

	//
	// see if we have any messages to process, a NULL window handle tells the
	// OS to look at the main window associated with the calling thread, us!
	//
	while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{

		// get the message
		returnValue = GetMessage( &msg, NULL, 0, 0 );

		// this is one possible way to check for quitting conditions as a message
		// of WM_QUIT will cause GetMessage() to return 0
/*
		if( returnValue == 0 )
		{

			setQuitting( true );
			break;

		}
*/

		TheMessageTime = msg.time;
		// translate and dispatch the message
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		TheMessageTime = 0;

	}
#else
	// Phase 27.1.5: SDL2 event loop for cross-platform support (macOS/Linux)
	SDL_Event event;
	
	// Debug: Count events polled
	static int event_count = 0;
	
	// Poll all pending SDL events
	while (SDL_PollEvent(&event)) {
		if (event_count < 10) {
			printf("Phase 27.1.5 DEBUG: SDL event type=%d (count=%d)\n", event.type, event_count);
		}
		event_count++;
		
		switch (event.type) {
			case SDL_QUIT:
				// User closed the window or system quit
				printf("Phase 27.1.5: SDL_QUIT event received - setting quitting flag\n");
				setQuitting(true);
				break;
				
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_CLOSE:
						printf("Phase 27.1.5: SDL_WINDOWEVENT_CLOSE - setting quitting flag\n");
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
						
					case SDL_WINDOWEVENT_MINIMIZED:
						setIsActive(false);
						break;
						
					case SDL_WINDOWEVENT_RESTORED:
						setIsActive(true);
						break;
				}
				break;
				
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				// Keyboard events - will be handled by input system
				// For now, just pass through (input handling is in separate system)
				break;
				
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				// Mouse events - will be handled by mouse input system
				break;
				
			default:
				// Other events can be safely ignored for now
				break;
		}
	}
#endif
}

