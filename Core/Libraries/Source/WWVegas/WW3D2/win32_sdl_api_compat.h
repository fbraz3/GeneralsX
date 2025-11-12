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

/******************************************************************************
**
**  FILE: win32_sdl_api_compat.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: SDL2 Compatibility Layer for Windows API
**
**  DESCRIPTION:
**    This header provides Win32-compatible interfaces over SDL2 for
**    cross-platform window creation, event handling, and input processing.
**    It allows existing game code that uses Windows API to work on
**    macOS, Linux, and other SDL2-supported platforms.
**
**  USAGE:
**    1. Include this header in code that needs SDL2 window/event handling
**    2. Use SDL2_CreateWindow() to create windows
**    3. Use SDL2_PollEvent() to poll for events
**    4. Use SDL2_TranslateKeycode() to translate input events
**
**  ARCHITECTURE:
**    Phase 01: SDL2 Window & Event Loop
**    - Layer 1: Core Compatibility (this file)
**    - Layer 2: Platform Backend (SDL2, Metal, OpenGL)
**    - Layer 3: Game-Specific Extensions (GeneralsMD/Code/)
**
******************************************************************************/

#ifndef WIN32_SDL_API_COMPAT_H
#define WIN32_SDL_API_COMPAT_H

#include <stdint.h>
#include <SDL3/SDL.h>

/* ============================================================================
 * SDL2 WINDOW CREATION - Win32-compatible interface
 * ============================================================================ */

/**
 * SDL2-based window creation with Win32 parameter compatibility
 * 
 * @param title     Window title (UTF-8 string)
 * @param x         Initial X position (SDL_WINDOWPOS_CENTERED for center)
 * @param y         Initial Y position (SDL_WINDOWPOS_CENTERED for center)
 * @param width     Window width in pixels
 * @param height    Window height in pixels
 * @param flags     SDL_WindowFlags for window properties
 * @return          SDL_Window* cast to void* for HWND compatibility
 */
SDL_Window* SDL2_CreateWindow(
    const char* title,
    int x,
    int y,
    int width,
    int height,
    SDL_WindowFlags flags
);

/**
 * Destroy an SDL2 window created with SDL2_CreateWindow
 * 
 * @param window    SDL_Window* (cast from HWND)
 */
void SDL2_DestroyWindow(SDL_Window* window);

/**
 * Get window size for SDL2 windows
 * 
 * @param window    SDL_Window* (cast from HWND)
 * @param width     Pointer to store width
 * @param height    Pointer to store height
 */
void SDL2_GetWindowSize(SDL_Window* window, int* width, int* height);

/**
 * Set window size for SDL2 windows
 * 
 * @param window    SDL_Window* (cast from HWND)
 * @param width     New width in pixels
 * @param height    New height in pixels
 */
void SDL2_SetWindowSize(SDL_Window* window, int width, int height);

/**
 * Toggle fullscreen mode for SDL2 windows
 * 
 * @param window        SDL_Window* (cast from HWND)
 * @param fullscreen    true for fullscreen, false for windowed
 */
void SDL2_SetFullscreen(SDL_Window* window, int fullscreen);

/**
 * Enable/disable VSync for SDL2 windows
 * 
 * @param enabled   1 to enable VSync, 0 to disable
 */
void SDL2_SetVSync(int enabled);

/* ============================================================================
 * SDL2 EVENT POLLING - SDL event handling with translation
 * ============================================================================ */

/**
 * Poll next SDL2 event
 * 
 * @param event     Pointer to SDL_Event structure to fill
 * @return          1 if event was retrieved, 0 if queue is empty
 */
int SDL2_PollEvent(SDL_Event* event);

/**
 * Get SDL2 window from event (for multi-window support)
 * 
 * @param event     SDL_Event that occurred
 * @return          SDL_Window* for the event's window, or NULL
 */
SDL_Window* SDL2_GetWindowFromEvent(const SDL_Event* event);

/* ============================================================================
 * INPUT TRANSLATION - SDL2 key codes to Windows VK_* constants
 * ============================================================================ */

/* ============================================================================
 * INPUT TRANSLATION - SDL2 key codes to Windows VK_* constants
 * ============================================================================ */

