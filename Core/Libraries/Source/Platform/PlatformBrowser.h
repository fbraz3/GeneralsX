/*
**	Command & Conquer(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// PlatformBrowser.h - Cross-platform default web browser dispatch
// GeneralsX @feature fbraz3 27/09/2026 Platform abstraction for browser launching

#pragma once

#if defined(SAGE_USE_SDL3)
#include <SDL3/SDL.h>
#elif defined(_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

namespace Platform {

inline bool OpenBrowserURL(const char *url)
{
	if (!url || url[0] == '\0') {
		return false;
	}
#if defined(SAGE_USE_SDL3)
	return (SDL_OpenURL(url) == 0);
#elif defined(_WIN32)
	HINSTANCE hInst = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
	return (reinterpret_cast<intptr_t>(hInst) > 32);
#else
	return false;
#endif
}

inline bool CanOpenBrowser()
{
#if defined(SAGE_USE_SDL3) || defined(_WIN32)
	return true;
#else
	return false;
#endif
}

} // namespace Platform
