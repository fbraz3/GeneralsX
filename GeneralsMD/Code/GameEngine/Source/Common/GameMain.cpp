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


/**
 * This is the entry point for the game system.
 */
Int GameMain()
{
	int exitcode = 0;
	fprintf(stderr, "GameMain(): Starting\n");
	fflush(stderr);

	// initialize the game engine using factory function
	TheFramePacer = new FramePacer();
	fprintf(stderr, "GameMain(): FramePacer created\n");
	fflush(stderr);

	TheFramePacer->enableFramesPerSecondLimit(TRUE);
	TheGameEngine = CreateGameEngine();
	fprintf(stderr, "GameMain(): GameEngine created: %p\n", (void*)TheGameEngine);
	fflush(stderr);

	fprintf(stderr, "GameMain(): Calling TheGameEngine->init()...\n");
	fflush(stderr);
	TheGameEngine->init();
	fprintf(stderr, "GameMain(): TheGameEngine->init() completed\n");
	fflush(stderr);

	if (!TheGlobalData->m_simulateReplays.empty())
	{
		fprintf(stderr, "GameMain(): Simulating replays\n");
		fflush(stderr);
		exitcode = ReplaySimulation::simulateReplays(TheGlobalData->m_simulateReplays, TheGlobalData->m_simulateReplayJobs);
	}
	else
	{
		// run it
		fprintf(stderr, "GameMain(): Calling TheGameEngine->execute()...\n");
		fflush(stderr);
		TheGameEngine->execute();
		fprintf(stderr, "GameMain(): TheGameEngine->execute() completed\n");
		fflush(stderr);
	}

	// since execute() returned, we are exiting the game
	fprintf(stderr, "GameMain(): Cleaning up\n");
	fflush(stderr);

	delete TheFramePacer;
	TheFramePacer = NULL;
	delete TheGameEngine;
	TheGameEngine = NULL;

	fprintf(stderr, "GameMain(): Returning exitcode = %d\n", exitcode);
	fflush(stderr);

	return exitcode;
}

