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
**  FILE: win32_gamepad_config_compat.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Gamepad Configuration Persistence Layer
**
**  DESCRIPTION:
**    Persists gamepad mappings and settings via Phase 05 Registry/INI.
**    Enables per-device custom bindings, sensitivity tuning, and fire thresholds.
**    Automatically loads/saves configuration on gamepad connect/disconnect.
**
**  ARCHITECTURE:
**    Phase 07: Event Queue Integration & Configuration Persistence
**    - Part A: Event Integration (win32_gamepad_event_integration.h/cpp)
**    - Part B: Configuration Persistence (this file + .cpp)
**    
**    Integration with Phase 05:
**    Gamepad Config → Registry Keys → INI Files (platform-specific directories)
**    
**    Registry Path:
**    HKEY_CURRENT_USER\Software\Electronic Arts\EA Games\{Game}\Gamepads\Gamepad{N}
**
******************************************************************************/

#ifndef WIN32_GAMEPAD_CONFIG_COMPAT_H
#define WIN32_GAMEPAD_CONFIG_COMPAT_H

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_config_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * GAMEPAD CONFIGURATION STRUCTURE
 * ============================================================================ */

/**
 * Complete gamepad configuration (persisted to INI)
 */
typedef struct {
    /* Device identification */
    DWORD vendor_id;
    DWORD product_id;
    char device_name[256];
    
    /* Button mappings (VK_* codes) */
    BYTE button_to_vkey[SDL2_GAMEPAD_BUTTON_COUNT];
    
    /* Analog stick configuration */
    float stick_sensitivity[2];      /* Left & right stick */
    float stick_deadzone[2];         /* Custom deadzone per stick */
    BOOL stick_invert_y[2];          /* Invert Y axis */
    
    /* Trigger configuration */
    float trigger_fire_threshold[2]; /* Left & right trigger */
    BOOL trigger_fire_enabled[2];
    float trigger_sensitivity[2];    /* Pressure sensitivity */
    
    /* Advanced */
    BOOL enabled;
    BOOL vibration_enabled;
} SDL2_GamepadConfig;

/* ============================================================================
 * CONFIGURATION INITIALIZATION & MANAGEMENT
 * ============================================================================ */

/**
 * Initialize gamepad configuration system
 * Loads all existing gamepad configs from Registry/INI
 *
 * @return  0 on success, -1 on error
 */
int SDL2_InitGamepadConfig(void);

/**
 * Shutdown gamepad configuration system
 */
void SDL2_ShutdownGamepadConfig(void);

/**
 * Load gamepad configuration from Registry/INI
 * Searches for device by vendor_id/product_id
 *
 * @param vendor_id  USB vendor ID
 * @param product_id USB product ID
 * @param config     Output configuration structure
 * @return           0 on success, -1 if not found (will use defaults)
 */
int SDL2_LoadGamepadConfigByDeviceID(DWORD vendor_id, DWORD product_id, SDL2_GamepadConfig *config);

/**
 * Load gamepad configuration by gamepad index
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param config        Output configuration structure
 * @return              0 on success, -1 if not found
 */
int SDL2_LoadGamepadConfig(int gamepad_index, SDL2_GamepadConfig *config);

/**
 * Save gamepad configuration to Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param config        Configuration to save
 * @return              0 on success, -1 on error
 */
int SDL2_SaveGamepadConfig(int gamepad_index, const SDL2_GamepadConfig *config);

/**
 * Delete gamepad configuration from Registry/INI
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_DeleteGamepadConfig(int gamepad_index);

/**
 * Get default gamepad configuration
 * Creates standard configuration for new gamepad
 *
 * @param config  Output configuration structure
 * @return        0 on success
 */
int SDL2_GetDefaultGamepadConfig(SDL2_GamepadConfig *config);

/* ============================================================================
 * CONFIGURATION QUERY & MODIFICATION
 * ============================================================================ */

/**
 * Get gamepad configuration for specific index
 * Returns current or default if not yet configured
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param config        Output configuration structure
 * @return              0 on success
 */
int SDL2_GetGamepadConfig(int gamepad_index, SDL2_GamepadConfig *config);

