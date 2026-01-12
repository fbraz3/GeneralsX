/**
 * @file GameTiming.h
 * @brief SDL2-based cross-platform timing system
 *
 * Provides frame timing, performance measurement, and sleep utilities.
 * Replaces Win32 GetTickCount/Sleep with SDL2 equivalents for cross-platform support.
 */

#pragma once

// Forward declarations
struct SDL_TimerID;

/**
 * @class GameTiming
 * @brief Cross-platform timing system using SDL2 backend
 *
 * Provides:
 * - Frame timing and delta time calculation
 * - Performance counter for FPS tracking
 * - Frame pacing with SDL_Delay
 * - Platform-agnostic millisecond timing
 *
 * Usage:
 *   UInt32 ticks = TheGameTiming->getTicks();
 *   float deltaTime = TheGameTiming->getFrameTime();
 *   TheGameTiming->sleep(16);  // ~60 FPS pacing
 */
class GameTiming
{
public:
    /**
     * @brief Initialize timing system
     * Called during engine startup (in SDL2GameEngine::initialize)
     */
    virtual void initialize() = 0;

    /**
     * @brief Shutdown timing system
     * Called during engine shutdown
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get current time in milliseconds since SDL initialization
     *
     * @return Current time in milliseconds (UInt32)
     *
     * Replaces: GetTickCount()
     * Note: Wraps around after ~49 days, but wrapping is handled correctly
     *       if used for frame deltas (e.g., current - previous)
     */
    virtual UInt32 getTicks() const = 0;

    /**
     * @brief Get milliseconds elapsed since last frame
     *
     * @return Frame delta time in milliseconds
     *
     * Properly handles SDL_GetTicks() wraparound by using 32-bit unsigned arithmetic.
     * Should be called once per frame to update timing state.
     */
    virtual UInt32 getFrameTime() const = 0;

    /**
     * @brief Update frame timing
     *
     * Calculates elapsed time since last update.
     * Must be called once per frame in main loop.
     */
    virtual void updateFrameTime() = 0;

    /**
     * @brief Get frame rate (frames per second)
     *
     * @return Current FPS (calculated from frame time)
     *
     * Returns smoothed FPS value for stability.
     */
    virtual float getFrameRate() const = 0;

    /**
     * @brief Sleep for specified milliseconds
     *
     * @param milliseconds Time to sleep in milliseconds
     *
     * Replaces: Sleep(ms)
     * Note: SDL_Delay is not precise for very short times (<1ms)
     */
    virtual void sleep(UInt32 milliseconds) const = 0;

    /**
     * @brief Get high-resolution performance counter
     *
     * @return Performance counter value
     *
     * For high-precision timing if needed (e.g., profiling).
     * Optional platform feature, may not be available on all systems.
     *
     * Replaces: QueryPerformanceCounter()
     */
    virtual UInt64 getPerformanceCounter() const = 0;

    /**
     * @brief Get performance counter frequency
     *
     * @return Frequency in counts per second
     *
     * Use this to convert performance counter to seconds:
     *   seconds = (counter2 - counter1) / getPerformanceFrequency()
     */
    virtual UInt64 getPerformanceFrequency() const = 0;

    /**
     * @brief Destructor
     */
    virtual ~GameTiming() {}
};

/**
 * @global TheGameTiming
 * @brief Global singleton for timing system
 *
 * Access pattern:
 *   TheGameTiming->getTicks();
 *   TheGameTiming->getFrameTime();
 *   TheGameTiming->sleep(16);
 */
extern GameTiming* TheGameTiming;

#endif // GAME_TIMING_H
