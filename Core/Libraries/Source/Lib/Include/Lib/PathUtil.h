/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file contains macros and functions to help with path handling.

#pragma once

#include <algorithm>
#include <cctype>
#include <string>

inline Char getNativePathSeparator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

// Normalizes path separators to the host platform's native format.
inline void normalizePathSeparatorsToNative(std::string& path, size_t startOffset = 0)
{
	const char nativeSep = getNativePathSeparator();
	const char foreignSep = (nativeSep == '/') ? '\\' : '/';
	if (startOffset < path.size())
	{
		std::replace(path.begin() + startOffset, path.end(), foreignSep, nativeSep);
	}
}

// Normalizes path separators to standard portable network format (Windows backslashes).
inline void normalizePathSeparatorsToPortable(std::string& path, size_t startOffset = 0)
{
	if (startOffset < path.size())
	{
		std::replace(path.begin() + startOffset, path.end(), '/', '\\');
	}
}

// Returns the leaf directory and map file name: "dir/map.map" (normalized to POSIX separators).
inline std::string getMapDirAndFileName(const std::string& path)
{
	size_t lastSep = path.find_last_of("/\\");
	if (lastSep == std::string::npos)
		return path;
	size_t prevSep = (lastSep > 0) ? path.find_last_of("/\\", lastSep - 1) : std::string::npos;
	if (prevSep == std::string::npos)
		return path;
	std::string result = path.substr(prevSep + 1);
	for (char &c : result) {
		if (c == '\\') c = '/';
	}
	return result;
}

// Returns the logical map subpath relative to map roots (e.g. "UserData/Maps/", "Maps/UserMaps/", "Maps/").
inline std::string getLogicalMapSubpath(const std::string& normalizedPath)
{
	const std::string userDataPrefix = "userdata/maps/";
	if (normalizedPath.rfind(userDataPrefix, 0) == 0)
	{
		return normalizedPath.substr(userDataPrefix.length());
	}
	const std::string userMapsPrefix = "maps/usermaps/";
	if (normalizedPath.rfind(userMapsPrefix, 0) == 0)
	{
		return normalizedPath.substr(userMapsPrefix.length());
	}
	const std::string mapsPrefix = "maps/";
	if (normalizedPath.rfind(mapsPrefix, 0) == 0)
	{
		return normalizedPath.substr(mapsPrefix.length());
	}

	size_t mapsPos = normalizedPath.rfind("/maps/");
	if (mapsPos != std::string::npos)
	{
		return normalizedPath.substr(mapsPos + 6);
	}

	return normalizedPath;
}

// Validates whether the candidate map path matches the requested map name/path.
inline bool isMapPathMatching(const std::string& requested, const std::string& actual)
{
	if (requested.empty() || actual.empty())
		return false;
	if (requested == actual)
		return true;

	auto normalize = [](const std::string& input) -> std::string {
		std::string s = input;
		std::transform(s.begin(), s.end(), s.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		for (char &c : s) {
			if (c == '\\') c = '/';
		}
		while (s.length() > 1 && s.back() == '/') s.pop_back();
		return s;
	};

	std::string reqNorm = normalize(requested);
	std::string actNorm = normalize(actual);

	if (reqNorm == actNorm)
		return true;

	// Compare logical map subpaths relative to map root (e.g. "userdata/maps/", "maps/", or "/.../maps/").
	std::string reqSub = getLogicalMapSubpath(reqNorm);
	std::string actSub = getLogicalMapSubpath(actNorm);

	if (!reqSub.empty() && reqSub == actSub)
		return true;

	// Check path-boundary suffix matching if one path is a full filesystem path and the other is a subpath.
	if (actNorm.length() > reqNorm.length())
	{
		const size_t suffixPos = actNorm.length() - reqNorm.length();
		if (actNorm[suffixPos - 1] == '/' &&
			actNorm.compare(suffixPos, reqNorm.length(), reqNorm) == 0)
		{
			return true;
		}
	}
	else if (reqNorm.length() > actNorm.length())
	{
		const size_t suffixPos = reqNorm.length() - actNorm.length();
		if (reqNorm[suffixPos - 1] == '/' &&
			reqNorm.compare(suffixPos, actNorm.length(), actNorm) == 0)
		{
			return true;
		}
	}

	return false;
}

// Returns true for a separator the host platform uses to open files.
template <typename T> Bool isFileSystemPathSeparator(T ch)
{
#ifdef _WIN32
	return ch == '\\' || ch == '/';
#else
	return ch == '/';
#endif
}

// Returns true for either separator. Game data paths carry '\\' on every platform.
template <typename T> Bool isPathSeparator(T ch)
{
	return ch == '/' || ch == '\\';
}

// Returns whether the given path is an absolute file path.
template <typename T> Bool isAbsolutePath(const T* path)
{
	if (path == nullptr)
	{
		return false;
	}

	if (isFileSystemPathSeparator(path[0]))
	{
		return true;
	}

#ifdef _WIN32
	const bool hasDriveLetter =
		(path[0] >= 'A' && path[0] <= 'Z') ||
		(path[0] >= 'a' && path[0] <= 'z');

	if (hasDriveLetter && path[1] == ':' && isFileSystemPathSeparator(path[2]))
	{
		return true;
	}
#endif

	return false;
}

// Returns a pointer to the first path separator of the given path.
template <typename T> T* getFirstPathSeparator(T* path)
{
	for (; *path; ++path)
	{
		if (isPathSeparator(*path))
			return path;
	}
	return nullptr;
}

// Returns a pointer to the last path separator of the given path.
template <typename T> T* getLastPathSeparator(T* path)
{
	T* last = nullptr;
	for (; *path; ++path)
	{
		if (isPathSeparator(*path))
			last = path;
	}
	return last;
}

// Returns the file name after the last path separator or the whole path as is if none was found.
template <typename T> T* getFileName(T* path)
{
	T* lastSeparator = getLastPathSeparator(path);
	return lastSeparator ? lastSeparator + 1 : path;
}

// Returns null when the file name has no extension, ignoring dots in directory names.
template <typename T> T* getExtension(T* path)
{
	T* lastDot = nullptr;
	for (; *path; ++path)
	{
		if (*path == '.')
			lastDot = path;
		else if (isPathSeparator(*path))
			lastDot = nullptr;
	}
	return lastDot;
}
