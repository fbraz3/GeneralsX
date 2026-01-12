/**
 * @file WindowSystem.h
 * @brief Window system lifecycle management
 *
 * Handles creation, initialization, and destruction of the global window
 * management system. Follows the same pattern as GameTimingSystem.
 */

#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

/**
 * @class SDL2Window
 * @brief Forward declaration
 */
class SDL2Window;

/**
 * @brief Global window manager pointer
 *
 * Initialized by createWindowSystem() and destroyed by destroyWindowSystem().
 * Access pattern: if (TheSDL2Window) { TheSDL2Window->getWidth(); }
 */
extern SDL2Window* TheSDL2Window;

/**
 * @brief Create and initialize the global window system
 *
 * This function must be called during engine initialization (in SDL2GameEngine::init).
 * It creates the concrete SDL2WindowImpl instance and stores it in TheSDL2Window.
 *
 * @param window Void pointer to SDL_Window* object (SDL2GameEngine passes this)
 * @return Pointer to created SDL2Window, or nullptr on failure
 *
 * @note
 * - Must be called after SDL2 initialization and window creation
 * - Must be called before any code accesses TheSDL2Window
 * - Calling multiple times returns existing instance without re-initializing
 *
 * @example
 * @code
 * // During SDL2GameEngine::init
 * createWindowSystem(m_sdlWindow);  // Pass the SDL_Window*
 * if (TheSDL2Window) {
 *     TheSDL2Window->setTitle("My Game");
 *     int w = TheSDL2Window->getWidth();
 * }
 * @endcode
 */
SDL2Window* createWindowSystem(void* window);

/**
 * @brief Retrieve the global window system pointer
 *
 * @return Pointer to SDL2Window, or nullptr if not initialized
 *
 * @note
 * This function is mainly used for validation. In normal operation,
 * access TheSDL2Window directly.
 *
 * @example
 * @code
 * SDL2Window* windowMgr = getWindowSystem();
 * if (windowMgr) {
 *     windowMgr->centerOnScreen();
 * }
 * @endcode
 */
SDL2Window* getWindowSystem();

/**
 * @brief Destroy and cleanup the global window system
 *
 * This function must be called during engine shutdown (in SDL2GameEngine::shutdown).
 * It destroys the SDL2WindowImpl instance and clears TheSDL2Window to nullptr.
 *
 * @note
 * - Must be called before SDL2 shutdown
 * - Safe to call multiple times (subsequent calls are no-ops)
 * - All subsequent accesses to TheSDL2Window should be nullptr-checked
 *
 * @example
 * @code
 * // During SDL2GameEngine::shutdown
 * destroyWindowSystem();
 * TheSDL2Window = nullptr;  // Already nullptr, but good practice
 * @endcode
 */
void destroyWindowSystem();

#endif  // WINDOW_SYSTEM_H
