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
#include <filesystem>
#include <dirent.h>

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

// TheSuperHackers @build Bender 11/02/2026 Win32 file system APIs → std::filesystem (C++17)
// SetCurrentDirectory - change working directory
inline int SetCurrentDirectory(const char* path) {
  try {
    std::filesystem::current_path(path);
    return 1; // TRUE
  } catch (...) {
    return 0; // FALSE
  }
}

// Note: CreateDirectory() is in socket_compat.h (avoid duplicate)

// DeleteFile - delete file
inline int DeleteFile(const char* path) {
  try {
    return std::filesystem::remove(path) ? 1 : 0;
  } catch (...) {
    return 0; // FALSE
  }
}

// TheSuperHackers @build BenderAI 12/02/2026 CopyFile stub for Linux
// Replay save system needs this to duplicate .rep files
inline int CopyFile(const char* existingFile, const char* newFile, int failIfExists) {
  try {
    std::filesystem::copy_options opts = failIfExists 
      ? std::filesystem::copy_options::none 
      : std::filesystem::copy_options::overwrite_existing;
    std::filesystem::copy_file(existingFile, newFile, opts);
    return 1; // TRUE
  } catch (...) {
    return 0; // FALSE
  }
}

// TheSuperHackers @build BenderAI 12/02/2026 FormatMessageW stub for Linux
// Used to format error messages from GetLastError() - always returns generic error
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
inline int FormatMessageW(unsigned long flags, const void* source, unsigned long messageId, 
                          unsigned long languageId, wchar_t* buffer, unsigned long size, void* args) {
  if (buffer && size > 0) {
    // Generic error message in wide string
    const wchar_t* msg = L"File operation failed";
    size_t len = 0;
    while (msg[len] && len < size - 1) {
      buffer[len] = msg[len];
      len++;
    }
    buffer[len] = 0;
    return (int)len;
  }
  return 0;
}

// TheSuperHackers @build BenderAI 12/02/2026 FormatMessage stub (ASCII version)
// Used by ReplayMenu.cpp for error message formatting
inline int FormatMessage(unsigned long flags, const void* source, unsigned long messageId, 
                         unsigned long languageId, char* buffer, unsigned long size, void* args) {
  if (buffer && size > 0) {
    const char* msg = "File operation failed";
    size_t len = 0;
    while (msg[len] && len < size - 1) {
      buffer[len] = msg[len];
      len++;
    }
    buffer[len] = 0;
    return (int)len;
  }
  return 0;
}

// TheSuperHackers @build BenderAI 12/02/2026 Windows Shell API stubs
// Used by ReplayMenu.cpp to get desktop folder path - stubbed for Linux
typedef void* LPITEMIDLIST;  // Pointer to item ID list (shell folder identifier)
#define CSIDL_DESKTOPDIRECTORY 0x0010  // Desktop folder constant

// Get special folder location (always fails on Linux)
inline int SHGetSpecialFolderLocation(void* hwndOwner, int nFolder, LPITEMIDLIST* ppidl) {
  if (ppidl) *ppidl = nullptr;
  return -1;  // E_FAIL
}

// Get path from item ID list (always returns false)
inline int SHGetPathFromIDList(LPITEMIDLIST pidl, char* pszPath) {
  return 0;  // FALSE
}

// WIN32_FIND_DATA structure for file iteration
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define FILE_ATTRIBUTE_DIRECTORY 0x10

struct WIN32_FIND_DATA {
  uint32_t dwFileAttributes;
  char cFileName[MAX_PATH];
};

// FindFirstFile / FindNextFile  / FindClose - directory iteration
// Simple wrapper using readdir (POSIX)
struct FIND_HANDLE_DATA {
  DIR* dir;
  struct dirent* entry;
  char pattern[MAX_PATH];
};

inline void* FindFirstFile(const char* pattern, WIN32_FIND_DATA* findData) {
  FIND_HANDLE_DATA* handle = new FIND_HANDLE_DATA();
  handle->dir = opendir(".");
  if (!handle->dir) {
    delete handle;
    return (void*)-1; // INVALID_HANDLE_VALUE
  }
  
  // Store pattern for filtering (just "*" for now)
  strncpy(handle->pattern, pattern, MAX_PATH - 1);
  handle->pattern[MAX_PATH - 1] = '\0';
  
  // Read first entry
  handle->entry = readdir(handle->dir);
  if (!handle->entry) {
    closedir(handle->dir);
    delete handle;
    return (void*)-1;
  }
  
  // Fill find data
  strncpy(findData->cFileName, handle->entry->d_name, MAX_PATH - 1);
  findData->cFileName[MAX_PATH - 1] = '\0';
  findData->dwFileAttributes = (handle->entry->d_type == DT_DIR) ? FILE_ATTRIBUTE_DIRECTORY : 0;
  
  return handle;
}

inline int FindNextFile(void* hFindFile, WIN32_FIND_DATA* findData) {
  if (!hFindFile || hFindFile == (void*)-1) {
    return 0; // FALSE
  }
  
  FIND_HANDLE_DATA* handle = static_cast<FIND_HANDLE_DATA*>(hFindFile);
  handle->entry = readdir(handle->dir);
  if (!handle->entry) {
    return 0; // FALSE - no more files
  }
  
  // Fill find data
  strncpy(findData->cFileName, handle->entry->d_name, MAX_PATH - 1);
  findData->cFileName[MAX_PATH - 1] = '\0';
  findData->dwFileAttributes = (handle->entry->d_type == DT_DIR) ? FILE_ATTRIBUTE_DIRECTORY : 0;
  
  return 1; // TRUE
}

inline int FindClose(void* hFindFile) {
  if (!hFindFile || hFindFile == (void*)-1) {
    return 1; // TRUE
  }
  
  FIND_HANDLE_DATA* handle = static_cast<FIND_HANDLE_DATA*>(hFindFile);
  if (handle->dir) {
    closedir(handle->dir);
  }
  delete handle;
  return 1; // TRUE
}

#define INVALID_HANDLE_VALUE ((void*)-1)

#endif // !_WIN32
