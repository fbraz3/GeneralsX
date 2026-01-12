/**
 * @file GameTimingSystem.cpp
 * @brief Global GameTiming system instantiation and lifecycle
 *
 * Creates and manages the global TheGameTiming singleton.
 * Ensures proper initialization and cleanup order.
 */

#include "SDL2Device/Common/SDL2GameTiming.h"
#include "Common/GameTiming.h"
#include "Common/Debug.h"

/**
 * @global TheGameTiming
 * @brief Global singleton for timing system
 *
 * Initialized by createGameTimingSystem()
 * Destroyed by destroyGameTimingSystem()
 */
GameTiming* TheGameTiming = nullptr;

/**
 * @brief Create the global timing system
 *
 * Called during engine initialization (in SDL2GameEngine::init).
 * Creates SDL2GameTiming instance and assigns to TheGameTiming.
 *
 * @return Pointer to created GameTiming (also accessible via TheGameTiming)
 */
GameTiming* createGameTimingSystem()
{
    if (TheGameTiming != nullptr)
    {
        DEBUG_LOG(("GameTiming system already created"));
        return TheGameTiming;
    }

    TheGameTiming = new SDL2GameTiming();
    if (TheGameTiming != nullptr)
    {
        TheGameTiming->initialize();
        DEBUG_LOG(("GameTiming system created and initialized"));
    }
    else
    {
        DEBUG_CRASH(("Failed to create GameTiming system"));
    }

    return TheGameTiming;
}

/**
 * @brief Destroy the global timing system
 *
 * Called during engine shutdown.
 * Deletes SDL2GameTiming instance and clears TheGameTiming pointer.
 */
void destroyGameTimingSystem()
{
    if (TheGameTiming != nullptr)
    {
        TheGameTiming->shutdown();
        delete TheGameTiming;
        TheGameTiming = nullptr;
        DEBUG_LOG(("GameTiming system destroyed"));
    }
}

/**
 * @brief Get the global timing system
 *
 * @return Pointer to TheGameTiming (or nullptr if not initialized)
 *
 * Useful for validation checks before access.
 */
GameTiming* getGameTimingSystem()
{
    return TheGameTiming;
}
