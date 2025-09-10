#pragma once

// Input/Output compatibility header for non-Windows systems
// This header provides Windows-specific I/O functions

#ifndef _WIN32

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// Windows-style file access constants
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_CREAT O_CREAT
#define _O_TRUNC O_TRUNC
#define _O_APPEND O_APPEND
#define _O_BINARY 0  // No binary mode distinction on Unix

// File access permission constants
#define _S_IREAD S_IRUSR
#define _S_IWRITE S_IWUSR

// Function mappings
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
#define _access access

// Access mode constants
#define _A_NORMAL 0
#define _A_RDONLY 1
#define _A_HIDDEN 2
#define _A_SYSTEM 4
#define _A_SUBDIR 16
#define _A_ARCH 32

// Seek constants
#define _SEEK_SET SEEK_SET
#define _SEEK_CUR SEEK_CUR
#define _SEEK_END SEEK_END

// File finding structures (stub implementation)
struct _finddata_t {
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    size_t size;
    char name[260];
};

// File finding functions (stub implementations)
inline long _findfirst(const char* filespec, struct _finddata_t* fileinfo) {
    return -1; // Not found
}

inline int _findnext(long handle, struct _finddata_t* fileinfo) {
    return -1; // No more files
}

inline int _findclose(long handle) {
    return 0; // Success
}

#else
// On Windows, include the real io.h
#include <io.h>
#endif
