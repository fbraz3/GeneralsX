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
//  SDL2Main.h - SDL2 Entry Point Header
//
//  Global declarations for SDL2-based entry point
//  Mirrors WinMain.h but uses SDL2 window and renderer
//
//  Phase 02: SDL2 Application Entry Point
////////////////////////////////////////////////////////////////////////////////

#include <SDL2/SDL.h>

class GameEngine;

// GLOBALS ////////////////////////////////////////////////////////////////////

/// SDL2 main window (replaces HWND ApplicationHWnd)
extern SDL_Window *g_applicationWindow;

/// SDL2 renderer for basic rendering (auxiliary)
extern SDL_Renderer *g_applicationRenderer;

/// SDL2 Mouse implementation (replaces Win32Mouse *TheWin32Mouse)
class SDL2Mouse;
extern SDL2Mouse *TheSDL2Mouse;

/// Event timestamp (replaces DWORD TheMessageTime)
extern Uint32 g_eventTimestamp;

// FUNCTION DECLARATIONS //////////////////////////////////////////////////////

/// Create the SDL2 game engine
extern GameEngine *CreateGameEngine(void);
