/**
 * @file WindowSystem.cpp
 * @brief Window system lifecycle implementation
 */

#include "SDL2Device/GameClient/WindowSystem.h"
#include "SDL2Device/GameClient/SDL2Window.h"
#include "Common/Debug.h"

/**
 * @brief Create and initialize the global window system
 */
SDL2Window* createWindowSystem(void* window)
{
    // Defined in SDL2Window.cpp - function signature:
    // SDL2Window* createSDL2WindowSystem(void* window);
    extern SDL2Window* createSDL2WindowSystem(void* window);
    return createSDL2WindowSystem(window);
}

/**
 * @brief Retrieve the global window system pointer
 */
SDL2Window* getWindowSystem()
{
    return TheSDL2Window;
}

/**
 * @brief Destroy and cleanup the global window system
 */
void destroyWindowSystem()
{
    // Defined in SDL2Window.cpp - function signature:
    // void destroySDL2WindowSystem();
    extern void destroySDL2WindowSystem();
    destroySDL2WindowSystem();
}
