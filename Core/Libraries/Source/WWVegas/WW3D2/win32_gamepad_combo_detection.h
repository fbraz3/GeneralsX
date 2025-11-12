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
**  FILE: win32_gamepad_combo_detection.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Button Combo Detection & Sequence Recognition
**
**  DESCRIPTION:
**    Implements input combo detection for fighting game-style button sequences.
**    Detects multi-button press patterns with timing constraints.
**
******************************************************************************/

#ifndef __WIN32_GAMEPAD_COMBO_DETECTION_H__
#define __WIN32_GAMEPAD_COMBO_DETECTION_H__

#include "win32_gamepad_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * COMBO PATTERN DEFINITIONS
 * ============================================================================ */

#define GAMEPAD_COMBO_MAX_BUTTONS 8         /* Max buttons in sequence */
#define GAMEPAD_COMBO_MAX_PATTERNS 64       /* Max combo patterns */
#define GAMEPAD_COMBO_MAX_GAMEPADS 4
#define GAMEPAD_COMBO_PATTERN_NAME_SIZE 64  /* Max pattern name length */
#define GAMEPAD_COMBO_MAX_TIMEOUT 5000      /* 5 second max combo timeout */
#define GAMEPAD_COMBO_DEFAULT_TIMEOUT 2000  /* 2 second default timeout */

typedef enum {
    COMBO_BUTTON_ANY = -1,      /* Wildcard - any button */
    COMBO_BUTTON_NONE = 0,      /* End of sequence */
    COMBO_BUTTON_A = 1,
    COMBO_BUTTON_B = 2,
    COMBO_BUTTON_X = 3,
    COMBO_BUTTON_Y = 4,
    COMBO_BUTTON_LB = 5,
    COMBO_BUTTON_RB = 6,
    COMBO_BUTTON_BACK = 7,
    COMBO_BUTTON_START = 8,
} GamepadComboButton;

typedef struct {
    char name[GAMEPAD_COMBO_PATTERN_NAME_SIZE];     /* Pattern name */
    GamepadComboButton buttons[GAMEPAD_COMBO_MAX_BUTTONS]; /* Button sequence */
    unsigned int timeout_ms;                        /* Max time between inputs */
    unsigned int combo_id;                          /* Unique combo ID */
    BOOL enabled;                                   /* Pattern active */
} GamepadComboPattern;

typedef struct {
    unsigned int last_combo_id;                     /* Last detected combo */
    unsigned int combo_count;                       /* Total combos detected this frame */
    unsigned int pattern_match_count;               /* Total pattern matches */
} GamepadComboStats;

/* ============================================================================
 * COMBO SYSTEM INITIALIZATION
 * ============================================================================ */

/**
 * Initialize button combo detection system
 * Must be called before registering any combo patterns
 * 
 * Returns: 0 on success, -1 on failure
 */
int SDL2_InitGamepadComboDetection(void);

/**
 * Shutdown combo detection system
 * Called at game shutdown
 */
void SDL2_ShutdownGamepadComboDetection(void);

/**
 * Enable/disable combo detection
 * 
 * enabled: TRUE to enable detection, FALSE to disable
 * 
 * Returns: 0 on success
 */
int SDL2_SetGamepadComboDetectionEnabled(BOOL enabled);

/**
 * Check if combo detection is enabled
 * 
 * Returns: TRUE if enabled, FALSE otherwise
 */
BOOL SDL2_IsGamepadComboDetectionEnabled(void);

/* ============================================================================
 * COMBO PATTERN REGISTRATION
 * ============================================================================ */

/**
 * Register a new combo pattern
 * 
 * pattern: Combo pattern to register (must be valid)
 * combo_id_out: Output pointer for unique combo ID (may be NULL)
 * 
 * Returns: 0 on success, -1 if too many patterns registered
 */
int SDL2_RegisterComboPattern(const GamepadComboPattern *pattern, unsigned int *combo_id_out);

/**
 * Unregister combo pattern by ID
 * 
 * combo_id: Pattern ID returned from SDL2_RegisterComboPattern
 * 
 * Returns: 0 on success, -1 if pattern not found
 */
int SDL2_UnregisterComboPattern(unsigned int combo_id);

/**
 * Get registered pattern by ID
 * 
 * combo_id: Pattern ID
 * pattern_out: Output buffer for pattern (must be non-NULL)
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_GetComboPattern(unsigned int combo_id, GamepadComboPattern *pattern_out);

/**
 * List all registered patterns
 * 
 * patterns_out: Output buffer for patterns
 * max_patterns: Maximum patterns to return
 * 
 * Returns: Number of patterns returned, -1 on error
 */
int SDL2_ListComboPatterns(GamepadComboPattern *patterns_out, int max_patterns);

/**
 * Find pattern by name
 * 
 * pattern_name: Name of pattern to find
 * 
 * Returns: Pattern ID if found, -1 if not found
 */
unsigned int SDL2_FindComboPatternByName(const char *pattern_name);

/**
 * Disable all patterns
 * 
 * Returns: 0 on success
 */
int SDL2_DisableAllComboPatterns(void);

/**
 * Enable specific pattern
 * 
 * combo_id: Pattern ID
 * enabled: TRUE to enable, FALSE to disable
 * 
 * Returns: 0 on success
 */
