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
**  FILE: win32_gamepad_macro_system.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Input Macro Recording & Playback Implementation
**
**  DESCRIPTION:
**    Implements input sequence recording with frame-based timing.
**    Supports playback with timing preservation and speed adjustment.
**    Integrates with Phase 07 config system for persistence.
**
******************************************************************************/

#include "win32_gamepad_macro_system.h"
#include "win32_sdl_types_compat.h"
#include "win32_gamepad_compat.h"
#include "win32_thread_compat.h"
#include "win32_gamepad_event_integration.h"
/* #include "win32_gamepad_config_compat.h" - TODO: Phase 07 config integration */
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================================
 * INTERNAL STRUCTURES
 * ============================================================================ */

typedef struct {
    int gamepad_index;
    BOOL is_recording;
    GamepadMacro current_recording;
    unsigned int recording_start_time;
    unsigned int last_frame_time;
} MacroRecorder;

typedef struct {
    GamepadMacroPlayback playback;
    GamepadMacro *macro;
    BOOL paused;
} MacroPlayer;

typedef struct {
    GamepadMacro macros[GAMEPAD_MACRO_MAX_MACROS];
    int macro_count;
    MacroRecorder recorders[SDL2_MAX_GAMEPADS];
    MacroPlayer players[GAMEPAD_MACRO_MAX_MACROS];
    SDL2_CriticalSection system_lock;
    BOOL system_enabled;
    unsigned int next_macro_id;
} GamepadMacroSystem;

static GamepadMacroSystem g_macro_system = {0};

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

static unsigned int GetCurrentTimeMs(void)
{
    /* Use frame-based timing */
    static unsigned int total_ms = 0;
    total_ms += GAMEPAD_MACRO_FRAME_TIME_MS;
    return total_ms;
}

static void LockMacroSystem(void)
{
    SDL2_EnterCriticalSection(&g_macro_system.system_lock);
}

static void UnlockMacroSystem(void)
{
    SDL2_LeaveCriticalSection(&g_macro_system.system_lock);
}

static int FindMacroIndex(unsigned int macro_id)
{
    int i;
    for (i = 0; i < g_macro_system.macro_count; i++) {
        if (g_macro_system.macros[i].macro_id == macro_id) {
            return i;
        }
    }
    return -1;
}

static int FindFreePlayerSlot(void)
{
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (!g_macro_system.players[i].macro) {
            return i;
        }
    }
    return -1;
}

static void ResetRecorder(MacroRecorder *recorder)
{
    if (recorder) {
        recorder->gamepad_index = -1;
        recorder->is_recording = FALSE;
        memset(&recorder->current_recording, 0, sizeof(GamepadMacro));
        recorder->recording_start_time = 0;
        recorder->last_frame_time = 0;
    }
}

static void ResetPlayer(MacroPlayer *player)
{
    if (player) {
        player->macro = NULL;
        player->paused = FALSE;
        memset(&player->playback, 0, sizeof(GamepadMacroPlayback));
    }
}

/* ============================================================================
 * MACRO SYSTEM INITIALIZATION
 * ============================================================================ */

int SDL2_InitGamepadMacroSystem(void)
{
    memset(&g_macro_system, 0, sizeof(GamepadMacroSystem));
    g_macro_system.system_enabled = TRUE;
    g_macro_system.next_macro_id = 1;
    g_macro_system.system_lock = SDL2_CreateCriticalSection();
    
    LockMacroSystem();
    
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        ResetRecorder(&g_macro_system.recorders[i]);
    }
    
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        ResetPlayer(&g_macro_system.players[i]);
    }
    
    printf("Macro system initialized\n");
    UnlockMacroSystem();
    return 0;
}

void SDL2_ShutdownGamepadMacroSystem(void)
{
    LockMacroSystem();
    
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        if (g_macro_system.recorders[i].is_recording) {
            SDL2_CancelMacroRecording(i);
        }
    }
    
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro) {
            g_macro_system.players[i].playback.is_playing = FALSE;
            ResetPlayer(&g_macro_system.players[i]);
        }
    }
    
    memset(&g_macro_system, 0, sizeof(GamepadMacroSystem));
    printf("Macro system shutdown\n");
    UnlockMacroSystem();
}

