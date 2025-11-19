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
**  FILE: win32_gamepad_event_integration.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Gamepad Event Queue Integration Implementation
**
**  DESCRIPTION:
**    Implements gamepad event integration with keyboard/mouse queues.
**    Translates gamepad state to input events.
**    Manages sensitivities and fire thresholds.
**
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <unistd.h>

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_gamepad_event_integration.h"
#include "win32_thread_compat.h"

/* ============================================================================
 * GAMEPAD EVENT INTEGRATION STATE
 * ============================================================================ */

/**
 * Per-gamepad event configuration
 */
typedef struct {
    float stick_sensitivity[2];      /* Sensitivity for left & right sticks */
    float trigger_fire_threshold[2]; /* Fire threshold for left & right triggers */
    BOOL trigger_fire_enabled[2];    /* Enable fire for left & right triggers */
    BOOL last_button_state[SDL2_GAMEPAD_BUTTON_COUNT]; /* Previous frame button state */
    short last_axis_state[SDL2_GAMEPAD_AXIS_COUNT];    /* Previous frame axis state */
} GamepadEventConfig;

static SDL2_CriticalSection g_event_integration_lock;
static BOOL g_event_integration_initialized = FALSE;
static BOOL g_event_generation_enabled = TRUE;
static GamepadEventConfig g_event_config[SDL2_MAX_GAMEPADS];

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitGamepadEventIntegration(void)
{
    if (g_event_integration_initialized) {
        return 0;
    }
    
    g_event_integration_lock = SDL2_CreateCriticalSection();
    SDL2_EnterCriticalSection(&g_event_integration_lock);
    {
        printf("Phase 07: Initializing gamepad event integration\n");
        
        /* Initialize default configurations */
        for (int i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            /* Default sensitivity: 1.0 (neutral) */
            g_event_config[i].stick_sensitivity[0] = 1.0f;  /* Left stick */
            g_event_config[i].stick_sensitivity[1] = 1.0f;  /* Right stick */
            
            /* Default fire threshold: 0.5 (50% trigger press) */
            g_event_config[i].trigger_fire_threshold[0] = 0.5f;  /* Left trigger */
            g_event_config[i].trigger_fire_threshold[1] = 0.5f;  /* Right trigger */
            
            /* Enable firing by default */
            g_event_config[i].trigger_fire_enabled[0] = TRUE;
            g_event_config[i].trigger_fire_enabled[1] = TRUE;
            
            /* Clear state tracking */
            memset(g_event_config[i].last_button_state, 0, sizeof(g_event_config[i].last_button_state));
            memset(g_event_config[i].last_axis_state, 0, sizeof(g_event_config[i].last_axis_state));
        }
        
        g_event_integration_initialized = TRUE;
        printf("Phase 07: Gamepad event integration initialized\n");
    }
    SDL2_LeaveCriticalSection(&g_event_integration_lock);
    
    return 0;
}

void SDL2_ShutdownGamepadEventIntegration(void)
{
    if (!g_event_integration_initialized) {
        return;
    }
    
    SDL2_EnterCriticalSection(&g_event_integration_lock);
    {
        printf("Phase 07: Shutting down gamepad event integration\n");
        g_event_integration_initialized = FALSE;
    }
    SDL2_LeaveCriticalSection(&g_event_integration_lock);
    
    SDL2_DestroyCriticalSection(&g_event_integration_lock);
}

void SDL2_SetGamepadEventGenerationEnabled(BOOL enabled)
{
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    g_event_generation_enabled = enabled;
    printf("Phase 07: Gamepad event generation %s\n", enabled ? "enabled" : "disabled");
}

/* ============================================================================
 * KEYBOARD EVENT POSTING
 * ============================================================================ */

