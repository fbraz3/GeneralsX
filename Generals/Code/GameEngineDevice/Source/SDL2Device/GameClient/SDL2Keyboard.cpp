/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//  SDL2 Keyboard Input Implementation
//  Cross-platform keyboard input handling using SDL2
////////////////////////////////////////////////////////////////////////////////

#include "SDL2Device/GameClient/SDL2Keyboard.h"
#include "GameClient/KeyDefs.h"
#include <SDL2/SDL.h>
#include <cstring>

// Global instance
SDL2Keyboard *TheSDL2Keyboard = nullptr;

// ============================================================================
// Constructor / Destructor
// ============================================================================

SDL2Keyboard::SDL2Keyboard()
    : m_modifiers(KEY_STATE_NONE)
{
    std::memset(m_keyStates, KEY_STATE_UP, sizeof(m_keyStates));
    std::memset(m_keyDownTime, 0, sizeof(m_keyDownTime));
    
    // Check initial Caps Lock state
    SDL_Keymod mod = SDL_GetModState();
    if (mod & KMOD_CAPS) {
        m_modifiers |= KEY_STATE_CAPSLOCK;
    }
}

SDL2Keyboard::~SDL2Keyboard()
{
    // Cleanup
}

// ============================================================================
// Event Handlers
// ============================================================================

void SDL2Keyboard::onKeyDown(const SDL_KeyboardEvent &event)
{
    // Sanity check
    if (event.type != SDL_KEYDOWN) {
        return;
    }

    // Convert SDL scancode to engine key code
    KeyDefType gameKey = sdlScancodeToGameKey(event.keysym.scancode);

    // Ignore unmapped keys
    if (gameKey == KEY_NONE) {
        return;
    }

    // Get current state with modifiers
    UnsignedShort keyState = getKeyStateFromEvent(event);
    keyState |= KEY_STATE_DOWN;

    // If this is a repeat (already down), add auto-repeat flag
    if (m_keyStates[gameKey] & KEY_STATE_DOWN) {
        keyState |= KEY_STATE_AUTOREPEAT;
    }

    // Update key state and timestamp
    m_keyStates[gameKey] = keyState;
    m_keyDownTime[gameKey] = event.timestamp;

    // Update modifiers for special keys
    updateModifiers();
}

void SDL2Keyboard::onKeyUp(const SDL_KeyboardEvent &event)
{
    // Sanity check
    if (event.type != SDL_KEYUP) {
        return;
    }

    // Convert SDL scancode to engine key code
    KeyDefType gameKey = sdlScancodeToGameKey(event.keysym.scancode);

    // Ignore unmapped keys
    if (gameKey == KEY_NONE) {
        return;
    }

    // Get current state without the DOWN flag
    UnsignedShort keyState = getKeyStateFromEvent(event);
    keyState &= ~KEY_STATE_DOWN;  // Clear DOWN flag
    keyState |= KEY_STATE_UP;     // Set UP flag

    // Update key state
    m_keyStates[gameKey] = keyState;
    m_keyDownTime[gameKey] = 0;

    // Update modifiers for special keys
    updateModifiers();
}

// ============================================================================
// Modifier Tracking
// ============================================================================

UnsignedShort SDL2Keyboard::getModifierFlags() const
{
    return m_modifiers;
}

Bool SDL2Keyboard::isShiftDown() const
{
    return (m_modifiers & (KEY_STATE_LSHIFT | KEY_STATE_RSHIFT)) != 0;
}

Bool SDL2Keyboard::isCtrlDown() const
{
    return (m_modifiers & (KEY_STATE_LCONTROL | KEY_STATE_RCONTROL)) != 0;
}

Bool SDL2Keyboard::isAltDown() const
{
    return (m_modifiers & (KEY_STATE_LALT | KEY_STATE_RALT)) != 0;
}

Bool SDL2Keyboard::getCapsLockState() const
{
    return (m_modifiers & KEY_STATE_CAPSLOCK) != 0;
}

Bool SDL2Keyboard::isKeyDown(KeyDefType keyCode) const
{
    if (keyCode >= KEY_COUNT) {
        return FALSE;
    }
    return (m_keyStates[keyCode] & KEY_STATE_DOWN) != 0;
}

void SDL2Keyboard::updateModifiers()
{
    SDL_Keymod modState = SDL_GetModState();

    // Clear modifier flags
    m_modifiers &= ~(KEY_STATE_LSHIFT | KEY_STATE_RSHIFT | 
                     KEY_STATE_LCONTROL | KEY_STATE_RCONTROL | 
                     KEY_STATE_LALT | KEY_STATE_RALT);

    // Set modifier flags based on SDL state
    if (modState & KMOD_LSHIFT) {
        m_modifiers |= KEY_STATE_LSHIFT;
    }
    if (modState & KMOD_RSHIFT) {
        m_modifiers |= KEY_STATE_RSHIFT;
    }
    if (modState & KMOD_LCTRL) {
        m_modifiers |= KEY_STATE_LCONTROL;
    }
    if (modState & KMOD_RCTRL) {
        m_modifiers |= KEY_STATE_RCONTROL;
    }
    if (modState & KMOD_LALT) {
        m_modifiers |= KEY_STATE_LALT;
    }
    if (modState & KMOD_RALT) {
        m_modifiers |= KEY_STATE_RALT;
    }

    // Caps Lock state
    if (modState & KMOD_CAPS) {
        m_modifiers |= KEY_STATE_CAPSLOCK;
    } else {
        m_modifiers &= ~KEY_STATE_CAPSLOCK;
    }
}

