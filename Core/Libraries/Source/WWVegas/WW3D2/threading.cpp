#include "threading.h"

#ifndef _WIN32
#include <stdint.h>

// Thread wrapper function implementation
void* thread_wrapper(void* param) {
    ThreadHandle* handle = (ThreadHandle*)param;
    if (!handle || !handle->start_routine) {
        return (void*)1;
    }
    
    // Call the original thread function
    DWORD result = handle->start_routine(handle->parameter);
    
    return (void*)(uintptr_t)result;
}

#endif // !_WIN32