int SDL2_PostGamepadButtonKeyboardEvent(int gamepad_index, int button, BOOL pressed)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return -1;
    }
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (button < 0 || button >= SDL2_GAMEPAD_BUTTON_COUNT) {
        return -1;
    }
    
    SDL2_GamepadState state;
    if (SDL2_GetGamepadState(gamepad_index, &state) != 0) {
        return -1;
    }
    
    if (!state.connected) {
        return -1;
    }
    
    SDL2_GamepadMapping mapping;
    SDL2_GetDefaultGamepadMapping(&mapping);
    
    BYTE vkey = mapping.button_to_vkey[button];
    
    printf("Phase 07: Gamepad %d button %d %s → VK_0x%X\n",
           gamepad_index, button, pressed ? "DOWN" : "UP", vkey);
    
    /* TODO: Post to TheWin32Keyboard->addWin32Event(pressed ? WM_KEYDOWN : WM_KEYUP, vkey, ...) */
    /* This requires integration with keyboard input class (pending) */
    
    return 0;
}

int SDL2_PostKeyboardEvent(BYTE vkey, BOOL pressed)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return -1;
    }
    
    printf("Phase 07: Posting keyboard event: VK_0x%X %s\n",
           vkey, pressed ? "DOWN" : "UP");
    
    /* TODO: Post to TheWin32Keyboard->addWin32Event(pressed ? WM_KEYDOWN : WM_KEYUP, vkey, ...) */
    
    return 0;
}

/* ============================================================================
 * MOUSE EVENT POSTING
 * ============================================================================ */

int SDL2_PostAnalogStickMouseEvent(int gamepad_index, int stick_id, float sensitivity)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return -1;
    }
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (stick_id < 0 || stick_id > 1) {
        return -1;
    }
    
    SDL2_GamepadState state;
    if (SDL2_GetGamepadState(gamepad_index, &state) != 0) {
        return -1;
    }
    
    if (!state.connected) {
        return -1;
    }
    
    /* Get axis values for the stick */
    SDL2_GamepadAxis axis_x = (stick_id == 0) ? SDL2_GAMEPAD_AXIS_LEFTX : SDL2_GAMEPAD_AXIS_RIGHTX;
    SDL2_GamepadAxis axis_y = (stick_id == 0) ? SDL2_GAMEPAD_AXIS_LEFTY : SDL2_GAMEPAD_AXIS_RIGHTY;
    
    float normalized_x = SDL2_GetGamepadAxisNormalized(gamepad_index, axis_x);
    float normalized_y = SDL2_GetGamepadAxisNormalized(gamepad_index, axis_y);
    
    /* Apply sensitivity multiplier */
    int mouse_dx = (int)(normalized_x * sensitivity * 10.0f);  /* Scale to reasonable mouse pixels */
    int mouse_dy = (int)(normalized_y * sensitivity * 10.0f);
    
    const char *stick_name = (stick_id == 0) ? "left" : "right";
    
    if (mouse_dx != 0 || mouse_dy != 0) {
        printf("Phase 07: Gamepad %d %s stick → mouse delta (%d, %d)\n",
               gamepad_index, stick_name, mouse_dx, mouse_dy);
    }
    
    /* TODO: Post to TheWin32Mouse->addWin32Event(WM_MOUSEMOVE, ..., mouse_dx, mouse_dy) */
    
    return 0;
}

int SDL2_PostMouseEvent(int dx, int dy, int button, BOOL pressed)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return -1;
    }
    
    const char *button_name = "none";
    if (button == 1) button_name = "left";
    else if (button == 2) button_name = "right";
    else if (button == 3) button_name = "middle";
    
    printf("Phase 07: Posting mouse event: delta (%d, %d) button %s %s\n",
           dx, dy, button_name, pressed ? "DOWN" : "UP");
    
    /* TODO: Post to TheWin32Mouse->addWin32Event() */
    
    return 0;
}

/* ============================================================================
 * TRIGGER MAPPING / FIRE COMMANDS
 * ============================================================================ */

