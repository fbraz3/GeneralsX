/**
 * @file SDL2GameTiming.h
 * @brief SDL2-based implementation of GameTiming
 *
 * Uses SDL_GetTicks() for portable cross-platform timing.
 * Handles frame delta calculation with proper 32-bit wraparound.
 */

#pragma once

#include "Common/GameTiming.h"

/**
 * @class SDL2GameTiming
 * @brief SDL2 backend for GameTiming system
 *
 * Features:
 * - SDL_GetTicks() for millisecond timing (wraps every ~49 days)
 * - Frame delta calculation with wraparound safety
 * - Smoothed FPS calculation
 * - SDL_Delay() for frame pacing
 * - SDL_GetPerformanceCounter() for high-res timing (if available)
 *
 * Architecture:
 * - Singleton pattern with global TheGameTiming pointer
 * - Initialized in SDL2GameEngine::initialize()
 * - Updated each frame in main loop
 */
class SDL2GameTiming : public GameTiming
{
public:
    /**
     * @brief Constructor
     */
    SDL2GameTiming();

    /**
     * @brief Destructor
     */
    virtual ~SDL2GameTiming();

    /**
     * @brief Initialize timing system
     * Stores initial tick value for reference.
     */
    virtual void initialize();

    /**
     * @brief Shutdown timing system
     */
    virtual void shutdown();

    /**
     * @brief Get current time in milliseconds
     */
    virtual UInt32 getTicks() const;

    /**
     * @brief Get frame delta time in milliseconds
     */
    virtual UInt32 getFrameTime() const;

    /**
     * @brief Update frame timing state
     * Must be called once per frame in main loop.
     */
    virtual void updateFrameTime();

    /**
     * @brief Get current frame rate (FPS)
     */
    virtual float getFrameRate() const;

    /**
     * @brief Sleep for specified milliseconds
     */
    virtual void sleep(UInt32 milliseconds) const;

    /**
     * @brief Get performance counter value
     */
    virtual UInt64 getPerformanceCounter() const;

    /**
     * @brief Get performance counter frequency
     */
    virtual UInt64 getPerformanceFrequency() const;

private:
    /**
     * @brief Initialize tick for the system
     */
    UInt32 m_initTicks;

    /**
     * @brief Last frame tick value
     */
    UInt32 m_lastFrameTicks;

    /**
     * @brief Current frame delta time in milliseconds
     */
    UInt32 m_frameTimeDelta;

    /**
     * @brief Smoothed FPS value
     * Updated each frame, smoothed over multiple frames for stability
     */
    float m_smoothedFPS;

    /**
     * @brief FPS smoothing factor (0.0-1.0)
     * Higher values = more responsive, lower values = more stable
     * 0.1 means new FPS contributes 10% to smoothed value
     */
    static const float FPS_SMOOTHING_FACTOR;

    /**
     * @brief Target FPS for frame pacing
     */
    static const UInt32 TARGET_FPS;

    /**
     * @brief Target frame time in milliseconds (1000/TARGET_FPS)
     */
    static const UInt32 TARGET_FRAME_TIME_MS;
};

#endif // SDL2_GAME_TIMING_H
