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
//  Registry.cpp
//  Cross-platform Registry wrapper for Windows Registry compatibility
//  Maps old Registry calls to new INI-based configuration system
//  Enables mods/tools (e.g., gentools) to work without modification
//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/Registry.h"
#include "Common/ConfigurationManager.h"

// ============================================================================
// Legacy GetRegistryXxx functions - compatibility wrappers for mods/tools
// ============================================================================

/**
 * Get string value from INI-based config (mimics old Registry behavior)
 * Maps registry-like calls to ConfigurationManager
 */
AsciiString GetRegistryString(const AsciiString& keyPath, const AsciiString& defaultValue)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator (Windows registry style)
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);  // Skip the backslash
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);  // Skip the slash
		}
		else
		{
			// No separator found - invalid format
			return defaultValue;
		}
	}
	
	AsciiString result;
	if (ConfigurationManager::getString(section, key, result))
	{
		return result;
	}
	
	return defaultValue;
}

/**
 * Get integer value from INI-based config
 */
Int GetRegistryInt(const AsciiString& keyPath, Int defaultValue)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return defaultValue;
		}
	}
	
	Int result = 0;
	if (ConfigurationManager::getInteger(section, key, result))
	{
		return result;
	}
	
	return defaultValue;
}

/**
 * Get boolean value from INI-based config
 */
Bool GetRegistryBool(const AsciiString& keyPath, Bool defaultValue)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return defaultValue;
		}
	}
	
	Bool result = FALSE;
	if (ConfigurationManager::getBoolean(section, key, result))
	{
		return result;
	}
	
	return defaultValue;
}

/**
 * Get real/float value from INI-based config
 */
Real GetRegistryReal(const AsciiString& keyPath, Real defaultValue)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return defaultValue;
		}
	}
	
	Real result = 0.0f;
	if (ConfigurationManager::getReal(section, key, result))
	{
		return result;
	}
	
	return defaultValue;
}

// ============================================================================
// Legacy SetRegistryXxx functions - compatibility wrappers for writing
// ============================================================================

/**
 * Set string value in INI-based config
 */
void SetRegistryString(const AsciiString& keyPath, const AsciiString& value)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return;
		}
	}
	
	ConfigurationManager::setString(section, key, value);
	ConfigurationManager::flush();
}

/**
 * Set integer value in INI-based config
 */
void SetRegistryInt(const AsciiString& keyPath, Int value)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return;
		}
	}
	
	ConfigurationManager::setInteger(section, key, value);
	ConfigurationManager::flush();
}

/**
 * Set boolean value in INI-based config
 */
void SetRegistryBool(const AsciiString& keyPath, Bool value)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return;
		}
	}
	
	ConfigurationManager::setBoolean(section, key, value);
	ConfigurationManager::flush();
}

/**
 * Set real/float value in INI-based config
 */
void SetRegistryReal(const AsciiString& keyPath, Real value)
{
	// Parse key path format: "Section\Key" or "Section/Key"
	AsciiString section, key;
	
	// Try backslash separator
	const char* ptr = keyPath.find('\\');
	if (ptr)
	{
		int slashPos = ptr - keyPath.str();
		section = AsciiString(keyPath.str(), slashPos);
		key = AsciiString(ptr + 1);
	}
	else
	{
		// Try forward slash separator
		ptr = keyPath.find('/');
		if (ptr)
		{
			int slashPos = ptr - keyPath.str();
			section = AsciiString(keyPath.str(), slashPos);
			key = AsciiString(ptr + 1);
		}
		else
		{
			return;
		}
	}
	
	ConfigurationManager::setReal(section, key, value);
	ConfigurationManager::flush();
}

/**
 * Flush all pending configuration changes to disk
 */
void FlushRegistry()
{
	ConfigurationManager::flush();
}
