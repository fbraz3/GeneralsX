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

#ifndef INPUTCONFIGURATION_H
#define INPUTCONFIGURATION_H

#include "Lib/BaseType.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
**  Phase 47: Input Configuration System
**  
**  Purpose: Provide cross-platform input configuration with rebindable keys,
**  mouse sensitivity adjustment, and hotkey management.
**
**  Architecture:
**  - InputConfiguration: High-level configuration API
**  - InputBinding: Key-to-action mapping
**  - InputProfile: Named configuration profiles
**  - Persistent storage: INI file format
*/

/* Action type identifiers */
typedef enum {
    ACTION_MOVE_UP = 0,
    ACTION_MOVE_DOWN = 1,
    ACTION_MOVE_LEFT = 2,
    ACTION_MOVE_RIGHT = 3,
    ACTION_SELECT_ALL = 4,
    ACTION_ATTACK_MOVE = 5,
    ACTION_STOP_UNITS = 6,
    ACTION_BUILD_MENU = 7,
    ACTION_GENERAL_ABILITIES = 8,
    ACTION_CASH_BOUNTY = 9,
    ACTION_REPLAY_PREVIOUS = 10,
    ACTION_HOTKEY_UNIT_TYPE_BASE = 11,  /* Hotkeys for unit types (11-20) */
    ACTION_HOTKEY_UNIT_TYPE_END = 20,
    ACTION_HOTKEY_ABILITY_BASE = 21,    /* Hotkeys for abilities (21-30) */
    ACTION_HOTKEY_ABILITY_END = 30,
    ACTION_MAX_ACTIONS = 31
} InputActionType;

/* Input binding data */
typedef struct {
    uint32_t primary_key;       /* Primary key binding (DIK_* scancode) */
    uint32_t secondary_key;     /* Secondary key binding (alternative) */
    uint32_t modifier_flags;    /* Modifier flags (Shift, Ctrl, Alt) */
} InputBinding;

/* Modifier key flags */
typedef enum {
    MODIFIER_NONE = 0x00,
    MODIFIER_SHIFT = 0x01,
    MODIFIER_CTRL = 0x02,
    MODIFIER_ALT = 0x04,
    MODIFIER_WIN = 0x08
} ModifierFlagType;

/* Mouse configuration */
typedef struct {
    float sensitivity;          /* Mouse sensitivity multiplier (0.1 - 2.0) */
    float acceleration;         /* Mouse acceleration (0.0 - 1.0) */
    Bool invert_y;              /* Invert Y-axis */
    Bool raw_input;             /* Use raw mouse input (no acceleration) */
} MouseConfiguration;

/* Gamepad configuration */
typedef struct {
    float analog_deadzone;      /* Deadzone for analog sticks (0.0 - 0.5) */
    float trigger_threshold;    /* Trigger activation threshold (0.0 - 1.0) */
    float stick_sensitivity;    /* Analog stick sensitivity (0.5 - 2.0) */
    Bool button_mapping_enabled; /* Enable custom button mapping */
} GamepadConfiguration;

/* Main input configuration structure */
typedef struct InputConfiguration {
    InputBinding key_bindings[ACTION_MAX_ACTIONS];
    MouseConfiguration mouse_config;
    GamepadConfiguration gamepad_config;
    char profile_name[64];      /* Current profile name */
    Bool auto_save_enabled;     /* Auto-save config on changes */
} InputConfiguration;

/*
**  InputConfiguration API
*/

/* Lifecycle */
InputConfiguration* InputConfiguration_Create(void);
void InputConfiguration_Destroy(InputConfiguration* config);

/* File I/O */
Bool InputConfiguration_LoadFromFile(InputConfiguration* config, const char* filename);
Bool InputConfiguration_SaveToFile(InputConfiguration* config, const char* filename);
Bool InputConfiguration_LoadProfile(InputConfiguration* config, const char* profile_name);
Bool InputConfiguration_SaveProfile(InputConfiguration* config, const char* profile_name);

/* Key binding management */
Bool InputConfiguration_SetKeyBinding(InputConfiguration* config, InputActionType action, 
                                      uint32_t key, uint32_t modifiers);
Bool InputConfiguration_SetSecondaryKeyBinding(InputConfiguration* config, InputActionType action,
                                               uint32_t key, uint32_t modifiers);
Bool InputConfiguration_GetKeyBinding(InputConfiguration* config, InputActionType action,
                                      uint32_t* out_key, uint32_t* out_modifiers);
Bool InputConfiguration_GetSecondaryKeyBinding(InputConfiguration* config, InputActionType action,
                                               uint32_t* out_key, uint32_t* out_modifiers);
Bool InputConfiguration_ClearKeyBinding(InputConfiguration* config, InputActionType action);

/* Action lookup by key */
InputActionType InputConfiguration_GetActionFromKey(InputConfiguration* config, 
                                                     uint32_t key, uint32_t modifiers);

/* Mouse configuration */
void InputConfiguration_SetMouseSensitivity(InputConfiguration* config, float sensitivity);
float InputConfiguration_GetMouseSensitivity(InputConfiguration* config);
void InputConfiguration_SetMouseAcceleration(InputConfiguration* config, float acceleration);
void InputConfiguration_SetMouseInvertY(InputConfiguration* config, Bool invert);

/* Gamepad configuration */
void InputConfiguration_SetGamepadDeadzone(InputConfiguration* config, float deadzone);
float InputConfiguration_GetGamepadDeadzone(InputConfiguration* config);
void InputConfiguration_SetGamepadSensitivity(InputConfiguration* config, float sensitivity);

/* Profile management */
Bool InputConfiguration_ListProfiles(const char*** out_profiles, int* out_count);
Bool InputConfiguration_CreateProfile(const char* name);
Bool InputConfiguration_DeleteProfile(const char* name);
Bool InputConfiguration_RenameProfile(const char* old_name, const char* new_name);

/* Default configuration */
void InputConfiguration_SetDefaults(InputConfiguration* config);
void InputConfiguration_ResetToDefaults(InputConfiguration* config);

/* Validation */
Bool InputConfiguration_IsValid(InputConfiguration* config);
const char* InputConfiguration_GetLastError(void);

#ifdef __cplusplus
}
#endif

#endif /* INPUTCONFIGURATION_H */
