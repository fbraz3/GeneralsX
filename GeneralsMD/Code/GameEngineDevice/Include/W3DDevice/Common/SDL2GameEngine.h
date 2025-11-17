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

// FILE: SDL2GameEngine.h //////////////////////////////////////////////////////
// Cross-platform game engine implementation using SDL2
// Author: Modernization Port, 2025
// Description:
//   Device implementation of the game engine for cross-platform (non-Windows)
//   systems using SDL2 for window management and input handling
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/GameEngine.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"

// Forward declarations
class LocalFileSystem;
class ArchiveFileSystem;

//-------------------------------------------------------------------------------------------------
/** Class declaration for the SDL2 game engine (cross-platform) */
//-------------------------------------------------------------------------------------------------
class SDL2GameEngine : public GameEngine
{

public:

	SDL2GameEngine();
	virtual ~SDL2GameEngine();

	virtual void init( void );														///< initialization
	virtual void reset( void );														///< reset engine
	virtual void update( void );													///< update the game engine
	virtual void serviceWindowsOS( void );								///< allow OS maintenance in background (stub for compatibility)

protected:

	virtual GameLogic *createGameLogic( void );						///< factory for game logic
 	virtual GameClient *createGameClient( void );					///< factory for game client
	virtual ModuleFactory *createModuleFactory( void );		///< factory for creating modules
	virtual ThingFactory *createThingFactory( void );			///< factory for the thing factory
	virtual FunctionLexicon *createFunctionLexicon( void );///< factory for function lexicon
	virtual LocalFileSystem *createLocalFileSystem( void );///< factory for local file system
	virtual ArchiveFileSystem *createArchiveFileSystem( void );	///< factory for archive file system
	virtual NetworkInterface *createNetwork( void );			///< Factory for the network
	virtual Radar *createRadar( void );										///< Factory for radar
	virtual WebBrowser *createWebBrowser( void );				///< Factory for web browser
	virtual ParticleSystemManager* createParticleSystemManager( void ); ///< Factory for particle systems
	virtual AudioManager *createAudioManager( void );		///< Factory for audio manager

private:

	// SDL2-specific data members would go here if needed
	// Currently relies on base class and W3D subsystems

};
