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

// GXD3DWin32.h - Shared Win32 DXVK probe, crash-marker persistence, and SDL3 HWND icon helpers.
// GeneralsX @refactor fbraz3 26/11/2026 Extract duplicated Win32 helpers from WinMain.cpp into Win32Device.

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cwchar>

#include "Win32Device/Common/GXD3DBackend.h"
#include "Win32Device/Common/GXVulkanCheck.h"

namespace gxwin32 {

inline void *vkOpen(const char *name)
{
	return reinterpret_cast<void *>(LoadLibraryA(name));
}

inline void *vkSymbol(void *library, const char *name)
{
	return reinterpret_cast<void *>(GetProcAddress(static_cast<HMODULE>(library), name));
}

inline void vkClose(void *library)
{
	FreeLibrary(static_cast<HMODULE>(library));
}

inline gxvk::Loader makeVulkanLoader()
{
	const gxvk::Loader loader = { vkOpen, vkSymbol, vkClose };
	return loader;
}

inline bool usesDxvkPair()
{
	wchar_t path[MAX_PATH];
	const DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
	if (n == 0 || n >= MAX_PATH)
		return false;
	wchar_t *slash = wcsrchr(path, L'\\');
	if (slash == nullptr)
		return false;
	const size_t room = MAX_PATH - static_cast<size_t>(slash + 1 - path);
	auto beside = [&](const wchar_t *leaf) {
		if (wcscpy_s(slash + 1, room, leaf) != 0)
			return false;
		const DWORD a = GetFileAttributesW(path);
		return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY) == 0;
	};
	return gxvk::usesDxvkPair(beside(L"d3d8.dll"), beside(L"d3d9.dll"));
}

inline GxD3DState readGraphicsState(const char *path)
{
	char text[512] = "";
	if (path && path[0] != '\0')
	{
		if (FILE *f = fopen(path, "r"))
		{
			text[fread(text, 1, sizeof(text) - 1, f)] = '\0';
			fclose(f);
		}
	}
	return gxD3DParseState(text);
}

inline bool writeGraphicsState(const char *path, int backend, int reason, GxD3DState st)
{
	char text[256];
	const int n = gxD3DFormatFile(text, sizeof(text), backend, reason, st);
	if (n <= 0 || n >= static_cast<int>(sizeof(text)) || !path || path[0] == '\0')
		return false;
	FILE *f = fopen(path, "w");
	if (f == nullptr)
		return false;
	const bool written = fputs(text, f) >= 0;
	return fclose(f) == 0 && written;
}

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

} // namespace gxwin32

#endif // _WIN32
