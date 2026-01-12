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

#pragma once

#include "GameClient/Keyboard.h"
#include <SDL2/SDL.h>

// Forward declarations
class GameMessage;

/**
 * SDL2Keyboard - Keyboard input class for SDL2-based systems
 *
 * Provides keyboard event handling for Windows, macOS, and Linux using SDL2.
 * Converts SDL2 keyboard events (SDL_KEYDOWN, SDL_KEYUP) to engine KeyboardIO
 * format with proper key code translation and modifier tracking.
 *
 * Key Features:
 * - Full keyboard event handling from SDL2
 * - Key code translation from SDL2 to engine KEY_* codes
 * - Modifier key tracking (Shift, Ctrl, Alt, Caps Lock)
 * - Key repeat detection
 * - Text input support via TEXT_INPUT events (handled separately by IME manager)
 * - Proper state machine for key up/down transitions
 */
class SDL2Keyboard
{
public:
    SDL2Keyboard();
    virtual ~SDL2Keyboard();

    /**
     * Handle SDL2 key down event
     * @param event SDL_KeyboardEvent with KEYDOWN type
     */
    void onKeyDown(const SDL_KeyboardEvent &event);

    /**
     * Handle SDL2 key up event
     * @param event SDL_KeyboardEvent with KEYUP type
     */
    void onKeyUp(const SDL_KeyboardEvent &event);

    /**
     * Get current modifier flags (Shift, Ctrl, Alt, Caps Lock)
     * @return Combination of KEY_STATE_* flags
     */
    UnsignedShort getModifierFlags() const;

    /**
     * Check if shift key is currently pressed
     * @return TRUE if either Shift key is down
     */
    Bool isShiftDown() const;

    /**
     * Check if control key is currently pressed
     * @return TRUE if either Ctrl key is down
     */
    Bool isCtrlDown() const;

    /**
     * Check if alt key is currently pressed
     * @return TRUE if either Alt key is down
     */
    Bool isAltDown() const;

    /**
     * Check if caps lock is active
     * @return TRUE if Caps Lock LED is on
     */
    Bool getCapsLockState() const;

    /**
     * Check if a specific key is currently down
     * @param keyCode Engine KEY_* code to check
     * @return TRUE if key is currently pressed
     */
    Bool isKeyDown(KeyDefType keyCode) const;

private:
    // Keyboard state tracking
    UnsignedByte m_keyStates[KEY_COUNT];  ///< Current state for each key (KEY_STATE_* flags)
    UnsignedShort m_modifiers;            ///< Current modifier flags (Shift, Ctrl, Alt, Caps)
    UnsignedInt m_keyDownTime[KEY_COUNT]; ///< Timestamp (SDL_GetTicks) when each key went down

    /**
     * Convert SDL2 keycode to engine KEY_* constant
     * @param scancode SDL_Scancode value
     * @return KeyDefType (engine key code), or KEY_NONE if not mapped
     */
    KeyDefType sdlScancodeToGameKey(SDL_Scancode scancode) const;

    /**
     * Update modifier flags based on current SDL keyboard state
     * This reads SDL_GetModState() and updates m_modifiers accordingly
     */
    void updateModifiers();

    /**
     * Get the current KEY_STATE_* flags for a given key down event
     * Includes modifier states and other flags
     * @param event SDL_KeyboardEvent to extract state from
     * @return Combination of KEY_STATE_* flags
     */
    UnsignedShort getKeyStateFromEvent(const SDL_KeyboardEvent &event) const;
};

// Global instance
extern SDL2Keyboard *TheSDL2Keyboard;

#endif  // SDL2KEYBOARD_H
