#pragma once
#ifndef _DIRECT_H_
#define _DIRECT_H_

// Direct.h compatibility for macOS port
#ifdef __APPLE__

#include <unistd.h>
#include <sys/stat.h>

// Directory functions - using macros for compatibility
#ifdef __APPLE__
#include <unistd.h>
#include <sys/stat.h>
#ifndef _mkdir
#define _mkdir(path) mkdir(path, 0755)
#endif
#ifndef _rmdir
#define _rmdir(path) rmdir(path)
#endif
#ifndef _getcwd
#define _getcwd(buffer, size) getcwd(buffer, size)
#endif
#ifndef _chdir
#define _chdir(path) chdir(path)
#endif
#endif

inline int _rmdir(const char* dirname) {
    return rmdir(dirname);
}

inline char* _getcwd(char* buffer, int maxlen) {
    return getcwd(buffer, maxlen);
}

inline int _chdir(const char* dirname) {
    return chdir(dirname);
}

#endif // __APPLE__
#endif // _DIRECT_H_
