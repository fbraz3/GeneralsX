#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#define _access access
#define _stat stat

// TheSuperHackers @build fighter19 10/02/2026 Bender - Win32 file API → POSIX
#ifndef _WIN32

#include <unistd.h>
#include <climits>
#include <cstring>

// TheSuperHackers @build Bender 11/02/2026 Windows _mkdir → POSIX mkdir (with default perms)
inline int _mkdir(const char* path)  {
    return mkdir(path, 0755); // rwxr-xr-x
}

// Windows file attributes → POSIX stat
inline uint32_t GetFileAttributes(const char* path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    return 0xFFFFFFFF; // INVALID_FILE_ATTRIBUTES
  }
  return 0; // FILE_ATTRIBUTE_NORMAL
}

// Windows current directory → POSIX getcwd
inline uint32_t GetCurrentDirectory(uint32_t buflen, char* buf) {
  if (getcwd(buf, buflen) != nullptr) {
    return static_cast<uint32_t>(strlen(buf));
  }
  return 0;
}

#endif // !_WIN32
