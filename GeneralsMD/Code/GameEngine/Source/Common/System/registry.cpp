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
// Author: Matthew D. Campbell, December 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Registry.h"

#if defined(_WIN32)


Bool  getStringFromRegistry(HKEY root, AsciiString path, AsciiString key, AsciiString& val)
{
	HKEY handle;
	unsigned char buffer[256];
	unsigned long size = 256;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.str(), NULL, &type, (unsigned char *) &buffer, &size);
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
		returnValue = RegQueryValueEx(handle, key.str(), NULL, &type, (unsigned char *) &buffer, &size);
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

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL )) == ERROR_SUCCESS)
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

	if ((returnValue = RegCreateKeyEx( root, path.str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL )) == ERROR_SUCCESS)
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
	if (getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getStringFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val);
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";

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
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";

	fullPath.concat(path);
	DEBUG_LOG(("GetUnsignedIntFromRegistry - looking in %s for key %s", fullPath.str(), key.str()));
	if (getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.str(), key.str(), val))
	{
		return TRUE;
	}

	return getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.str(), key.str(), val);
}

AsciiString GetRegistryLanguage(void)
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

#else

#include "Common/GlobalData.h"

#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{
	struct RegStore {
		std::unordered_map<std::string, std::string> strings;
		std::unordered_map<std::string, UnsignedInt> dwords;
		bool loaded = false;
	};

	static std::string toLower(std::string s)
	{
		for (char &ch : s) {
			ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
		}
		return s;
	}

	static void rtrim(std::string &s)
	{
		while (!s.empty()) {
			char c = s.back();
			if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
				s.pop_back();
				continue;
			}
			break;
		}
	}

	static bool startsWith(const std::string &s, const char *prefix)
	{
		const size_t n = std::strlen(prefix);
		return s.size() >= n && std::memcmp(s.data(), prefix, n) == 0;
	}

	static bool parseQuoted(const std::string &s, size_t &i, std::string &out)
	{
		if (i >= s.size() || s[i] != '"') {
			return false;
		}
		++i;
		out.clear();
		while (i < s.size()) {
			char c = s[i++];
			if (c == '"') {
				return true;
			}
			out.push_back(c);
		}
		return false;
	}

	static std::string normalizePathKey(const std::string &section, const std::string &name)
	{
		return toLower(section) + "\\" + toLower(name);
	}

	static void loadRegFile(RegStore &lm, RegStore &cu, const char *path)
	{
		if (!path || !*path) {
			return;
		}
		std::FILE *f = std::fopen(path, "rb");
		if (!f) {
			return;
		}

		char lineBuf[2048];
		std::string currentRoot;
		std::string currentSection;
		while (std::fgets(lineBuf, sizeof(lineBuf), f)) {
			std::string line(lineBuf);
			rtrim(line);
			if (line.empty()) {
				continue;
			}
			if (startsWith(line, "Windows Registry Editor")) {
				continue;
			}
			if (line[0] == ';') {
				continue;
			}

			if (line.front() == '[' && line.back() == ']') {
				std::string inside = line.substr(1, line.size() - 2);
				if (startsWith(inside, "HKEY_LOCAL_MACHINE\\")) {
					currentRoot = "HKEY_LOCAL_MACHINE";
					currentSection = inside.substr(std::strlen("HKEY_LOCAL_MACHINE\\"));
				} else if (startsWith(inside, "HKEY_CURRENT_USER\\")) {
					currentRoot = "HKEY_CURRENT_USER";
					currentSection = inside.substr(std::strlen("HKEY_CURRENT_USER\\"));
				} else {
					currentRoot.clear();
					currentSection.clear();
				}
				continue;
			}

			if (currentRoot.empty() || currentSection.empty()) {
				continue;
			}

			size_t i = 0;
			std::string name;
			if (line[i] == '@') {
				name = "";
				++i;
			} else {
				if (!parseQuoted(line, i, name)) {
					continue;
				}
			}
			while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
				++i;
			}
			if (i >= line.size() || line[i] != '=') {
				continue;
			}
			++i;
			while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
				++i;
			}

			RegStore *store = (currentRoot == "HKEY_LOCAL_MACHINE") ? &lm : &cu;
			const std::string mapKey = normalizePathKey(currentSection, name);

			if (i < line.size() && line[i] == '"') {
				std::string value;
				if (!parseQuoted(line, i, value)) {
					continue;
				}
				store->strings[mapKey] = value;
				continue;
			}
			if (startsWith(line.substr(i), "dword:")) {
				i += std::strlen("dword:");
				UnsignedInt v = 0;
				for (; i < line.size(); ++i) {
					char c = line[i];
					unsigned int digit;
					if (c >= '0' && c <= '9') digit = static_cast<unsigned int>(c - '0');
					else if (c >= 'a' && c <= 'f') digit = static_cast<unsigned int>(10 + (c - 'a'));
					else if (c >= 'A' && c <= 'F') digit = static_cast<unsigned int>(10 + (c - 'A'));
					else break;
					v = (v << 4) | digit;
				}
				store->dwords[mapKey] = v;
				continue;
			}
		}

		std::fclose(f);
	}

	static std::pair<RegStore &, RegStore &> stores()
	{
		static RegStore lm;
		static RegStore cu;
		if (!lm.loaded || !cu.loaded) {
			lm.loaded = true;
			cu.loaded = true;

			const char *lmPath = std::getenv("GENERALSX_LOCAL_MACHINE_REG");
			const char *cuPath = std::getenv("GENERALSX_CURRENT_USER_REG");
			loadRegFile(lm, cu, lmPath ? lmPath : "assets/local_machine.reg");
			loadRegFile(lm, cu, cuPath ? cuPath : "assets/current_user.reg");

			if (TheGlobalData) {
				AsciiString userReg = TheGlobalData->getPath_UserData();
				userReg.concat("registry_user.reg");
				loadRegFile(lm, cu, userReg.str());
			}
		}
		return {lm, cu};
	}

	static bool getStringFromStore(const RegStore &store, const AsciiString &path, const AsciiString &key, AsciiString &val)
	{
		const std::string mapKey = normalizePathKey(path.str(), key.str());
		auto it = store.strings.find(mapKey);
		if (it == store.strings.end()) {
			return false;
		}
		val = it->second.c_str();
		return true;
	}

	static bool getDwordFromStore(const RegStore &store, const AsciiString &path, const AsciiString &key, UnsignedInt &val)
	{
		const std::string mapKey = normalizePathKey(path.str(), key.str());
		auto it = store.dwords.find(mapKey);
		if (it == store.dwords.end()) {
			return false;
		}
		val = it->second;
		return true;
	}

	static void persistCurrentUserAppend(const AsciiString &fullPath, const AsciiString &key, const char *line)
	{
		if (!TheGlobalData) {
			return;
		}
		AsciiString userReg = TheGlobalData->getPath_UserData();
		userReg.concat("registry_user.reg");

		std::FILE *f = std::fopen(userReg.str(), "ab");
		if (!f) {
			return;
		}
		std::fprintf(f, "\n[HKEY_CURRENT_USER\\%s]\n%s\n", fullPath.str(), line);
		std::fclose(f);
	}
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
	fullPath.concat(path);

	auto [lm, cu] = stores();
	if (getStringFromStore(lm, fullPath, key, val)) {
		return TRUE;
	}
	return getStringFromStore(cu, fullPath, key, val) ? TRUE : FALSE;
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
	fullPath.concat(path);

	auto [lm, cu] = stores();
	if (getStringFromStore(lm, fullPath, key, val)) {
		return TRUE;
	}
	return getStringFromStore(cu, fullPath, key, val) ? TRUE : FALSE;
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
	fullPath.concat(path);

	auto [lm, cu] = stores();
	if (getDwordFromStore(lm, fullPath, key, val)) {
		return TRUE;
	}
	return getDwordFromStore(cu, fullPath, key, val) ? TRUE : FALSE;
}

