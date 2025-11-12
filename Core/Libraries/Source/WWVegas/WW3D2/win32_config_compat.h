/*
 * GeneralsX - Cross-Platform Port of Command & Conquer: Generals (2003)
 * Copyright (C) 2025
 *
 * Phase 05: Registry & Configuration Compatibility Layer
 *
 * This file provides cross-platform registry emulation using INI configuration files.
 * Windows Registry calls (RegOpenKeyEx, RegQueryValueEx, etc.) are mapped to INI file operations
 * stored in platform-appropriate locations:
 *
 * Platform Paths:
 * - Windows: %APPDATA%\Electronic Arts\EA Games\{game_name}\
 * - macOS: ~/Library/Application Support/Electronic Arts/EA Games/{game_name}/
 * - Linux: ~/.config/electronic-arts/ea-games/{game_name}/
 *
 * Registry Structure:
 * - HKEY_LOCAL_MACHINE registry paths → INI files in app config directory (system-wide)
 * - HKEY_CURRENT_USER registry paths → INI files in user config directory (user-specific)
 *
 * INI File Format:
 * [SectionName]
 * KeyName=ValueData
 * Type_KeyName=REG_DWORD|REG_SZ|REG_BINARY (internal type tracking)
 */

#ifndef __WIN32_CONFIG_COMPAT_H__
#define __WIN32_CONFIG_COMPAT_H__

#include <windows.h>  // For HKEY, DWORD, ERROR_SUCCESS, etc.

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Registry Root Key Compatibility
 * HKEY_LOCAL_MACHINE and HKEY_CURRENT_USER are mapped to different INI files
 */
#define HKEY_LOCAL_MACHINE      ((HKEY)0x80000002)  // System-wide settings
#define HKEY_CURRENT_USER       ((HKEY)0x80000001)  // User-specific settings


/*
 * Registry Key Constants (mimics Windows winreg.h)
 */
#define KEY_READ                0x00020019
#define KEY_WRITE               0x00020006
#define KEY_ALL_ACCESS          0x000F003F
#define REG_OPTION_NON_VOLATILE 0


/*
 * Registry Value Types
 */
#define REG_NONE                0
#define REG_SZ                  1  // String
#define REG_EXPAND_SZ           2  // Expandable string
#define REG_BINARY              3  // Binary
#define REG_DWORD               4  // 32-bit integer


/*
 * Function Prototypes
 */

/**
 * Initialize the configuration system
 * Must be called once before any registry operations
 * Platform detection and INI file directory creation happens here
 *
 * Returns: ERROR_SUCCESS (0) on success, non-zero error code on failure
 */
LONG SDL2_InitializeConfigSystem(void);

/**
 * Shutdown the configuration system
 * Cleanup and resource deallocation
 */
void SDL2_ShutdownConfigSystem(void);

/**
 * Get the base configuration directory for a given registry root
 *
 * Parameters:
 *   root: HKEY_LOCAL_MACHINE (system-wide) or HKEY_CURRENT_USER (user-specific)
 *   buffer: Output buffer for directory path
 *   buffer_size: Size of output buffer
 *
 * Returns: ERROR_SUCCESS (0) on success, ERROR_PATH_NOT_FOUND (-1) on failure
 */
LONG SDL2_GetConfigDirectory(HKEY root, char *buffer, unsigned long buffer_size);

/**
 * Convert registry path to INI file path
 *
 * Parameters:
 *   registry_path: Windows registry path (e.g., "SOFTWARE\\Electronic Arts\\EA Games\\Generals")
 *   output_path: Output INI file path
 *   output_size: Size of output buffer
 *
 * Returns: ERROR_SUCCESS (0) on success, negative value on failure
 */
LONG SDL2_RegistryPathToINIPath(const char *registry_path, char *output_path, unsigned long output_size);

/**
 * Extract section and key name from registry path
 *
 * Parameters:
 *   registry_path: Full registry path
 *   section: Output buffer for section name [out]
 *   section_size: Size of section buffer
 *   key: Output buffer for key name [out]
 *   key_size: Size of key buffer
 *
 * Returns: ERROR_SUCCESS (0) on success, negative on failure
 */
