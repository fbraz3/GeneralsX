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
// Phase 39.4: Temporarily disable include - header path issue to be resolved in CMakeLists configuration
// #include "WWVegas/win32_thread_compat.h"

// SDL2 Mutex stubs (Phase 39.4 - implementation is no-op for cross-platform compatibility)
inline void* SDL2_CreateMutex(const char* name) { return nullptr; }
inline void SDL2_DestroyMutex(void* mutex) { }
inline int SDL2_LockMutex(void* mutex) { return 0; }
inline void SDL2_UnlockMutex(void* mutex) { }

#define GENERALS_GUID "685EAFF2-3216-4265-B047-251C5F4B82F3"

namespace rts
{
void* ClientInstance::s_mutexHandle = nullptr;  // Phase 39.4: Opaque handle to SDL2_Mutex
UnsignedInt ClientInstance::s_instanceIndex = 0;

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
		if (isMultiInstance())
		{
			std::string guidStr = getFirstInstanceName();
			if (s_instanceIndex > 0u)
			{
				char idStr[33];
				snprintf(idStr, sizeof(idStr), "%u", s_instanceIndex);
				guidStr.push_back('-');
				guidStr.append(idStr);
			}
			s_mutexHandle = SDL2_CreateMutex(guidStr.c_str());
			if (s_mutexHandle == NULL)
			{
				// Mutex already exists, try again with a new instance index
				++s_instanceIndex;
				continue;
			}
		}
		else
		{
			s_mutexHandle = SDL2_CreateMutex(getFirstInstanceName());
			if (s_mutexHandle == NULL)
			{
				return false;
			}
		}
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
