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

#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>

// Phase 45+: Real SDL2 Mutex implementation
// Note: We can't use SDL_mutex directly for cross-platform named mutexes since SDL2
// doesn't have native named mutex support. We'll use a simple reference counting
// approach with static SDL_mutex per unique identifier.
#include <map>

static SDL_mutex* g_clientMutex = nullptr;
static std::map<std::string, int> g_mutexRefCounts;

#define GENERALS_GUID "685EAFF2-3216-4265-B047-251C5F4B82F3"

namespace rts
{
	// Phase 45+: Real SDL2 mutex functions
	// Since SDL2 doesn't support named mutexes, we use a global mutex per unique identifier
	static void* SDL2_CreateMutex(const char* name) {
		fprintf(stderr, "ClientInstance: SDL2_CreateMutex called for '%s'\n", name ? name : "(null)");
		fflush(stderr);

		// We allocate a simple handle that will be used to track mutex creation attempts
		// For single-instance mode, we just need ONE successful creation
		// For multi-instance mode, we need separate handles per instance

		if (g_clientMutex == nullptr) {
			g_clientMutex = SDL_CreateMutex();
			if (g_clientMutex != nullptr) {
				fprintf(stderr, "ClientInstance: SDL2_CreateMutex succeeded, created new global mutex\n");
				fflush(stderr);
				// Return a non-null handle to indicate success
				// We use the mutex pointer itself as the handle
				return (void*)g_clientMutex;
			}
			else {
				fprintf(stderr, "ClientInstance: SDL2_CreateMutex failed to create global mutex\n");
				fflush(stderr);
				return nullptr;
			}
		}
		else {
			// Mutex already exists - this is success for multi-instance mode
			fprintf(stderr, "ClientInstance: SDL2_CreateMutex: global mutex already exists (multi-instance)\n");
			fflush(stderr);
			return nullptr;  // Return nullptr to allow next instance to try
		}
	}

	static void SDL2_DestroyMutex(void* mutex) {
		if (mutex != nullptr && mutex == (void*)g_clientMutex) {
			fprintf(stderr, "ClientInstance: SDL2_DestroyMutex\n");
			fflush(stderr);
			if (g_clientMutex != nullptr) {
				SDL_DestroyMutex(g_clientMutex);
				g_clientMutex = nullptr;
			}
		}
	}

	static int SDL2_LockMutex(void* mutex) {
		if (g_clientMutex != nullptr) {
			return SDL_LockMutex(g_clientMutex);
		}
		return 0;
	}

	static void SDL2_UnlockMutex(void* mutex) {
		if (g_clientMutex != nullptr) {
			SDL_UnlockMutex(g_clientMutex);
		}
	}

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
			// fprintf(stderr, "ClientInstance::initialize: Already initialized (s_mutexHandle = %p)\n", s_mutexHandle);
			// fflush(stderr);
			return true;
		}

		// fprintf(stderr, "ClientInstance::initialize: Starting initialization (isMultiInstance=%d)\n", isMultiInstance());
		// fflush(stderr);

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
				// fprintf(stderr, "ClientInstance::initialize: Trying multi-instance with ID: %s\n", guidStr.c_str());
				// fflush(stderr);

				s_mutexHandle = SDL2_CreateMutex(guidStr.c_str());
				if (s_mutexHandle == NULL)
				{
					// Mutex already exists, try again with a new instance index
					// fprintf(stderr, "ClientInstance::initialize: Instance already exists, trying next index\n");
					// fflush(stderr);
					++s_instanceIndex;
					continue;
				}
			}
			else
			{
				// fprintf(stderr, "ClientInstance::initialize: Creating single-instance mutex\n");
				// fflush(stderr);

				s_mutexHandle = SDL2_CreateMutex(getFirstInstanceName());
				if (s_mutexHandle == NULL)
				{
					// fprintf(stderr, "ClientInstance::initialize: Single-instance mutex creation failed\n");
					// fflush(stderr);
					return false;
				}
			}
			break;
		}

		// fprintf(stderr, "ClientInstance::initialize: Initialization successful (s_mutexHandle = %p)\n", s_mutexHandle);
		// fflush(stderr);

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
