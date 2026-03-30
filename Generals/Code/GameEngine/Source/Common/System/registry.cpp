/*
**	Command & Conquer Generals(tm)
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
// Simple interface for storing/retrieving registry values
// Author: Matthew D. Campbell, December 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Registry.h"
#include "registryini.h"

// TheSuperHackers @build felipebraz 11/02/2026 Phase 1.5 - Linux port
// Windows Registry types not available on Linux - define stub types
#ifdef _UNIX
#ifndef HKEY_LOCAL_MACHINE
typedef void* HKEY;  // Stub type for Linux (unused but needed for compilation)
#define HKEY_LOCAL_MACHINE  ((HKEY)(uintptr_t)0x80000002)
#define HKEY_CURRENT_USER   ((HKEY)(uintptr_t)0x80000001)
#endif
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// TheSuperHackers @build felipebraz 11/02/2026 Phase 1.5 - Linux port
// Windows Registry API not available on Linux - stub implementations return failure
// Game gracefully degrades to default values (later can be replaced with INI files)
#ifdef _UNIX

// GeneralsX @feature GitHubCopilot 29/03/2026 Persist non-Windows registry values in registry.ini.
static const char *getRegistryIniRoot(HKEY root)
{
	return root == HKEY_LOCAL_MACHINE ? RegistryIni::LocalMachineRoot() : RegistryIni::CurrentUserRoot();
}

// GeneralsX @feature GitHubCopilot 29/03/2026 Route low-level non-Windows registry reads through registry.ini.
Bool  getStringFromRegistry(HKEY root, AsciiString path, AsciiString key, AsciiString& val)
{
	std::string storedValue;
	if (!RegistryIni::ReadString(getRegistryIniRoot(root), path.str(), key.str(), storedValue))
	{
		return FALSE;
	}

	val = storedValue.c_str();
	return TRUE;
}

Bool getUnsignedIntFromRegistry(HKEY root, AsciiString path, AsciiString key, UnsignedInt& val)
{
	unsigned int storedValue = 0;
	if (!RegistryIni::ReadUnsignedInt(getRegistryIniRoot(root), path.str(), key.str(), storedValue))
	{
		return FALSE;
	}

	val = storedValue;
	return TRUE;
}

Bool setStringInRegistry( HKEY root, AsciiString path, AsciiString key, AsciiString val)
{
	return RegistryIni::WriteString(getRegistryIniRoot(root), path.str(), key.str(), val.str()) ? TRUE : FALSE;
}

Bool setUnsignedIntInRegistry( HKEY root, AsciiString path, AsciiString key, UnsignedInt val)
{
	return RegistryIni::WriteUnsignedInt(getRegistryIniRoot(root), path.str(), key.str(), val) ? TRUE : FALSE;
}

// GeneralsX @feature felipebraz 14/03/2026 Linux registry env mapping for base Generals
// Maps registry key lookups to environment variables so Linux users can override
// install path and language without a Windows registry.
// Pattern: CNC_GENERALS_<UPPERCASED_KEY>
static Bool getEnvVar(const char *prefix, AsciiString key, AsciiString& val)
{
	char envName[256] = { 0 };
	const char* keyStr = key.str();
	int prefixLen = strlen(prefix);

	if (prefixLen + strlen(keyStr) + 1 > sizeof(envName))
		return FALSE;

	strcpy(envName, prefix);
	for (int i = 0; keyStr[i] != '\0'; ++i)
		envName[prefixLen + i] = toupper((unsigned char)keyStr[i]);
	envName[prefixLen + strlen(keyStr)] = '\0';

	const char* envValue = getenv(envName);
	if (envValue && envValue[0] != '\0')
	{
		val = envValue;
		DEBUG_LOG(("getEnvVar - found %s = %s", envName, envValue));
		return TRUE;
	}

	return FALSE;
}

// GeneralsX @bugfix GitHubCopilot 16/03/2026 Detect language BIGs in configured asset roots, not only CWD.
static Bool doesLanguageBigExist(const char* rootPath, const char* bigFile)
{
	struct stat st;
	char lowerBigFile[256] = { 0 };

	for (int i = 0; bigFile[i] != '\0' && i < static_cast<int>(sizeof(lowerBigFile) - 1); ++i) {
		lowerBigFile[i] = static_cast<char>(tolower(static_cast<unsigned char>(bigFile[i])));
	}

	if (rootPath == nullptr || rootPath[0] == '\0') {
		if (stat(bigFile, &st) == 0 || stat(lowerBigFile, &st) == 0) {
			return TRUE;
		}
		return FALSE;
	}

	AsciiString fullPath = rootPath;
	const Int len = fullPath.getLength();
	if (len > 0) {
		const char lastChar = fullPath.getCharAt(len - 1);
		if (lastChar != '/' && lastChar != '\\') {
			fullPath.concat('/');
		}
	}
	fullPath.concat(bigFile);

	if (stat(fullPath.str(), &st) == 0) {
		return TRUE;
	}

	AsciiString lowerPath = rootPath;
	const Int lowerLen = lowerPath.getLength();
	if (lowerLen > 0) {
		const char lastChar = lowerPath.getCharAt(lowerLen - 1);
		if (lastChar != '/' && lastChar != '\\') {
			lowerPath.concat('/');
		}
	}
	lowerPath.concat(lowerBigFile);

	return stat(lowerPath.str(), &st) == 0;
}

// GeneralsX @feature felipebraz 14/03/2026 Linux language autodetect for non-English installs
// Detect language from localized BIG files when registry is unavailable.
// This preserves original behavior where installer-provided language drives Data/<lang>/ paths.
static Bool tryAutoDetectLanguage(AsciiString& val)
{
	// Prefer base Generals BIG names first, then accept ZH names for shared installs.
	const struct { const char *bigFile; const char *language; } candidates[] = {
		{ "Brazilian.big",   "brazilian" },
		{ "English.big",     "english"   },
		{ "German.big",      "german"    },
		{ "French.big",      "french"    },
		{ "Spanish.big",     "spanish"   },
		{ "Chinese.big",     "chinese"   },
		{ "Korean.big",      "korean"    },
		{ "Polish.big",      "polish"    },
		{ "BrazilianZH.big", "brazilian" },
		{ "EnglishZH.big",   "english"   },
		{ "GermanZH.big",    "german"    },
		{ "FrenchZH.big",    "french"    },
		{ "SpanishZH.big",   "spanish"   },
		{ "ChineseZH.big",   "chinese"   },
		{ "KoreanZH.big",    "korean"    },
		{ "PolishZH.big",    "polish"    },
		{ nullptr,            nullptr      }
	};

	const char* searchRoots[] = {
		getenv("CNC_GENERALS_PATH"),
		getenv("CNC_GENERALS_ZH_PATH"),
		getenv("CNC_GENERALS_INSTALLPATH"),
		nullptr
	};

	for (int i = 0; candidates[i].bigFile != nullptr; ++i)
	{
		for (int rootIndex = 0; rootIndex < static_cast<int>(ARRAY_SIZE(searchRoots)); ++rootIndex) {
			if (doesLanguageBigExist(searchRoots[rootIndex], candidates[i].bigFile)) {
				val = candidates[i].language;
				DEBUG_LOG(("tryAutoDetectLanguage - detected language '%s' from %s (root=%s)",
						candidates[i].language,
						candidates[i].bigFile,
						searchRoots[rootIndex] != nullptr ? searchRoots[rootIndex] : "<cwd>"));
				return TRUE;
			}
		}
	}

	return FALSE;
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	if (getEnvVar("CNC_GENERALS_", key, val))
		return TRUE;

	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
	fullPath.concat(path);
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	if (getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	if (key == "Language")
	{
		if (tryAutoDetectLanguage(val))
			return TRUE;
	}

	return FALSE;
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	if (getEnvVar("CNC_GENERALS_", key, val))
		return TRUE;

	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
	fullPath.concat(path);
	if (getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	if (key == "Language")
	{
		if (tryAutoDetectLanguage(val))
			return TRUE;
	}

	return FALSE;
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
	fullPath.concat(path);
	if (getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

#else // Windows implementation

Bool  getStringFromRegistry(HKEY root, AsciiString path, AsciiString key, AsciiString& val)
{
	HKEY handle;
	unsigned char buffer[256];
	unsigned long size = 256;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = (char *)buffer;
		return TRUE;
	}

	return FALSE;
}

Bool getUnsignedIntFromRegistry(HKEY root, AsciiString path, AsciiString key, UnsignedInt& val)
{
	HKEY handle;
	unsigned char buffer[4];
	unsigned long size = 4;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = *(UnsignedInt *)buffer;
		return TRUE;
	}

	return FALSE;
}

Bool setStringInRegistry( HKEY root, AsciiString path, AsciiString key, AsciiString val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_SZ;
		size = val.getLength()+1;
		returnValue = RegSetValueEx(handle, key.str(), 0, type, (unsigned char *)val.str(), size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

Bool setUnsignedIntInRegistry( HKEY root, AsciiString path, AsciiString key, UnsignedInt val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_DWORD;
		size = 4;
		returnValue = RegSetValueEx(handle, key.str(), 0, type, (unsigned char *)&val, size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";

	fullPath.concat(path);
	DEBUG_LOG(("GetStringFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
#if RTS_GENERALS
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.concat(path);
	DEBUG_LOG(("GetStringFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val);
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
#if RTS_GENERALS
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.concat(path);
	DEBUG_LOG(("GetUnsignedIntFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val);
}

#endif // _UNIX

// TheSuperHackers @build felipebraz 11/02/2026 Phase 1.5 - Linux port
// These functions work on both platforms - call registry functions which return FALSE on Linux
AsciiString GetRegistryLanguage()
{
	static Bool cached = FALSE;
	// NOTE: static causes a memory leak, but we have to keep it because the value is cached.
	static AsciiString val = "english";
	if (cached) {
		return val;
	} else {
		cached = TRUE;
	}

	GetStringFromRegistry("", "Language", val);
	return val;
}

AsciiString GetRegistryGameName()
{
	AsciiString val = "GeneralsMPTest";
	GetStringFromRegistry("", "SKU", val);
	return val;
}

UnsignedInt GetRegistryVersion()
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "Version", val);
	return val;
}

UnsignedInt GetRegistryMapPackVersion()
{
	UnsignedInt val = 65536;
	GetUnsignedIntFromRegistry("", "MapPackVersion", val);
	return val;
}
