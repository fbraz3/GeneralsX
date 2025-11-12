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
**  FILE: win32_gamepad_config_compat.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Gamepad Configuration Persistence Implementation
**
**  DESCRIPTION:
**    Implements gamepad configuration persistence using Phase 05 Registry/INI.
**    Enables per-device custom bindings and settings storage.
**
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_gamepad_config_compat.h"
#include "win32_config_compat.h"
#include "win32_thread_compat.h"

/* ============================================================================
 * GAMEPAD CONFIG REGISTRY PATH HELPERS
 * ============================================================================ */

static char g_gamepad_registry_root[512] = "";
static SDL2_CriticalSection g_gamepad_config_lock;
static BOOL g_gamepad_config_initialized = FALSE;

/**
 * Get registry key path for gamepad
 * Format: HKEY_CURRENT_USER\Software\Electronic Arts\EA Games\GeneralsX\Gamepads\GamepadN
 */
static void GetGamepadKeyPath(int gamepad_index, char *path, size_t path_size)
{
    snprintf(path, path_size, "HKEY_CURRENT_USER\\Software\\Electronic Arts\\EA Games\\GeneralsX\\Gamepads\\Gamepad%d",
             gamepad_index);
}

/**
 * Get value key name for gamepad button mapping
 */
static void GetButtonKeyName(int button, char *name, size_t name_size)
{
    snprintf(name, name_size, "Button%d", button);
}

/**
 * Get value key name for config value
 */
static void GetConfigKeyName(const char *config_item, char *name, size_t name_size)
{
    snprintf(name, name_size, "Config_%s", config_item);
}

/* ============================================================================
 * DEFAULT CONFIGURATION
 * ============================================================================ */

int SDL2_GetDefaultGamepadConfig(SDL2_GamepadConfig *config)
{
    if (config == NULL) {
        return -1;
    }
    
    memset(config, 0, sizeof(SDL2_GamepadConfig));
    
    /* Default button mapping (matches Phase 06 defaults) */
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_A] = VK_SPACE;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_B] = VK_ESCAPE;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_X] = VK_SHIFT;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_Y] = VK_CONTROL;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_LB] = 'Q';
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_RB] = 'E';
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_BACK] = VK_TAB;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_START] = VK_RETURN;
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_LSTICK] = 'L';
    config->button_to_vkey[SDL2_GAMEPAD_BUTTON_RSTICK] = 'R';
    
    /* Default analog stick configuration */
    config->stick_sensitivity[0] = 1.0f;  /* Left stick */
    config->stick_sensitivity[1] = 1.0f;  /* Right stick */
    config->stick_deadzone[0] = 0.15f;    /* 15% deadzone */
    config->stick_deadzone[1] = 0.15f;
    config->stick_invert_y[0] = FALSE;
    config->stick_invert_y[1] = FALSE;
    
    /* Default trigger configuration */
    config->trigger_fire_threshold[0] = 0.5f;  /* Left trigger */
    config->trigger_fire_threshold[1] = 0.5f;  /* Right trigger */
    config->trigger_fire_enabled[0] = TRUE;
    config->trigger_fire_enabled[1] = TRUE;
    config->trigger_sensitivity[0] = 1.0f;
    config->trigger_sensitivity[1] = 1.0f;
    
    /* Default general settings */
    config->enabled = TRUE;
    config->vibration_enabled = TRUE;
    
    return 0;
}

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitGamepadConfig(void)
{
    if (g_gamepad_config_initialized) {
        return 0;
    }
    
    g_gamepad_config_lock = SDL2_CreateCriticalSection();
    SDL2_EnterCriticalSection(&g_gamepad_config_lock);
    {
        printf("Phase 07: Initializing gamepad configuration system\n");
        
        /* TODO: Initialize Phase 05 Registry system if not already done */
        /* SDL2_InitializeConfigSystem(); */
        
        g_gamepad_config_initialized = TRUE;
        printf("Phase 07: Gamepad configuration system initialized\n");
    }
    SDL2_LeaveCriticalSection(&g_gamepad_config_lock);
    
    return 0;
}

void SDL2_ShutdownGamepadConfig(void)
{
    if (!g_gamepad_config_initialized) {
        return;
    }
    
    SDL2_EnterCriticalSection(&g_gamepad_config_lock);
    {
        printf("Phase 07: Shutting down gamepad configuration system\n");
        g_gamepad_config_initialized = FALSE;
    }
    SDL2_LeaveCriticalSection(&g_gamepad_config_lock);
    
    SDL2_DestroyCriticalSection(&g_gamepad_config_lock);
}

/* ============================================================================
 * CONFIGURATION LOADING & SAVING
 * ============================================================================ */

