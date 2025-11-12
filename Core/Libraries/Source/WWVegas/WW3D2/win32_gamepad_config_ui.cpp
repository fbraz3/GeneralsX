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
**  FILE: win32_gamepad_config_ui.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: In-Game Gamepad Configuration UI Menu System - Implementation
**
**  DESCRIPTION:
**    Implements interactive in-game menu for gamepad configuration.
**    Provides button remapping, sensitivity adjustment, and profile management.
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
#include "win32_gamepad_config_ui.h"
#include "win32_thread_compat.h"

/* ============================================================================
 * INTERNAL UI STATE MANAGEMENT
 * ============================================================================ */

typedef struct {
    GamepadConfigUIState state;
    GamepadConfigUIState previous_state;
    int current_gamepad;
    int current_menu_item;
    int selected_button;        /* During remapping */
    BYTE remapped_key;          /* Detected during remapping */
    float current_sensitivity;
    int sensitivity_target;     /* 0=stick left, 1=stick right, 2=trigger left, 3=trigger right */
    GamepadConfigVisualFeedback feedback;
    unsigned int feedback_duration;
    unsigned int feedback_timer;
    BOOL is_enabled;
    SDL2_CriticalSection lock;
} GamepadConfigUIGlobalState;

static GamepadConfigUIGlobalState g_ui_state;
static BOOL g_ui_initialized = FALSE;

typedef struct {
    char toast_message[256];
    unsigned int toast_duration;
    unsigned int toast_timer;
    BOOL toast_active;
} GamepadConfigUINotifications;

static GamepadConfigUINotifications g_notifications;

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitGamepadConfigUI(void)
{
    if (g_ui_initialized) {
        return 0;
    }
    
    printf("Phase 08: Initializing gamepad configuration UI system\n");
    
    memset(&g_ui_state, 0, sizeof(GamepadConfigUIGlobalState));
    memset(&g_notifications, 0, sizeof(GamepadConfigUINotifications));
    
    g_ui_state.lock = SDL2_CreateCriticalSection();
    g_ui_state.state = UI_STATE_INACTIVE;
    g_ui_state.current_gamepad = -1;
    g_ui_state.current_menu_item = 0;
    g_ui_state.current_sensitivity = 1.0f;
    g_ui_state.feedback = VISUAL_FEEDBACK_NONE;
    g_ui_state.is_enabled = FALSE;
    
    printf("Phase 08: Gamepad configuration UI system initialized\n");
    g_ui_initialized = TRUE;
    return 0;
}

void SDL2_ShutdownGamepadConfigUI(void)
{
    if (!g_ui_initialized) {
        return;
    }
    
    printf("Phase 08: Shutting down gamepad configuration UI system\n");
    
    SDL2_EnterCriticalSection(&g_ui_state.lock);
    {
        g_ui_state.state = UI_STATE_INACTIVE;
        g_ui_state.is_enabled = FALSE;
    }
    SDL2_LeaveCriticalSection(&g_ui_state.lock);
    
    SDL2_DestroyCriticalSection(&g_ui_state.lock);
    g_ui_initialized = FALSE;
}

/* ============================================================================
 * UI STATE CONTROL
 * ============================================================================ */

int SDL2_SetGamepadConfigUIEnabled(int gamepad_index, BOOL enabled)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (enabled) {
        if (gamepad_index < -1 || gamepad_index >= SDL2_MAX_GAMEPADS) {
            return -1;
        }
        
        printf("Phase 08: Enabling gamepad configuration UI for gamepad %d\n", gamepad_index);
        g_ui_state.current_gamepad = gamepad_index;
        g_ui_state.is_enabled = TRUE;
        g_ui_state.state = (gamepad_index == -1) ? UI_STATE_SELECT_GAMEPAD : UI_STATE_MAIN_MENU;
        g_ui_state.current_menu_item = 0;
    } else {
        printf("Phase 08: Disabling gamepad configuration UI\n");
        g_ui_state.is_enabled = FALSE;
        g_ui_state.state = UI_STATE_INACTIVE;
    }
    
    return 0;
}

BOOL SDL2_IsGamepadConfigUIEnabled(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    return g_ui_state.is_enabled;
}