/**
 * Translate SDL key code to Windows virtual key code (VK_*)
 * 
 * @param sdl_keycode   SDL_Keycode from SDL_KeyboardEvent.keysym.sym
 * @param scancode      SDL_Scancode for fallback mapping
 * @return              Windows VK_* constant (e.g., VK_ESCAPE, VK_RETURN)
 *
 * EXAMPLES:
 *   SDL_SCANCODE_ESCAPE  → VK_ESCAPE (0x1B)
 *   SDL_SCANCODE_RETURN  → VK_RETURN (0x0D)
 *   SDL_SCANCODE_UP      → VK_UP     (0x26)
 *   SDL_SCANCODE_DOWN    → VK_DOWN   (0x28)
 *   SDL_SCANCODE_LEFT    → VK_LEFT   (0x25)
 *   SDL_SCANCODE_RIGHT   → VK_RIGHT  (0x27)
 *   SDL_SCANCODE_A       → VK_A      (0x41)
 *   SDL_SCANCODE_LSHIFT  → VK_LSHIFT (0xA0)
 *
 * KEYBOARD COVERAGE:
 *   - Function keys: F1-F12
 *   - Special keys: ESC, TAB, RETURN, BACKSPACE, SPACE
 *   - Modifier keys: SHIFT (left/right), CTRL (left/right), ALT (left/right)
 *   - Navigation: UP, DOWN, LEFT, RIGHT, HOME, END, PAGEUP, PAGEDOWN
 *   - Editing: INSERT, DELETE
 *   - Lock keys: CAPSLOCK, NUMLOCK, SCROLLLOCK
 *   - Numeric keypad: All 0-9, +, -, *, /, DECIMAL, ENTER
 *   - Symbols: ;:, =+, ,<, -_, .>, /?, `~, [{, \|, ]}, '"
 *   - Alphanumeric: A-Z, 0-9
 */
uint32_t SDL2_TranslateKeycode(SDL_Keycode sdl_keycode, SDL_Scancode scancode);

/**
 * Translate SDL mouse button to Windows WM_* message constant
 * 
 * @param sdl_button    SDL_MouseButton from SDL_MouseButtonEvent.button
 * @param is_down       1 for button down, 0 for button up
 * @return              Windows WM_* message constant
 *
 * EXAMPLES:
 *   SDL_BUTTON_LEFT + down   → WM_LBUTTONDOWN   (0x0201)
 *   SDL_BUTTON_LEFT + up     → WM_LBUTTONUP     (0x0202)
 *   SDL_BUTTON_RIGHT + down  → WM_RBUTTONDOWN   (0x0204)
 *   SDL_BUTTON_MIDDLE + down → WM_MBUTTONDOWN   (0x0207)
 */
uint32_t SDL2_TranslateMouseButton(uint8_t sdl_button, int is_down);

/**
 * Convert SDL mouse position to Windows LPARAM format
 * Encodes x,y coordinates as LOWORD(x) and HIWORD(y)
 * 
 * @param x             Mouse X coordinate
 * @param y             Mouse Y coordinate
 * @return              LPARAM-encoded coordinates
 *
 * USAGE in Win32 emulation:
 *   int x = SDL_event.motion.x;
 *   int y = SDL_event.motion.y;
 *   LPARAM lParam = SDL2_EncodeMouseCoords(x, y);
 *   int decoded_x = (int)(short)LOWORD(lParam);
 *   int decoded_y = (int)(short)HIWORD(lParam);
 */
uint32_t SDL2_EncodeMouseCoords(int x, int y);

/**
 * Decode mouse coordinates from Windows LPARAM format
 * 
 * @param lparam        LPARAM value containing encoded coordinates
 * @param x             Pointer to store decoded X coordinate
 * @param y             Pointer to store decoded Y coordinate
 */
void SDL2_DecodeMouseCoords(uint32_t lparam, int* x, int* y);

/* ============================================================================
 * SDL2 KEYBOARD STATE - Query key states
 * ============================================================================ */

/**
 * Check if a key is currently pressed
 * 
 * @param scancode      SDL_Scancode for the key to check
 * @return              1 if key is pressed, 0 if not
 */
int SDL2_IsKeyPressed(SDL_Scancode scancode);

/**
 * Check if a key modifier is active (Shift, Ctrl, Alt, etc.)
 * 
 * @param mod_flag      SDL_Keymod flag (SDL_KMOD_SHIFT, SDL_KMOD_CTRL, etc.)
 * @return              1 if modifier is active, 0 if not
 */
int SDL2_IsKeyModActive(SDL_Keymod mod_flag);

/* ============================================================================
 * SDL2 MOUSE STATE - Query mouse state
 * ============================================================================ */

/**
 * Get current mouse position
 * 
 * @param x             Pointer to store X coordinate
 * @param y             Pointer to store Y coordinate
 * @return              SDL_MouseButtonFlags indicating which buttons are pressed
 */
uint32_t SDL2_GetMousePosition(int* x, int* y);

/**
 * Set mouse position (warp cursor)
 * 
 * @param window        SDL_Window* for the window context
 * @param x             Target X coordinate
 * @param y             Target Y coordinate
 */
