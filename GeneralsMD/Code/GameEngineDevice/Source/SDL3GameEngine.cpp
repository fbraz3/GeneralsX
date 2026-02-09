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
** SDL3GameEngine.cpp
**
** Linux implementation of GameEngine using SDL3 for windowing/input.
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Provides SDL3-based input and window management for Linux builds.
** Based on fighter19 reference implementation.
*/

#ifndef _WIN32

#include "SDL3GameEngine.h"
#include "OpenALAudioManager.h"
#include <cstdio>
#include <cstdlib>

/**
 * Constructor: Initialize SDL3 game engine state
 */
SDL3GameEngine::SDL3GameEngine()
	: GameEngine(),
	  m_SDLWindow(nullptr),
	  m_IsInitialized(false),
	  m_IsActive(false)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::SDL3GameEngine() created\n");
}

/**
 * Destructor: Cleanup SDL3 resources
 */
SDL3GameEngine::~SDL3GameEngine()
{
	if (m_IsInitialized) {
		// Window cleanup is done in reset/shutdown
	}
	fprintf(stderr, "DEBUG: SDL3GameEngine::~SDL3GameEngine() destroyed\n");
}

/**
 * From GameEngine: init() - initialize subsystems
 */
void SDL3GameEngine::init(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::init() starting\n");

	// Set DXVK WSI driver BEFORE loading Vulkan libraries
	setenv("DXVK_WSI_DRIVER", "SDL3", 1);

	// Initialize SDL3
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		fprintf(stderr, "ERROR: Failed to initialize SDL3: %s\n", SDL_GetError());
		return;
	}

	// Load Vulkan library for SDL3  
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		fprintf(stderr, "ERROR: Failed to load Vulkan library for SDL3: %s\n", SDL_GetError());
		SDL_Quit();
		return;
	}

	// Create SDL3 window with Vulkan support
	Uint32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
	m_SDLWindow = SDL_CreateWindow(
		"Command & Conquer Generals: Zero Hour",
		1024, 768,  // Default resolution
		flags
	);

	if (!m_SDLWindow) {
		fprintf(stderr, "ERROR: Failed to create SDL3 window: %s\n", SDL_GetError());
		SDL_Quit();
		return;
	}

	m_IsInitialized = true;
	m_IsActive = true;

	fprintf(stderr, "INFO: SDL3GameEngine::init() - Vulkan window created\n");

	// Call parent init to initialize subsystems
	GameEngine::init();
}

/**
 * From GameEngine: reset() - reset system to starting state
 */
void SDL3GameEngine::reset(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::reset()\n");
	GameEngine::reset();
}

/**
 * From GameEngine: update() - per-frame update
 */
void SDL3GameEngine::update(void)
{
	pollSDL3Events();
	GameEngine::update();
}

/**
 * From GameEngine: execute() - main game loop
 */
void SDL3GameEngine::execute(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::execute() - entering main loop\n");
	GameEngine::execute();
	fprintf(stderr, "INFO: SDL3GameEngine::execute() - exited main loop\n");
}

/**
 * From GameEngine: serviceWindowsOS() - native OS service
 * On Linux, process SDL3 events
 */
void SDL3GameEngine::serviceWindowsOS(void)
{
	pollSDL3Events();
}

/**
 * Check if game has OS focus
 */
Bool SDL3GameEngine::isActive(void)
{
	return m_IsActive;
}

/**
 * Set OS focus status
 */
void SDL3GameEngine::setIsActive(Bool isActive)
{
	m_IsActive = isActive;
}

/**
 * Poll and process SDL3 events
 * Handles keyboard, mouse, window, and quit events
 */
void SDL3GameEngine::pollSDL3Events(void)
{
	if (!m_SDLWindow) {
		return;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				m_quitting = true;
				break;

			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				m_quitting = true;
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				m_IsActive = true;
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				m_IsActive = false;
				break;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				handleKeyboardEvent(event.key);
				break;

			case SDL_EVENT_MOUSE_MOTION:
				handleMouseMotionEvent(event.motion);
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				handleMouseButtonEvent(event.button);
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				handleWindowEvent(event.window);
				break;

			default:
				// Ignore other events for now
				break;
		}
	}
}

/**
 * Handle keyboard event - dispatch to Keyboard manager
 * TODO: Phase 2 - Wire to actual Keyboard manager
 */
void SDL3GameEngine::handleKeyboardEvent(const SDL_KeyboardEvent& event)
{
	// TODO: Phase 2 - Get Keyboard manager from subsystems and dispatch event
	// fprintf(stderr, "DEBUG: Keyboard event (keycode=%d, pressed=%s)\n", event.keysym.scancode, event.down ? "true" : "false");
}

