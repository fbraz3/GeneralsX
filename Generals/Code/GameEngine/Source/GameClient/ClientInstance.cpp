/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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
#include "PreRTS.h"
#include "GameClient/ClientInstance.h"

#define GENERALS_GUID "685EAFF2-3216-4265-B047-251C5F4B82F3"

namespace rts
{
#ifdef _WIN32
HANDLE ClientInstance::s_mutexHandle = NULL;
#else
void* ClientInstance::s_mutexHandle = NULL;
#endif
UnsignedInt ClientInstance::s_instanceIndex = 0;

#if defined(RTS_MULTI_INSTANCE)
Bool ClientInstance::s_isMultiInstance = true;
#else
Bool ClientInstance::s_isMultiInstance = false;
#endif

#ifdef _WIN32
bool ClientInstance::initialize()
{
	if (isInitialized())
	{
		return true;
	}

	// Create a mutex with a unique name to Generals in order to determine if our app is already running.
	// WARNING: DO NOT use this number for any other application except Generals.
	while (true)
	{
		if (isMultiInstance())
		{
			std::string guidStr = getFirstInstanceName();
			if (s_instanceIndex > 0u)
			{
				char idStr[33];
				itoa(s_instanceIndex, idStr, 10);
				guidStr.push_back('-');
				guidStr.append(idStr);
			}
			s_mutexHandle = CreateMutex(NULL, FALSE, guidStr.c_str());
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				if (s_mutexHandle != NULL)
				{
					CloseHandle(s_mutexHandle);
					s_mutexHandle = NULL;
				}
				// Try again with a new instance.
				++s_instanceIndex;
				continue;
			}
		}
		else
		{
			s_mutexHandle = CreateMutex(NULL, FALSE, getFirstInstanceName());
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				if (s_mutexHandle != NULL)
				{
					CloseHandle(s_mutexHandle);
					s_mutexHandle = NULL;
				}
				return false;
			}
		}
		break;
	}

	return true;
}
#else
bool ClientInstance::initialize()
{
	// macOS version - simplified instance management without Windows mutexes
	if (isInitialized())
	{
		return true;
	}
	
	s_mutexHandle = (void*)1; // dummy handle to indicate initialized
	return true;
}
#endif

bool ClientInstance::isInitialized()
{
	return s_mutexHandle != NULL;
}

bool ClientInstance::isMultiInstance()
{
	return s_isMultiInstance;
}

void ClientInstance::setMultiInstance(bool v)
{
	if (isInitialized())
	{
		DEBUG_CRASH(("ClientInstance::setMultiInstance(%d) - cannot set multi instance after initialization", (int)v));
		return;
	}
	s_isMultiInstance = v;
}

void ClientInstance::skipPrimaryInstance()
{
	if (isInitialized())
	{
		DEBUG_CRASH(("ClientInstance::skipPrimaryInstance() - cannot skip primary instance after initialization"));
		return;
	}
	s_instanceIndex = 1;
}

UnsignedInt ClientInstance::getInstanceIndex()
{
	DEBUG_ASSERTLOG(isInitialized(), ("ClientInstance::isInitialized() failed"));
	return s_instanceIndex;
}

UnsignedInt ClientInstance::getInstanceId()
{
	return getInstanceIndex() + 1;
}

const char* ClientInstance::getFirstInstanceName()
{
	return GENERALS_GUID;
}

} // namespace rts
