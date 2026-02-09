#pragma once

#include <pthread.h>
#include <stdint.h>
typedef pthread_t THREAD_ID;
typedef uint32_t (*start_routine)(void *);

THREAD_ID GetCurrentThreadId();
void* CreateThread(void *lpSecure, size_t dwStackSize, start_routine lpStartAddress, void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId);
int TerminateThread(void *hThread, unsigned long dwExitCode);