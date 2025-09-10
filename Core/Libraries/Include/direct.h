#pragma once
#ifndef _DIRECT_H_
#define _DIRECT_H_

// Direct.h compatibility for macOS port
#ifdef __APPLE__

#include <unistd.h>
#include <sys/stat.h>

// Directory functions
inline int _mkdir(const char* dirname) {
    return mkdir(dirname, 0755);
}

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
