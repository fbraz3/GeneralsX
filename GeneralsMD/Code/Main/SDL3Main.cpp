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
** Provides SDL3 window creation with Vulkan support for DXVK integration.
*/

#ifndef _WIN32

#include <SDL3/SDL.h>
#include <cstdlib>
#include <cstring>
#include "gameengine.h"

/**
 * SDL3Main_InitWindow
 *
 * Creates an SDL3 window with Vulkan support for DXVK.
 * Sets required environment variables for DXVK WSI configuration.
 *
 * @return SDL_Window pointer on success, nullptr on failure
 */
SDL_Window* SDL3Main_InitWindow(int width, int height, bool windowed)
{
	// Set DXVK WSI driver to SDL3 before loading Vulkan libraries
	setenv("DXVK_WSI_DRIVER", "SDL3", 1);

	// Load Vulkan library for SDL3
	if (!SDL_Vulkan_LoadLibrary(nullptr)) {
		fprintf(stderr, "ERROR: Failed to load Vulkan library for SDL3\n");
		return nullptr;
	}

	// Create SDL3 window with Vulkan support
	Uint32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
	if (!windowed) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	SDL_Window* window = SDL_CreateWindow(
		"Command & Conquer Generals: Zero Hour",
		width,
		height,
		flags
	);

	if (!window) {
		fprintf(stderr, "ERROR: Failed to create SDL3 window: %s\n", SDL_GetError());
		return nullptr;
	}

	return window;
}

/**
 * SDL3Main_Init
 *
 * Initialize SDL3 for the game.
 * Called once at startup.
 *
 * @return true on success, false on failure
 */
bool SDL3Main_Init()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		fprintf(stderr, "ERROR: Failed to initialize SDL3: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

/**
 * SDL3Main_Shutdown
 *
 * Clean up SDL3 resources.
 * Called on game shutdown.
 */
void SDL3Main_Shutdown()
{
	SDL_Quit();
}

#endif // !_WIN32
