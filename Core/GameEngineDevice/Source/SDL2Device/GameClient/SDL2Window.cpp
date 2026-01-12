/**
 * @file SDL2Window.cpp
 * @brief SDL2-based implementation of window management
 */

#include "SDL2Device/GameClient/SDL2Window.h"
#include "Common/Debug.h"

#include <SDL2/SDL.h>
#include <cstring>
#include <algorithm>

/**
 * @global TheSDL2Window
 * @brief Global singleton instance
 */
SDL2Window* TheSDL2Window = nullptr;

/**
 * @brief Constructor
 */
SDL2Window::SDL2Window()
{
}

/**
 * @brief Destructor
 */
SDL2Window::~SDL2Window()
{
}

/**
 * @class SDL2WindowImpl
 * @brief Concrete SDL2 implementation of SDL2Window interface
 */
class SDL2WindowImpl : public SDL2Window
{
public:
    /**
     * @brief Constructor
     */
    SDL2WindowImpl();

    /**
     * @brief Destructor
     */
    virtual ~SDL2WindowImpl();

    /**
     * @brief Initialize window management
     */
    virtual void initialize(void* window);

    /**
     * @brief Shutdown window management
     */
    virtual void shutdown();

    /**
     * @brief Attach to existing SDL window
     */
    virtual void attachWindow(void* window);

    /**
     * @brief Get window width
     */
    virtual int getWidth() const;

    /**
     * @brief Get window height
     */
    virtual int getHeight() const;

    /**
     * @brief Get window size
     */
    virtual void getSize(int* outWidth, int* outHeight) const;

    /**
     * @brief Set window size
     */
    virtual void setSize(int width, int height);

    /**
     * @brief Get window X position
     */
    virtual int getX() const;

    /**
     * @brief Get window Y position
     */
    virtual int getY() const;

    /**
     * @brief Get window position
     */
    virtual void getPosition(int* outX, int* outY) const;

    /**
     * @brief Set window position
     */
    virtual void setPosition(int x, int y);

    /**
     * @brief Get window rectangle
     */
    virtual void getRect(int* outX, int* outY, int* outWidth, int* outHeight) const;

    /**
     * @brief Set fullscreen mode
     */
    virtual void setFullscreen(bool fullscreen, bool exclusive = false);

    /**
     * @brief Check if fullscreen
     */
    virtual bool isFullscreen() const;

    /**
     * @brief Check if visible
     */
    virtual bool isVisible() const;

    /**
     * @brief Show window
     */
    virtual void show();

    /**
     * @brief Hide window
     */
    virtual void hide();

    /**
     * @brief Minimize window
     */
    virtual void minimize();

    /**
     * @brief Maximize window
     */
    virtual void maximize();

    /**
     * @brief Restore window
     */
    virtual void restore();

    /**
     * @brief Check if minimized
     */
    virtual bool isMinimized() const;

    /**
     * @brief Check if maximized
     */
    virtual bool isMaximized() const;

    /**
     * @brief Set window title
     */
    virtual void setTitle(const char* title);

    /**
     * @brief Get window title
     */
    virtual const char* getTitle() const;

    /**
     * @brief Raise window
     */
    virtual void raise();

    /**
     * @brief Get DPI scale
     */
    virtual float getDPIScale() const;

    /**
     * @brief Get display width
     */
    virtual int getDisplayWidth() const;

    /**
     * @brief Get display height
     */
    virtual int getDisplayHeight() const;

    /**
     * @brief Center on screen
     */
    virtual void centerOnScreen();

private:
    /**
     * @brief Underlying SDL_Window pointer
     */
    SDL_Window* m_window;

    /**
     * @brief Cached window title
     */
    char m_title[256];

    /**
     * @brief Track fullscreen state
     */
    bool m_isFullscreen;
};

/**
 * @brief Constructor
 */
SDL2WindowImpl::SDL2WindowImpl()
    : m_window(nullptr)
    , m_isFullscreen(false)
{
    std::memset(m_title, 0, sizeof(m_title));
}

/**
 * @brief Destructor
 */
SDL2WindowImpl::~SDL2WindowImpl()
{
    shutdown();
}

/**
 * @brief Initialize window management
 */