GamepadConfigUIState SDL2_GetGamepadConfigUIState(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    return g_ui_state.state;
}

/* ============================================================================
 * UI RENDERING & INPUT HANDLING
 * ============================================================================ */

int SDL2_UpdateGamepadConfigUI(unsigned int delta_time)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (!g_ui_state.is_enabled) {
        return 0;
    }
    
    /* Update feedback timer */
    if (g_ui_state.feedback != VISUAL_FEEDBACK_NONE && g_ui_state.feedback_duration > 0) {
        g_ui_state.feedback_timer += delta_time;
        if (g_ui_state.feedback_timer >= g_ui_state.feedback_duration) {
            g_ui_state.feedback = VISUAL_FEEDBACK_NONE;
            g_ui_state.feedback_timer = 0;
        }
    }
    
    /* Update toast notification timer */
    if (g_notifications.toast_active && g_notifications.toast_duration > 0) {
        g_notifications.toast_timer += delta_time;
        if (g_notifications.toast_timer >= g_notifications.toast_duration) {
            g_notifications.toast_active = FALSE;
            g_notifications.toast_timer = 0;
        }
    }
    
    printf("Phase 08: UI update delta=%ums, state=%d, gamepad=%d\n", 
           delta_time, g_ui_state.state, g_ui_state.current_gamepad);
    
    return 0;
}

int SDL2_GamepadConfigUIHandleInput(int gamepad_index, SDL2_GamepadButton button, BOOL pressed)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (!g_ui_state.is_enabled) {
        return 0;  /* UI not active, pass input through */
    }
    
    if (!pressed) {
        return 1;  /* Ignore button releases in UI */
    }
    
    printf("Phase 08: UI input - gamepad %d, button %d, pressed\n", gamepad_index, button);
    
    /* TODO: State machine for menu navigation */
    /* TODO: Call appropriate handler based on current state */
    
    return 1;  /* Input consumed by UI */
}

int SDL2_RenderGamepadConfigUI(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (!g_ui_state.is_enabled) {
        return 0;
    }
    
    printf("Phase 08: Rendering UI - state %d\n", g_ui_state.state);
    
    /* TODO: Render menu based on state */
    /* TODO: Render selected items, feedback, notifications */
    
    return 0;
}

/* ============================================================================
 * BUTTON REMAPPING
 * ============================================================================ */

int SDL2_StartButtonRemapping(int gamepad_index, SDL2_GamepadButton button)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return -1;
    }
    
    if (g_ui_state.state == UI_STATE_BUTTON_MAPPING) {
        return -1;  /* Already remapping */
    }
    
    printf("Phase 08: Starting button remapping - gamepad %d, button %d\n", gamepad_index, button);
    
    g_ui_state.previous_state = g_ui_state.state;
    g_ui_state.state = UI_STATE_BUTTON_MAPPING;
    g_ui_state.selected_button = button;
    g_ui_state.remapped_key = 0;
    g_ui_state.feedback = VISUAL_FEEDBACK_WAITING_INPUT;
    
    SDL2_ShowGamepadConfigUIToast("Press any key to remap this button", 0);
    
    return 0;
}

int SDL2_CancelButtonRemapping(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (g_ui_state.state != UI_STATE_BUTTON_MAPPING) {
        return -1;
    }
    
    printf("Phase 08: Cancelled button remapping\n");
    
    g_ui_state.state = g_ui_state.previous_state;
    g_ui_state.selected_button = -1;
    g_ui_state.remapped_key = 0;
    g_ui_state.feedback = VISUAL_FEEDBACK_NONE;
    
    return 0;
}

