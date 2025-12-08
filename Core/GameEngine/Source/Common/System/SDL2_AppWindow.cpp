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
**  FILE: SDL2_AppWindow.cpp
**
**  PURPOSE: Cross-platform application window implementation via SDL2
**
**  DESCRIPTION:
**    Replaces Win32 HWND ApplicationHWnd with SDL2 window pointer
**    Provides window initialization and shutdown functions
**    Phase 40: Complete SDL2 Migration & Win32 API Removal
**
******************************************************************************/

#include "../../../Include/Common/System/SDL2_AppWindow.h"
#include <cstdio>
#include <cstring>
#include <string>

// Phase 40: Platform-specific headers for SDL2_GetModuleFilePath
// Note: <mach-o/dyld.h> is already included via msvc_types_compat.h (line 35)
// so we only need to include missing headers here

#ifdef __linux__
#include <unistd.h>       // for readlink
#endif

#ifdef _WIN32
#include <windows.h>      // for GetModuleFileName
#endif

/**
 * Global application window pointer
 * Initialized by SDL2_InitApplicationWindow()
 * Used by all game code that previously used HWND ApplicationHWnd
 */
SDL_Window* g_applicationWindow = NULL;

bool SDL2_InitApplicationWindow(const char* title, int width, int height)
{
    // Ensure SDL2 is initialized
    if ((SDL_WasInit(SDL_INIT_VIDEO)) == 0) {
        printf("SDL2_InitApplicationWindow: SDL video subsystem not initialized\n");
        return false;
    }
    
    // Check if window already exists
    if (g_applicationWindow != NULL) {
        printf("SDL2_InitApplicationWindow: Window already initialized\n");
        return false;
    }
    
    // Create main window
    g_applicationWindow = SDL_CreateWindow(
        title ? title : "Command & Conquer Generals Zero Hour",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width ? width : 1024,
        height ? height : 768,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (g_applicationWindow == NULL) {
        printf("SDL2_InitApplicationWindow: Failed to create window: %s\n", SDL_GetError());
        return false;
    }
    
    printf("SDL2_InitApplicationWindow: Window created (%dx%d): %s\n", width, height, title);
    return true;
}

void SDL2_ShutdownApplicationWindow(void)
{
    if (g_applicationWindow != NULL) {
        SDL_DestroyWindow(g_applicationWindow);
        g_applicationWindow = NULL;
        printf("SDL2_ShutdownApplicationWindow: Window destroyed\n");
    }
}

SDL_Window* SDL2_GetApplicationWindow(void)
{
    return g_applicationWindow;
}

size_t SDL2_GetModuleFilePath(char* buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        printf("SDL2_GetModuleFilePath: Invalid buffer or size\n");
        return 0;
    }
    
#ifdef __APPLE__
    // macOS: Use _NSGetExecutablePath
    uint32_t buflen = (uint32_t)size;
    if (_NSGetExecutablePath(buffer, &buflen) == 0) {
        printf("SDL2_GetModuleFilePath (macOS): %s\n", buffer);
        return strlen(buffer);
    } else {
        printf("SDL2_GetModuleFilePath (macOS): Failed to get executable path\n");
        return 0;
    }
    
#elif defined(__linux__)
    // Linux: Use readlink on /proc/self/exe
    ssize_t result = readlink("/proc/self/exe", buffer, size - 1);
    if (result > 0 && result < (ssize_t)size) {
        buffer[result] = '\0';
        printf("SDL2_GetModuleFilePath (Linux): %s\n", buffer);
        return (size_t)result;
    } else {
        printf("SDL2_GetModuleFilePath (Linux): Failed to read /proc/self/exe\n");
        return 0;
    }
    
#elif defined(_WIN32)
    // Windows: Use native GetModuleFileName
    DWORD result = GetModuleFileName(NULL, buffer, (DWORD)size);
    if (result > 0 && result < (DWORD)size) {
        printf("SDL2_GetModuleFilePath (Windows): %s\n", buffer);
        return (size_t)result;
    } else {
        printf("SDL2_GetModuleFilePath (Windows): Failed to get module filename\n");
        return 0;
    }
    
#else
    // Fallback for unknown platforms
    printf("SDL2_GetModuleFilePath: Unsupported platform\n");
    buffer[0] = '\0';
    return 0;
#endif
}

