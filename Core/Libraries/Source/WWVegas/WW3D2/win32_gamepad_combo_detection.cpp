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
**  FILE: win32_gamepad_combo_detection.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Button Combo Detection & Sequence Recognition - Implementation
**
**  DESCRIPTION:
**    Implements input combo detection for fighting game-style button sequences.
**    Detects multi-button press patterns with timing constraints.
**
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_gamepad_combo_detection.h"
#include "win32_thread_compat.h"

/* ============================================================================
 * INTERNAL STATE MANAGEMENT
 * ============================================================================ */

typedef struct {
    GamepadComboButton sequence[GAMEPAD_COMBO_MAX_BUTTONS];
    int sequence_length;
    unsigned int sequence_timer;
    unsigned int last_combo_detected;
} GamepadComboInputState;

typedef struct {
    GamepadComboPattern patterns[GAMEPAD_COMBO_MAX_PATTERNS];
    int pattern_count;
    GamepadComboInputState gamepad_state[GAMEPAD_COMBO_MAX_GAMEPADS];
    BOOL enabled;
    SDL2_CriticalSection lock;
} GamepadComboGlobalState;

static GamepadComboGlobalState g_combo_state;
static BOOL g_combo_initialized = FALSE;

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================ */

int SDL2_InitGamepadComboDetection(void)
{
    if (g_combo_initialized) {
        return 0;
    }
    
    printf("Phase 08: Initializing gamepad combo detection system\n");
    
    memset(&g_combo_state, 0, sizeof(GamepadComboGlobalState));
    
    g_combo_state.lock = SDL2_CreateCriticalSection();
    g_combo_state.pattern_count = 0;
    g_combo_state.enabled = TRUE;
    
    for (int i = 0; i < GAMEPAD_COMBO_MAX_GAMEPADS; i++) {
        g_combo_state.gamepad_state[i].sequence_length = 0;
        g_combo_state.gamepad_state[i].sequence_timer = 0;
        g_combo_state.gamepad_state[i].last_combo_detected = 0;
    }
    
    printf("Phase 08: Gamepad combo detection system initialized\n");
    g_combo_initialized = TRUE;
    return 0;
}

void SDL2_ShutdownGamepadComboDetection(void)
{
    if (!g_combo_initialized) {
        return;
    }
    
    printf("Phase 08: Shutting down gamepad combo detection system\n");
    
    SDL2_EnterCriticalSection(&g_combo_state.lock);
    {
        g_combo_state.enabled = FALSE;
        g_combo_state.pattern_count = 0;
    }
    SDL2_LeaveCriticalSection(&g_combo_state.lock);
    
    SDL2_DestroyCriticalSection(&g_combo_state.lock);
    g_combo_initialized = FALSE;
}

int SDL2_SetGamepadComboDetectionEnabled(BOOL enabled)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    g_combo_state.enabled = enabled;
    printf("Phase 08: Combo detection %s\n", enabled ? "enabled" : "disabled");
    
    return 0;
}

BOOL SDL2_IsGamepadComboDetectionEnabled(void)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    return g_combo_state.enabled;
}

/* ============================================================================
 * COMBO PATTERN REGISTRATION
 * ============================================================================ */

int SDL2_RegisterComboPattern(const GamepadComboPattern *pattern, unsigned int *combo_id_out)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (pattern == NULL) {
        return -1;
    }
    
    if (g_combo_state.pattern_count >= GAMEPAD_COMBO_MAX_PATTERNS) {
        printf("Phase 08: ERROR - Maximum combo patterns reached (%d)\n", GAMEPAD_COMBO_MAX_PATTERNS);
        return -1;
    }
    
    int index = g_combo_state.pattern_count;
    memcpy(&g_combo_state.patterns[index], pattern, sizeof(GamepadComboPattern));
    
    unsigned int combo_id = (index + 1) * 1000 + g_combo_state.pattern_count;
    g_combo_state.patterns[index].combo_id = combo_id;
    
    g_combo_state.pattern_count++;
    
    if (combo_id_out != NULL) {
        *combo_id_out = combo_id;
    }
    
    printf("Phase 08: Registered combo pattern '%s' (ID: %u)\n", pattern->name, combo_id);
    
    return 0;
}

