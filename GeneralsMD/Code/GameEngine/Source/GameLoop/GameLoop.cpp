/**
 * @file GameLoop.cpp
 * @brief Phase 29: Game Loop Integration - Main game loop implementation
 */

#include "GameLoop.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define ERROR_BUFFER_SIZE 256
#define FRAME_STATS_WINDOW 120  // Track last 120 frames for average

// Platform-specific timing
#include <sys/time.h>
typedef struct timespec TimerValue;

// Game loop state
typedef struct {
    int is_initialized;
    int is_running;
    int is_paused;
    int frame_limiter_enabled;
    
    uint32_t target_fps;
    float target_frame_time;
    
    uint64_t frame_number;
    uint64_t start_time_ms;
    uint64_t last_frame_time_ms;
    uint64_t current_frame_time_ms;
    
    float delta_time;
    float total_time;
    float current_fps;
    
    uint64_t frame_time_ms;
    uint64_t update_time_ms;
    uint64_t render_time_ms;
    
    // Statistics
    FrameStats stats;
    float fps_samples[FRAME_STATS_WINDOW];
    uint32_t fps_sample_count;
    
    // Callbacks
    GameLoopUpdateCallback update_callback;
    void* update_user_data;
    GameLoopRenderCallback render_callback;
    void* render_user_data;
    GameLoopFrameCallback frame_callback;
    void* frame_user_data;
    
    char error_message[ERROR_BUFFER_SIZE];
} GameLoopState;

static GameLoopState g_loop_state = {0};

// Get current time in milliseconds (platform-specific)
static uint64_t GetTimeMS(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

// Set error message
static void SetError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_loop_state.error_message, ERROR_BUFFER_SIZE, format, args);
    va_end(args);
}

// Update frame statistics
static void UpdateStats(float fps) {
    if (g_loop_state.fps_sample_count < FRAME_STATS_WINDOW) {
        g_loop_state.fps_samples[g_loop_state.fps_sample_count++] = fps;
    } else {
        // Ring buffer: shift samples
        for (int i = 0; i < FRAME_STATS_WINDOW - 1; i++) {
            g_loop_state.fps_samples[i] = g_loop_state.fps_samples[i + 1];
        }
        g_loop_state.fps_samples[FRAME_STATS_WINDOW - 1] = fps;
    }
    
    // Calculate average
    float sum = 0.0f;
    float min_fps = fps;
    float max_fps = fps;
    
    for (uint32_t i = 0; i < g_loop_state.fps_sample_count; i++) {
        sum += g_loop_state.fps_samples[i];
        if (g_loop_state.fps_samples[i] < min_fps) min_fps = g_loop_state.fps_samples[i];
        if (g_loop_state.fps_samples[i] > max_fps) max_fps = g_loop_state.fps_samples[i];
    }
    
    g_loop_state.stats.average_fps = sum / g_loop_state.fps_sample_count;
    g_loop_state.stats.min_fps = min_fps;
    g_loop_state.stats.max_fps = max_fps;
    g_loop_state.stats.total_frames++;
}

int GameLoop_Initialize(uint32_t target_fps) {
    if (g_loop_state.is_initialized) {
        SetError("Game loop already initialized");
        return GAMELOOP_ERROR_INVALID;
    }
    
    memset(&g_loop_state, 0, sizeof(GameLoopState));
    
    g_loop_state.is_initialized = 1;
    g_loop_state.is_running = 0;
    g_loop_state.is_paused = 0;
    g_loop_state.frame_limiter_enabled = 1;
    
    g_loop_state.target_fps = target_fps > 0 ? target_fps : 60;
    g_loop_state.target_frame_time = target_fps > 0 ? 1000.0f / target_fps : 0.0f;
    
    g_loop_state.frame_number = 0;
    g_loop_state.delta_time = 0.0f;
    g_loop_state.total_time = 0.0f;
    g_loop_state.current_fps = 0.0f;
    
    g_loop_state.stats.total_frames = 0;
    g_loop_state.stats.average_fps = 0.0f;
    g_loop_state.stats.min_fps = 0.0f;
    g_loop_state.stats.max_fps = 0.0f;
    g_loop_state.stats.frames_dropped = 0;
    g_loop_state.stats.total_update_time_ms = 0;
    g_loop_state.stats.total_render_time_ms = 0;
    g_loop_state.stats.total_frame_time_ms = 0;
    
    g_loop_state.fps_sample_count = 0;
    
    g_loop_state.start_time_ms = GetTimeMS();
    g_loop_state.last_frame_time_ms = g_loop_state.start_time_ms;
    
    return GAMELOOP_OK;
}