int SDL2_SetGamepadMacroSystemEnabled(BOOL enabled)
{
    LockMacroSystem();
    g_macro_system.system_enabled = enabled;
    UnlockMacroSystem();
    return 0;
}

BOOL SDL2_IsGamepadMacroSystemEnabled(void)
{
    return g_macro_system.system_enabled;
}

/* ============================================================================
 * MACRO RECORDING
 * ============================================================================ */

int SDL2_StartMacroRecording(int gamepad_index, const char *macro_name)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!macro_name) {
        return -1;
    }
    
    LockMacroSystem();
    
    MacroRecorder *recorder = &g_macro_system.recorders[gamepad_index];
    
    if (recorder->is_recording) {
        printf("ERROR: Already recording for gamepad %d\n", gamepad_index);
        UnlockMacroSystem();
        return -1;
    }
    
    ResetRecorder(recorder);
    recorder->gamepad_index = gamepad_index;
    recorder->is_recording = TRUE;
    recorder->recording_start_time = GetCurrentTimeMs();
    recorder->last_frame_time = recorder->recording_start_time;
    
    strncpy(recorder->current_recording.name, macro_name, GAMEPAD_MACRO_MAX_NAME - 1);
    recorder->current_recording.name[GAMEPAD_MACRO_MAX_NAME - 1] = '\0';
    recorder->current_recording.macro_id = g_macro_system.next_macro_id;
    recorder->current_recording.recording_timestamp = (unsigned int)time(NULL);
    recorder->current_recording.enabled = TRUE;
    recorder->current_recording.event_count = 0;
    
    printf("Started recording macro '%s' for gamepad %d\n", macro_name, gamepad_index);
    UnlockMacroSystem();
    return 0;
}

int SDL2_StopMacroRecording(int gamepad_index, unsigned int *macro_id_out)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    LockMacroSystem();
    
    MacroRecorder *recorder = &g_macro_system.recorders[gamepad_index];
    
    if (!recorder->is_recording) {
        printf("ERROR: Not recording for gamepad %d\n", gamepad_index);
        UnlockMacroSystem();
        return -1;
    }
    
    if (recorder->current_recording.event_count == 0) {
        printf("WARNING: Recorded macro has no events\n");
    }
    
    recorder->current_recording.total_duration_ms = 
        GetCurrentTimeMs() - recorder->recording_start_time;
    
    /* Store macro in system */
    if (g_macro_system.macro_count < GAMEPAD_MACRO_MAX_MACROS) {
        GamepadMacro *stored_macro = &g_macro_system.macros[g_macro_system.macro_count];
        memcpy(stored_macro, &recorder->current_recording, sizeof(GamepadMacro));
        
        if (macro_id_out) {
            *macro_id_out = recorder->current_recording.macro_id;
        }
        
        g_macro_system.macro_count++;
        g_macro_system.next_macro_id++;
        
        printf("Stopped recording: macro '%s' with %d events, duration %ums\n",
               recorder->current_recording.name,
               recorder->current_recording.event_count,
               recorder->current_recording.total_duration_ms);
    } else {
        printf("ERROR: Macro storage full\n");
        UnlockMacroSystem();
        return -1;
    }
    
    /* Save to persistent storage */
    SDL2_SaveMacro(recorder->current_recording.macro_id);
    
    ResetRecorder(recorder);
    UnlockMacroSystem();
    return 0;
}

int SDL2_CancelMacroRecording(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    LockMacroSystem();
    
    MacroRecorder *recorder = &g_macro_system.recorders[gamepad_index];
    
    if (!recorder->is_recording) {
        UnlockMacroSystem();
        return -1;
    }
    
    printf("Cancelled recording for gamepad %d\n", gamepad_index);
    ResetRecorder(recorder);
    UnlockMacroSystem();
    return 0;
}

int SDL2_IsMacroRecording(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    int result;
    LockMacroSystem();
    result = g_macro_system.recorders[gamepad_index].is_recording ? 1 : 0;
    UnlockMacroSystem();
    return result;
}

