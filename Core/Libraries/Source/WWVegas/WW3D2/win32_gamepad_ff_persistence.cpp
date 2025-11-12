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

/******************************************************************************
**
**  FILE: win32_gamepad_ff_persistence.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Force Feedback Profiles Persistence Implementation
**
**  DESCRIPTION:
**    Implements profile save/load via Registry/INI using Phase 05 config layer.
**    Placeholder implementation for future integration with Phase 05 persistence.
**    Current version provides stub functions with logging for testing.
**
******************************************************************************/

#include "win32_gamepad_ff_persistence.h"
#include "win32_thread_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * INTERNAL STRUCTURES
 * ============================================================================ */

typedef struct {
    SDL2_CriticalSection ff_lock;
    BOOL initialized;
} FFPersistenceState;

static FFPersistenceState g_ff_persist_state = { 0 };

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitFFPersistence(void)
{
    if (g_ff_persist_state.initialized) {
        return 0;
    }

    g_ff_persist_state.ff_lock = SDL2_CreateCriticalSection();
    g_ff_persist_state.initialized = TRUE;

    printf("Force feedback persistence system initialized\n");
    return 0;
}

void SDL2_ShutdownFFPersistence(void)
{
    if (!g_ff_persist_state.initialized) {
        return;
    }

    g_ff_persist_state.initialized = FALSE;
    printf("Force feedback persistence system shut down\n");
}

/* ============================================================================
 * FORCE FEEDBACK PROFILE PERSISTENCE (Stub - Phase 05 integration pending)
 * ============================================================================ */

int SDL2_SaveFFProfileToConfig(int gamepad_index, int profile_id, const GamepadFFProfile *profile)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4 ||
        profile_id < 0 || profile_id >= 16 || profile == NULL) {
        return -1;
    }

    printf("TODO: Persist FF profile %d:%d to Registry/INI\n", gamepad_index, profile_id);
    return 0;
}

int SDL2_LoadFFProfileFromConfig(int gamepad_index, int profile_id, GamepadFFProfile *profile)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4 ||
        profile_id < 0 || profile_id >= 16 || profile == NULL) {
        return -1;
    }

    printf("TODO: Load FF profile %d:%d from Registry/INI\n", gamepad_index, profile_id);
    return -1;  /* Not found */
}

int SDL2_DeleteFFProfileFromConfig(int gamepad_index, int profile_id)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4 ||
        profile_id < 0 || profile_id >= 16) {
        return -1;
    }

    printf("TODO: Delete FF profile %d:%d from Registry/INI\n", gamepad_index, profile_id);
    return 0;
}

int SDL2_SaveAllFFProfilesForGamepad(int gamepad_index)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4) {
        return -1;
    }

    printf("Saving all FF profiles for gamepad %d\n", gamepad_index);
    return 0;
}

int SDL2_LoadAllFFProfilesForGamepad(int gamepad_index)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4) {
        return -1;
    }

    printf("Loading all FF profiles for gamepad %d\n", gamepad_index);
    return 0;
}

/* ============================================================================
 * FORCE FEEDBACK PATTERN PERSISTENCE
 * ============================================================================ */

int SDL2_SaveFFPatternToConfig(int gamepad_index, int pattern_id, const GamepadFFPattern *pattern)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4 ||
        pattern_id < 0 || pattern_id >= 32 || pattern == NULL) {
        return -1;
    }

    printf("TODO: Persist FF pattern %d:%d to Registry/INI\n", gamepad_index, pattern_id);
    return 0;
}

int SDL2_LoadFFPatternFromConfig(int gamepad_index, int pattern_id, GamepadFFPattern *pattern)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4 ||
        pattern_id < 0 || pattern_id >= 32 || pattern == NULL) {
        return -1;
    }

    printf("TODO: Load FF pattern %d:%d from Registry/INI\n", gamepad_index, pattern_id);
    return -1;  /* Not found */
}

int SDL2_DeleteFFPatternFromConfig(int gamepad_index, int pattern_id)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || pattern_id < 0 || pattern_id >= 32) {
        return -1;
    }

    printf("TODO: Delete FF pattern %d:%d from Registry/INI\n", gamepad_index, pattern_id);
    return 0;
}

/* ============================================================================
 * PROFILE NAMING & METADATA
 * ============================================================================ */

int SDL2_SetFFProfileName(int gamepad_index, int profile_id, const char *name)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || profile_id < 0 || profile_id >= 16 ||
        name == NULL || strlen(name) >= 64) {
        return -1;
    }

    printf("Set FF profile %d:%d name: \"%s\"\n", gamepad_index, profile_id, name);
    return 0;
}

int SDL2_GetFFProfileName(int gamepad_index, int profile_id, char *name, int max_len)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || profile_id < 0 || profile_id >= 16 ||
        name == NULL || max_len < 2) {
        return -1;
    }

    snprintf(name, max_len, "Profile_%d", profile_id);
    return 0;
}

int SDL2_GetSavedFFProfileCount(int gamepad_index)
{
    if (!g_ff_persist_state.initialized || gamepad_index < 0 || gamepad_index >= 4) {
        return 0;
    }

    return 0;  /* No saved profiles yet */
}

int SDL2_GetSavedFFProfileInfo(int gamepad_index, int profile_index, int *profile_id,
                               char *name, int max_name_len)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || profile_index < 0) {
        return -1;
    }

    return -1;  /* Not found */
}

/* ============================================================================
 * EXPORT & IMPORT
 * ============================================================================ */

int SDL2_ExportFFProfilesToFile(int gamepad_index, const char *filepath)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || filepath == NULL) {
        return -1;
    }

    printf("TODO: Export FF profiles for gamepad %d to %s\n", gamepad_index, filepath);
    return 0;
}

int SDL2_ImportFFProfilesFromFile(int gamepad_index, const char *filepath)
{
    printf("TODO: Import FF profiles from file\n");
    return -1;
}

int SDL2_ExportAllFFProfilesPersistence(const char *filepath)
{
    if (filepath == NULL) {
        return -1;
    }

    printf("TODO: Export all FF profiles to %s\n", filepath);
    return 0;
}

int SDL2_ImportAllFFProfilesPersistence(const char *filepath)
{
    printf("TODO: Import all FF profiles from file\n");
    return -1;
}

/* ============================================================================
 * VALIDATION & DIAGNOSTICS
 * ============================================================================ */

int SDL2_ValidateFFProfileIntegrity(int gamepad_index, int profile_id)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || profile_id < 0 || profile_id >= 16) {
        return 0;
    }

    return 0;  /* No profiles loaded yet */
}

void SDL2_DumpFFConfig(int gamepad_index)
{
    printf("\n=== FORCE FEEDBACK CONFIGURATION DUMP ===\n");

    if (gamepad_index == -1) {
        printf("All gamepads: 0 saved profiles\n");
    } else if (gamepad_index >= 0 && gamepad_index < 4) {
        printf("Gamepad %d: 0 saved profiles\n", gamepad_index);
    }

    printf("=== END FORCE FEEDBACK DUMP ===\n\n");
}
