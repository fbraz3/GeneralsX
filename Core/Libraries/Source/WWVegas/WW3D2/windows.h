#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#ifndef _WIN32

// Windows compatibility layer for non-Windows platforms

// Include base compatibility
#include "win32_compat.h"

// Additional Windows types
#ifndef HANDLE
typedef void* HANDLE;
#endif
#ifndef LONG_PTR
typedef long LONG_PTR;
#endif

// Additional Windows constants and macros
#define WINAPI
#define CALLBACK
#define PASCAL
#define CDECL
#define APIENTRY

// File handling constants
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define GENERIC_READ 0x80000000L
#define GENERIC_WRITE 0x40000000L
#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

// Registry constants
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)((LONG)0x80000001))
#define HKEY_LOCAL_MACHINE ((HKEY)(ULONG_PTR)((LONG)0x80000002))
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define REG_SZ 1
#define REG_DWORD 4

// Error codes
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_ACCESS_DENIED 5L

// Message box constants
#define MB_OK 0x00000000L
#define MB_OKCANCEL 0x00000001L
#define MB_YESNO 0x00000004L
#define MB_ICONERROR 0x00000010L
#define MB_ICONWARNING 0x00000030L
#define MB_ICONINFORMATION 0x00000040L

#define IDOK 1
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

// Additional types
typedef void* HKEY;
typedef HKEY* PHKEY;

// Stub functions for Windows API
inline DWORD GetLastError() { return 0; }
inline void SetLastError(DWORD dwErrCode) {}
inline int MessageBoxA(void* hWnd, const char* lpText, const char* lpCaption, UINT uType) { return IDOK; }
inline HANDLE CreateFileA(const char* lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) { return INVALID_HANDLE_VALUE; }
inline BOOL CloseHandle(HANDLE hObject) { return TRUE; }
inline BOOL ReadFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped) { return FALSE; }
inline BOOL WriteFile(HANDLE hFile, const void* lpBuffer, DWORD nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten, void* lpOverlapped) { return FALSE; }
inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh) { return 0; }

// Registry functions
inline LONG RegOpenKeyExA(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, PHKEY phkResult) { return ERROR_FILE_NOT_FOUND; }
inline LONG RegCloseKey(HKEY hKey) { return ERROR_SUCCESS; }
inline LONG RegQueryValueExA(HKEY hKey, const char* lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData) { return ERROR_FILE_NOT_FOUND; }
inline LONG RegSetValueExA(HKEY hKey, const char* lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) { return ERROR_ACCESS_DENIED; }

#endif // !_WIN32

#endif // WINDOWS_COMPAT_H
