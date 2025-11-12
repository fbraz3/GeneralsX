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
**  FILE: win32_gamepad_force_feedback.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Advanced Force Feedback & Haptic Feedback System
**
**  DESCRIPTION:
**    Provides advanced rumble profiles, intensity curves, and
**    context-specific haptic feedback patterns.
**
******************************************************************************/

#ifndef __WIN32_GAMEPAD_FORCE_FEEDBACK_H__
#define __WIN32_GAMEPAD_FORCE_FEEDBACK_H__

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FORCE FEEDBACK DEFINITIONS
 * ============================================================================ */

#define GAMEPAD_FF_MAX_PROFILES 16              /* Max force feedback profiles */
#define GAMEPAD_FF_MAX_PATTERNS 32              /* Max vibration patterns */
#define GAMEPAD_FF_PATTERN_NAME_LEN 64          /* Pattern name length */
#define GAMEPAD_FF_PATTERN_FRAMES 120           /* Frames per pattern (2 seconds at 60 FPS) */

typedef enum {
    FF_INTENSITY_NONE = 0,          /* No vibration */
    FF_INTENSITY_WEAK = 1,          /* Weak vibration (20%) */
    FF_INTENSITY_LIGHT = 2,         /* Light vibration (35%) */
    FF_INTENSITY_MEDIUM = 3,        /* Medium vibration (50%) */
    FF_INTENSITY_STRONG = 4,        /* Strong vibration (75%) */
    FF_INTENSITY_VERY_STRONG = 5,   /* Very strong vibration (100%) */
    FF_INTENSITY_MAX = 6            /* Maximum safe vibration */
} GamepadFFIntensity;

typedef enum {
    FF_CURVE_LINEAR = 0,            /* Linear intensity scaling */
    FF_CURVE_EXPONENTIAL = 1,       /* Exponential (accelerating) intensity */
    FF_CURVE_SIGMOID = 2,           /* S-curve (smooth acceleration) */
    FF_CURVE_CUSTOM = 3             /* Custom curve (user-defined) */
} GamepadFFCurve;

typedef enum {
    FF_WEAPON_UNARMED = 0,          /* Hand-to-hand (light impact) */
    FF_WEAPON_PISTOL = 1,           /* Light weapon fire */
    FF_WEAPON_RIFLE = 2,            /* Medium weapon fire */
    FF_WEAPON_SHOTGUN = 3,          /* Heavy weapon fire */
    FF_WEAPON_MELEE = 4,            /* Melee weapon impact */
    FF_WEAPON_EXPLOSION = 5,        /* Explosion nearby */
    FF_WEAPON_HEAVY = 6,            /* Heavy weapon (tank, aircraft) */
    FF_EFFECT_HIT = 7,              /* Being hit/damaged */
    FF_EFFECT_HEAL = 8,             /* Healing/support */
    FF_EFFECT_POWER_UP = 9          /* Power-up activated */
} GamepadFFWeaponType;

typedef enum {
    FF_ENVIRONMENT_NONE = 0,        /* No environmental feedback */
    FF_ENVIRONMENT_IMPACT = 1,      /* Ground impact/impact surface */
    FF_ENVIRONMENT_WATER = 2,       /* Water splash/water physics */
    FF_ENVIRONMENT_FIRE = 3,        /* Fire/burning */
    FF_ENVIRONMENT_ELECTRIC = 4,    /* Electrical effects */
    FF_ENVIRONMENT_WIND = 5,        /* Wind/air pressure */
    FF_ENVIRONMENT_FREEZE = 6,      /* Freezing/ice effects */
    FF_ENVIRONMENT_SAND = 7         /* Desert/sand environment */
} GamepadFFEnvironment;

typedef struct {
    GamepadFFIntensity left_motor;   /* Left motor intensity (0-100) */
    GamepadFFIntensity right_motor;  /* Right motor intensity (0-100) */
    unsigned int duration_ms;        /* Duration in milliseconds */
    GamepadFFCurve intensity_curve;  /* Intensity scaling curve */
} GamepadFFEffect;

