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
//  ConfigurationManager.h
//  Cross-platform INI-based configuration management system
//  Replaces Windows Registry with INI files for universal compatibility
//  Author: GeneralsX Development Team
//
//  Purpose:
//    - Load/save game configuration from INI files instead of registry
//    - Store config in %USERPROFILE%\.GeneralsX or %USERPROFILE%\.GeneralsXZH
//    - Auto-generate default INI files on first run
//    - Support cross-platform (Windows, macOS, Linux via Wine)
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CONFIGURATIONMANAGER_H__
#define __CONFIGURATIONMANAGER_H__

#include "Common/AsciiString.h"

/**
 * Configuration Manager - handles INI-based game configuration
 * 
 * Supports sections: General, Graphics, Audio, Player, Network, Advanced
 * Automatically creates default INI if not present
 */
class ConfigurationManager
{
public:
	enum GameVariant
	{
		VARIANT_GENERALS,       ///< Command & Conquer Generals (base game)
		VARIANT_ZERO_HOUR       ///< Command & Conquer Generals Zero Hour (expansion)
	};

	/**
	 * Initialize configuration system
	 * @param variant - Which game variant (Generals or Zero Hour)
	 * @return TRUE if initialization successful
	 */
	static Bool init(GameVariant variant);

	/**
	 * Shutdown configuration system
	 */
	static void shutdown();

	// String configuration accessors
	static Bool getString(const AsciiString& section, const AsciiString& key, AsciiString& outValue);
	static Bool setString(const AsciiString& section, const AsciiString& key, const AsciiString& value);

	// Integer configuration accessors
	static Bool getInteger(const AsciiString& section, const AsciiString& key, Int& outValue);
	static Bool setInteger(const AsciiString& section, const AsciiString& key, Int value);

	// Unsigned integer configuration accessors
	static Bool getUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt& outValue);
	static Bool setUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt value);

	// Boolean configuration accessors
	static Bool getBoolean(const AsciiString& section, const AsciiString& key, Bool& outValue);
	static Bool setBoolean(const AsciiString& section, const AsciiString& key, Bool value);

	// Real (float) configuration accessors
	static Bool getReal(const AsciiString& section, const AsciiString& key, Real& outValue);
	static Bool setReal(const AsciiString& section, const AsciiString& key, Real value);

	/**
	 * Get the full path to the configuration INI file
	 * @return Full path to INI file (e.g., C:\Users\Felipe\.GeneralsX\GeneralsX.ini)
	 */
	static AsciiString getConfigFilePath();

	/**
	 * Check if configuration file exists
	 * @return TRUE if INI file exists
	 */
	static Bool configFileExists();

	/**
	 * Flush all changes to disk
	 * @return TRUE if successful
	 */
	static Bool flush();

	/**
	 * Reload configuration from file (discards any unsaved changes)
	 * @return TRUE if successful
	 */
	static Bool reload();

	/**
	 * Get asset search path (can be overridden via INI [Advanced] section)
	 * @return Full path to asset directory
	 */
	static AsciiString getAssetSearchPath();

	/**
	 * Get the configuration directory path
	 * (e.g., %USERPROFILE%\.GeneralsX or ~/.config/GeneralsX on Linux)
	 * @return Path to configuration directory
	 */
	static AsciiString getConfigDirectory();

private:
	/**
	 * Create default INI file with all sections and default values
	 * @return TRUE if successful
	 */
	static Bool createDefaultINI();

	/**
	 * Load INI file from disk
	 * @return TRUE if successful
	 */
	static Bool loadINI();

	/**
	 * Determine which INI filename to use based on game variant
	 * @return Filename (e.g., "GeneralsX.ini" or "GeneralsXZH.ini")
	 */
	static AsciiString getINIFilename();
};

#endif // __CONFIGURATIONMANAGER_H__
