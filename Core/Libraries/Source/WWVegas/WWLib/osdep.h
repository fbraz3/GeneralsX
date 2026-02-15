#ifndef OSDEP_H
#define OSDEP_H

#ifdef _UNIX
#include <alloca.h>
#define _alloca alloca
#include <ctype.h>
#include <cstring>
#include <wchar.h>
typedef char TCHAR;
typedef wchar_t WCHAR; 
#define _tcslen strlen
#define _tcsclen strlen
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _wcsicmp wcscasecmp 
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define strcmpi strcasecmp

#define _vsnprintf vsnprintf
#define _snprintf snprintf
#define _strdup strdup      // GeneralsX @build BenderAI 10/02/2026 - String duplication
#define lstrcpy strcpy      // GeneralsX @build BenderAI 10/02/2026 - String copy
#define lstrcpyn strncpy    // GeneralsX @build BenderAI 10/02/2026 - String copy with length
#define lstrcat strcat      // GeneralsX @build BenderAI 10/02/2026 - String concatenation

// GeneralsX @TheSuperHackers @build BenderAI 11/02/2026 Only define strupr/strrev if not already provided by compat headers
// Note: Check for macro definition, not function existence
#if !defined(strupr)
static char *strupr(char *str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);

    return str;
}
#endif

#if !defined(strrev)
static char *strrev(char *str)
{
    if (!str || ! *str)
        return str;

    int i = strlen(str) - 1, j = 0;

    char ch;
    while (i > j)
    {
        ch = str[i];
        str[i] = str[j];
        str[j] = ch;
        i--;
        j++;
    }
    return str;
}
#endif

#endif // _UNIX

#endif // OSDEP_H