int SDL2_ConfirmButtonRemapping(int gamepad_index, SDL2_GamepadButton button, BYTE vkey)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return -1;
    }
    
    if (vkey == 0) {
        return -1;
    }
    
    printf("Phase 08: Confirming button mapping - gamepad %d, button %d -> VK_0x%X\n",
           gamepad_index, button, vkey);
    
    /* Check for conflicts */
    SDL2_GamepadButton conflicting;
    if (SDL2_CheckButtonMappingConflict(gamepad_index, vkey, &conflicting) > 0) {
        printf("Phase 08: WARNING - Key conflict with button %d\n", conflicting);
        g_ui_state.feedback = VISUAL_FEEDBACK_CONFLICT;
        g_ui_state.feedback_duration = 2000;  /* 2 seconds */
        return -1;
    }
    
    /* Save mapping */
    int result = SDL2_SetGamepadButtonMapping(gamepad_index, button, vkey);
    
    if (result == 0) {
        g_ui_state.feedback = VISUAL_FEEDBACK_SUCCESS;
        g_ui_state.feedback_duration = 1000;  /* 1 second */
        SDL2_ShowGamepadConfigUIToast("Button remapped successfully", 1000);
    } else {
        g_ui_state.feedback = VISUAL_FEEDBACK_ERROR;
        g_ui_state.feedback_duration = 2000;
        SDL2_ShowGamepadConfigUIToast("Failed to save button mapping", 2000);
    }
    
    g_ui_state.state = g_ui_state.previous_state;
    g_ui_state.selected_button = -1;
    
    return result;
}

int SDL2_GetRemappingButton(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (g_ui_state.state != UI_STATE_BUTTON_MAPPING) {
        return -1;
    }
    
    return g_ui_state.selected_button;
}

int SDL2_CheckButtonMappingConflict(int gamepad_index, BYTE vkey, SDL2_GamepadButton *conflicting_button)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (vkey == 0) {
        return -1;
    }
    
    printf("Phase 08: Checking for mapping conflicts - VK_0x%X\n", vkey);
    
    /* TODO: Check against existing mappings */
    /* TODO: Return conflicting button if found */
    
    return 0;  /* No conflict */
}

int SDL2_PreviewButtonMapping(int gamepad_index, SDL2_GamepadButton button)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Previewing button mapping - gamepad %d, button %d\n", gamepad_index, button);
    
    BYTE vkey = SDL2_GetGamepadButtonMapping(gamepad_index, button);
    printf("Phase 08: Button %d currently maps to VK_0x%X\n", button, vkey);
    
    /* TODO: Display side-by-side comparison on screen */
    
    return 0;
}

int SDL2_ResetButtonMappingsToDefaults(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    printf("Phase 08: Resetting button mappings to defaults - gamepad %d\n", gamepad_index);
    
    SDL2_GamepadConfig default_config;
    SDL2_GetDefaultGamepadConfig(&default_config);
    
    for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
        SDL2_SetGamepadButtonMapping(gamepad_index, i, default_config.button_to_vkey[i]);
    }
    
    g_ui_state.feedback = VISUAL_FEEDBACK_SUCCESS;
    g_ui_state.feedback_duration = 2000;
    SDL2_ShowGamepadConfigUIToast("Button mappings reset to defaults", 2000);
    
    return 0;
}

/* ============================================================================
 * SENSITIVITY ADJUSTMENT
 * ============================================================================ */

int SDL2_ShowStickSensitivitySlider(int gamepad_index, int stick_id)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Showing stick sensitivity slider - gamepad %d, stick %d\n",
           gamepad_index, stick_id);
    
    g_ui_state.previous_state = g_ui_state.state;
    g_ui_state.state = UI_STATE_SENSITIVITY;
    g_ui_state.sensitivity_target = stick_id;
    g_ui_state.current_sensitivity = SDL2_GetGamepadStickSensitivity(gamepad_index, stick_id);
    
    return 0;
}

int SDL2_ShowTriggerSensitivitySlider(int gamepad_index, int trigger_id)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Showing trigger sensitivity slider - gamepad %d, trigger %d\n",
           gamepad_index, trigger_id);
    
    g_ui_state.previous_state = g_ui_state.state;
    g_ui_state.state = UI_STATE_SENSITIVITY;
    g_ui_state.sensitivity_target = 2 + trigger_id;  /* 2=left trigger, 3=right trigger */
    g_ui_state.current_sensitivity = SDL2_GetGamepadTriggerThreshold(gamepad_index, trigger_id);
    
    return 0;
}

