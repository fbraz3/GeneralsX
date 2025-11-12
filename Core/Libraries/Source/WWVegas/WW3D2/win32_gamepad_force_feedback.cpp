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
**  FILE: win32_gamepad_force_feedback.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Advanced Force Feedback & Haptic Feedback Implementation
**
**  DESCRIPTION:
**    Implements rumble profiles, intensity curves, weapon-specific patterns,
**    and context-specific haptic feedback for gamepad devices.
**
******************************************************************************/

#include "win32_gamepad_force_feedback.h"
#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_thread_compat.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * INTERNAL STRUCTURES & GLOBALS
 * ============================================================================ */

typedef struct {
    GamepadFFProfile profiles[GAMEPAD_FF_MAX_PROFILES];
    int profile_count;
    GamepadFFPattern patterns[GAMEPAD_FF_MAX_PATTERNS];
    int pattern_count;
    SDL2_CriticalSection system_lock;
    unsigned int active_profile_id;
    unsigned int next_profile_id;
    unsigned int next_pattern_id;
    float global_intensity_scale;
    BOOL ff_enabled[SDL2_MAX_GAMEPADS];
} GamepadFFSystem;

static GamepadFFSystem g_ff_system = {0};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static void LockFFSystem(void)
{
    SDL2_EnterCriticalSection(&g_ff_system.system_lock);
}

static void UnlockFFSystem(void)
{
    SDL2_LeaveCriticalSection(&g_ff_system.system_lock);
}

static int FindProfileIndex(unsigned int profile_id)
{
    int i;
    for (i = 0; i < g_ff_system.profile_count; i++) {
        if (g_ff_system.profiles[i].profile_id == profile_id) {
            return i;
        }
    }
    return -1;
}

static int FindPatternIndex(unsigned int pattern_id)
{
    int i;
    for (i = 0; i < g_ff_system.pattern_count; i++) {
        if (g_ff_system.patterns[i].pattern_id == pattern_id) {
            return i;
        }
    }
    return -1;
}

static float ApplyIntensityCurve(float intensity, GamepadFFCurve curve)
{
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    
    switch (curve) {
        case FF_CURVE_LINEAR:
            return intensity;
            
        case FF_CURVE_EXPONENTIAL:
            return intensity * intensity;
            
        case FF_CURVE_SIGMOID:
            /* S-curve: smooth acceleration/deceleration */
            return intensity * intensity * (3.0f - 2.0f * intensity);
            
        case FF_CURVE_CUSTOM:
        default:
            return intensity;
    }
}