/**
 * Handle mouse motion event - dispatch to Mouse manager
 * TODO: Phase 2 - Wire to actual Mouse manager
 */
void SDL3GameEngine::handleMouseMotionEvent(const SDL_MouseMotionEvent& event)
{
	// TODO: Phase 2 - Get Mouse manager and dispatch motion event
	// fprintf(stderr, "DEBUG: Mouse motion (x=%d, y=%d, rel_x=%d, rel_y=%d)\n", 
	//        event.x, event.y, event.rel_x, event.rel_y);
}

/**
 * Handle mouse button event - dispatch to Mouse manager
 * TODO: Phase 2 - Wire to actual Mouse manager
 */
void SDL3GameEngine::handleMouseButtonEvent(const SDL_MouseButtonEvent& event)
{
	// TODO: Phase 2 - Get Mouse manager and dispatch button event
	// fprintf(stderr, "DEBUG: Mouse button (button=%d, pressed=%s)\n", event.button, event.down ? "true" : "false");
}

/**
 * Handle window event (resize, etc.)
 */
void SDL3GameEngine::handleWindowEvent(const SDL_WindowEvent& event)
{
	// TODO: Phase 2 - Handle window resize, notify graphics subsystem
	// fprintf(stderr, "DEBUG: Window event (type=%d)\n", event.type);
}

/**
 * Factory method: LocalFileSystem
 * TODO: Phase 2 - Return SDL3-compatible LocalFileSystem
 */
LocalFileSystem *SDL3GameEngine::createLocalFileSystem(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createLocalFileSystem() - stub\n");
	return GameEngine::createLocalFileSystem();  // Call parent for now
}

/**
 * Factory method: ArchiveFileSystem
 * TODO: Phase 2 - Return SDL3-compatible ArchiveFileSystem
 */
ArchiveFileSystem *SDL3GameEngine::createArchiveFileSystem(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createArchiveFileSystem() - stub\n");
	return GameEngine::createArchiveFileSystem();  // Call parent for now
}

/**
 * Factory method: GameLogic
 * TODO: Phase 2 - Return GameLogic subsystem
 */
GameLogic *SDL3GameEngine::createGameLogic(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createGameLogic() - stub\n");
	return GameEngine::createGameLogic();  // Call parent for now
}

/**
 * Factory method: GameClient
 * TODO: Phase 2 - Return GameClient subsystem
 */
GameClient *SDL3GameEngine::createGameClient(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createGameClient() - stub\n");
	return GameEngine::createGameClient();  // Call parent for now
}

/**
 * Factory method: ModuleFactory
 */
ModuleFactory *SDL3GameEngine::createModuleFactory(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createModuleFactory() - stub\n");
	return GameEngine::createModuleFactory();  // Call parent for now
}

/**
 * Factory method: ThingFactory
 */
ThingFactory *SDL3GameEngine::createThingFactory(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createThingFactory() - stub\n");
	return GameEngine::createThingFactory();  // Call parent for now
}

/**
 * Factory method: FunctionLexicon
 */
FunctionLexicon *SDL3GameEngine::createFunctionLexicon(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createFunctionLexicon() - stub\n");
	return GameEngine::createFunctionLexicon();  // Call parent for now
}

/**
 * Factory method: Radar
 */
Radar *SDL3GameEngine::createRadar(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createRadar() - stub\n");
	return GameEngine::createRadar();  // Call parent for now
}

/**
 * Factory method: WebBrowser
 */
WebBrowser *SDL3GameEngine::createWebBrowser(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createWebBrowser() - stub\n");
	return GameEngine::createWebBrowser();  // Call parent for now
}

/**
 * Factory method: ParticleSystemManager
 */
ParticleSystemManager* SDL3GameEngine::createParticleSystemManager(void)
{
	fprintf(stderr, "DEBUG: SDL3GameEngine::createParticleSystemManager() - stub\n");
	return GameEngine::createParticleSystemManager();  // Call parent for now
}

/**
 * Factory method: AudioManager
 * Select audio backend based on compile flags
 */
AudioManager *SDL3GameEngine::createAudioManager(void)
{
	fprintf(stderr, "INFO: SDL3GameEngine::createAudioManager()\n");

#ifdef SAGE_USE_OPENAL
	fprintf(stderr, "INFO: Creating OpenAL audio backend\n");
	return new OpenALAudioManager();
#else
	fprintf(stderr, "INFO: Audio backend not available (SAGE_USE_OPENAL not defined)\n");
	fprintf(stderr, "WARNING: Falls back to parent implementation or silent mode\n");
	return GameEngine::createAudioManager();  // Call parent (may return stub)
#endif
}

#endif // !_WIN32

