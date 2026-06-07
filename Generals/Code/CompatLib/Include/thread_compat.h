#pragma once

#include <pthread.h>
#include <stdint.h>
typedef pthread_t THREAD_ID;
typedef uint32_t (*start_routine)(void *);

// Avoid conflict with old Dependencies/Utility/Utility/thread_compat.h
// The old header defines int GetCurrentThreadId(), we define THREAD_ID GetCurrentThreadId()
// When both are included, we only declare if not already defined as the old int version
#if !defined(GetCurrentThreadId) || !defined(__THREAD_COMPAT_OLD_INCLUDED)
THREAD_ID GetCurrentThreadId();
#endif

// GeneralsX @feature BenderAI 24/02/2026 Phase 5 - Convert pthread_t to int for compatibility
// Maps pthread_t to unique integers (max 256 threads tracked)
int GetCurrentThreadIdAsInt();

void* CreateThread(void *lpSecure, size_t dwStackSize, start_routine lpStartAddress, void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId);
int TerminateThread(void *hThread, unsigned long dwExitCode);