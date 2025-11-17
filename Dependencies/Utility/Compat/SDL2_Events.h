// SDL2-Based Event/Synchronization Primitives
// Replaces Win32 HANDLE-based CreateEvent/SetEvent/WaitForSingleObject
// Part of Phase 39.2: Cross-Platform Event System

#pragma once

#include <SDL2/SDL.h>
#include <cstring>

// NOTE: HANDLE, DWORD, BOOL are expected to be defined by the including file (msvc_types_compat.h)
// Do NOT include msvc_types_compat.h here to avoid circular includes

// Return codes matching Win32
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define WAIT_FAILED ((unsigned long)-1)
#define INFINITE 0xFFFFFFFF

// SDL2-based Event structure (internal representation)
struct SDL2Event {
    SDL_mutex* mutex;
    SDL_cond* condition;  // Condition variable for proper signaling
    int signaled;
    int manual_reset;
    
    SDL2Event(int manual_reset_flag = 0, int initial_state = 0) 
        : manual_reset(manual_reset_flag), signaled(initial_state)
    {
        mutex = SDL_CreateMutex();
        condition = SDL_CreateCond();
    }
    
    ~SDL2Event() {
        if (condition) SDL_DestroyCond(condition);
        if (mutex) SDL_DestroyMutex(mutex);
    }
};

// Inline implementations for Windows compatibility layer
namespace SDL2Compat {

// CreateEvent: Create an event object
// Returns HANDLE (opaque pointer to SDL2Event)
inline HANDLE CreateEvent(void* lpEventAttributes, int bManualReset, 
                         int bInitialState, const char* lpName)
{
    // lpEventAttributes, lpName are ignored on cross-platform
    return (HANDLE)(new SDL2Event(bManualReset, bInitialState));
}

// SetEvent: Set event to signaled state
inline BOOL SetEvent(HANDLE hEvent)
{
    if (!hEvent) return 0;  // FALSE
    
    SDL2Event* evt = (SDL2Event*)hEvent;
    
    SDL_LockMutex(evt->mutex);
    evt->signaled = 1;
    
    if (!evt->manual_reset) {
        // Auto-reset: signal one waiter and reset
        SDL_CondSignal(evt->condition);
    } else {
        // Manual reset: signal all waiters, will stay signaled
        SDL_CondBroadcast(evt->condition);
    }
    
    SDL_UnlockMutex(evt->mutex);
    return 1;  // TRUE
}

// ResetEvent: Set event to non-signaled state
inline BOOL ResetEvent(HANDLE hEvent)
{
    if (!hEvent) return 0;  // FALSE
    
    SDL2Event* evt = (SDL2Event*)hEvent;
    
    SDL_LockMutex(evt->mutex);
    evt->signaled = 0;
    SDL_UnlockMutex(evt->mutex);
    
    return 1;  // TRUE
}

// WaitForSingleObject: Wait for event with timeout
// dwMilliseconds: timeout in milliseconds, or 0xFFFFFFFF for infinite
inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
    if (!hHandle) return WAIT_FAILED;
    
    SDL2Event* evt = (SDL2Event*)hHandle;
    int timeout_ms = (dwMilliseconds == 0xFFFFFFFF) ? -1 : (int)dwMilliseconds;
    
    SDL_LockMutex(evt->mutex);
    
    // If already signaled, return immediately
    if (evt->signaled) {
        // Auto-reset: consume the signal
        if (!evt->manual_reset) {
            evt->signaled = 0;
        }
        SDL_UnlockMutex(evt->mutex);
        return WAIT_OBJECT_0;
    }
    
    // Wait with condition variable
    int ret = SDL_CondWaitTimeout(evt->condition, evt->mutex, timeout_ms);
    
    // Check signal status after wakeup
    if (ret == 0) {
        // Condition signaled - check if still signaled
        if (evt->signaled) {
            // Auto-reset: consume the signal
            if (!evt->manual_reset) {
                evt->signaled = 0;
            }
            SDL_UnlockMutex(evt->mutex);
            return WAIT_OBJECT_0;
        }
    } else if (ret == SDL_MUTEX_TIMEDOUT) {
        SDL_UnlockMutex(evt->mutex);
        return WAIT_TIMEOUT;
    }
    
    SDL_UnlockMutex(evt->mutex);
    return WAIT_FAILED;
}

// CloseEvent: Destroy event object
inline BOOL CloseEvent(HANDLE hEvent)
{
    if (!hEvent) return 0;  // FALSE
    delete (SDL2Event*)hEvent;
    return 1;  // TRUE
}

// ReleaseMutex: Release a mutex (for compatibility with Win32 mutex operations)
// Note: This is primarily for event-based synchronization cleanup
// For actual mutexes, CreateMutex/ReleaseMutex should be used separately
inline BOOL ReleaseMutex(HANDLE hMutex)
{
    if (!hMutex) return 0;  // FALSE
    SDL2Event* evt = (SDL2Event*)hMutex;
    SDL_UnlockMutex(evt->mutex);
    return 1;  // TRUE
}

} // namespace SDL2Compat

// Expose at global scope for compatibility
using SDL2Compat::CreateEvent;
using SDL2Compat::SetEvent;
using SDL2Compat::ResetEvent;
using SDL2Compat::WaitForSingleObject;
using SDL2Compat::CloseEvent;
using SDL2Compat::ReleaseMutex;
