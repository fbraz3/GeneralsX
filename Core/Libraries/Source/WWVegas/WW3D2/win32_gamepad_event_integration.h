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
**  FILE: win32_gamepad_event_integration.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Gamepad Event Queue Integration Layer
**
**  DESCRIPTION:
**    Connects gamepad input (Phase 06) to keyboard and mouse event queues.
**    Translates gamepad buttons to keyboard events (WM_KEYDOWN/WM_KEYUP).
**    Translates analog sticks to mouse movement (WM_MOUSEMOVE).
**    Translates triggers to fire commands (custom game events).
**
**  ARCHITECTURE:
**    Phase 07: Event Queue Integration & Configuration Persistence
**    - Part A: Event Integration (this file + .cpp)
**    - Part B: Configuration Persistence (win32_gamepad_config_compat.h/cpp)
**    
**    Integration Path:
**    Phase 06 (Gamepad State) → Phase 07A (Event Translation) → Keyboard/Mouse Queues
**
******************************************************************************/

#ifndef WIN32_GAMEPAD_EVENT_INTEGRATION_H
#define WIN32_GAMEPAD_EVENT_INTEGRATION_H

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * GAMEPAD EVENT INTEGRATION - INITIALIZATION & CONTROL
 * ============================================================================ */

/**
 * Initialize gamepad event integration
 * Sets up event queues, state tracking, and integration with keyboard/mouse
 *
 * @return  0 on success, -1 on error
 */
int SDL2_InitGamepadEventIntegration(void);

/**
 * Shutdown gamepad event integration
 * Cleanup and resource deallocation
 */
void SDL2_ShutdownGamepadEventIntegration(void);

/**
 * Enable/disable gamepad event generation
 * Useful for temporarily disabling gamepad input (e.g., during menu)
 *
 * @param enabled  1 to enable, 0 to disable
 */
void SDL2_SetGamepadEventGenerationEnabled(BOOL enabled);

/* ============================================================================
 * KEYBOARD EVENT POSTING - Gamepad buttons to keyboard queue
 * ============================================================================ */

/**
 * Post keyboard event for gamepad button press
 * Translates gamepad button through mapping to keyboard VK_* code
 * Posts WM_KEYDOWN to keyboard input queue
 *
 * @param gamepad_index  Gamepad index (0-3)
 * @param button         Gamepad button (SDL2_GAMEPAD_BUTTON_*)
 * @return               0 on success, -1 on error
 */
int SDL2_PostGamepadButtonKeyboardEvent(int gamepad_index, int button, BOOL pressed);

/**
 * Post raw keyboard event
 * Direct posting to keyboard input queue without mapping
 *
 * @param vkey    Virtual key code (VK_*)
 * @param pressed 1 for press, 0 for release
 * @return        0 on success, -1 on error
 */
int SDL2_PostKeyboardEvent(BYTE vkey, BOOL pressed);

/* ============================================================================
 * MOUSE EVENT POSTING - Analog sticks to mouse movement
 * ============================================================================ */

/**
 * Post mouse movement event for analog stick
 * Translates analog stick values to mouse movement delta
 * Posts WM_MOUSEMOVE to mouse input queue
 *
 * @param gamepad_index  Gamepad index (0-3)
 * @param stick_id       0 for left stick, 1 for right stick
 * @param sensitivity    Sensitivity multiplier (0.5-2.0, default 1.0)
 * @return               0 on success, -1 on error
 *
 * BEHAVIOR:
 * - Left stick (ID=0): Mapped to unit movement in game
 * - Right stick (ID=1): Mapped to camera/view rotation
 * - Deadzone filtering already applied (Phase 06)
 * - Normalization to -1.0 to 1.0 scale
 */
int SDL2_PostAnalogStickMouseEvent(int gamepad_index, int stick_id, float sensitivity);

/**
 * Post raw mouse event
 * Direct posting to mouse input queue
 *
 * @param dx     Delta X (pixels)
 * @param dy     Delta Y (pixels)
 * @param button Button code (0=none, 1=left, 2=right, 3=middle)
 * @param pressed 1 for press, 0 for release
 * @return        0 on success, -1 on error
 */
int SDL2_PostMouseEvent(int dx, int dy, int button, BOOL pressed);

/* ============================================================================
 * TRIGGER MAPPING - Fire command generation
 * ============================================================================ */

/**
 * Post fire command for trigger input
 * Translates trigger axis to fire intensity (0.0-1.0)
 * Posts game-specific fire event (custom implementation dependent)
 *
 * @param gamepad_index  Gamepad index (0-3)
 * @param trigger_id     0 for left trigger, 1 for right trigger
 * @param fire_threshold Threshold for firing (0.0-1.0, default 0.5)
 * @return               0 on success, -1 on error
 *
 * BEHAVIOR:
 * - Left trigger (ID=0): Primary fire
 * - Right trigger (ID=1): Secondary fire / alt fire
 * - Intensity mapped to fire strength (some weapons respond to pressure)
 */
int SDL2_PostGamepadTriggerFireEvent(int gamepad_index, int trigger_id, float fire_threshold);

/* ============================================================================
 * POLLING / MAIN LOOP INTEGRATION
 * ============================================================================ */

/**
 * Update gamepad events from current state
 * Called each frame from main game loop
 * Processes gamepad state and posts appropriate events
 *
 * USAGE:
 *   while (game_running) {
 *       // ... handle keyboard/mouse ...
 *       SDL2_UpdateGamepadEvents();  // Process gamepad → events
 *       // ... update game logic ...
 *   }
 *
 * @return  0 on success, -1 on error
 */
int SDL2_UpdateGamepadEvents(void);

/* ============================================================================
 * SENSITIVITY & CONFIGURATION
 * ============================================================================ */

/**
 * Set analog stick sensitivity multiplier
 * Higher values = faster cursor/camera movement
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param stick_id      0 for left stick, 1 for right stick
 * @param sensitivity   Multiplier (0.1-3.0, default 1.0)
 * @return              0 on success
 */
int SDL2_SetAnalogStickSensitivity(int gamepad_index, int stick_id, float sensitivity);

/**
 * Set trigger fire threshold
 * Triggers above this threshold value generate fire events
 *
 * @param gamepad_index   Gamepad index (0-3)
 * @param trigger_id      0 for left trigger, 1 for right trigger
 * @param fire_threshold  Threshold (0.0-1.0, default 0.5)
 * @return                0 on success
 */
int SDL2_SetTriggerFireThreshold(int gamepad_index, int trigger_id, float fire_threshold);

/**
 * Enable/disable trigger-based firing
 * When disabled, triggers are ignored for fire events
 *
 * @param gamepad_index Gamepad index (0-3)
 * @param trigger_id    0 for left trigger, 1 for right trigger
 * @param enabled       1 to enable, 0 to disable
 * @return              0 on success
 */
int SDL2_SetTriggerFireEnabled(int gamepad_index, int trigger_id, BOOL enabled);

/* ============================================================================
 * DEBUG / DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump current gamepad event integration state for debugging
 * Prints configuration, sensitivities, fire thresholds
 *
 * @param gamepad_index Gamepad index (0-3), -1 for all
 */
void SDL2_DumpGamepadEventState(int gamepad_index);

/**
 * Test gamepad event generation
 * Simulates gamepad input and traces events
 *
 * @param gamepad_index Gamepad index (0-3)
 * @return              0 on success, -1 on error
 */
int SDL2_TestGamepadEvents(int gamepad_index);

#ifdef __cplusplus
}
#endif

#endif /* WIN32_GAMEPAD_EVENT_INTEGRATION_H */