// ============================================================================
// Key Code Translation
// ============================================================================

KeyDefType SDL2Keyboard::sdlScancodeToGameKey(SDL_Scancode scancode) const
{
    // Map SDL scancodes to engine KEY_* constants
    // SDL scancodes are hardware-independent, unlike keycodes
    
    switch (scancode) {
        // Function keys
        case SDL_SCANCODE_ESCAPE:       return KEY_ESC;
        case SDL_SCANCODE_F1:           return KEY_F1;
        case SDL_SCANCODE_F2:           return KEY_F2;
        case SDL_SCANCODE_F3:           return KEY_F3;
        case SDL_SCANCODE_F4:           return KEY_F4;
        case SDL_SCANCODE_F5:           return KEY_F5;
        case SDL_SCANCODE_F6:           return KEY_F6;
        case SDL_SCANCODE_F7:           return KEY_F7;
        case SDL_SCANCODE_F8:           return KEY_F8;
        case SDL_SCANCODE_F9:           return KEY_F9;
        case SDL_SCANCODE_F10:          return KEY_F10;
        case SDL_SCANCODE_F11:          return KEY_F11;
        case SDL_SCANCODE_F12:          return KEY_F12;

        // Number keys (1-9, 0)
        case SDL_SCANCODE_1:            return KEY_1;
        case SDL_SCANCODE_2:            return KEY_2;
        case SDL_SCANCODE_3:            return KEY_3;
        case SDL_SCANCODE_4:            return KEY_4;
        case SDL_SCANCODE_5:            return KEY_5;
        case SDL_SCANCODE_6:            return KEY_6;
        case SDL_SCANCODE_7:            return KEY_7;
        case SDL_SCANCODE_8:            return KEY_8;
        case SDL_SCANCODE_9:            return KEY_9;
        case SDL_SCANCODE_0:            return KEY_0;

        // Letter keys (A-Z)
        case SDL_SCANCODE_A:            return KEY_A;
        case SDL_SCANCODE_B:            return KEY_B;
        case SDL_SCANCODE_C:            return KEY_C;
        case SDL_SCANCODE_D:            return KEY_D;
        case SDL_SCANCODE_E:            return KEY_E;
        case SDL_SCANCODE_F:            return KEY_F;
        case SDL_SCANCODE_G:            return KEY_G;
        case SDL_SCANCODE_H:            return KEY_H;
        case SDL_SCANCODE_I:            return KEY_I;
        case SDL_SCANCODE_J:            return KEY_J;
        case SDL_SCANCODE_K:            return KEY_K;
        case SDL_SCANCODE_L:            return KEY_L;
        case SDL_SCANCODE_M:            return KEY_M;
        case SDL_SCANCODE_N:            return KEY_N;
        case SDL_SCANCODE_O:            return KEY_O;
        case SDL_SCANCODE_P:            return KEY_P;
        case SDL_SCANCODE_Q:            return KEY_Q;
        case SDL_SCANCODE_R:            return KEY_R;
        case SDL_SCANCODE_S:            return KEY_S;
        case SDL_SCANCODE_T:            return KEY_T;
        case SDL_SCANCODE_U:            return KEY_U;
        case SDL_SCANCODE_V:            return KEY_V;
        case SDL_SCANCODE_W:            return KEY_W;
        case SDL_SCANCODE_X:            return KEY_X;
        case SDL_SCANCODE_Y:            return KEY_Y;
        case SDL_SCANCODE_Z:            return KEY_Z;

        // Symbol/punctuation keys
        case SDL_SCANCODE_MINUS:        return KEY_MINUS;
        case SDL_SCANCODE_EQUALS:       return KEY_EQUAL;
        case SDL_SCANCODE_LEFTBRACKET:  return KEY_LBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return KEY_RBRACKET;
        case SDL_SCANCODE_BACKSLASH:    return KEY_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON:    return KEY_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE:   return KEY_APOSTROPHE;
        case SDL_SCANCODE_GRAVE:        return KEY_TICK;
        case SDL_SCANCODE_COMMA:        return KEY_COMMA;
        case SDL_SCANCODE_PERIOD:       return KEY_PERIOD;
        case SDL_SCANCODE_SLASH:        return KEY_SLASH;

        // Whitespace/control keys
        case SDL_SCANCODE_SPACE:        return KEY_SPACE;
        case SDL_SCANCODE_TAB:          return KEY_TAB;
        case SDL_SCANCODE_RETURN:       return KEY_ENTER;
        case SDL_SCANCODE_BACKSPACE:    return KEY_BACKSPACE;

        // Modifier keys
        case SDL_SCANCODE_LCTRL:        return KEY_LCTRL;
        case SDL_SCANCODE_RCTRL:        return KEY_RCTRL;
        case SDL_SCANCODE_LSHIFT:       return KEY_LSHIFT;
        case SDL_SCANCODE_RSHIFT:       return KEY_RSHIFT;
        case SDL_SCANCODE_LALT:         return KEY_LALT;
        case SDL_SCANCODE_RALT:         return KEY_RALT;
        case SDL_SCANCODE_CAPSLOCK:     return KEY_CAPS;

        // Lock keys
        case SDL_SCANCODE_NUMLOCKCLEAR: return KEY_NUM;
        case SDL_SCANCODE_SCROLLLOCK:   return KEY_SCROLL;

        // Arrow keys
        case SDL_SCANCODE_UP:           return KEY_UP;
        case SDL_SCANCODE_DOWN:         return KEY_DOWN;
        case SDL_SCANCODE_LEFT:         return KEY_LEFT;
        case SDL_SCANCODE_RIGHT:        return KEY_RIGHT;

        // Navigation keys
        case SDL_SCANCODE_INSERT:       return KEY_INS;
        case SDL_SCANCODE_DELETE:       return KEY_DEL;
        case SDL_SCANCODE_HOME:         return KEY_HOME;
        case SDL_SCANCODE_END:          return KEY_END;
        case SDL_SCANCODE_PAGEUP:       return KEY_PGUP;
        case SDL_SCANCODE_PAGEDOWN:     return KEY_PGDN;

        // Numpad keys
        case SDL_SCANCODE_KP_0:         return KEY_KP0;
        case SDL_SCANCODE_KP_1:         return KEY_KP1;
        case SDL_SCANCODE_KP_2:         return KEY_KP2;
        case SDL_SCANCODE_KP_3:         return KEY_KP3;
        case SDL_SCANCODE_KP_4:         return KEY_KP4;
        case SDL_SCANCODE_KP_5:         return KEY_KP5;
        case SDL_SCANCODE_KP_6:         return KEY_KP6;
        case SDL_SCANCODE_KP_7:         return KEY_KP7;
        case SDL_SCANCODE_KP_8:         return KEY_KP8;
        case SDL_SCANCODE_KP_9:         return KEY_KP9;
        case SDL_SCANCODE_KP_PLUS:      return KEY_KPPLUS;
        case SDL_SCANCODE_KP_MINUS:     return KEY_KPMINUS;
        case SDL_SCANCODE_KP_MULTIPLY:  return KEY_KPSTAR;
        case SDL_SCANCODE_KP_DIVIDE:    return KEY_KPSLASH;
        case SDL_SCANCODE_KP_PERIOD:    return KEY_KPDEL;
        case SDL_SCANCODE_KP_ENTER:     return KEY_KPENTER;

        // Special keys
        case SDL_SCANCODE_SYSREQ:       return KEY_SYSREQ;
        case SDL_SCANCODE_INTERNATIONAL2: return KEY_102;  // OEM_102 key (<> | on UK/Germany keyboards)

        // Japanese keyboard keys
        case SDL_SCANCODE_LANG1:        return KEY_CONVERT;    // Hiragana/Katakana
        case SDL_SCANCODE_LANG2:        return KEY_NOCONVERT;  // Henkan/Non-henkan
        case SDL_SCANCODE_INTERNATIONAL4: return KEY_KANJI;    // Kanji mode
        case SDL_SCANCODE_INTERNATIONAL5: return KEY_KANA;     // Kana mode
        case SDL_SCANCODE_INTERNATIONAL6: return KEY_CIRCUMFLEX; // Circumflex
        case SDL_SCANCODE_INTERNATIONAL0: return KEY_YEN;      // Yen sign

        // Unmapped or unknown
        default:
            return KEY_NONE;
    }
}