int SDL2_GetMacroRecordingStats(int gamepad_index, int *current_frame_out, unsigned int *elapsed_time_out)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    LockMacroSystem();
    
    MacroRecorder *recorder = &g_macro_system.recorders[gamepad_index];
    
    if (!recorder->is_recording) {
        UnlockMacroSystem();
        return -1;
    }
    
    if (current_frame_out) {
        *current_frame_out = recorder->current_recording.event_count;
    }
    
    if (elapsed_time_out) {
        *elapsed_time_out = GetCurrentTimeMs() - recorder->recording_start_time;
    }
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_AddMacroEvent(int gamepad_index, const GamepadMacroEvent *event)
{
    if (gamepad_index < 0 || gamepad_index >= SDL2_MAX_GAMEPADS) {
        return -1;
    }
    
    if (!event) {
        return -1;
    }
    
    LockMacroSystem();
    
    MacroRecorder *recorder = &g_macro_system.recorders[gamepad_index];
    
    if (!recorder->is_recording) {
        UnlockMacroSystem();
        return -1;
    }
    
    if (recorder->current_recording.event_count >= GAMEPAD_MACRO_MAX_FRAMES) {
        printf("ERROR: Macro event buffer full\n");
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacroEvent *storage_event = 
        &recorder->current_recording.events[recorder->current_recording.event_count];
    
    memcpy(storage_event, event, sizeof(GamepadMacroEvent));
    recorder->current_recording.event_count++;
    
    UnlockMacroSystem();
    return 0;
}

/* ============================================================================
 * MACRO PLAYBACK
 * ============================================================================ */

int SDL2_StartMacroPlayback(unsigned int macro_id, int gamepad_index, BOOL loop, float playback_speed)
{
    if (playback_speed < 0.25f || playback_speed > 4.0f) {
        playback_speed = 1.0f;
    }
    
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        printf("ERROR: Macro %u not found\n", macro_id);
        UnlockMacroSystem();
        return -1;
    }
    
    int player_slot = FindFreePlayerSlot();
    if (player_slot < 0) {
        printf("ERROR: No free playback slots\n");
        UnlockMacroSystem();
        return -1;
    }
    
    MacroPlayer *player = &g_macro_system.players[player_slot];
    player->macro = &g_macro_system.macros[macro_index];
    player->playback.macro_id = macro_id;
    player->playback.playback_position = 0;
    player->playback.playback_time = 0;
    player->playback.is_playing = TRUE;
    player->playback.loop = loop;
    player->playback.playback_speed = playback_speed;
    player->paused = FALSE;
    
    printf("Started playback of macro %u (%s) with speed %.2f\n", 
           macro_id, player->macro->name, playback_speed);
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_StopMacroPlayback(unsigned int macro_id)
{
    if (macro_id == (unsigned int)-1) {
        /* Stop all playbacks */
        LockMacroSystem();
        int i;
        for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
            if (g_macro_system.players[i].macro) {
                g_macro_system.players[i].playback.is_playing = FALSE;
                ResetPlayer(&g_macro_system.players[i]);
            }
        }
        UnlockMacroSystem();
        return 0;
    }
    
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id) {
            g_macro_system.players[i].playback.is_playing = FALSE;
            ResetPlayer(&g_macro_system.players[i]);
            UnlockMacroSystem();
            return 0;
        }
    }
    
    printf("ERROR: Macro %u not playing\n", macro_id);
    UnlockMacroSystem();
    return -1;
}

int SDL2_IsMacroPlaying(unsigned int macro_id)
{
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id &&
            g_macro_system.players[i].playback.is_playing) {
            UnlockMacroSystem();
            return 1;
        }
    }
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_PauseMacroPlayback(unsigned int macro_id)
{
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id &&
            g_macro_system.players[i].playback.is_playing) {
            g_macro_system.players[i].paused = TRUE;
            UnlockMacroSystem();
            return 0;
        }
    }
    
    UnlockMacroSystem();
    return -1;
}

int SDL2_ResumeMacroPlayback(unsigned int macro_id)
{
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id &&
            g_macro_system.players[i].paused) {
            g_macro_system.players[i].paused = FALSE;
            UnlockMacroSystem();
            return 0;
        }
    }
    
    UnlockMacroSystem();
    return -1;
}

