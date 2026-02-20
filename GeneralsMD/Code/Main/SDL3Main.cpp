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

/*
** SDL3Main.cpp
**
** Entry point for Linux builds using SDL3 windowing and DXVK graphics.
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Entry point replaces WinMain() for Linux builds.
** Instantiates SDL3GameEngine and calls GameMain().
*/

#ifndef _WIN32

// SYSTEM INCLUDES
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// USER INCLUDES (match WinMain.cpp pattern)
#include "Lib/BaseType.h"
#include "Common/CommandLine.h"
#include "Common/CriticalSection.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "Common/GameMemory.h"
#include "Common/Debug.h"
#include "Common/version.h"  // GeneralsX @bugfix BenderAI 14/02/2026 Version class + TheVersion extern
#include "SDL3GameEngine.h"

// DXVK WSI
#define DXVK_WSI_SDL3 1
#include <wsi/native_wsi.h>

// CRITICAL SECTIONS (Linux needs these too)
static CriticalSection critSec1;
static CriticalSection critSec2;
static CriticalSection critSec3;
static CriticalSection critSec4;
static CriticalSection critSec5;

// GLOBAL COMMAND LINE ARGUMENTS
// TheSuperHackers @build felipebraz 13/02/2026
// Store argc/argv from main() for use by CommandLine.cpp parseCommandLine() on Linux
// Windows provides these automatically; Linux needs explicit globals
int __argc = 0;          ///< global argument count
char** __argv = nullptr; ///< global argument vector

// GLOBAL WINDOW HANDLE
// TheSuperHackers @build felipebraz 13/02/2026
// ApplicationHWnd is declared extern in GeneralsMD/Code/Main/WinMain.h
// On Linux, we cast SDL_Window* to HWND type for compatibility
HWND ApplicationHWnd = nullptr;  ///< our application window handle

// GLOBAL SDL3 WINDOW
// GeneralsX @feature felipebraz 16/02/2026
// SDL3 window created in main() before GameMain(), stored globally for engine access
SDL_Window* TheSDL3Window = nullptr;

// GAME TEXT FILE PATHS
// TheSuperHackers @build felipebraz 13/02/2026
// GameText.cpp uses these paths to load CSF and STR files (game localization)
// Format %s is replaced with language code in GameTextManager::init()
// GeneralsX @bugfix BenderAI 13/02/2026 - Fix case-sensitivity on Linux (generals.csf vs Generals.csf)
const Char *g_csfFile = "data/%s/generals.csf";  ///< CSF file path (lowercase for Linux compatibility)
const Char *g_strFile = "data/Generals.str";     ///< STR file path

// Extern declarations (from GameMain.cpp)
extern Int GameMain();

/**
 * CreateGameEngine
 *
 * Factory function for SDL3GameEngine on Linux.
 * Called by GameMain() to instantiate platform-specific engine.
 *
 * @return SDL3GameEngine instance
 */
GameEngine *CreateGameEngine(void)
{
	fprintf(stderr, "INFO: CreateGameEngine() - Creating SDL3GameEngine for Linux\n");
	SDL3GameEngine *engine = NEW SDL3GameEngine();
	return engine;
}

/**
 * main
 *
 * Linux entry point (replaces WinMain on Windows).
 * Initializes subsystems and calls GameMain().
 *
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return Exit code (0 = success)
 */
