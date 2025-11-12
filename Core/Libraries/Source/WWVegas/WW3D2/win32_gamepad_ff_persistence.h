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
**  FILE: win32_gamepad_ff_persistence.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Force Feedback Profiles Persistence Layer
**
**  DESCRIPTION:
**    Persists force feedback profiles and patterns to Registry/INI.
**    Enables saving/loading of custom rumble configurations per gamepad.
**    Integrates with Phase 07 event system for combo-triggered feedback.
**
**  ARCHITECTURE:
**    Phase 07B Integration: Force Feedback System → Registry/INI Persistence
**    Phase 08B Integration: Combos can trigger custom FF patterns
**    
**    Registry Path:
**    HKEY_CURRENT_USER\Software\Electronic Arts\EA Games\{Game}\ForceFeedback\Gamepad{N}\Profile{M}
**
******************************************************************************/

#ifndef WIN32_GAMEPAD_FF_PERSISTENCE_H
#define WIN32_GAMEPAD_FF_PERSISTENCE_H

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_force_feedback.h"

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
 * FORCE FEEDBACK PERSISTENCE - INITIALIZATION & MANAGEMENT
 * ============================================================================ */

/**
 * Initialize force feedback persistence system
 * Loads all saved profiles from Registry/INI
 *
 * @return  0 on success, -1 on error
 */
int SDL2_InitFFPersistence(void);

/**
 * Shutdown force feedback persistence system
 * Flushes any unsaved profiles
 */
void SDL2_ShutdownFFPersistence(void);

/**
 * Save a force feedback profile to Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @param profile       Profile data to save
 * @return              0 on success, -1 on error
 */
int SDL2_SaveFFProfileToConfig(int gamepad_index, int profile_id, const GamepadFFProfile *profile);

/**
 * Load a force feedback profile from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @param profile       Output profile data
 * @return              0 on success, -1 if not found
 */
int SDL2_LoadFFProfileFromConfig(int gamepad_index, int profile_id, GamepadFFProfile *profile);

/**
 * Delete a force feedback profile from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @return              0 on success, -1 on error
 */
int SDL2_DeleteFFProfileFromConfig(int gamepad_index, int profile_id);

/**
 * Save all force feedback profiles for a gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_SaveAllFFProfilesForGamepad(int gamepad_index);

/**
 * Load all force feedback profiles for a gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_LoadAllFFProfilesForGamepad(int gamepad_index);

/* ============================================================================
 * FORCE FEEDBACK PATTERN PERSISTENCE
 * ============================================================================ */

/**
 * Save a force feedback pattern to Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param pattern_id    Pattern slot (0-31)
 * @param pattern       Pattern data to save
 * @return              0 on success, -1 on error
 */
int SDL2_SaveFFPatternToConfig(int gamepad_index, int pattern_id, const GamepadFFPattern *pattern);

/**
 * Load a force feedback pattern from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param pattern_id    Pattern slot (0-31)
 * @param pattern       Output pattern data
 * @return              0 on success, -1 if not found
 */
int SDL2_LoadFFPatternFromConfig(int gamepad_index, int pattern_id, GamepadFFPattern *pattern);

/**
 * Delete a force feedback pattern from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param pattern_id    Pattern slot (0-31)
 * @return              0 on success, -1 on error
 */
int SDL2_DeleteFFPatternFromConfig(int gamepad_index, int pattern_id);

/* ============================================================================
 * PROFILE NAMING & METADATA
 * ============================================================================ */

/**
 * Set force feedback profile name
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @param name          Profile name (max 128 chars)
 * @return              0 on success
 */
int SDL2_SetFFProfileName(int gamepad_index, int profile_id, const char *name);

/**
 * Get force feedback profile name
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @param name          Output buffer (min 128 bytes)
 * @param max_len       Size of output buffer
 * @return              0 on success, -1 if not found
 */
int SDL2_GetFFProfileName(int gamepad_index, int profile_id, char *name, int max_len);

/**
 * List all saved force feedback profiles for a gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              Number of saved profiles (0-16)
 */
int SDL2_GetSavedFFProfileCount(int gamepad_index);

/**
 * Get saved profile metadata by index
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_index Index (0 to count-1)
 * @param profile_id    Output profile slot (0-15)
 * @param name          Output profile name (min 128 bytes)
 * @param max_name_len  Size of name buffer
 * @return              0 on success, -1 on error
 */
int SDL2_GetSavedFFProfileInfo(int gamepad_index, int profile_index, int *profile_id,
                               char *name, int max_name_len);

/* ============================================================================
 * PROFILE EXPORT & IMPORT (Backup/Restore)
 * ============================================================================ */

/**
 * Export force feedback profiles to backup file
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param filepath      Output file path
 * @return              0 on success, -1 on error
 */
int SDL2_ExportFFProfilesToFile(int gamepad_index, const char *filepath);

/**
 * Import force feedback profiles from backup file
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param filepath      Input file path
 * @return              0 on success, -1 on error
 */
int SDL2_ImportFFProfilesFromFile(int gamepad_index, const char *filepath);

/**
 * Export all force feedback profiles from all gamepads
 *
 * @param filepath File path for unified backup
 * @return         0 on success, -1 on error
 */
int SDL2_ExportAllFFProfilesPersistence(const char *filepath);

/**
 * Import all force feedback profiles to all gamepads
 *
 * @param filepath File path for unified backup
 * @return         0 on success, -1 on error
 */
int SDL2_ImportAllFFProfilesPersistence(const char *filepath);

/* ============================================================================
 * VALIDATION & DIAGNOSTICS
 * ============================================================================ */

/**
 * Validate force feedback profile integrity
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param profile_id    Profile slot (0-15)
 * @return              1 if valid, 0 if invalid or not found
 */
int SDL2_ValidateFFProfileIntegrity(int gamepad_index, int profile_id);

/**
 * Dump force feedback configuration for debugging
 *
 * @param gamepad_index Gamepad index (0-3), or -1 for all
 */
void SDL2_DumpFFConfig(int gamepad_index);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_GAMEPAD_FF_PERSISTENCE_H */
