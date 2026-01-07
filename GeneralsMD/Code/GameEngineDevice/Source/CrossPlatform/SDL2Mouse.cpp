// ============================================================================
// SDL2 Mouse Implementation
// Cross-platform mouse input handling using SDL2
// ============================================================================

#include "CrossPlatform/SDL2Mouse.h"
#include "Common/Debug.h"
#include "Common/GlobalData.h"
#include "GameClient/GameClient.h"
#include "WinMain.h"
#include <cstring>

// Global reference for external access (if needed)
SDL2Mouse* TheSDL2Mouse = nullptr;

// ============================================================================
// Constructor / Destructor
// ============================================================================

SDL2Mouse::SDL2Mouse(void)
    : Mouse()
    , m_nextFreeIndex(0)
    , m_nextGetIndex(0)
    , m_currentSDL2Cursor(NONE)
    , m_directionFrame(0)
    , m_lostFocus(false)
{
    DEBUG_LOG(("SDL2Mouse: Created"));

    // Zero out event buffer
    memset(&m_eventBuffer, 0, sizeof(m_eventBuffer));

    // Initialize cursor array to null
    for (Int i = 0; i < NUM_MOUSE_CURSORS; i++)
    {
        m_sdlCursors[i] = nullptr;
    }
}

SDL2Mouse::~SDL2Mouse(void)
{
    DEBUG_LOG(("SDL2Mouse: Destroyed"));

    // Free SDL2 cursor resources
    for (Int i = 0; i < NUM_MOUSE_CURSORS; i++)
    {
        if (m_sdlCursors[i] != nullptr)
        {
            SDL_FreeCursor(m_sdlCursors[i]);
            m_sdlCursors[i] = nullptr;
        }
    }

    TheSDL2Mouse = nullptr;
}

// ============================================================================
// Initialization / Reset
// ============================================================================

void SDL2Mouse::init(void)
{
    DEBUG_LOG(("SDL2Mouse::init() - Initializing SDL2 mouse input"));

    // Call base class init
    Mouse::init();

    // When we receive SDL2 mouse events, the mouse moves report
    // current cursor position (not deltas), so process as absolute
    m_inputMovesAbsolute = TRUE;

    // Set global reference
    TheSDL2Mouse = this;

    // Ensure SDL2 is initialized for mouse
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
    {
        DEBUG_LOG(("SDL2Mouse::init() - WARNING: SDL_VIDEO not initialized"));
    }
}

void SDL2Mouse::reset(void)
{
    DEBUG_LOG(("SDL2Mouse::reset()"));

    // Call base class reset
    Mouse::reset();

    // Clear event buffer
    memset(&m_eventBuffer, 0, sizeof(m_eventBuffer));
    m_nextFreeIndex = 0;
    m_nextGetIndex = 0;
}

// ============================================================================
// Update
// ============================================================================

void SDL2Mouse::update(void)
{
    // Process any pending SDL2 events
    processSDL2Events();

    // Call base class update
    Mouse::update();
}

void SDL2Mouse::processSDL2Events(void)
{
    SDL_Event event;

    // Poll all pending SDL2 events
    // Note: We use PeepEvents instead of PollEvent to avoid consuming
    // events that other systems may need
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEWHEEL) > 0)
    {
        addSDL2Event(event);
    }
}

void SDL2Mouse::addSDL2Event(const SDL_Event& event)
{
    // Check if buffer is full
    if (m_eventBuffer[m_nextFreeIndex].type != 0)
    {
        // Buffer full, event will be lost
        return;
    }

    // Store event data based on type
    m_eventBuffer[m_nextFreeIndex].type = event.type;
    m_eventBuffer[m_nextFreeIndex].timestamp = event.common.timestamp;

    switch (event.type)
    {
        case SDL_MOUSEMOTION:
            m_eventBuffer[m_nextFreeIndex].x = event.motion.x;
            m_eventBuffer[m_nextFreeIndex].y = event.motion.y;
            m_eventBuffer[m_nextFreeIndex].button = 0;
            m_eventBuffer[m_nextFreeIndex].wheelY = 0;
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            m_eventBuffer[m_nextFreeIndex].x = event.button.x;
            m_eventBuffer[m_nextFreeIndex].y = event.button.y;
            m_eventBuffer[m_nextFreeIndex].button = event.button.button;
            m_eventBuffer[m_nextFreeIndex].wheelY = 0;
            break;

        case SDL_MOUSEWHEEL:
            // Get current mouse position for wheel events
            SDL_GetMouseState(&m_eventBuffer[m_nextFreeIndex].x,
                             &m_eventBuffer[m_nextFreeIndex].y);
            m_eventBuffer[m_nextFreeIndex].button = 0;
            m_eventBuffer[m_nextFreeIndex].wheelY = event.wheel.y * MOUSE_WHEEL_DELTA;
            break;

        default:
            // Unknown event type, don't store
            return;
    }

    // Advance to next free slot, wrapping at buffer end
    m_nextFreeIndex++;
    if (m_nextFreeIndex >= Mouse::NUM_MOUSE_EVENTS)
    {
        m_nextFreeIndex = 0;
    }
}

