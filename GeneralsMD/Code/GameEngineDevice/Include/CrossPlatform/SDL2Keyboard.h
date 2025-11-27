// ============================================================================
// SDL2 Keyboard Header
// Cross-platform keyboard input handling using SDL2
// ============================================================================

#pragma once

#ifndef __SDL2_KEYBOARD_H__
#define __SDL2_KEYBOARD_H__

#include "GameClient/Keyboard.h"

/**
 * SDL2Keyboard - Cross-platform keyboard input implementation
 * 
 * Provides keyboard input handling for macOS, Linux, and Windows
 * using SDL2 as the input backend instead of Windows DirectInput.
 */
class SDL2Keyboard : public Keyboard
{
public:
    SDL2Keyboard();
    virtual ~SDL2Keyboard();

    virtual void init(void) override;
    virtual void reset(void) override;
    virtual void update(void) override;
    virtual Bool getCapsState(void) override;

protected:
    virtual void getKey(KeyboardIO* key) override;

private:
    const unsigned char* m_sdlKeyState;
    int m_numKeys;

    unsigned char sdlKeyToGameKey(int sdlKey);
    void updateModifiers(void);
};

/**
 * Factory function for creating keyboard instances
 * Used by W3DGameClient::createKeyboard()
 */
Keyboard* CreateKeyboard(void);

#endif // __SDL2_KEYBOARD_H__
