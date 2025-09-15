#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#ifndef _WIN32

// Standard C libraries needed for compatibility functions
#include <string.h>
#include <strings.h>  // for strcasecmp
#include <unistd.h>   // for getcwd
#include <sys/stat.h> // for stat

// Windows compatibility layer for non-Windows platforms

// Include base compatibility
#include "win32_compat.h"

// Include additional compatibility layers
#include "threading.h"
#include "filesystem.h"  
#include "network.h"
#include "string_compat.h"

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

// Registry creation disposition
#define REG_CREATED_NEW_KEY     0x00000001
#define REG_OPENED_EXISTING_KEY 0x00000002

// Bitmap compression constants
#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3

// DirectX texture transform flags
#define D3DTTFF_DISABLE         0
#define D3DTTFF_COUNT1          1
#define D3DTTFF_COUNT2          2
#define D3DTTFF_COUNT3          3
#define D3DTTFF_COUNT4          4
#define D3DTTFF_PROJECTED       256

// Error codes
#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_ACCESS_DENIED 5L
#define ERROR_MORE_DATA 234L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_INVALID_PARAMETER 87L

// Additional Registry constants
#define REG_OPTION_NON_VOLATILE 0x00000000L

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

// MessageBox constants
#define MB_OK                0x00000000L
#define MB_ICONERROR         0x00000010L
#define MB_SYSTEMMODAL       0x00001000L

// Additional types
typedef void* HKEY;
typedef HKEY* PHKEY;

// Stub functions for Windows API
inline DWORD GetLastError() { return 0; }
inline void SetLastError(DWORD dwErrCode) {}
inline int MessageBoxA(void* hWnd, const char* lpText, const char* lpCaption, UINT uType) { return IDOK; }
inline int MessageBox(void* hWnd, const char* lpText, const char* lpCaption, UINT uType) { return IDOK; }
inline int MessageBoxW(void* hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType) { return IDOK; }
inline HANDLE CreateFileA(const char* lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) { return INVALID_HANDLE_VALUE; }
// CloseHandle is defined in threading.h
// ReadFile and WriteFile are defined in filesystem.h
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
static int g_nextHandleCounter = 1000;

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
    
    *phkResult = (HKEY)(uintptr_t)g_nextHandleCounter++;
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

// String and file functions provided by win32_compat.h
// (lstrcmpi, GetCurrentDirectory, GetFileAttributes)

// Dynamic library functions defined in win32_compat.h
// (LoadLibrary, GetProcAddress, FreeLibrary)

#endif // !_WIN32

#endif // WIN32_COMPAT_H