LONG SDL2_ParseRegistryPath(const char *registry_path, char *section, unsigned long section_size, 
                           char *key, unsigned long key_size);


/*
 * Registry API Compatibility Functions
 * These mimic Windows RegOpenKeyEx, RegQueryValueEx, etc.
 */

/**
 * Open a registry key
 * Mimics: RegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
 *
 * Parameters:
 *   hKey: Root key (HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER)
 *   lpSubKey: Subkey path (e.g., "SOFTWARE\\Electronic Arts\\EA Games\\Generals")
 *   ulOptions: Options (unused, Windows compatibility)
 *   samDesired: Access rights (KEY_READ, KEY_WRITE, etc.) - determines file mode
 *   phkResult: Output handle to opened key
 *
 * Returns: ERROR_SUCCESS (0) on success, non-zero on failure
 *
 * Notes:
 * - Creates directories and INI files as needed
 * - Returned handle is used by RegQueryValueEx, RegSetValueEx, RegCloseKey
 * - File is not locked; concurrent access may occur
 */
LONG WINAPI RegOpenKeyExA(HKEY hKey, const char *lpSubKey, DWORD ulOptions, 
                          REGSAM samDesired, PHKEY phkResult);

/**
 * Query a registry value
 * Mimics: RegQueryValueEx(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, 
 *                         LPBYTE lpData, LPDWORD lpcbData)
 *
 * Parameters:
 *   hKey: Key handle from RegOpenKeyEx
 *   lpValueName: Value name to query (e.g., "InstallPath")
 *   lpReserved: Reserved, must be NULL
 *   lpType: Output value type (REG_SZ, REG_DWORD, REG_BINARY, etc.)
 *   lpData: Output buffer for value data
 *   lpcbData: On input, size of lpData; on output, bytes written
 *
 * Returns: ERROR_SUCCESS (0) on success, ERROR_FILE_NOT_FOUND if key/value doesn't exist
 *
 * Notes:
 * - If lpData is NULL, returns required buffer size in lpcbData
 * - If buffer is too small, returns ERROR_MORE_DATA and required size
 */
