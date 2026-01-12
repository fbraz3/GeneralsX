/**
 * @file GameTimingSystem.h
 * @brief Global GameTiming system lifecycle management
 */

#pragma once

// Forward declaration
class GameTiming;

/**
 * @brief Create and initialize the global timing system
 *
 * Called during SDL2GameEngine initialization.
 * Creates SDL2GameTiming instance and calls initialize().
 *
 * @return Pointer to created GameTiming
 */
GameTiming* createGameTimingSystem();

/**
 * @brief Destroy the global timing system
 *
 * Called during SDL2GameEngine shutdown.
 * Calls shutdown() and deletes the instance.
 */
void destroyGameTimingSystem();

/**
 * @brief Get the global timing system
 *
 * @return Pointer to TheGameTiming (or nullptr if not initialized)
 *
 * Useful for validation before calling methods.
 */
GameTiming* getGameTimingSystem();

#endif // GAME_TIMING_SYSTEM_H