Bool SetStringInRegistry(AsciiString path, AsciiString key, AsciiString val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
	fullPath.concat(path);

	auto [lm, cu] = stores();
	const std::string mapKey = normalizePathKey(fullPath.str(), key.str());
	cu.strings[mapKey] = val.str();

	AsciiString line;
	line.format("\"%s\"=\"%s\"", key.str(), val.str());
	persistCurrentUserAppend(fullPath, key, line.str());
	return TRUE;
}

Bool SetUnsignedIntInRegistry(AsciiString path, AsciiString key, UnsignedInt val)
{
	AsciiString fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
	fullPath.concat(path);

	auto [lm, cu] = stores();
	const std::string mapKey = normalizePathKey(fullPath.str(), key.str());
	cu.dwords[mapKey] = val;

	AsciiString line;
	line.format("\"%s\"=dword:%08x", key.str(), (unsigned int)val);
	persistCurrentUserAppend(fullPath, key, line.str());
	return TRUE;
}

AsciiString GetRegistryLanguage(void)
{
	static Bool cached = FALSE;
	static AsciiString val = "english";
	if (!cached) {
		cached = TRUE;
		const char *env = std::getenv("GENERALSX_LANGUAGE");
		if (env && *env) {
			val = env;
		} else {
			GetStringFromRegistry("", "Language", val);
		}
	}
	return val;
}

AsciiString GetRegistryGameName(void)
{
	AsciiString val = "GeneralsMPTest";
	const char *env = std::getenv("GENERALSX_SKU");
	if (env && *env) {
		val = env;
	} else {
		GetStringFromRegistry("", "SKU", val);
	}
	return val;
}

UnsignedInt GetRegistryVersion(void)
{
	UnsignedInt val = 65536;
	const char *env = std::getenv("GENERALSX_VERSION");
	if (env && *env) {
		val = (UnsignedInt)std::strtoul(env, nullptr, 0);
	} else {
		GetUnsignedIntFromRegistry("", "Version", val);
	}
	return val;
}

UnsignedInt GetRegistryMapPackVersion(void)
{
	UnsignedInt val = 65536;
	const char *env = std::getenv("GENERALSX_MAPPACKVERSION");
	if (env && *env) {
		val = (UnsignedInt)std::strtoul(env, nullptr, 0);
	} else {
		GetUnsignedIntFromRegistry("", "MapPackVersion", val);
	}
	return val;
}

#endif
