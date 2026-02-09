#include "types_compat.h"
#include "module_compat.h"

#include <unistd.h>

#include <dlfcn.h>

#include <string.h>

#include <filesystem>

bool GetModuleFileName(HINSTANCE hInstance, char* buffer, int size)
{
  ssize_t count = readlink("/proc/self/exe", buffer, size);
  if (count == -1)
    return false;
  buffer[count] = '\0';
  return true;
}

HMODULE LoadLibrary(const char* lpFileName)
{
  std::filesystem::path pathFile(lpFileName);
  if (pathFile.extension() == ".dll")
  {
    // Remove extension
    std::string pathCurrent = pathFile.replace_extension().string();

    return dlopen(pathCurrent.c_str(), RTLD_LAZY);
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