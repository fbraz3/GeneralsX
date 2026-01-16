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
//
////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Common/ConfigurationManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

// ============================================================================
// INI Data Structure - Simple map for storing key-value pairs by section
// ============================================================================

struct INISection
{
	AsciiString name;
	// Store as simple arrays of key-value pairs (VC6 compatible)
	AsciiString keys[256];      // Max 256 keys per section
	AsciiString values[256];    // Corresponding values
	int keyCount;
	
	INISection() : keyCount(0) { }
	
	Bool getKeyValue(const AsciiString& key, AsciiString& outValue) const
	{
		for (int i = 0; i < keyCount; i++)
		{
			if (keys[i].compareNoCase(key) == 0)
			{
				outValue = values[i];
				return TRUE;
			}
		}
		return FALSE;
	}
	
	void setKeyValue(const AsciiString& key, const AsciiString& value)
	{
		for (int i = 0; i < keyCount; i++)
		{
			if (keys[i].compareNoCase(key) == 0)
			{
				values[i] = value;
				return;
			}
		}
		// Add new key if space available
		if (keyCount < 256)
		{
			keys[keyCount] = key;
			values[keyCount] = value;
			keyCount++;
		}
	}
};

// Static instance data
static ConfigurationManager::GameVariant g_currentVariant = ConfigurationManager::VARIANT_GENERALS;
static AsciiString g_configDirectory;
static AsciiString g_configFilePath;
static Bool g_initialized = FALSE;
static INISection g_sections[32];      // Max 32 sections
static int g_sectionCount = 0;

// ============================================================================
// Helper Functions
// ============================================================================

static INISection* findOrCreateSection(const AsciiString& sectionName)
{
	// Find existing section
	for (int i = 0; i < g_sectionCount; i++)
	{
		if (g_sections[i].name.compareNoCase(sectionName) == 0)
		{
			return &g_sections[i];
		}
	}
	
	// Create new section if space available
	if (g_sectionCount < 32)
	{
		g_sections[g_sectionCount].name = sectionName;
		g_sections[g_sectionCount].keyCount = 0;
		return &g_sections[g_sectionCount++];
	}
	
	return NULL;
}

static INISection* findSection(const AsciiString& sectionName)
{
	for (int i = 0; i < g_sectionCount; i++)
	{
		if (g_sections[i].name.compareNoCase(sectionName) == 0)
		{
			return &g_sections[i];
		}
	}
	return NULL;
}

static void trimWhitespace(AsciiString& str)
{
	// Use standard library to strip whitespace
	str.trim();
}

// Helper to find '=' or ']' position
static int findCharPos(const AsciiString& str, char c)
{
	const char* ptr = str.str();
	for (int i = 0; ptr[i]; i++)
	{
		if (ptr[i] == c)
			return i;
	}
	return -1;
}

// Helper to extract substring from position
static AsciiString extractSubstring(const AsciiString& str, int startPos, int endPos)
{
	if (startPos < 0 || endPos < startPos || startPos >= str.getLength())
		return "";
	
	int len = endPos - startPos;
	if (len <= 0)
		return "";
	
	AsciiString result;
	char buffer[512];
	const char* srcStr = str.str();
	
	// Copy characters
	int bufPos = 0;
	for (int i = 0; i < len && srcStr[startPos + i] && bufPos < 511; i++)
	{
		buffer[bufPos++] = srcStr[startPos + i];
	}
	buffer[bufPos] = '\0';
	
	result = buffer;
	return result;
}

// ============================================================================
// ConfigurationManager Implementation
// ============================================================================

Bool ConfigurationManager::init(GameVariant variant)
{
	g_currentVariant = variant;
	
	// Get user profile directory (files go directly in profile, no subdirs)
#ifdef _WIN32
	char buffer[512] = { 0 };
	if (GetEnvironmentVariableA("USERPROFILE", buffer, sizeof(buffer)))
	{
		g_configDirectory = buffer;
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
		g_configDirectory = home;
	}
	else
	{
		g_configDirectory = ".";
	}
#endif

	// Set INI file path
	if (variant == VARIANT_ZERO_HOUR)
	{
#ifdef _WIN32
		g_configFilePath.format("%s\\.GeneralsXZH.ini", g_configDirectory.str());
#else
		g_configFilePath.format("%s/.GeneralsXZH.ini", g_configDirectory.str());
#endif
	}
	else
	{
#ifdef _WIN32
		g_configFilePath.format("%s\\.GeneralsX.ini", g_configDirectory.str());
#else
		g_configFilePath.format("%s/.GeneralsX.ini", g_configDirectory.str());
#endif
	}

	// Load INI file
	loadINI();

	g_initialized = TRUE;
	return TRUE;
}

void ConfigurationManager::shutdown()
{
	g_initialized = FALSE;
	g_sectionCount = 0;
}