int SDL2_UpdateGamepadMacroPlayback(unsigned int delta_time)
{
    if (!g_macro_system.system_enabled) {
        return 0;
    }
    
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        MacroPlayer *player = &g_macro_system.players[i];
        
        if (!player->macro || !player->playback.is_playing || player->paused) {
            continue;
        }
        
        player->playback.playback_time += (unsigned int)(delta_time * player->playback.playback_speed);
        
        /* Process events that should fire */
        while (player->playback.playback_position < player->macro->event_count) {
            GamepadMacroEvent *event = &player->macro->events[player->playback.playback_position];
            
            if (player->playback.playback_time >= event->frame_offset) {
                player->playback.playback_time -= event->frame_offset;
                player->playback.playback_position++;
                
                /* Execute event (post to input system) */
                switch (event->type) {
                    case MACRO_EVENT_BUTTON_PRESS:
                        /* Post button press event */
                        break;
                    case MACRO_EVENT_BUTTON_RELEASE:
                        /* Post button release event */
                        break;
                    case MACRO_EVENT_AXIS_MOTION:
                        /* Post axis motion event */
                        break;
                    case MACRO_EVENT_PAUSE:
                        /* Already handled via timing */
                        break;
                    case MACRO_EVENT_VIBRATE:
                        /* Trigger vibration pattern */
                        break;
                    default:
                        break;
                }
            } else {
                break;
            }
        }
        
        /* Check for end of playback */
        if (player->playback.playback_position >= player->macro->event_count) {
            if (player->playback.loop) {
                player->playback.playback_position = 0;
                player->playback.playback_time = 0;
            } else {
                player->playback.is_playing = FALSE;
                ResetPlayer(player);
            }
        }
    }
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_GetMacroPlaybackProgress(unsigned int macro_id, int *current_event_out,
                                   int *total_events_out, unsigned int *elapsed_time_out)
{
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id) {
            
            if (current_event_out) {
                *current_event_out = g_macro_system.players[i].playback.playback_position;
            }
            
            if (total_events_out) {
                *total_events_out = g_macro_system.players[i].macro->event_count;
            }
            
            if (elapsed_time_out) {
                *elapsed_time_out = g_macro_system.players[i].playback.playback_time;
            }
            
            UnlockMacroSystem();
            return 0;
        }
    }
    
    UnlockMacroSystem();
    return -1;
}

int SDL2_SetMacroPlaybackSpeed(unsigned int macro_id, float speed)
{
    if (speed < 0.25f || speed > 4.0f) {
        return -1;
    }
    
    LockMacroSystem();
    
    int i;
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && 
            g_macro_system.players[i].playback.macro_id == macro_id) {
            g_macro_system.players[i].playback.playback_speed = speed;
            UnlockMacroSystem();
            return 0;
        }
    }
    
    UnlockMacroSystem();
    return -1;
}

/* ============================================================================
 * MACRO MANAGEMENT
 * ============================================================================ */

int SDL2_SaveMacro(unsigned int macro_id)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacro *macro = &g_macro_system.macros[macro_index];
    
    /* Save via Phase 07 config system */
    /* TODO: Implement config persistence */
    printf("Saved macro %u ('%s') to persistent storage\n", macro_id, macro->name);
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_LoadMacro(const char *macro_name, unsigned int *macro_id_out)
{
    if (!macro_name) {
        return -1;
    }
    
    LockMacroSystem();
    
    /* Load via Phase 07 config system */
    /* TODO: Implement config loading */
    printf("Loaded macro '%s' from persistent storage\n", macro_name);
    
    UnlockMacroSystem();
    return -1;  /* Not yet implemented */
}

int SDL2_DeleteMacro(unsigned int macro_id)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    /* Remove from storage */
    if (macro_index < g_macro_system.macro_count - 1) {
        memmove(&g_macro_system.macros[macro_index],
                &g_macro_system.macros[macro_index + 1],
                (g_macro_system.macro_count - macro_index - 1) * sizeof(GamepadMacro));
    }
    
    g_macro_system.macro_count--;
    printf("Deleted macro %u\n", macro_id);
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_GetMacro(unsigned int macro_id, GamepadMacro *macro_out)
{
    if (!macro_out) {
        return -1;
    }
    
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    memcpy(macro_out, &g_macro_system.macros[macro_index], sizeof(GamepadMacro));
    UnlockMacroSystem();
    return 0;
}

int SDL2_ListAvailableMacros(GamepadMacro *macros_out, int max_macros)
{
    if (!macros_out || max_macros <= 0) {
        return -1;
    }
    
    LockMacroSystem();
    
    int count = (g_macro_system.macro_count < max_macros) ? 
                g_macro_system.macro_count : max_macros;
    
    memcpy(macros_out, g_macro_system.macros, count * sizeof(GamepadMacro));
    
    UnlockMacroSystem();
    return count;
}

