/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
**
**  FILE: win32_thread_compat.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Cross-Platform Threading & Synchronization Compatibility Layer
**
**  DESCRIPTION:
**    Provides Windows-compatible threading and synchronization primitives
**    that work on macOS, Linux, and Windows using std::thread and pthreads.
**    Abstracts away platform differences for thread creation, mutexes,
**    condition variables, and critical sections.
**
**  ARCHITECTURE:
**    Phase 04: Threading & Synchronization Layer
**    - Layer 1: Core Compatibility (this file)
**    - Layer 2: Platform Backend (std::thread, pthread_mutex, pthread_cond)
**    - Layer 3: Game-Specific Extensions (GeneralsMD/Code/)
**
******************************************************************************/

#ifndef WIN32_THREAD_COMPAT_H
#define WIN32_THREAD_COMPAT_H

#include <stdint.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <pthread.h>
#include <unistd.h>

/* ============================================================================
 * THREAD CREATION AND MANAGEMENT
 * ============================================================================ */

/**
 * Cross-platform thread handle abstraction
 * On Windows: HANDLE, on POSIX: pthread_t
 */
typedef pthread_t SDL2_ThreadHandle;
typedef pthread_t SDL2_ThreadID;

/**
 * Thread function signature - returns void, takes void* argument
 */
typedef void (*SDL2_ThreadFunction)(void* arg);

/**
 * Create a new thread with the given function and argument
 * 
 * @param func          Thread function to execute
 * @param arg           Argument to pass to thread function
 * @param name          Optional thread name (for debugging)
 * @param stack_size    Stack size in bytes (0 for default)
 * @return              Thread handle, NULL on error
 *
 * USAGE:
 *   void thread_func(void* arg) { printf("Thread running\n"); }
 *   SDL2_ThreadHandle h = SDL2_CreateThread(thread_func, NULL, "worker", 0);
 *   SDL2_WaitThread(h);
 */
SDL2_ThreadHandle SDL2_CreateThread(
    SDL2_ThreadFunction func,
    void* arg,
    const char* name,
    size_t stack_size
);

/**
 * Wait for thread completion (join)
 * 
 * @param thread        Thread handle from SDL2_CreateThread
 * @return              0 on success, -1 on error
 */
int SDL2_WaitThread(SDL2_ThreadHandle thread);

/**
 * Detach thread (allow it to run independently)
 * 
 * @param thread        Thread handle from SDL2_CreateThread
 * @return              0 on success, -1 on error
 */
int SDL2_DetachThread(SDL2_ThreadHandle thread);

/**
 * Get current thread ID
 * 
 * @return              Current thread ID
 */
SDL2_ThreadID SDL2_GetCurrentThreadID(void);

/**
 * Sleep the current thread for milliseconds
 * 
 * @param milliseconds  Time to sleep
 */
void SDL2_Sleep(unsigned int milliseconds);

/**
 * Yield to other threads
 */
void SDL2_YieldThread(void);

/* ============================================================================
 * MUTEX (Mutual Exclusion) - For inter-process locking
 * ============================================================================ */

/**
 * Mutex handle abstraction
 */
typedef pthread_mutex_t* SDL2_Mutex;

/**
 * Create a new mutex
 * 
 * @param name          Optional mutex name for system-wide visibility
 * @return              Mutex handle, NULL on error
 */
SDL2_Mutex SDL2_CreateMutex(const char* name);

/**
 * Destroy a mutex
 * 
 * @param mutex         Mutex handle from SDL2_CreateMutex
 */
void SDL2_DestroyMutex(SDL2_Mutex mutex);

/**
 * Lock a mutex (wait if locked by another thread)
 * 
 * @param mutex         Mutex handle
 * @param timeout_ms    Timeout in milliseconds (-1 for infinite)
 * @return              0 on success, -1 on timeout/error
 */
int SDL2_LockMutex(SDL2_Mutex mutex, int timeout_ms);

/**
 * Unlock a mutex
 * 
 * @param mutex         Mutex handle
 * @return              0 on success, -1 on error
 */
int SDL2_UnlockMutex(SDL2_Mutex mutex);

/**
 * Mutex RAII lock guard for automatic lock/unlock
 * 
 * USAGE:
 *   {
 *       SDL2_MutexLock lock(mutex);
 *       // Critical section - automatically unlocked on scope exit
 *   }
 */
class SDL2_MutexLock
{
    SDL2_Mutex mutex;
    bool locked;

public:
    SDL2_MutexLock(SDL2_Mutex m, int timeout_ms = -1);
    ~SDL2_MutexLock();
    
    bool Failed() const { return !locked; }

private:
    SDL2_MutexLock(const SDL2_MutexLock&) = delete;
    SDL2_MutexLock& operator=(const SDL2_MutexLock&) = delete;
};

