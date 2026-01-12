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
//  SDL2 Entry Point for macOS / Linux / Windows (non-MSVC builds)
//  Mirrors WinMain.cpp structure but uses SDL2 instead of Win32 APIs
//
//  Phase 02: Initial entry point with global initialization
//  - Memory manager setup
//  - Global data initialization
//  - Game main loop entry
//  - Shutdown and cleanup
//
//  Author: Phase 02 (SDL2 Port)
////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <cstdlib>
#include <cstring>
#include <signal.h>

// SDL2 INCLUDES //////////////////////////////////////////////////////////////
#include <SDL2/SDL.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "SDL2Main.h"
#include "Lib/BaseType.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/GameSounds.h"
#include "Common/Debug.h"
#include "Common/GameMemory.h"
#include "Common/StackDump.h"
#include "Common/MessageStream.h"
#include "Common/Registry.h"
#include "Common/Team.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/Mouse.h"
#include "GameClient/IMEManager.h"
#include "SDL2Device/GameClient/SDL2Mouse.h"
#include "SDL2Device/Common/SDL2GameEngine.h"
#include "Common/version.h"
#include "BuildVersion.h"
#include "GeneratedVersion.h"

#include <rts/profile.h>

// GLOBALS ////////////////////////////////////////////////////////////////////

/// @note SDL2 window handle (replaces Win32 HWND)
SDL_Window *g_applicationWindow = NULL;

/// @note SDL2 renderer (will be used for basic rendering if needed)
SDL_Renderer *g_applicationRenderer = NULL;

/// @note SDL2 mouse implementation (replaces Win32Mouse)
class SDL2Mouse *TheSDL2Mouse = NULL;

/// @note Timestamp of last event (mirrors Win32 TheMessageTime)
Uint32 g_eventTimestamp = 0;

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";
const char *gAppPrefix = "";

static Bool gInitializing = false;
static Bool gDoPaint = true;
static Bool isApplicationActive = false;

