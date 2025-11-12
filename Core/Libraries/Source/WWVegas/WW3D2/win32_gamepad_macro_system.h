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
**  FILE: win32_gamepad_macro_system.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Input Macro Recording & Playback System
**
**  DESCRIPTION:
**    Records gamepad input sequences with timing preservation.
**    Allows playback of complex input patterns for automation.
**
******************************************************************************/

#ifndef __WIN32_GAMEPAD_MACRO_SYSTEM_H__
#define __WIN32_GAMEPAD_MACRO_SYSTEM_H__

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_thread_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * MACRO SYSTEM DEFINITIONS
 * ============================================================================ */

#define GAMEPAD_MACRO_MAX_FRAMES 2000           /* Max frames per macro */
#define GAMEPAD_MACRO_MAX_MACROS 32             /* Max saved macros */
#define GAMEPAD_MACRO_MAX_NAME 64               /* Macro name length */
#define GAMEPAD_MACRO_FRAME_TIME_MS 16          /* ~60 FPS frame time */

typedef enum {
    MACRO_EVENT_BUTTON_PRESS = 0,       /* Button pressed */
    MACRO_EVENT_BUTTON_RELEASE = 1,     /* Button released */
    MACRO_EVENT_AXIS_MOTION = 2,        /* Analog stick/trigger moved */
    MACRO_EVENT_PAUSE = 3,              /* Wait (delay) */
    MACRO_EVENT_VIBRATE = 4             /* Rumble trigger */
} GamepadMacroEventType;

typedef struct {
    GamepadMacroEventType type;
    unsigned short frame_offset;        /* Frames since last event */
    union {
        SDL2_GamepadButton button;      /* For button press/release */
        struct {
            SDL2_GamepadAxis axis;      /* For axis motion */
            short axis_value;           /* Normalized -32768 to 32767 */
        } axis_motion;
        unsigned int pause_ms;          /* For pause events */
        unsigned int vibration_pattern; /* For vibration (pattern ID) */
    } data;
} GamepadMacroEvent;

typedef struct {
    char name[GAMEPAD_MACRO_MAX_NAME];
    GamepadMacroEvent events[GAMEPAD_MACRO_MAX_FRAMES];
    int event_count;
    unsigned int total_duration_ms;     /* Total macro runtime */
    unsigned int recording_timestamp;   /* When macro was recorded */
    BOOL enabled;
    unsigned int macro_id;
} GamepadMacro;

typedef struct {
    unsigned int macro_id;
    int playback_position;              /* Current event index */
    unsigned int playback_time;         /* Current playback time */
    BOOL is_playing;
    BOOL loop;                          /* Loop playback */
    float playback_speed;               /* 0.5 = half speed, 2.0 = double */
} GamepadMacroPlayback;

/* ============================================================================
 * MACRO SYSTEM INITIALIZATION
 * ============================================================================ */

/**
 * Initialize macro recording/playback system
 * Must be called before any macro operations
 * 
 * Returns: 0 on success, -1 on failure
 */
int SDL2_InitGamepadMacroSystem(void);

/**
 * Shutdown macro system
 * Called at game shutdown
 */
void SDL2_ShutdownGamepadMacroSystem(void);

/**
 * Enable/disable entire macro system
 * 
 * enabled: TRUE to enable, FALSE to disable all playback
 * 
 * Returns: 0 on success
 */
int SDL2_SetGamepadMacroSystemEnabled(BOOL enabled);

/**
 * Check if macro system is enabled
 * 
 * Returns: TRUE if enabled, FALSE otherwise
 */
BOOL SDL2_IsGamepadMacroSystemEnabled(void);

/* ============================================================================
 * MACRO RECORDING
 * ============================================================================ */

/**
 * Start recording macro for gamepad
 * Records all button presses, releases, and axis motions with timing
 * 
 * gamepad_index: Gamepad to record from
 * macro_name: Name for the macro being recorded
 * 
 * Returns: 0 on success, -1 if already recording
 */
int SDL2_StartMacroRecording(int gamepad_index, const char *macro_name);

/**
 * Stop recording and save macro
 * Finalizes timing and stores macro
 * 
 * gamepad_index: Gamepad being recorded
 * macro_id_out: Output pointer for macro ID (may be NULL)
 * 
 * Returns: 0 on success, -1 if not recording or too many events
 */