int main(int argc, char* argv[])
{
	int exitcode = 1;

	// TheSuperHackers @build felipebraz 13/02/2026
	// Store command line arguments in globals for CommandLine.cpp parser
	__argc = argc;
	__argv = argv;

	fprintf(stderr, "=================================================\n");
	fprintf(stderr, " Command & Conquer Generals: Zero Hour (Linux)\n");
	fprintf(stderr, " SDL3 + DXVK Build\n");
	fprintf(stderr, "=================================================\n\n");

	try {
		// Initialize critical sections (required by game engine)
		TheAsciiStringCriticalSection = &critSec1;
		TheUnicodeStringCriticalSection = &critSec2;
		TheDmaCriticalSection = &critSec3;
		TheMemoryPoolCriticalSection = &critSec4;
		TheDebugLogCriticalSection = &critSec5;

		// Initialize memory manager early (required by NEW operator)
		initMemoryManager();

		// GeneralsX @bugfix BenderAI 14/02/2026 Initialize Version singleton
		// GameEngine::init() calls updateWindowTitle() which uses TheVersion
		// Must be created before GameMain() to avoid nullptr dereference
		TheVersion = NEW Version;

		// Parse command line (CommandLine class handles argc/argv internally)
		// TheSuperHackers @build felipebraz 10/02/2026 Phase 1.5
		// Store argc/argv for CommandLine parser to access via _NSGetArgc/_NSGetArgv or /proc/self/cmdline
		// For now, let CommandLine::parseCommandLineForStartup() handle this
		CommandLine::parseCommandLineForStartup();

		// GeneralsX @bugfix felipebraz 16/02/2026
		// Initialize SDL3 and Vulkan BEFORE creating GameEngine (fighter19 pattern)
		// This prevents LLVM SIGSEGV crash during Vulkan driver enumeration
		// Must be done here, not in SDL3GameEngine::init() which is too late
		fprintf(stderr, "INFO: Initializing SDL3 video subsystem...\n");
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
			fprintf(stderr, "FATAL: Failed to initialize SDL3: %s\n", SDL_GetError());
			return 1;
		}

		// Set DXVK WSI driver before loading Vulkan
		setenv("DXVK_WSI_DRIVER", "SDL3", 1);

		// Load Vulkan library for DXVK DirectX8→Vulkan translation
		fprintf(stderr, "INFO: Loading Vulkan library...\n");
		if (!SDL_Vulkan_LoadLibrary(nullptr)) {
			fprintf(stderr, "WARNING: Failed to load Vulkan: %s\n", SDL_GetError());
			fprintf(stderr, "WARNING: Continuing without Vulkan (may use software rendering)\n");
		}

		// Create SDL3 window with Vulkan support
		fprintf(stderr, "INFO: Creating SDL3 Vulkan window...\n");
		Uint32 windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;  // Start hidden, show after D3D init
		TheSDL3Window = SDL_CreateWindow(
			"Command & Conquer Generals: Zero Hour",
			1024, 768,  // Default resolution
			windowFlags
		);

		if (!TheSDL3Window) {
			fprintf(stderr, "FATAL: Failed to create SDL3 window: %s\n", SDL_GetError());
			SDL_Quit();
			return 1;
		}

		// Store window handle globally (cast SDL_Window* to HWND for compatibility)
		ApplicationHWnd = (HWND)TheSDL3Window;
		fprintf(stderr, "INFO: SDL3 window created successfully\n");

		// Call cross-platform game entry point
		exitcode = GameMain();

		fprintf(stderr, "INFO: GameMain() returned with code %d\n", exitcode);

	} catch (const std::exception& e) {
		fprintf(stderr, "FATAL: Unhandled exception in main(): %s\n", e.what());
		exitcode = 1;
	} catch (...) {
		fprintf(stderr, "FATAL: Unknown exception in main()\n");
		exitcode = 1;
	}

	// Cleanup SDL3 resources
	if (TheSDL3Window) {
		SDL_DestroyWindow(TheSDL3Window);
		TheSDL3Window = nullptr;
		ApplicationHWnd = nullptr;
	}
	SDL_Quit();

	// GeneralsX @bugfix BenderAI 14/02/2026 Cleanup Version singleton
	if (TheVersion) {
		delete TheVersion;
		TheVersion = nullptr;
	}

	// GeneralsX @bugfix BenderAI 19/02/2026 Shutdown memory manager BEFORE nulling critical
	// sections. Without this, global pool destructors (ObjectPoolClass) crash during atexit()
	// because they call ::operator delete after the memory manager is already gone (SIGSEGV).
	// Matches WinMain.cpp cleanup order: TheVersion -> shutdownMemoryManager -> null critSecs.
	shutdownMemoryManager();

	// Cleanup critical sections (after memory manager, which may use them during shutdown)
	TheAsciiStringCriticalSection = nullptr;
	TheUnicodeStringCriticalSection = nullptr;
	TheDmaCriticalSection = nullptr;
	TheMemoryPoolCriticalSection = nullptr;
	TheDebugLogCriticalSection = nullptr;

	fprintf(stderr, "\nExiting with code %d\n", exitcode);
	return exitcode;
}

#endif // !_WIN32
