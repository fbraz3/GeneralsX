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
**  FILE: win32_sdl_api_compat.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: SDL2 Compatibility Layer Implementation
**
**  DESCRIPTION:
**    Implementation of Win32-compatible interfaces over SDL2.
**    Provides window creation, event handling, and input translation.
**
******************************************************************************/

#include "win32_sdl_api_compat.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * STATIC STATE - Global SDL2 state management
 * ============================================================================ */

static SDL_Window* g_primaryWindow = NULL;
static int g_vsyncEnabled = 1;
static int g_sdlInitialized = 0;

/* ============================================================================
 * SDL2 INITIALIZATION - Ensure SDL2 is initialized
 * ============================================================================ */

static void SDL2_EnsureInitialized(void)
{
    if (!g_sdlInitialized) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "Phase 01: SDL_Init failed: %s\n", SDL_GetError());
        } else {
            printf("Phase 01: SDL2 initialized successfully\n");
            g_sdlInitialized = 1;
        }
    }
}

/* ============================================================================
 * SDL2 WINDOW CREATION
 * ============================================================================ */

SDL_Window* SDL2_CreateWindow(
    const char* title,
    int x,
    int y,
    int width,
    int height,
    SDL_WindowFlags flags)
{
    SDL_Window* window;

    printf("Phase 01: SDL2_CreateWindow(%s, %d, %d, %d, %d, 0x%X)\n",
           title ? title : "untitled", x, y, width, height, flags);

    SDL2_EnsureInitialized();

    /* Create the SDL2 window */
    window = SDL_CreateWindow(
        title ? title : "GeneralsX",
        width,
        height,
        flags
    );

    if (!window) {
        fprintf(stderr, "Phase 01: SDL_CreateWindow failed: %s\n", SDL_GetError());
        return NULL;
    }

    /* Set window position if specified */
    if (x != SDL_WINDOWPOS_CENTERED && y != SDL_WINDOWPOS_CENTERED) {
        SDL_SetWindowPosition(window, x, y);
    }

    printf("Phase 01: SDL2 window created successfully\n");

    /* Store reference to primary window for global access */
    if (!g_primaryWindow) {
        g_primaryWindow = window;
    }

    return window;
}

void SDL2_DestroyWindow(SDL_Window* window)
{
    if (!window) {
        fprintf(stderr, "Phase 01: SDL2_DestroyWindow called with NULL window\n");
        return;
    }

    printf("Phase 01: SDL2_DestroyWindow called\n");

    if (window == g_primaryWindow) {
        g_primaryWindow = NULL;
    }

    SDL_DestroyWindow(window);
}

void SDL2_GetWindowSize(SDL_Window* window, int* width, int* height)
{
    if (!window || !width || !height) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }

    SDL_GetWindowSize(window, width, height);
}

void SDL2_SetWindowSize(SDL_Window* window, int width, int height)
{
    if (!window) {
        return;
    }

    printf("Phase 01: SDL2_SetWindowSize(%d, %d)\n", width, height);
    SDL_SetWindowSize(window, width, height);
}

void SDL2_SetFullscreen(SDL_Window* window, int fullscreen)
{
    if (!window) {
        return;
    }

    printf("Phase 01: SDL2_SetFullscreen(%s)\n", fullscreen ? "true" : "false");

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_TRUE);
    } else {
        SDL_SetWindowFullscreen(window, SDL_FALSE);
    }
}

void SDL2_SetVSync(int enabled)
{
    g_vsyncEnabled = enabled ? 1 : 0;
    printf("Phase 01: SDL2_SetVSync(%s)\n", enabled ? "enabled" : "disabled");
}

/* ============================================================================
 * SDL2 EVENT POLLING
 * ============================================================================ */

int SDL2_PollEvent(SDL_Event* event)
{
    if (!event) {
        return 0;
    }

    SDL2_EnsureInitialized();
    return SDL_PollEvent(event);
}

SDL_Window* SDL2_GetWindowFromEvent(const SDL_Event* event)
{
    if (!event) {
        return NULL;
    }

    /* Different event types store window ID in different locations */
    switch (event->type) {
        case SDL_WINDOW_OCCLUDED:
        case SDL_WINDOW_EXPOSED:
        case SDL_WINDOW_MOVED:
        case SDL_WINDOW_RESIZED:
        case SDL_WINDOW_MINIMIZED:
        case SDL_WINDOW_MAXIMIZED:
        case SDL_WINDOW_RESTORED:
        case SDL_WINDOW_MOUSE_ENTER:
        case SDL_WINDOW_MOUSE_LEAVE:
        case SDL_WINDOW_FOCUS_GAINED:
        case SDL_WINDOW_FOCUS_LOST:
            return SDL_GetWindowFromID(event->window.windowID);

        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return SDL_GetWindowFromID(event->key.windowID);

        case SDL_MOUSEMOTION:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEWHEEL:
            return SDL_GetWindowFromID(event->motion.windowID);

        default:
            return NULL;
    }
}

