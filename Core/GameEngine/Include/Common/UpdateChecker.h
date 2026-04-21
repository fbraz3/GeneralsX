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

// FILE: UpdateChecker.h //////////////////////////////////////////////////
// GeneralsX @feature BenderAI 21/04/2026 Non-blocking update checker via
// GitHub Releases API. Only active for tagged release builds with SAGE_USE_SDL3.

#pragma once

#ifdef SAGE_UPDATE_CHECK

// ---------------------------------------------------------------------------
// UpdateChecker
//
// Spawns a background SDL_Thread to query the GitHub Releases API and detect
// if a newer release tag is available. Only runs for tagged release builds
// (GitTag non-empty, no uncommitted changes).
//
// NOTE: SDL3 types are intentionally NOT exposed here to avoid propagating
// SDL3 headers to every translation unit that includes this header.
// All implementation state lives as file-statics in UpdateChecker.cpp.
// ---------------------------------------------------------------------------
class UpdateChecker
{
public:
    // Spawn the background check thread.
    // Safe to call multiple times: a second call is a no-op if already started.
    // Skips silently if: not a release build, m_checkForUpdates is false,
    // or SAGE_UPDATE_CHECK is not defined.
    static void start();

    // Poll for the result from the main thread (call from MainMenuUpdate).
    // Returns true and fills outLatestTag if a newer version was detected.
    // Returns false if still in progress or no update found.
    static bool poll(const char** outLatestTag);

    // GitHub repository used for the API query.
    static const char* getReleasesUrl();
};

#endif // SAGE_UPDATE_CHECK
