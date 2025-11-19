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

// Default values for INI file initialization
// These values match the template files in assets/ini/
namespace DefaultValues
{
	// GeneralsXZH Settings section
	static const char* LANGUAGE = "english";
	static const char* SKU_GENERALS = "GeneralsX";
	static const char* SKU_ZH = "GeneralsZH";
	static const UnsignedInt VERSION = 65540;
	static const UnsignedInt MAP_PACK_VERSION = 65536;
	static const char* INSTALL_PATH = "";
	static const char* PROXY = "";
	static const char* ERGC = "GP205480888522112040";
	
	// Graphics section
	static const UnsignedInt WIDTH = 1024;
	static const UnsignedInt HEIGHT = 768;
	static const UnsignedInt WINDOWED = 0;
	static const UnsignedInt COLOR_DEPTH = 32;
	static const UnsignedInt USE_METAL_BACKEND = 1;
	
	// Audio section
	static const UnsignedInt AUDIO_ENABLED = 1;
	static const UnsignedInt MUSIC_VOLUME = 100;
	static const UnsignedInt SOUND_VOLUME = 100;
	
	// Network section
	static const char* CONNECTION_TYPE = "LAN";
	static const UnsignedInt BANDWIDTH = 100000;
	
	// Player section
	static const char* PLAYER_NAME = "Player";
	static const char* PLAYER_SIDE = "USA";
	static const char* PLAYER_DIFFICULTY = "Hard";
	static const UnsignedInt GENERAL_INDEX = 0;
	
	// Advanced section
	static const UnsignedInt ENABLE_DEBUG = 0;
	static const UnsignedInt LOG_LEVEL = 0;
	static const char* ASSET_PATH = "";
	static const char* MAP_PATH = "";
}

// Initialize default INI file if it doesn't exist
static void InitializeDefaultConfigurationFile(const AsciiString& ini_file)
{
	try
	{
		// Check if file already exists
		std::ifstream check(ini_file.str());
		if (check.good())
		{
			check.close();
			return;  // File exists, no need to create
		}
		check.close();

		// File doesn't exist, create with defaults
		std::ofstream output(ini_file.str());
		if (!output.is_open())
		{
			DEBUG_LOG(("Registry: Failed to create default INI file: %s", ini_file.str()));
			return;
		}

		// Determine if this is Zero Hour or Generals
		bool is_zh = (std::string(ini_file.str()).find("GeneralsXZH.ini") != std::string::npos);

		// Write GeneralsXZH Settings or Generals Settings section
		if (is_zh)
		{
			output << "[GeneralsXZH Settings]" << std::endl;
		}
		else
		{
			output << "[Generals Settings]" << std::endl;
		}
		output << "Language = " << DefaultValues::LANGUAGE << std::endl;
		output << "SKU = " << (is_zh ? DefaultValues::SKU_ZH : DefaultValues::SKU_GENERALS) << std::endl;
		output << "Version = " << DefaultValues::VERSION << std::endl;
		output << "MapPackVersion = " << DefaultValues::MAP_PACK_VERSION << std::endl;
		output << "InstallPath = " << DefaultValues::INSTALL_PATH << std::endl;
		output << "Proxy = " << DefaultValues::PROXY << std::endl;
		
		if (is_zh)
		{
			output << "ERGC = " << DefaultValues::ERGC << std::endl;
		}

		// Graphics section
		output << std::endl << "[Graphics]" << std::endl;
		output << "Width = " << DefaultValues::WIDTH << std::endl;
		output << "Height = " << DefaultValues::HEIGHT << std::endl;
		output << "Windowed = " << DefaultValues::WINDOWED << std::endl;
		output << "ColorDepth = " << DefaultValues::COLOR_DEPTH << std::endl;
		
		if (is_zh)
		{
			output << "UseMetalBackend = " << DefaultValues::USE_METAL_BACKEND << std::endl;
		}

		// Audio section
		output << std::endl << "[Audio]" << std::endl;
		output << "Enabled = " << DefaultValues::AUDIO_ENABLED << std::endl;
		output << "MusicVolume = " << DefaultValues::MUSIC_VOLUME << std::endl;
		output << "SoundVolume = " << DefaultValues::SOUND_VOLUME << std::endl;

		// Network section
		output << std::endl << "[Network]" << std::endl;
		output << "ConnectionType = " << DefaultValues::CONNECTION_TYPE << std::endl;
		output << "Bandwidth = " << DefaultValues::BANDWIDTH << std::endl;

		// Player section
		output << std::endl << "[Player]" << std::endl;
		output << "Name = " << DefaultValues::PLAYER_NAME << std::endl;
		output << "Side = " << DefaultValues::PLAYER_SIDE << std::endl;
		output << "Difficulty = " << DefaultValues::PLAYER_DIFFICULTY << std::endl;
		
		if (is_zh)
		{
			output << "GeneralIndex = " << DefaultValues::GENERAL_INDEX << std::endl;
		}

		// Advanced section
		output << std::endl << "[Advanced]" << std::endl;
		output << "EnableDebug = " << DefaultValues::ENABLE_DEBUG << std::endl;
		output << "LogLevel = " << DefaultValues::LOG_LEVEL << std::endl;
		
		if (is_zh)
		{
			output << "AssetPath = " << DefaultValues::ASSET_PATH << std::endl;
			output << "MapPath = " << DefaultValues::MAP_PATH << std::endl;
		}

		output.close();
		DEBUG_LOG(("Registry: Created default INI file: %s", ini_file.str()));
	}
	catch (const std::exception& e)
	{
		DEBUG_LOG(("Registry: Error creating default INI file: %s", e.what()));
	}
}

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
	
	// Initialize default configuration if file doesn't exist
	InitializeDefaultConfigurationFile(ini_file);
	
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