int SDL2_StopMacroRecording(int gamepad_index, unsigned int *macro_id_out);

/**
 * Cancel ongoing recording without saving
 * 
 * gamepad_index: Gamepad being recorded
 * 
 * Returns: 0 on success, -1 if not recording
 */
int SDL2_CancelMacroRecording(int gamepad_index);

/**
 * Check if recording is active for gamepad
 * 
 * gamepad_index: Gamepad to check
 * 
 * Returns: 1 if recording, 0 if not recording, -1 on error
 */
int SDL2_IsMacroRecording(int gamepad_index);

/**
 * Get current recording stats
 * 
 * gamepad_index: Gamepad being recorded
 * current_frame_out: Output for current frame count (may be NULL)
 * elapsed_time_out: Output for elapsed time in ms (may be NULL)
 * 
 * Returns: 0 on success, -1 if not recording
 */
int SDL2_GetMacroRecordingStats(int gamepad_index, int *current_frame_out, unsigned int *elapsed_time_out);

/**
 * Add input event to current recording manually
 * Used for programmatic macro creation
 * 
 * gamepad_index: Gamepad being recorded
 * event: Event to add to recording
 * 
 * Returns: 0 on success, -1 if not recording or buffer full
 */
int SDL2_AddMacroEvent(int gamepad_index, const GamepadMacroEvent *event);

/* ============================================================================
 * MACRO PLAYBACK
 * ============================================================================ */

/**
 * Start playback of saved macro
 * 
 * macro_id: ID of macro to play
 * gamepad_index: Gamepad to send input to (or -1 to broadcast all)
 * loop: TRUE to loop playback, FALSE to play once
 * playback_speed: Speed multiplier (0.5 = half speed, 2.0 = double speed)
 * 
 * Returns: 0 on success, -1 if macro not found or already playing
 */
int SDL2_StartMacroPlayback(unsigned int macro_id, int gamepad_index, BOOL loop, float playback_speed);

/**
 * Stop playback of macro
 * 
 * macro_id: ID of macro to stop (-1 to stop all)
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_StopMacroPlayback(unsigned int macro_id);

/**
 * Check if macro is currently playing
 * 
 * macro_id: ID of macro to check
 * 
 * Returns: 1 if playing, 0 if not playing, -1 on error
 */
int SDL2_IsMacroPlaying(unsigned int macro_id);

/**
 * Pause ongoing macro playback
 * 
 * macro_id: ID of macro to pause
 * 
 * Returns: 0 on success, -1 if not playing
 */
int SDL2_PauseMacroPlayback(unsigned int macro_id);

/**
 * Resume paused macro playback
 * 
 * macro_id: ID of macro to resume
 * 
 * Returns: 0 on success, -1 if not paused
 */
int SDL2_ResumeMacroPlayback(unsigned int macro_id);

/**
 * Update macro playback system each frame
 * Processes pending macro events and advances playback
 * Call once per game loop
 * 
 * delta_time: Time since last frame in milliseconds
 * 
 * Returns: 0 on success
 */
int SDL2_UpdateGamepadMacroPlayback(unsigned int delta_time);

/**
 * Get current playback position/progress
 * 
 * macro_id: ID of macro to query
 * current_event_out: Output for current event index (may be NULL)
 * total_events_out: Output for total events (may be NULL)
 * elapsed_time_out: Output for elapsed playback time (may be NULL)
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_GetMacroPlaybackProgress(unsigned int macro_id, int *current_event_out,
                                   int *total_events_out, unsigned int *elapsed_time_out);

/**
 * Set playback speed for running macro
 * 
 * macro_id: ID of macro to adjust
 * speed: Speed multiplier (0.25 to 4.0)
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_SetMacroPlaybackSpeed(unsigned int macro_id, float speed);

/* ============================================================================
 * MACRO MANAGEMENT
 * ============================================================================ */

