#include "types_compat.h"
#include "module_compat.h"

#include <unistd.h>

#include <dlfcn.h>

#include <string.h>

#include <filesystem>

// GeneralsX @feature BenderAI 24/02/2026 Phase 5 macOS executable path detection
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

bool GetModuleFileName(HINSTANCE hInstance, char* buffer, int size)
{
#ifdef __linux__
  ssize_t count = readlink("/proc/self/exe", buffer, size);
  if (count == -1)
    return false;
  buffer[count] = '\0';
  return true;
#elif defined(__APPLE__)
  uint32_t bufsize = (uint32_t)size;
  int ret = _NSGetExecutablePath(buffer, &bufsize);
  if (ret == 0) {
    return true;
  }
  return false;
#else
  return false;
#endif
}

HMODULE LoadLibrary(const char* lpFileName)
{
  // GeneralsX @feature BenderAI 24/02/2026 Phase 5 - Platform-agnostic library loading without C++17
  // Check if filename ends with ".dll" and remove extension if so
  const char* ext = strrchr(lpFileName, '.');
  if (ext && strcmp(ext, ".dll") == 0) {
    // Create a copy without the .dll extension
    size_t len = ext - lpFileName;
    char pathCurrent[512];
    if (len < sizeof(pathCurrent)) {
      strncpy(pathCurrent, lpFileName, len);
      pathCurrent[len] = '\0';
      return dlopen(pathCurrent, RTLD_LAZY);
    }
  }
  
  return dlopen(lpFileName, RTLD_LAZY);
}

FARPROC GetProcAddress(HMODULE hModule, const char* lpProcName)
{
  return (FARPROC)dlsym(hModule, lpProcName);
}

void FreeLibrary(HMODULE hModule)
{
  // Currently misbehaves by unmapping regions not related to the library
  // dlclose(hModule);
}