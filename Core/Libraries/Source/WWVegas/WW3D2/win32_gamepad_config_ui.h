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
**  FILE: win32_gamepad_config_ui.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: In-Game Gamepad Configuration UI Menu System
**
**  DESCRIPTION:
**    Provides in-game menu for interactive gamepad configuration.
**    Allows button rebinding, sensitivity adjustment, and profile management.
**
******************************************************************************/

#ifndef __WIN32_GAMEPAD_CONFIG_UI_H__
#define __WIN32_GAMEPAD_CONFIG_UI_H__

#include "win32_gamepad_compat.h"
#include "win32_gamepad_config_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * UI MENU STATE DEFINITIONS
 * ============================================================================ */

typedef enum {
    UI_STATE_INACTIVE = 0,      /* Menu not displayed */
    UI_STATE_MAIN_MENU,         /* Main configuration menu */
    UI_STATE_SELECT_GAMEPAD,    /* Gamepad selection screen */
    UI_STATE_BUTTON_MAPPING,    /* Button remapping mode */
    UI_STATE_SENSITIVITY,       /* Stick/trigger sensitivity config */
    UI_STATE_CONFIRM_ACTION,    /* Confirmation dialog */
    UI_STATE_PROFILE_SELECT,    /* Profile load/save selection */
    UI_STATE_COUNT
} GamepadConfigUIState;

typedef enum {
    MENU_ITEM_REMAP_BUTTONS = 0,
    MENU_ITEM_ADJUST_SENSITIVITY,
    MENU_ITEM_LOAD_PROFILE,
    MENU_ITEM_SAVE_PROFILE,
    MENU_ITEM_RESET_DEFAULTS,
    MENU_ITEM_VIBRATION_TEST,
    MENU_ITEM_EXIT,
    MENU_ITEM_COUNT
} GamepadConfigMenuItem;

typedef enum {
    VISUAL_FEEDBACK_NONE = 0,
    VISUAL_FEEDBACK_HIGHLIGHT,     /* Current selection highlighted */
    VISUAL_FEEDBACK_WAITING_INPUT,  /* Waiting for button press */
    VISUAL_FEEDBACK_CONFLICT,       /* Button conflict detected */
    VISUAL_FEEDBACK_SUCCESS,        /* Operation successful */
    VISUAL_FEEDBACK_ERROR           /* Operation failed */
} GamepadConfigVisualFeedback;

/* ============================================================================
 * UI SYSTEM INITIALIZATION & CONTROL
 * ============================================================================ */

/**
 * Initialize the gamepad configuration UI system
 * Must be called before any UI operations
 * 
 * Returns: 0 on success, -1 on failure
 */
int SDL2_InitGamepadConfigUI(void);

/**
 * Shutdown the gamepad configuration UI system
 * Called at game shutdown
 */
void SDL2_ShutdownGamepadConfigUI(void);

/**
 * Enable/disable the configuration UI menu
 * When enabled, UI will render and accept input
 * 
 * gamepad_index: Index of gamepad being configured (-1 for selection mode)
 * enabled: TRUE to show menu, FALSE to hide
 * 
 * Returns: 0 on success, -1 on invalid gamepad_index
 */
int SDL2_SetGamepadConfigUIEnabled(int gamepad_index, BOOL enabled);

/**
 * Check if configuration UI is currently active
 * 
 * Returns: TRUE if menu is displayed, FALSE otherwise
 */
BOOL SDL2_IsGamepadConfigUIEnabled(void);

/**
 * Get current UI menu state
 * 
 * Returns: Current GamepadConfigUIState enum value
 */
GamepadConfigUIState SDL2_GetGamepadConfigUIState(void);

/* ============================================================================
 * UI RENDERING & INPUT HANDLING
 * ============================================================================ */

/**
 * Update and render the configuration UI each frame
 * Call once per game loop when UI is enabled
 * 
 * delta_time: Time since last frame in milliseconds
 * 
 * Returns: 0 if UI handling succeeded, -1 on error
 */
int SDL2_UpdateGamepadConfigUI(unsigned int delta_time);

/**
 * Handle gamepad input while UI is active
 * Automatically called from SDL2_UpdateGamepadConfigUI
 * 
 * gamepad_index: Index of gamepad providing input
 * button: Button pressed (SDL2_GamepadButton enum)
 * pressed: TRUE if pressed, FALSE if released
 * 
 * Returns: 1 if input was consumed by UI, 0 if should pass through to game
 */
