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

        case SDL_KEY_DOWN:
        case SDL_KEY_UP:
            return SDL_GetWindowFromID(event->key.windowID);

        case SDL_MOUSE_MOTION:
        case SDL_MOUSE_BUTTON_DOWN:
        case SDL_MOUSE_BUTTON_UP:
        case SDL_MOUSE_WHEEL:
            return SDL_GetWindowFromID(event->motion.windowID);

        default:
            return NULL;
    }
}

/* ============================================================================
 * INPUT TRANSLATION - Keycodes
 * ============================================================================ */

uint32_t SDL2_TranslateKeycode(SDL_Keycode sdl_keycode, SDL_Scancode scancode)
{
    /* Map SDL keycodes to Windows VK_* codes */
    switch (sdl_keycode) {
        /* Function keys */
        case SDLK_F1:        return VK_F1;
        case SDLK_F2:        return VK_F2;
        case SDLK_F3:        return VK_F3;
        case SDLK_F4:        return VK_F4;
        case SDLK_F5:        return VK_F5;
        case SDLK_F6:        return VK_F6;
        case SDLK_F7:        return VK_F7;
        case SDLK_F8:        return VK_F8;
        case SDLK_F9:        return VK_F9;
        case SDLK_F10:       return VK_F10;
        case SDLK_F11:       return VK_F11;
        case SDLK_F12:       return VK_F12;

        /* Special keys */
        case SDLK_ESCAPE:    return VK_ESCAPE;
        case SDLK_TAB:       return VK_TAB;
        case SDLK_RETURN:    return VK_RETURN;
        case SDLK_SPACE:     return VK_SPACE;

        /* Modifier keys */
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:    return VK_SHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:     return VK_CONTROL;
        case SDLK_LALT:
        case SDLK_RALT:      return VK_MENU;

        /* Navigation keys */
        case SDLK_UP:        return VK_UP;
        case SDLK_DOWN:      return VK_DOWN;
        case SDLK_LEFT:      return VK_LEFT;
        case SDLK_RIGHT:     return VK_RIGHT;
        case SDLK_HOME:      return VK_HOME;
        case SDLK_END:       return VK_END;
        case SDLK_PAGEUP:    return VK_PRIOR;
        case SDLK_PAGEDOWN:  return VK_NEXT;
        case SDLK_DELETE:    return VK_DELETE;

        /* Numeric and character keys - use scancode as fallback */
        default:
            /* For alphanumeric keys, SDL typically preserves ASCII values */
            if (sdl_keycode >= 'a' && sdl_keycode <= 'z') {
                return (sdl_keycode - 'a') + 'A';  /* Convert to uppercase VK_A, VK_B, etc. */
            }
            if (sdl_keycode >= '0' && sdl_keycode <= '9') {
                return sdl_keycode;  /* '0'-'9' are same in Win32 */
            }
            return sdl_keycode;  /* Return as-is for other characters */
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
    float fx, fy;
    uint32_t buttons = SDL_GetMouseState(&fx, &fy);

    if (x) {
        *x = (int)fx;
    }
    if (y) {
        *y = (int)fy;
    }

    return buttons;
}

void SDL2_SetMousePosition(SDL_Window* window, int x, int y)
{
    if (!window) {
        fprintf(stderr, "Phase 01: SDL2_SetMousePosition called with NULL window\n");
        return;
    }

    SDL_WarpMouseInWindow(window, (float)x, (float)y);
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
