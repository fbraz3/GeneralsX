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

#if !defined(_WIN32)
#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#define GENERALS_GUID "685EAFF2-3216-4265-B047-251C5F4B82F3"

namespace rts
{
void* ClientInstance::s_mutexHandle = nullptr;
UnsignedInt ClientInstance::s_instanceIndex = 0;

#if !defined(_WIN32)
static int g_clientInstanceLockFd = -1;

static bool tryAcquireInstanceLock(const std::string& name)
{
	if (g_clientInstanceLockFd != -1) {
		return true;
	}

	std::string lockPath = "/tmp/GeneralsX_";
	lockPath += name;
	lockPath += ".lock";

	int fd = open(lockPath.c_str(), O_CREAT | O_RDWR, 0600);
	if (fd == -1) {
		return false;
	}

	if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
		const int err = errno;
		close(fd);
		if (err == EWOULDBLOCK) {
			return false;
		}
		return false;
	}

	g_clientInstanceLockFd = fd;
	return true;
}
#endif

#if defined(RTS_MULTI_INSTANCE)
Bool ClientInstance::s_isMultiInstance = true;
#else
Bool ClientInstance::s_isMultiInstance = false;
#endif

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
		std::string guidStr = getFirstInstanceName();
		if (s_instanceIndex > 0u)
		{
			char idStr[33];
			snprintf(idStr, sizeof(idStr), "%u", s_instanceIndex);
			guidStr.push_back('-');
			guidStr.append(idStr);
		}

	#if defined(_WIN32)
		HANDLE mutexHandle = CreateMutex(NULL, FALSE, guidStr.c_str());
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			if (mutexHandle != NULL)
			{
				CloseHandle(mutexHandle);
			}
			if (isMultiInstance())
			{
				++s_instanceIndex;
				continue;
			}
			return false;
		}
		s_mutexHandle = (void*)mutexHandle;
	#else
		if (!tryAcquireInstanceLock(guidStr))
		{
			if (isMultiInstance())
			{
				++s_instanceIndex;
				continue;
			}
			return false;
		}
		s_mutexHandle = (void*)1;
	#endif
		break;
	}

	return true;
}

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
