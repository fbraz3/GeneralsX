/*
 * GeneralsX - Cross-Platform Port of Command & Conquer: Generals (2003)
 * Copyright (C) 2025
 *
 * Phase 06: Complete Input System - Gamepad/Joystick Support
 *
 * This file extends Phase 03 SDL2 keyboard/mouse to add gamepad/joystick support.
 * Provides Windows-compatible gamepad API using SDL2 backend.
 *
 * Supported Gamepad Types:
 * - Xbox 360 controllers
 * - PlayStation controllers
 * - Nintendo controllers
 * - Generic USB gamepads (HID)
 *
 * Button Mapping (to Windows DirectInput codes):
 * - A/Cross (0) → DIK_SPACE
 * - B/Circle (1) → DIK_ESCAPE
 * - X/Square (2) → DIK_LSHIFT
 * - Y/Triangle (3) → DIK_LCONTROL
 * - LB/L1 (4) → DIK_Q
 * - RB/R1 (5) → DIK_E
 * - Back/Select (6) → DIK_TAB
 * - Start (7) → DIK_RETURN
 * - Left Stick Click (8) → DIK_L
 * - Right Stick Click (9) → DIK_R
 *
 * Axis Mapping (Analog Sticks & Triggers):
 * - Left Stick X/Y → Mouse movement simulation
 * - Right Stick X/Y → Camera rotation (game-specific)
 * - Left Trigger (LT) → Weapon fire
 * - Right Trigger (RT) → Alt fire
 */

#ifndef __WIN32_GAMEPAD_COMPAT_H__
#define __WIN32_GAMEPAD_COMPAT_H__

#include "win32_sdl_types_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Gamepad Constants
 */
#define SDL2_MAX_GAMEPADS           4   /* Maximum simultaneous gamepads */
#define SDL2_GAMEPAD_AXIS_MAX       32767  /* Max analog stick value */
#define SDL2_GAMEPAD_AXIS_MIN       -32768  /* Min analog stick value */
#define SDL2_GAMEPAD_AXIS_DEADZONE  5000  /* Deadzone for axis movement */
#define SDL2_GAMEPAD_TRIGGER_MAX    255  /* Max trigger value */


/*
 * Gamepad Button Codes (SDL2 standard)
 */
typedef enum {
    SDL2_GAMEPAD_BUTTON_A           = 0,  /* A / Cross */
    SDL2_GAMEPAD_BUTTON_B           = 1,  /* B / Circle */
    SDL2_GAMEPAD_BUTTON_X           = 2,  /* X / Square */
    SDL2_GAMEPAD_BUTTON_Y           = 3,  /* Y / Triangle */
    SDL2_GAMEPAD_BUTTON_LB          = 4,  /* LB / L1 */
    SDL2_GAMEPAD_BUTTON_RB          = 5,  /* RB / R1 */
    SDL2_GAMEPAD_BUTTON_BACK        = 6,  /* Back / Select */
    SDL2_GAMEPAD_BUTTON_START       = 7,  /* Start */
    SDL2_GAMEPAD_BUTTON_LSTICK      = 8,  /* Left Stick Click */
    SDL2_GAMEPAD_BUTTON_RSTICK      = 9,  /* Right Stick Click */
    SDL2_GAMEPAD_BUTTON_COUNT       = 10
} SDL2_GamepadButton;


/*
 * Gamepad Axis Codes (SDL2 standard)
 */
typedef enum {
    SDL2_GAMEPAD_AXIS_LEFTX         = 0,  /* Left Stick X (-32768 to 32767) */
    SDL2_GAMEPAD_AXIS_LEFTY         = 1,  /* Left Stick Y (-32768 to 32767) */
    SDL2_GAMEPAD_AXIS_RIGHTX        = 2,  /* Right Stick X (-32768 to 32767) */
    SDL2_GAMEPAD_AXIS_RIGHTY        = 3,  /* Right Stick Y (-32768 to 32767) */
    SDL2_GAMEPAD_AXIS_TRIGGERLEFT   = 4,  /* Left Trigger (0 to 32767) */
    SDL2_GAMEPAD_AXIS_TRIGGERRIGHT  = 5,  /* Right Trigger (0 to 32767) */
    SDL2_GAMEPAD_AXIS_COUNT         = 6
} SDL2_GamepadAxis;


