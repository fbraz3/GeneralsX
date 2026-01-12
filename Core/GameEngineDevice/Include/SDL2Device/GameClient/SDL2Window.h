/**
 * @file SDL2Window.h
 * @brief SDL2-based window management system
 *
 * Provides cross-platform window operations replacing Win32 window APIs.
 * Handles window size, position, fullscreen, and state management.
 */

#pragma once

/**
 * @class SDL2Window
 * @brief SDL2 backend for window management
 *
 * Features:
 * - Get/set window size and position
 * - Fullscreen toggle
 * - Minimize/maximize/restore operations
 * - Window title management
 * - Show/hide window
 * - Display mode queries
 *
 * Architecture:
 * - Wraps SDL_Window pointer from SDL2GameEngine
 * - Singleton pattern with global TheSDL2Window pointer
 * - Initialized in SDL2GameEngine::initialize()
 * - Handles window events from SDL2 event loop
 *
 * Replaces:
 * - GetWindowRect() / GetClientRect()
 * - SetWindowPos()
 * - ShowWindow()
 * - GetDesktopWindow()
 * - IsWindowVisible()
 */
class SDL2Window
{
public:
    /**
     * @brief Constructor
     */
    SDL2Window();

    /**
     * @brief Destructor
     */
    virtual ~SDL2Window();

    /**
     * @brief Initialize window management
     *
     * Called during engine startup.
     * Must be called after SDL_Window creation in SDL2GameEngine.
     *
     * @param window SDL_Window pointer from SDL2GameEngine
     */
    virtual void initialize(void* window) = 0;

    /**
     * @brief Shutdown window management
     */
    virtual void shutdown() = 0;

    /**
     * @brief Attach to existing SDL window
     *
     * @param window SDL_Window pointer
     */
    virtual void attachWindow(void* window) = 0;

    /**
     * @brief Get window width in pixels
     *
     * @return Width in pixels
     */
    virtual int getWidth() const = 0;

    /**
     * @brief Get window height in pixels
     *
     * @return Height in pixels
     */
    virtual int getHeight() const = 0;

    /**
     * @brief Get window size
     *
     * @param outWidth Pointer to receive width (can be nullptr)
     * @param outHeight Pointer to receive height (can be nullptr)
     */
    virtual void getSize(int* outWidth, int* outHeight) const = 0;

    /**
     * @brief Set window size in pixels
     *
     * @param width New width
     * @param height New height
     *
     * Replaces: SetWindowPos with SWP_NOSIZE
     */
    virtual void setSize(int width, int height) = 0;

    /**
     * @brief Get window X position
     *
     * @return X coordinate in pixels
     */
    virtual int getX() const = 0;

    /**
     * @brief Get window Y position
     *
     * @return Y coordinate in pixels
     */
    virtual int getY() const = 0;

    /**
     * @brief Get window position
     *
     * @param outX Pointer to receive X coordinate (can be nullptr)
     * @param outY Pointer to receive Y coordinate (can be nullptr)
     */
    virtual void getPosition(int* outX, int* outY) const = 0;

    /**
     * @brief Set window position
     *
     * @param x New X coordinate
     * @param y New Y coordinate
     *
     * Replaces: SetWindowPos with SWP_NOSIZE flag
     */
    virtual void setPosition(int x, int y) = 0;

    /**
     * @brief Get window rectangle (position and size)
     *
     * @param outX Pointer to receive X (can be nullptr)
     * @param outY Pointer to receive Y (can be nullptr)
     * @param outWidth Pointer to receive width (can be nullptr)
     * @param outHeight Pointer to receive height (can be nullptr)
     *
     * Replaces: GetWindowRect() + GetClientRect()
     */
    virtual void getRect(int* outX, int* outY, int* outWidth, int* outHeight) const = 0;

    /**
     * @brief Set window to fullscreen mode
     *
     * @param fullscreen true for fullscreen, false for windowed
     * @param exclusive true for exclusive fullscreen, false for borderless windowed
     *
     * Replaces: SetWindowPos with fullscreen coordinates
     */
    virtual void setFullscreen(bool fullscreen, bool exclusive = false) = 0;

    /**
     * @brief Check if window is fullscreen
     *
     * @return true if in fullscreen mode
     */
    virtual bool isFullscreen() const = 0;

    /**
     * @brief Check if window is visible
     *
     * @return true if window is visible
     *
     * Replaces: IsWindowVisible()
     */
    virtual bool isVisible() const = 0;

    /**
     * @brief Show the window
     *
     * Replaces: ShowWindow(SW_SHOW)
     */
    virtual void show() = 0;

    /**
     * @brief Hide the window
     *
     * Replaces: ShowWindow(SW_HIDE)
     */
    virtual void hide() = 0;

    /**
     * @brief Minimize the window
     *
     * Replaces: ShowWindow(SW_MINIMIZE)
     */
    virtual void minimize() = 0;

    /**
     * @brief Maximize the window
     *
     * Replaces: ShowWindow(SW_MAXIMIZE)
     */
    virtual void maximize() = 0;

    /**
     * @brief Restore window from minimized/maximized state
     *
     * Replaces: ShowWindow(SW_RESTORE)
     */
    virtual void restore() = 0;

    /**
     * @brief Check if window is minimized
     *
     * @return true if window is minimized
     */
    virtual bool isMinimized() const = 0;

    /**
     * @brief Check if window is maximized
     *
     * @return true if window is maximized
     */
    virtual bool isMaximized() const = 0;

    /**
     * @brief Set window title/caption
     *
     * @param title UTF-8 encoded title string
     *
     * Replaces: SetWindowText()
     */
    virtual void setTitle(const char* title) = 0;

    /**
     * @brief Get window title/caption
     *
     * @return Title string (UTF-8 encoded)
     */
    virtual const char* getTitle() const = 0;

    /**
     * @brief Raise/focus the window
     *
     * Brings window to front and gives it input focus.
     *
     * Replaces: SetForegroundWindow() / BringWindowToTop()
     */
    virtual void raise() = 0;

    /**
     * @brief Get display DPI scaling factor
     *
     * @return DPI scale (1.0 = 96 DPI, 1.25 = 120 DPI, etc.)
     *
     * Useful for high-DPI displays (Retina, 4K, etc.)
     */
    virtual float getDPIScale() const = 0;

    /**
     * @brief Get primary display width in pixels
     *
     * @return Display width
     */
    virtual int getDisplayWidth() const = 0;

    /**
     * @brief Get primary display height in pixels
     *
     * @return Display height
     */
    virtual int getDisplayHeight() const = 0;

    /**
     * @brief Center window on screen
     *
     * Positions window in center of primary display.
     */
    virtual void centerOnScreen() = 0;
};

/**
 * @global TheSDL2Window
 * @brief Global singleton for window management
 *
 * Access pattern:
 *   TheSDL2Window->getWidth();
 *   TheSDL2Window->setSize(800, 600);
 *   TheSDL2Window->setFullscreen(true);
 */
extern SDL2Window* TheSDL2Window;

#endif // SDL2_WINDOW_H
