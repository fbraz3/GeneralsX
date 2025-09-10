#ifndef PROCESS_H
#define PROCESS_H

#ifdef _WIN32
    #include <process.h>
#else
    // Unix/macOS process compatibility
    #include <unistd.h>
    #include <sys/wait.h>
    #include <spawn.h>
    
    // Windows process functions
    #define _beginthread(func, stack_size, arg_list) 0
    #define _endthread() 
    
    // Process creation
    inline int _spawnl(int mode, const char* path, const char* arg0, ...) {
        return 0; // Stub implementation
    }
    
    inline int _spawnv(int mode, const char* path, const char* const* argv) {
        return 0; // Stub implementation
    }
    
    // Process modes
    #define _P_WAIT 0
    #define _P_NOWAIT 1
    #define _P_OVERLAY 2
    
    // Process IDs
    #define _getpid() getpid()
    
#endif

#endif // PROCESS_H