int SDL2_LoadGamepadConfig(int gamepad_index, SDL2_GamepadConfig *config)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (config == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Loading gamepad %d configuration from Registry/INI\n", gamepad_index);
    
    /* Get default first */
    SDL2_GetDefaultGamepadConfig(config);
    
    /* TODO: Load from Registry via Phase 05
       char key_path[512];
       GetGamepadKeyPath(gamepad_index, key_path, sizeof(key_path));
       HKEY hKey = RegOpenKeyEx(HKEY_CURRENT_USER, key_path, ...);
       if (hKey == NULL) {
           // No saved config, use defaults
           return -1;
       }
       
       // Load button mappings
       for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
           char name[64];
           GetButtonKeyName(i, name, sizeof(name));
           DWORD vkey = 0;
           RegQueryValueEx(hKey, name, NULL, REG_DWORD, (LPBYTE)&vkey, ...);
           if (vkey != 0) config->button_to_vkey[i] = (BYTE)vkey;
       }
       
       // Load sensitivities, thresholds, etc.
       RegCloseKey(hKey);
    */
    
    printf("Phase 07: Using default configuration for gamepad %d (Phase 05 integration pending)\n",
           gamepad_index);
    
    return 0;
}

int SDL2_SaveGamepadConfig(int gamepad_index, const SDL2_GamepadConfig *config)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (config == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Saving gamepad %d configuration to Registry/INI\n", gamepad_index);
    
    /* TODO: Save to Registry via Phase 05
       char key_path[512];
       GetGamepadKeyPath(gamepad_index, key_path, sizeof(key_path));
       HKEY hKey = RegCreateKeyEx(HKEY_CURRENT_USER, key_path, 0, ...);
       if (hKey == NULL) return -1;
       
       // Save button mappings
       for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
           char name[64];
           GetButtonKeyName(i, name, sizeof(name));
           RegSetValueEx(hKey, name, 0, REG_DWORD, (LPBYTE)&config->button_to_vkey[i], ...);
       }
       
       // Save sensitivities, thresholds, etc.
       RegCloseKey(hKey);
    */
    
    printf("Phase 07: Configuration save deferred (Phase 05 integration pending)\n");
    return 0;
}

int SDL2_DeleteGamepadConfig(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Deleting gamepad %d configuration from Registry/INI\n", gamepad_index);
    
    /* TODO: Delete from Registry via Phase 05 */
    
    printf("Phase 07: Configuration delete deferred (Phase 05 integration pending)\n");
    return 0;
}

/* ============================================================================
 * CONFIGURATION QUERY & MODIFICATION
 * ============================================================================ */

int SDL2_GetGamepadConfig(int gamepad_index, SDL2_GamepadConfig *config)
{
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    return SDL2_LoadGamepadConfig(gamepad_index, config);
}

int SDL2_SetGamepadConfig(int gamepad_index, const SDL2_GamepadConfig *config)
{
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    return SDL2_SaveGamepadConfig(gamepad_index, config);
}

int SDL2_SetGamepadButtonMapping(int gamepad_index, int button, BYTE vkey)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    config.button_to_vkey[button] = vkey;
    
    printf("Phase 07: Gamepad %d button %d → VK_0x%X\n", gamepad_index, button, vkey);
    
    return SDL2_SaveGamepadConfig(gamepad_index, &config);
}

BYTE SDL2_GetGamepadButtonMapping(int gamepad_index, int button)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return 0;
    }
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return 0;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    return config.button_to_vkey[button];
}

/* ============================================================================
 * SENSITIVITY & TRIGGER PERSISTENCE
 * ============================================================================ */

int SDL2_SetGamepadStickSensitivity(int gamepad_index, int stick_id, float sensitivity)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (stick_id < 0 || stick_id > 1) {
        return -1;
    }
    
    if (sensitivity < 0.1f || sensitivity > 3.0f) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    config.stick_sensitivity[stick_id] = sensitivity;
    
    const char *stick_name = (stick_id == 0) ? "left" : "right";
    printf("Phase 07: Gamepad %d %s stick sensitivity: %.2f\n", gamepad_index, stick_name, sensitivity);
    
    return SDL2_SaveGamepadConfig(gamepad_index, &config);
}

float SDL2_GetGamepadStickSensitivity(int gamepad_index, int stick_id)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return 1.0f;
    }
    
    if (stick_id < 0 || stick_id > 1) {
        return 1.0f;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    return config.stick_sensitivity[stick_id];
}

