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
//  FILE: SDL2GameEngine.cpp
//
//  SDL2 Platform Game Engine Implementation (STUB for Phase 08)
//  Handles SDL2 event polling and window lifecycle for macOS/Linux/other
//
//  Phase 08: SDL2 Infrastructure (stub - full implementation in Phase 09+)
//
//  Author: Phase 08 (SDL2 Port)
//
///////////////////////////////////////////////////////////////////////////////

#include "SDL2Device/Common/SDL2GameEngine.h"

//-------------------------------------------------------------------------------------------------
/** Constructor for SDL2GameEngine */
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::SDL2GameEngine()
{
	// Stub constructor
}

//-------------------------------------------------------------------------------------------------
/** Destructor for SDL2GameEngine */
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::~SDL2GameEngine()
{
	// Stub destructor
}

//-------------------------------------------------------------------------------------------------
/** Initialize the game engine */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::init(void)
{
	// Stub: SDL2 initialization will be done in Phase 09
	GameEngine::init();
}

//-------------------------------------------------------------------------------------------------
/** Reset the system */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::reset(void)
{
	// Stub: Reset logic will be done in Phase 09
	GameEngine::reset();
}

//-------------------------------------------------------------------------------------------------
/** Update the game engine */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::update(void)
{
	// Stub: Event loop will be done in Phase 09
	GameEngine::update();
}

