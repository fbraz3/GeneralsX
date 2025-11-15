/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// POSIX/Unix entry point for GeneralsX Zero Hour on macOS/Linux

#include "Common/GameEngine.h"
#include "Common/GameText.h"
#include "Common/GameMain.h"
#include "Common/GlobalData.h"
#include <cstdlib>

extern GameEngine *TheGameEngine;
extern GameMain *TheGameMain;

/**
 * POSIX/Unix main entry point
 * Initializes game engine and runs main game loop
 */
int main(int argc, char* argv[])
{
    // Initialize game engine
    if (!TheGameEngine) {
        fprintf(stderr, "ERROR: Game Engine not initialized\n");
        return EXIT_FAILURE;
    }

    // Create and initialize game main
    if (!TheGameMain) {
        fprintf(stderr, "ERROR: Game Main not initialized\n");
        return EXIT_FAILURE;
    }

    try {
        // Initialize game systems
        if (!TheGameMain->Init()) {
            fprintf(stderr, "ERROR: Failed to initialize game\n");
            return EXIT_FAILURE;
        }

        // Main game loop
        while (TheGameMain->isGameRunning()) {
            TheGameMain->Update();
            TheGameMain->Render();
        }

        // Cleanup
        TheGameMain->Shutdown();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        fprintf(stderr, "FATAL ERROR: %s\n", e.what());
        return EXIT_FAILURE;
    }
    catch (...) {
        fprintf(stderr, "FATAL ERROR: Unknown exception\n");
        return EXIT_FAILURE;
    }
}