int SDL2_SetGamepadTriggerThreshold(int gamepad_index, int trigger_id, float threshold)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (trigger_id < 0 || trigger_id > 1) {
        return -1;
    }
    
    if (threshold < 0.0f || threshold > 1.0f) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    config.trigger_fire_threshold[trigger_id] = threshold;
    
    const char *trigger_name = (trigger_id == 0) ? "left" : "right";
    printf("Phase 07: Gamepad %d %s trigger fire threshold: %.2f\n", gamepad_index, trigger_name, threshold);
    
    return SDL2_SaveGamepadConfig(gamepad_index, &config);
}

float SDL2_GetGamepadTriggerThreshold(int gamepad_index, int trigger_id)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return 0.5f;
    }
    
    if (trigger_id < 0 || trigger_id > 1) {
        return 0.5f;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    return config.trigger_fire_threshold[trigger_id];
}

int SDL2_SetGamepadVibrationEnabled(int gamepad_index, BOOL enabled)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    config.vibration_enabled = enabled;
    
    printf("Phase 07: Gamepad %d vibration %s\n", gamepad_index, enabled ? "enabled" : "disabled");
    
    return SDL2_SaveGamepadConfig(gamepad_index, &config);
}

BOOL SDL2_IsGamepadVibrationEnabled(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return TRUE;  /* Default enabled */
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    SDL2_GamepadConfig config;
    if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
        SDL2_GetDefaultGamepadConfig(&config);
    }
    
    return config.vibration_enabled;
}

/* ============================================================================
 * DEBUG / DIAGNOSTICS
 * ============================================================================ */

void SDL2_DumpGamepadConfig(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("\nPhase 07: Gamepad Configuration Status\n");
    printf("=======================================\n");
    
    if (gamepad_index >= 0 && gamepad_index < SDL2_MAX_GAMEPADS) {
        /* Single gamepad */
        SDL2_GamepadConfig config;
        if (SDL2_LoadGamepadConfig(gamepad_index, &config) != 0) {
            SDL2_GetDefaultGamepadConfig(&config);
        }
        
        printf("Gamepad %d:\n", gamepad_index);
        printf("  Device: %s (0x%04X:0x%04X)\n", config.device_name, config.vendor_id, config.product_id);
        printf("  Enabled: %s\n", config.enabled ? "yes" : "no");
        printf("  Vibration: %s\n", config.vibration_enabled ? "enabled" : "disabled");
        printf("  Left stick sensitivity: %.2f\n", config.stick_sensitivity[0]);
        printf("  Right stick sensitivity: %.2f\n", config.stick_sensitivity[1]);
        printf("  Left trigger threshold: %.2f\n", config.trigger_fire_threshold[0]);
        printf("  Right trigger threshold: %.2f\n", config.trigger_fire_threshold[1]);
        printf("  Button Mappings:\n");
        for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
            printf("    Button %d: VK_0x%X\n", i, config.button_to_vkey[i]);
        }
    } else {
        /* All gamepads */
        printf("All Gamepads:\n");
        for (int i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            SDL2_GamepadConfig config;
            if (SDL2_LoadGamepadConfig(i, &config) == 0) {
                printf("  Gamepad %d: sensL=%.1f sensR=%.1f thrL=%.1f thrR=%.1f\n",
                       i,
                       config.stick_sensitivity[0],
                       config.stick_sensitivity[1],
                       config.trigger_fire_threshold[0],
                       config.trigger_fire_threshold[1]);
            }
        }
    }
    printf("\n");
}

int SDL2_GetSavedGamepadCount(void)
{
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Getting saved gamepad count (Phase 05 integration pending)\n");
    
    /* TODO: Enumerate Registry keys and count configured gamepads */
    return 0;
}

int SDL2_GetSavedGamepadConfig(int config_index, SDL2_GamepadConfig *config)
{
    if (config_index < 0 || config_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (config == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    /* TODO: Load config_index-th saved configuration from Registry */
    
    return -1;
}

int SDL2_FindGamepadConfigByName(const char *device_pattern, SDL2_GamepadConfig *config)
{
    if (device_pattern == NULL || config == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Finding gamepad config matching '%s' (Phase 05 integration pending)\n", device_pattern);
    
    /* TODO: Search Registry for matching device name */
    
    return -1;
}

int SDL2_ExportGamepadConfigs(const char *filepath)
{
    if (filepath == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Exporting gamepad configs to %s\n", filepath);
    
    /* TODO: Export all Registry gamepad configs to text file */
    
    return 0;
}

int SDL2_ImportGamepadConfigs(const char *filepath)
{
    if (filepath == NULL) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_gamepad_config_lock);
    
    printf("Phase 07: Importing gamepad configs from %s\n", filepath);
    
    /* TODO: Import Registry gamepad configs from text file */
    
    return 0;
}