int SDL2_UnregisterComboPattern(unsigned int combo_id)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (g_combo_state.patterns[i].combo_id == combo_id) {
            /* Shift remaining patterns */
            for (int j = i; j < g_combo_state.pattern_count - 1; j++) {
                memcpy(&g_combo_state.patterns[j], &g_combo_state.patterns[j + 1],
                       sizeof(GamepadComboPattern));
            }
            g_combo_state.pattern_count--;
            printf("Phase 08: Unregistered combo pattern (ID: %u)\n", combo_id);
            return 0;
        }
    }
    
    return -1;  /* Not found */
}

int SDL2_GetComboPattern(unsigned int combo_id, GamepadComboPattern *pattern_out)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (pattern_out == NULL) {
        return -1;
    }
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (g_combo_state.patterns[i].combo_id == combo_id) {
            memcpy(pattern_out, &g_combo_state.patterns[i], sizeof(GamepadComboPattern));
            return 0;
        }
    }
    
    return -1;  /* Not found */
}

int SDL2_ListComboPatterns(GamepadComboPattern *patterns_out, int max_patterns)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (patterns_out == NULL) {
        return -1;
    }
    
    int count = (max_patterns < g_combo_state.pattern_count) ? max_patterns : g_combo_state.pattern_count;
    
    for (int i = 0; i < count; i++) {
        memcpy(&patterns_out[i], &g_combo_state.patterns[i], sizeof(GamepadComboPattern));
    }
    
    return count;
}

unsigned int SDL2_FindComboPatternByName(const char *pattern_name)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (pattern_name == NULL) {
        return (unsigned int)-1;
    }
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (strcmp(g_combo_state.patterns[i].name, pattern_name) == 0) {
            return g_combo_state.patterns[i].combo_id;
        }
    }
    
    return (unsigned int)-1;  /* Not found */
}

int SDL2_DisableAllComboPatterns(void)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        g_combo_state.patterns[i].enabled = FALSE;
    }
    
    printf("Phase 08: Disabled all combo patterns\n");
    return 0;
}

int SDL2_SetComboPatternEnabled(unsigned int combo_id, BOOL enabled)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (g_combo_state.patterns[i].combo_id == combo_id) {
            g_combo_state.patterns[i].enabled = enabled;
            printf("Phase 08: Combo pattern %u %s\n", combo_id, enabled ? "enabled" : "disabled");
            return 0;
        }
    }
    
    return -1;  /* Not found */
}

/* ============================================================================
 * COMBO INPUT PROCESSING
 * ============================================================================ */

int SDL2_ProcessGamepadComboInput(int gamepad_index, GamepadComboButton button, BOOL pressed)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (!g_combo_state.enabled || gamepad_index < 0 || gamepad_index >= GAMEPAD_COMBO_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!pressed) {
        return -1;  /* Ignore button releases */
    }
    
    GamepadComboInputState *state = &g_combo_state.gamepad_state[gamepad_index];
    
    /* Add button to sequence */
    if (state->sequence_length < GAMEPAD_COMBO_MAX_BUTTONS - 1) {
        state->sequence[state->sequence_length++] = button;
        state->sequence_timer = 0;
    }
    
    printf("Phase 08: Button input - gamepad %d, button %d, sequence length %d\n",
           gamepad_index, button, state->sequence_length);
    
    /* Check for pattern matches */
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (!g_combo_state.patterns[i].enabled) {
            continue;
        }
        
        GamepadComboPattern *pattern = &g_combo_state.patterns[i];
        
        /* Check if current sequence matches pattern start */
        BOOL matches = TRUE;
        for (int j = 0; j < state->sequence_length && pattern->buttons[j] != COMBO_BUTTON_NONE; j++) {
            if (pattern->buttons[j] != COMBO_BUTTON_ANY && pattern->buttons[j] != state->sequence[j]) {
                matches = FALSE;
                break;
            }
        }
        
        if (!matches) {
            continue;
        }
        
        /* Check if sequence is complete */
        if (pattern->buttons[state->sequence_length] == COMBO_BUTTON_NONE) {
            printf("Phase 08: COMBO DETECTED - '%s' (ID: %u)\n", pattern->name, pattern->combo_id);
            state->last_combo_detected = pattern->combo_id;
            state->sequence_length = 0;  /* Reset after match */
            return pattern->combo_id;
        }
    }
    
    return -1;  /* No combo match */
}