void SDL2WindowImpl::initialize(void* window)
{
    m_window = static_cast<SDL_Window*>(window);
    if (m_window == nullptr)
    {
        DEBUG_CRASH(("SDL2Window::initialize - window is nullptr"));
    }

    // Get initial title
    const char* title = SDL_GetWindowTitle(m_window);
    if (title != nullptr)
    {
        std::strncpy(m_title, title, sizeof(m_title) - 1);
    }

    // Check initial fullscreen state
    Uint32 flags = SDL_GetWindowFlags(m_window);
    m_isFullscreen = (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;

    DEBUG_LOG(("SDL2Window initialized: %dx%d %s", 
        getWidth(), getHeight(), m_isFullscreen ? "FULLSCREEN" : "WINDOWED"));
}

/**
 * @brief Shutdown window management
 */
void SDL2WindowImpl::shutdown()
{
    // SDL_Window is managed by SDL2GameEngine, don't delete it
    m_window = nullptr;
}

/**
 * @brief Attach to existing SDL window
 */
void SDL2WindowImpl::attachWindow(void* window)
{
    m_window = static_cast<SDL_Window*>(window);
}

/**
 * @brief Get window width
 */
int SDL2WindowImpl::getWidth() const
{
    if (m_window == nullptr) return 0;
    int w = 0;
    SDL_GetWindowSize(m_window, &w, nullptr);
    return w;
}

/**
 * @brief Get window height
 */
int SDL2WindowImpl::getHeight() const
{
    if (m_window == nullptr) return 0;
    int h = 0;
    SDL_GetWindowSize(m_window, nullptr, &h);
    return h;
}

/**
 * @brief Get window size
 */
void SDL2WindowImpl::getSize(int* outWidth, int* outHeight) const
{
    if (m_window == nullptr) return;
    SDL_GetWindowSize(m_window, outWidth, outHeight);
}

/**
 * @brief Set window size
 */
void SDL2WindowImpl::setSize(int width, int height)
{
    if (m_window == nullptr) return;
    SDL_SetWindowSize(m_window, width, height);
}

/**
 * @brief Get window X position
 */
int SDL2WindowImpl::getX() const
{
    if (m_window == nullptr) return 0;
    int x = 0;
    SDL_GetWindowPosition(m_window, &x, nullptr);
    return x;
}

/**
 * @brief Get window Y position
 */
int SDL2WindowImpl::getY() const
{
    if (m_window == nullptr) return 0;
    int y = 0;
    SDL_GetWindowPosition(m_window, nullptr, &y);
    return y;
}

/**
 * @brief Get window position
 */
void SDL2WindowImpl::getPosition(int* outX, int* outY) const
{
    if (m_window == nullptr) return;
    SDL_GetWindowPosition(m_window, outX, outY);
}

/**
 * @brief Set window position
 */
void SDL2WindowImpl::setPosition(int x, int y)
{
    if (m_window == nullptr) return;
    SDL_SetWindowPosition(m_window, x, y);
}

/**
 * @brief Get window rectangle
 */
void SDL2WindowImpl::getRect(int* outX, int* outY, int* outWidth, int* outHeight) const
{
    if (m_window == nullptr) return;
    SDL_GetWindowPosition(m_window, outX, outY);
    SDL_GetWindowSize(m_window, outWidth, outHeight);
}

/**
 * @brief Set fullscreen mode
 */
void SDL2WindowImpl::setFullscreen(bool fullscreen, bool exclusive)
{
    if (m_window == nullptr) return;

    Uint32 flags = 0;
    if (fullscreen)
    {
        // exclusive = true: SDL_WINDOW_FULLSCREEN (real fullscreen)
        // exclusive = false: SDL_WINDOW_FULLSCREEN_DESKTOP (borderless fullscreen)
        flags = exclusive ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (SDL_SetWindowFullscreen(m_window, flags) == 0)
    {
        m_isFullscreen = fullscreen;
        DEBUG_LOG(("Fullscreen: %s (exclusive=%d)", fullscreen ? "ON" : "OFF", exclusive));
    }
    else
    {
        DEBUG_LOG(("Failed to set fullscreen: %s", SDL_GetError()));
    }
}

/**
 * @brief Check if fullscreen
 */
bool SDL2WindowImpl::isFullscreen() const
{
    return m_isFullscreen;
}

/**
 * @brief Check if visible
 */
bool SDL2WindowImpl::isVisible() const
{
    if (m_window == nullptr) return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_SHOWN) != 0;
}

/**
 * @brief Show window
 */
void SDL2WindowImpl::show()
{
    if (m_window == nullptr) return;
    SDL_ShowWindow(m_window);
}

/**
 * @brief Hide window
 */
void SDL2WindowImpl::hide()
{
    if (m_window == nullptr) return;
    SDL_HideWindow(m_window);
}

/**
 * @brief Minimize window
 */
void SDL2WindowImpl::minimize()
{
    if (m_window == nullptr) return;
    SDL_MinimizeWindow(m_window);
}

/**
 * @brief Maximize window
 */
void SDL2WindowImpl::maximize()
{
    if (m_window == nullptr) return;
    SDL_MaximizeWindow(m_window);
}

/**
 * @brief Restore window
 */
void SDL2WindowImpl::restore()
{
    if (m_window == nullptr) return;
    SDL_RestoreWindow(m_window);
}

/**
 * @brief Check if minimized
 */
bool SDL2WindowImpl::isMinimized() const
{
    if (m_window == nullptr) return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_MINIMIZED) != 0;
}

