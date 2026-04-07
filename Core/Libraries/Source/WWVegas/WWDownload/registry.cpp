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

// Registry.cpp
// Simple interface for storing/retrieving registry values
// Author: Matthew D. Campbell, December 2001

#include "Registry.h"
#include <string>
#include "registryini.h"
#include "win.h"

#ifdef _WIN32
// Windows: Full registry implementation

bool  getStringFromRegistry(HKEY root, std::string path, std::string key, std::string& val)
{
	HKEY handle;
	unsigned char buffer[256];
	unsigned long size = 256;
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.c_str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.c_str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = (char *)buffer;
		return true;
	}

	return false;
}

bool getUnsignedIntFromRegistry(HKEY root, std::string path, std::string key, unsigned int& val)
{
	HKEY handle;
	unsigned long buffer;
	unsigned long size = sizeof(buffer);
	unsigned long type;
	int returnValue;

	if ((returnValue = RegOpenKeyEx( root, path.c_str(), 0, KEY_READ, &handle )) == ERROR_SUCCESS)
	{
		returnValue = RegQueryValueEx(handle, key.c_str(), nullptr, &type, (unsigned char *) &buffer, &size);
		RegCloseKey( handle );
	}

	if (returnValue == ERROR_SUCCESS)
	{
		val = buffer;
		return true;
	}

	return false;
}

bool setStringInRegistry( HKEY root, std::string path, std::string key, std::string val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.c_str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_SZ;
		size = val.length()+1;
		returnValue = RegSetValueEx(handle, key.c_str(), 0, type, (unsigned char *)val.c_str(), size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

bool setUnsignedIntInRegistry( HKEY root, std::string path, std::string key, unsigned int val)
{
	HKEY handle;
	unsigned long type;
	unsigned long returnValue;
	int size;
	char lpClass[] = "REG_NONE";

	if ((returnValue = RegCreateKeyEx( root, path.c_str(), 0, lpClass, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &handle, nullptr )) == ERROR_SUCCESS)
	{
		type = REG_DWORD;
		size = 4;
		returnValue = RegSetValueEx(handle, key.c_str(), 0, type, (unsigned char *)&val, size);
		RegCloseKey( handle );
	}

	return (returnValue == ERROR_SUCCESS);
}

bool GetStringFromRegistry(std::string path, std::string key, std::string& val)
{
#if RTS_GENERALS
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.append(path);
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.c_str(), key.c_str(), val))
	{
		return true;
	}

	return getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.c_str(), key.c_str(), val);
}

bool GetUnsignedIntFromRegistry(std::string path, std::string key, unsigned int& val)
{
#if RTS_GENERALS
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif

	fullPath.append(path);
	if (getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.c_str(), key.c_str(), val))
	{
		return true;
	}

	return getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.c_str(), key.c_str(), val);
}

bool SetStringInRegistry( std::string path, std::string key, std::string val)
{
#if RTS_GENERALS
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif
	fullPath.append(path);

	// TheSuperHackers @fix bobtista 12/02/2026 Always write to HKCU. Per-user settings belong
	// in HKEY_CURRENT_USER and writes there should always succeed without admin privileges.
	return setStringInRegistry( HKEY_CURRENT_USER, fullPath, key, val );
}

bool SetUnsignedIntInRegistry( std::string path, std::string key, unsigned int val)
{
#if RTS_GENERALS
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	std::string fullPath = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif
	fullPath.append(path);

	// TheSuperHackers @fix bobtista 12/02/2026 Always write to HKCU. Per-user settings belong
	// in HKEY_CURRENT_USER and writes there should always succeed without admin privileges.
	return setUnsignedIntInRegistry( HKEY_CURRENT_USER, fullPath, key, val );
}

#else // _WIN32 - Linux: No registry support

// GeneralsX @feature GitHubCopilot 29/03/2026 Persist WWDownload registry values in registry.ini.
static const char *getRegistryIniRoot(HKEY root)
{
	return root == HKEY_LOCAL_MACHINE ? RegistryIni::LocalMachineRoot() : RegistryIni::CurrentUserRoot();
}

static std::string getProductRegistryPath()
{
#if RTS_GENERALS
	return "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
#elif RTS_ZEROHOUR
	return "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#else
	return "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
#endif
}

bool  getStringFromRegistry(HKEY root, std::string path, std::string key, std::string& val)
{
	return RegistryIni::ReadString(getRegistryIniRoot(root), path.c_str(), key.c_str(), val);
}

bool getUnsignedIntFromRegistry(HKEY root, std::string path, std::string key, unsigned int& val)
{
	return RegistryIni::ReadUnsignedInt(getRegistryIniRoot(root), path.c_str(), key.c_str(), val);
}

bool setStringInRegistry(HKEY root, std::string path, std::string key, std::string val)
{
	return RegistryIni::WriteString(getRegistryIniRoot(root), path.c_str(), key.c_str(), val.c_str());
}

bool setUnsignedIntInRegistry(HKEY root, std::string path, std::string key, unsigned int val)
{
	return RegistryIni::WriteUnsignedInt(getRegistryIniRoot(root), path.c_str(), key.c_str(), val);
}

bool GetStringFromRegistry(std::string path, std::string key, std::string& val)
{
	std::string fullPath = getProductRegistryPath();
	fullPath.append(path);
	if (getStringFromRegistry(HKEY_CURRENT_USER, fullPath.c_str(), key.c_str(), val))
	{
		return true;
	}

	return getStringFromRegistry(HKEY_LOCAL_MACHINE, fullPath.c_str(), key.c_str(), val);
}

bool GetUnsignedIntFromRegistry(std::string path, std::string key, unsigned int& val)
{
	std::string fullPath = getProductRegistryPath();
	fullPath.append(path);
	if (getUnsignedIntFromRegistry(HKEY_CURRENT_USER, fullPath.c_str(), key.c_str(), val))
	{
		return true;
	}

	return getUnsignedIntFromRegistry(HKEY_LOCAL_MACHINE, fullPath.c_str(), key.c_str(), val);
}

bool SetStringInRegistry(std::string path, std::string key, std::string val)
{
	std::string fullPath = getProductRegistryPath();
	fullPath.append(path);
	return setStringInRegistry(HKEY_CURRENT_USER, fullPath, key, val);
}

bool SetUnsignedIntInRegistry(std::string path, std::string key, unsigned int val)
{
	std::string fullPath = getProductRegistryPath();
	fullPath.append(path);
	return setUnsignedIntInRegistry(HKEY_CURRENT_USER, fullPath, key, val);
}

void setTeamsPath() {}
void getTeamsPath(std::string &path) { path = ""; }
void setGamePath( unsigned int index, std::string path ) {}
void getGamePath( std::string &path ) {}

#endif // _WIN32