/* ============================================================================
 * INPUT TRANSLATION - Keycodes (SDL → Windows VK_*)
 * ============================================================================ */

/**
 * Enhanced SDL keycode to Windows VK_* translation with complete coverage
 * Handles all keys used by the game engine
 */
uint32_t SDL2_TranslateKeycode(SDL_Keycode sdl_keycode, SDL_Scancode scancode)
{
    /* Map SDL keycodes to Windows VK_* codes */
    switch (sdl_keycode) {
        /* Function keys F1-F12 */
        case SDLK_F1:        return VK_F1;      /* 0x70 */
        case SDLK_F2:        return VK_F2;      /* 0x71 */
        case SDLK_F3:        return VK_F3;      /* 0x72 */
        case SDLK_F4:        return VK_F4;      /* 0x73 */
        case SDLK_F5:        return VK_F5;      /* 0x74 */
        case SDLK_F6:        return VK_F6;      /* 0x75 */
        case SDLK_F7:        return VK_F7;      /* 0x76 */
        case SDLK_F8:        return VK_F8;      /* 0x77 */
        case SDLK_F9:        return VK_F9;      /* 0x78 */
        case SDLK_F10:       return VK_F10;     /* 0x79 */
        case SDLK_F11:       return VK_F11;     /* 0x7A */
        case SDLK_F12:       return VK_F12;     /* 0x7B */

        /* Special keys */
        case SDLK_ESCAPE:    return VK_ESCAPE;  /* 0x1B */
        case SDLK_TAB:       return VK_TAB;     /* 0x09 */
        case SDLK_RETURN:    return VK_RETURN;  /* 0x0D */
        case SDLK_BACKSPACE: return 0x08;       /* VK_BACK */
        case SDLK_SPACE:     return VK_SPACE;   /* 0x20 */

        /* Modifier keys - note: use specific left/right codes for better compatibility */
        case SDLK_LSHIFT:    return 0xA0;       /* VK_LSHIFT */
        case SDLK_RSHIFT:    return 0xA1;       /* VK_RSHIFT */
        case SDLK_LCTRL:     return 0xA2;       /* VK_LCONTROL */
        case SDLK_RCTRL:     return 0xA3;       /* VK_RCONTROL */
        case SDLK_LALT:      return 0xA4;       /* VK_LMENU */
        case SDLK_RALT:      return 0xA5;       /* VK_RMENU */

        /* Navigation keys */
        case SDLK_UP:        return VK_UP;      /* 0x26 */
        case SDLK_DOWN:      return VK_DOWN;    /* 0x28 */
        case SDLK_LEFT:      return VK_LEFT;    /* 0x25 */
        case SDLK_RIGHT:     return VK_RIGHT;   /* 0x27 */
        case SDLK_HOME:      return VK_HOME;    /* 0x24 */
        case SDLK_END:       return VK_END;     /* 0x23 */
        case SDLK_PAGEUP:    return VK_PRIOR;   /* 0x21 */
        case SDLK_PAGEDOWN:  return VK_NEXT;    /* 0x22 */
        case SDLK_INSERT:    return 0x2D;       /* VK_INSERT */
        case SDLK_DELETE:    return VK_DELETE;  /* 0x2E */

        /* Lock keys */
        case SDLK_CAPSLOCK:  return 0x14;       /* VK_CAPITAL */
        case SDLK_NUMLOCKCLEAR: return 0x90;    /* VK_NUMLOCK */
        case SDLK_SCROLLLOCK: return 0x91;      /* VK_SCROLL */

        /* Numeric keypad keys */
        case SDLK_KP_0:      return 0x60;       /* VK_NUMPAD0 */
        case SDLK_KP_1:      return 0x61;       /* VK_NUMPAD1 */
        case SDLK_KP_2:      return 0x62;       /* VK_NUMPAD2 */
        case SDLK_KP_3:      return 0x63;       /* VK_NUMPAD3 */
        case SDLK_KP_4:      return 0x64;       /* VK_NUMPAD4 */
        case SDLK_KP_5:      return 0x65;       /* VK_NUMPAD5 */
        case SDLK_KP_6:      return 0x66;       /* VK_NUMPAD6 */
        case SDLK_KP_7:      return 0x67;       /* VK_NUMPAD7 */
        case SDLK_KP_8:      return 0x68;       /* VK_NUMPAD8 */
        case SDLK_KP_9:      return 0x69;       /* VK_NUMPAD9 */
        case SDLK_KP_DECIMAL: return 0x6E;      /* VK_DECIMAL */
        case SDLK_KP_DIVIDE: return 0x6F;       /* VK_DIVIDE */
        case SDLK_KP_MULTIPLY: return 0x6A;     /* VK_MULTIPLY */
        case SDLK_KP_MINUS:  return 0x6D;       /* VK_SUBTRACT */
        case SDLK_KP_PLUS:   return 0x6B;       /* VK_ADD */
        case SDLK_KP_ENTER:  return 0x0D;       /* VK_RETURN (numpad enter) */

        /* Symbol keys */
        case SDLK_SEMICOLON: return 0xBA;       /* VK_OEM_1 (;:) */
        case SDLK_EQUALS:    return 0xBB;       /* VK_OEM_PLUS (=+) */
        case SDLK_COMMA:     return 0xBC;       /* VK_OEM_COMMA (,<) */
        case SDLK_MINUS:     return 0xBD;       /* VK_OEM_MINUS (-_) */
        case SDLK_PERIOD:    return 0xBE;       /* VK_OEM_PERIOD (.>) */
        case SDLK_SLASH:     return 0xBF;       /* VK_OEM_2 (/?  ) */
        case SDLK_BACKQUOTE: return 0xC0;       /* VK_OEM_3 (`~) */
        case SDLK_LEFTBRACKET: return 0xDB;     /* VK_OEM_4 ([{) */
        case SDLK_BACKSLASH: return 0xDC;       /* VK_OEM_5 (\|) */
        case SDLK_RIGHTBRACKET: return 0xDD;    /* VK_OEM_6 (]}) */
        case SDLK_QUOTE:     return 0xDE;       /* VK_OEM_7 ('") */

        /* Numeric and character keys - use scancode as fallback */
        default:
            /* For alphanumeric keys, SDL typically preserves ASCII values */
            if (sdl_keycode >= 'a' && sdl_keycode <= 'z') {
                /* Convert lowercase to uppercase ASCII (VK_A = 0x41 = 'A', etc.) */
                return (uint32_t)((sdl_keycode - 'a') + 'A');
            }
            if (sdl_keycode >= '0' && sdl_keycode <= '9') {
                /* Numeric keys: '0'-'9' are 0x30-0x39 in both SDL and Windows */
                return (uint32_t)sdl_keycode;
            }
            if (sdl_keycode >= 32 && sdl_keycode <= 126) {
                /* Printable ASCII range */
                return (uint32_t)sdl_keycode;
            }
            
            /* Fallback: use scancode-based translation for unknown keys */
            printf("Phase 03: Unknown SDL keycode 0x%X (scancode 0x%X), returning keycode as-is\n",
                   sdl_keycode, scancode);
            return (uint32_t)sdl_keycode;
    }
}