//-------------------------------------------------------------------------------------------------
/** Signal handler for SIGTERM and SIGINT (graceful shutdown) */
//-------------------------------------------------------------------------------------------------
static void signal_handler(int sig)
{
	if (TheMessageStream && TheGameEngine)
	{
		DEBUG_LOG(("Received signal %d, initiating graceful shutdown...", sig));
		TheMessageStream->appendMessage(GameMessage::MSG_META_DEMO_INSTANT_QUIT);
	}
	else
	{
		exit(EXIT_SUCCESS);
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize SDL2 and create main window */
//-------------------------------------------------------------------------------------------------
static Bool initializeAppWindow(Int width, Int height, Bool runWindowed)
{
	// Initialize SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		DEBUG_CRASH(("SDL_Init failed: %s", SDL_GetError()));
		return false;
	}

	// Determine window flags
	Uint32 windowFlags = SDL_WINDOW_SHOWN;
	
	if (!runWindowed)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	// Create window
	g_applicationWindow = SDL_CreateWindow(
		"Command and Conquer Generals",  // Title
		SDL_WINDOWPOS_CENTERED,           // X position
		SDL_WINDOWPOS_CENTERED,           // Y position
		width,                            // Width
		height,                           // Height
		windowFlags                       // Flags
	);

	if (!g_applicationWindow)
	{
		DEBUG_CRASH(("SDL_CreateWindow failed: %s", SDL_GetError()));
		SDL_Quit();
		return false;
	}

	// Create renderer for future use (SDL2 graphics)
	// For now, we're not using it (W3D handles rendering)
	g_applicationRenderer = SDL_CreateRenderer(
		g_applicationWindow,
		-1,
		SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
	);

	gInitializing = false;
	isApplicationActive = true;

	DEBUG_LOG(("SDL2 window created successfully (windowed=%d)", runWindowed));
	return true;
}

//-------------------------------------------------------------------------------------------------
/** Shutdown SDL2 and cleanup */
//-------------------------------------------------------------------------------------------------
static void shutdownAppWindow(void)
{
	if (g_applicationRenderer)
	{
		SDL_DestroyRenderer(g_applicationRenderer);
		g_applicationRenderer = NULL;
	}

	if (g_applicationWindow)
	{
		SDL_DestroyWindow(g_applicationWindow);
		g_applicationWindow = NULL;
	}

	SDL_Quit();
	DEBUG_LOG(("SDL2 window shutdown complete"));
}

// Necessary to allow memory managers and such to have useful critical sections
static CriticalSection critSec1, critSec2, critSec3, critSec4, critSec5;

//-------------------------------------------------------------------------------------------------
/** Create the SDL2 game engine we're going to use */
//-------------------------------------------------------------------------------------------------
GameEngine *CreateGameEngine(void)
{
	SDL2GameEngine *engine;

	engine = NEW SDL2GameEngine;
	engine->setIsActive(isApplicationActive);

	return engine;
}

//-------------------------------------------------------------------------------------------------
/** Main entry point for SDL2 platforms */
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	Int exitcode = 1;

#ifdef RTS_PROFILE
	Profile::StartRange("init");
#endif

	try
	{
		// Install signal handlers for graceful shutdown
		signal(SIGTERM, signal_handler);
		signal(SIGINT, signal_handler);

		// Initialize critical sections
		TheAsciiStringCriticalSection = &critSec1;
		TheUnicodeStringCriticalSection = &critSec2;
		TheDmaCriticalSection = &critSec3;
		TheMemoryPoolCriticalSection = &critSec4;
		TheDebugLogCriticalSection = &critSec5;

		// Initialize the memory manager early
		initMemoryManager();

		// Set working directory to executable directory
		// On SDL2 platforms, we typically use standard paths for assets
		char buffer[1024];
		if (argv[0])
		{
			strncpy(buffer, argv[0], sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = 0;
			
			// Find last path separator
			char *slash = strrchr(buffer, '/');
#ifdef _WIN32
			char *backslash = strrchr(buffer, '\\');
			if (backslash > slash)
				slash = backslash;
#endif
			if (slash)
			{
				*slash = 0;
				chdir(buffer);
			}
		}

#ifdef RTS_DEBUG
		// Memory tracking (if available on this platform)
#endif

		// Parse command line arguments
		CommandLine::parseCommandLineForStartup();

		// Create SDL2 window
		gInitializing = true;
		Int displayWidth = TheGlobalData ? TheGlobalData->m_displayWidth : DEFAULT_DISPLAY_WIDTH;
		Int displayHeight = TheGlobalData ? TheGlobalData->m_displayHeight : DEFAULT_DISPLAY_HEIGHT;
		
		if (!TheGlobalData->m_headless && 
		    !initializeAppWindow(displayWidth, displayHeight, TheGlobalData->m_windowed))
		{
			return exitcode;
		}

		// Set up version info
		TheVersion = NEW Version;
		TheVersion->setVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNUM, VERSION_LOCALBUILDNUM,
			AsciiString(VERSION_BUILDUSER), AsciiString(VERSION_BUILDLOC),
			AsciiString(__TIME__), AsciiString(__DATE__));

		// Initialize single-instance mechanism
		if (!rts::ClientInstance::initialize())
		{
			// Another instance is running - for SDL2, we can't easily activate it
			// Just log and exit (or print a message)
			DEBUG_LOG(("Generals is already running...Bail!"));
			delete TheVersion;
			TheVersion = NULL;
			shutdownMemoryManager();
			return exitcode;
		}
		DEBUG_LOG(("Create Generals Mutex okay."));

		DEBUG_LOG(("CRC message is %d", GameMessage::MSG_LOGIC_CRC));

		// Run the game main loop
		exitcode = GameMain();

		delete TheVersion;
		TheVersion = NULL;

#ifdef MEMORYPOOL_DEBUG
		TheMemoryPoolFactory->debugMemoryReport(
			REPORT_POOLINFO | REPORT_POOL_OVERFLOW | REPORT_SIMPLE_LEAKS, 0, 0);
#endif
#if defined(RTS_DEBUG)
		TheMemoryPoolFactory->memoryPoolUsageReport("AAAMemStats");
#endif

		shutdownMemoryManager();
	}
	catch (...)
	{
		DEBUG_CRASH(("Uncaught exception in main..."));
	}

	// Cleanup SDL2
	shutdownAppWindow();

	TheUnicodeStringCriticalSection = NULL;
	TheDmaCriticalSection = NULL;
	TheMemoryPoolCriticalSection = NULL;

	return exitcode;
}
