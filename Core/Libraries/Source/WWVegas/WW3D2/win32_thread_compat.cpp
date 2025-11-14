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
**  FILE: win32_thread_compat.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Cross-Platform Threading & Synchronization Implementation
**
******************************************************************************/

#include "win32_thread_compat.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * THREAD CREATION AND MANAGEMENT
 * ============================================================================ */

#ifdef _WIN32

SDL2_ThreadHandle SDL2_CreateThread(
    SDL2_ThreadFunction func,
    void* arg,
    const char* name,
    size_t stack_size)
{
    if (!func) {
        fprintf(stderr, "Phase 04: SDL2_CreateThread - NULL function pointer\n");
        return NULL;
    }

    printf("Phase 04: SDL2_CreateThread(%s, stack_size=%zu)\n",
           name ? name : "unnamed", stack_size);

    /* Windows thread creation via _beginthread */
    uintptr_t handle = _beginthread(
        (void(__cdecl*)(void*))func,
        stack_size ? (unsigned)stack_size : 0,
        arg
    );

    if (handle == -1) {
        fprintf(stderr, "Phase 04: _beginthread failed\n");
        return NULL;
    }

    printf("Phase 04: Thread created successfully (handle: %p)\n", (void*)handle);
    return (SDL2_ThreadHandle)handle;
}

int SDL2_WaitThread(SDL2_ThreadHandle thread)
{
    if (!thread) {
        return -1;
    }

    printf("Phase 04: SDL2_WaitThread - waiting for thread (handle: %p)\n", thread);

    DWORD result = WaitForSingleObject(thread, INFINITE);

    if (result == WAIT_OBJECT_0) {
        CloseHandle(thread);
        printf("Phase 04: Thread joined successfully\n");
        return 0;
    }

    fprintf(stderr, "Phase 04: WaitForSingleObject failed (result: %lu)\n", result);
    return -1;
}

int SDL2_DetachThread(SDL2_ThreadHandle thread)
{
    if (!thread) {
        return -1;
    }

    printf("Phase 04: SDL2_DetachThread - detaching thread (handle: %p)\n", thread);

    if (CloseHandle(thread)) {
        printf("Phase 04: Thread detached successfully\n");
        return 0;
    }

    fprintf(stderr, "Phase 04: CloseHandle failed\n");
    return -1;
}

SDL2_ThreadID SDL2_GetCurrentThreadID(void)
{
    SDL2_ThreadID id = GetCurrentThread();
    printf("Phase 04: Current thread ID: %p\n", (void*)id);
    return id;
}

void SDL2_Sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
}

void SDL2_YieldThread(void)
{
    Sleep(0);
}

#else /* !_WIN32 - POSIX implementation */

struct ThreadWrapper {
    SDL2_ThreadFunction func;
    void* arg;
    const char* name;
};

static void* thread_wrapper_func(void* arg)
{
    ThreadWrapper* wrapper = (ThreadWrapper*)arg;

    printf("Phase 04: Thread started (name: %s, id: %p)\n",
           wrapper->name ? wrapper->name : "unnamed",
           (void*)pthread_self());

    /* Call the actual thread function */
    wrapper->func(wrapper->arg);

    printf("Phase 04: Thread exiting (name: %s, id: %p)\n",
           wrapper->name ? wrapper->name : "unnamed",
           (void*)pthread_self());

    delete wrapper;
    return NULL;
}

SDL2_ThreadHandle SDL2_CreateThread(
    SDL2_ThreadFunction func,
    void* arg,
    const char* name,
    size_t stack_size)
{
    if (!func) {
        fprintf(stderr, "Phase 04: SDL2_CreateThread - NULL function pointer\n");
        return (SDL2_ThreadHandle)-1;
    }

    printf("Phase 04: SDL2_CreateThread(%s, stack_size=%zu)\n",
           name ? name : "unnamed", stack_size);

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if (stack_size > 0) {
        pthread_attr_setstacksize(&attr, stack_size);
    }

    ThreadWrapper* wrapper = new ThreadWrapper{func, arg, name};
    if (!wrapper) {
        fprintf(stderr, "Phase 04: Failed to allocate thread wrapper\n");
        pthread_attr_destroy(&attr);
        return (SDL2_ThreadHandle)-1;
    }

    pthread_t thread;
    int result = pthread_create(&thread, &attr, thread_wrapper_func, wrapper);

    pthread_attr_destroy(&attr);

    if (result != 0) {
        fprintf(stderr, "Phase 04: pthread_create failed (result: %d)\n", result);
        delete wrapper;
        return (SDL2_ThreadHandle)-1;
    }

    printf("Phase 04: Thread created successfully (id: %p)\n", (void*)thread);
    return thread;
}