LONG WINAPI RegQueryValueExA(HKEY hKey, const char *lpValueName, LPDWORD lpReserved,
                             LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

/**
 * Set a registry value
 * Mimics: RegSetValueEx(HKEY hKey, LPCTSTR lpValueName, DWORD Reserved, DWORD dwType,
 *                       const BYTE* lpData, DWORD cbData)
 *
 * Parameters:
 *   hKey: Key handle from RegOpenKeyEx or RegCreateKeyEx
 *   lpValueName: Value name (e.g., "InstallPath")
 *   Reserved: Reserved, must be 0
 *   dwType: Value type (REG_SZ for strings, REG_DWORD for integers, etc.)
 *   lpData: Pointer to value data
 *   cbData: Size of value data in bytes
 *
 * Returns: ERROR_SUCCESS (0) on success, non-zero on failure
 *
 * Notes:
 * - Creates value if it doesn't exist
 * - Overwrites existing value with same name
 * - Supports REG_SZ (strings), REG_DWORD (integers), REG_BINARY (raw data)
 */
LONG WINAPI RegSetValueExA(HKEY hKey, const char *lpValueName, DWORD Reserved, DWORD dwType,
                           const BYTE* lpData, DWORD cbData);

/**
 * Create a registry key
 * Mimics: RegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass,
 *                        DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
 *                        PHKEY phkResult, LPDWORD lpdwDisposition)
 *
 * Parameters:
 *   hKey: Root key (HKEY_LOCAL_MACHINE or HKEY_CURRENT_USER)
 *   lpSubKey: Subkey path
 *   Reserved: Reserved, must be 0
 *   lpClass: Class name (unused for INI files, Windows compatibility)
 *   dwOptions: Options (REG_OPTION_NON_VOLATILE recommended)
 *   samDesired: Access rights
 *   lpSecurityAttributes: Security attributes (unused, Windows compatibility)
 *   phkResult: Output handle to created key
 *   lpdwDisposition: Output - REG_CREATED_NEW_KEY or REG_OPENED_EXISTING_KEY
 *
 * Returns: ERROR_SUCCESS (0) on success
 *
 * Notes:
 * - Creates all missing subdirectories
 * - Equivalent to RegOpenKeyEx for INI system (creates if not exists)
 */
LONG WINAPI RegCreateKeyExA(HKEY hKey, const char *lpSubKey, DWORD Reserved, char *lpClass,
                            DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            PHKEY phkResult, LPDWORD lpdwDisposition);

/**
 * Close a registry key
 * Mimics: RegCloseKey(HKEY hKey)
 *
 * Parameters:
 *   hKey: Key handle from RegOpenKeyEx/RegCreateKeyEx
 *
 * Returns: ERROR_SUCCESS (0) on success
 *
 * Notes:
 * - Must be called after finishing with a registry key
 * - Flushes any pending writes to INI files
 * - Resource cleanup
 */
LONG WINAPI RegCloseKeyA(HKEY hKey);

/**
 * Enumerate registry values in a key
 * Mimics: RegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName,
 *                       LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
 *
 * Parameters:
 *   hKey: Key handle
 *   dwIndex: Index of value to enumerate (0-based)
 *   lpValueName: Output buffer for value name
 *   lpcchValueName: On input, size of name buffer; on output, length of name
 *   lpReserved: Reserved, must be NULL
 *   lpType: Output value type
 *   lpData: Output buffer for value data (NULL to skip)
 *   lpcbData: On input, size of data buffer; on output, bytes written
 *
 * Returns: ERROR_SUCCESS (0) on success, ERROR_NO_MORE_ITEMS when all values enumerated
 */
LONG WINAPI RegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName,
                          LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

/**
 * Query information about a registry key
 * Mimics: RegQueryInfoKeyA(HKEY hKey, ...)
 *
 * Parameters:
 *   hKey: Key handle
 *   [other parameters]: Windows compatibility - most unused for INI system
 *   lpcValues: Output - number of values in key
 *   [remaining parameters]: Mostly unused
 *
 * Returns: ERROR_SUCCESS (0) on success
 */
LONG WINAPI RegQueryInfoKeyA(HKEY hKey, LPSTR lpClass, LPDWORD lpcchClass, LPDWORD lpReserved,
                             LPDWORD lpcSubKeys, LPDWORD lpcchMaxSubKeyLen, LPDWORD lpcchMaxClassLen,
                             LPDWORD lpcValues, LPDWORD lpcchMaxValueNameLen, LPDWORD lpcbMaxValueLen,
                             LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);

/**
 * Delete a registry key
 * Mimics: RegDeleteKeyA(HKEY hKey, LPCTSTR lpSubKey)
 *
 * Parameters:
 *   hKey: Root key
 *   lpSubKey: Subkey path to delete
 *
 * Returns: ERROR_SUCCESS (0) on success, ERROR_FILE_NOT_FOUND if key doesn't exist
 */
LONG WINAPI RegDeleteKeyA(HKEY hKey, const char *lpSubKey);

/**
 * Delete a registry value
 * Mimics: RegDeleteValueA(HKEY hKey, LPCTSTR lpValueName)
 *
 * Parameters:
 *   hKey: Key handle
 *   lpValueName: Value name to delete
 *
 * Returns: ERROR_SUCCESS (0) on success, ERROR_FILE_NOT_FOUND if value doesn't exist
 */
LONG WINAPI RegDeleteValueA(HKEY hKey, const char *lpValueName);


/*
 * Additional Utility Functions
 */

/**
 * Check if a configuration value exists
 *
 * Parameters:
 *   hKey: Key handle
 *   lpValueName: Value name to check
 *
 * Returns: TRUE if value exists, FALSE otherwise
 */
BOOL SDL2_RegistryValueExists(HKEY hKey, const char *lpValueName);

/**
 * Get the full file path for a given registry key
 * Useful for debugging and direct file access if needed
 *
 * Parameters:
 *   hKey: Key handle from RegOpenKeyEx
 *   buffer: Output buffer for file path
 *   buffer_size: Size of output buffer
 *
 * Returns: ERROR_SUCCESS (0) on success, negative on failure
 */
LONG SDL2_GetRegistryKeyFilePath(HKEY hKey, char *buffer, unsigned long buffer_size);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __WIN32_CONFIG_COMPAT_H__