int GameLoop_Shutdown(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    if (g_loop_state.is_running) {
        GameLoop_Stop();
    }
    
    memset(&g_loop_state, 0, sizeof(GameLoopState));
    
    return GAMELOOP_OK;
}

int GameLoop_Start(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    if (g_loop_state.is_running) {
        SetError("Game loop already running");
        return GAMELOOP_ERROR_RUNNING;
    }
    
    g_loop_state.is_running = 1;
    g_loop_state.is_paused = 0;
    g_loop_state.last_frame_time_ms = GetTimeMS();
    
    return GAMELOOP_OK;
}

int GameLoop_Stop(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.is_running = 0;
    g_loop_state.is_paused = 0;
    
    return GAMELOOP_OK;
}

int GameLoop_IsRunning(void) {
    return g_loop_state.is_running;
}

int GameLoop_ExecuteFrame(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    if (!g_loop_state.is_running) {
        return GAMELOOP_OK;
    }
    
    // Get current time
    uint64_t frame_start_ms = GetTimeMS();
    uint64_t update_start_ms, update_end_ms;
    uint64_t render_start_ms, render_end_ms;
    
    // Calculate delta time
    uint64_t frame_delta_ms = frame_start_ms - g_loop_state.last_frame_time_ms;
    g_loop_state.delta_time = frame_delta_ms / 1000.0f;
    g_loop_state.total_time += g_loop_state.delta_time;
    
    // Skip update/render if paused
    if (!g_loop_state.is_paused) {
        // Call frame callback if registered
        if (g_loop_state.frame_callback) {
            FrameTiming timing;
            timing.frame_number = g_loop_state.frame_number;
            timing.delta_time = g_loop_state.delta_time;
            timing.total_time = g_loop_state.total_time;
            timing.fps = g_loop_state.current_fps;
            timing.frame_time_ms = g_loop_state.frame_time_ms;
            
            g_loop_state.frame_callback(&timing, g_loop_state.frame_user_data);
        }
        
        // Update phase
        if (g_loop_state.update_callback) {
            update_start_ms = GetTimeMS();
            g_loop_state.update_callback(g_loop_state.delta_time, g_loop_state.update_user_data);
            update_end_ms = GetTimeMS();
            g_loop_state.update_time_ms = update_end_ms - update_start_ms;
            g_loop_state.stats.total_update_time_ms += g_loop_state.update_time_ms;
        }
        
        // Render phase
        if (g_loop_state.render_callback) {
            render_start_ms = GetTimeMS();
            g_loop_state.render_callback(g_loop_state.render_user_data);
            render_end_ms = GetTimeMS();
            g_loop_state.render_time_ms = render_end_ms - render_start_ms;
            g_loop_state.stats.total_render_time_ms += g_loop_state.render_time_ms;
        }
    }
    
    // Frame rate limiting
    uint64_t frame_end_ms = GetTimeMS();
    uint64_t frame_total_ms = frame_end_ms - frame_start_ms;
    
    if (g_loop_state.frame_limiter_enabled && g_loop_state.target_frame_time > 0) {
        uint64_t target_frame_ms = (uint64_t)g_loop_state.target_frame_time;
        
        if (frame_total_ms < target_frame_ms) {
            uint64_t sleep_ms = target_frame_ms - frame_total_ms;
            
            struct timespec sleep_time;
            sleep_time.tv_sec = sleep_ms / 1000;
            sleep_time.tv_nsec = (sleep_ms % 1000) * 1000000;
            nanosleep(&sleep_time, NULL);
        } else if (frame_total_ms > target_frame_ms) {
            g_loop_state.stats.frames_dropped++;
        }
    }
    
    // Update timing information
    uint64_t final_frame_end_ms = GetTimeMS();
    g_loop_state.frame_time_ms = final_frame_end_ms - frame_start_ms;
    g_loop_state.stats.total_frame_time_ms += g_loop_state.frame_time_ms;
    
    // Calculate FPS
    if (g_loop_state.frame_time_ms > 0) {
        g_loop_state.current_fps = 1000.0f / g_loop_state.frame_time_ms;
    }
    
    UpdateStats(g_loop_state.current_fps);
    
    g_loop_state.last_frame_time_ms = frame_start_ms;
    g_loop_state.frame_number++;
    
    return GAMELOOP_OK;
}