int SDL2_WaitThread(SDL2_ThreadHandle thread)
{
    if (thread == (pthread_t)-1) {
        return -1;
    }

    printf("Phase 04: SDL2_WaitThread - waiting for thread (id: %p)\n", (void*)thread);

    int result = pthread_join(thread, NULL);

    if (result == 0) {
        printf("Phase 04: Thread joined successfully\n");
        return 0;
    }

    fprintf(stderr, "Phase 04: pthread_join failed (result: %d)\n", result);
    return -1;
}

int SDL2_DetachThread(SDL2_ThreadHandle thread)
{
    if (thread == (pthread_t)-1) {
        return -1;
    }

    printf("Phase 04: SDL2_DetachThread - detaching thread (id: %p)\n", (void*)thread);

    int result = pthread_detach(thread);

    if (result == 0) {
        printf("Phase 04: Thread detached successfully\n");
        return 0;
    }

    fprintf(stderr, "Phase 04: pthread_detach failed (result: %d)\n", result);
    return -1;
}

SDL2_ThreadID SDL2_GetCurrentThreadID(void)
{
    SDL2_ThreadID id = pthread_self();
    printf("Phase 04: Current thread ID: %p\n", (void*)id);
    return id;
}

void SDL2_Sleep(unsigned int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void SDL2_YieldThread(void)
{
    sched_yield();
}

#endif /* _WIN32 */

/* ============================================================================
 * MUTEX (Mutual Exclusion)
 * ============================================================================ */

#ifdef _WIN32

SDL2_Mutex SDL2_CreateMutex(const char* name)
{
    printf("Phase 04: SDL2_CreateMutex(%s)\n", name ? name : "unnamed");

    HANDLE handle = CreateMutexA(NULL, FALSE, name);

    if (!handle) {
        fprintf(stderr, "Phase 04: CreateMutexA failed\n");
        return NULL;
    }

    printf("Phase 04: Mutex created successfully (handle: %p)\n", handle);
    return handle;
}

void SDL2_DestroyMutex(SDL2_Mutex mutex)
{
    if (!mutex) {
        return;
    }

    printf("Phase 04: SDL2_DestroyMutex (handle: %p)\n", mutex);
    CloseHandle(mutex);
}

int SDL2_LockMutex(SDL2_Mutex mutex, int timeout_ms)
{
    if (!mutex) {
        return -1;
    }

    DWORD ms = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    DWORD result = WaitForSingleObject(mutex, ms);

    if (result == WAIT_OBJECT_0) {
        return 0;
    }

    if (result == WAIT_TIMEOUT) {
        return -1;  /* Timeout */
    }

    fprintf(stderr, "Phase 04: WaitForSingleObject failed (result: %lu)\n", result);
    return -1;
}

int SDL2_UnlockMutex(SDL2_Mutex mutex)
{
    if (!mutex) {
        return -1;
    }

    if (ReleaseMutex(mutex)) {
        return 0;
    }

    fprintf(stderr, "Phase 04: ReleaseMutex failed\n");
    return -1;
}

#else /* !_WIN32 - POSIX implementation */

SDL2_Mutex SDL2_CreateMutex(const char* name)
{
    printf("Phase 04: SDL2_CreateMutex(%s)\n", name ? name : "unnamed");

    pthread_mutex_t* mutex = new pthread_mutex_t;
    if (!mutex) {
        fprintf(stderr, "Phase 04: Failed to allocate mutex\n");
        return NULL;
    }

    int result = pthread_mutex_init(mutex, NULL);

    if (result != 0) {
        fprintf(stderr, "Phase 04: pthread_mutex_init failed (result: %d)\n", result);
        delete mutex;
        return NULL;
    }

    printf("Phase 04: Mutex created successfully (ptr: %p)\n", (void*)mutex);
    return mutex;
}

void SDL2_DestroyMutex(SDL2_Mutex mutex)
{
    if (!mutex) {
        return;
    }

    printf("Phase 04: SDL2_DestroyMutex (ptr: %p)\n", (void*)mutex);

    pthread_mutex_destroy(mutex);
    delete (pthread_mutex_t*)mutex;
}

int SDL2_LockMutex(SDL2_Mutex mutex, int timeout_ms)
{
    if (!mutex) {
        return -1;
    }

    if (timeout_ms < 0) {
        /* Infinite wait */
        int result = pthread_mutex_lock(mutex);
        return (result == 0) ? 0 : -1;
    }

    /* Timed wait */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    // Use fallback: try mutex lock with timeout using pthreads condition variable
    #ifdef __APPLE__
    // macOS doesn't support pthread_mutex_timedlock, use alternative
    int result = pthread_mutex_trylock(mutex);
    if (result == 0) {
        return 0;  /* Acquired immediately */
    }
    if (result != EBUSY) {
        fprintf(stderr, "Phase 04: pthread_mutex_trylock failed (result: %d)\n", result);
        return -1;
    }
    
    // Fallback: simulate timeout with small sleep iterations
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    while (now.tv_sec < ts.tv_sec || (now.tv_sec == ts.tv_sec && now.tv_nsec < ts.tv_nsec)) {
        usleep(1000);  /* Sleep 1ms */
        result = pthread_mutex_trylock(mutex);
        if (result == 0) {
            return 0;  /* Acquired */
        }
        if (result != EBUSY) {
            fprintf(stderr, "Phase 04: pthread_mutex_trylock failed (result: %d)\n", result);
            return -1;
        }
        clock_gettime(CLOCK_REALTIME, &now);
    }
    return -1;  /* Timeout */
    #else
    // Linux and other POSIX systems
    int result = pthread_mutex_timedlock(mutex, &ts);
    if (result == 0) {
        return 0;
    }
    if (result == ETIMEDOUT) {
        return -1;  /* Timeout */
    }
    fprintf(stderr, "Phase 04: pthread_mutex_timedlock failed (result: %d)\n", result);
    return -1;
    #endif
}

int SDL2_UnlockMutex(SDL2_Mutex mutex)
{
    if (!mutex) {
        return -1;
    }

    int result = pthread_mutex_unlock(mutex);
    return (result == 0) ? 0 : -1;
}

#endif /* _WIN32 */

/* ============================================================================
 * MUTEX RAII LOCK GUARD
 * ============================================================================ */

SDL2_MutexLock::SDL2_MutexLock(SDL2_Mutex m, int timeout_ms)
    : mutex(m), locked(false)
{
    if (mutex && SDL2_LockMutex(mutex, timeout_ms) == 0) {
        locked = true;
    }
}

SDL2_MutexLock::~SDL2_MutexLock()
{
    if (locked) {
        SDL2_UnlockMutex(mutex);
    }
}

/* ============================================================================
 * CRITICAL SECTION
 * ============================================================================ */

#ifdef _WIN32

SDL2_CriticalSection SDL2_CreateCriticalSection(void)
{
    printf("Phase 04: SDL2_CreateCriticalSection\n");

    SDL2_CriticalSection cs;
    InitializeCriticalSection(&cs);
    return cs;
}

void SDL2_DestroyCriticalSection(SDL2_CriticalSection* cs)
{
    if (!cs) {
        return;
    }

    printf("Phase 04: SDL2_DestroyCriticalSection (ptr: %p)\n", (void*)cs);
    DeleteCriticalSection(cs);
}

void SDL2_EnterCriticalSection(SDL2_CriticalSection* cs)
{
    if (cs) {
        EnterCriticalSection(cs);
    }
}

void SDL2_LeaveCriticalSection(SDL2_CriticalSection* cs)
{
    if (cs) {
        LeaveCriticalSection(cs);
    }
}

#else /* !_WIN32 - POSIX implementation */

SDL2_CriticalSection SDL2_CreateCriticalSection(void)
{
    printf("Phase 04: SDL2_CreateCriticalSection\n");

    pthread_mutex_t cs = PTHREAD_MUTEX_INITIALIZER;
    return cs;
}

void SDL2_DestroyCriticalSection(SDL2_CriticalSection* cs)
{
    if (!cs) {
        return;
    }

    printf("Phase 04: SDL2_DestroyCriticalSection (ptr: %p)\n", (void*)cs);
    pthread_mutex_destroy(cs);
}

void SDL2_EnterCriticalSection(SDL2_CriticalSection* cs)
{
    if (cs) {
        pthread_mutex_lock(cs);
    }
}

void SDL2_LeaveCriticalSection(SDL2_CriticalSection* cs)
{
    if (cs) {
        pthread_mutex_unlock(cs);
    }
}

#endif /* _WIN32 */

/* ============================================================================
 * CRITICAL SECTION RAII LOCK GUARD
 * ============================================================================ */

SDL2_CriticalSectionLock::SDL2_CriticalSectionLock(SDL2_CriticalSection* c)
    : cs(c)
{
    if (cs) {
        SDL2_EnterCriticalSection(cs);
    }
}

SDL2_CriticalSectionLock::~SDL2_CriticalSectionLock()
{
    if (cs) {
        SDL2_LeaveCriticalSection(cs);
    }
}

/* ============================================================================
 * CONDITION VARIABLE
 * ============================================================================ */

#ifdef _WIN32

SDL2_ConditionVariable SDL2_CreateConditionVariable(void)
{
    printf("Phase 04: SDL2_CreateConditionVariable\n");

    SDL2_ConditionVariable cv;
    InitializeConditionVariable(&cv);
    return cv;
}

void SDL2_DestroyConditionVariable(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return;
    }

    printf("Phase 04: SDL2_DestroyConditionVariable (ptr: %p)\n", (void*)cond);
    /* Windows condition variables don't require explicit cleanup */
}