/**
 * Update gamepad configuration and persist
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param config        Configuration to set
 * @return              0 on success, -1 on error
 */
int SDL2_SetGamepadConfig(int gamepad_index, const SDL2_GamepadConfig *config);

/**
 * Set button mapping for gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param button        Button code (SDL2_GAMEPAD_BUTTON_*)
 * @param vkey          Virtual key code (VK_*)
 * @return              0 on success, -1 on error
 */
int SDL2_SetGamepadButtonMapping(int gamepad_index, int button, BYTE vkey);

/**
 * Get button mapping for gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param button        Button code (SDL2_GAMEPAD_BUTTON_*)
 * @return              Virtual key code (VK_*), or 0 if not mapped
 */
BYTE SDL2_GetGamepadButtonMapping(int gamepad_index, int button);

/* ============================================================================
 * SENSITIVITY & TRIGGER CONFIGURATION PERSISTENCE
 * ============================================================================ */

/**
 * Set analog stick sensitivity (persisted)
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param stick_id      0 for left, 1 for right
 * @param sensitivity   Multiplier (0.1-3.0)
 * @return              0 on success
 */
int SDL2_SetGamepadStickSensitivity(int gamepad_index, int stick_id, float sensitivity);

/**
 * Get analog stick sensitivity from config
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param stick_id      0 for left, 1 for right
 * @return              Sensitivity value
 */
float SDL2_GetGamepadStickSensitivity(int gamepad_index, int stick_id);

/**
 * Set trigger fire threshold (persisted)
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param trigger_id    0 for left, 1 for right
 * @param threshold     Threshold (0.0-1.0)
 * @return              0 on success
 */
int SDL2_SetGamepadTriggerThreshold(int gamepad_index, int trigger_id, float threshold);

/**
 * Get trigger fire threshold from config
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param trigger_id    0 for left, 1 for right
 * @return              Threshold value
 */
float SDL2_GetGamepadTriggerThreshold(int gamepad_index, int trigger_id);

/**
 * Enable/disable vibration for gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param enabled       1 for enabled, 0 for disabled
 * @return              0 on success
 */
int SDL2_SetGamepadVibrationEnabled(int gamepad_index, BOOL enabled);

/**
 * Check if vibration is enabled for gamepad
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              1 if enabled, 0 if disabled
 */
BOOL SDL2_IsGamepadVibrationEnabled(int gamepad_index);

/* ============================================================================
 * GAMEPAD DEVICE ENUMERATION
 * ============================================================================ */

/**
 * List all saved gamepad configurations
 * Returns count of configured gamepads
 *
 * @return  Number of saved configurations (0-4)
 */
int SDL2_GetSavedGamepadCount(void);

/**
 * Get saved gamepad configuration by index
 *
 * @param config_index Index (0 to SDL2_GetSavedGamepadCount()-1)
 * @param config       Output configuration structure
 * @return             0 on success, -1 if index out of range
 */
int SDL2_GetSavedGamepadConfig(int config_index, SDL2_GamepadConfig *config);

/**
 * Find gamepad configuration by device name pattern
 * Performs substring search on device name
 *
 * @param device_pattern Pattern to search for (e.g., "Xbox")
 * @param config         Output configuration structure
 * @return               0 if found, -1 if not found
 */
int SDL2_FindGamepadConfigByName(const char *device_pattern, SDL2_GamepadConfig *config);

/* ============================================================================
 * DEBUG / DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump gamepad configuration for debugging
 *
 * @param gamepad_index Gamepad index (0-3), or -1 for all
 */
void SDL2_DumpGamepadConfig(int gamepad_index);

/**
 * Export all gamepad configs to text file (backup)
 *
 * @param filepath File path to export to
 * @return         0 on success, -1 on error
 */
int SDL2_ExportGamepadConfigs(const char *filepath);

/**
 * Import gamepad configs from backup file
 *
 * @param filepath File path to import from
 * @return         0 on success, -1 on error
 */
int SDL2_ImportGamepadConfigs(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_GAMEPAD_CONFIG_COMPAT_H */
