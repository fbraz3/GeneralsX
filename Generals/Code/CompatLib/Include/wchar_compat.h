#pragma once

#include <stddef.h>
#include <stdarg.h>
#include <wchar.h>
#include <wctype.h>

int _wtoi(const wchar_t* str);
int _vsnwprintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list args);
#define _vsnprintf vsnprintf
#define _wcsicmp wcscasecmp
#define wcsicmp wcscasecmp

inline bool iswascii(wchar_t c)
{
	return (c >= 0 && c <= 127);
}