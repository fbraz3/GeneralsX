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
**  FILE: win32_gamepad_macro_persistence.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Macro Recording Persistence Implementation
**
**  DESCRIPTION:
**    Implements macro save/load via Registry/INI using Phase 05 config layer.
**    Serializes GamepadMacroEvent arrays to Registry for later playback.
**    Provides backup/restore functionality for user-recorded sequences.
**
******************************************************************************/

#include "win32_gamepad_macro_persistence.h"
#include "win32_thread_compat.h"
/* #include "win32_config_compat.h" - Included via macro_persistence.h, forward declarations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * INTERNAL STRUCTURES
 * ============================================================================ */

typedef struct {
    int gamepad_index;
    int macro_id;
    char macro_name[128];
    int event_count;
    GamepadMacro macro_data;
} MacroCacheEntry;

typedef struct {
    MacroCacheEntry entries[32];  /* 4 gamepads × 8 macros each */
    int entry_count;
    SDL2_CriticalSection macro_lock;
    BOOL initialized;
} MacroPersistenceSystem;

static MacroPersistenceSystem g_macro_persistence = { 0 };

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitMacroPersistence(void)
{
    if (g_macro_persistence.initialized) {
        return 0;  /* Already initialized */
    }

    /* Create critical section for thread-safe access */
    g_macro_persistence.macro_lock = SDL2_CreateCriticalSection();

    g_macro_persistence.entry_count = 0;
    memset(g_macro_persistence.entries, 0, sizeof(g_macro_persistence.entries));
    g_macro_persistence.initialized = TRUE;

    printf("Macro persistence system initialized\n");
    return 0;
}