/*
 * Gamepad State Structure
 */
typedef struct {
    BOOL connected;                         /* Is gamepad connected? */
    char name[256];                         /* Gamepad name/model */
    DWORD vendor_id;                        /* USB Vendor ID */
    DWORD product_id;                       /* USB Product ID */
    BOOL buttons[SDL2_GAMEPAD_BUTTON_COUNT]; /* Button states (0=released, 1=pressed) */
    short axes[SDL2_GAMEPAD_AXIS_COUNT];    /* Axis values */
} SDL2_GamepadState;


/*
 * Gamepad Input Mapping Structure
 * Maps gamepad buttons/axes to keyboard/mouse actions
 */
typedef struct {
    /* Button Mappings */
    BYTE button_to_vkey[SDL2_GAMEPAD_BUTTON_COUNT];  /* Maps gamepad button to VK_* code */
    
    /* Axis Mappings */
    BOOL axis_triggers_fire;    /* Whether triggers should fire weapon */
    float axis_sensitivity;     /* Analog stick sensitivity (0.0-2.0) */
    float axis_deadzone;        /* Deadzone threshold (0.0-1.0) */
} SDL2_GamepadMapping;


/*
 * Gamepad Management Functions
 */

/**
 * Initialize gamepad system
 * Scans for connected gamepads and prepares for input
 * Must be called before any gamepad queries
 *
 * Returns: 0 on success, -1 on error
 */
int SDL2_InitGamepads(void);

/**
 * Shutdown gamepad system
 * Cleanup and resource deallocation
 */
void SDL2_ShutdownGamepads(void);

/**
 * Get number of connected gamepads
 *
 * Returns: Number of connected gamepads (0-4)
 */
int SDL2_GetGamepadCount(void);

/**
 * Get gamepad state by index
 *
 * Parameters:
 *   index: Gamepad index (0 to SDL2_GetGamepadCount()-1)
 *   state: Output gamepad state structure
 *
 * Returns: 0 on success, -1 if gamepad index invalid
 */
int SDL2_GetGamepadState(int index, SDL2_GamepadState *state);

/**
 * Get gamepad name (e.g., "Xbox 360 Wireless Controller")
 *
 * Parameters:
 *   index: Gamepad index
 *   buffer: Output buffer for name
 *   buffer_size: Size of output buffer
 *
 * Returns: 0 on success, -1 if invalid
 */
int SDL2_GetGamepadName(int index, char *buffer, unsigned int buffer_size);

/**
 * Check if button is pressed
 *
 * Parameters:
 *   index: Gamepad index
 *   button: Button code (SDL2_GAMEPAD_BUTTON_*)
 *
 * Returns: 1 if pressed, 0 if released, -1 if invalid
 */
int SDL2_IsGamepadButtonPressed(int index, SDL2_GamepadButton button);

/**
 * Get analog axis value
 *
 * Parameters:
 *   index: Gamepad index
 *   axis: Axis code (SDL2_GAMEPAD_AXIS_*)
 *
 * Returns: Axis value (-32768 to 32767), 0 if invalid
 */
short SDL2_GetGamepadAxis(int index, SDL2_GamepadAxis axis);

/**
 * Get analog axis value as normalized float (-1.0 to 1.0)
 * Automatically applies deadzone filtering
 *
 * Parameters:
 *   index: Gamepad index
 *   axis: Axis code
 *
 * Returns: Normalized value (-1.0 to 1.0), 0.0 if within deadzone
 */
float SDL2_GetGamepadAxisNormalized(int index, SDL2_GamepadAxis axis);

