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

// SDL3SplashScreen.h - Cross-platform SDL3 startup splash screen (Install_Final.bmp).
// GeneralsX @feature fbraz3 27/11/2026 Show Install_Final.bmp in a borderless SDL3 window until W3DDisplay opens.

#pragma once

#ifdef SAGE_USE_SDL3

#include <SDL3/SDL.h>

namespace gx {
namespace SDL3SplashScreen {

inline SDL_Window *&windowSlot()
{
	static SDL_Window *s_splashWindow = nullptr;
	return s_splashWindow;
}

inline void dismiss()
{
	SDL_Window *&win = windowSlot();
	if (win != nullptr)
	{
		SDL_DestroyWindow(win);
		win = nullptr;
	}
}

inline void show(const char *title, const char *bmpPath = "Install_Final.bmp")
{
	if (windowSlot() != nullptr || !bmpPath || bmpPath[0] == '\0')
	{
		return;
	}

	SDL_Surface *bmp = SDL_LoadBMP(bmpPath);
#ifndef _WIN32
	if (bmp == nullptr)
	{
		bmp = SDL_LoadBMP("install_final.bmp");
	}
#endif
	if (bmp == nullptr)
	{
		return;
	}

	const Uint32 flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_NOT_FOCUSABLE;
	SDL_Window *win = SDL_CreateWindow(title ? title : "Command & Conquer Generals", bmp->w, bmp->h, flags);
	if (win == nullptr)
	{
		SDL_DestroySurface(bmp);
		return;
	}

	const SDL_DisplayID primaryDisplay = SDL_GetPrimaryDisplay();
	if (primaryDisplay != 0)
	{
		const int centered = SDL_WINDOWPOS_CENTERED_DISPLAY(primaryDisplay);
		SDL_SetWindowPosition(win, centered, centered);
	}

	SDL_ShowWindow(win);
	SDL_Surface *winSurface = SDL_GetWindowSurface(win);
	const bool drawn = (winSurface != nullptr)
		&& SDL_BlitSurface(bmp, nullptr, winSurface, nullptr)
		&& SDL_UpdateWindowSurface(win);
	SDL_DestroySurface(bmp);
	if (!drawn)
	{
		SDL_DestroyWindow(win);
		return;
	}

	SDL_PumpEvents();
	windowSlot() = win;
}

} // namespace SDL3SplashScreen
} // namespace gx

#endif // SAGE_USE_SDL3