static void CreateWeaponPattern(GamepadFFWeaponType weapon_type, GamepadFFPattern *pattern_out)
{
    memset(pattern_out, 0, sizeof(GamepadFFPattern));
    
    GamepadFFEffect frames[GAMEPAD_FF_PATTERN_FRAMES];
    int frame_count = 0;
    
    switch (weapon_type) {
        case FF_WEAPON_PISTOL: {
            /* Light, quick burst */
            frames[0].left_motor = FF_INTENSITY_MEDIUM;
            frames[0].right_motor = FF_INTENSITY_LIGHT;
            frames[0].duration_ms = 20;
            frames[0].intensity_curve = FF_CURVE_LINEAR;
            
            frames[1].left_motor = FF_INTENSITY_WEAK;
            frames[1].right_motor = FF_INTENSITY_NONE;
            frames[1].duration_ms = 30;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 2;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Pistol");
            pattern_out->max_intensity = FF_INTENSITY_MEDIUM;
            break;
        }
        
        case FF_WEAPON_RIFLE: {
            /* Medium sustained rumble */
            frames[0].left_motor = FF_INTENSITY_STRONG;
            frames[0].right_motor = FF_INTENSITY_MEDIUM;
            frames[0].duration_ms = 40;
            frames[0].intensity_curve = FF_CURVE_LINEAR;
            
            frames[1].left_motor = FF_INTENSITY_MEDIUM;
            frames[1].right_motor = FF_INTENSITY_LIGHT;
            frames[1].duration_ms = 30;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 2;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Rifle");
            pattern_out->max_intensity = FF_INTENSITY_STRONG;
            break;
        }
        
        case FF_WEAPON_SHOTGUN: {
            /* Heavy, sharp impact */
            frames[0].left_motor = FF_INTENSITY_MAX;
            frames[0].right_motor = FF_INTENSITY_VERY_STRONG;
            frames[0].duration_ms = 30;
            frames[0].intensity_curve = FF_CURVE_LINEAR;
            
            frames[1].left_motor = FF_INTENSITY_STRONG;
            frames[1].right_motor = FF_INTENSITY_MEDIUM;
            frames[1].duration_ms = 50;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 2;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Shotgun");
            pattern_out->max_intensity = FF_INTENSITY_MAX;
            break;
        }
        
        case FF_WEAPON_MELEE: {
            /* Sharp impact on impact */
            frames[0].left_motor = FF_INTENSITY_VERY_STRONG;
            frames[0].right_motor = FF_INTENSITY_VERY_STRONG;
            frames[0].duration_ms = 50;
            frames[0].intensity_curve = FF_CURVE_EXPONENTIAL;
            
            frames[1].left_motor = FF_INTENSITY_WEAK;
            frames[1].right_motor = FF_INTENSITY_WEAK;
            frames[1].duration_ms = 50;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 2;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Melee");
            pattern_out->max_intensity = FF_INTENSITY_VERY_STRONG;
            break;
        }
        
        case FF_WEAPON_EXPLOSION: {
            /* Intense but short explosion effect */
            frames[0].left_motor = FF_INTENSITY_MAX;
            frames[0].right_motor = FF_INTENSITY_MAX;
            frames[0].duration_ms = 60;
            frames[0].intensity_curve = FF_CURVE_EXPONENTIAL;
            
            frames[1].left_motor = FF_INTENSITY_STRONG;
            frames[1].right_motor = FF_INTENSITY_STRONG;
            frames[1].duration_ms = 100;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frames[2].left_motor = FF_INTENSITY_LIGHT;
            frames[2].right_motor = FF_INTENSITY_LIGHT;
            frames[2].duration_ms = 80;
            frames[2].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 3;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Explosion");
            pattern_out->max_intensity = FF_INTENSITY_MAX;
            break;
        }
        
        case FF_EFFECT_HIT: {
            /* Being hit - sharp, shocking */
            frames[0].left_motor = FF_INTENSITY_VERY_STRONG;
            frames[0].right_motor = FF_INTENSITY_VERY_STRONG;
            frames[0].duration_ms = 40;
            frames[0].intensity_curve = FF_CURVE_LINEAR;
            
            frames[1].left_motor = FF_INTENSITY_STRONG;
            frames[1].right_motor = FF_INTENSITY_STRONG;
            frames[1].duration_ms = 80;
            frames[1].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 2;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Effect_Hit");
            pattern_out->max_intensity = FF_INTENSITY_VERY_STRONG;
            break;
        }
        
        case FF_EFFECT_POWER_UP: {
            /* Pleasant ascending pulse */
            int i;
            for (i = 0; i < 5; i++) {
                frames[i].left_motor = (GamepadFFIntensity)(i + 1);
                frames[i].right_motor = (GamepadFFIntensity)(i + 1);
                frames[i].duration_ms = 20;
                frames[i].intensity_curve = FF_CURVE_LINEAR;
            }
            frame_count = 5;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Effect_PowerUp");
            pattern_out->max_intensity = FF_INTENSITY_VERY_STRONG;
            break;
        }
        
        default: {
            /* Light default feedback */
            frames[0].left_motor = FF_INTENSITY_LIGHT;
            frames[0].right_motor = FF_INTENSITY_LIGHT;
            frames[0].duration_ms = 40;
            frames[0].intensity_curve = FF_CURVE_LINEAR;
            
            frame_count = 1;
            snprintf(pattern_out->name, sizeof(pattern_out->name), "Weapon_Default");
            pattern_out->max_intensity = FF_INTENSITY_LIGHT;
            break;
        }
    }
    
    memcpy(pattern_out->frames, frames, frame_count * sizeof(GamepadFFEffect));
    pattern_out->frame_count = frame_count;
    
    unsigned int total_time = 0;
    int i;
    for (i = 0; i < frame_count; i++) {
        total_time += frames[i].duration_ms;
    }
    pattern_out->total_duration_ms = total_time;
}