int SDL2_UpdateGamepadComboDetection(unsigned int delta_time)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    for (int i = 0; i < GAMEPAD_COMBO_MAX_GAMEPADS; i++) {
        GamepadComboInputState *state = &g_combo_state.gamepad_state[i];
        
        if (state->sequence_length > 0) {
            state->sequence_timer += delta_time;
            
            /* Check timeout - find max timeout from matching patterns */
            unsigned int max_timeout = GAMEPAD_COMBO_DEFAULT_TIMEOUT;
            
            for (int j = 0; j < g_combo_state.pattern_count; j++) {
                if (g_combo_state.patterns[j].enabled &&
                    g_combo_state.patterns[j].timeout_ms > max_timeout) {
                    max_timeout = g_combo_state.patterns[j].timeout_ms;
                }
            }
            
            if (state->sequence_timer > max_timeout) {
                printf("Phase 08: Combo sequence timeout for gamepad %d\n", i);
                state->sequence_length = 0;
                state->sequence_timer = 0;
            }
        }
    }
    
    return 0;
}

int SDL2_ResetGamepadComboState(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (gamepad_index == -1) {
        /* Reset all gamepads */
        for (int i = 0; i < GAMEPAD_COMBO_MAX_GAMEPADS; i++) {
            g_combo_state.gamepad_state[i].sequence_length = 0;
            g_combo_state.gamepad_state[i].sequence_timer = 0;
        }
        printf("Phase 08: Reset combo state for all gamepads\n");
    } else if (gamepad_index >= 0 && gamepad_index < GAMEPAD_COMBO_MAX_GAMEPADS) {
        g_combo_state.gamepad_state[gamepad_index].sequence_length = 0;
        g_combo_state.gamepad_state[gamepad_index].sequence_timer = 0;
        printf("Phase 08: Reset combo state for gamepad %d\n", gamepad_index);
    } else {
        return -1;
    }
    
    return 0;
}

/* ============================================================================
 * COMBO DETECTION QUERYING
 * ============================================================================ */

int SDL2_WasComboDetected(unsigned int combo_id)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    for (int i = 0; i < GAMEPAD_COMBO_MAX_GAMEPADS; i++) {
        if (g_combo_state.gamepad_state[i].last_combo_detected == combo_id) {
            return 1;
        }
    }
    
    return 0;
}

unsigned int SDL2_GetLastDetectedCombo(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= GAMEPAD_COMBO_MAX_GAMEPADS) {
        return (unsigned int)-1;
    }
    
    return g_combo_state.gamepad_state[gamepad_index].last_combo_detected;
}

int SDL2_GetComboDetectionStats(int gamepad_index, GamepadComboStats *stats_out)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= GAMEPAD_COMBO_MAX_GAMEPADS) {
        return -1;
    }
    
    if (stats_out == NULL) {
        return -1;
    }
    
    memset(stats_out, 0, sizeof(GamepadComboStats));
    stats_out->last_combo_id = g_combo_state.gamepad_state[gamepad_index].last_combo_detected;
    
    /* TODO: Accumulate stats over time */
    
    return 0;
}

int SDL2_GetGamepadComboSequence(int gamepad_index, GamepadComboButton *buttons_out, int max_buttons)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= GAMEPAD_COMBO_MAX_GAMEPADS) {
        return -1;
    }
    
    if (buttons_out == NULL || max_buttons <= 0) {
        return -1;
    }
    
    GamepadComboInputState *state = &g_combo_state.gamepad_state[gamepad_index];
    int count = (state->sequence_length < max_buttons) ? state->sequence_length : max_buttons;
    
    for (int i = 0; i < count; i++) {
        buttons_out[i] = state->sequence[i];
    }
    
    return count;
}

