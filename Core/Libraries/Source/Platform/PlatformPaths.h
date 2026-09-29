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
// GeneralsX @refactor felipebraz 26/09/2026 Encapsulate font probing and file accessibility in platform layer

#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <io.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace Platform {

inline bool IsFileReadable(const char *path)
{
	if ( !path || path[0] == '\0' ) {
		return false;
	}
#if defined(_WIN32)
	return ( _access( path, 4 ) == 0 );
#else
	return ( access( path, R_OK ) == 0 );
#endif
}

inline bool GetMacOSBundleFontDirectories(char *resFonts, size_t resFontsSize,
                                          char *binFonts, size_t binFontsSize)
{
#if defined(__APPLE__)
	char rawExecPath[1024] = {0};
	uint32_t bufSize = sizeof(rawExecPath);
	if ( _NSGetExecutablePath( rawExecPath, &bufSize ) != 0 ) {
		return false;
	}

	char realExecPath[1024] = {0};
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
#else
	(void)resFonts;
	(void)resFontsSize;
	(void)binFonts;
	(void)binFontsSize;
	return false;
#endif
}

inline bool FindLocalFontFile(const char *const *candidates, int candidateCount,
                              char *outPath, size_t outPathSize)
{
	if ( !candidates || candidateCount <= 0 || !outPath || outPathSize == 0 ) {
		return false;
	}

	const char *searchDirs[24];
	int searchDirCount = 0;

	char envBundleFonts[512] = {0};
	const char *gxBundleFonts = getenv( "GX_BUNDLE_FONTS" );
	if ( gxBundleFonts && gxBundleFonts[0] != '\0' ) {
		snprintf( envBundleFonts, sizeof(envBundleFonts), "%s", gxBundleFonts );
		searchDirs[searchDirCount++] = envBundleFonts;
	}

#if defined(__APPLE__)
	char macosResFonts[512] = {0};
	char macosBinFonts[512] = {0};
	if ( GetMacOSBundleFontDirectories( macosResFonts, sizeof(macosResFonts),
	                                    macosBinFonts, sizeof(macosBinFonts) ) ) {
		if ( IsFileReadable( macosResFonts ) && searchDirCount < 24 ) {
			searchDirs[searchDirCount++] = macosResFonts;
		}
		if ( IsFileReadable( macosBinFonts ) && searchDirCount < 24 ) {
			searchDirs[searchDirCount++] = macosBinFonts;
		}
	}
#elif defined(_WIN32)
	// GeneralsX @bugfix felipebraz 29/09/2026 Probe fonts directory relative to Windows executable
	char winExeFonts[MAX_PATH] = {0};
	char winExeAssetsFonts[MAX_PATH] = {0};
	char winExeDir[MAX_PATH] = {0};
	if ( GetModuleFileNameA( NULL, winExeDir, MAX_PATH ) > 0 ) {
		char *lastBackslash = strrchr( winExeDir, '\\' );
		char *lastSlash = strrchr( winExeDir, '/' );
		char *sep = (lastBackslash > lastSlash) ? lastBackslash : lastSlash;
		if ( sep != nullptr ) {
			*sep = '\0';
			snprintf( winExeFonts, sizeof(winExeFonts), "%s\\fonts", winExeDir );
			snprintf( winExeAssetsFonts, sizeof(winExeAssetsFonts), "%s\\assets\\fonts", winExeDir );
			if ( searchDirCount < 24 ) {
				searchDirs[searchDirCount++] = winExeFonts;
			}
			if ( searchDirCount < 24 ) {
				searchDirs[searchDirCount++] = winExeAssetsFonts;
			}
		}
	}
#endif

	if ( searchDirCount < 24 ) searchDirs[searchDirCount++] = "fonts";
	if ( searchDirCount < 24 ) searchDirs[searchDirCount++] = "./fonts";
	if ( searchDirCount < 24 ) searchDirs[searchDirCount++] = "../Resources/fonts";
	if ( searchDirCount < 24 ) searchDirs[searchDirCount++] = "Resources/fonts";
	if ( searchDirCount < 24 ) searchDirs[searchDirCount++] = "assets/fonts";

	char envZhFonts[512] = {0};
	const char *zhPath = getenv( "CNC_GENERALS_ZH_PATH" );
	if ( zhPath && zhPath[0] != '\0' && searchDirCount < 24 ) {
		snprintf( envZhFonts, sizeof(envZhFonts), "%s/fonts", zhPath );
		searchDirs[searchDirCount++] = envZhFonts;
	}

	char envGenFonts[512] = {0};
	const char *genPath = getenv( "CNC_GENERALS_PATH" );
	if ( genPath && genPath[0] != '\0' && searchDirCount < 24 ) {
		snprintf( envGenFonts, sizeof(envGenFonts), "%s/fonts", genPath );
		searchDirs[searchDirCount++] = envGenFonts;
	}

	char homeZhFonts[512] = {0};
	char homeGenFonts[512] = {0};
	const char *homePath = getenv( "HOME" );
	if ( homePath && homePath[0] != '\0' ) {
		if ( searchDirCount < 24 ) {
			snprintf( homeZhFonts, sizeof(homeZhFonts), "%s/GeneralsX/GeneralsZH/fonts", homePath );
			searchDirs[searchDirCount++] = homeZhFonts;
		}
		if ( searchDirCount < 24 ) {
			snprintf( homeGenFonts, sizeof(homeGenFonts), "%s/GeneralsX/Generals/fonts", homePath );
			searchDirs[searchDirCount++] = homeGenFonts;
		}
	}

	if ( searchDirCount < 24 ) {
		searchDirs[searchDirCount++] = "/app/share/fonts";
	}
	if ( searchDirCount < 24 ) {
		searchDirs[searchDirCount++] = "/usr/share/fonts/truetype/liberation";
	}

	static const char *extensions[] = { ".ttf", ".otf", ".ttc" };
	char candidatePath[1024];

	for ( int d = 0; d < searchDirCount; ++d ) {
		for ( int c = 0; c < candidateCount; ++c ) {
			const char *cand = candidates[c];
			if ( !cand || cand[0] == '\0' ) {
				continue;
			}
			if ( strrchr( cand, '.' ) != nullptr ) {
				snprintf( candidatePath, sizeof(candidatePath), "%s/%s", searchDirs[d], cand );
				if ( IsFileReadable( candidatePath ) ) {
					snprintf( outPath, outPathSize, "%s", candidatePath );
					return true;
				}
			} else {
				for ( size_t e = 0; e < sizeof(extensions) / sizeof(extensions[0]); ++e ) {
					snprintf( candidatePath, sizeof(candidatePath), "%s/%s%s",
					          searchDirs[d], cand, extensions[e] );
					if ( IsFileReadable( candidatePath ) ) {
						snprintf( outPath, outPathSize, "%s", candidatePath );
						return true;
					}
				}
			}
		}
	}

	return false;
}

} // namespace Platform