int SDL2_UpdateSensitivityValue(float sensitivity)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (sensitivity < 0.1f || sensitivity > 3.0f) {
        return -1;
    }
    
    g_ui_state.current_sensitivity = sensitivity;
    printf("Phase 08: Sensitivity updated to %.2f\n", sensitivity);
    
    return 0;
}

float SDL2_GetCurrentSensitivityValue(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    return g_ui_state.current_sensitivity;
}

int SDL2_TestStickSensitivity(int gamepad_index, int stick_id)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Testing stick sensitivity - gamepad %d, stick %d\n",
           gamepad_index, stick_id);
    
    /* TODO: Get current stick position and apply sensitivity */
    /* TODO: Display test visualization on screen */
    
    return 0;
}

int SDL2_TestTriggerSensitivity(int gamepad_index, int trigger_id)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Testing trigger sensitivity - gamepad %d, trigger %d\n",
           gamepad_index, trigger_id);
    
    /* TODO: Get current trigger pressure and apply threshold */
    /* TODO: Display fire events and pressure visualization */
    
    return 0;
}

/* ============================================================================
 * PROFILE MANAGEMENT
 * ============================================================================ */

int SDL2_ShowProfileSelectionMenu(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Showing profile selection menu\n");
    
    g_ui_state.previous_state = g_ui_state.state;
    g_ui_state.state = UI_STATE_PROFILE_SELECT;
    
    /* TODO: Enumerate profiles and display */
    
    return 0;
}

int SDL2_LoadProfileFromUI(int gamepad_index, const char *profile_name)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (profile_name == NULL) {
        return -1;
    }
    
    printf("Phase 08: Loading profile '%s' for gamepad %d\n", profile_name, gamepad_index);
    
    /* TODO: Load profile from Phase 07 config system */
    
    return 0;
}

int SDL2_SaveProfileFromUI(int gamepad_index, const char *profile_name)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (profile_name == NULL) {
        return -1;
    }
    
    printf("Phase 08: Saving profile '%s' for gamepad %d\n", profile_name, gamepad_index);
    
    /* TODO: Save profile to Phase 07 config system */
    
    return 0;
}

int SDL2_DeleteProfileFromUI(const char *profile_name)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (profile_name == NULL) {
        return -1;
    }
    
    printf("Phase 08: Deleting profile '%s'\n", profile_name);
    
    /* TODO: Delete profile from Phase 07 config system */
    
    return 0;
}

/* ============================================================================
 * VISUAL FEEDBACK
 * ============================================================================ */

int SDL2_SetGamepadConfigUIFeedback(GamepadConfigVisualFeedback feedback_type, unsigned int duration_ms)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    g_ui_state.feedback = feedback_type;
    g_ui_state.feedback_duration = duration_ms;
    g_ui_state.feedback_timer = 0;
    
    printf("Phase 08: Set feedback type %d, duration %ums\n", feedback_type, duration_ms);
    
    return 0;
}

GamepadConfigVisualFeedback SDL2_GetGamepadConfigUIFeedback(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    return g_ui_state.feedback;
}

int SDL2_ShowGamepadConfigUIToast(const char *message, unsigned int duration_ms)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (message == NULL) {
        return -1;
    }
    
    strncpy(g_notifications.toast_message, message, sizeof(g_notifications.toast_message) - 1);
    g_notifications.toast_message[sizeof(g_notifications.toast_message) - 1] = '\0';
    g_notifications.toast_duration = duration_ms;
    g_notifications.toast_timer = 0;
    g_notifications.toast_active = TRUE;
    
    printf("Phase 08: Toast: %s (duration %ums)\n", message, duration_ms);
    
    return 0;
}

int SDL2_ShowGamepadConfigUIError(const char *title, const char *message)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (title == NULL || message == NULL) {
        return -1;
    }
    
    printf("Phase 08: Error Dialog - %s: %s\n", title, message);
    
    g_ui_state.feedback = VISUAL_FEEDBACK_ERROR;
    g_ui_state.feedback_duration = 3000;  /* 3 seconds */
    
    /* TODO: Display error dialog on screen */
    
    return 0;
}

/* ============================================================================
 * VIBRATION TEST
 * ============================================================================ */

