#pragma once
#ifndef _TCHAR_H_
#define _TCHAR_H_

// TCHAR (Generic-Text Character) compatibility for macOS port
#ifdef __APPLE__

#include <string.h>
#include <wchar.h>
#include <stdlib.h>

// TCHAR defines generic text mappings for character and string types
// On Windows, this switches between ANSI and Unicode based on compilation settings
// For macOS port, we'll default to char-based (ANSI) implementation

#ifndef _UNICODE

// ANSI (char-based) definitions
typedef char TCHAR;
typedef char* PTCHAR;
typedef char* LPTSTR;
typedef const char* LPCTSTR;

// String functions
#define _T(x) x
#define _TEXT(x) x

#define _tcscpy strcpy
#define _tcsncpy strncpy
#define _tcscat strcat
#define _tcsncat strncat
#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsncmp strncmp
#define _tcsicmp strcasecmp
#define _tcsnicmp strncasecmp
#define _tcschr strchr
#define _tcsrchr strrchr
#define _tcsstr strstr
#define _tcsspn strspn
#define _tcscspn strcspn
#define _tcstok strtok
#define _tcsnlen strnlen
#define _tcsdup strdup

// Character functions
#define _totlower tolower
#define _totupper toupper
#define _istdigit isdigit
#define _istalpha isalpha
#define _istalnum isalnum
#define _istspace isspace
#define _istprint isprint

// Formatted I/O
#define _stprintf sprintf
#define _sntprintf snprintf
#define _vsprintf vsprintf
#define _vsnprintf vsnprintf
#define _tprintf printf
#define _ftprintf fprintf
#define _stscanf sscanf

// File I/O
#define _tfopen fopen
#define _tfreopen freopen
#define _tperror perror
#define _tremove remove
#define _trename rename

// String conversion
#define _tcstol strtol
#define _tcstoul strtoul
#define _tcstod strtod
#define _tstoi atoi
#define _tstol atol
#define _tstof atof

// Environment
#define _tgetenv getenv
#define _tputenv putenv

#else

// Unicode (wchar_t-based) definitions
typedef wchar_t TCHAR;
typedef wchar_t* PTCHAR;
typedef wchar_t* LPTSTR;
typedef const wchar_t* LPCTSTR;

// String functions
#define _T(x) L##x
#define _TEXT(x) L##x

#define _tcscpy wcscpy
#define _tcsncpy wcsncpy
#define _tcscat wcscat
#define _tcsncat wcsncat
#define _tcslen wcslen
#define _tcscmp wcscmp
#define _tcsncmp wcsncmp
#define _tcsicmp wcscasecmp
#define _tcsnicmp wcsncasecmp
#define _tcschr wcschr
#define _tcsrchr wcsrchr
#define _tcsstr wcsstr
#define _tcsspn wcsspn
#define _tcscspn wcscspn
#define _tcstok wcstok
#define _tcsnlen wcsnlen
#define _tcsdup wcsdup

// Character functions
#define _totlower towlower
#define _totupper towupper
#define _istdigit iswdigit
#define _istalpha iswalpha
#define _istalnum iswalnum
#define _istspace iswspace
#define _istprint iswprint

// Formatted I/O
#define _stprintf swprintf
#define _sntprintf swprintf
#define _vsprintf vswprintf
#define _vsnprintf vswprintf
#define _tprintf wprintf
#define _ftprintf fwprintf
#define _stscanf swscanf

// File I/O (these would need wide character file function implementations)
#define _tfopen fopen  // Simplified - would need proper wide char implementation
#define _tfreopen freopen
#define _tperror perror
#define _tremove remove
#define _trename rename

// String conversion
#define _tcstol wcstol
#define _tcstoul wcstoul
#define _tcstod wcstod
#define _tstoi _wtoi
#define _tstol _wtol
#define _tstof _wtof

// Environment (simplified - would need wide char implementations)
#define _tgetenv getenv
#define _tputenv putenv

// Helper functions for wide character conversion (stubs)
inline int _wtoi(const wchar_t* str) {
    char buffer[32];
    wcstombs(buffer, str, sizeof(buffer));
    return atoi(buffer);
}

inline long _wtol(const wchar_t* str) {
    char buffer[32];
    wcstombs(buffer, str, sizeof(buffer));
    return atol(buffer);
}

inline double _wtof(const wchar_t* str) {
    char buffer[64];
    wcstombs(buffer, str, sizeof(buffer));
    return atof(buffer);
}

#endif // _UNICODE

// Additional TCHAR macros for compatibility
#define TCHAR_MAX 255
#define _TEOF EOF

// File mode constants
#define _T_READ "r"
#define _T_WRITE "w"
#define _T_APPEND "a"
#define _T_READWRITE "r+"

#endif // __APPLE__
#endif // _TCHAR_H_
