#include "string_compat.h"

#include <cctype>
#include <cwchar>
#include <string>

// GeneralsX @bugfix Copilot 26/08/2026 Provide portable null-terminated integer conversion.
char* itoa(int value, char* str, int base)
{
  if (base < 2 || base > 36)
  {
    str[0] = '\0';
    return str;
  }

  static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  const bool is_negative = value < 0 && base == 10;
  unsigned int magnitude = static_cast<unsigned int>(value);
  if (is_negative)
  {
    magnitude = 0U - magnitude;
  }

  char* output = str;
  do
  {
    *output++ = digits[magnitude % static_cast<unsigned int>(base)];
    magnitude /= static_cast<unsigned int>(base);
  } while (magnitude != 0);

  if (is_negative)
  {
    *output++ = '-';
  }
  *output = '\0';

  for (char* left = str, *right = output - 1; left < right; ++left, --right)
  {
    const char temp = *left;
    *left = *right;
    *right = temp;
  }
  return str;
}

int _vsnwprintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list args)
{
  std::wstring format_fixup(format);

  // Replace all %s with %ls
  size_t pos = format_fixup.find(L"%s", 0);
  while (pos != std::wstring::npos)
  {
    format_fixup.replace(pos, 2, L"%ls");
    pos += 3;
    pos = format_fixup.find(L"%s", pos);
  }

  // Replace all %S with %s
  pos = format_fixup.find(L"%S", 0);
  while (pos != std::wstring::npos)
  {
    format_fixup.replace(pos, 2, L"%s");
    pos += 2;
    pos = format_fixup.find(L"%S", pos);
  }


  return vswprintf(buffer, count, format_fixup.c_str(), args);
}

// Also defined in GameSpy gsplatformutil
__attribute__((weak))
char* _strlwr(char* str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {
    str[i] = tolower(str[i]);
  }
  return str;
}

// GeneralsX @build fbraz 11/02/2026 BenderAI - Linux portability: uppercase string
__attribute__((weak))
char* _strupr(char* str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {
    str[i] = toupper(str[i]);
  }
  return str;
}