int SDL2_PostGamepadTriggerFireEvent(int gamepad_index, int trigger_id, float fire_threshold)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return -1;
    }
    
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (trigger_id < 0 || trigger_id > 1) {
        return -1;
    }
    
    SDL2_GamepadState state;
    if (SDL2_GetGamepadState(gamepad_index, &state) != 0) {
        return -1;
    }
    
    if (!state.connected) {
        return -1;
    }
    
    SDL2_GamepadAxis trigger_axis = (trigger_id == 0) ? 
        SDL2_GAMEPAD_AXIS_TRIGGERLEFT : SDL2_GAMEPAD_AXIS_TRIGGERRIGHT;
    
    float trigger_value = SDL2_GetGamepadAxisNormalized(gamepad_index, trigger_axis);
    
    const char *trigger_name = (trigger_id == 0) ? "left" : "right";
    const char *fire_type = (trigger_id == 0) ? "primary fire" : "secondary fire";
    
    if (trigger_value > fire_threshold) {
        printf("Phase 07: Gamepad %d %s trigger: %.2f (above threshold %.2f) → %s\n",
               gamepad_index, trigger_name, trigger_value, fire_threshold, fire_type);
        
        /* TODO: Post game-specific fire command event */
        /* Depending on game architecture:
           - Post to input queue as custom event
           - Call game fire function directly
           - Set fire flag for game logic to read
         */
    }
    
    return 0;
}

/* ============================================================================
 * POLLING / MAIN LOOP INTEGRATION
 * ============================================================================ */

int SDL2_UpdateGamepadEvents(void)
{
    if (!g_event_integration_initialized || !g_event_generation_enabled) {
        return 0;
    }
    
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    
    for (int i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        SDL2_GamepadState state;
        if (SDL2_GetGamepadState(i, &state) != 0 || !state.connected) {
            continue;
        }
        
        /* Check button state changes */
        for (int button = 0; button < SDL2_GAMEPAD_BUTTON_COUNT; button++) {
            BOOL current = state.buttons[button];
            BOOL last = g_event_config[i].last_button_state[button];
            
            if (current != last) {
                SDL2_PostGamepadButtonKeyboardEvent(i, button, current);
                g_event_config[i].last_button_state[button] = current;
            }
        }
        
        /* Update analog stick events */
        SDL2_PostAnalogStickMouseEvent(i, 0, g_event_config[i].stick_sensitivity[0]);
        SDL2_PostAnalogStickMouseEvent(i, 1, g_event_config[i].stick_sensitivity[1]);
        
        /* Update trigger fire events */
        if (g_event_config[i].trigger_fire_enabled[0]) {
            SDL2_PostGamepadTriggerFireEvent(i, 0, g_event_config[i].trigger_fire_threshold[0]);
        }
        if (g_event_config[i].trigger_fire_enabled[1]) {
            SDL2_PostGamepadTriggerFireEvent(i, 1, g_event_config[i].trigger_fire_threshold[1]);
        }
    }
    
    return 0;
}

/* ============================================================================
 * SENSITIVITY & CONFIGURATION
 * ============================================================================ */

int SDL2_SetAnalogStickSensitivity(int gamepad_index, int stick_id, float sensitivity)
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
    
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    
    g_event_config[gamepad_index].stick_sensitivity[stick_id] = sensitivity;
    
    const char *stick_name = (stick_id == 0) ? "left" : "right";
    printf("Phase 07: Gamepad %d %s stick sensitivity set to %.2f\n",
           gamepad_index, stick_name, sensitivity);
    
    return 0;
}

int SDL2_SetTriggerFireThreshold(int gamepad_index, int trigger_id, float fire_threshold)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (trigger_id < 0 || trigger_id > 1) {
        return -1;
    }
    
    if (fire_threshold < 0.0f || fire_threshold > 1.0f) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    
    g_event_config[gamepad_index].trigger_fire_threshold[trigger_id] = fire_threshold;
    
    const char *trigger_name = (trigger_id == 0) ? "left" : "right";
    printf("Phase 07: Gamepad %d %s trigger fire threshold set to %.2f\n",
           gamepad_index, trigger_name, fire_threshold);
    
    return 0;
}

int SDL2_SetTriggerFireEnabled(int gamepad_index, int trigger_id, BOOL enabled)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (trigger_id < 0 || trigger_id > 1) {
        return -1;
    }
    
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    
    g_event_config[gamepad_index].trigger_fire_enabled[trigger_id] = enabled;
    
    const char *trigger_name = (trigger_id == 0) ? "left" : "right";
    printf("Phase 07: Gamepad %d %s trigger fire %s\n",
           gamepad_index, trigger_name, enabled ? "enabled" : "disabled");
    
    return 0;
}

/* ============================================================================
 * DEBUG / DIAGNOSTICS
 * ============================================================================ */

