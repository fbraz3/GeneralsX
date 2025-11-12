/*
** Phase 03: Windows I/O API Compatibility Stub
** io.h - Windows low-level I/O functions
**
** Pattern: source_dest_type_compat
** Source: Windows io.h (_access, _open, _close, _read, _write, etc.)
** Destination: POSIX/Linux/macOS platforms
** Type: Windows I/O API compatibility stubs
**
** io.h provides low-level I/O operations on Windows.
** On POSIX platforms, we map these to standard POSIX functions.
*/

#pragma once

#ifndef IO_COMPAT_H_INCLUDED
#define IO_COMPAT_H_INCLUDED

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Map Windows file access modes to POSIX */
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_RDWR O_RDWR
#define _O_APPEND O_APPEND
#define _O_CREAT O_CREAT
#define _O_EXCL O_EXCL
#define _O_TRUNC O_TRUNC
#define _O_BINARY 0  /* Binary mode is default on POSIX */

/* Map Windows access check modes to POSIX */
#define _F_OK F_OK  /* File exists */
#define _R_OK R_OK  /* Read permission */
#define _W_OK W_OK  /* Write permission */
#define _X_OK X_OK  /* Execute permission */

/* 
 * _access - Check if file has given access mode
 * Returns 0 if file has the specified mode, -1 otherwise
 * This maps directly to POSIX access() function
 */
static inline int _access(const char *pathname, int mode)
{
    return access(pathname, mode);
}

/* Map other common Windows I/O functions to POSIX equivalents */
#define _open open
#define _close close
#define _read read
#define _write write

#endif /* IO_COMPAT_H_INCLUDED */