unsigned int SDL2_GetGamepadComboTimeRemaining(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    if (gamepad_index < 0 || gamepad_index >= GAMEPAD_COMBO_MAX_GAMEPADS) {
        return 0;
    }
    
    GamepadComboInputState *state = &g_combo_state.gamepad_state[gamepad_index];
    
    if (state->sequence_length == 0) {
        return 0;
    }
    
    unsigned int max_timeout = GAMEPAD_COMBO_DEFAULT_TIMEOUT;
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        if (g_combo_state.patterns[i].enabled &&
            g_combo_state.patterns[i].timeout_ms > max_timeout) {
            max_timeout = g_combo_state.patterns[i].timeout_ms;
        }
    }
    
    if (state->sequence_timer >= max_timeout) {
        return 0;
    }
    
    return max_timeout - state->sequence_timer;
}

/* ============================================================================
 * PREDEFINED COMBO TEMPLATES
 * ============================================================================ */

int SDL2_CreateHadokenComboPattern(GamepadComboPattern *pattern_out)
{
    if (pattern_out == NULL) {
        return -1;
    }
    
    memset(pattern_out, 0, sizeof(GamepadComboPattern));
    strncpy(pattern_out->name, "Hadoken", GAMEPAD_COMBO_PATTERN_NAME_SIZE - 1);
    
    pattern_out->buttons[0] = COMBO_BUTTON_Y;  /* Down */
    pattern_out->buttons[1] = COMBO_BUTTON_X;  /* Down-Right */
    pattern_out->buttons[2] = COMBO_BUTTON_B;  /* Right */
    pattern_out->buttons[3] = COMBO_BUTTON_A;  /* Punch */
    pattern_out->buttons[4] = COMBO_BUTTON_NONE;
    
    pattern_out->timeout_ms = 1500;
    pattern_out->enabled = TRUE;
    
    return 0;
}

int SDL2_CreateShoryukenComboPattern(GamepadComboPattern *pattern_out)
{
    if (pattern_out == NULL) {
        return -1;
    }
    
    memset(pattern_out, 0, sizeof(GamepadComboPattern));
    strncpy(pattern_out->name, "Shoryuken", GAMEPAD_COMBO_PATTERN_NAME_SIZE - 1);
    
    pattern_out->buttons[0] = COMBO_BUTTON_B;  /* Right */
    pattern_out->buttons[1] = COMBO_BUTTON_X;  /* Down */
    pattern_out->buttons[2] = COMBO_BUTTON_Y;  /* Down-Left */
    pattern_out->buttons[3] = COMBO_BUTTON_A;  /* Punch */
    pattern_out->buttons[4] = COMBO_BUTTON_NONE;
    
    pattern_out->timeout_ms = 1500;
    pattern_out->enabled = TRUE;
    
    return 0;
}

int SDL2_CreateKonamiCodePattern(GamepadComboPattern *pattern_out)
{
    if (pattern_out == NULL) {
        return -1;
    }
    
    memset(pattern_out, 0, sizeof(GamepadComboPattern));
    strncpy(pattern_out->name, "KonamiCode", GAMEPAD_COMBO_PATTERN_NAME_SIZE - 1);
    
    pattern_out->buttons[0] = COMBO_BUTTON_Y;   /* Up */
    pattern_out->buttons[1] = COMBO_BUTTON_Y;   /* Up */
    pattern_out->buttons[2] = COMBO_BUTTON_X;   /* Down */
    pattern_out->buttons[3] = COMBO_BUTTON_X;   /* Down */
    pattern_out->buttons[4] = COMBO_BUTTON_BACK; /* Left */
    pattern_out->buttons[5] = COMBO_BUTTON_B;   /* Right */
    pattern_out->buttons[6] = COMBO_BUTTON_BACK; /* Left */
    pattern_out->buttons[7] = COMBO_BUTTON_B;   /* Right */
    pattern_out->buttons[8] = COMBO_BUTTON_NONE;
    
    pattern_out->timeout_ms = 5000;
    pattern_out->enabled = TRUE;
    
    return 0;
}