/**
 * Rumble/vibration feedback
 *
 * Parameters:
 *   index: Gamepad index
 *   left_intensity: Left motor intensity (0-255)
 *   right_intensity: Right motor intensity (0-255)
 *   duration_ms: Duration in milliseconds
 *
 * Returns: 0 on success, -1 if not supported or invalid
 */
int SDL2_SetGamepadRumble(int index, BYTE left_intensity, BYTE right_intensity, unsigned int duration_ms);


/*
 * Gamepad Input Mapping Functions
 * Maps gamepad inputs to Windows key codes
 */

/**
 * Get default gamepad button mapping
 * Creates standard mapping (A→SPACE, B→ESC, etc.)
 *
 * Parameters:
 *   mapping: Output mapping structure
 *
 * Returns: 0 on success
 */
int SDL2_GetDefaultGamepadMapping(SDL2_GamepadMapping *mapping);

/**
 * Load gamepad mapping from configuration (Phase 05 Registry/INI)
 *
 * Parameters:
 *   index: Gamepad index
 *   mapping: Output mapping structure
 *
 * Returns: 0 on success, -1 if not found (will use defaults)
 */
int SDL2_LoadGamepadMapping(int index, SDL2_GamepadMapping *mapping);

/**
 * Save gamepad mapping to configuration (Phase 05 Registry/INI)
 *
 * Parameters:
 *   index: Gamepad index
 *   mapping: Mapping to save
 *
 * Returns: 0 on success, -1 on error
 */
int SDL2_SaveGamepadMapping(int index, const SDL2_GamepadMapping *mapping);

/**
 * Apply gamepad input mapping and generate keyboard/mouse events
 * Translates gamepad button/axis input to simulated keyboard/mouse events
 * Posts events to TheWin32Keyboard and TheWin32Mouse event queues
 *
 * Parameters:
 *   index: Gamepad index
 *   mapping: Mapping to apply
 *
 * Returns: 0 on success, -1 on error
 *
 * Side Effects:
 * - Posts WM_KEYDOWN/WM_KEYUP to keyboard event queue
 * - Posts mouse motion/button events to mouse event queue
 * - Updates axis sensitivity based on mapping
 */
int SDL2_ApplyGamepadMapping(int index, const SDL2_GamepadMapping *mapping);


/*
 * Event Processing Functions
 * Called by SDL2 event loop
 */

/**
 * Process SDL gamepad event (internal use)
 * Called from SDL event loop when gamepad changes state
 *
 * Parameters:
 *   event: SDL_GamepadEvent structure
 *
 * Returns: 0 if handled, -1 if not recognized
 */
int SDL2_ProcessGamepadEvent(const void *event);

/**
 * Process SDL joystick event (internal use)
 * Legacy joystick input support
 *
 * Parameters:
 *   event: SDL_JoystickEvent structure
 *
 * Returns: 0 if handled, -1 if not recognized
 */
int SDL2_ProcessJoystickEvent(const void *event);


/*
 * Utility Functions
 */

/**
 * Get gamepad button name (e.g., "A Button", "Left Trigger")
 *
 * Parameters:
 *   button: Button code
 *   buffer: Output buffer
 *   buffer_size: Size of buffer
 *
 * Returns: 0 on success, -1 if invalid button
 */
int SDL2_GetGamepadButtonName(SDL2_GamepadButton button, char *buffer, unsigned int buffer_size);

/**
 * Get gamepad axis name (e.g., "Left Stick X")
 *
 * Parameters:
 *   axis: Axis code
 *   buffer: Output buffer
 *   buffer_size: Size of buffer
 *
 * Returns: 0 on success, -1 if invalid axis
 */
int SDL2_GetGamepadAxisName(SDL2_GamepadAxis axis, char *buffer, unsigned int buffer_size);

/**
 * Test gamepad connectivity and functionality
 * Diagnostic function for debugging gamepad issues
 *
 * Parameters:
 *   index: Gamepad index to test
 *
 * Returns: 0 if working, -1 if not connected, -2 if error
 */
int SDL2_TestGamepad(int index);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* __WIN32_GAMEPAD_COMPAT_H__ */
