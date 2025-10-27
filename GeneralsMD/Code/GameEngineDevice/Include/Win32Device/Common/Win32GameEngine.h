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

// FILE: Win32GameEngine.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Description:
//   Device implementation of the game engine ... this is, of course, the
//   highest level of the game that creates the necessary interfaces to the
//   devices we need
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/GameEngine.h"
#include "Common/GameAudio.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "MilesAudioDevice/MilesAudioManager.h"
#include "Win32Device/Common/Win32BIGFileSystem.h"
#include "Win32Device/Common/Win32LocalFileSystem.h"
#ifndef _WIN32
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "StdDevice/Common/StdBIGFileSystem.h"
#endif
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DThingFactory.h"




//-------------------------------------------------------------------------------------------------
/** Class declaration for the Win32 game engine */
//-------------------------------------------------------------------------------------------------
class Win32GameEngine : public GameEngine
{

public:

	Win32GameEngine();
	virtual ~Win32GameEngine();

	virtual void init( void );															///< initialization
	virtual void reset( void );															///< reset engine
	virtual void update( void );														///< update the game engine
	virtual void serviceWindowsOS( void );									///< allow windows maintenance in background

protected:

	virtual GameLogic *createGameLogic( void );							///< factory for game logic
 	virtual GameClient *createGameClient( void );						///< factory for game client
	virtual ModuleFactory *createModuleFactory( void );			///< factory for creating modules
	virtual ThingFactory *createThingFactory( void );				///< factory for the thing factory
	virtual FunctionLexicon *createFunctionLexicon( void ); ///< factory for function lexicon
	virtual LocalFileSystem *createLocalFileSystem( void ); ///< factory for local file system
	virtual ArchiveFileSystem *createArchiveFileSystem( void );	///< factory for archive file system
	virtual NetworkInterface *createNetwork( void );				///< Factory for the network
	virtual Radar *createRadar( void );											///< Factory for radar
	virtual WebBrowser *createWebBrowser( void );						///< Factory for embedded browser
	virtual AudioManager *createAudioManager( void );				///< Factory for audio device
	virtual ParticleSystemManager* createParticleSystemManager( void );


protected:
	UINT m_previousErrorMode;
};

// INLINE -----------------------------------------------------------------------------------------
inline GameLogic *Win32GameEngine::createGameLogic( void ) { return NEW W3DGameLogic; }
inline GameClient *Win32GameEngine::createGameClient( void ) { return NEW W3DGameClient; }
inline ModuleFactory *Win32GameEngine::createModuleFactory( void ) { return NEW W3DModuleFactory; }
inline ThingFactory *Win32GameEngine::createThingFactory( void ) { return NEW W3DThingFactory; }
inline FunctionLexicon *Win32GameEngine::createFunctionLexicon( void ) { return NEW W3DFunctionLexicon; }
inline LocalFileSystem *Win32GameEngine::createLocalFileSystem( void ) {
#ifdef _WIN32
    return NEW Win32LocalFileSystem;
#else
    return NEW StdLocalFileSystem;
#endif
}
inline ArchiveFileSystem *Win32GameEngine::createArchiveFileSystem( void ) {
#ifdef _WIN32
    return NEW Win32BIGFileSystem;
#else
    return NEW StdBIGFileSystem;
#endif
}
inline ParticleSystemManager* Win32GameEngine::createParticleSystemManager( void ) { return NEW W3DParticleSystemManager; }

inline NetworkInterface *Win32GameEngine::createNetwork( void ) { return NetworkInterface::createNetwork(); }
inline Radar *Win32GameEngine::createRadar( void ) { return NEW W3DRadar; }
inline WebBrowser *Win32GameEngine::createWebBrowser( void ) { 
#ifdef _WIN32
    return NEW CComObject<W3DWebBrowser>; 
#else
    return NEW W3DWebBrowser;
#endif
}
inline AudioManager *Win32GameEngine::createAudioManager( void ) { 
#ifdef _WIN32
    return NEW MilesAudioManager; 
#else
    return NEW AudioManagerDummy; // Stub - audio disabled on non-Windows for now
#endif
}