typedef struct {
    char name[GAMEPAD_FF_PATTERN_NAME_LEN];
    GamepadFFEffect frames[GAMEPAD_FF_PATTERN_FRAMES];
    int frame_count;
    unsigned int total_duration_ms;
    GamepadFFIntensity max_intensity;
    BOOL enabled;
    unsigned int pattern_id;
} GamepadFFPattern;

typedef struct {
    char profile_name[GAMEPAD_FF_PATTERN_NAME_LEN];
    GamepadFFCurve intensity_curve;
    float intensity_scale;           /* Global intensity multiplier (0.0-1.0) */
    float weapon_sensitivity;        /* Weapon feedback sensitivity (0.5-2.0) */
    float environment_sensitivity;   /* Environmental feedback sensitivity (0.5-2.0) */
    BOOL adaptive_intensity;         /* Scale intensity based on gamepad health/status */
    BOOL haptic_feedback_enabled;    /* Enable haptic-specific patterns */
    unsigned int profile_id;
    BOOL enabled;
} GamepadFFProfile;

/* ============================================================================
 * FORCE FEEDBACK INITIALIZATION
 * ============================================================================ */

/**
 * Initialize force feedback system
 * Must be called before any FF operations
 * 
 * Returns: 0 on success, -1 on failure
 */
int SDL2_InitGamepadForceFeeback(void);

/**
 * Shutdown force feedback system
 * Called at game shutdown
 */
void SDL2_ShutdownGamepadForceFeeback(void);

/**
 * Enable/disable all force feedback
 * 
 * gamepad_index: Gamepad to enable/disable (-1 for all)
 * enabled: TRUE to enable, FALSE to disable
 * 
 * Returns: 0 on success
 */
int SDL2_SetGamepadForceFeeedbackEnabled(int gamepad_index, BOOL enabled);

/**
 * Check if force feedback is enabled for gamepad
 * 
 * gamepad_index: Gamepad to check
 * 
 * Returns: TRUE if enabled, FALSE otherwise
 */
BOOL SDL2_IsGamepadForceFeeedbackEnabled(int gamepad_index);

/**
 * Set global force feedback intensity scale
 * 
 * scale: Intensity scale (0.0 = silent, 1.0 = normal, 1.5 = enhanced)
 * 
 * Returns: 0 on success
 */
int SDL2_SetGamepadForceFeeedbackIntensityScale(float scale);

/**
 * Get current force feedback intensity scale
 * 
 * Returns: Current intensity scale
 */
float SDL2_GetGamepadForceFeeedbackIntensityScale(void);

/* ============================================================================
 * FORCE FEEDBACK PROFILES
 * ============================================================================ */

/**
 * Create new force feedback profile
 * 
 * gamepad_index: Gamepad for profile
 * profile_name: Name for the profile
 * intensity_curve: Intensity curve type (linear, exponential, sigmoid)
 * profile_id_out: Output pointer for profile ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_CreateFFProfile(int gamepad_index, const char *profile_name, 
                         GamepadFFCurve intensity_curve, unsigned int *profile_id_out);

/**
 * Delete force feedback profile
 * 
 * profile_id: ID of profile to delete
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_DeleteFFProfile(unsigned int profile_id);

/**
 * Get force feedback profile
 * 
 * profile_id: ID of profile to query
 * profile_out: Output buffer for profile (must be non-NULL)
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_GetFFProfile(unsigned int profile_id, GamepadFFProfile *profile_out);

/**
 * Activate force feedback profile
 * Sets profile as active for all subsequent effects
 * 
 * profile_id: ID of profile to activate
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_ActivateFFProfile(unsigned int profile_id);

/**
 * Get currently active profile ID
 * 
 * Returns: Active profile ID, or 0 if none
 */
unsigned int SDL2_GetActiveFFProfile(void);

/**
 * Set profile intensity scale
 * 
 * profile_id: ID of profile to modify
 * scale: Intensity scale (0.0 - 1.0)
 * 
 * Returns: 0 on success
 */