Bool ConfigurationManager::loadINI()
{
	// Clear existing data
	g_sectionCount = 0;
	for (int i = 0; i < 32; i++)
	{
		g_sections[i].keyCount = 0;
		g_sections[i].name = "";
	}
	
	FILE* fp = fopen(g_configFilePath.str(), "r");
	if (!fp)
	{
		DEBUG_LOG(("ConfigurationManager: INI file not found: %s\n", g_configFilePath.str()));
		return FALSE;
	}
	
	char line[512];
	AsciiString currentSection = "";
	
	while (fgets(line, sizeof(line), fp))
	{
		AsciiString lineStr(line);
		trimWhitespace(lineStr);
		
		// Skip empty lines and comments
		if (lineStr.isEmpty() || lineStr.getCharAt(0) == ';' || lineStr.getCharAt(0) == '#')
			continue;
		
		// Check for section header [SectionName]
		if (lineStr.getCharAt(0) == '[')
		{
			int endBracket = findCharPos(lineStr, ']');
			if (endBracket > 0)
			{
				currentSection = extractSubstring(lineStr, 1, endBracket);
				trimWhitespace(currentSection);
				findOrCreateSection(currentSection);
			}
			continue;
		}
		
		// Parse key=value
		int equalsPos = findCharPos(lineStr, '=');
		if (equalsPos > 0 && !currentSection.isEmpty())
		{
			AsciiString key = extractSubstring(lineStr, 0, equalsPos);
			AsciiString value = extractSubstring(lineStr, equalsPos + 1, lineStr.getLength());
			
			trimWhitespace(key);
			trimWhitespace(value);
			
			INISection* section = findSection(currentSection);
			if (section)
			{
				section->setKeyValue(key, value);
			}
		}
	}
	
	fclose(fp);
	DEBUG_LOG(("ConfigurationManager: Loaded INI file: %s\n", g_configFilePath.str()));
	return TRUE;
}

Bool ConfigurationManager::getString(const AsciiString& section, const AsciiString& key, AsciiString& outValue)
{
	INISection* sec = findSection(section);
	if (sec)
	{
		return sec->getKeyValue(key, outValue);
	}
	return FALSE;
}

Bool ConfigurationManager::setString(const AsciiString& section, const AsciiString& key, const AsciiString& value)
{
	INISection* sec = findOrCreateSection(section);
	if (sec)
	{
		sec->setKeyValue(key, value);
		return TRUE;
	}
	return FALSE;
}

Bool ConfigurationManager::getInteger(const AsciiString& section, const AsciiString& key, Int& outValue)
{
	AsciiString strValue;
	if (getString(section, key, strValue))
	{
		outValue = atoi(strValue.str());
		return TRUE;
	}
	return FALSE;
}

Bool ConfigurationManager::setInteger(const AsciiString& section, const AsciiString& key, Int value)
{
	AsciiString strValue;
	strValue.format("%d", value);
	return setString(section, key, strValue);
}

Bool ConfigurationManager::getUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt& outValue)
{
	AsciiString strValue;
	if (getString(section, key, strValue))
	{
		outValue = (UnsignedInt)strtoul(strValue.str(), NULL, 10);
		return TRUE;
	}
	return FALSE;
}

Bool ConfigurationManager::setUnsignedInt(const AsciiString& section, const AsciiString& key, UnsignedInt value)
{
	AsciiString strValue;
	strValue.format("%u", value);
	return setString(section, key, strValue);
}

Bool ConfigurationManager::getBoolean(const AsciiString& section, const AsciiString& key, Bool& outValue)
{
	AsciiString strValue;
	if (getString(section, key, strValue))
	{
		strValue.toLower();
		outValue = (strValue == "true" || strValue == "1" || strValue == "yes");
		return TRUE;
	}
	return FALSE;
}

Bool ConfigurationManager::setBoolean(const AsciiString& section, const AsciiString& key, Bool value)
{
	AsciiString strValue = value ? "true" : "false";
	return setString(section, key, strValue);
}

Bool ConfigurationManager::getReal(const AsciiString& section, const AsciiString& key, Real& outValue)
{
	AsciiString strValue;
	if (getString(section, key, strValue))
	{
		outValue = (Real)atof(strValue.str());
		return TRUE;
	}
	return FALSE;
}

Bool ConfigurationManager::setReal(const AsciiString& section, const AsciiString& key, Real value)
{
	AsciiString strValue;
	strValue.format("%.6f", value);
	return setString(section, key, strValue);
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
	// Write all sections and keys to INI file
	FILE* fp = fopen(g_configFilePath.str(), "w");
	if (!fp)
	{
		DEBUG_LOG(("ConfigurationManager: Failed to open INI file for writing: %s\n", g_configFilePath.str()));
		return FALSE;
	}
	
	// Write each section
	for (int i = 0; i < g_sectionCount; i++)
	{
		INISection& section = g_sections[i];
		
		// Write section header
		fprintf(fp, "[%s]\n", section.name.str());
		
		// Write all key=value pairs in this section
		for (int j = 0; j < section.keyCount; j++)
		{
			fprintf(fp, "%s = %s\n", section.keys[j].str(), section.values[j].str());
		}
		
		// Blank line between sections for readability
		fprintf(fp, "\n");
	}
	
	fclose(fp);
	DEBUG_LOG(("ConfigurationManager: Flushed INI file: %s\n", g_configFilePath.str()));
	return TRUE;
}

Bool ConfigurationManager::reload()
{
	return loadINI();
}

AsciiString ConfigurationManager::getAssetSearchPath()
{
	AsciiString assetPath;
	if (getString("Advanced", "AssetPath", assetPath) && !assetPath.isEmpty())
	{
		return assetPath;
	}
	return "";
}

AsciiString ConfigurationManager::getConfigDirectory()
{
	return g_configDirectory;
}

Bool ConfigurationManager::createDefaultINI()
{
	// For future use - would create default INI with standard values
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

