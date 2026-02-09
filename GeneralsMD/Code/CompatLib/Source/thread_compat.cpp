#include "thread_compat.h"

THREAD_ID GetCurrentThreadId()
{
	pthread_t thread_id = pthread_self();
	return (THREAD_ID)thread_id;
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