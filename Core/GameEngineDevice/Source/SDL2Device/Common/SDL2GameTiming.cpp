/**
 * @file SDL2GameTiming.cpp
 * @brief SDL2-based implementation of GameTiming
 */

#include "SDL2Device/Common/SDL2GameTiming.h"
#include "Common/GameTiming.h"

#include <SDL2/SDL.h>
#include <algorithm>

// Static member initialization
const float SDL2GameTiming::FPS_SMOOTHING_FACTOR = 0.1f;  // 10% new, 90% old
const UInt32 SDL2GameTiming::TARGET_FPS = 60;
const UInt32 SDL2GameTiming::TARGET_FRAME_TIME_MS = 1000 / TARGET_FPS;  // ~16ms for 60 FPS

/**
 * @global TheGameTiming
 * @brief Global singleton instance
 */
GameTiming* TheGameTiming = nullptr;

/**
 * @brief Constructor
 */
SDL2GameTiming::SDL2GameTiming()
    : m_initTicks(0)
    , m_lastFrameTicks(0)
    , m_frameTimeDelta(0)
    , m_smoothedFPS(60.0f)  // Start assuming 60 FPS
{
}

/**
 * @brief Destructor
 */
SDL2GameTiming::~SDL2GameTiming()
{
}

/**
 * @brief Initialize timing system
 * 
 * Called during engine startup. Records initial SDL tick value and resets frame timing.
 */
void SDL2GameTiming::initialize()
{
    m_initTicks = SDL_GetTicks();
    m_lastFrameTicks = m_initTicks;
    m_frameTimeDelta = 0;
    m_smoothedFPS = 60.0f;
}

/**
 * @brief Shutdown timing system
 */
void SDL2GameTiming::shutdown()
{
    // SDL cleanup handled by SDL2GameEngine
}

/**
 * @brief Get current time in milliseconds since SDL initialization
 * 
 * @return Current milliseconds (UInt32)
 *
 * This replaces GetTickCount() on Windows. The value wraps after ~49 days,
 * but frame delta calculations handle this correctly due to 32-bit wraparound.
 */
UInt32 SDL2GameTiming::getTicks() const
{
    return SDL_GetTicks();
}

/**
 * @brief Get frame delta time in milliseconds
 *
 * @return Frame delta time in milliseconds
 *
 * Properly handles SDL_GetTicks() wraparound by using 32-bit unsigned arithmetic:
 *   If current > last: delta = current - last (normal case)
 *   If current < last: delta = current - last (wrapped, but subtraction handles it)
 *
 * Example: At wraparound (2^32 ms ≈ 49 days):
 *   last = 0xFFFFFF00 (4294967040)
 *   current = 0x00000100 (256)
 *   delta = 256 - 4294967040 = -4294966784
 *   As UInt32: -4294966784 = 0x00000200 = 512 ms (correct!)
 */
UInt32 SDL2GameTiming::getFrameTime() const
{
    return m_frameTimeDelta;
}

/**
 * @brief Update frame timing
 *
 * Must be called once per frame in main loop to calculate frame delta.
 * Updates smoothed FPS for performance monitoring.
 *
 * Typical usage in game loop:
 *   while (running) {
 *       // ... process events, render, update logic ...
 *       TheGameTiming->updateFrameTime();  // <-- Call at frame end
 *   }
 */
void SDL2GameTiming::updateFrameTime()
{
    UInt32 currentTicks = SDL_GetTicks();
    
    // Calculate delta with proper 32-bit wraparound handling
    // UInt32 subtraction automatically handles wraparound
    m_frameTimeDelta = currentTicks - m_lastFrameTicks;
    m_lastFrameTicks = currentTicks;
    
    // Update smoothed FPS
    // FPS = 1000 / deltaTime (in ms)
    // Smoothed: newFPS = (1 - FACTOR) * oldFPS + FACTOR * instantFPS
    if (m_frameTimeDelta > 0)
    {
        float instantFPS = 1000.0f / static_cast<float>(m_frameTimeDelta);
        m_smoothedFPS = (1.0f - FPS_SMOOTHING_FACTOR) * m_smoothedFPS + 
                        FPS_SMOOTHING_FACTOR * instantFPS;
    }
}

/**
 * @brief Get current frame rate (FPS)
 *
 * @return Smoothed FPS value
 *
 * Returns exponentially smoothed FPS for stability. The smoothing prevents
 * rapid fluctuations from single frame anomalies.
 */
float SDL2GameTiming::getFrameRate() const
{
    return m_smoothedFPS;
}

/**
 * @brief Sleep for specified milliseconds
 *
 * @param milliseconds Time to sleep in milliseconds
 *
 * This replaces Win32 Sleep(ms). SDL_Delay is cross-platform but not
 * high-precision for very short durations (<1ms).
 *
 * Note: For frame pacing, prefer to sleep until target frame time:
 *   UInt32 frameTime = TheGameTiming->getFrameTime();
 *   if (frameTime < TARGET_FRAME_TIME_MS) {
 *       TheGameTiming->sleep(TARGET_FRAME_TIME_MS - frameTime);
 *   }
 */
void SDL2GameTiming::sleep(UInt32 milliseconds) const
{
    if (milliseconds > 0)
    {
        SDL_Delay(milliseconds);
    }
}

/**
 * @brief Get high-resolution performance counter
 *
 * @return Performance counter value (UInt64)
 *
 * Returns SDL_GetPerformanceCounter() for high-precision timing.
 * Use getPerformanceFrequency() to convert to seconds:
 *
 *   UInt64 start = TheGameTiming->getPerformanceCounter();
 *   // ... do work ...
 *   UInt64 end = TheGameTiming->getPerformanceCounter();
 *   float seconds = (float)(end - start) / TheGameTiming->getPerformanceFrequency();
 */
UInt64 SDL2GameTiming::getPerformanceCounter() const
{
    return SDL_GetPerformanceCounter();
}

/**
 * @brief Get performance counter frequency
 *
 * @return Frequency in counts per second (UInt64)
 *
 * Returns SDL_GetPerformanceFrequency() for converting performance counter
 * values to absolute time.
 */
UInt64 SDL2GameTiming::getPerformanceFrequency() const
{
    return SDL_GetPerformanceFrequency();
}