/* ============================================================================
 * CRITICAL SECTION - Faster than mutex, thread-local only
 * ============================================================================ */

/**
 * Critical section handle abstraction
 */
typedef pthread_mutex_t SDL2_CriticalSection;

/**
 * Create a new critical section
 * 
 * @return              Critical section (stack-allocated)
 * 
 * USAGE:
 *   SDL2_CriticalSection cs = SDL2_CreateCriticalSection();
 */
SDL2_CriticalSection SDL2_CreateCriticalSection(void);

/**
 * Destroy a critical section
 * 
 * @param cs            Critical section to destroy
 */
void SDL2_DestroyCriticalSection(SDL2_CriticalSection* cs);

/**
 * Enter a critical section (lock)
 * 
 * @param cs            Critical section pointer
 */
void SDL2_EnterCriticalSection(SDL2_CriticalSection* cs);

/**
 * Leave a critical section (unlock)
 * 
 * @param cs            Critical section pointer
 */
void SDL2_LeaveCriticalSection(SDL2_CriticalSection* cs);

/**
 * Critical section RAII lock guard
 * 
 * USAGE:
 *   {
 *       SDL2_CriticalSectionLock lock(&cs);
 *       // Critical section - automatically unlocked on scope exit
 *   }
 */
class SDL2_CriticalSectionLock
{
    SDL2_CriticalSection* cs;

public:
    SDL2_CriticalSectionLock(SDL2_CriticalSection* c);
    ~SDL2_CriticalSectionLock();

private:
    SDL2_CriticalSectionLock(const SDL2_CriticalSectionLock&) = delete;
    SDL2_CriticalSectionLock& operator=(const SDL2_CriticalSectionLock&) = delete;
};

/* ============================================================================
 * CONDITION VARIABLE - For thread synchronization
 * ============================================================================ */

/**
 * Condition variable handle abstraction
 */
typedef pthread_cond_t SDL2_ConditionVariable;

/**
 * Create a new condition variable
 * 
 * @return              Condition variable
 */
SDL2_ConditionVariable SDL2_CreateConditionVariable(void);

/**
 * Destroy a condition variable
 * 
 * @param cond          Condition variable pointer
 */
void SDL2_DestroyConditionVariable(SDL2_ConditionVariable* cond);

/**
 * Wait on a condition variable (releases lock until signaled)
 * 
 * @param cond          Condition variable pointer
 * @param cs            Critical section holding the lock
 * @param timeout_ms    Timeout in milliseconds (-1 for infinite)
 * @return              0 on signal, -1 on timeout/error
 *
 * NOTE: Caller must hold lock on critical section before calling
 */
int SDL2_ConditionWait(
    SDL2_ConditionVariable* cond,
    SDL2_CriticalSection* cs,
    int timeout_ms
);

/**
 * Signal one waiting thread
 * 
 * @param cond          Condition variable pointer
 * @return              0 on success, -1 on error
 */
int SDL2_ConditionSignal(SDL2_ConditionVariable* cond);

/**
 * Signal all waiting threads
 * 
 * @param cond          Condition variable pointer
 * @return              0 on success, -1 on error
 */
int SDL2_ConditionBroadcast(SDL2_ConditionVariable* cond);

/* ============================================================================
 * SEMAPHORE - Counting synchronization primitive
 * ============================================================================ */

/**
 * Semaphore handle abstraction
 */
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
} SDL2_Semaphore_Internal;
typedef SDL2_Semaphore_Internal* SDL2_Semaphore;

/**
 * Create a new semaphore
 * 
 * @param initial_count Initial count value
 * @param max_count     Maximum count value (-1 for unlimited)
 * @return              Semaphore handle, NULL on error
 */
SDL2_Semaphore SDL2_CreateSemaphore(int initial_count, int max_count);

/**
 * Destroy a semaphore
 * 
 * @param sem           Semaphore handle
 */
void SDL2_DestroySemaphore(SDL2_Semaphore sem);

/**
 * Wait on a semaphore (decrement count)
 * 
 * @param sem           Semaphore handle
 * @param timeout_ms    Timeout in milliseconds (-1 for infinite)
 * @return              0 on success, -1 on timeout/error
 */
int SDL2_WaitSemaphore(SDL2_Semaphore sem, int timeout_ms);

/**
 * Post to a semaphore (increment count)
 * 
 * @param sem           Semaphore handle
 * @param count         Number to increment by
 * @return              0 on success, -1 on error
 */
int SDL2_PostSemaphore(SDL2_Semaphore sem, int count);

/**
 * Get current semaphore count
 * 
 * @param sem           Semaphore handle
 * @return              Current count value
 */
int SDL2_GetSemaphoreValue(SDL2_Semaphore sem);

#endif /* WIN32_THREAD_COMPAT_H */
