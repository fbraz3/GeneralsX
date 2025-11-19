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
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// Registry.cpp
// Simple interface for storing/retreiving registry values
// Cross-platform implementation using INI files instead of Windows Registry
// Author: Matthew D. Campbell, December 2001
// Ported to SDL2/INI: Phase 39.5 Week 4

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Registry.h"
#include "WWVegas/win32_thread_compat.h"
#include <filesystem>
#include <fstream>
#include <sstream>

// Global configuration file path (cached after first use)
static AsciiString g_configFilePath;

// Initialize config file path using SDL_GetPrefPath
static AsciiString GetConfigFilePath()
{
	if (!g_configFilePath.isEmpty())
	{
		return g_configFilePath;
	}

	// Get application config directory
	const char* pref_path = SDL_GetPrefPath("GeneralsX", "");
	if (pref_path)
	{
		g_configFilePath = pref_path;
		SDL_free((void*)pref_path);
	}
	else
	{
		// Fallback to current directory if SDL_GetPrefPath fails
		g_configFilePath = "./";
	}

	// Ensure directory exists
	try
	{
		std::filesystem::create_directories(std::filesystem::path(g_configFilePath.str()));
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		DEBUG_LOG(("Registry: Failed to create config directory: %s", e.what()));
	}

	return g_configFilePath;
}

// Get the full INI file path
static AsciiString GetINIFilePath()
{
	AsciiString config_dir = GetConfigFilePath();
	AsciiString ini_file = config_dir;
	
	// Add filename - use GeneralsXZH.ini for Zero Hour expansion
	#ifdef GENERALS_ZERO_HOUR
		ini_file.concat("GeneralsXZH.ini");
	#else
		ini_file.concat("GeneralsX.ini");
	#endif
	
	return ini_file;
}

// Simple INI parser: Read value from INI file
static Bool ReadINIValue(const AsciiString& section, const AsciiString& key, AsciiString& val)
{
	AsciiString ini_file = GetINIFilePath();
	
	try
	{
		std::ifstream file(ini_file.str());
		if (!file.is_open())
		{
			return FALSE;
		}

		std::string line;
		std::string current_section;
		bool found = false;

		while (std::getline(file, line))
		{
			// Trim whitespace
			size_t start = line.find_first_not_of(" \t\r\n");
			size_t end = line.find_last_not_of(" \t\r\n");
			
			if (start == std::string::npos)
				continue;

			line = line.substr(start, end - start + 1);

			// Check for section header [Section]
			if (line[0] == '[' && line[line.length() - 1] == ']')
			{
				current_section = line.substr(1, line.length() - 2);
				continue;
			}

			// Check if we're in the right section
			if (section.str() != current_section && !section.isEmpty())
				continue;

			// Look for key=value
			size_t eq_pos = line.find('=');
			if (eq_pos != std::string::npos)
			{
				std::string key_str = line.substr(0, eq_pos);
				std::string val_str = line.substr(eq_pos + 1);

				// Trim key and value
				key_str.erase(0, key_str.find_first_not_of(" \t"));
				key_str.erase(key_str.find_last_not_of(" \t") + 1);
				val_str.erase(0, val_str.find_first_not_of(" \t"));
				val_str.erase(val_str.find_last_not_of(" \t") + 1);

				if (key_str == key.str())
				{
					val = val_str.c_str();
					found = true;
					break;
				}
			}
		}

		file.close();
		return found;
	}
	catch (const std::exception& e)
	{
		DEBUG_LOG(("Registry: Error reading INI file: %s", e.what()));
		return FALSE;
	}
}

