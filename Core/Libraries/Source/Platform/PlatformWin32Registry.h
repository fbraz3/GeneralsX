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

// PlatformWin32Registry.h - Win32 registry lookup with 32-bit WOW64 fallback on x64.
// GeneralsX @refactor fbraz3 26/11/2026 Isolate Win32 WOW64 registry fallback in platform layer.

#pragma once

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstddef>
#include <cstdint>

namespace Platform {

inline bool QueryWin32RegistryValue(HKEY root, const char *path, const char *key, DWORD samDesired, unsigned char *buffer, DWORD &inOutSize, DWORD &outType)
{
	if (!path || !key || !buffer || inOutSize == 0)
	{
		return false;
	}

	HKEY handle = nullptr;
	if (RegOpenKeyExA(root, path, 0, samDesired, &handle) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD size = inOutSize;
	DWORD type = 0;
	const LSTATUS queryStatus = RegQueryValueExA(handle, key, nullptr, &type, buffer, &size);
	RegCloseKey(handle);
	if (queryStatus != ERROR_SUCCESS)
	{
		return false;
	}

	inOutSize = size;
	outType = type;
	return true;
}

inline bool ReadWin32RegistryString(HKEY root, const char *path, const char *key, char *outBuf, size_t outBufSize)
{
	if (!outBuf || outBufSize == 0)
	{
		return false;
	}

	auto tryRead = [&](DWORD samDesired) -> bool {
		// GeneralsX @bugfix fbraz3 27/11/2026 Advertise full buffer capacity; verify string type and accept exact-fit only when NUL-terminated.
		DWORD size = static_cast<DWORD>(outBufSize);
		DWORD type = 0;
		if (!QueryWin32RegistryValue(root, path, key, samDesired, reinterpret_cast<unsigned char *>(outBuf), size, type))
		{
			return false;
		}
		if (type != REG_SZ && type != REG_EXPAND_SZ)
		{
			return false;
		}
		if (size > outBufSize)
		{
			return false;
		}
		if (size == outBufSize)
		{
			return outBuf[outBufSize - 1] == '\0';
		}
		outBuf[size] = '\0';
		return true;
	};

	if (tryRead(KEY_READ))
	{
		return true;
	}

#ifdef _WIN64
	// GeneralsX @bugfix fbraz3 26/11/2026 Retry full open+query under WOW64 32-bit view when native key lacks the value.
	return tryRead(KEY_READ | KEY_WOW64_32KEY);
#else
	return false;
#endif
}

inline bool ReadWin32RegistryUInt32(HKEY root, const char *path, const char *key, uint32_t &outVal)
{
	auto tryRead = [&](DWORD samDesired) -> bool {
		uint32_t raw = 0;
		DWORD size = sizeof(raw);
		DWORD type = 0;
		if (!QueryWin32RegistryValue(root, path, key, samDesired, reinterpret_cast<unsigned char *>(&raw), size, type))
		{
			return false;
		}
		if (type != REG_DWORD || size != sizeof(raw))
		{
			return false;
		}
		outVal = raw;
		return true;
	};

	if (tryRead(KEY_READ))
	{
		return true;
	}

#ifdef _WIN64
	return tryRead(KEY_READ | KEY_WOW64_32KEY);
#else
	return false;
#endif
}

} // namespace Platform

#endif // _WIN32