int GameLoop_RegisterUpdateCallback(GameLoopUpdateCallback callback, void* user_data) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.update_callback = callback;
    g_loop_state.update_user_data = user_data;
    
    return GAMELOOP_OK;
}

int GameLoop_RegisterRenderCallback(GameLoopRenderCallback callback, void* user_data) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.render_callback = callback;
    g_loop_state.render_user_data = user_data;
    
    return GAMELOOP_OK;
}

int GameLoop_RegisterFrameCallback(GameLoopFrameCallback callback, void* user_data) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.frame_callback = callback;
    g_loop_state.frame_user_data = user_data;
    
    return GAMELOOP_OK;
}

int GameLoop_GetFrameTiming(FrameTiming* timing) {
    if (!g_loop_state.is_initialized || !timing) {
        SetError("Invalid parameters");
        return GAMELOOP_ERROR_INVALID;
    }
    
    timing->frame_number = g_loop_state.frame_number;
    timing->delta_time = g_loop_state.delta_time;
    timing->total_time = g_loop_state.total_time;
    timing->fps = g_loop_state.current_fps;
    timing->frame_time_ms = g_loop_state.frame_time_ms;
    
    return GAMELOOP_OK;
}

int GameLoop_GetFrameStats(FrameStats* stats) {
    if (!g_loop_state.is_initialized || !stats) {
        SetError("Invalid parameters");
        return GAMELOOP_ERROR_INVALID;
    }
    
    memcpy(stats, &g_loop_state.stats, sizeof(FrameStats));
    
    return GAMELOOP_OK;
}

int GameLoop_SetTargetFPS(uint32_t target_fps) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.target_fps = target_fps > 0 ? target_fps : 60;
    g_loop_state.target_frame_time = target_fps > 0 ? 1000.0f / target_fps : 0.0f;
    
    return GAMELOOP_OK;
}

uint32_t GameLoop_GetTargetFPS(void) {
    return g_loop_state.target_fps;
}

float GameLoop_GetCurrentFPS(void) {
    return g_loop_state.current_fps;
}

float GameLoop_GetDeltaTime(void) {
    return g_loop_state.delta_time;
}

float GameLoop_GetTotalTime(void) {
    return g_loop_state.total_time;
}

uint64_t GameLoop_GetFrameNumber(void) {
    return g_loop_state.frame_number;
}

int GameLoop_ResetStats(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.stats.total_frames = 0;
    g_loop_state.stats.average_fps = 0.0f;
    g_loop_state.stats.min_fps = 0.0f;
    g_loop_state.stats.max_fps = 0.0f;
    g_loop_state.stats.total_update_time_ms = 0;
    g_loop_state.stats.total_render_time_ms = 0;
    g_loop_state.stats.total_frame_time_ms = 0;
    g_loop_state.stats.frames_dropped = 0;
    g_loop_state.fps_sample_count = 0;
    
    return GAMELOOP_OK;
}

int GameLoop_Pause(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.is_paused = 1;
    
    return GAMELOOP_OK;
}

int GameLoop_Resume(void) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.is_paused = 0;
    g_loop_state.last_frame_time_ms = GetTimeMS();
    
    return GAMELOOP_OK;
}

int GameLoop_IsPaused(void) {
    return g_loop_state.is_paused;
}

int GameLoop_SetFrameLimiterEnabled(int enabled) {
    if (!g_loop_state.is_initialized) {
        SetError("Game loop not initialized");
        return GAMELOOP_ERROR_NOT_INIT;
    }
    
    g_loop_state.frame_limiter_enabled = enabled ? 1 : 0;
    
    return GAMELOOP_OK;
}

int GameLoop_IsFrameLimiterEnabled(void) {
    return g_loop_state.frame_limiter_enabled;
}

const char* GameLoop_GetError(void) {
    return g_loop_state.error_message;
}
