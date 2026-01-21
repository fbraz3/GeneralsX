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

#include "MilesAudioDevice/MilesAudioManager.h"

#include "Win32Device/Common/Win32BIGFileSystem.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"

#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
// Only forward declare what we need in the header to keep compile order flexible.
struct SDL_WindowEvent;

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

	/** Handle SDL window events */
	void handleWindowEvent(const SDL_WindowEvent &event);

	/** Handle SDL quit event */
	void handleQuitEvent(void);

	// Factories (must be implemented to make GameEngine concrete)
	virtual GameLogic *createGameLogic(void) override;
	virtual GameClient *createGameClient(void) override;
	virtual ModuleFactory *createModuleFactory(void) override;
	virtual ThingFactory *createThingFactory(void) override;
	virtual FunctionLexicon *createFunctionLexicon(void) override;
	virtual LocalFileSystem *createLocalFileSystem(void) override;
	virtual ArchiveFileSystem *createArchiveFileSystem(void) override;
	virtual NetworkInterface *createNetwork(void); // kept for parity with Win32GameEngine
	virtual Radar *createRadar(void) override;
	virtual WebBrowser *createWebBrowser(void) override;
	virtual AudioManager *createAudioManager(void) override;
	virtual ParticleSystemManager *createParticleSystemManager(void) override;

private:

	// No additional state needed for basic event handling
};

// INLINE -----------------------------------------------------------------------------------------
inline GameLogic *SDL2GameEngine::createGameLogic(void) { return NEW W3DGameLogic; }
inline GameClient *SDL2GameEngine::createGameClient(void) { return NEW W3DGameClient; }
inline ModuleFactory *SDL2GameEngine::createModuleFactory(void) { return NEW W3DModuleFactory; }
inline ThingFactory *SDL2GameEngine::createThingFactory(void) { return NEW W3DThingFactory; }
inline FunctionLexicon *SDL2GameEngine::createFunctionLexicon(void) { return NEW W3DFunctionLexicon; }
inline LocalFileSystem *SDL2GameEngine::createLocalFileSystem(void) { return NEW Win32LocalFileSystem; }
inline ArchiveFileSystem *SDL2GameEngine::createArchiveFileSystem(void) { return NEW Win32BIGFileSystem; }
inline ParticleSystemManager *SDL2GameEngine::createParticleSystemManager(void) { return NEW W3DParticleSystemManager; }

inline NetworkInterface *SDL2GameEngine::createNetwork(void) { return NetworkInterface::createNetwork(); }
inline Radar *SDL2GameEngine::createRadar(void) { return NEW W3DRadar; }
inline WebBrowser *SDL2GameEngine::createWebBrowser(void) { return NEW W3DWebBrowser; }