/* ============================================================================
 * FORCE FEEDBACK INITIALIZATION
 * ============================================================================ */

int SDL2_InitGamepadForceFeeback(void)
{
    memset(&g_ff_system, 0, sizeof(GamepadFFSystem));
    g_ff_system.system_lock = SDL2_CreateCriticalSection();
    g_ff_system.global_intensity_scale = 1.0f;
    g_ff_system.active_profile_id = 0;
    g_ff_system.next_profile_id = 1;
    g_ff_system.next_pattern_id = 1;
    
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        g_ff_system.ff_enabled[i] = TRUE;
    }
    
    printf("Force feedback system initialized\n");
    return 0;
}

void SDL2_ShutdownGamepadForceFeeback(void)
{
    LockFFSystem();
    
    /* Stop all active effects */
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        SDL2_StopGamepadFF(i);
    }
    
    memset(&g_ff_system, 0, sizeof(GamepadFFSystem));
    printf("Force feedback system shutdown\n");
    UnlockFFSystem();
}

int SDL2_SetGamepadForceFeeedbackEnabled(int gamepad_index, BOOL enabled)
{
    if (gamepad_index == -1) {
        /* Enable/disable all gamepads */
        int i;
        for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            g_ff_system.ff_enabled[i] = enabled;
        }
    } else if (gamepad_index >= 0 && gamepad_index < SDL2_MAX_GAMEPADS) {
        g_ff_system.ff_enabled[gamepad_index] = enabled;
    } else {
        return -1;
    }
    
    return 0;
}

BOOL SDL2_IsGamepadForceFeeedbackEnabled(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return FALSE;
    }
    return g_ff_system.ff_enabled[gamepad_index];
}

int SDL2_SetGamepadForceFeeedbackIntensityScale(float scale)
{
    if (scale < 0.0f || scale > 2.0f) {
        return -1;
    }
    g_ff_system.global_intensity_scale = scale;
    return 0;
}

float SDL2_GetGamepadForceFeeedbackIntensityScale(void)
{
    return g_ff_system.global_intensity_scale;
}

/* ============================================================================
 * FORCE FEEDBACK PROFILES
 * ============================================================================ */