void SDL2_SetMousePosition(SDL_Window* window, int x, int y);

/* ============================================================================
 * SDL2 EVENT PROCESSING - Helper functions for game input integration
 * ============================================================================ */

/**
 * Process SDL2 keyboard event into Win32-compatible message parameters
 * Translates SDL_KeyboardEvent to WM_KEYDOWN/WM_KEYUP with proper wParam/lParam
 * 
 * @param sdl_event     SDL_KeyboardEvent to process
 * @param out_msg       Output WM_KEYDOWN or WM_KEYUP constant
 * @param out_wparam    Output virtual key code (VK_*)
 * @param out_lparam    Output repeat count and scan code
 * @return              1 if event was translated, 0 if not
 *
 * USAGE:
 *   uint32_t msg, wparam, lparam;
 *   if (SDL2_ProcessKeyboardEvent(&event.key, &msg, &wparam, &lparam)) {
 *       TheWin32Keyboard->addWin32Event(msg, wparam, lparam);
 *   }
 */
int SDL2_ProcessKeyboardEvent(
    const SDL_KeyboardEvent* sdl_event,
    uint32_t* out_msg,
    uint32_t* out_wparam,
    uint32_t* out_lparam
);

/**
 * Get SDL2 keyboard modifier state as Windows-compatible bitmask
 * Translates SDL_GetModState() to Win32 modifier flags
 * 
 * @return              Bitmask of KEY_STATE_* flags
 *
 * MAPPING:
 *   SDL_KMOD_LCTRL → KEY_STATE_LCONTROL
 *   SDL_KMOD_RCTRL → KEY_STATE_RCONTROL
 *   SDL_KMOD_LSHIFT → KEY_STATE_LSHIFT
 *   SDL_KMOD_RSHIFT → KEY_STATE_RSHIFT
 *   SDL_KMOD_LALT → KEY_STATE_LALT
 *   SDL_KMOD_RALT → KEY_STATE_RALT
 *   SDL_KMOD_CAPS → KEY_STATE_CAPSLOCK
 */
uint32_t SDL2_GetModifierState(void);

/* ============================================================================
 * WINDOWS API COMPATIBILITY - Win32-style constants and types
 * ============================================================================ */

/* Virtual Key Codes (VK_*) - for SDL2_TranslateKeycode() output */
#ifndef VK_ESCAPE
#define VK_ESCAPE       0x1B
#define VK_TAB          0x09
#define VK_RETURN       0x0D
#define VK_SHIFT        0x10
#define VK_CONTROL      0x11
#define VK_MENU         0x12  /* ALT key */
#define VK_SPACE        0x20
#define VK_PRIOR        0x21  /* Page Up */
#define VK_NEXT         0x22  /* Page Down */
#define VK_END          0x23
#define VK_HOME         0x24
#define VK_LEFT         0x25
#define VK_UP           0x26
#define VK_RIGHT        0x27
#define VK_DOWN         0x28
#define VK_DELETE       0x2E
#define VK_F1           0x70
#define VK_F2           0x71
#define VK_F3           0x72
#define VK_F4           0x73
#define VK_F5           0x74
#define VK_F6           0x75
#define VK_F7           0x76
#define VK_F8           0x77
#define VK_F9           0x78
#define VK_F10          0x79
#define VK_F11          0x7A
#define VK_F12          0x7B
#endif

/* Windows Message Constants (WM_*) - for SDL2_TranslateMouseButton() output */
#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MBUTTONDBLCLK    0x0209
#define WM_MOUSEWHEEL       0x020A
#define WM_MOUSEMOVE        0x0200
#endif

/* Macro for encoding/decoding LPARAM mouse coordinates */
#define LOWORD(l)      ((uint16_t)(l & 0xFFFF))
#define HIWORD(l)      ((uint16_t)((l >> 16) & 0xFFFF))
#define MAKELPARAM(lo, hi) (((uint32_t)(hi) << 16) | ((uint32_t)(lo) & 0xFFFF))

/* SDL Window Flags for compatibility with CreateWindowEx */
#define SDL2_WINDOW_FULLSCREEN    SDL_WINDOW_FULLSCREEN
#define SDL2_WINDOW_SHOWN         SDL_WINDOW_SHOWN
#define SDL2_WINDOW_HIDDEN        SDL_WINDOW_HIDDEN
#define SDL2_WINDOW_RESIZABLE     SDL_WINDOW_RESIZABLE
#define SDL2_WINDOW_BORDERLESS    SDL_WINDOW_BORDERLESS

#endif /* WIN32_SDL_API_COMPAT_H */
