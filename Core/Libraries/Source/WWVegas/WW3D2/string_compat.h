#pragma once

#ifdef __APPLE__

#include <cstring>
#include <cctype>
#include <algorithm>

// Windows string API compatibility for macOS

// Note: strupr is provided by win32_compat.h

// Note: _strlwr is provided by Dependencies/Utility/Utility/string_compat.h

// String case conversion functions
// Note: strupr is provided by win32_compat.h
#ifdef __APPLE__
// All string functions are provided by win32_compat.h
#endif

// String comparison functions
inline int stricmp(const char* str1, const char* str2) {
    return strcasecmp(str1, str2);
}

inline int strnicmp(const char* str1, const char* str2, size_t n) {
    return strncasecmp(str1, str2, n);
}

// Windows bitmap constants
#ifndef BI_RGB
#define BI_RGB 0
#endif

#ifndef BI_RLE8
#define BI_RLE8 1
#endif

#ifndef BI_RLE4
#define BI_RLE4 2
#endif

#ifndef BI_BITFIELDS
#define BI_BITFIELDS 3
#endif

#endif // __APPLE__