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
**  FILE: win32_gamepad_macro_persistence.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Macro Recording Persistence Layer
**
**  DESCRIPTION:
**    Persists gamepad macros to Registry/INI via Phase 05 config system.
**    Enables saving/loading of recorded input sequences per gamepad.
**    Integrates with Phase 07 event system for combo detection triggers.
**
**  ARCHITECTURE:
**    Phase 07B Integration: Macro System → Registry/INI Persistence
**    Phase 08B Integration: Combos can trigger saved macros
**    
**    Registry Path:
**    HKEY_CURRENT_USER\Software\Electronic Arts\EA Games\{Game}\Macros\Gamepad{N}\Macro{M}
**
******************************************************************************/

#ifndef WIN32_GAMEPAD_MACRO_PERSISTENCE_H
#define WIN32_GAMEPAD_MACRO_PERSISTENCE_H

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_macro_system.h"

/* Forward declarations for Registry API - defined in win32_config_compat.h
   We avoid including config_compat.h here to prevent windows.h from being pulled in */
typedef void* HKEY;
typedef unsigned long DWORD;
typedef unsigned char BYTE;

#define HKEY_CURRENT_USER ((HKEY)(long)0x80000001)
#define KEY_READ          0x20019
#define KEY_WRITE         0x20006
#define REG_SZ            1
#define REG_DWORD         4
#define REG_BINARY        3

/* Forward declarations of config functions */
int SDL2_RegCreateKeyEx(HKEY hkey, const char *subkey, DWORD reserved, void *class_ptr,
                       DWORD options, DWORD sam, void *sec_attr, HKEY *phkresult, DWORD *disp);
int SDL2_RegOpenKeyEx(HKEY hkey, const char *subkey, DWORD reserved, DWORD sam, HKEY *phkresult);
int SDL2_RegCloseKey(HKEY hkey);
int SDL2_RegSetValueEx(HKEY hkey, const char *value_name, DWORD reserved, DWORD type,
                       const BYTE *data, DWORD cb_data);
int SDL2_RegQueryValueEx(HKEY hkey, const char *value_name, DWORD *reserved, DWORD *type,
                         BYTE *data, DWORD *cb_data);
int SDL2_RegDeleteKey(HKEY hkey, const char *subkey);

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MACRO PERSISTENCE - INITIALIZATION & MANAGEMENT
 * ============================================================================ */

/**
 * Initialize macro persistence system
 * Loads all saved macros from Registry/INI into memory
 *
 * @return  0 on success, -1 on error
 */
int SDL2_InitMacroPersistence(void);

/**
 * Shutdown macro persistence system
 * Flushes any unsaved macros to Registry/INI
 */
void SDL2_ShutdownMacroPersistence(void);

/**
 * Save a macro to Registry/INI
 * Persists recorded input sequence with metadata
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @param macro         Macro data to save
 * @return              0 on success, -1 on error
 */
int SDL2_SaveMacroToConfig(int gamepad_index, int macro_id, const GamepadMacro *macro);

/**
 * Load a macro from Registry/INI
 * Restores recorded input sequence
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @param macro         Output macro data
 * @return              0 on success, -1 if not found
 */
int SDL2_LoadMacroFromConfig(int gamepad_index, int macro_id, GamepadMacro *macro);

/**
 * Delete a macro from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @return              0 on success, -1 on error
 */
int SDL2_DeleteMacroFromConfig(int gamepad_index, int macro_id);

/**
 * Save all macros for a gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_SaveAllMacrosForGamepad(int gamepad_index);

/**
 * Load all macros for a gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_LoadAllMacrosForGamepad(int gamepad_index);

/* ============================================================================
 * MACRO NAMING & METADATA
 * ============================================================================ */

/**
 * Set macro name/description
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @param name          Macro name (max 128 chars)
 * @return              0 on success
 */
int SDL2_SetMacroName(int gamepad_index, int macro_id, const char *name);

/**
 * Get macro name/description
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @param name          Output buffer (min 128 bytes)
 * @param max_len       Size of output buffer
 * @return              0 on success, -1 if not found
 */
int SDL2_GetMacroName(int gamepad_index, int macro_id, char *name, int max_len);

/**
 * List all saved macros for a gamepad
 * Returns count of saved macros
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              Number of saved macros (0-16)
 */
int SDL2_GetSavedMacroCount(int gamepad_index);

/**
 * Get saved macro metadata by index
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_index   Index (0 to count-1)
 * @param macro_id      Output macro slot (0-15)
 * @param name          Output macro name (min 128 bytes)
 * @param max_name_len  Size of name buffer
 * @param event_count   Output event count in macro
 * @return              0 on success, -1 on error
 */
int SDL2_GetSavedMacroInfo(int gamepad_index, int macro_index, int *macro_id,
                          char *name, int max_name_len, int *event_count);

/* ============================================================================
 * MACRO EXPORT & IMPORT (Backup/Restore)
 * ============================================================================ */

/**
 * Export macros to backup file (text format)
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param filepath      Output file path
 * @return              0 on success, -1 on error
 */
int SDL2_ExportMacrosToFile(int gamepad_index, const char *filepath);

/**
 * Import macros from backup file
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param filepath      Input file path
 * @return              0 on success, -1 on error
 */
int SDL2_ImportMacrosFromFile(int gamepad_index, const char *filepath);

/**
 * Export all macros from all gamepads
 *
 * @param filepath File path for unified backup
 * @return         0 on success, -1 on error
 */
int SDL2_ExportAllMacrosPersistence(const char *filepath);

/**
 * Import all macros to all gamepads
 *
 * @param filepath File path for unified backup
 * @return         0 on success, -1 on error
 */
int SDL2_ImportAllMacrosPersistence(const char *filepath);

/* ============================================================================
 * MACRO VALIDATION & DIAGNOSTICS
 * ============================================================================ */

/**
 * Validate macro data integrity
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param macro_id      Macro slot (0-15)
 * @return              1 if valid, 0 if invalid or not found
 */
int SDL2_ValidateMacroIntegrity(int gamepad_index, int macro_id);

/**
 * Dump macro configuration for debugging
 *
 * @param gamepad_index Gamepad index (0-3), or -1 for all
 */
void SDL2_DumpMacroConfig(int gamepad_index);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_GAMEPAD_MACRO_PERSISTENCE_H */
