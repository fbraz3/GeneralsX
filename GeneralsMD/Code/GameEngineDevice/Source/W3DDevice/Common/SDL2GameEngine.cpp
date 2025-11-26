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
//  (c) 2025 Electronic Arts Inc.
//
////////////////////////////////////////////////////////////////////////////////

// FILE: SDL2GameEngine.cpp
// Cross-platform game engine implementation
// Author: Modernization Port, 2025

#include "W3DDevice/Common/SDL2GameEngine.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "StdDevice/Common/StdBIGFileSystem.h"
#include "Audio/OpenALAudioManager.h"

// Forward declarations
class AudioManager;

//-------------------------------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::SDL2GameEngine()
{
	// Base class constructor handles initialization
	DEBUG_LOG(("SDL2GameEngine::SDL2GameEngine - Creating cross-platform game engine\n"));
}

//-------------------------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------------------------
SDL2GameEngine::~SDL2GameEngine()
{
	DEBUG_LOG(("SDL2GameEngine::~SDL2GameEngine - Destroying cross-platform game engine\n"));
}

//-------------------------------------------------------------------------------------------------
// Initialize the game engine
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::init( void )
{
	DEBUG_LOG(("SDL2GameEngine::init - Initializing game engine subsystems\n"));
	
	// Call base class initialization
	GameEngine::init();
}

//-------------------------------------------------------------------------------------------------
// Reset the game engine
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::reset( void )
{
	DEBUG_LOG(("SDL2GameEngine::reset - Resetting game engine\n"));
	
	// Call base class reset
	GameEngine::reset();
}

//-------------------------------------------------------------------------------------------------
// Update the game engine (called once per frame)
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::update( void )
{
	DEBUG_LOG(("SDL2GameEngine::update - Updating game engine\n"));
	
	// Call base class update
	GameEngine::update();
}

//-------------------------------------------------------------------------------------------------
// Service OS maintenance (stub for cross-platform compatibility)
//-------------------------------------------------------------------------------------------------
void SDL2GameEngine::serviceWindowsOS( void )
{
	// This is a no-op on non-Windows platforms, but required by base class interface
	// On Windows, this would handle Windows-specific events
}

//-------------------------------------------------------------------------------------------------
// Factory methods for creating subsystems
//-------------------------------------------------------------------------------------------------

GameLogic *SDL2GameEngine::createGameLogic( void )
{
	DEBUG_LOG(("SDL2GameEngine::createGameLogic - Creating W3D game logic\n"));
	return NEW W3DGameLogic;
}

GameClient *SDL2GameEngine::createGameClient( void )
{
	DEBUG_LOG(("SDL2GameEngine::createGameClient - Creating W3D game client\n"));
	return NEW W3DGameClient;
}

ModuleFactory *SDL2GameEngine::createModuleFactory( void )
{
	DEBUG_LOG(("SDL2GameEngine::createModuleFactory - Creating W3D module factory\n"));
	return NEW W3DModuleFactory;
}

ThingFactory *SDL2GameEngine::createThingFactory( void )
{
	DEBUG_LOG(("SDL2GameEngine::createThingFactory - Creating W3D thing factory\n"));
	return NEW W3DThingFactory;
}

FunctionLexicon *SDL2GameEngine::createFunctionLexicon( void )
{
	fprintf(stderr, "SDL2GameEngine::createFunctionLexicon - About to create W3D function lexicon\n");
	fflush(stderr);
	FunctionLexicon *result = NEW W3DFunctionLexicon;
	fprintf(stderr, "SDL2GameEngine::createFunctionLexicon - W3D function lexicon created: %p\n", result);
	fflush(stderr);
	return result;
}

LocalFileSystem *SDL2GameEngine::createLocalFileSystem( void )
{
	DEBUG_LOG(("SDL2GameEngine::createLocalFileSystem - Creating standard POSIX-compatible file system\n"));
	return NEW StdLocalFileSystem;
}

ArchiveFileSystem *SDL2GameEngine::createArchiveFileSystem( void )
{
	DEBUG_LOG(("SDL2GameEngine::createArchiveFileSystem - Creating standard BIG file system\n"));
	return NEW StdBIGFileSystem;
}

NetworkInterface *SDL2GameEngine::createNetwork( void )
{
	DEBUG_LOG(("SDL2GameEngine::createNetwork - Creating network interface\n"));
	try {
		NetworkInterface *network = NetworkInterface::createNetwork();
		if (network) {
			DEBUG_LOG(("SDL2GameEngine::createNetwork - Network interface created successfully\n"));
			return network;
		}
		DEBUG_LOG(("SDL2GameEngine::createNetwork - Failed to create network interface\n"));
		return nullptr;
	} catch (const std::exception& e) {
		DEBUG_LOG(("SDL2GameEngine::createNetwork - Exception: %s\n", e.what()));
		return nullptr;
	} catch (...) {
		DEBUG_LOG(("SDL2GameEngine::createNetwork - Unknown exception\n"));
		return nullptr;
	}
}

Radar *SDL2GameEngine::createRadar( void )
{
	DEBUG_LOG(("SDL2GameEngine::createRadar - Creating radar\n"));
	return NEW W3DRadar;
}

WebBrowser *SDL2GameEngine::createWebBrowser( void )
{
	DEBUG_LOG(("SDL2GameEngine::createWebBrowser - Creating web browser\n"));
	
	// Note: WebBrowser is Windows-specific COM interface (not available cross-platform)
	// For now, web browser functionality is disabled on macOS/Linux
	// This is acceptable as WebBrowser is an optional subsystem, not critical for gameplay
	
	#ifdef _WIN32
		// Windows: Return real WebBrowser instance if available
		try {
			WebBrowser *browser = NEW W3DWebBrowser();
			if (browser) {
				DEBUG_LOG(("SDL2GameEngine::createWebBrowser - WebBrowser created successfully\n"));
				return browser;
			}
		} catch (const std::exception& e) {
			DEBUG_LOG(("SDL2GameEngine::createWebBrowser - Exception: %s\n", e.what()));
		}
	#else
		// macOS/Linux: WebBrowser not available (Windows COM API)
		DEBUG_LOG(("SDL2GameEngine::createWebBrowser - WebBrowser not available on this platform\n"));
	#endif
	
	return nullptr;
}

ParticleSystemManager* SDL2GameEngine::createParticleSystemManager( void )
{
	DEBUG_LOG(("SDL2GameEngine::createParticleSystemManager - Creating W3D particle system manager\n"));
	// W3DParticleSystemManager provides platform-independent rendering for particle effects
	// including smoke, explosions, weather effects, and other visual effects
	return NEW W3DParticleSystemManager;
}

AudioManager *SDL2GameEngine::createAudioManager( void )
{
	DEBUG_LOG(("SDL2GameEngine::createAudioManager - Creating OpenAL audio manager\n"));
	// Return new OpenALAudioManager instance (Phase 33 OpenAL backend)
	return NEW OpenALAudioManager;
}
