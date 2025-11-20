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
**  FILE: SDL2_AppWindow.h
**
**  PURPOSE: Cross-platform application window management via SDL2
**
**  DESCRIPTION:
**    Provides global SDL2_Window pointer replacement for Win32 HWND
**    This header replaces HWND ApplicationHWnd with SDL2 equivalent.
**    Phase 40: Complete SDL2 Migration & Win32 API Removal
**
**  USAGE:
**    extern SDL_Window* g_applicationWindow;
**    
**    // Show message box instead of Win32 MessageBox
**    SDL_ShowSimpleMessageBox(
**        SDL_MESSAGEBOX_WARNING,
**        "Warning Title",
**        "Message text",
**        g_applicationWindow
**    );
**
******************************************************************************/

#pragma once

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Global application window pointer
 * Replaces Win32 HWND ApplicationHWnd
 * 
 * Set by main application initialization (WinMain or equivalent)
 * Used by game code for window operations and dialogs
 */
extern SDL_Window* g_applicationWindow;

/**
 * Initialize the global application window
 * Called once during game startup after SDL_Init()
 * 
 * @param title     Window title
 * @param width     Initial window width
 * @param height    Initial window height
 * @return          true if successful, false otherwise
 */
bool SDL2_InitApplicationWindow(const char* title, int width, int height);

/**
 * Shutdown the global application window
 * Called during game shutdown, before SDL_Quit()
 */
void SDL2_ShutdownApplicationWindow(void);

/**
 * Get current application window
 * @return          Pointer to SDL_Window or NULL if not initialized
 */
SDL_Window* SDL2_GetApplicationWindow(void);

/**
 * Get the application executable path (SDL2 replacement for GetModuleFileName)
 * 
 * On macOS: Uses _NSGetExecutablePath()
 * On Linux: Uses readlink("/proc/self/exe")
 * On Windows: Uses GetModuleFileName() directly
 * 
 * @param buffer    Buffer to store path (should be at least MAX_PATH or 4096 bytes)
 * @param size      Size of buffer in bytes
 * @return          Number of bytes written to buffer (excluding null terminator), 
 *                  or 0 if error occurred
 */
size_t SDL2_GetModuleFilePath(char* buffer, size_t size);

#ifdef __cplusplus
}
#endif
