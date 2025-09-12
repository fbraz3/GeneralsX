#pragma once

#ifdef __APPLE__

#include <cstring>
#include <cctype>
#include <algorithm>

// Windows string API compatibility for macOS

// Case conversion functions
inline char* strupr(char* str) {
    if (!str) return nullptr;
    char* p = str;
    while (*p) {
        *p = (char)std::toupper(*p);
        ++p;
    }
    return str;
}

inline char* strlwr(char* str) {
    if (!str) return nullptr;
    char* p = str;
    while (*p) {
        *p = (char)std::tolower(*p);
        ++p;
    }
    return str;
}

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