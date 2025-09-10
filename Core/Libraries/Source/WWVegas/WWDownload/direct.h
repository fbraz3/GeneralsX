#ifndef DIRECT_H
#define DIRECT_H

#ifdef _WIN32
    #include <direct.h>
#else
    // Unix/macOS directory compatibility
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    
    // Directory functions
    #define _mkdir(path) mkdir(path, 0755)
    #define _rmdir(path) rmdir(path)
    #define _getcwd(buffer, size) getcwd(buffer, size)
    #define _chdir(path) chdir(path)
    
    // Maximum path length
    #ifndef _MAX_PATH
    #define _MAX_PATH 260
    #endif
    
    #ifndef _MAX_DIR
    #define _MAX_DIR 256
    #endif
    
    #ifndef _MAX_FNAME
    #define _MAX_FNAME 256
    #endif
    
    #ifndef _MAX_EXT
    #define _MAX_EXT 256
    #endif
    
#endif

#endif // DIRECT_H
