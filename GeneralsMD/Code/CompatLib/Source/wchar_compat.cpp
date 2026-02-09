#include "wchar_compat.h"

#include <sstream>

int _wtoi(const wchar_t* str)
{
  std::wstringstream ss(str);
  int i;
  ss >> i;
  return i;
}

int _vsnwprintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list args)
{
  return vswprintf(buffer, count, format, args);
}