/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// GXD3DWin32.h - Shared Win32 SDL3 HWND/icon helpers and embedded d3d8to9 graphics backend setup.
// GeneralsX @refactor fbraz3 26/11/2026 Isolate Win32 windowing and d3d8to9 backend wiring in Win32Device.

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <cstdio>

#ifdef SAGE_USE_SDL3
#include <SDL3/SDL.h>
#endif

#ifdef GX_COMPAT_D3D9
#include "gx_d3d8to9.h"
#include "WW3D2/dx8wrapper.h"
#endif

namespace gxwin32 {

inline void attachWindowIcon(HWND hWnd, HINSTANCE hInstance, WORD iconResourceId)
{
	if (!hWnd || !hInstance)
		return;
	HICON appIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iconResourceId));
	if (appIcon)
	{
		SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(appIcon));
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(appIcon));
	}
}

#ifdef SAGE_USE_SDL3
// Create the SDL3 window on Windows, extract its native Win32 HWND, and attach the application icon.
inline bool initializeSDL3Window(HINSTANCE hInstance, const char *title, WORD iconResourceId, SDL_Window *&outWindow, HWND &outHwnd)
{
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		fprintf(stderr, "FATAL: SDL_InitSubSystem(SDL_INIT_VIDEO) failed: %s\n", SDL_GetError());
		return false;
	}

	outWindow = SDL_CreateWindow(title, 1024, 768, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	if (!outWindow)
	{
		fprintf(stderr, "FATAL: SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return false;
	}

	const SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
	if (primaryDisplay != 0)
	{
		const int centered = SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay);
		if (!SDL_SetWindowPosition(outWindow, centered, centered))
		{
			fprintf(stderr, "WARNING: SDL_SetWindowPosition(primary display) failed: %s\n", SDL_GetError());
		}
	}

	outHwnd = static_cast<HWND>(SDL_GetPointerProperty(
		SDL_GetWindowProperties(outWindow),
		SDL_PROP_WINDOW_WIN32_HWND_POINTER,
		nullptr));
	if (!outHwnd)
	{
		fprintf(stderr, "FATAL: Failed to extract Win32 HWND from SDL3 window\n");
		SDL_DestroyWindow(outWindow);
		outWindow = nullptr;
		SDL_Quit();
		return false;
	}

	attachWindowIcon(outHwnd, hInstance, iconResourceId);

	fprintf(stderr, "INFO: SDL3 window initialized cleanly (SDL_Window=%p, HWND=%p)\n",
		static_cast<void *>(outWindow), static_cast<void *>(outHwnd));
	return true;
}

inline void shutdownSDL3Window(SDL_Window *&window, HWND &hwnd)
{
	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	hwnd = nullptr;
	SDL_Quit();
}
#endif

#ifdef GX_COMPAT_D3D9
inline IDirect3D8 *WINAPI compatDirect3DCreate8(UINT sdk)
{
	char why[256] = "";
	IDirect3D8 *const d = static_cast<IDirect3D8 *>(gx_d3d8to9_Direct3DCreate8(sdk, why, sizeof(why)));
	fprintf(stderr, "[GXGFX] d3d9=%ls%s%s\n", gx_d3d8to9_LoadedD3D9Path(), d != nullptr ? "" : " FAILED: ", d != nullptr ? "" : why);
	fflush(stderr);
	return d;
}
#endif

// Install embedded d3d8to9 (Direct3D 9) graphics translation on Windows x64.
inline void installD3D8To9Backend()
{
#ifdef GX_COMPAT_D3D9
	DX8Wrapper::Set_Direct3DCreate8_Override(compatDirect3DCreate8, "d3d8to9");
	fprintf(stderr, "[GXGFX] backend=d3d8to9\n");
	fflush(stderr);
#endif
}

} // namespace gxwin32

#endif // _WIN32