//-------------------------------------------------------------------------------------------------
/** Service SDL2 OS events */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::serviceSDL2OS(void)
{
	// Stub: SDL2 event handling will be done in Phase 09
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL window events */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::handleWindowEvent(const SDL_WindowEvent &event)
{
	// Stub: Window event handling will be done in Phase 09
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL quit event */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::handleQuitEvent(void)
{
	// Stub: Quit handling will be done in Phase 09
}

//-------------------------------------------------------------------------------------------------
/** Constructor for SDL2GameEngine */
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::SDL2GameEngine()
{
	DEBUG_LOG(("SDL2GameEngine constructor"));
}

//-------------------------------------------------------------------------------------------------
/** Destructor for SDL2GameEngine */
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::~SDL2GameEngine()
{
	DEBUG_LOG(("SDL2GameEngine destructor"));
}

//-------------------------------------------------------------------------------------------------
/** Initialize the game engine */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::init(void)
{
	// Call parent class initialization
	GameEngine::init();

	DEBUG_LOG(("SDL2GameEngine initialized"));
}

//-------------------------------------------------------------------------------------------------
/** Reset the system */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::reset(void)
{
	// Call parent class reset
	GameEngine::reset();

	DEBUG_LOG(("SDL2GameEngine reset"));
}

//-------------------------------------------------------------------------------------------------
/** Update the game engine by updating GameClient and GameLogic singletons */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::update(void)
{
	// Call the engine normal update
	GameEngine::update();

	// Check if window is minimized
	if (g_applicationWindow)
	{
		Uint32 windowFlags = SDL_GetWindowFlags(g_applicationWindow);
		
		if (windowFlags & SDL_WINDOW_MINIMIZED)
		{
			while (g_applicationWindow && (SDL_GetWindowFlags(g_applicationWindow) & SDL_WINDOW_MINIMIZED))
			{
				// We are minimized (equivalent to alt-tabbed out on Windows)
				// Sleep a bit and process SDL events so we can become un-minimized
				SDL_Delay(5);
				serviceSDL2OS();

				if (TheLAN != NULL)
				{
					// Need to update TheLAN so we can process and respond to other
					// players' messages who may not be minimized like we are.
					TheLAN->setIsActive(isActive());
					TheLAN->update();
				}

				// If we are running a multiplayer game, keep running the logic.
				// The client will skip redraw if we are minimized.
				if (TheGameEngine->getQuitting() || 
				    TheGameLogic->isInInternetGame() || 
				    TheGameLogic->isInLanGame())
				{
					break; // Keep running
				}
			}
		}
	}

	// Process SDL2 events once per frame
	serviceSDL2OS();
}

//-------------------------------------------------------------------------------------------------
/** 
 * Service SDL2 OS events
 * Processes all pending SDL events and translates them to engine events
 */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::serviceSDL2OS(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		g_eventTimestamp = event.common.timestamp;

		switch (event.type)
		{
			case SDL_QUIT:
			{
				handleQuitEvent();
				break;
			}

			case SDL_WINDOWEVENT:
			{
				handleWindowEvent(event.window);
				break;
			}

			case SDL_KEYDOWN:
			{
				if (TheSDL2Keyboard)
				{
					TheSDL2Keyboard->onKeyDown(event.key);
				}
				break;
			}

			case SDL_KEYUP:
			{
				if (TheSDL2Keyboard)
				{
					TheSDL2Keyboard->onKeyUp(event.key);
				}
				break;
			}

		case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				if (TheSDL2Mouse)
				{
					TheSDL2Mouse->onMouseButtonUp(event.button);
				}
				break;
			}

			case SDL_MOUSEMOTION:
			{
				if (TheSDL2Mouse)
				{
					TheSDL2Mouse->onMouseMotion(event.motion);
				}
				break;
			}

			case SDL_MOUSEWHEEL:
			{
				if (TheSDL2Mouse)
				{
					TheSDL2Mouse->onMouseWheel(event.wheel);
				}
				break;
			}

			case SDL_TEXTEDITING:
			{
				if (TheSDL2IMEManager)
				{
					TheSDL2IMEManager->onTextEditing(event.edit);
				}
				break;
			}

			case SDL_TEXTINPUT:
			{
				if (TheSDL2IMEManager)
				{
					TheSDL2IMEManager->onTextInput(event.text);
				}
				break;
			}

			default:
			{
				// Ignore other events for now
				break;
			}
		}

		g_eventTimestamp = 0;
	}
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL window events (focus, minimize, resize, close, etc.) */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::handleWindowEvent(const SDL_WindowEvent &event)
{
	switch (event.event)
	{
		case SDL_WINDOWEVENT_CLOSE:
		{
			// Window close requested
			if (!getQuitting())
			{
				TheMessageStream->appendMessage(GameMessage::MSG_META_DEMO_INSTANT_QUIT);
			}
			break;
		}

		case SDL_WINDOWEVENT_FOCUS_GAINED:
		{
			// Window gained focus
			DEBUG_LOG(("Window focus gained"));
			setIsActive(true);
			
			if (TheKeyboard)
			{
				TheKeyboard->resetKeys();
			}

			if (TheMouse)
			{
				TheMouse->regainFocus();
			}

			if (TheAudio)
			{
				TheAudio->regainFocus();
			}

			// Cursor can only be captured after activation
			if (TheMouse)
			{
				TheMouse->refreshCursorCapture();
			}

			break;
		}

		case SDL_WINDOWEVENT_FOCUS_LOST:
		{
			// Window lost focus
			DEBUG_LOG(("Window focus lost"));
			setIsActive(false);

			if (TheKeyboard)
			{
				TheKeyboard->resetKeys();
			}

			if (TheMouse)
			{
				TheMouse->loseFocus();

				if (TheMouse->isCursorInside())
				{
					TheMouse->onCursorMovedOutside();
				}
			}

			if (TheAudio)
			{
				TheAudio->loseFocus();
			}

			break;
		}

		case SDL_WINDOWEVENT_MINIMIZED:
		{
			DEBUG_LOG(("Window minimized"));
			setIsActive(false);
			break;
		}

		case SDL_WINDOWEVENT_RESTORED:
		{
			DEBUG_LOG(("Window restored"));
			setIsActive(true);
			break;
		}

		case SDL_WINDOWEVENT_MOVED:
		{
			// Window moved
			if (TheMouse)
			{
				TheMouse->refreshCursorCapture();
			}
			break;
		}

		case SDL_WINDOWEVENT_RESIZED:
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			// Window resized
			if (TheMouse)
			{
				TheMouse->refreshCursorCapture();
			}
			break;
		}

		case SDL_WINDOWEVENT_ENTER:
		{
			// Mouse entered window
			if (TheMouse && !TheMouse->isCursorInside())
			{
				TheMouse->onCursorMovedInside();
			}
			break;
		}

		case SDL_WINDOWEVENT_LEAVE:
		{
			// Mouse left window
			if (TheMouse && TheMouse->isCursorInside())
			{
				TheMouse->onCursorMovedOutside();
			}
			break;
		}

		default:
		{
			// Ignore other window events
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL quit event */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::handleQuitEvent(void)
{
	if (!getQuitting())
	{
		TheMessageStream->appendMessage(GameMessage::MSG_META_DEMO_INSTANT_QUIT);
	}
}