/* ============================================================================
 * INPUT TRANSLATION - Mouse buttons
 * ============================================================================ */

uint32_t SDL2_TranslateMouseButton(uint8_t sdl_button, int is_down)
{
    switch (sdl_button) {
        case SDL_BUTTON_LEFT:
            return is_down ? WM_LBUTTONDOWN : WM_LBUTTONUP;

        case SDL_BUTTON_MIDDLE:
            return is_down ? WM_MBUTTONDOWN : WM_MBUTTONUP;

        case SDL_BUTTON_RIGHT:
            return is_down ? WM_RBUTTONDOWN : WM_RBUTTONUP;

        case SDL_BUTTON_X1:
        case SDL_BUTTON_X2:
            /* Extended buttons (X1/X2 on some mice) */
            /* Translate to left button for now (TODO: add WM_XBUTTONDOWN/UP support) */
            return is_down ? WM_LBUTTONDOWN : WM_LBUTTONUP;

        default:
            return WM_MOUSEMOVE;
    }
}

/* ============================================================================
 * INPUT TRANSLATION - Mouse coordinates
 * ============================================================================ */

uint32_t SDL2_EncodeMouseCoords(int x, int y)
{
    /* Windows LPARAM format: LOWORD = x, HIWORD = y (signed 16-bit values) */
    uint16_t x_word = (uint16_t)x;
    uint16_t y_word = (uint16_t)y;
    return ((uint32_t)y_word << 16) | x_word;
}

void SDL2_DecodeMouseCoords(uint32_t lparam, int* x, int* y)
{
    if (x) {
        *x = (int)(int16_t)LOWORD(lparam);  /* Sign-extend to handle negative coords */
    }
    if (y) {
        *y = (int)(int16_t)HIWORD(lparam);  /* Sign-extend to handle negative coords */
    }
}

/* ============================================================================
 * KEYBOARD STATE
 * ============================================================================ */