void SDL2_ShutdownMacroPersistence(void)
{
    if (!g_macro_persistence.initialized) {
        return;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        /* Flush any unsaved macros - already saved on individual save calls */
        g_macro_persistence.entry_count = 0;
        memset(g_macro_persistence.entries, 0, sizeof(g_macro_persistence.entries));
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    g_macro_persistence.initialized = FALSE;

    printf("Macro persistence system shut down\n");
}

/* ============================================================================
 * MACRO PERSISTENCE - SAVE/LOAD/DELETE
 * ============================================================================ */

int SDL2_SaveMacroToConfig(int gamepad_index, int macro_id, const GamepadMacro *macro)
{
    HKEY hkey = NULL;
    char key_path[512];
    char value_name[256];
    int result = 0;

    if (!g_macro_persistence.initialized) {
        printf("ERROR: Macro persistence not initialized\n");
        return -1;
    }

    if (gamepad_index < 0 || gamepad_index >= 4) {
        printf("ERROR: Invalid gamepad index %d\n", gamepad_index);
        return -1;
    }

    if (macro_id < 0 || macro_id >= 16) {
        printf("ERROR: Invalid macro id %d\n", macro_id);
        return -1;
    }

    if (macro == NULL) {
        printf("ERROR: Macro data is NULL\n");
        return -1;
    }

    /* Create registry key path: Gamepads\Gamepad{N}\Macros\Macro{M} */
    snprintf(key_path, sizeof(key_path),
             "Software\\Electronic Arts\\EA Games\\Generals\\Macros\\Gamepad%d\\Macro%d",
             gamepad_index, macro_id);

    /* Create/open registry key */
    if (SDL2_RegCreateKeyEx(HKEY_CURRENT_USER, key_path, 0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL) != 0) {
        printf("ERROR: Failed to create registry key for macro %d:%d\n", gamepad_index, macro_id);
        return -1;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        /* Save macro event count */
        snprintf(value_name, sizeof(value_name), "EventCount");
        if (SDL2_RegSetValueEx(hkey, value_name, 0, REG_DWORD, (BYTE *)&macro->event_count,
                              sizeof(macro->event_count)) != 0) {
            result = -1;
        }

        /* Save macro name */
        if (result == 0 && macro->name[0] != '\0') {
            snprintf(value_name, sizeof(value_name), "Name");
            if (SDL2_RegSetValueEx(hkey, value_name, 0, REG_SZ, (BYTE *)macro->name,
                                  strlen(macro->name) + 1) != 0) {
                result = -1;
            }
        }

        /* Save macro enabled flag */
        if (result == 0) {
            snprintf(value_name, sizeof(value_name), "Enabled");
            DWORD enabled = macro->enabled ? 1 : 0;
            if (SDL2_RegSetValueEx(hkey, value_name, 0, REG_DWORD, (BYTE *)&enabled,
                                  sizeof(enabled)) != 0) {
                result = -1;
            }
        }

        /* Save events array as binary data */
        if (result == 0 && macro->event_count > 0) {
            size_t events_size = macro->event_count * sizeof(GamepadMacroEvent);
            snprintf(value_name, sizeof(value_name), "Events");
            if (SDL2_RegSetValueEx(hkey, value_name, 0, REG_BINARY, (BYTE *)macro->events,
                                  (DWORD)events_size) != 0) {
                result = -1;
            }
        }

        if (result == 0) {
            printf("Saved macro %d:%d with %d events to config\n", gamepad_index, macro_id, macro->event_count);
        }
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    SDL2_RegCloseKey(hkey);
    return result;
}

int SDL2_LoadMacroFromConfig(int gamepad_index, int macro_id, GamepadMacro *macro)
{
    HKEY hkey = NULL;
    char key_path[512];
    char value_name[256];
    BYTE buffer[8192];
    DWORD buffer_size = sizeof(buffer);
    DWORD event_count = 0;
    int result = 0;

    if (!g_macro_persistence.initialized) {
        printf("ERROR: Macro persistence not initialized\n");
        return -1;
    }

    if (gamepad_index < 0 || gamepad_index >= 4) {
        printf("ERROR: Invalid gamepad index %d\n", gamepad_index);
        return -1;
    }

    if (macro_id < 0 || macro_id >= 16) {
        printf("ERROR: Invalid macro id %d\n", macro_id);
        return -1;
    }

    if (macro == NULL) {
        printf("ERROR: Output macro is NULL\n");
        return -1;
    }

    memset(macro, 0, sizeof(GamepadMacro));

    /* Open registry key */
    snprintf(key_path, sizeof(key_path),
             "Software\\Electronic Arts\\EA Games\\Generals\\Macros\\Gamepad%d\\Macro%d",
             gamepad_index, macro_id);

    if (SDL2_RegOpenKeyEx(HKEY_CURRENT_USER, key_path, 0, KEY_READ, &hkey) != 0) {
        /* Key doesn't exist - return not found (not an error) */
        return -1;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        /* Load event count */
        buffer_size = sizeof(DWORD);
        snprintf(value_name, sizeof(value_name), "EventCount");
        if (SDL2_RegQueryValueEx(hkey, value_name, NULL, NULL, buffer, &buffer_size) == 0) {
            event_count = *(DWORD *)buffer;
            macro->event_count = (int)event_count;
        }

        /* Load name */
        buffer_size = sizeof(buffer);
        snprintf(value_name, sizeof(value_name), "Name");
        if (SDL2_RegQueryValueEx(hkey, value_name, NULL, NULL, buffer, &buffer_size) == 0) {
            strncpy(macro->name, (char *)buffer, sizeof(macro->name) - 1);
            macro->name[sizeof(macro->name) - 1] = '\0';
        }

        /* Load enabled flag */
        buffer_size = sizeof(DWORD);
        snprintf(value_name, sizeof(value_name), "Enabled");
        if (SDL2_RegQueryValueEx(hkey, value_name, NULL, NULL, buffer, &buffer_size) == 0) {
            macro->enabled = (*(DWORD *)buffer != 0);
        }

        /* Load events */
        if (macro->event_count > 0 && macro->event_count <= GAMEPAD_MACRO_MAX_FRAMES) {
            buffer_size = sizeof(buffer);
            snprintf(value_name, sizeof(value_name), "Events");
            if (SDL2_RegQueryValueEx(hkey, value_name, NULL, NULL, buffer, &buffer_size) == 0) {
                size_t events_size = macro->event_count * sizeof(GamepadMacroEvent);
                if (buffer_size == events_size) {
                    memcpy(macro->events, buffer, events_size);
                    printf("Loaded macro %d:%d with %d events from config\n", gamepad_index, macro_id, macro->event_count);
                } else {
                    printf("WARNING: Event buffer size mismatch for macro %d:%d\n", gamepad_index, macro_id);
                    macro->event_count = 0;
                    result = -1;
                }
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    SDL2_RegCloseKey(hkey);
    return result;
}

int SDL2_DeleteMacroFromConfig(int gamepad_index, int macro_id)
{
    char key_path[512];
    int result = 0;

    if (!g_macro_persistence.initialized) {
        printf("ERROR: Macro persistence not initialized\n");
        return -1;
    }

    if (gamepad_index < 0 || gamepad_index >= 4) {
        return -1;
    }

    if (macro_id < 0 || macro_id >= 16) {
        return -1;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        snprintf(key_path, sizeof(key_path),
                 "Software\\Electronic Arts\\EA Games\\Generals\\Macros\\Gamepad%d\\Macro%d",
                 gamepad_index, macro_id);
        result = SDL2_RegDeleteKey(HKEY_CURRENT_USER, key_path);
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    if (result == 0) {
        printf("Deleted macro %d:%d from config\n", gamepad_index, macro_id);
    }

    return result;
}

int SDL2_SaveAllMacrosForGamepad(int gamepad_index)
{
    int macro_id;
    int result = 0;

    if (!g_macro_persistence.initialized) {
        return -1;
    }

    if (gamepad_index < 0 || gamepad_index >= 4) {
        return -1;
    }

    printf("Saving all macros for gamepad %d\n", gamepad_index);

    for (macro_id = 0; macro_id < 16; macro_id++) {
        /* Find macro in cache and save if populated */
        SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
        {
            for (int i = 0; i < g_macro_persistence.entry_count; i++) {
                MacroCacheEntry *entry = &g_macro_persistence.entries[i];
                if (entry->gamepad_index == gamepad_index && entry->macro_id == macro_id) {
                    if (SDL2_SaveMacroToConfig(gamepad_index, macro_id, &entry->macro_data) != 0) {
                        result = -1;
                    }
                    break;
                }
            }
        }
        SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);
    }

    return result;
}

int SDL2_LoadAllMacrosForGamepad(int gamepad_index)
{
    int macro_id;
    GamepadMacro macro = { 0 };

    if (!g_macro_persistence.initialized) {
        return -1;
    }

    if (gamepad_index < 0 || gamepad_index >= 4) {
        return -1;
    }

    printf("Loading all macros for gamepad %d\n", gamepad_index);

    for (macro_id = 0; macro_id < 16; macro_id++) {
        if (SDL2_LoadMacroFromConfig(gamepad_index, macro_id, &macro) == 0) {
            /* Successfully loaded macro - add to cache */
            SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
            {
                if (g_macro_persistence.entry_count < 32) {
                    MacroCacheEntry *entry = &g_macro_persistence.entries[g_macro_persistence.entry_count];
                    entry->gamepad_index = gamepad_index;
                    entry->macro_id = macro_id;
                    strncpy(entry->macro_name, macro.name, sizeof(entry->macro_name) - 1);
                    entry->event_count = macro.event_count;
                    memcpy(&entry->macro_data, &macro, sizeof(GamepadMacro));
                    g_macro_persistence.entry_count++;
                }
            }
            SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);
        }
    }

    return 0;
}

/* ============================================================================
 * MACRO NAMING & METADATA
 * ============================================================================ */

int SDL2_SetMacroName(int gamepad_index, int macro_id, const char *name)
{
    HKEY hkey = NULL;
    char key_path[512];

    if (gamepad_index < 0 || gamepad_index >= 4 || macro_id < 0 || macro_id >= 16) {
        return -1;
    }

    if (name == NULL || strlen(name) >= 128) {
        return -1;
    }

    snprintf(key_path, sizeof(key_path),
             "Software\\Electronic Arts\\EA Games\\Generals\\Macros\\Gamepad%d\\Macro%d",
             gamepad_index, macro_id);

    if (SDL2_RegOpenKeyEx(HKEY_CURRENT_USER, key_path, 0, KEY_WRITE, &hkey) != 0) {
        return -1;
    }

    int result = SDL2_RegSetValueEx(hkey, "Name", 0, REG_SZ, (BYTE *)name, strlen(name) + 1);
    SDL2_RegCloseKey(hkey);
    return result;
}

int SDL2_GetMacroName(int gamepad_index, int macro_id, char *name, int max_len)
{
    HKEY hkey = NULL;
    char key_path[512];
    DWORD buffer_size = max_len;
    BYTE buffer[256];
    int result = -1;

    if (gamepad_index < 0 || gamepad_index >= 4 || macro_id < 0 || macro_id >= 16) {
        return -1;
    }

    if (name == NULL || max_len < 2) {
        return -1;
    }

    snprintf(key_path, sizeof(key_path),
             "Software\\Electronic Arts\\EA Games\\Generals\\Macros\\Gamepad%d\\Macro%d",
             gamepad_index, macro_id);

    if (SDL2_RegOpenKeyEx(HKEY_CURRENT_USER, key_path, 0, KEY_READ, &hkey) == 0) {
        buffer_size = sizeof(buffer);
        if (SDL2_RegQueryValueEx(hkey, "Name", NULL, NULL, buffer, &buffer_size) == 0) {
            strncpy(name, (char *)buffer, max_len - 1);
            name[max_len - 1] = '\0';
            result = 0;
        }
        SDL2_RegCloseKey(hkey);
    }

    if (result != 0) {
        strncpy(name, "Unnamed", max_len - 1);
        name[max_len - 1] = '\0';
    }

    return result;
}

int SDL2_GetSavedMacroCount(int gamepad_index)
{
    int count = 0;

    if (!g_macro_persistence.initialized || gamepad_index < 0 || gamepad_index >= 4) {
        return 0;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        for (int i = 0; i < g_macro_persistence.entry_count; i++) {
            if (g_macro_persistence.entries[i].gamepad_index == gamepad_index) {
                count++;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    return count;
}

int SDL2_GetSavedMacroInfo(int gamepad_index, int macro_index, int *macro_id,
                          char *name, int max_name_len, int *event_count)
{
    int found_count = 0;

    if (gamepad_index < 0 || gamepad_index >= 4 || macro_index < 0) {
        return -1;
    }

    SDL2_EnterCriticalSection(&g_macro_persistence.macro_lock);
    {
        for (int i = 0; i < g_macro_persistence.entry_count; i++) {
            MacroCacheEntry *entry = &g_macro_persistence.entries[i];
            if (entry->gamepad_index == gamepad_index) {
                if (found_count == macro_index) {
                    *macro_id = entry->macro_id;
                    if (name != NULL && max_name_len > 0) {
                        strncpy(name, entry->macro_name, max_name_len - 1);
                        name[max_name_len - 1] = '\0';
                    }
                    if (event_count != NULL) {
                        *event_count = entry->event_count;
                    }
                    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);
                    return 0;
                }
                found_count++;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_macro_persistence.macro_lock);

    return -1;
}

/* ============================================================================
 * MACRO EXPORT & IMPORT
 * ============================================================================ */

int SDL2_ExportMacrosToFile(int gamepad_index, const char *filepath)
{
    FILE *f = NULL;
    int macro_id;

    if (gamepad_index < 0 || gamepad_index >= 4 || filepath == NULL) {
        return -1;
    }

    f = fopen(filepath, "w");
    if (f == NULL) {
        printf("ERROR: Cannot open file %s for writing\n", filepath);
        return -1;
    }

    fprintf(f, "# GeneralsX Macro Export - Gamepad %d\n", gamepad_index);
    fprintf(f, "# Format: each line is macro_id:event_count:name\n\n");

    for (macro_id = 0; macro_id < 16; macro_id++) {
        GamepadMacro macro = { 0 };
        if (SDL2_LoadMacroFromConfig(gamepad_index, macro_id, &macro) == 0 && macro.event_count > 0) {
            fprintf(f, "MACRO %d %d \"%s\"\n", macro_id, macro.event_count, macro.name);
            for (int i = 0; i < macro.event_count; i++) {
                fprintf(f, "  EVENT %d %d\n",
                       macro.events[i].type,
                       macro.events[i].frame_offset);
            }
        }
    }

    fclose(f);
    printf("Exported macros for gamepad %d to %s\n", gamepad_index, filepath);
    return 0;
}

int SDL2_ImportMacrosFromFile(int gamepad_index, const char *filepath)
{
    printf("TODO: Implement macro import from file for gamepad %d\n", gamepad_index);
    return -1;
}

int SDL2_ExportAllMacrosPersistence(const char *filepath)
{
    if (filepath == NULL) {
        return -1;
    }

    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        return -1;
    }

    fprintf(f, "# GeneralsX Complete Macro Backup\n\n");

    for (int gp = 0; gp < 4; gp++) {
        fprintf(f, "[Gamepad %d]\n", gp);
        SDL2_ExportMacrosToFile(gp, filepath);
    }

    fclose(f);
    return 0;
}

int SDL2_ImportAllMacrosPersistence(const char *filepath)
{
    printf("TODO: Implement complete macro import\n");
    return -1;
}

/* ============================================================================
 * MACRO VALIDATION & DIAGNOSTICS
 * ============================================================================ */

int SDL2_ValidateMacroIntegrity(int gamepad_index, int macro_id)
{
    GamepadMacro macro = { 0 };

    if (gamepad_index < 0 || gamepad_index >= 4 || macro_id < 0 || macro_id >= 16) {
        return 0;
    }

    if (SDL2_LoadMacroFromConfig(gamepad_index, macro_id, &macro) != 0) {
        return 0;  /* Not found */
    }

    /* Validate structure integrity */
    if (macro.event_count < 0 || macro.event_count > GAMEPAD_MACRO_MAX_FRAMES) {
        printf("WARNING: Macro %d:%d has invalid event count %d\n", gamepad_index, macro_id, macro.event_count);
        return 0;
    }

    if (macro.event_count == 0) {
        return 0;  /* Empty but valid */
    }

    /* Validate events */
    for (int i = 0; i < macro.event_count; i++) {
        GamepadMacroEvent *event = &macro.events[i];
        if (event->frame_offset > 10000) {
            printf("WARNING: Macro %d:%d event %d has invalid frame offset %d\n",
                   gamepad_index, macro_id, i, event->frame_offset);
            return 0;
        }
    }

    return 1;  /* Valid */
}

void SDL2_DumpMacroConfig(int gamepad_index)
{
    printf("\n=== MACRO CONFIGURATION DUMP ===\n");

    if (gamepad_index == -1) {
        /* Dump all gamepads */
        for (int gp = 0; gp < 4; gp++) {
            printf("\nGamepad %d:\n", gp);
            int count = SDL2_GetSavedMacroCount(gp);
            printf("  Saved macros: %d\n", count);

            for (int i = 0; i < count; i++) {
                int macro_id;
                char name[128];
                int event_count;
                if (SDL2_GetSavedMacroInfo(gp, i, &macro_id, name, sizeof(name), &event_count) == 0) {
                    printf("    Macro %d: \"%s\" (%d events)\n", macro_id, name, event_count);
                    if (SDL2_ValidateMacroIntegrity(gp, macro_id)) {
                        printf("      Status: VALID\n");
                    } else {
                        printf("      Status: INVALID\n");
                    }
                }
            }
        }
    } else if (gamepad_index >= 0 && gamepad_index < 4) {
        printf("\nGamepad %d:\n", gamepad_index);
        int count = SDL2_GetSavedMacroCount(gamepad_index);
        printf("  Saved macros: %d\n", count);

        for (int i = 0; i < count; i++) {
            int macro_id;
            char name[128];
            int event_count;
            if (SDL2_GetSavedMacroInfo(gamepad_index, i, &macro_id, name, sizeof(name), &event_count) == 0) {
                printf("    Macro %d: \"%s\" (%d events)\n", macro_id, name, event_count);
                if (SDL2_ValidateMacroIntegrity(gamepad_index, macro_id)) {
                    printf("      Status: VALID\n");
                } else {
                    printf("      Status: INVALID\n");
                }
            }
        }
    }

    printf("\n=== END MACRO DUMP ===\n\n");
}
