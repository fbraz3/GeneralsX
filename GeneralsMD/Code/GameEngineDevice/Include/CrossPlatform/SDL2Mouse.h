// ============================================================================
// SDL2 Mouse Header
// Cross-platform mouse input handling using SDL2
// ============================================================================

#pragma once

#ifndef __SDL2_MOUSE_H__
#define __SDL2_MOUSE_H__

#include "GameClient/Mouse.h"
#include "Lib/BaseType.h"
#include <SDL2/SDL.h>

/**
 * SDL2Mouse - Cross-platform mouse input implementation
 * 
 * Provides mouse input handling for macOS, Linux, and Windows
 * using SDL2 as the input backend instead of Windows DirectInput/Win32 messages.
 */
class SDL2Mouse : public Mouse
{
public:
    SDL2Mouse(void);
    virtual ~SDL2Mouse(void);

    virtual void init(void) override;
    virtual void reset(void) override;
    virtual void update(void) override;
    virtual void initCursorResources(void) override;

    virtual void setCursor(MouseCursor cursor) override;
    virtual void setVisibility(Bool visible) override;

    virtual void loseFocus() override;
    virtual void regainFocus() override;

protected:
    virtual void capture(void) override;
    virtual void releaseCapture(void) override;
    virtual UnsignedByte getMouseEvent(MouseIO* result, Bool flush) override;

private:
    // Event buffer for SDL2 mouse events
    struct SDL2MouseEvent
    {
        Uint32 type;        // SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP, SDL_MOUSEMOTION, SDL_MOUSEWHEEL
        Int x;              // Mouse X position
        Int y;              // Mouse Y position
        Uint8 button;       // Button that was pressed/released
        Int wheelY;         // Wheel delta Y
        Uint32 timestamp;   // Event timestamp
    };

    SDL2MouseEvent m_eventBuffer[Mouse::NUM_MOUSE_EVENTS];
    UnsignedInt m_nextFreeIndex;
    UnsignedInt m_nextGetIndex;
    
    MouseCursor m_currentSDL2Cursor;
    Int m_directionFrame;
    Bool m_lostFocus;
    
    // SDL2 cursor resources
    SDL_Cursor* m_sdlCursors[NUM_MOUSE_CURSORS];
    
    // Translate SDL2 mouse event to our MouseIO format
    void translateEvent(UnsignedInt eventIndex, MouseIO* result);
    
    // Add an SDL2 event to the buffer
    void addSDL2Event(const SDL_Event& event);
    
    // Process SDL2 events
    void processSDL2Events(void);
};

/**
 * Factory function for creating mouse instances
 * Used by W3DGameClient::createMouse()
 */
Mouse* CreateMouse(void);

#endif // __SDL2_MOUSE_H__