/**
 * Save macro to persistent storage
 * 
 * macro_id: ID of macro to save
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_SaveMacro(unsigned int macro_id);

/**
 * Load macro from persistent storage
 * 
 * macro_name: Name of macro to load
 * macro_id_out: Output pointer for macro ID (may be NULL)
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_LoadMacro(const char *macro_name, unsigned int *macro_id_out);

/**
 * Delete macro (from memory and storage)
 * 
 * macro_id: ID of macro to delete
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_DeleteMacro(unsigned int macro_id);

/**
 * Get macro information
 * 
 * macro_id: ID of macro to query
 * macro_out: Output buffer for macro data (must be non-NULL)
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_GetMacro(unsigned int macro_id, GamepadMacro *macro_out);

/**
 * List all available macros
 * 
 * macros_out: Output buffer for macro names
 * max_macros: Maximum macros to return
 * 
 * Returns: Number of macros returned, -1 on error
 */
int SDL2_ListAvailableMacros(GamepadMacro *macros_out, int max_macros);

/**
 * Find macro by name
 * 
 * macro_name: Name of macro to find
 * 
 * Returns: Macro ID if found, -1 if not found
 */
unsigned int SDL2_FindMacroByName(const char *macro_name);

/**
 * Duplicate macro with new name
 * 
 * source_macro_id: ID of macro to duplicate
 * new_name: Name for duplicated macro
 * new_macro_id_out: Output pointer for new macro ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_DuplicateMacro(unsigned int source_macro_id, const char *new_name, unsigned int *new_macro_id_out);

/**
 * Merge two macros (append second to first)
 * 
 * macro1_id: First macro ID
 * macro2_id: Second macro ID
 * merged_macro_id_out: Output pointer for merged macro ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_MergeMacros(unsigned int macro1_id, unsigned int macro2_id, unsigned int *merged_macro_id_out);

/**
 * Enable/disable macro playback (without stopping)
 * 
 * macro_id: ID of macro to enable/disable
 * enabled: TRUE to enable, FALSE to disable
 * 
 * Returns: 0 on success
 */
int SDL2_SetMacroEnabled(unsigned int macro_id, BOOL enabled);

/**
 * Get total count of recorded macros
 * 
 * Returns: Number of macros in system
 */
int SDL2_GetMacroCount(void);

/* ============================================================================
 * MACRO IMPORT/EXPORT
 * ============================================================================ */

/**
 * Export macro to file (text format)
 * 
 * macro_id: ID of macro to export
 * filepath: File path to save macro to
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ExportMacro(unsigned int macro_id, const char *filepath);

/**
 * Import macro from file
 * 
 * filepath: File path to load macro from
 * macro_id_out: Output pointer for loaded macro ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ImportMacro(const char *filepath, unsigned int *macro_id_out);

/**
 * Export all macros to file
 * 
 * filepath: File path to save all macros to
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ExportAllMacros(const char *filepath);

/**
 * Import all macros from file
 * 
 * filepath: File path to load macros from
 * imported_count_out: Output pointer for count imported (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ImportAllMacros(const char *filepath, int *imported_count_out);

/* ============================================================================
 * MACRO EDITING
 * ============================================================================ */

/**
 * Clear all events from macro
 * 
 * macro_id: ID of macro to clear
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_ClearMacroEvents(unsigned int macro_id);

/**
 * Rename macro
 * 
 * macro_id: ID of macro to rename
 * new_name: New name for macro
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_RenameMacro(unsigned int macro_id, const char *new_name);

/**
 * Optimize macro (remove redundant events, compress timing)
 * 
 * macro_id: ID of macro to optimize
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_OptimizeMacro(unsigned int macro_id);

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump macro system state for debugging
 * 
 * Returns: 0 on success
 */
int SDL2_DumpMacroSystemState(void);

/**
 * Dump macro events
 * 
 * macro_id: ID of macro to dump
 * 
 * Returns: 0 on success, -1 if macro not found
 */
int SDL2_DumpMacroEvents(unsigned int macro_id);

/**
 * Validate macro integrity
 * Checks for corrupted events or timing issues
 * 
 * macro_id: ID of macro to validate
 * 
 * Returns: 1 if valid, 0 if corrupted, -1 if not found
 */
int SDL2_ValidateMacro(unsigned int macro_id);

#ifdef __cplusplus
}
#endif

#endif /* __WIN32_GAMEPAD_MACRO_SYSTEM_H__ */
