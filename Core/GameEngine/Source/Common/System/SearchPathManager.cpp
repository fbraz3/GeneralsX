/*
**	Command & Conquer Generals Zero Hour(tm)
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

////////////////////////////////////////////////////////////////////////////////
//
//  SearchPathManager.cpp
//  Implementation of asset and file path discovery system
//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/SearchPathManager.h"
#include "Common/ConfigurationManager.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#define F_OK 0
	#define access _access
#else
	#include <unistd.h>
#endif

// Static instance data
static SearchPathManager::GameVariant g_gameVariant = SearchPathManager::VARIANT_GENERALS;
static AsciiString g_assetPath;
static AsciiString g_secondaryAssetPath;
static AsciiString g_installationBasePath;
static Bool g_initialized = FALSE;

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Expand %USERPROFILE% environment variable
 */
static AsciiString expandUserProfile()
{
	AsciiString result;

#ifdef _WIN32
	char buffer[MAX_PATH];
	if (GetEnvironmentVariableA("USERPROFILE", buffer, sizeof(buffer)))
	{
		result = buffer;
	}
	else
	{
		// Fallback to HOMEDRIVE + HOMEPATH
		char drive[MAX_PATH] = { 0 };
		char homepath[MAX_PATH] = { 0 };
		GetEnvironmentVariableA("HOMEDRIVE", drive, sizeof(drive));
		GetEnvironmentVariableA("HOMEPATH", homepath, sizeof(homepath));
		result.format("%s%s", drive, homepath);
	}
#else
	const char* homeEnv = getenv("HOME");
	if (homeEnv)
	{
		result = homeEnv;
	}
#endif

	return result;
}

/**
 * Check if a file exists
 */
static Bool fileExistsAtPath(const AsciiString& path)
{
	return (access(path.str(), F_OK) == 0);
}

/**
 * Combine two paths with proper separator
 */
static AsciiString combinePaths(const AsciiString& basePath, const AsciiString& subPath)
{
	AsciiString result;

#ifdef _WIN32
	const char sep = '\\';
#else
	const char sep = '/';
#endif

	// Check if basePath ends with separator
	if (basePath.getLength() > 0)
	{
		char lastChar = basePath.getCharAt(basePath.getLength()-1);
		if (lastChar != sep && lastChar != '/')
		{
			result.format("%s%c%s", basePath.str(), sep, subPath.str());
		}
		else
		{
			result.format("%s%s", basePath.str(), subPath.str());
		}
	}
	else
	{
		result.format("%s%s", basePath.str(), subPath.str());
	}

	return result;
}

/**
 * Normalize path slashes to platform-specific separator
 */
static AsciiString normalizePath(const AsciiString& path)
{
	// Windows and modern OSes handle both slash types fine
	// So we can return the path as-is
	return path;
}

// ============================================================================
// SearchPathManager Implementation
// ============================================================================

Bool SearchPathManager::init(GameVariant variant)
{
	g_gameVariant = variant;
	g_initialized = FALSE;

	// Get user profile base
	AsciiString userProfile = expandUserProfile();
	if (userProfile.isEmpty())
	{
		DEBUG_LOG(("SearchPathManager: Failed to get user profile directory\n"));
		return FALSE;
	}

	// Set installation base path
#ifdef _WIN32
	g_installationBasePath.format("%s\\GeneralsX", userProfile.str());
#else
	g_installationBasePath.format("%s/GeneralsX", userProfile.str());
#endif

	// Discover asset path
	g_assetPath = discoverAssetPath();
	if (g_assetPath.isEmpty())
	{
		DEBUG_LOG(("SearchPathManager: Failed to discover asset path\n"));
		return FALSE;
	}

	// For Zero Hour, also set secondary path to base Generals directory
	if (variant == VARIANT_ZERO_HOUR)
	{
#ifdef _WIN32
		g_secondaryAssetPath.format("%s\\Generals", g_installationBasePath.str());
#else
		g_secondaryAssetPath.format("%s/Generals", g_installationBasePath.str());
#endif
	}

	g_initialized = TRUE;
	logDiscoveredPaths();

	return TRUE;
}

AsciiString SearchPathManager::getAssetPath()
{
	if (!g_initialized)
	{
		return "";
	}
	return g_assetPath;
}

AsciiString SearchPathManager::getSecondaryAssetPath()
{
	if (!g_initialized)
	{
		return "";
	}
	return g_secondaryAssetPath;
}

