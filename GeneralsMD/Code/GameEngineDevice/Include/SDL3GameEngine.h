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
** SDL3GameEngine.h
**
** Implementation of GameEngine using SDL3 for windowing/input.
** Enabled when SAGE_USE_SDL3 is ON (cross-platform: Linux and Windows Modern).
**
** TheSuperHackers @feature CnC_Generals_Linux 07/02/2026
** Provides SDL3-based input and window management.
** Based on fighter19 reference implementation.
*/

#pragma once

#ifdef SAGE_USE_SDL3

#include "Common/GameEngine.h"
#include "Common/LocalFileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include <SDL3/SDL.h>

// EXTERNALS
// GeneralsX @feature felipebraz 16/02/2026
// SDL3 window created in SDL3Main.cpp before GameEngine instantiation (fighter19 pattern)
extern SDL_Window* TheSDL3Window;

// Forward declarations - full definitions in SDL3GameEngine.cpp
class StdLocalFileSystem;
class StdBIGFileSystem;
class W3DModuleFactory;
class W3DGameLogic;
class W3DGameClient;
class W3DWebBrowser;
class W3DFunctionLexicon;
class W3DRadar;
class W3DThingFactory;
class W3DParticleSystemManager;

// Forward declarations for base classes
class AudioManager;
class Mouse;
class Keyboard;

/**
 * SDL3GameEngine
 *
 * GameEngine subclass that uses SDL3 for windowing and input.
 * Replaces Win32-specific window handling with SDL3 on Linux.
 *
 * Features:
 * - SDL3 window creation with Vulkan support (for DXVK)
 * - SDL3 event polling integrated with game loop
 * - Factory methods for input (Mouse, Keyboard) and audio managers
 * - Compatible with existing GameEngine subsystems
 */
class SDL3GameEngine : public GameEngine
{
public:
	SDL3GameEngine();
	virtual ~SDL3GameEngine();

	// GameEngine interface
	virtual void init(void);
	virtual void reset(void);
	virtual void update(void);
	virtual void execute(void);
	virtual void serviceWindowsOS(void);
	virtual Bool isActive(void);
	virtual void setIsActive(Bool isActive);

	// Factory methods (override GameEngine)
	virtual LocalFileSystem *createLocalFileSystem(void);
	virtual ArchiveFileSystem *createArchiveFileSystem(void);
	virtual GameLogic *createGameLogic(void);
	virtual GameClient *createGameClient(void);
	virtual ModuleFactory *createModuleFactory(void);
	virtual ThingFactory *createThingFactory(void);
	virtual FunctionLexicon *createFunctionLexicon(void);
	virtual Radar *createRadar(void);
	virtual WebBrowser *createWebBrowser(void);
	virtual ParticleSystemManager* createParticleSystemManager(void);
	virtual AudioManager *createAudioManager(void);

	// SDL3 specific
	virtual SDL_Window* getSDLWindow(void) const { return m_SDLWindow; }

protected:
	SDL_Window*		m_SDLWindow;
	Bool			m_IsInitialized;
	Bool			m_IsActive;

	// Event processing
	void pollSDL3Events(void);
	void handleKeyboardEvent(const SDL_KeyboardEvent& event);
	void handleMouseMotionEvent(const SDL_MouseMotionEvent& event);
	void handleMouseButtonEvent(const SDL_MouseButtonEvent& event);
	void handleMouseWheelEvent(const SDL_MouseWheelEvent& event);  //TheSuperHackers @build 10/02/2026 Bender
	void handleWindowEvent(const SDL_WindowEvent& event);
};

#endif // SAGE_USE_SDL3