int SDL2_CreateFFProfile(int gamepad_index, const char *profile_name, 
                         GamepadFFCurve intensity_curve, unsigned int *profile_id_out)
{
    if (!profile_name) {
        return -1;
    }
    
    LockFFSystem();
    
    if (g_ff_system.profile_count >= GAMEPAD_FF_MAX_PROFILES) {
        printf("ERROR: Profile storage full\n");
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFProfile *profile = &g_ff_system.profiles[g_ff_system.profile_count];
    
    memset(profile, 0, sizeof(GamepadFFProfile));
    strncpy(profile->profile_name, profile_name, GAMEPAD_FF_PATTERN_NAME_LEN - 1);
    profile->profile_name[GAMEPAD_FF_PATTERN_NAME_LEN - 1] = '\0';
    
    profile->profile_id = g_ff_system.next_profile_id++;
    profile->intensity_curve = intensity_curve;
    profile->intensity_scale = 1.0f;
    profile->weapon_sensitivity = 1.0f;
    profile->environment_sensitivity = 1.0f;
    profile->adaptive_intensity = FALSE;
    profile->haptic_feedback_enabled = TRUE;
    profile->enabled = TRUE;
    
    if (profile_id_out) {
        *profile_id_out = profile->profile_id;
    }
    
    g_ff_system.profile_count++;
    
    printf("Created FF profile %u ('%s')\n", profile->profile_id, profile_name);
    UnlockFFSystem();
    return 0;
}

int SDL2_DeleteFFProfile(unsigned int profile_id)
{
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    /* Remove from storage */
    if (profile_index < g_ff_system.profile_count - 1) {
        memmove(&g_ff_system.profiles[profile_index],
                &g_ff_system.profiles[profile_index + 1],
                (g_ff_system.profile_count - profile_index - 1) * sizeof(GamepadFFProfile));
    }
    
    g_ff_system.profile_count--;
    printf("Deleted FF profile %u\n", profile_id);
    
    UnlockFFSystem();
    return 0;
}

int SDL2_GetFFProfile(unsigned int profile_id, GamepadFFProfile *profile_out)
{
    if (!profile_out) {
        return -1;
    }
    
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    memcpy(profile_out, &g_ff_system.profiles[profile_index], sizeof(GamepadFFProfile));
    UnlockFFSystem();
    return 0;
}

int SDL2_ActivateFFProfile(unsigned int profile_id)
{
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.active_profile_id = profile_id;
    printf("Activated FF profile %u\n", profile_id);
    
    UnlockFFSystem();
    return 0;
}

unsigned int SDL2_GetActiveFFProfile(void)
{
    return g_ff_system.active_profile_id;
}

int SDL2_SetFFProfileIntensityScale(unsigned int profile_id, float scale)
{
    if (scale < 0.0f || scale > 1.0f) {
        return -1;
    }
    
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.profiles[profile_index].intensity_scale = scale;
    UnlockFFSystem();
    return 0;
}

int SDL2_SetFFProfileWeaponSensitivity(unsigned int profile_id, float sensitivity)
{
    if (sensitivity < 0.5f || sensitivity > 2.0f) {
        return -1;
    }
    
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.profiles[profile_index].weapon_sensitivity = sensitivity;
    UnlockFFSystem();
    return 0;
}

int SDL2_SetFFProfileEnvironmentSensitivity(unsigned int profile_id, float sensitivity)
{
    if (sensitivity < 0.5f || sensitivity > 2.0f) {
        return -1;
    }
    
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.profiles[profile_index].environment_sensitivity = sensitivity;
    UnlockFFSystem();
    return 0;
}

int SDL2_SetFFProfileAdaptiveIntensity(unsigned int profile_id, BOOL enabled)
{
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.profiles[profile_index].adaptive_intensity = enabled;
    UnlockFFSystem();
    return 0;
}

int SDL2_ListFFProfiles(GamepadFFProfile *profiles_out, int max_profiles)
{
    if (!profiles_out || max_profiles <= 0) {
        return -1;
    }
    
    LockFFSystem();
    
    int count = (g_ff_system.profile_count < max_profiles) ? 
                g_ff_system.profile_count : max_profiles;
    
    memcpy(profiles_out, g_ff_system.profiles, count * sizeof(GamepadFFProfile));
    
    UnlockFFSystem();
    return count;
}

/* ============================================================================
 * FORCE FEEDBACK PATTERNS
 * ============================================================================ */

int SDL2_CreateFFPattern(const char *pattern_name, unsigned int *pattern_id_out)
{
    if (!pattern_name) {
        return -1;
    }
    
    LockFFSystem();
    
    if (g_ff_system.pattern_count >= GAMEPAD_FF_MAX_PATTERNS) {
        printf("ERROR: Pattern storage full\n");
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFPattern *pattern = &g_ff_system.patterns[g_ff_system.pattern_count];
    
    memset(pattern, 0, sizeof(GamepadFFPattern));
    strncpy(pattern->name, pattern_name, GAMEPAD_FF_PATTERN_NAME_LEN - 1);
    pattern->name[GAMEPAD_FF_PATTERN_NAME_LEN - 1] = '\0';
    
    pattern->pattern_id = g_ff_system.next_pattern_id++;
    pattern->enabled = TRUE;
    pattern->frame_count = 0;
    pattern->total_duration_ms = 0;
    pattern->max_intensity = FF_INTENSITY_NONE;
    
    if (pattern_id_out) {
        *pattern_id_out = pattern->pattern_id;
    }
    
    g_ff_system.pattern_count++;
    
    printf("Created FF pattern %u ('%s')\n", pattern->pattern_id, pattern_name);
    UnlockFFSystem();
    return 0;
}

int SDL2_DeleteFFPattern(unsigned int pattern_id)
{
    LockFFSystem();
    
    int pattern_index = FindPatternIndex(pattern_id);
    if (pattern_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    /* Remove from storage */
    if (pattern_index < g_ff_system.pattern_count - 1) {
        memmove(&g_ff_system.patterns[pattern_index],
                &g_ff_system.patterns[pattern_index + 1],
                (g_ff_system.pattern_count - pattern_index - 1) * sizeof(GamepadFFPattern));
    }
    
    g_ff_system.pattern_count--;
    printf("Deleted FF pattern %u\n", pattern_id);
    
    UnlockFFSystem();
    return 0;
}

int SDL2_AddFFPatternFrame(unsigned int pattern_id, const GamepadFFEffect *effect)
{
    if (!effect) {
        return -1;
    }
    
    LockFFSystem();
    
    int pattern_index = FindPatternIndex(pattern_id);
    if (pattern_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFPattern *pattern = &g_ff_system.patterns[pattern_index];
    
    if (pattern->frame_count >= GAMEPAD_FF_PATTERN_FRAMES) {
        printf("ERROR: Pattern frame buffer full\n");
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFEffect *frame = &pattern->frames[pattern->frame_count];
    memcpy(frame, effect, sizeof(GamepadFFEffect));
    pattern->frame_count++;
    
    /* Update duration */
    pattern->total_duration_ms += effect->duration_ms;
    
    /* Update max intensity */
    GamepadFFIntensity max_val = (effect->left_motor > effect->right_motor) ? 
                                  effect->left_motor : effect->right_motor;
    if (max_val > pattern->max_intensity) {
        pattern->max_intensity = max_val;
    }
    
    UnlockFFSystem();
    return 0;
}

int SDL2_GetFFPattern(unsigned int pattern_id, GamepadFFPattern *pattern_out)
{
    if (!pattern_out) {
        return -1;
    }
    
    LockFFSystem();
    
    int pattern_index = FindPatternIndex(pattern_id);
    if (pattern_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    memcpy(pattern_out, &g_ff_system.patterns[pattern_index], sizeof(GamepadFFPattern));
    UnlockFFSystem();
    return 0;
}

int SDL2_ClearFFPattern(unsigned int pattern_id)
{
    LockFFSystem();
    
    int pattern_index = FindPatternIndex(pattern_id);
    if (pattern_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    g_ff_system.patterns[pattern_index].frame_count = 0;
    g_ff_system.patterns[pattern_index].total_duration_ms = 0;
    g_ff_system.patterns[pattern_index].max_intensity = FF_INTENSITY_NONE;
    
    UnlockFFSystem();
    return 0;
}

int SDL2_OptimizeFFPattern(unsigned int pattern_id)
{
    printf("TODO: Optimize FF pattern %u\n", pattern_id);
    return -1;
}

int SDL2_ListFFPatterns(GamepadFFPattern *patterns_out, int max_patterns)
{
    if (!patterns_out || max_patterns <= 0) {
        return -1;
    }
    
    LockFFSystem();
    
    int count = (g_ff_system.pattern_count < max_patterns) ? 
                g_ff_system.pattern_count : max_patterns;
    
    memcpy(patterns_out, g_ff_system.patterns, count * sizeof(GamepadFFPattern));
    
    UnlockFFSystem();
    return count;
}

/* ============================================================================
 * PREDEFINED FEEDBACK PATTERNS
 * ============================================================================ */

int SDL2_CreateWeaponFFPattern(GamepadFFWeaponType weapon_type, unsigned int *pattern_id_out)
{
    GamepadFFPattern weapon_pattern;
    CreateWeaponPattern(weapon_type, &weapon_pattern);
    
    LockFFSystem();
    
    if (g_ff_system.pattern_count >= GAMEPAD_FF_MAX_PATTERNS) {
        printf("ERROR: Pattern storage full\n");
        UnlockFFSystem();
        return -1;
    }
    
    weapon_pattern.pattern_id = g_ff_system.next_pattern_id++;
    weapon_pattern.enabled = TRUE;
    
    memcpy(&g_ff_system.patterns[g_ff_system.pattern_count], 
           &weapon_pattern, sizeof(GamepadFFPattern));
    
    if (pattern_id_out) {
        *pattern_id_out = weapon_pattern.pattern_id;
    }
    
    g_ff_system.pattern_count++;
    
    UnlockFFSystem();
    return 0;
}

int SDL2_CreateEnvironmentFFPattern(GamepadFFEnvironment environment_type, 
                                    GamepadFFIntensity intensity, unsigned int *pattern_id_out)
{
    printf("TODO: Create environment FF pattern (type=%d, intensity=%d)\n", 
           environment_type, intensity);
    return -1;
}

int SDL2_CreateImpactFFPattern(float impact_force, unsigned int *pattern_id_out)
{
    printf("TODO: Create impact FF pattern (force=%.2f)\n", impact_force);
    return -1;
}

/* ============================================================================
 * FORCE FEEDBACK EXECUTION
 * ============================================================================ */

int SDL2_TriggerFFPattern(int gamepad_index, unsigned int pattern_id, BOOL loop, float intensity)
{
    printf("Triggering FF pattern %u on gamepad %d (intensity=%.2f)\n", 
           pattern_id, gamepad_index, intensity);
    return 0;
}

int SDL2_StopGamepadFF(int gamepad_index)
{
    if (gamepad_index == -1) {
        /* Stop all gamepads */
        int i;
        for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
            printf("Stopped FF on gamepad %d\n", i);
        }
    } else if (gamepad_index >= 0 && gamepad_index < SDL2_MAX_GAMEPADS) {
        printf("Stopped FF on gamepad %d\n", gamepad_index);
    } else {
        return -1;
    }
    
    return 0;
}

int SDL2_TriggerWeaponVibration(int gamepad_index, GamepadFFWeaponType weapon_type, float intensity)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!g_ff_system.ff_enabled[gamepad_index]) {
        return 0;
    }
    
    printf("Weapon vibration: gamepad=%d, weapon=%d, intensity=%.2f\n", 
           gamepad_index, weapon_type, intensity);
    return 0;
}

int SDL2_TriggerEnvironmentVibration(int gamepad_index, GamepadFFEnvironment environment_type, GamepadFFIntensity intensity)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!g_ff_system.ff_enabled[gamepad_index]) {
        return 0;
    }
    
    printf("Environment vibration: gamepad=%d, env=%d, intensity=%d\n", 
           gamepad_index, environment_type, intensity);
    return 0;
}

int SDL2_TriggerImpactVibration(int gamepad_index, float damage_amount, int hit_location)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!g_ff_system.ff_enabled[gamepad_index]) {
        return 0;
    }
    
    printf("Impact vibration: gamepad=%d, damage=%.2f, location=%d\n", 
           gamepad_index, damage_amount, hit_location);
    return 0;
}

