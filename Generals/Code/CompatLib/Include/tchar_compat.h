#pragma once

#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcsclen strlen
#define _tcscpy strcpy

// These need to match windows_base.h from DXVK, as long as it's being used
typedef char TCHAR;
typedef const TCHAR* LPCTSTR;
typedef TCHAR* LPTSTR;