int SDL2_IsKeyPressed(SDL_Scancode scancode)
{
    int numkeys;
    const uint8_t* keys = SDL_GetKeyboardState(&numkeys);

    if (!keys) {
        return 0;
    }

    if ((int)scancode < numkeys) {
        return keys[scancode];
    }

    return 0;
}

int SDL2_IsKeyModActive(SDL_Keymod mod_flag)
{
    SDL_Keymod current_mods = SDL_GetModState();
    return (current_mods & mod_flag) != 0;
}

/* ============================================================================
 * MOUSE STATE
 * ============================================================================ */

uint32_t SDL2_GetMousePosition(int* x, int* y)
{
    int ix, iy;
    uint32_t buttons = SDL_GetMouseState(&ix, &iy);

    if (x) {
        *x = ix;
    }
    if (y) {
        *y = iy;
    }

    return buttons;
}

void SDL2_SetMousePosition(SDL_Window* window, int x, int y)
{
    if (!window) {
        fprintf(stderr, "Phase 01: SDL2_SetMousePosition called with NULL window\n");
        return;
    }

    SDL_WarpMouseInWindow(window, x, y);
}

/* ============================================================================
 * SDL2 EVENT PROCESSING - Keyboard and mouse event conversion
 * ============================================================================ */

int SDL2_ProcessKeyboardEvent(
    const SDL_KeyboardEvent* sdl_event,
    uint32_t* out_msg,
    uint32_t* out_wparam,
    uint32_t* out_lparam)
{
    if (!sdl_event || !out_msg || !out_wparam || !out_lparam) {
        return 0;
    }

    /* Translate event type to Windows message */
    if (sdl_event->type == SDL_KEYDOWN) {
        *out_msg = 0x0100;  /* WM_KEYDOWN */
    } else if (sdl_event->type == SDL_KEYUP) {
        *out_msg = 0x0101;  /* WM_KEYUP */
    } else {
        return 0;
    }

    /* Translate keycode to Windows VK_* constant */
    *out_wparam = SDL2_TranslateKeycode(sdl_event->keysym.sym, sdl_event->keysym.scancode);

    /* Encode LPARAM: repeat count (0-15) + scan code (16-23) + extended flag (24) + reserved + context (29) + previous (30) + transition (31) */
    uint32_t repeat_count = (sdl_event->repeat) ? (sdl_event->repeat & 0xFFFF) : 1;
    uint32_t scan_code = (uint32_t)sdl_event->keysym.scancode & 0xFF;
    uint32_t extended = (scan_code > 0x53) ? 0x01000000 : 0;  /* Extended key flag */
    uint32_t previous = (sdl_event->type == SDL_KEYUP) ? 0 : 0x40000000;  /* Previous state */
    uint32_t transition = (sdl_event->type == SDL_KEYUP) ? 0x80000000 : 0;  /* Transition state */

    *out_lparam = repeat_count | (scan_code << 16) | extended | previous | transition;

    printf("Phase 03: SDL keyboard event - type:%s key:0x%X scan:0x%X → WM_KEY%s wParam:0x%X lParam:0x%X\n",
           sdl_event->type == SDL_KEYDOWN ? "DOWN" : "UP",
           sdl_event->keysym.sym,
           sdl_event->keysym.scancode,
           sdl_event->type == SDL_KEYDOWN ? "DOWN" : "UP",
           *out_wparam,
           *out_lparam);

    return 1;
}

uint32_t SDL2_GetModifierState(void)
{
    SDL_Keymod mods = SDL_GetModState();
    uint32_t key_state = 0;

    if (mods & SDL_KMOD_LCTRL)   key_state |= 0x0004;  /* KEY_STATE_LCONTROL */
    if (mods & SDL_KMOD_RCTRL)   key_state |= 0x0008;  /* KEY_STATE_RCONTROL */
    if (mods & SDL_KMOD_LSHIFT)  key_state |= 0x0010;  /* KEY_STATE_LSHIFT */
    if (mods & SDL_KMOD_RSHIFT)  key_state |= 0x0020;  /* KEY_STATE_RSHIFT */
    if (mods & SDL_KMOD_LALT)    key_state |= 0x0040;  /* KEY_STATE_LALT */
    if (mods & SDL_KMOD_RALT)    key_state |= 0x0080;  /* KEY_STATE_RALT */
    if (mods & SDL_KMOD_CAPS)    key_state |= 0x0200;  /* KEY_STATE_CAPSLOCK */

    return key_state;
}

/* ============================================================================
 * CLEANUP
 * ============================================================================ */

void SDL2_Cleanup(void)
{
    if (g_primaryWindow) {
        SDL_DestroyWindow(g_primaryWindow);
        g_primaryWindow = NULL;
    }

    if (g_sdlInitialized) {
        SDL_Quit();
        g_sdlInitialized = 0;
    }

    printf("Phase 01: SDL2 cleanup complete\n");
}
