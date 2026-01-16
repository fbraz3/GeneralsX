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
//  ConfigurationManager.cpp
//  Implementation of cross-platform INI-based configuration management
//  Stub implementation for VC6 compatibility - real implementation deferred to Phase 2
//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/ConfigurationManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

// Static instance data
static ConfigurationManager::GameVariant g_currentVariant = ConfigurationManager::VARIANT_GENERALS;
static AsciiString g_configDirectory;
static AsciiString g_configFilePath;
static Bool g_initialized = FALSE;

// ============================================================================
// ConfigurationManager Implementation
// ============================================================================

Bool ConfigurationManager::init(GameVariant variant)
{
	g_currentVariant = variant;
	
	// Get user profile directory
#ifdef _WIN32
	char buffer[512] = { 0 };
	if (GetEnvironmentVariableA("USERPROFILE", buffer, sizeof(buffer)))
	{
		g_configDirectory.format("%s\\.GeneralsX", buffer);
	}
	else
	{
		// Fallback 
		g_configDirectory = ".";
	}
#else
	const char* home = getenv("HOME");
	if (home)
	{
		g_configDirectory.format("%s/.config/GeneralsX", home);
	}
	else
	{
		g_configDirectory = ".";
	}
#endif

	// Create config directory if needed
	CreateDirectoryA(g_configDirectory.str(), NULL);

	// Set INI file path
	if (variant == VARIANT_ZERO_HOUR)
	{
		g_configFilePath.format("%s\\GeneralsXZH.ini", g_configDirectory.str());
	}
	else
	{
		g_configFilePath.format("%s\\GeneralsX.ini", g_configDirectory.str());
	}

	DEBUG_LOG(("ConfigurationManager: Initialized at %s\n", g_configFilePath.str()));

	g_initialized = TRUE;
	return TRUE;
}

void ConfigurationManager::shutdown()
{
	g_initialized = FALSE;
}

Bool ConfigurationManager::getString(const AsciiString& section, const AsciiString& key, AsciiString& outValue)
{
	// Stub implementation - just return empty for now
	// Full implementation deferred to next phase when we have proper INI parsing
	outValue = "";
	return FALSE;
}

Bool ConfigurationManager::setString(const AsciiString& section, const AsciiString& key, const AsciiString& value)
{
	// Stub implementation
	return FALSE;
}

Bool ConfigurationManager::getInteger(const AsciiString& section, const AsciiString& key, Int& outValue)
{
	outValue = 0;
	return FALSE;
}

Bool ConfigurationManager::setInteger(const AsciiString& section, const AsciiString& key, Int value)
{
	return FALSE;
}

Bool ConfigurationManager::getUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt& outValue)
{
	outValue = 0;
	return FALSE;
}

Bool ConfigurationManager::setUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt value)
{
	return FALSE;
}

Bool ConfigurationManager::getBoolean(const AsciiString& section, const AsciiString& key, Bool& outValue)
{
	outValue = FALSE;
	return FALSE;
}

Bool ConfigurationManager::setBoolean(const AsciiString& section, const AsciiString& key, Bool value)
{
	return FALSE;
}

Bool ConfigurationManager::getReal(const AsciiString& section, const AsciiString& key, Real& outValue)
{
	outValue = 0.0f;
	return FALSE;
}

Bool ConfigurationManager::setReal(const AsciiString& section, const AsciiString& key, Real value)
{
	return FALSE;
}

AsciiString ConfigurationManager::getConfigFilePath()
{
	return g_configFilePath;
}

Bool ConfigurationManager::configFileExists()
{
#ifdef _WIN32
	HANDLE hFile = CreateFileA(g_configFilePath.str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
#else
	return (access(g_configFilePath.str(), F_OK) == 0);
#endif
}

Bool ConfigurationManager::flush()
{
	// Stub - will implement when INI writing is needed
	return TRUE;
}

Bool ConfigurationManager::reload()
{
	// Stub - will implement when INI reading is needed
	return TRUE;
}

AsciiString ConfigurationManager::getAssetSearchPath()
{
	// Stub - SearchPathManager will provide this
	return "";
}

AsciiString ConfigurationManager::getConfigDirectory()
{
	return g_configDirectory;
}

Bool ConfigurationManager::createDefaultINI()
{
	// Stub - will implement in full version
	return TRUE;
}

Bool ConfigurationManager::loadINI()
{
	// Stub - will implement in full version
	return TRUE;
}

AsciiString ConfigurationManager::getINIFilename()
{
	if (g_currentVariant == VARIANT_ZERO_HOUR)
	{
		return "GeneralsXZH.ini";
	}
	else
	{
		return "GeneralsX.ini";
	}
}