int SDL2_ConditionWait(
    SDL2_ConditionVariable* cond,
    SDL2_CriticalSection* cs,
    int timeout_ms)
{
    if (!cond || !cs) {
        return -1;
    }

    DWORD ms = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    BOOL result = SleepConditionVariableCS(cond, cs, ms);

    if (result) {
        return 0;
    }

    if (GetLastError() == ERROR_TIMEOUT) {
        return -1;  /* Timeout */
    }

    fprintf(stderr, "Phase 04: SleepConditionVariableCS failed\n");
    return -1;
}

int SDL2_ConditionSignal(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return -1;
    }

    WakeConditionVariable(cond);
    return 0;
}

int SDL2_ConditionBroadcast(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return -1;
    }

    WakeAllConditionVariable(cond);
    return 0;
}

#else /* !_WIN32 - POSIX implementation */

SDL2_ConditionVariable SDL2_CreateConditionVariable(void)
{
    printf("Phase 04: SDL2_CreateConditionVariable\n");

    SDL2_ConditionVariable cv = PTHREAD_COND_INITIALIZER;
    return cv;
}

void SDL2_DestroyConditionVariable(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return;
    }

    printf("Phase 04: SDL2_DestroyConditionVariable (ptr: %p)\n", (void*)cond);
    pthread_cond_destroy(cond);
}

