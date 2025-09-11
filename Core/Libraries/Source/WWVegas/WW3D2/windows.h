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
#ifndef ULONG_PTR
typedef unsigned long ULONG_PTR;
#endif
#ifndef DWORD
typedef unsigned long DWORD;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef unsigned short WORD;
#endif
#ifndef LONG
typedef long LONG;
#endif
#ifndef HKEY
typedef HANDLE HKEY;
typedef HKEY* PHKEY;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef LPSTR
typedef char* LPSTR;
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

// Forward declaration to avoid circular includes
class ConfigManager;
extern ConfigManager* g_configManager;

// Registry functions - now implemented using ConfigManager
#include <map>

// Global registry "handles" - simplified approach using registry path strings
struct RegistryHandle {
    std::string path;
    bool valid;
    RegistryHandle(const std::string& p = "") : path(p), valid(!p.empty()) {}
};

static std::map<HKEY, RegistryHandle> g_registryHandles;
static HKEY g_nextHandle = (HKEY)1000;

inline LONG RegOpenKeyEx(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult) {
    if (!g_configManager) return ERROR_FILE_NOT_FOUND;
    
    std::string fullPath;
    if (hKey == HKEY_LOCAL_MACHINE || hKey == HKEY_CURRENT_USER) {
        fullPath = lpSubKey ? lpSubKey : "";
    } else {
        auto it = g_registryHandles.find(hKey);
        if (it != g_registryHandles.end() && it->second.valid) {
            fullPath = it->second.path + (lpSubKey ? std::string("\\") + lpSubKey : "");
        } else {
            return ERROR_FILE_NOT_FOUND;
        }
    }
    
    *phkResult = g_nextHandle++;
    g_registryHandles[*phkResult] = RegistryHandle(fullPath);
    return ERROR_SUCCESS;
}

inline LONG RegOpenKeyExA(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, PHKEY phkResult) {
    return RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

inline LONG RegQueryValueEx(HKEY hKey, const char* lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData) {
    if (!g_configManager || !lpValueName) return ERROR_FILE_NOT_FOUND;
    
    auto it = g_registryHandles.find(hKey);
    if (it == g_registryHandles.end() || !it->second.valid) {
        return ERROR_FILE_NOT_FOUND;
    }
    
    // For now, return simple stub - full implementation will be added when ConfigManager is available
    return ERROR_FILE_NOT_FOUND;
}

inline LONG RegQueryValueExA(HKEY hKey, const char* lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData) {
    return RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

inline LONG RegSetValueEx(HKEY hKey, const char* lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) {
    if (!g_configManager || !lpValueName || !lpData) return ERROR_ACCESS_DENIED;
    
    auto it = g_registryHandles.find(hKey);
    if (it == g_registryHandles.end() || !it->second.valid) {
        return ERROR_ACCESS_DENIED;
    }
    
    // For now, return simple stub - full implementation will be added when ConfigManager is available
    return ERROR_ACCESS_DENIED;
}

inline LONG RegSetValueExA(HKEY hKey, const char* lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) {
    return RegSetValueEx(hKey, lpValueName, Reserved, dwType, lpData, cbData);
}

inline LONG RegCloseKey(HKEY hKey) {
    auto it = g_registryHandles.find(hKey);
    if (it != g_registryHandles.end()) {
        g_registryHandles.erase(it);
        return ERROR_SUCCESS;
    }
    return ERROR_INVALID_HANDLE;
}

inline LONG RegCreateKeyEx(HKEY hKey, const char* lpSubKey, DWORD Reserved, char* lpClass, DWORD dwOptions, DWORD samDesired, void* lpSecurityAttributes, PHKEY phkResult, DWORD* lpdwDisposition) { 
    // For simplicity, create key is the same as open key in our implementation
    LONG result = RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, phkResult);
    if (lpdwDisposition) *lpdwDisposition = REG_CREATED_NEW_KEY;
    return result;
}
inline LONG RegCreateKeyExA(HKEY hKey, const char* lpSubKey, DWORD Reserved, char* lpClass, DWORD dwOptions, DWORD samDesired, void* lpSecurityAttributes, PHKEY phkResult, DWORD* lpdwDisposition) { 
    return RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
}

// Registry constants
#define REG_OPTION_NON_VOLATILE 0x00000000L
#define KEY_READ 0x20019L
#define KEY_WRITE 0x20006L
#define REG_DWORD 4
#define REG_SZ 1
#define REG_CREATED_NEW_KEY 0x00000001L
#define ERROR_MORE_DATA 234L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_INVALID_PARAMETER 87L

#endif // !_WIN32

#endif // WINDOWS_COMPAT_H