int SDL2_SetFFProfileIntensityScale(unsigned int profile_id, float scale);

/**
 * Set weapon sensitivity multiplier
 * 
 * profile_id: ID of profile to modify
 * sensitivity: Sensitivity multiplier (0.5 - 2.0)
 * 
 * Returns: 0 on success
 */
int SDL2_SetFFProfileWeaponSensitivity(unsigned int profile_id, float sensitivity);

/**
 * Set environmental feedback sensitivity multiplier
 * 
 * profile_id: ID of profile to modify
 * sensitivity: Sensitivity multiplier (0.5 - 2.0)
 * 
 * Returns: 0 on success
 */
int SDL2_SetFFProfileEnvironmentSensitivity(unsigned int profile_id, float sensitivity);

/**
 * Enable/disable adaptive intensity
 * Scales feedback based on player health/status
 * 
 * profile_id: ID of profile to modify
 * enabled: TRUE to enable adaptive intensity
 * 
 * Returns: 0 on success
 */
int SDL2_SetFFProfileAdaptiveIntensity(unsigned int profile_id, BOOL enabled);

/**
 * List all force feedback profiles
 * 
 * profiles_out: Output buffer for profiles
 * max_profiles: Maximum profiles to return
 * 
 * Returns: Number of profiles returned, -1 on error
 */
int SDL2_ListFFProfiles(GamepadFFProfile *profiles_out, int max_profiles);

/* ============================================================================
 * FORCE FEEDBACK PATTERNS
 * ============================================================================ */

/**
 * Create custom force feedback pattern
 * 
 * pattern_name: Name for the pattern
 * pattern_id_out: Output pointer for pattern ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_CreateFFPattern(const char *pattern_name, unsigned int *pattern_id_out);

/**
 * Delete force feedback pattern
 * 
 * pattern_id: ID of pattern to delete
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_DeleteFFPattern(unsigned int pattern_id);

/**
 * Add frame to force feedback pattern
 * 
 * pattern_id: ID of pattern to modify
 * effect: Effect to add
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_AddFFPatternFrame(unsigned int pattern_id, const GamepadFFEffect *effect);

/**
 * Get force feedback pattern
 * 
 * pattern_id: ID of pattern to query
 * pattern_out: Output buffer for pattern (must be non-NULL)
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_GetFFPattern(unsigned int pattern_id, GamepadFFPattern *pattern_out);

/**
 * Clear all frames from pattern
 * 
 * pattern_id: ID of pattern to clear
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_ClearFFPattern(unsigned int pattern_id);

/**
 * Optimize pattern (compress frames, adjust timing)
 * 
 * pattern_id: ID of pattern to optimize
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_OptimizeFFPattern(unsigned int pattern_id);

/**
 * List all force feedback patterns
 * 
 * patterns_out: Output buffer for patterns
 * max_patterns: Maximum patterns to return
 * 
 * Returns: Number of patterns returned, -1 on error
 */
int SDL2_ListFFPatterns(GamepadFFPattern *patterns_out, int max_patterns);

/* ============================================================================
 * PREDEFINED FEEDBACK PATTERNS
 * ============================================================================ */

/**
 * Create predefined weapon feedback pattern
 * 
 * weapon_type: Type of weapon
 * pattern_id_out: Output pointer for pattern ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_CreateWeaponFFPattern(GamepadFFWeaponType weapon_type, unsigned int *pattern_id_out);

/**
 * Create predefined environmental feedback pattern
 * 
 * environment_type: Type of environment
 * intensity: Pattern intensity (1-6)
 * pattern_id_out: Output pointer for pattern ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_CreateEnvironmentFFPattern(GamepadFFEnvironment environment_type, 
                                    GamepadFFIntensity intensity, unsigned int *pattern_id_out);

/**
 * Create impact feedback pattern
 * 
 * impact_force: Force of impact (0.0 - 1.0)
 * pattern_id_out: Output pointer for pattern ID (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_CreateImpactFFPattern(float impact_force, unsigned int *pattern_id_out);

/* ============================================================================
 * FORCE FEEDBACK EXECUTION
 * ============================================================================ */