// Simple INI writer: Write value to INI file
static Bool WriteINIValue(const AsciiString& section, const AsciiString& key, const AsciiString& val)
{
	AsciiString ini_file = GetINIFilePath();
	
	try
	{
		// Read existing file
		std::vector<std::string> lines;
		std::ifstream input(ini_file.str());
		std::string line;
		bool found = false;
		std::string current_section;

		if (input.is_open())
		{
			while (std::getline(input, line))
			{
				// Trim whitespace
				size_t start = line.find_first_not_of(" \t\r\n");
				if (start == std::string::npos)
				{
					lines.push_back(line);
					continue;
				}

				std::string trimmed = line.substr(start);

				// Check for section header
				if (trimmed[0] == '[' && trimmed[trimmed.length() - 1] == ']')
				{
					current_section = trimmed.substr(1, trimmed.length() - 2);
					lines.push_back(line);
					continue;
				}

				// Check if we're in the right section and found our key
				if (current_section == section.str())
				{
					size_t eq_pos = trimmed.find('=');
					if (eq_pos != std::string::npos)
					{
						std::string key_str = trimmed.substr(0, eq_pos);
						key_str.erase(0, key_str.find_first_not_of(" \t"));
						key_str.erase(key_str.find_last_not_of(" \t") + 1);

						if (key_str == key.str())
						{
							lines.push_back(std::string(key.str()) + " = " + std::string(val.str()));
							found = true;
							continue;
						}
					}
				}

				lines.push_back(line);
			}
			input.close();
		}

		// If not found, add it
		if (!found)
		{
			// Add section if needed
			if (!section.isEmpty())
			{
				bool has_section = false;
				for (const auto& l : lines)
				{
					if (l.find("[" + std::string(section.str()) + "]") != std::string::npos)
					{
						has_section = true;
						break;
					}
				}

				if (!has_section)
				{
					lines.push_back(std::string("[") + section.str() + "]");
				}
			}

			lines.push_back(std::string(key.str()) + " = " + std::string(val.str()));
		}

		// Write back
		std::ofstream output(ini_file.str());
		for (const auto& l : lines)
		{
			output << l << std::endl;
		}
		output.close();

		return TRUE;
	}
	catch (const std::exception& e)
	{
		DEBUG_LOG(("Registry: Error writing INI file: %s", e.what()));
		return FALSE;
	}
}

// Convert AsciiString number to unsigned int
static UnsignedInt StringToUInt(const AsciiString& str)
{
	try
	{
		return std::stoul(str.str());
	}
	catch (...)
	{
		return 0;
	}
}

// Convert unsigned int to AsciiString
static AsciiString UIntToString(UnsignedInt val)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%u", val);
	return AsciiString(buffer);
}

// Implementation of public API functions

Bool getStringFromRegistry(const AsciiString& section, const AsciiString& key, AsciiString& val)
{
	return ReadINIValue(section, key, val);
}

Bool getUnsignedIntFromRegistry(const AsciiString& section, const AsciiString& key, UnsignedInt& val)
{
	AsciiString str_val;
	if (ReadINIValue(section, key, str_val))
	{
		val = StringToUInt(str_val);
		return TRUE;
	}
	return FALSE;
}

Bool setStringInRegistry(const AsciiString& section, const AsciiString& key, const AsciiString& val)
{
	return WriteINIValue(section, key, val);
}

Bool setUnsignedIntInRegistry(const AsciiString& section, const AsciiString& key, UnsignedInt val)
{
	AsciiString str_val = UIntToString(val);
	return WriteINIValue(section, key, str_val);
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	// For Generals base game, use [Generals Settings] section
	AsciiString section = "Generals Settings";
	if (!path.isEmpty())
	{
		section.concat("\\");
		section.concat(path);
	}

	DEBUG_LOG(("GetStringFromGeneralsRegistry - looking for key %s in section %s", key.str(), section.str()));
	return getStringFromRegistry(section, key, val);
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	// For Zero Hour expansion, use [GeneralsXZH Settings] section
	AsciiString section = "GeneralsXZH Settings";
	if (!path.isEmpty())
	{
		section.concat("\\");
		section.concat(path);
	}

	DEBUG_LOG(("GetStringFromRegistry - looking for key %s in section %s", key.str(), section.str()));
	return getStringFromRegistry(section, key, val);
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
	// For Zero Hour expansion, use [GeneralsXZH Settings] section
	AsciiString section = "GeneralsXZH Settings";
	if (!path.isEmpty())
	{
		section.concat("\\");
		section.concat(path);
	}

	DEBUG_LOG(("GetUnsignedIntFromRegistry - looking for key %s in section %s", key.str(), section.str()));
	return getUnsignedIntFromRegistry(section, key, val);
}

AsciiString GetRegistryLanguage(void)
{
	static Bool cached = FALSE;
	static AsciiString val = "english";
	if (cached) {
		return val;
	} else {
		cached = TRUE;
	}

	GetStringFromRegistry("", "Language", val);
	return val;
}

AsciiString GetRegistryGameName(void)
{
	AsciiString val = "GeneralsMPTest";
	GetStringFromRegistry("", "SKU", val);
	return val;
}

UnsignedInt GetRegistryVersion(void)
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "Version", val);
	return val;
}

UnsignedInt GetRegistryMapPackVersion(void)
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "MapPackVersion", val);
	return val;
}