int SDL2_CreateCustomComboPattern(const char *name, const GamepadComboButton *buttons,
                                  unsigned int timeout_ms, GamepadComboPattern *pattern_out)
{
    if (name == NULL || buttons == NULL || pattern_out == NULL) {
        return -1;
    }
    
    memset(pattern_out, 0, sizeof(GamepadComboPattern));
    strncpy(pattern_out->name, name, GAMEPAD_COMBO_PATTERN_NAME_SIZE - 1);
    
    int button_count = 0;
    for (int i = 0; i < GAMEPAD_COMBO_MAX_BUTTONS - 1 && buttons[i] != COMBO_BUTTON_NONE; i++) {
        pattern_out->buttons[i] = buttons[i];
        button_count++;
    }
    pattern_out->buttons[button_count] = COMBO_BUTTON_NONE;
    
    if (timeout_ms > 0 && timeout_ms <= GAMEPAD_COMBO_MAX_TIMEOUT) {
        pattern_out->timeout_ms = timeout_ms;
    } else {
        pattern_out->timeout_ms = GAMEPAD_COMBO_DEFAULT_TIMEOUT;
    }
    
    pattern_out->enabled = TRUE;
    
    return 0;
}

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

int SDL2_DumpComboDetectionState(int gamepad_index)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    printf("\nPhase 08: Combo Detection State Dump\n");
    printf("===================================\n");
    printf("System enabled: %s\n", g_combo_state.enabled ? "yes" : "no");
    printf("Registered patterns: %d\n", g_combo_state.pattern_count);
    
    if (gamepad_index == -1) {
        /* Dump all gamepads */
        for (int i = 0; i < GAMEPAD_COMBO_MAX_GAMEPADS; i++) {
            GamepadComboInputState *state = &g_combo_state.gamepad_state[i];
            printf("  Gamepad %d: sequence length %d, timer %ums, last combo %u\n",
                   i, state->sequence_length, state->sequence_timer,
                   state->last_combo_detected);
        }
    } else if (gamepad_index >= 0 && gamepad_index < GAMEPAD_COMBO_MAX_GAMEPADS) {
        GamepadComboInputState *state = &g_combo_state.gamepad_state[gamepad_index];
        printf("Gamepad %d:\n", gamepad_index);
        printf("  Sequence: ");
        for (int i = 0; i < state->sequence_length; i++) {
            printf("%d ", state->sequence[i]);
        }
        printf("(length %d, timer %ums)\n", state->sequence_length, state->sequence_timer);
        printf("  Last combo: %u\n", state->last_combo_detected);
    }
    
    printf("\n");
    return 0;
}

int SDL2_DumpRegisteredComboPatterns(void)
{
    SDL2_CriticalSectionLock lock(&g_combo_state.lock);
    
    printf("\nPhase 08: Registered Combo Patterns\n");
    printf("===================================\n");
    printf("Total patterns: %d\n", g_combo_state.pattern_count);
    
    for (int i = 0; i < g_combo_state.pattern_count; i++) {
        GamepadComboPattern *pattern = &g_combo_state.patterns[i];
        printf("  Pattern %d: '%s' (ID: %u, enabled: %s, timeout: %ums)\n",
               i, pattern->name, pattern->combo_id,
               pattern->enabled ? "yes" : "no", pattern->timeout_ms);
        printf("    Sequence: ");
        for (int j = 0; j < GAMEPAD_COMBO_MAX_BUTTONS && pattern->buttons[j] != COMBO_BUTTON_NONE; j++) {
            printf("%d ", pattern->buttons[j]);
        }
        printf("\n");
    }
    
    printf("\n");
    return 0;
}

int SDL2_GetComboButtonName(GamepadComboButton button, char *name_out, size_t name_size)
{
    if (name_out == NULL || name_size == 0) {
        return -1;
    }
    
    const char *names[] = {
        "NONE", "A", "B", "X", "Y", "LB", "RB", "BACK", "START", "ANY"
    };
    
    const char *name = "UNKNOWN";
    if (button >= 0 && button <= 8) {
        name = names[button];
    } else if (button == -1) {
        name = "ANY";
    }
    
    strncpy(name_out, name, name_size - 1);
    name_out[name_size - 1] = '\0';
    
    return 0;
}
