#ifndef THREADING_COMPAT_H
#define THREADING_COMPAT_H

#pragma once

// Include Windows compatibility layer for basic types
#include "win32_compat.h"

#ifndef _WIN32

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

// Windows threading types
typedef void* HANDLE;
typedef unsigned long DWORD;
typedef void* LPVOID;
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef int BOOL;

// Threading constants
#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 0x00000000L
#define WAIT_TIMEOUT 0x00000102L
#define WAIT_FAILED 0xFFFFFFFF

// Thread structure to hold pthread data
typedef struct {
    pthread_t thread;
    LPTHREAD_START_ROUTINE start_routine;
    LPVOID parameter;
    DWORD thread_id;
    bool valid;
} ThreadHandle;

// Mutex structure
typedef struct {
    pthread_mutex_t mutex;
    bool valid;
} MutexHandle;

// Thread function wrapper
void* thread_wrapper(void* param);

// Threading functions
inline HANDLE CreateThread(
    void* lpThreadAttributes,
    size_t dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    DWORD* lpThreadId
) {
    ThreadHandle* handle = new ThreadHandle();
    handle->start_routine = lpStartAddress;
    handle->parameter = lpParameter;
    handle->valid = false;
    
    // Generate thread ID
    static DWORD next_thread_id = 1;
    handle->thread_id = next_thread_id++;
    if (lpThreadId) *lpThreadId = handle->thread_id;
    
    // Create pthread
    int result = pthread_create(&handle->thread, NULL, 
                               (void*(*)(void*))thread_wrapper, handle);
    
    if (result == 0) {
        handle->valid = true;
        return (HANDLE)handle;
    } else {
        delete handle;
        return NULL;
    }
}

inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    
    ThreadHandle* thread_handle = (ThreadHandle*)hHandle;
    if (!thread_handle->valid) return WAIT_FAILED;
    
    if (dwMilliseconds == INFINITE) {
        int result = pthread_join(thread_handle->thread, NULL);
        return (result == 0) ? WAIT_OBJECT_0 : WAIT_FAILED;
    } else {
        // For timeout, we'll use a simplified approach
        // In a real implementation, you'd use pthread_timedjoin_np or similar
        return WAIT_TIMEOUT;
    }
}

inline BOOL CloseHandle(HANDLE hObject) {
    if (!hObject) return FALSE;
    
    // Try to cast as ThreadHandle first
    ThreadHandle* thread_handle = (ThreadHandle*)hObject;
    if (thread_handle->valid) {
        // Don't join here, just mark as invalid
        thread_handle->valid = false;
        delete thread_handle;
        return TRUE;
    }
    
    // Try as MutexHandle
    MutexHandle* mutex_handle = (MutexHandle*)hObject;
    if (mutex_handle->valid) {
        pthread_mutex_destroy(&mutex_handle->mutex);
        mutex_handle->valid = false;
        delete mutex_handle;
        return TRUE;
    }
    
    return FALSE;
}

inline HANDLE CreateMutex(void* lpMutexAttributes, BOOL bInitialOwner, const char* lpName) {
    MutexHandle* handle = new MutexHandle();
    
    int result = pthread_mutex_init(&handle->mutex, NULL);
    if (result == 0) {
        handle->valid = true;
        
        if (bInitialOwner) {
            pthread_mutex_lock(&handle->mutex);
        }
        
        return (HANDLE)handle;
    } else {
        delete handle;
        return NULL;
    }
}

inline BOOL ReleaseMutex(HANDLE hMutex) {
    if (!hMutex) return FALSE;
    
    MutexHandle* mutex_handle = (MutexHandle*)hMutex;
    if (!mutex_handle->valid) return FALSE;
    
    int result = pthread_mutex_unlock(&mutex_handle->mutex);
    return (result == 0) ? TRUE : FALSE;
}

#endif // !_WIN32

#endif // THREADING_COMPAT_H