int SDL2_ConditionWait(
    SDL2_ConditionVariable* cond,
    SDL2_CriticalSection* cs,
    int timeout_ms)
{
    if (!cond || !cs) {
        return -1;
    }

    if (timeout_ms < 0) {
        /* Infinite wait */
        int result = pthread_cond_wait(cond, cs);
        return (result == 0) ? 0 : -1;
    }

    /* Timed wait */
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(cond, cs, &ts);

    if (result == 0) {
        return 0;
    }

    if (result == ETIMEDOUT) {
        return -1;  /* Timeout */
    }

    fprintf(stderr, "Phase 04: pthread_cond_timedwait failed (result: %d)\n", result);
    return -1;
}

int SDL2_ConditionSignal(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return -1;
    }

    int result = pthread_cond_signal(cond);
    return (result == 0) ? 0 : -1;
}

int SDL2_ConditionBroadcast(SDL2_ConditionVariable* cond)
{
    if (!cond) {
        return -1;
    }

    int result = pthread_cond_broadcast(cond);
    return (result == 0) ? 0 : -1;
}

#endif /* _WIN32 */

/* ============================================================================
 * SEMAPHORE
 * ============================================================================ */

#ifdef _WIN32

SDL2_Semaphore SDL2_CreateSemaphore(int initial_count, int max_count)
{
    printf("Phase 04: SDL2_CreateSemaphore (initial: %d, max: %d)\n", initial_count, max_count);

    HANDLE handle = CreateSemaphoreA(
        NULL,
        initial_count,
        (max_count < 0) ? INT_MAX : max_count,
        NULL
    );

    if (!handle) {
        fprintf(stderr, "Phase 04: CreateSemaphoreA failed\n");
        return NULL;
    }

    printf("Phase 04: Semaphore created successfully (handle: %p)\n", handle);
    return handle;
}