void SDL2_DumpGamepadEventState(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_event_integration_lock);
    
    printf("\nPhase 07: Gamepad Event Integration State\n");
    printf("==========================================\n");
    printf("Event generation: %s\n", g_event_generation_enabled ? "enabled" : "disabled");
    
    if (gamepad_index >= 0 && gamepad_index < SDL2_MAX_GAMEPADS) {
        /* Single gamepad */
        printf("\nGamepad %d:\n", gamepad_index);
        printf("  Left stick sensitivity:    %.2f\n", g_event_config[gamepad_index].stick_sensitivity[0]);
        printf("  Right stick sensitivity:   %.2f\n", g_event_config[gamepad_index].stick_sensitivity[1]);
        printf("  Left trigger fire threshold:  %.2f\n", g_event_config[gamepad_index].trigger_fire_threshold[0]);
        printf("  Right trigger fire threshold: %.2f\n", g_event_config[gamepad_index].trigger_fire_threshold[1]);
        printf("  Left trigger fire enabled:  %s\n", g_event_config[gamepad_index].trigger_fire_enabled[0] ? "yes" : "no");
        printf("  Right trigger fire enabled: %s\n", g_event_config[gamepad_index].trigger_fire_enabled[1] ? "yes" : "no");
    } else {
        /* All gamepads */
        for (int i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            printf("\nGamepad %d:\n", i);
            printf("  Sticks: L=%.2f R=%.2f\n",
                   g_event_config[i].stick_sensitivity[0],
                   g_event_config[i].stick_sensitivity[1]);
            printf("  Triggers: L_thr=%.2f L_en=%s R_thr=%.2f R_en=%s\n",
                   g_event_config[i].trigger_fire_threshold[0],
                   g_event_config[i].trigger_fire_enabled[0] ? "y" : "n",
                   g_event_config[i].trigger_fire_threshold[1],
                   g_event_config[i].trigger_fire_enabled[1] ? "y" : "n");
        }
    }
    printf("\n");
}

int SDL2_TestGamepadEvents(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        printf("Phase 07: Invalid gamepad index: %d\n", gamepad_index);
        return -1;
    }
    
    SDL2_GamepadState state;
    if (SDL2_GetGamepadState(gamepad_index, &state) != 0) {
        printf("Phase 07: Failed to get gamepad %d state\n", gamepad_index);
        return -1;
    }
    
    if (!state.connected) {
        printf("Phase 07: Gamepad %d not connected\n", gamepad_index);
        return -1;
    }
    
    printf("\nPhase 07: Testing Gamepad %d Event Generation\n", gamepad_index);
    printf("============================================\n");
    printf("Connected: %s\n", state.connected ? "yes" : "no");
    printf("Name: %s\n", state.name);
    printf("Vendor ID: 0x%04lX\n", (unsigned long)state.vendor_id);
    printf("Product ID: 0x%04lX\n", (unsigned long)state.product_id);
    
    printf("\nButton States:\n");
    for (int i = 0; i < SDL2_GAMEPAD_BUTTON_COUNT; i++) {
        if (state.buttons[i]) {
            char button_name[64];
            SDL2_GetGamepadButtonName((SDL2_GamepadButton)i, button_name, sizeof(button_name));
            printf("  Button %d (%s): PRESSED\n", i, button_name);
        }
    }
    
    printf("\nAxis States (normalized -1.0 to 1.0):\n");
    for (int i = 0; i < SDL2_GAMEPAD_AXIS_COUNT; i++) {
        float normalized = SDL2_GetGamepadAxisNormalized(gamepad_index, (SDL2_GamepadAxis)i);
        if (fabs(normalized) > 0.1f) {  /* Only show non-trivial values */
            char axis_name[64];
            SDL2_GetGamepadAxisName((SDL2_GamepadAxis)i, axis_name, sizeof(axis_name));
            printf("  Axis %d (%s): %.2f\n", i, axis_name, normalized);
        }
    }
    
    printf("\nEvent Configuration:\n");
    SDL2_DumpGamepadEventState(gamepad_index);
    
    printf("Phase 07: Test complete\n\n");
    return 0;
}
