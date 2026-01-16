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
//  SearchPathManager.h
//  Asset and file path discovery system
//  Automatically finds game data directories and assets
//  Author: GeneralsX Development Team
//
//  Purpose:
//    - Locate asset directories without requiring exe to be in data folder
//    - Support multiple search paths (home directory, install locations, etc.)
//    - Allow override via INI configuration
//    - Work cross-platform (Windows, macOS, Linux via Wine)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __SEARCHPATHMANAGER_H__
#define __SEARCHPATHMANAGER_H__

#include "Common/AsciiString.h"

/**
 * Search Path Manager - handles file and asset path discovery
 * 
 * Automatically discovers game asset locations in standard directories:
 * - %USERPROFILE%\GeneralsX\GeneralsMD  (Zero Hour assets)
 * - %USERPROFILE%\GeneralsX\Generals    (Generals base game assets)
 * 
 * Can be overridden via INI file [Advanced] section:
 * - AssetPath = /custom/path/to/assets
 */
class SearchPathManager
{
public:
	enum GameVariant
	{
		VARIANT_GENERALS,       ///< Command & Conquer Generals (base game)
		VARIANT_ZERO_HOUR       ///< Command & Conquer Generals Zero Hour (expansion)
	};

	/**
	 * Initialize path discovery system
	 * @param variant - Which game variant (Generals or Zero Hour)
	 * @return TRUE if at least one valid asset path found
	 */
	static Bool init(GameVariant variant);

	/**
	 * Get primary asset search path
	 * Priority order:
	 *   1. INI [Advanced] AssetPath override
	 *   2. %USERPROFILE%\GeneralsX\GeneralsMD (for ZH)
	 *   3. %USERPROFILE%\GeneralsX\Generals (for base game)
	 *   4. Current working directory
	 * @return Full path to asset directory
	 */
	static AsciiString getAssetPath();

	/**
	 * Get secondary asset search path (fallback)
	 * For Zero Hour, returns Generals base game path for shared assets
	 * @return Full path to secondary asset directory (or empty if none)
	 */
	static AsciiString getSecondaryAssetPath();

	/**
	 * Check if a file exists at standard asset locations
	 * Checks both primary and secondary paths
	 * @param filename - Relative path to file (e.g., "Data\big.dat")
	 * @return Full path if found, or empty AsciiString if not found
	 */
	static AsciiString findAssetFile(const AsciiString& filename);

	/**
	 * Get game installation base directory
	 * (e.g., %USERPROFILE%\GeneralsX\ for all variants)
	 * @return Full path to installation base
	 */
	static AsciiString getInstallationBasePath();

	/**
	 * Log all discovered paths for debugging
	 */
	static void logDiscoveredPaths();

	/**
	 * Check if asset path is valid (contains recognizable game files)
	 * @param path - Path to check
	 * @return TRUE if valid asset directory
	 */
	static Bool isValidAssetPath(const AsciiString& path);

private:
	/**
	 * Discover asset path from standard locations
	 * @return Full path if found, or empty AsciiString if not
	 */
	static AsciiString discoverAssetPath();

	/**
	 * Get override path from INI configuration if set
	 * @return Path from INI or empty AsciiString if not configured
	 */
	static AsciiString getConfigOverridePath();

	/**
	 * Get game-specific subdirectory name
	 * @param variant - Game variant
	 * @return "GeneralsMD" for Zero Hour, "Generals" for base game
	 */
	static AsciiString getGameSubdirectory(GameVariant variant);
};

#endif // __SEARCHPATHMANAGER_H__
