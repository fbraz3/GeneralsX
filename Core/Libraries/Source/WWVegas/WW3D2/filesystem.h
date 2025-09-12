#ifndef FILESYSTEM_COMPAT_H
#define FILESYSTEM_COMPAT_H

#ifndef _WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

// Windows file system types
typedef void* HANDLE;
typedef unsigned long DWORD;
typedef int BOOL;
typedef void* LPVOID;
typedef const char* LPCSTR;
typedef void* LPSECURITY_ATTRIBUTES;

// File system constants
#define INVALID_HANDLE_VALUE ((HANDLE)(long)-1)
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5
#define FILE_ATTRIBUTE_NORMAL 0x00000080L
#define TRUE 1
#define FALSE 0

// File system functions
inline BOOL CreateDirectory(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    if (!lpPathName) return FALSE;
    
    int result = mkdir(lpPathName, 0755);
    if (result == 0) {
        return TRUE;
    } else if (errno == EEXIST) {
        // Directory already exists, consider it success
        return TRUE;
    } else {
        return FALSE;
    }
}

inline BOOL DeleteFile(LPCSTR lpFileName) {
    if (!lpFileName) return FALSE;
    
    int result = unlink(lpFileName);
    return (result == 0) ? TRUE : FALSE;
}

inline HANDLE CreateFile(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    if (!lpFileName) return INVALID_HANDLE_VALUE;
    
    int flags = 0;
    
    // Set access flags
    if (dwDesiredAccess & GENERIC_READ && dwDesiredAccess & GENERIC_WRITE) {
        flags = O_RDWR;
    } else if (dwDesiredAccess & GENERIC_WRITE) {
        flags = O_WRONLY;
    } else {
        flags = O_RDONLY;
    }
    
    // Set creation flags
    switch (dwCreationDisposition) {
        case CREATE_NEW:
            flags |= O_CREAT | O_EXCL;
            break;
        case CREATE_ALWAYS:
            flags |= O_CREAT | O_TRUNC;
            break;
        case OPEN_EXISTING:
            // No additional flags needed
            break;
        case OPEN_ALWAYS:
            flags |= O_CREAT;
            break;
        case TRUNCATE_EXISTING:
            flags |= O_TRUNC;
            break;
    }
    
    int fd = open(lpFileName, flags, 0644);
    if (fd == -1) {
        return INVALID_HANDLE_VALUE;
    }
    
    // Return file descriptor as handle (need to cast carefully)
    return (HANDLE)(uintptr_t)fd;
}

inline BOOL WriteFile(
    HANDLE hFile,
    const void* lpBuffer,
    DWORD nNumberOfBytesToWrite,
    DWORD* lpNumberOfBytesWritten,
    void* lpOverlapped
) {
    if (hFile == INVALID_HANDLE_VALUE || !lpBuffer) return FALSE;
    
    int fd = (int)(uintptr_t)hFile;
    ssize_t bytes_written = write(fd, lpBuffer, nNumberOfBytesToWrite);
    
    if (bytes_written >= 0) {
        if (lpNumberOfBytesWritten) {
            *lpNumberOfBytesWritten = (DWORD)bytes_written;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

inline BOOL ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    DWORD* lpNumberOfBytesRead,
    void* lpOverlapped
) {
    if (hFile == INVALID_HANDLE_VALUE || !lpBuffer) return FALSE;
    
    int fd = (int)(uintptr_t)hFile;
    ssize_t bytes_read = read(fd, lpBuffer, nNumberOfBytesToRead);
    
    if (bytes_read >= 0) {
        if (lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = (DWORD)bytes_read;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

// Permission function
inline int _chmod(const char* filename, int pmode) {
    if (!filename) return -1;
    
    mode_t mode = 0;
    
    // Convert Windows permission mode to Unix mode
    if (pmode & 0x0080) mode |= S_IRUSR; // Read permission for owner
    if (pmode & 0x0040) mode |= S_IWUSR; // Write permission for owner
    if (pmode & 0x0020) mode |= S_IXUSR; // Execute permission for owner
    
    // Add group and other permissions
    mode |= S_IRGRP | S_IROTH; // Read for group and others
    if (pmode & 0x0040) {
        mode |= S_IWGRP; // Write for group if owner has write
    }
    
    return chmod(filename, mode);
}

#endif // !_WIN32

#endif // FILESYSTEM_COMPAT_H