/**
 * @file GameLoop.h
 * @brief Phase 29: Game Loop Integration - Main game loop with timing and render phases
 *
 * Main game loop managing frame timing, delta time, update/render phases, and frame rate limiting.
 */

#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define GAMELOOP_OK               0
#define GAMELOOP_ERROR_INVALID    -1
#define GAMELOOP_ERROR_NOT_INIT   -2
#define GAMELOOP_ERROR_RUNNING    -3

// Frame timing data
typedef struct {
    uint64_t frame_number;
    float delta_time;      // Frame delta in seconds
    float total_time;      // Total elapsed time in seconds
    float fps;             // Current FPS
    uint64_t frame_time_ms; // Frame duration in milliseconds
} FrameTiming;

// Frame statistics
typedef struct {
    uint64_t total_frames;
    float average_fps;
    float min_fps;
    float max_fps;
    uint64_t total_update_time_ms;
    uint64_t total_render_time_ms;
    uint64_t total_frame_time_ms;
    uint64_t frames_dropped;
} FrameStats;

// Game loop callbacks
typedef int (*GameLoopUpdateCallback)(float delta_time, void* user_data);
typedef int (*GameLoopRenderCallback)(void* user_data);
typedef int (*GameLoopFrameCallback)(const FrameTiming* timing, void* user_data);

/**
 * Initialize game loop
 * @param target_fps Target frames per second (0 = unlimited)
 * @return GAMELOOP_OK on success
 */
int GameLoop_Initialize(uint32_t target_fps);

/**
 * Shutdown game loop
 * @return GAMELOOP_OK on success
 */
int GameLoop_Shutdown(void);

/**
 * Start game loop
 * @return GAMELOOP_OK on success
 */
int GameLoop_Start(void);

/**
 * Stop game loop
 * @return GAMELOOP_OK on success
 */
int GameLoop_Stop(void);

/**
 * Check if game loop is running
 * @return 1 if running, 0 if not
 */
int GameLoop_IsRunning(void);

/**
 * Execute one frame of the game loop
 * Performs: Update -> Render -> Present
 * @return GAMELOOP_OK on success
 */
int GameLoop_ExecuteFrame(void);

/**
 * Register update phase callback
 * Called once per frame for game logic
 * @param callback Update callback function
 * @param user_data User data to pass to callback
 * @return GAMELOOP_OK on success
 */
int GameLoop_RegisterUpdateCallback(GameLoopUpdateCallback callback, void* user_data);

/**
 * Register render phase callback
 * Called once per frame for rendering
 * @param callback Render callback function
 * @param user_data User data to pass to callback
 * @return GAMELOOP_OK on success
 */
int GameLoop_RegisterRenderCallback(GameLoopRenderCallback callback, void* user_data);

/**
 * Register frame callback
 * Called at start of each frame with timing data
 * @param callback Frame callback function
 * @param user_data User data to pass to callback
 * @return GAMELOOP_OK on success
 */
int GameLoop_RegisterFrameCallback(GameLoopFrameCallback callback, void* user_data);

/**
 * Get current frame timing information
 * @param timing Pointer to frame timing structure to fill
 * @return GAMELOOP_OK on success
 */
int GameLoop_GetFrameTiming(FrameTiming* timing);

/**
 * Get frame statistics
 * @param stats Pointer to frame statistics structure to fill
 * @return GAMELOOP_OK on success
 */
int GameLoop_GetFrameStats(FrameStats* stats);

/**
 * Set target FPS
 * @param target_fps Target frames per second (0 = unlimited)
 * @return GAMELOOP_OK on success
 */
int GameLoop_SetTargetFPS(uint32_t target_fps);

/**
 * Get target FPS
 * @return Target FPS value
 */
uint32_t GameLoop_GetTargetFPS(void);

/**
 * Get current FPS
 * @return Current frames per second
 */
float GameLoop_GetCurrentFPS(void);

/**
 * Get delta time for current frame
 * @return Delta time in seconds
 */
float GameLoop_GetDeltaTime(void);

/**
 * Get total elapsed time
 * @return Total time in seconds
 */
float GameLoop_GetTotalTime(void);

/**
 * Get current frame number
 * @return Frame number (0-based)
 */
uint64_t GameLoop_GetFrameNumber(void);

/**
 * Reset frame statistics
 * @return GAMELOOP_OK on success
 */
int GameLoop_ResetStats(void);

/**
 * Pause game loop (timing continues)
 * @return GAMELOOP_OK on success
 */
int GameLoop_Pause(void);

/**
 * Resume game loop
 * @return GAMELOOP_OK on success
 */
int GameLoop_Resume(void);

/**
 * Check if game loop is paused
 * @return 1 if paused, 0 if running
 */
int GameLoop_IsPaused(void);

/**
 * Set frame rate limiter enabled/disabled
 * @param enabled 1 to enable frame rate limiting, 0 to disable
 * @return GAMELOOP_OK on success
 */
int GameLoop_SetFrameLimiterEnabled(int enabled);

/**
 * Check if frame rate limiter is enabled
 * @return 1 if enabled, 0 if disabled
 */
int GameLoop_IsFrameLimiterEnabled(void);

/**
 * Get error message from last operation
 * @return Error message string
 */
const char* GameLoop_GetError(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_LOOP_H
