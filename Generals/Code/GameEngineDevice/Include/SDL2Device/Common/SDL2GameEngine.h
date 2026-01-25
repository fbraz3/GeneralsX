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
//  SDL2GameEngine.h - SDL2 Platform Game Engine
//
//  Implements GameEngine for SDL2-based platforms (macOS, Linux, etc.)
//  Handles SDL2 event polling and window lifecycle management
//
//  Phase 02: SDL2 Application Event Loop
//
//  Author: Phase 02 (SDL2 Port)
////////////////////////////////////////////////////////////////////////////////

#include "Common/GameEngine.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "AudioDevice/OpenALAudioManager.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
// SDL2 types - forward declared to avoid SDL2 header include issues in VC6
// Implementations use void* pointers to avoid SDL.h inclusion in headers
typedef struct SDL_Window SDL_Window;

// CONSTANTS //////////////////////////////////////////////////////////////////

// TYPES / CLASSES ////////////////////////////////////////////////////////////

/**
 * SDL2GameEngine - SDL2-based implementation of GameEngine
 *
 * This class replaces Win32GameEngine for non-Windows platforms.
 * It handles SDL2 event polling instead of Windows message pump.
 *
 * Key differences from Win32GameEngine:
 * - Uses SDL_PollEvent() instead of PeekMessage/GetMessage
 * - Maps SDL_WindowEvent to engine lifecycle events
 * - Handles minimized window via SDL_GetWindowFlags()
 *
 * Factory methods are inherited from Win32GameEngine pattern.
 */
class SDL2GameEngine : public GameEngine
{
public:

	/** Constructor */
	SDL2GameEngine();

	/** Destructor */
	~SDL2GameEngine();

	/** Initialize the game engine */
	void init(void);

	/** Reset the system */
	void reset(void);

	/**
	 * Update the game engine
	 * Called once per frame from GameMain()
	 */
	void update(void);

	/**
	 * Service SDL2 OS events
	 * Processes pending SDL events and translates them to internal engine events
	 */
	void serviceSDL2OS(void);

protected:

	/** Handle SDL window events (implementation in cpp file for SDL2 visibility) */
	void handleWindowEvent(void *sdlWindowEvent);

	// Pure virtual factory methods - inherited from GameEngine
	virtual GameLogic *createGameLogic(void);
	virtual GameClient *createGameClient(void);
	virtual ModuleFactory *createModuleFactory(void);
	virtual ThingFactory *createThingFactory(void);
	virtual FunctionLexicon *createFunctionLexicon(void);
	virtual LocalFileSystem *createLocalFileSystem(void);
	virtual ArchiveFileSystem *createArchiveFileSystem(void);
	virtual NetworkInterface *createNetwork(void);
	virtual Radar *createRadar(void);
	virtual WebBrowser *createWebBrowser(void);
	virtual AudioManager *createAudioManager(void);
	virtual ParticleSystemManager* createParticleSystemManager(void);

private:

	// No additional state needed for basic event handling
};

// INLINE FACTORIES ///////////////////////////////////////////////////////////
// These factories return the same objects as Win32GameEngine
inline GameLogic *SDL2GameEngine::createGameLogic(void) { return NEW W3DGameLogic; }
inline GameClient *SDL2GameEngine::createGameClient(void) { return NEW W3DGameClient; }
inline ModuleFactory *SDL2GameEngine::createModuleFactory(void) { return NEW W3DModuleFactory; }
inline ThingFactory *SDL2GameEngine::createThingFactory(void) { return NEW W3DThingFactory; }
inline FunctionLexicon *SDL2GameEngine::createFunctionLexicon(void) { return NEW W3DFunctionLexicon; }
inline LocalFileSystem *SDL2GameEngine::createLocalFileSystem(void) { return NEW Win32LocalFileSystem; }
inline ArchiveFileSystem *SDL2GameEngine::createArchiveFileSystem(void) { return NEW Win32BIGFileSystem; }
inline ParticleSystemManager* SDL2GameEngine::createParticleSystemManager(void) { return NEW W3DParticleSystemManager; }

inline NetworkInterface *SDL2GameEngine::createNetwork(void) { return NetworkInterface::createNetwork(); }
inline Radar *SDL2GameEngine::createRadar(void) { return NEW W3DRadar; }
inline WebBrowser *SDL2GameEngine::createWebBrowser(void) { return NEW CComObject<W3DWebBrowser>; }
inline AudioManager *SDL2GameEngine::createAudioManager(void) { return NEW OpenALAudioManager; }