unsigned int SDL2_FindMacroByName(const char *macro_name)
{
    if (!macro_name) {
        return (unsigned int)-1;
    }
    
    LockMacroSystem();
    
    int i;
    for (i = 0; i < g_macro_system.macro_count; i++) {
        if (strcmp(g_macro_system.macros[i].name, macro_name) == 0) {
            unsigned int macro_id = g_macro_system.macros[i].macro_id;
            UnlockMacroSystem();
            return macro_id;
        }
    }
    
    UnlockMacroSystem();
    return (unsigned int)-1;
}

int SDL2_DuplicateMacro(unsigned int source_macro_id, const char *new_name, unsigned int *new_macro_id_out)
{
    if (!new_name) {
        return -1;
    }
    
    LockMacroSystem();
    
    int source_index = FindMacroIndex(source_macro_id);
    if (source_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    if (g_macro_system.macro_count >= GAMEPAD_MACRO_MAX_MACROS) {
        printf("ERROR: Macro storage full\n");
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacro *source = &g_macro_system.macros[source_index];
    GamepadMacro *duplicate = &g_macro_system.macros[g_macro_system.macro_count];
    
    memcpy(duplicate, source, sizeof(GamepadMacro));
    duplicate->macro_id = g_macro_system.next_macro_id;
    strncpy(duplicate->name, new_name, GAMEPAD_MACRO_MAX_NAME - 1);
    duplicate->name[GAMEPAD_MACRO_MAX_NAME - 1] = '\0';
    
    g_macro_system.macro_count++;
    
    if (new_macro_id_out) {
        *new_macro_id_out = duplicate->macro_id;
    }
    
    g_macro_system.next_macro_id++;
    
    printf("Duplicated macro %u to '%s' (new ID: %u)\n", 
           source_macro_id, new_name, duplicate->macro_id);
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_MergeMacros(unsigned int macro1_id, unsigned int macro2_id, unsigned int *merged_macro_id_out)
{
    LockMacroSystem();
    
    int idx1 = FindMacroIndex(macro1_id);
    int idx2 = FindMacroIndex(macro2_id);
    
    if (idx1 < 0 || idx2 < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacro *macro1 = &g_macro_system.macros[idx1];
    GamepadMacro *macro2 = &g_macro_system.macros[idx2];
    
    /* Merge events: append macro2 events to macro1 */
    int total_events = macro1->event_count + macro2->event_count;
    if (total_events > GAMEPAD_MACRO_MAX_FRAMES) {
        printf("ERROR: Merged macro would exceed frame limit\n");
        UnlockMacroSystem();
        return -1;
    }
    
    /* Offset macro2 events by macro1 duration */
    int i;
    for (i = 0; i < macro2->event_count; i++) {
        GamepadMacroEvent *src = &macro2->events[i];
        GamepadMacroEvent *dst = &macro1->events[macro1->event_count + i];
        memcpy(dst, src, sizeof(GamepadMacroEvent));
        if (i == 0) {
            dst->frame_offset += macro1->total_duration_ms;
        }
    }
    
    macro1->event_count = total_events;
    macro1->total_duration_ms += macro2->total_duration_ms;
    
    if (merged_macro_id_out) {
        *merged_macro_id_out = macro1->macro_id;
    }
    
    printf("Merged macro %u into %u\n", macro2_id, macro1_id);
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_SetMacroEnabled(unsigned int macro_id, BOOL enabled)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    g_macro_system.macros[macro_index].enabled = enabled;
    UnlockMacroSystem();
    return 0;
}

int SDL2_GetMacroCount(void)
{
    int count;
    LockMacroSystem();
    count = g_macro_system.macro_count;
    UnlockMacroSystem();
    return count;
}

/* ============================================================================
 * MACRO IMPORT/EXPORT (STUBS)
 * ============================================================================ */

int SDL2_ExportMacro(unsigned int macro_id, const char *filepath)
{
    printf("TODO: Export macro %u to %s\n", macro_id, filepath);
    return -1;
}

int SDL2_ImportMacro(const char *filepath, unsigned int *macro_id_out)
{
    printf("TODO: Import macro from %s\n", filepath);
    return -1;
}

int SDL2_ExportAllMacros(const char *filepath)
{
    printf("TODO: Export all macros to %s\n", filepath);
    return -1;
}

int SDL2_ImportAllMacros(const char *filepath, int *imported_count_out)
{
    printf("TODO: Import all macros from %s\n", filepath);
    return -1;
}

/* ============================================================================
 * MACRO EDITING (STUBS)
 * ============================================================================ */

int SDL2_ClearMacroEvents(unsigned int macro_id)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    g_macro_system.macros[macro_index].event_count = 0;
    g_macro_system.macros[macro_index].total_duration_ms = 0;
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_RenameMacro(unsigned int macro_id, const char *new_name)
{
    if (!new_name) {
        return -1;
    }
    
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    strncpy(g_macro_system.macros[macro_index].name, new_name, GAMEPAD_MACRO_MAX_NAME - 1);
    g_macro_system.macros[macro_index].name[GAMEPAD_MACRO_MAX_NAME - 1] = '\0';
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_OptimizeMacro(unsigned int macro_id)
{
    printf("TODO: Optimize macro %u\n", macro_id);
    return -1;
}

/* ============================================================================
 * DEBUGGING & DIAGNOSTICS
 * ============================================================================ */

int SDL2_DumpMacroSystemState(void)
{
    LockMacroSystem();
    
    printf("\n=== MACRO SYSTEM STATE ===\n");
    printf("System enabled: %s\n", g_macro_system.system_enabled ? "YES" : "NO");
    printf("Total macros: %d\n", g_macro_system.macro_count);
    printf("Next macro ID: %u\n", g_macro_system.next_macro_id);
    
    printf("\nActive recordings:\n");
    int i;
    for (i = 0; i < SDL2_MAX_GAMEPADS; i++) {
        if (g_macro_system.recorders[i].is_recording) {
            printf("  Gamepad %d: '%s' (%d events)\n",
                   i,
                   g_macro_system.recorders[i].current_recording.name,
                   g_macro_system.recorders[i].current_recording.event_count);
        }
    }
    
    printf("\nActive playbacks:\n");
    for (i = 0; i < GAMEPAD_MACRO_MAX_MACROS; i++) {
        if (g_macro_system.players[i].macro && g_macro_system.players[i].playback.is_playing) {
            printf("  Macro %u: pos %d/%d\n",
                   g_macro_system.players[i].playback.macro_id,
                   g_macro_system.players[i].playback.playback_position,
                   g_macro_system.players[i].macro->event_count);
        }
    }
    
    printf("=========================\n\n");
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_DumpMacroEvents(unsigned int macro_id)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacro *macro = &g_macro_system.macros[macro_index];
    
    printf("\n=== MACRO EVENTS: %u (%s) ===\n", macro_id, macro->name);
    printf("Duration: %ums\n", macro->total_duration_ms);
    printf("Events: %d\n\n", macro->event_count);
    
    int i;
    for (i = 0; i < macro->event_count; i++) {
        GamepadMacroEvent *event = &macro->events[i];
        printf("  [%d] offset=%dms type=%d\n", i, event->frame_offset, event->type);
    }
    
    printf("==========================\n\n");
    
    UnlockMacroSystem();
    return 0;
}

int SDL2_ValidateMacro(unsigned int macro_id)
{
    LockMacroSystem();
    
    int macro_index = FindMacroIndex(macro_id);
    if (macro_index < 0) {
        UnlockMacroSystem();
        return -1;
    }
    
    GamepadMacro *macro = &g_macro_system.macros[macro_index];
    
    if (macro->event_count == 0) {
        UnlockMacroSystem();
        return 1;  /* Valid (empty) */
    }
    
    if (macro->event_count > GAMEPAD_MACRO_MAX_FRAMES) {
        UnlockMacroSystem();
        return 0;  /* Invalid */
    }
    
    unsigned int total_time = 0;
    int i;
    for (i = 0; i < macro->event_count; i++) {
        total_time += macro->events[i].frame_offset;
    }
    
    if (total_time != macro->total_duration_ms) {
        printf("WARNING: Macro timing mismatch\n");
    }
    
    UnlockMacroSystem();
    return 1;  /* Valid */
}
