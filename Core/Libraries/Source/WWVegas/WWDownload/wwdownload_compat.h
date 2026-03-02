/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// GeneralsX @build BenderAI 03/03/2026 - Windows POSIX compatibility shims for WWDownload module.
// Provides strlcpy/strlcat polyfills, socklen_t, and ARRAY_SIZE for building on MSVC (win64-modern).
// Include this header inside the #ifdef _WIN32 platform block of FTP.cpp and Download.cpp ONLY.
// winsock2.h must already be included before this header (via ftp.h).

#pragma once

#ifdef _WIN32

// ws2tcpip.h: defines socklen_t (typedef int socklen_t) and getaddrinfo/freeaddrinfo.
// Requires winsock2.h first (already included via ftp.h -> winsock2.h).
#include <ws2tcpip.h>

// ARRAY_SIZE: sizeof-based count for fixed-size arrays
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

// strlcpy: POSIX-2008 safe string copy; not available in MSVC CRT.
// Copies at most (size-1) chars from src to dst; always NUL-terminates if size > 0.
// Returns the total length of src (as if size were unlimited).
#ifndef strlcpy
static inline size_t strlcpy(char* dst, const char* src, size_t size)
{
    const char* s = src;
    if (size > 0) {
        size_t n = size - 1;
        while (n && *s) { *dst++ = *s++; --n; }
        *dst = '\0';
    }
    while (*s) ++s;
    return (size_t)(s - src);
}
#endif // strlcpy

// strlcat: POSIX-2008 safe string concatenate; not available in MSVC CRT.
// Appends src to dst up to (size - strlen(dst) - 1) chars; always NUL-terminates.
// Returns the total length of the string that would have been created.
#ifndef strlcat
static inline size_t strlcat(char* dst, const char* src, size_t size)
{
    size_t dstlen = strnlen(dst, size);
    if (dstlen >= size) return dstlen + strlen(src);
    return dstlen + strlcpy(dst + dstlen, src, size - dstlen);
}
#endif // strlcat

#endif // _WIN32
