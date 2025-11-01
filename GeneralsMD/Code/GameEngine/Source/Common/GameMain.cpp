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

// GameMain.cpp
// The main entry point for the game
// Author: Michael S. Booth, April 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FramePacer.h"
#include "Common/GameEngine.h"
#include "Common/ReplaySimulation.h"

// Phase 38: Graphics Backend Abstraction Layer
#include "../../Libraries/Source/WWVegas/WW3D2/graphics_backend.h"

// External window handle from WinMain
extern HWND ApplicationHWnd;


/**
 * This is the entry point for the game system.
 */
Int GameMain()
{
	int exitcode = 0;
	printf("GameMain - Starting game engine initialization\n");
	
	// initialize the game engine using factory function
	printf("GameMain - Creating game engine...\n");
	TheFramePacer = new FramePacer();
	TheFramePacer->enableFramesPerSecondLimit(TRUE);
	TheGameEngine = CreateGameEngine();
	printf("GameMain - Game engine created successfully\n");
	
	// Phase 38.3: Initialize graphics backend abstraction layer
	printf("GameMain - Initializing graphics backend...\n");
	fflush(stdout);
	
	// Step 1: Check window handle
	printf("GameMain - DEBUG: ApplicationHWnd=%p\n", (void*)ApplicationHWnd);
	fflush(stdout);
	
	if (ApplicationHWnd == nullptr) {
		printf("GameMain - ERROR: ApplicationHWnd is NULL! Window was not created properly.\n");
		fflush(stdout);
		return 1;
	}
	
	// Step 2: Create backend instance
	printf("GameMain - Calling CreateGraphicsBackend()...\n");
	fflush(stdout);
	HRESULT backend_create_hr = CreateGraphicsBackend();
	if (backend_create_hr != 0) {
		printf("GameMain - FATAL: Failed to create graphics backend (0x%08lx)\n", (unsigned long)backend_create_hr);
		fflush(stdout);
		delete TheFramePacer;
		TheFramePacer = NULL;
		delete TheGameEngine;
		TheGameEngine = NULL;
		return 1;
	}
	printf("GameMain - Graphics backend instance created: %p\n", (void*)g_graphicsBackend);
	fflush(stdout);
	
	// Step 3: Set window handle
	printf("GameMain - Setting window handle: %p\n", ApplicationHWnd);
	fflush(stdout);
	g_graphicsBackend->SetWindowHandle(ApplicationHWnd);
	printf("GameMain - Window handle set successfully\n");
	fflush(stdout);
	
	// Step 4: Initialize backend
	printf("GameMain - Calling InitializeGraphicsBackendNow()...\n");
	fflush(stdout);
	HRESULT backend_init_hr = InitializeGraphicsBackendNow();
	if (backend_init_hr != 0) {
		printf("GameMain - FATAL: Failed to initialize graphics backend (0x%08lx)\n", (unsigned long)backend_init_hr);
		fflush(stdout);
		delete TheFramePacer;
		TheFramePacer = NULL;
		delete TheGameEngine;
		TheGameEngine = NULL;
		return 1; // Error exit code
	}
	
	HRESULT backend_hr = g_graphicsBackend->Initialize();
	if (backend_hr != 0) { // S_OK = 0
		printf("GameMain - FATAL: Failed to initialize graphics backend (0x%08lx)\n", backend_hr);
		delete TheFramePacer;
		TheFramePacer = NULL;
		delete TheGameEngine;
		TheGameEngine = NULL;
		return 1; // Error exit code
	}
	printf("GameMain - Graphics backend initialized successfully\n");
	
	printf("GameMain - About to call TheGameEngine->init()\n");
	fflush(stdout);
	TheGameEngine->init();
	printf("GameMain - TheGameEngine->init() completed successfully\n");

	if (!TheGlobalData->m_simulateReplays.empty())
	{
		exitcode = ReplaySimulation::simulateReplays(TheGlobalData->m_simulateReplays, TheGlobalData->m_simulateReplayJobs);
	}
	else
	{
		// run it
		printf("GameMain - About to call TheGameEngine->execute()\n");
		TheGameEngine->execute();
		printf("GameMain - TheGameEngine->execute() completed\n");
	}

	// since execute() returned, we are exiting the game
	printf("GameMain - Shutting down graphics backend...\n");
	ShutdownGraphicsBackend();
	printf("GameMain - Graphics backend shut down\n");
	
	printf("GameMain - Deleting game engine\n");
	delete TheFramePacer;
	TheFramePacer = NULL;
	delete TheGameEngine;
	TheGameEngine = NULL;

	printf("GameMain - Returning exit code: %d\n", exitcode);
	return exitcode;
}

