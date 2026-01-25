/*
**Command & Conquer Generals Zero Hour(tm)
**Copyright 2025 Electronic Arts Inc.
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SDL2Device/Common/SDL2GameEngine.h"

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
/** Update the game engine - processes SDL2 events */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::update(void)
{
// Call parent class update
GameEngine::update();
}

//-------------------------------------------------------------------------------------------------
/** Service SDL2 OS events */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::serviceSDL2OS(void)
{
// SDL2 event processing will be implemented here in future
// For now, just call parent update
}

//-------------------------------------------------------------------------------------------------
/** Handle SDL window events */
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::handleWindowEvent(void *sdlWindowEvent)
{
// Window event handling will be implemented here in future
// For now, events are simply processed by the parent class
}