int SDL2_SetComboPatternEnabled(unsigned int combo_id, BOOL enabled);

/* ============================================================================
 * COMBO INPUT PROCESSING
 * ============================================================================ */

/**
 * Process button input for combo detection
 * Call for every button press/release on gamepad
 * 
 * gamepad_index: Gamepad source
 * button: Button pressed/released
 * pressed: TRUE for press, FALSE for release
 * 
 * Returns: Matched combo ID if combo detected, -1 if no match
 */
int SDL2_ProcessGamepadComboInput(int gamepad_index, GamepadComboButton button, BOOL pressed);

/**
 * Update combo detection system each frame
 * Handles combo timeout cleanup and state management
 * 
 * delta_time: Time since last frame (milliseconds)
 * 
 * Returns: 0 on success
 */
int SDL2_UpdateGamepadComboDetection(unsigned int delta_time);

/**
 * Reset combo detection state for gamepad
 * Clears pending button sequences
 * 
 * gamepad_index: Gamepad to reset (-1 for all)
 * 
 * Returns: 0 on success
 */
int SDL2_ResetGamepadComboState(int gamepad_index);

/* ============================================================================
 * COMBO DETECTION QUERYING
 * ============================================================================ */

/**
 * Check if specific combo was detected this frame
 * 
 * combo_id: Pattern ID to check
 * 
 * Returns: 1 if detected, 0 if not detected
 */
int SDL2_WasComboDetected(unsigned int combo_id);

/**
 * Get last detected combo ID
 * 
 * gamepad_index: Gamepad to check
 * 
 * Returns: Last combo ID, or -1 if none detected
 */
unsigned int SDL2_GetLastDetectedCombo(int gamepad_index);

/**
 * Get combo detection statistics
 * 
 * gamepad_index: Gamepad to get stats for
 * stats_out: Output buffer for statistics
 * 
 * Returns: 0 on success
 */
int SDL2_GetComboDetectionStats(int gamepad_index, GamepadComboStats *stats_out);

/**
 * Get current button sequence for gamepad
 * Shows buttons entered so far in combo detection
 * 
 * gamepad_index: Gamepad to query
 * buttons_out: Output buffer for buttons
 * max_buttons: Maximum buttons to return
 * 
 * Returns: Number of buttons in sequence, -1 on error
 */
int SDL2_GetGamepadComboSequence(int gamepad_index, GamepadComboButton *buttons_out, int max_buttons);

/**
 * Get time remaining for current combo sequence
 * 
 * gamepad_index: Gamepad to query
 * 
 * Returns: Milliseconds remaining, or 0 if no active sequence
 */
unsigned int SDL2_GetGamepadComboTimeRemaining(int gamepad_index);

/* ============================================================================
 * PREDEFINED COMBO TEMPLATES
 * ============================================================================ */

/**
 * Create "Hadoken" style combo pattern (fireball - classic fighting game)
 * Pattern: Down -> Down-Right -> Right, punch
 * Mapped to: Y -> X -> B, then A for example
 * 
 * pattern_out: Output buffer for pattern
 * 
 * Returns: 0 on success
 */
int SDL2_CreateHadokenComboPattern(GamepadComboPattern *pattern_out);

/**
 * Create "Shoryuken" style combo (rising punch - classic fighting game)
 * Pattern: Right -> Down -> Down-Right, punch
 * Mapped to: B -> X -> Y, then A for example
 * 
 * pattern_out: Output buffer for pattern
 * 
 * Returns: 0 on success
 */
int SDL2_CreateShoryukenComboPattern(GamepadComboPattern *pattern_out);

/**
 * Create "Konami Code" style pattern (up-up-down-down-left-right-left-right-B-A)
 * Classic secret combo pattern
 * 
 * pattern_out: Output buffer for pattern
 * 
 * Returns: 0 on success
 */
int SDL2_CreateKonamiCodePattern(GamepadComboPattern *pattern_out);

/**
 * Create custom combo pattern from button sequence
 * 
 * name: Pattern name
 * buttons: Array of buttons (terminated with COMBO_BUTTON_NONE)
 * timeout_ms: Timeout between inputs
 * pattern_out: Output buffer for pattern
 * 
 * Returns: 0 on success, -1 if invalid
 */
int SDL2_CreateCustomComboPattern(const char *name, const GamepadComboButton *buttons,
                                  unsigned int timeout_ms, GamepadComboPattern *pattern_out);

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump combo detection state for debugging
 * 
 * gamepad_index: Gamepad to dump (-1 for all)
 * 
 * Returns: 0 on success
 */
int SDL2_DumpComboDetectionState(int gamepad_index);

/**
 * Dump all registered combo patterns
 * 
 * Returns: 0 on success
 */
int SDL2_DumpRegisteredComboPatterns(void);

/**
 * Get human-readable name for combo button
 * 
 * button: GamepadComboButton enum value
 * name_out: Output buffer for name
 * name_size: Size of output buffer
 * 
 * Returns: 0 on success
 */
int SDL2_GetComboButtonName(GamepadComboButton button, char *name_out, size_t name_size);

#ifdef __cplusplus
}
#endif

#endif /* __WIN32_GAMEPAD_COMBO_DETECTION_H__ */