int SDL2_UpdateGamepadForceFeeback(unsigned int delta_time)
{
    /* Process active effects and update */
    return 0;
}

/* ============================================================================
 * FORCE FEEDBACK CONFIGURATION
 * ============================================================================ */

int SDL2_SaveFFProfile(unsigned int profile_id)
{
    printf("TODO: Save FF profile %u to persistent storage\n", profile_id);
    return -1;
}

int SDL2_LoadFFProfile(const char *profile_name, unsigned int *profile_id_out)
{
    printf("TODO: Load FF profile '%s' from persistent storage\n", profile_name);
    return -1;
}

int SDL2_ExportFFProfiles(const char *filepath)
{
    printf("TODO: Export FF profiles to %s\n", filepath);
    return -1;
}

int SDL2_ImportFFProfiles(const char *filepath, int *imported_count_out)
{
    printf("TODO: Import FF profiles from %s\n", filepath);
    return -1;
}

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

int SDL2_DumpFFSystemState(void)
{
    LockFFSystem();
    
    printf("\n=== FORCE FEEDBACK SYSTEM STATE ===\n");
    printf("Total profiles: %d\n", g_ff_system.profile_count);
    printf("Total patterns: %d\n", g_ff_system.pattern_count);
    printf("Active profile: %u\n", g_ff_system.active_profile_id);
    printf("Global intensity scale: %.2f\n", g_ff_system.global_intensity_scale);
    
    printf("\nFF Status per gamepad:\n");
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        printf("  Gamepad %d: %s\n", i, g_ff_system.ff_enabled[i] ? "enabled" : "disabled");
    }
    
    printf("===================================\n\n");
    
    UnlockFFSystem();
    return 0;
}