// ============================================================================
// Mouse Event Processing
// ============================================================================

UnsignedByte SDL2Mouse::getMouseEvent(MouseIO* result, Bool flush)
{
    // Check if there's an event available
    if (m_eventBuffer[m_nextGetIndex].type == 0)
    {
        return MOUSE_NONE;
    }

    // Translate the SDL2 mouse event to our own format
    translateEvent(m_nextGetIndex, result);

    // Remove this event from buffer by clearing type
    m_eventBuffer[m_nextGetIndex].type = 0;

    // Advance to next event, wrapping at buffer end
    m_nextGetIndex++;
    if (m_nextGetIndex >= Mouse::NUM_MOUSE_EVENTS)
    {
        m_nextGetIndex = 0;
    }

    return MOUSE_OK;
}

void SDL2Mouse::translateEvent(UnsignedInt eventIndex, MouseIO* result)
{
    SDL2MouseEvent& ev = m_eventBuffer[eventIndex];
    UnsignedInt frame;

    // Get current input frame from the client
    if (TheGameClient)
    {
        frame = TheGameClient->getFrame();
    }
    else
    {
        frame = 1;
    }

    // Initialize to defaults
    result->leftState = MBS_Up;
    result->middleState = MBS_Up;
    result->rightState = MBS_Up;
    result->leftEvent = 0;
    result->middleEvent = 0;
    result->rightEvent = 0;
    result->pos.x = 0;
    result->pos.y = 0;
    result->wheelPos = 0;
    result->time = ev.timestamp;

    switch (ev.type)
    {
        case SDL_MOUSEBUTTONDOWN:
        {
            result->pos.x = ev.x;
            result->pos.y = ev.y;

            switch (ev.button)
            {
                case SDL_BUTTON_LEFT:
                    result->leftState = MBS_Down;
                    result->leftEvent = frame;
                    break;
                case SDL_BUTTON_MIDDLE:
                    result->middleState = MBS_Down;
                    result->middleEvent = frame;
                    break;
                case SDL_BUTTON_RIGHT:
                    result->rightState = MBS_Down;
                    result->rightEvent = frame;
                    break;
            }
            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            result->pos.x = ev.x;
            result->pos.y = ev.y;

            switch (ev.button)
            {
                case SDL_BUTTON_LEFT:
                    result->leftState = MBS_Up;
                    result->leftEvent = frame;
                    break;
                case SDL_BUTTON_MIDDLE:
                    result->middleState = MBS_Up;
                    result->middleEvent = frame;
                    break;
                case SDL_BUTTON_RIGHT:
                    result->rightState = MBS_Up;
                    result->rightEvent = frame;
                    break;
            }
            break;
        }

        case SDL_MOUSEMOTION:
        {
            result->pos.x = ev.x;
            result->pos.y = ev.y;
            break;
        }

        case SDL_MOUSEWHEEL:
        {
            result->pos.x = ev.x;
            result->pos.y = ev.y;
            result->wheelPos = ev.wheelY;
            break;
        }

        default:
        {
            DEBUG_LOG(("SDL2Mouse::translateEvent - Unknown event type: %d", ev.type));
            break;
        }
    }
}

// ============================================================================
// Cursor Resources
// ============================================================================