int SDL2_GamepadConfigUIHandleInput(int gamepad_index, SDL2_GamepadButton button, BOOL pressed);

/**
 * Render the configuration UI to screen
 * Call from main render loop when UI is enabled
 * 
 * Returns: 0 on success, -1 on render failure
 */
int SDL2_RenderGamepadConfigUI(void);

/* ============================================================================
 * BUTTON REMAPPING INTERFACE
 * ============================================================================ */

/**
 * Enter button remapping mode for specified button
 * UI will enter "waiting for input" state and highlight the target button
 * 
 * gamepad_index: Gamepad being configured
 * button: SDL2_GamepadButton to be remapped
 * 
 * Returns: 0 on success, -1 if already in remap mode
 */
int SDL2_StartButtonRemapping(int gamepad_index, SDL2_GamepadButton button);

/**
 * Exit button remapping mode without saving
 * Returns: 0 on success
 */
int SDL2_CancelButtonRemapping(void);

/**
 * Confirm button remapping with detected key
 * Associates gamepad button with detected keyboard key
 * 
 * gamepad_index: Gamepad being configured
 * button: Original gamepad button
 * vkey: Detected VK_* code from keyboard
 * 
 * Returns: 0 on success, -1 on conflict/error
 */
int SDL2_ConfirmButtonRemapping(int gamepad_index, SDL2_GamepadButton button, BYTE vkey);

/**
 * Get the button currently being remapped (or -1 if none)
 * 
 * Returns: SDL2_GamepadButton being remapped, or -1 if not remapping
 */
int SDL2_GetRemappingButton(void);

/**
 * Check if specified VK code conflicts with existing mappings
 * 
 * gamepad_index: Gamepad being configured
 * vkey: VK_* code to check
 * conflicting_button: Output pointer to conflicting button (may be NULL)
 * 
 * Returns: 1 if conflict exists, 0 if available, -1 on error
 */
int SDL2_CheckButtonMappingConflict(int gamepad_index, BYTE vkey, SDL2_GamepadButton *conflicting_button);

/**
 * Show visual preview of button mapping for confirmation
 * Displays the gamepad button and its mapped keyboard key side-by-side
 * 
 * gamepad_index: Gamepad being configured
 * button: Button to preview
 * 
 * Returns: 0 on success
 */
int SDL2_PreviewButtonMapping(int gamepad_index, SDL2_GamepadButton button);

/**
 * Apply factory default button mappings
 * Reverts all remapped buttons to factory defaults
 * 
 * gamepad_index: Gamepad to reset
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ResetButtonMappingsToDefaults(int gamepad_index);

/* ============================================================================
 * SENSITIVITY ADJUSTMENT INTERFACE
 * ============================================================================ */

/**
 * Display sensitivity adjustment slider for analog stick
 * 
 * gamepad_index: Gamepad being configured
 * stick_id: 0 for left stick, 1 for right stick
 * 
 * Returns: 0 on success
 */
int SDL2_ShowStickSensitivitySlider(int gamepad_index, int stick_id);

/**
 * Display sensitivity adjustment slider for trigger
 * 
 * gamepad_index: Gamepad being configured
 * trigger_id: 0 for left trigger, 1 for right trigger
 * 
 * Returns: 0 on success
 */
int SDL2_ShowTriggerSensitivitySlider(int gamepad_index, int trigger_id);

/**
 * Update sensitivity value based on user input
 * 
 * sensitivity: New sensitivity value (0.1 to 3.0)
 * 
 * Returns: 0 on success, -1 on invalid range
 */
int SDL2_UpdateSensitivityValue(float sensitivity);

/**
 * Get current sensitivity value being adjusted
 * 
 * Returns: Sensitivity multiplier (0.1 to 3.0), or 1.0 if not adjusting
 */
float SDL2_GetCurrentSensitivityValue(void);

/**
 * Test stick sensitivity with live preview
 * Displays stick input with current sensitivity applied
 * 
 * gamepad_index: Gamepad providing test input
 * stick_id: 0 for left stick, 1 for right stick
 * 
 * Returns: 0 on success
 */