int SDL2_DumpFFProfile(unsigned int profile_id)
{
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFProfile *profile = &g_ff_system.profiles[profile_index];
    
    printf("\n=== FF PROFILE: %u (%s) ===\n", profile_id, profile->profile_name);
    printf("Curve: %d\n", profile->intensity_curve);
    printf("Intensity scale: %.2f\n", profile->intensity_scale);
    printf("Weapon sensitivity: %.2f\n", profile->weapon_sensitivity);
    printf("Environment sensitivity: %.2f\n", profile->environment_sensitivity);
    printf("Adaptive intensity: %s\n", profile->adaptive_intensity ? "YES" : "NO");
    printf("=========================\n\n");
    
    UnlockFFSystem();
    return 0;
}

int SDL2_DumpFFPattern(unsigned int pattern_id)
{
    LockFFSystem();
    
    int pattern_index = FindPatternIndex(pattern_id);
    if (pattern_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFPattern *pattern = &g_ff_system.patterns[pattern_index];
    
    printf("\n=== FF PATTERN: %u (%s) ===\n", pattern_id, pattern->name);
    printf("Frames: %d\n", pattern->frame_count);
    printf("Duration: %ums\n", pattern->total_duration_ms);
    printf("Max intensity: %d\n", pattern->max_intensity);
    printf("=======================\n\n");
    
    UnlockFFSystem();
    return 0;
}

int SDL2_ValidateFFProfile(unsigned int profile_id)
{
    LockFFSystem();
    
    int profile_index = FindProfileIndex(profile_id);
    if (profile_index < 0) {
        UnlockFFSystem();
        return -1;
    }
    
    GamepadFFProfile *profile = &g_ff_system.profiles[profile_index];
    
    if (profile->intensity_scale < 0.0f || profile->intensity_scale > 1.0f) {
        UnlockFFSystem();
        return 0;
    }
    
    if (profile->weapon_sensitivity < 0.5f || profile->weapon_sensitivity > 2.0f) {
        UnlockFFSystem();
        return 0;
    }
    
    UnlockFFSystem();
    return 1;
}