void SDL2Mouse::initCursorResources(void)
{
    DEBUG_LOG(("SDL2Mouse::initCursorResources() - Loading cursor resources"));

    // Create basic system cursors for now
    // In a full implementation, we would load custom cursor images from Data/Cursors/

    // Use system arrow cursor as default for most cursors
    SDL_Cursor* arrowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    SDL_Cursor* handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_Cursor* crosshairCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    SDL_Cursor* waitCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    SDL_Cursor* ibeamCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    SDL_Cursor* noCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
    SDL_Cursor* sizeAllCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);

    // Map game cursors to SDL2 system cursors
    // This is a simplified mapping - full implementation would load .ANI files
    for (Int i = FIRST_CURSOR; i < NUM_MOUSE_CURSORS; i++)
    {
        switch (i)
        {
            case NORMAL:
            case ARROW:
            case SELECTING:
                m_sdlCursors[i] = arrowCursor;
                break;

            case SCROLL:
            case MOVETO:
            case ATTACKMOVETO:
            case SET_RALLY_POINT:
                m_sdlCursors[i] = sizeAllCursor;
                break;

            case CROSS:
            case ATTACK_OBJECT:
            case FORCE_ATTACK_OBJECT:
            case FORCE_ATTACK_GROUND:
            case SNIPE_VEHICLE:
            case LASER_GUIDED_MISSILES:
            case PARTICLE_UPLINK_CANNON:
                m_sdlCursors[i] = crosshairCursor;
                break;

            case BUILD_PLACEMENT:
            case ENTER_FRIENDLY:
            case ENTER_AGGRESSIVELY:
            case GET_REPAIRED:
            case GET_HEALED:
            case DO_REPAIR:
            case RESUME_CONSTRUCTION:
            case CAPTUREBUILDING:
            case DOCK:
            case HACK:
                m_sdlCursors[i] = handCursor;
                break;

            case INVALID_BUILD_PLACEMENT:
            case GENERIC_INVALID:
            case STAB_ATTACK_INVALID:
            case PLACE_CHARGE_INVALID:
            case OUTRANGE:
                m_sdlCursors[i] = noCursor;
                break;

            case WAYPOINT:
            case PLACE_BEACON:
            case PLACE_REMOTE_CHARGE:
            case PLACE_TIMED_CHARGE:
                m_sdlCursors[i] = crosshairCursor;
                break;

            default:
                m_sdlCursors[i] = arrowCursor;
                break;
        }
    }

    DEBUG_LOG(("SDL2Mouse::initCursorResources() - Loaded %d cursors", NUM_MOUSE_CURSORS));
}

// ============================================================================
// Cursor Management
// ============================================================================

void SDL2Mouse::setCursor(MouseCursor cursor)
{
    // Call base class
    Mouse::setCursor(cursor);

    if (m_lostFocus)
    {
        return;
    }

    if (cursor == NONE || !m_visible)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    else
    {
        SDL_ShowCursor(SDL_ENABLE);

        if (cursor >= FIRST_CURSOR && cursor < NUM_MOUSE_CURSORS)
        {
            if (m_sdlCursors[cursor] != nullptr)
            {
                SDL_SetCursor(m_sdlCursors[cursor]);
            }
        }
    }

    m_currentSDL2Cursor = m_currentCursor = cursor;
}

void SDL2Mouse::setVisibility(Bool visible)
{
    // Call base class
    Mouse::setVisibility(visible);

    // Update SDL2 cursor visibility
    if (visible)
    {
        SDL_ShowCursor(SDL_ENABLE);
        setCursor(getMouseCursor());
    }
    else
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
}

// ============================================================================
// Focus Management
// ============================================================================

void SDL2Mouse::loseFocus()
{
    Mouse::loseFocus();
    m_lostFocus = true;
}

void SDL2Mouse::regainFocus()
{
    Mouse::regainFocus();
    m_lostFocus = false;
}

// ============================================================================
// Mouse Capture
// ============================================================================

void SDL2Mouse::capture(void)
{
    DEBUG_LOG(("SDL2Mouse::capture() - Capturing mouse"));

    // Get the SDL window
    SDL_Window* window = SDL_GetWindowFromID(1);  // Assume main window has ID 1
    if (window == nullptr)
    {
        // Try to get window from ApplicationHWnd if it's an SDL_Window pointer
        window = reinterpret_cast<SDL_Window*>(ApplicationHWnd);
    }

    if (window != nullptr)
    {
        // Constrain mouse to window
        SDL_SetWindowGrab(window, SDL_TRUE);

        // Also set relative mouse mode for better capture behavior
        // SDL_SetRelativeMouseMode(SDL_TRUE);  // Uncomment if needed

        onCursorCaptured(true);
    }
}

void SDL2Mouse::releaseCapture(void)
{
    DEBUG_LOG(("SDL2Mouse::releaseCapture() - Releasing mouse capture"));

    SDL_Window* window = SDL_GetWindowFromID(1);
    if (window == nullptr)
    {
        window = reinterpret_cast<SDL_Window*>(ApplicationHWnd);
    }

    if (window != nullptr)
    {
        SDL_SetWindowGrab(window, SDL_FALSE);
        // SDL_SetRelativeMouseMode(SDL_FALSE);  // Uncomment if capture uses relative mode

        onCursorCaptured(false);
    }
}

// ============================================================================
// Factory Function
// ============================================================================

Mouse* CreateMouse(void)
{
    DEBUG_LOG(("CreateMouse: Creating SDL2Mouse"));
    return NEW SDL2Mouse;
}
