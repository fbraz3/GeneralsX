#include "thread_compat.h"
#include <map>
#include <mutex>

// GeneralsX @feature BenderAI 24/02/2026 Phase 5 - Thread ID mapper for legacy int-based APIs
static std::map<pthread_t, int> thread_id_map;
static std::mutex thread_id_map_mutex;
static int next_thread_id = 1;

THREAD_ID GetCurrentThreadId()
{
	pthread_t thread_id = pthread_self();
	return (THREAD_ID)thread_id;
}

int GetCurrentThreadIdAsInt()
{
	// GeneralsX @feature BenderAI 24/02/2026 Phase 5 - Convert pthread_t to unique int for legacy code
	pthread_t current = pthread_self();
	
	std::lock_guard<std::mutex> lock(thread_id_map_mutex);
	
	auto it = thread_id_map.find(current);
	if (it != thread_id_map.end()) {
		return it->second;
	}
	
	// Add new thread mapping
	int new_id = next_thread_id++;
	if (new_id < 0) next_thread_id = 1;  // Prevent overflow
	thread_id_map[current] = new_id;
	return new_id;
}

struct thread_data
{
	start_routine start_routine_arg;
	void *lpParameter;
};

void* thread_start(void *arg)
{
	thread_data *data = (thread_data*)arg;
	data->start_routine_arg(data->lpParameter);
	delete data;
	return NULL;
}

void* CreateThread(void *lpSecure, size_t dwStackSize, start_routine lpStartAddress, void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId)
{
	pthread_t thread_id;
	thread_data *data = new thread_data;
	data->start_routine_arg = lpStartAddress;
	data->lpParameter = lpParameter;
	pthread_create(&thread_id, NULL, thread_start, data);
	return (void*)thread_id;
}

int TerminateThread(void *hThread, unsigned long dwExitCode)
{
	return pthread_cancel((pthread_t)hThread);
}