/**
 * Trigger force feedback pattern
 * 
 * gamepad_index: Gamepad to trigger feedback on
 * pattern_id: ID of pattern to play
 * loop: TRUE to loop pattern, FALSE to play once
 * intensity: Intensity multiplier (0.0 - 1.0)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_TriggerFFPattern(int gamepad_index, unsigned int pattern_id, BOOL loop, float intensity);

/**
 * Stop force feedback on gamepad
 * 
 * gamepad_index: Gamepad to stop feedback on (-1 for all)
 * 
 * Returns: 0 on success
 */
int SDL2_StopGamepadFF(int gamepad_index);

/**
 * Trigger weapon-specific vibration
 * 
 * gamepad_index: Gamepad to trigger feedback on
 * weapon_type: Type of weapon fired
 * intensity: Base intensity (0.0 - 1.0)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_TriggerWeaponVibration(int gamepad_index, GamepadFFWeaponType weapon_type, float intensity);

/**
 * Trigger environmental vibration
 * 
 * gamepad_index: Gamepad to trigger feedback on
 * environment_type: Environmental effect type
 * intensity: Effect intensity (1-6)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_TriggerEnvironmentVibration(int gamepad_index, GamepadFFEnvironment environment_type, GamepadFFIntensity intensity);

/**
 * Trigger impact vibration
 * Feedback when player is hit or damaged
 * 
 * gamepad_index: Gamepad to trigger feedback on
 * damage_amount: Amount of damage (0.0 - 1.0)
 * hit_location: Location of hit (0-7 for different bodyparts)
 * 
 * Returns: 0 on success
 */
int SDL2_TriggerImpactVibration(int gamepad_index, float damage_amount, int hit_location);

/**
 * Update force feedback system each frame
 * Processes active effects and applies feedback
 * Call once per game loop
 * 
 * delta_time: Time since last frame in milliseconds
 * 
 * Returns: 0 on success
 */
int SDL2_UpdateGamepadForceFeeback(unsigned int delta_time);

/* ============================================================================
 * FORCE FEEDBACK CONFIGURATION
 * ============================================================================ */

/**
 * Save force feedback profile to persistent storage
 * 
 * profile_id: ID of profile to save
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_SaveFFProfile(unsigned int profile_id);

/**
 * Load force feedback profile from persistent storage
 * 
 * profile_name: Name of profile to load
 * profile_id_out: Output pointer for profile ID (may be NULL)
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_LoadFFProfile(const char *profile_name, unsigned int *profile_id_out);

/**
 * Export all profiles to file
 * 
 * filepath: File path to save profiles to
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ExportFFProfiles(const char *filepath);

/**
 * Import all profiles from file
 * 
 * filepath: File path to load profiles from
 * imported_count_out: Output pointer for count imported (may be NULL)
 * 
 * Returns: 0 on success, -1 on error
 */
int SDL2_ImportFFProfiles(const char *filepath, int *imported_count_out);

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

/**
 * Dump force feedback system state for debugging
 * 
 * Returns: 0 on success
 */
int SDL2_DumpFFSystemState(void);

/**
 * Dump force feedback profile details
 * 
 * profile_id: ID of profile to dump
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_DumpFFProfile(unsigned int profile_id);

/**
 * Dump force feedback pattern details
 * 
 * pattern_id: ID of pattern to dump
 * 
 * Returns: 0 on success, -1 if not found
 */
int SDL2_DumpFFPattern(unsigned int pattern_id);

/**
 * Validate force feedback profile
 * Checks for invalid settings or corrupt data
 * 
 * profile_id: ID of profile to validate
 * 
 * Returns: 1 if valid, 0 if invalid, -1 if not found
 */
int SDL2_ValidateFFProfile(unsigned int profile_id);

#ifdef __cplusplus
}
#endif

#endif /* __WIN32_GAMEPAD_FORCE_FEEDBACK_H__ */