int SDL2_TestStickSensitivity(int gamepad_index, int stick_id);

/**
 * Test trigger sensitivity with fire event preview
 * Shows trigger pressure and fire events
 * 
 * gamepad_index: Gamepad providing test input
 * trigger_id: 0 for left trigger, 1 for right trigger
 * 
 * Returns: 0 on success
 */
int SDL2_TestTriggerSensitivity(int gamepad_index, int trigger_id);

/* ============================================================================
 * PROFILE MANAGEMENT INTERFACE
 * ============================================================================ */

/**
 * Display profile selection menu
 * Shows available saved profiles
 * 
 * Returns: 0 on success
 */
int SDL2_ShowProfileSelectionMenu(void);

/**
 * Load selected profile
 * 
 * gamepad_index: Gamepad to apply profile to
 * profile_name: Name of profile to load
 * 
 * Returns: 0 on success, -1 if profile not found
 */
int SDL2_LoadProfileFromUI(int gamepad_index, const char *profile_name);

/**
 * Save current configuration as new profile
 * 
 * gamepad_index: Gamepad configuration to save
 * profile_name: Name for new profile
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_SaveProfileFromUI(int gamepad_index, const char *profile_name);

/**
 * Delete profile
 * 
 * profile_name: Name of profile to delete
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_DeleteProfileFromUI(const char *profile_name);

/* ============================================================================
 * VISUAL FEEDBACK SYSTEM
 * ============================================================================ */

/**
 * Set visual feedback mode for UI
 * Controls highlighting, warnings, success indicators
 * 
 * feedback_type: GamepadConfigVisualFeedback enum value
 * duration_ms: How long to display feedback (0 = permanent until changed)
 * 
 * Returns: 0 on success
 */
int SDL2_SetGamepadConfigUIFeedback(GamepadConfigVisualFeedback feedback_type, unsigned int duration_ms);

/**
 * Get current visual feedback mode
 * 
 * Returns: Current GamepadConfigVisualFeedback enum value
 */
GamepadConfigVisualFeedback SDL2_GetGamepadConfigUIFeedback(void);

/**
 * Display toast message (temporary notification)
 * 
 * message: Message to display (max 256 chars)
 * duration_ms: How long to display message
 * 
 * Returns: 0 on success
 */
int SDL2_ShowGamepadConfigUIToast(const char *message, unsigned int duration_ms);

/**
 * Display error dialog
 * 
 * title: Dialog title
 * message: Error message
 * 
 * Returns: 0 on success
 */
int SDL2_ShowGamepadConfigUIError(const char *title, const char *message);

/* ============================================================================
 * VIBRATION TEST INTERFACE
 * ============================================================================ */

/**
 * Enter vibration test mode
 * Allows testing different rumble patterns
 * 
 * gamepad_index: Gamepad to test
 * 
 * Returns: 0 on success
 */
int SDL2_StartVibrationTest(int gamepad_index);

/**
 * Exit vibration test mode and stop all rumble
 * 
 * Returns: 0 on success
 */
int SDL2_StopVibrationTest(void);

/**
 * Trigger test rumble pattern
 * 
 * pattern_id: 0=weak, 1=medium, 2=strong, 3=pulse, 4=alternating
 * duration_ms: How long to rumble
 * 
 * Returns: 0 on success
 */
int SDL2_PlayTestRumblePattern(int pattern_id, unsigned int duration_ms);

/* ============================================================================
 * MENU NAVIGATION
 * ============================================================================ */

/**
 * Navigate to next menu item (scroll down)
 * 
 * Returns: 0 on success
 */
int SDL2_GamepadConfigUISelectNext(void);

/**
 * Navigate to previous menu item (scroll up)
 * 
 * Returns: 0 on success
 */
int SDL2_GamepadConfigUISelectPrevious(void);

/**
 * Activate current menu selection
 * 
 * Returns: 0 on success
 */
int SDL2_GamepadConfigUISelectCurrent(void);

/**
 * Return to previous menu level
 * 
 * Returns: 0 on success
 */
int SDL2_GamepadConfigUIBack(void);

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump UI state for debugging
 * Prints current menu selection, state, and active dialogs
 * 
 * Returns: 0 on success
 */
int SDL2_DumpGamepadConfigUIState(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIN32_GAMEPAD_CONFIG_UI_H__ */