int SDL2_StartVibrationTest(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    printf("Phase 08: Starting vibration test - gamepad %d\n", gamepad_index);
    
    /* TODO: Initialize vibration test mode */
    /* TODO: Allow cycling through test patterns */
    
    return 0;
}

int SDL2_StopVibrationTest(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Stopping vibration test\n");
    
    /* TODO: Stop all rumble and exit test mode */
    
    return 0;
}

int SDL2_PlayTestRumblePattern(int pattern_id, unsigned int duration_ms)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (pattern_id < 0 || pattern_id > 4) {
        return -1;
    }
    
    const char *pattern_names[] = {"weak", "medium", "strong", "pulse", "alternating"};
    printf("Phase 08: Playing test rumble pattern '%s' for %ums\n",
           pattern_names[pattern_id], duration_ms);
    
    /* TODO: Trigger rumble via Phase 06 gamepad system */
    
    return 0;
}

/* ============================================================================
 * MENU NAVIGATION
 * ============================================================================ */

int SDL2_GamepadConfigUISelectNext(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    g_ui_state.current_menu_item++;
    if (g_ui_state.current_menu_item >= MENU_ITEM_COUNT) {
        g_ui_state.current_menu_item = MENU_ITEM_COUNT - 1;
    }
    
    printf("Phase 08: Menu selection -> item %d\n", g_ui_state.current_menu_item);
    return 0;
}

int SDL2_GamepadConfigUISelectPrevious(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    g_ui_state.current_menu_item--;
    if (g_ui_state.current_menu_item < 0) {
        g_ui_state.current_menu_item = 0;
    }
    
    printf("Phase 08: Menu selection <- item %d\n", g_ui_state.current_menu_item);
    return 0;
}

int SDL2_GamepadConfigUISelectCurrent(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("Phase 08: Activating menu item %d\n", g_ui_state.current_menu_item);
    
    /* TODO: Handle menu item activation based on current selection */
    
    return 0;
}

int SDL2_GamepadConfigUIBack(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    if (g_ui_state.state == UI_STATE_MAIN_MENU) {
        g_ui_state.is_enabled = FALSE;
        g_ui_state.state = UI_STATE_INACTIVE;
        printf("Phase 08: Exiting gamepad configuration UI\n");
        return 0;
    }
    
    g_ui_state.state = g_ui_state.previous_state;
    printf("Phase 08: Returning to previous menu state %d\n", g_ui_state.state);
    return 0;
}

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

int SDL2_DumpGamepadConfigUIState(void)
{
    SDL2_CriticalSectionLock lock(&g_ui_state.lock);
    
    printf("\nPhase 08: Gamepad Configuration UI State Dump\n");
    printf("=========================================\n");
    printf("Enabled: %s\n", g_ui_state.is_enabled ? "yes" : "no");
    printf("State: %d (%s)\n", g_ui_state.state,
           g_ui_state.state == UI_STATE_INACTIVE ? "inactive" :
           g_ui_state.state == UI_STATE_MAIN_MENU ? "main_menu" :
           g_ui_state.state == UI_STATE_SELECT_GAMEPAD ? "select_gamepad" :
           g_ui_state.state == UI_STATE_BUTTON_MAPPING ? "button_mapping" :
           g_ui_state.state == UI_STATE_SENSITIVITY ? "sensitivity" :
           g_ui_state.state == UI_STATE_CONFIRM_ACTION ? "confirm_action" :
           g_ui_state.state == UI_STATE_PROFILE_SELECT ? "profile_select" : "unknown");
    printf("Current Gamepad: %d\n", g_ui_state.current_gamepad);
    printf("Menu Item: %d\n", g_ui_state.current_menu_item);
    printf("Feedback Type: %d\n", g_ui_state.feedback);
    printf("Feedback Duration: %ums (elapsed %ums)\n",
           g_ui_state.feedback_duration, g_ui_state.feedback_timer);
    
    if (g_notifications.toast_active) {
        printf("Toast: \"%s\" (expires in %ums)\n",
               g_notifications.toast_message,
               g_notifications.toast_duration - g_notifications.toast_timer);
    }
    printf("\n");
    
    return 0;
}