UnsignedShort SDL2Keyboard::getKeyStateFromEvent(const SDL_KeyboardEvent &event) const
{
    UnsignedShort state = KEY_STATE_NONE;

    // Start with the base state
    if (event.type == SDL_KEYDOWN) {
        state |= KEY_STATE_DOWN;
        if (event.repeat) {
            state |= KEY_STATE_AUTOREPEAT;
        }
    } else if (event.type == SDL_KEYUP) {
        state |= KEY_STATE_UP;
    }

    // Add modifier information from the event
    // Note: We also update m_modifiers separately, but include it here for completeness
    SDL_Keymod modifiers = event.keysym.mod;

    if (modifiers & KMOD_LSHIFT) {
        state |= KEY_STATE_LSHIFT;
    }
    if (modifiers & KMOD_RSHIFT) {
        state |= KEY_STATE_RSHIFT;
    }
    if (modifiers & KMOD_LCTRL) {
        state |= KEY_STATE_LCONTROL;
    }
    if (modifiers & KMOD_RCTRL) {
        state |= KEY_STATE_RCONTROL;
    }
    if (modifiers & KMOD_LALT) {
        state |= KEY_STATE_LALT;
    }
    if (modifiers & KMOD_RALT) {
        state |= KEY_STATE_RALT;
    }
    if (modifiers & KMOD_CAPS) {
        state |= KEY_STATE_CAPSLOCK;
    }

    return state;
}
