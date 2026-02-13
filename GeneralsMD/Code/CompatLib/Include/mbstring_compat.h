// TheSuperHackers @build BenderAI 12/02/2026 mbstring.h compatibility for Linux
#ifndef MBSTRING_COMPAT_H
#define MBSTRING_COMPAT_H

// Windows mbstring.h functions for multi-byte string handling
// On Linux we provide minimal stubs for what the game needs

#ifndef _WIN32

#include <cstring>
#include <cstddef>

// Count the number of multi-byte characters up to n bytes
// For UTF-8, we count how many characters (not bytes) are in the string
inline size_t _mbsnccnt(const unsigned char* str, size_t n) {
    if (!str || n == 0) {
        return 0;
    }
    
    size_t char_count = 0;
    size_t byte_index = 0;
    
    while (byte_index < n && str[byte_index] != '\0') {
        // UTF-8 character detection:
        // - 0xxxxxxx: single byte (ASCII)
        // - 110xxxxx: start of 2-byte character
        // - 1110xxxx: start of 3-byte character
        // - 11110xxx: start of 4-byte character
        unsigned char byte = str[byte_index];
        
        if ((byte & 0x80) == 0) {
            // Single byte character (ASCII)
            byte_index += 1;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte character
            byte_index += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte character
            byte_index += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte character
            byte_index += 4;
        } else {
            // Invalid UTF-8 sequence, skip this byte
            byte_index += 1;
        }
        
        char_count++;
    }
    
    return char_count;
}

#else
// On Windows, just include the real mbstring.h
#include <mbstring.h>
#endif // !_WIN32

#endif // MBSTRING_COMPAT_H