/**
 * @brief Check if maximized
 */
bool SDL2WindowImpl::isMaximized() const
{
    if (m_window == nullptr) return false;
    Uint32 flags = SDL_GetWindowFlags(m_window);
    return (flags & SDL_WINDOW_MAXIMIZED) != 0;
}

/**
 * @brief Set window title
 */
void SDL2WindowImpl::setTitle(const char* title)
{
    if (m_window == nullptr) return;
    if (title == nullptr) return;

    std::strncpy(m_title, title, sizeof(m_title) - 1);
    SDL_SetWindowTitle(m_window, title);
}

/**
 * @brief Get window title
 */
const char* SDL2WindowImpl::getTitle() const
{
    return m_title;
}

/**
 * @brief Raise window
 */
void SDL2WindowImpl::raise()
{
    if (m_window == nullptr) return;
    SDL_RaiseWindow(m_window);
}

/**
 * @brief Get DPI scale
 */
float SDL2WindowImpl::getDPIScale() const
{
    if (m_window == nullptr) return 1.0f;

    int displayIndex = SDL_GetWindowDisplayIndex(m_window);
    float dpi = 96.0f;  // Standard DPI

    if (SDL_GetDisplayDPI(displayIndex, nullptr, &dpi, nullptr) == 0)
    {
        return dpi / 96.0f;  // Normalize to 96 DPI baseline
    }

    return 1.0f;
}

/**
 * @brief Get display width
 */
int SDL2WindowImpl::getDisplayWidth() const
{
    SDL_Rect bounds;
    if (SDL_GetDisplayBounds(0, &bounds) == 0)
    {
        return bounds.w;
    }
    return 1024;  // Default fallback
}

/**
 * @brief Get display height
 */
int SDL2WindowImpl::getDisplayHeight() const
{
    SDL_Rect bounds;
    if (SDL_GetDisplayBounds(0, &bounds) == 0)
    {
        return bounds.h;
    }
    return 768;  // Default fallback
}

/**
 * @brief Center on screen
 */
void SDL2WindowImpl::centerOnScreen()
{
    if (m_window == nullptr) return;

    int displayWidth = getDisplayWidth();
    int displayHeight = getDisplayHeight();
    int windowWidth = getWidth();
    int windowHeight = getHeight();

    int x = (displayWidth - windowWidth) / 2;
    int y = (displayHeight - windowHeight) / 2;

    setPosition(x, y);
}

/**
 * @brief Create the global window management system
 *
 * Called during engine initialization (in SDL2GameEngine::init).
 * Creates SDL2WindowImpl instance and assigns to TheSDL2Window.
 *
 * @param window SDL_Window pointer
 * @return Pointer to created SDL2Window
 */
SDL2Window* createSDL2WindowSystem(void* window)
{
    if (TheSDL2Window != nullptr)
    {
        DEBUG_LOG(("SDL2Window system already created"));
        return TheSDL2Window;
    }

    TheSDL2Window = new SDL2WindowImpl();
    if (TheSDL2Window != nullptr)
    {
        TheSDL2Window->initialize(window);
        DEBUG_LOG(("SDL2Window system created and initialized"));
    }
    else
    {
        DEBUG_CRASH(("Failed to create SDL2Window system"));
    }

    return TheSDL2Window;
}

/**
 * @brief Destroy the global window management system
 *
 * Called during engine shutdown.
 */
void destroySDL2WindowSystem()
{
    if (TheSDL2Window != nullptr)
    {
        TheSDL2Window->shutdown();
        delete TheSDL2Window;
        TheSDL2Window = nullptr;
        DEBUG_LOG(("SDL2Window system destroyed"));
    }
}