void SDL2_DestroySemaphore(SDL2_Semaphore sem)
{
    if (!sem) {
        return;
    }

    printf("Phase 04: SDL2_DestroySemaphore (handle: %p)\n", sem);
    CloseHandle(sem);
}

int SDL2_WaitSemaphore(SDL2_Semaphore sem, int timeout_ms)
{
    if (!sem) {
        return -1;
    }

    DWORD ms = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    DWORD result = WaitForSingleObject(sem, ms);

    if (result == WAIT_OBJECT_0) {
        return 0;
    }

    if (result == WAIT_TIMEOUT) {
        return -1;  /* Timeout */
    }

    fprintf(stderr, "Phase 04: WaitForSingleObject failed (result: %lu)\n", result);
    return -1;
}

int SDL2_PostSemaphore(SDL2_Semaphore sem, int count)
{
    if (!sem) {
        return -1;
    }

    if (ReleaseSemaphore(sem, count, NULL)) {
        return 0;
    }

    fprintf(stderr, "Phase 04: ReleaseSemaphore failed\n");
    return -1;
}

int SDL2_GetSemaphoreValue(SDL2_Semaphore sem)
{
    if (!sem) {
        return -1;
    }

    LONG prev_count;
    if (ReleaseSemaphore(sem, 0, &prev_count)) {
        return (int)prev_count;
    }

    fprintf(stderr, "Phase 04: ReleaseSemaphore (peek) failed\n");
    return -1;
}

#else /* !_WIN32 - POSIX implementation */

SDL2_Semaphore SDL2_CreateSemaphore(int initial_count, int max_count)
{
    printf("Phase 04: SDL2_CreateSemaphore (initial: %d, max: %d)\n", initial_count, max_count);

    SDL2_Semaphore_Internal* sem = new SDL2_Semaphore_Internal;
    if (!sem) {
        fprintf(stderr, "Phase 04: Failed to allocate semaphore\n");
        return NULL;
    }

    pthread_mutex_init(&sem->mutex, NULL);
    pthread_cond_init(&sem->cond, NULL);
    sem->count = initial_count;

    printf("Phase 04: Semaphore created successfully (ptr: %p)\n", (void*)sem);
    return sem;
}

void SDL2_DestroySemaphore(SDL2_Semaphore sem)
{
    if (!sem) {
        return;
    }

    printf("Phase 04: SDL2_DestroySemaphore (ptr: %p)\n", (void*)sem);

    SDL2_Semaphore_Internal* s = (SDL2_Semaphore_Internal*)sem;
    pthread_cond_destroy(&s->cond);
    pthread_mutex_destroy(&s->mutex);
    delete s;
}

int SDL2_WaitSemaphore(SDL2_Semaphore sem, int timeout_ms)
{
    if (!sem) {
        return -1;
    }

    SDL2_Semaphore_Internal* s = (SDL2_Semaphore_Internal*)sem;

    pthread_mutex_lock(&s->mutex);

    if (timeout_ms < 0) {
        /* Infinite wait */
        while (s->count == 0) {
            pthread_cond_wait(&s->cond, &s->mutex);
        }
    } else {
        /* Timed wait */
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        while (s->count == 0) {
            int result = pthread_cond_timedwait(&s->cond, &s->mutex, &ts);
            if (result == ETIMEDOUT) {
                pthread_mutex_unlock(&s->mutex);
                return -1;  /* Timeout */
            }
        }
    }

    s->count--;
    pthread_mutex_unlock(&s->mutex);
    return 0;
}

int SDL2_PostSemaphore(SDL2_Semaphore sem, int count)
{
    if (!sem) {
        return -1;
    }

    SDL2_Semaphore_Internal* s = (SDL2_Semaphore_Internal*)sem;

    pthread_mutex_lock(&s->mutex);
    s->count += count;
    pthread_cond_broadcast(&s->cond);
    pthread_mutex_unlock(&s->mutex);

    return 0;
}

int SDL2_GetSemaphoreValue(SDL2_Semaphore sem)
{
    if (!sem) {
        return -1;
    }

    SDL2_Semaphore_Internal* s = (SDL2_Semaphore_Internal*)sem;

    pthread_mutex_lock(&s->mutex);
    int value = s->count;
    pthread_mutex_unlock(&s->mutex);

    return value;
}

#endif /* _WIN32 */
