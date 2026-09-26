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

// PlatformPaths.h - Cross-platform executable and bundle path resolution
// GeneralsX @feature felipebraz 26/09/2026 Platform abstraction for bundle paths

#pragma once

#include <cstddef>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

namespace Platform {

inline bool GetMacOSBundleFontDirectories(char *resFonts, size_t resFontsSize,
                                          char *binFonts, size_t binFontsSize)
{
	char rawExecPath[PATH_MAX] = {0};
	uint32_t bufSize = sizeof(rawExecPath);
	if ( _NSGetExecutablePath( rawExecPath, &bufSize ) != 0 ) {
		return false;
	}

	char realExecPath[PATH_MAX] = {0};
	if ( realpath( rawExecPath, realExecPath ) == nullptr ) {
		return false;
	}

	char *lastSlash = strrchr( realExecPath, '/' );
	if ( lastSlash == nullptr ) {
		return false;
	}
	*lastSlash = '\0';

	if ( resFonts && resFontsSize > 0 ) {
		snprintf( resFonts, resFontsSize, "%s/../Resources/fonts", realExecPath );
	}
	if ( binFonts && binFontsSize > 0 ) {
		snprintf( binFonts, binFontsSize, "%s/../fonts", realExecPath );
	}
	return true;
}

} // namespace Platform

#else // !__APPLE__

namespace Platform {

inline bool GetMacOSBundleFontDirectories(char *resFonts, size_t resFontsSize,
                                          char *binFonts, size_t binFontsSize)
{
	(void)resFonts;
	(void)resFontsSize;
	(void)binFonts;
	(void)binFontsSize;
	return false;
}

} // namespace Platform

#endif
