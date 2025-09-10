#pragma once
#ifndef _PROCESS_H_
#define _PROCESS_H_

// Process management compatibility for macOS port
#ifdef __APPLE__

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>

// Process execution constants
#define _P_WAIT 0
#define _P_NOWAIT 1
#define _P_OVERLAY 2
#define _P_NOWAITO 3
#define _P_DETACH 4

// Process execution functions (stubbed for compatibility)
inline int _spawnl(int mode, const char* cmdname, const char* arg0, ...) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnle(int mode, const char* cmdname, const char* arg0, ...) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnlp(int mode, const char* cmdname, const char* arg0, ...) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnlpe(int mode, const char* cmdname, const char* arg0, ...) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnv(int mode, const char* cmdname, const char* const* argv) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnve(int mode, const char* cmdname, const char* const* argv, const char* const* envp) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnvp(int mode, const char* cmdname, const char* const* argv) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

inline int _spawnvpe(int mode, const char* cmdname, const char* const* argv, const char* const* envp) {
    // Stub implementation - not actually used in this codebase
    return -1;
}

// Get process ID
inline int _getpid() {
    return getpid();
}

// Get parent process ID  
inline int _getppid() {
    return getppid();
}

// Thread functions (basic compatibility)
typedef unsigned long uintptr_t;

inline uintptr_t _beginthread(void (*start_address)(void*), unsigned stack_size, void* arglist) {
    // Stub implementation - should use pthreads in real implementation
    return 0;
}

inline void _endthread() {
    // Stub implementation - should use pthread_exit in real implementation
}

inline uintptr_t _beginthreadex(void* security, unsigned stack_size, 
                               unsigned (*start_address)(void*), void* arglist,
                               unsigned initflag, unsigned* thrdaddr) {
    // Stub implementation - should use pthreads in real implementation
    return 0;
}

inline void _endthreadex(unsigned retval) {
    // Stub implementation - should use pthread_exit in real implementation
}

#endif // __APPLE__
#endif // _PROCESS_H_