AsciiString SearchPathManager::findAssetFile(const AsciiString& filename)
{
	if (!g_initialized)
	{
		return "";
	}

	// Try primary path first
	AsciiString primaryPath = combinePaths(g_assetPath, filename);
	if (fileExistsAtPath(primaryPath))
	{
		return primaryPath;
	}

	// Try secondary path if available
	if (!g_secondaryAssetPath.isEmpty())
	{
		AsciiString secondaryPath = combinePaths(g_secondaryAssetPath, filename);
		if (fileExistsAtPath(secondaryPath))
		{
			return secondaryPath;
		}
	}

	// File not found
	return "";
}

AsciiString SearchPathManager::getInstallationBasePath()
{
	return g_installationBasePath;
}

void SearchPathManager::logDiscoveredPaths()
{
	DEBUG_LOG(("SearchPathManager: Installation Base = %s\n", g_installationBasePath.str()));
	DEBUG_LOG(("SearchPathManager: Primary Asset Path = %s\n", g_assetPath.str()));
	if (!g_secondaryAssetPath.isEmpty())
	{
		DEBUG_LOG(("SearchPathManager: Secondary Asset Path = %s\n", g_secondaryAssetPath.str()));
	}
}

Bool SearchPathManager::isValidAssetPath(const AsciiString& path)
{
	if (path.isEmpty())
	{
		return FALSE;
	}

	// Check for recognizable game files that should be in asset directory
	// Look for "Data" subdirectory or key .big files
	AsciiString dataPath = combinePaths(path, "Data");
	if (fileExistsAtPath(dataPath))
	{
		return TRUE;
	}

	// Check for common .big files
	const char* commonFiles[] = {
		"main.big",
		"art.big",
		"generals.big",
		"english.big"
	};

	for (int i = 0; i < 4; i++)
	{
		AsciiString filePath = combinePaths(path, commonFiles[i]);
		if (fileExistsAtPath(filePath))
		{
			return TRUE;
		}
	}

	return FALSE;
}

AsciiString SearchPathManager::discoverAssetPath()
{
	// Try configuration override first
	AsciiString overridePath = getConfigOverridePath();
	if (!overridePath.isEmpty() && isValidAssetPath(overridePath))
	{
		DEBUG_LOG(("SearchPathManager: Using INI override path: %s\n", overridePath.str()));
		return normalizePath(overridePath);
	}

	// Get game-specific subdirectory name
	AsciiString gameSubdir = getGameSubdirectory(g_gameVariant);

	// Try standard installation locations
	AsciiString userProfile = expandUserProfile();
	if (!userProfile.isEmpty())
	{
#ifdef _WIN32
		AsciiString standardPath;
		standardPath.format("%s\\GeneralsX\\%s", userProfile.str(), gameSubdir.str());
#else
		AsciiString standardPath;
		standardPath.format("%s/GeneralsX/%s", userProfile.str(), gameSubdir.str());
#endif

		if (isValidAssetPath(standardPath))
		{
			DEBUG_LOG(("SearchPathManager: Found assets at standard location: %s\n", standardPath.str()));
			return normalizePath(standardPath);
		}
	}

	// Try current working directory as last resort
	char cwd[512];
	if (getcwd(cwd, sizeof(cwd)))
	{
		AsciiString cwdPath = cwd;
		if (isValidAssetPath(cwdPath))
		{
			DEBUG_LOG(("SearchPathManager: Found assets in current directory: %s\n", cwdPath.str()));
			return normalizePath(cwdPath);
		}
	}

	// If nothing found, return the standard path anyway (game will fail later with better error)
	AsciiString defaultPath;
	defaultPath.format("%s\\GeneralsX\\%s", userProfile.str(), gameSubdir.str());
	DEBUG_LOG(("SearchPathManager: Using default path (may not exist): %s\n", defaultPath.str()));
	return normalizePath(defaultPath);
}

AsciiString SearchPathManager::getConfigOverridePath()
{
	// Get override from ConfigurationManager if available
	AsciiString overridePath;
	ConfigurationManager::getString("Advanced", "AssetPath", overridePath);
	
	if (!overridePath.isEmpty())
	{
		DEBUG_LOG(("SearchPathManager: Found INI override: %s\n", overridePath.str()));
	}

	return overridePath;
}

AsciiString SearchPathManager::getGameSubdirectory(GameVariant variant)
{
	if (variant == VARIANT_ZERO_HOUR)
	{
		return "GeneralsMD";
	}
	else
	{
		return "Generals";
	}
}
