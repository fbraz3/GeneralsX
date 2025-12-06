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
        printf("Phase 04: SDL2_CreateThread - NULL function pointer\n");
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
        printf("Phase 04: Failed to allocate thread wrapper\n");
        pthread_attr_destroy(&attr);
        return (SDL2_ThreadHandle)-1;
    }

    pthread_t thread;
    int result = pthread_create(&thread, &attr, thread_wrapper_func, wrapper);

    pthread_attr_destroy(&attr);

    if (result != 0) {
        printf("Phase 04: pthread_create failed (result: %d)\n", result);
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

    printf("Phase 04: pthread_join failed (result: %d)\n", result);
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

    printf("Phase 04: pthread_detach failed (result: %d)\n", result);
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


/* ============================================================================
 * MUTEX (Mutual Exclusion)
 * ============================================================================ */


SDL2_Mutex SDL2_CreateMutex(const char* name)
{
    printf("Phase 04: SDL2_CreateMutex(%s)\n", name ? name : "unnamed");

    pthread_mutex_t* mutex = new pthread_mutex_t;
    if (!mutex) {
        printf("Phase 04: Failed to allocate mutex\n");
        return NULL;
    }

    int result = pthread_mutex_init(mutex, NULL);

    if (result != 0) {
        printf("Phase 04: pthread_mutex_init failed (result: %d)\n", result);
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
        printf("Phase 04: pthread_mutex_trylock failed (result: %d)\n", result);
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
            printf("Phase 04: pthread_mutex_trylock failed (result: %d)\n", result);
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
    printf("Phase 04: pthread_mutex_timedlock failed (result: %d)\n", result);
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

    printf("Phase 04: pthread_cond_timedwait failed (result: %d)\n", result);
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


/* ============================================================================
 * SEMAPHORE
 * ============================================================================ */


SDL2_Semaphore SDL2_CreateSemaphore(int initial_count, int max_count)
{
    printf("Phase 04: SDL2_CreateSemaphore (initial: %d, max: %d)\n", initial_count, max_count);

    SDL2_Semaphore_Internal* sem = new SDL2_Semaphore_Internal;
    if (!sem) {
        printf("Phase 04: Failed to allocate semaphore\n");
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

