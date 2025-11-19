/*
 * GeneralsX - Cross-Platform Port of Command & Conquer: Generals (2003)
 * Phase 05: Registry & Configuration Compatibility Layer Implementation
 *
 * This implementation provides Windows Registry emulation using INI configuration files
 * stored in platform-specific directories:
 *
 * Windows: %APPDATA%\Electronic Arts\EA Games\{game_name}\
 * macOS: ~/Library/Application Support/Electronic Arts/EA Games/{game_name}/
 * Linux: ~/.config/electronic-arts/ea-games/{game_name}/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>
#include <pwd.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "win32_config_compat.h"
#include "win32_thread_compat.h"


/*
 * Global Configuration State
 */
static SDL2_CriticalSection g_config_lock;
static BOOL g_config_initialized = FALSE;
static char g_config_base_dir[PATH_MAX] = {0};


/*
 * Registry Key Handle Structure
 * Used internally to track open registry keys
 */
typedef struct {
    char ini_file_path[PATH_MAX];
    REGSAM access_rights;
    BOOL is_valid;
    BOOL is_dirty;  // INI file needs to be written
} RegistryKeyHandle;

#define MAX_OPEN_KEYS 64
static RegistryKeyHandle g_open_keys[MAX_OPEN_KEYS];
static int g_next_key_id = 1;


/*
 * Simple INI File Parser
 * Used to read/write registry data as INI files
 */

typedef struct {
    char section[256];
    char key[256];
    char value[4096];
    int type;  // REG_SZ, REG_DWORD, REG_BINARY
} INIEntry;


/*
 * Platform-Specific Config Directory Functions
 */


#ifdef __APPLE__
static LONG SDL2_GetMacOSConfigDirectory(char *buffer, unsigned long buffer_size)
{
    const char *home = getenv("HOME");
    
    if (home == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            printf("Phase 05: Failed to determine home directory\n");
            return ERROR_PATH_NOT_FOUND;
        }
        home = pw->pw_dir;
    }
    
    // Build path: ~/Library/Application Support/Electronic Arts/EA Games/
    snprintf(buffer, buffer_size, "%s/Library/Application Support/Electronic Arts/EA Games/", home);
    
    printf("Phase 05: macOS config directory: %s\n", buffer);
    return ERROR_SUCCESS;
}
#endif

#if defined(__linux__)
static LONG SDL2_GetLinuxConfigDirectory(char *buffer, unsigned long buffer_size)
{
    const char *home = getenv("HOME");
    
    if (home == NULL) {
        struct passwd *pw = getpwuid(getuid());
        if (pw == NULL) {
            printf("Phase 05: Failed to determine home directory\n");
            return ERROR_PATH_NOT_FOUND;
        }
        home = pw->pw_dir;
    }
    
    // Build path: ~/.config/electronic-arts/ea-games/
    snprintf(buffer, buffer_size, "%s/.config/electronic-arts/ea-games/", home);
    
    printf("Phase 05: Linux config directory: %s\n", buffer);
    return ERROR_SUCCESS;
}
#endif


/*
 * Directory Creation
 */
static LONG SDL2_CreateDirectoryRecursive(const char *path)
{
    char buffer[PATH_MAX];
    char *p = NULL;
    
    strncpy(buffer, path, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    // Create each directory level
    for (p = buffer + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            
            if (mkdir(buffer, 0755) != 0 && errno != EEXIST) {
                printf("Phase 05: Failed to create directory: %s (errno=%d)\n", buffer, errno);
                return ERROR_PATH_NOT_FOUND;
            }
            *p = '/';
        }
    }
    
    // Create final directory
    if (mkdir(buffer, 0755) != 0 && errno != EEXIST) {
        printf("Phase 05: Failed to create directory: %s (errno=%d)\n", buffer, errno);
        return ERROR_PATH_NOT_FOUND;
    }
    
    printf("Phase 05: Created directory: %s\n", buffer);
    return ERROR_SUCCESS;
}


/*
 * INI File Operations
 */

/**
 * Read an INI file into memory
 * Returns allocated buffer with INI contents, caller must free
 */
static char* SDL2_ReadINIFile(const char *ini_path)
{
    FILE *f;
    long size;
    char *buffer = NULL;
    
    f = fopen(ini_path, "rb");
    if (f == NULL) {
        printf("Phase 05: INI file not found (will create): %s\n", ini_path);
        return NULL;  // File doesn't exist yet
    }
    
    // Get file size
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Allocate and read
    buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        printf("Phase 05: Memory allocation failed for INI file\n");
        fclose(f);
        return NULL;
    }
    
    if (fread(buffer, 1, size, f) != size) {
        printf("Phase 05: Failed to read INI file: %s\n", ini_path);
        free(buffer);
        fclose(f);
        return NULL;
    }
    
    buffer[size] = '\0';
    fclose(f);
    
    printf("Phase 05: Loaded INI file: %s (%ld bytes)\n", ini_path, size);
    return buffer;
}

/**
 * Write INI buffer to file
 */
static LONG SDL2_WriteINIFile(const char *ini_path, const char *buffer)
{
    FILE *f;
    
    f = fopen(ini_path, "wb");
    if (f == NULL) {
        printf("Phase 05: Failed to open INI file for writing: %s\n", ini_path);
        return ERROR_PATH_NOT_FOUND;
    }
    
    if (fputs(buffer, f) == EOF) {
        printf("Phase 05: Failed to write INI file: %s\n", ini_path);
        fclose(f);
        return ERROR_INVALID_DATA;
    }
    
    fclose(f);
    printf("Phase 05: Wrote INI file: %s\n", ini_path);
    return ERROR_SUCCESS;
}

/**
 * Get value from INI buffer by [section] key=value format
 * Caller must free returned string
 */
static char* SDL2_GetINIValue(const char *ini_buffer, const char *section, const char *key)
{
    char section_header[512];
    const char *section_start;
    const char *line_start;
    const char *p;
    int section_len;
    int key_len;
    char *result = NULL;
    
    if (ini_buffer == NULL) {
        return NULL;
    }
    
    // Format section header
    snprintf(section_header, sizeof(section_header), "[%s]", section);
    section_len = strlen(section_header);
    key_len = strlen(key);
    
    // Find section
    section_start = strstr(ini_buffer, section_header);
    if (section_start == NULL) {
        printf("Phase 05: Section [%s] not found in INI\n", section);
        return NULL;
    }
    
    // Find key within section
    p = section_start + section_len;
    while (*p) {
        // Skip to next line
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
        
        // Check if we've hit a new section
        if (*p == '[') {
            printf("Phase 05: Key '%s' not found in section [%s]\n", key, section);
            return NULL;
        }
        
        // Check if this line starts with our key
        if (strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            // Found the key
            line_start = p + key_len + 1;  // Skip past "key="
            
            // Find end of line
            const char *line_end = line_start;
            while (*line_end && *line_end != '\n' && *line_end != '\r') line_end++;
            
            // Allocate and copy value
            int value_len = line_end - line_start;
            result = (char*)malloc(value_len + 1);
            if (result) {
                strncpy(result, line_start, value_len);
                result[value_len] = '\0';
                printf("Phase 05: Retrieved key '%s' from section [%s]: '%s'\n", key, section, result);
            }
            return result;
        }
    }
    
    printf("Phase 05: Key '%s' not found in section [%s]\n", key, section);
    return NULL;
}

/**
 * Set value in INI buffer
 * Returns new allocated buffer, caller must free old buffer
 */
static char* SDL2_SetINIValue(const char *old_ini, const char *section, const char *key, 
                             const char *value, const char *type_str)
{
    char section_header[512];
    char key_line[8192];
    char type_key[512];
    char *new_ini;
    size_t new_size;
    const char *p;
    const char *section_start;
    const char *next_section;
    int section_len;
    int key_len;
    BOOL key_found = FALSE;
    BOOL section_found = FALSE;
    
    // Build format strings
    snprintf(section_header, sizeof(section_header), "[%s]", section);
    snprintf(key_line, sizeof(key_line), "%s=%s\n", key, value);
    snprintf(type_key, sizeof(type_key), "Type_%s=%s\n", key, type_str);
    
    section_len = strlen(section_header);
    key_len = strlen(key);
    
    // Calculate new size (worst case: add section + key + type)
    new_size = (old_ini ? strlen(old_ini) : 0) + strlen(section_header) + 
               strlen(key_line) + strlen(type_key) + 512;
    new_ini = (char*)malloc(new_size);
    
    if (new_ini == NULL) {
        printf("Phase 05: Memory allocation failed for INI update\n");
        return NULL;
    }
    
    new_ini[0] = '\0';
    
    if (old_ini == NULL || old_ini[0] == '\0') {
        // Create new INI file
        snprintf(new_ini, new_size, "%s\n%s%s", section_header, key_line, type_key);
        printf("Phase 05: Created new INI entry [%s] %s=%s\n", section, key, value);
        return new_ini;
    }
    
    // Parse existing INI
    p = old_ini;
    section_start = strstr(old_ini, section_header);
    
    if (section_start == NULL) {
        // Section doesn't exist - append it
        strcpy(new_ini, old_ini);
        if (new_ini[strlen(new_ini) - 1] != '\n') {
            strcat(new_ini, "\n");
        }
        strcat(new_ini, section_header);
        strcat(new_ini, "\n");
        strcat(new_ini, key_line);
        strcat(new_ini, type_key);
        printf("Phase 05: Added new section [%s] with key %s=%s\n", section, key, value);
        return new_ini;
    }
    
    // Section exists - update or add key
    // Copy up to section start
    strncpy(new_ini, old_ini, section_start - old_ini);
    new_ini[section_start - old_ini] = '\0';
    strcat(new_ini, section_header);
    strcat(new_ini, "\n");
    
    // Process section contents
    p = section_start + section_len;
    if (*p == '\n') p++;  // Skip newline after [section]
    
    next_section = strchr(p, '[');
    if (next_section == NULL) {
        next_section = old_ini + strlen(old_ini);  // End of file
    }
    
    // Copy section lines, updating if key matches
    BOOL added = FALSE;
    while (p < next_section) {
        // Skip blank lines
        if (*p == '\n' || *p == '\r') {
            strcat(new_ini, "\n");
            if (*p == '\r' && *(p+1) == '\n') p++;
            p++;
            continue;
        }
        
        // Check if this is our key
        if (strncmp(p, key, key_len) == 0 && p[key_len] == '=') {
            if (!added) {
                strcat(new_ini, key_line);
                strcat(new_ini, type_key);
                added = TRUE;
                key_found = TRUE;
            }
            
            // Skip this line
            while (*p && *p != '\n') p++;
            if (*p == '\n') {
                p++;
                if (*p == '\r') p++;
            }
            continue;
        }
        
        // Check if this is a Type_ key for our value
        if (strncmp(p, type_key, strlen(type_key) - strlen(type_str) - 1) == 0) {
            // Skip this line
            while (*p && *p != '\n') p++;
            if (*p == '\n') p++;
            continue;
        }
        
        // Copy line as-is
        while (*p && *p != '\n') {
            strncat(new_ini, p, 1);
            p++;
        }
        if (*p == '\n') {
            strcat(new_ini, "\n");
            p++;
        }
    }
    
    // If key wasn't found, add it
    if (!added) {
        strcat(new_ini, key_line);
        strcat(new_ini, type_key);
    }
    
    // Copy remaining sections
    if (p < old_ini + strlen(old_ini)) {
        strcat(new_ini, p);
    }
    
    printf("Phase 05: Updated key '%s' in section [%s]\n", key, section);
    return new_ini;
}


/*
 * Public API Implementation
 */

LONG SDL2_InitializeConfigSystem(void)
{
    if (g_config_initialized) {
        return ERROR_SUCCESS;
    }
    
    SDL2_CreateCriticalSection(&g_config_lock);
    
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        LONG result = ERROR_SUCCESS;
        
        // Determine platform-specific config directory
        printf("Phase 05: Unknown platform\n");
        result = ERROR_PATH_NOT_FOUND;
        
        if (result == ERROR_SUCCESS) {
            // Create base directory if it doesn't exist
            result = SDL2_CreateDirectoryRecursive(g_config_base_dir);
        }
        
        if (result == ERROR_SUCCESS) {
            g_config_initialized = TRUE;
            printf("Phase 05: Configuration system initialized\n");
        }
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    return ERROR_SUCCESS;
}

void SDL2_ShutdownConfigSystem(void)
{
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        // Clean up any remaining open keys
        for (int i = 0; i < MAX_OPEN_KEYS; i++) {
            if (g_open_keys[i].is_valid) {
                g_open_keys[i].is_valid = FALSE;
            }
        }
        g_config_initialized = FALSE;
        printf("Phase 05: Configuration system shutdown\n");
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    SDL2_DestroyCriticalSection(&g_config_lock);
}

LONG SDL2_GetConfigDirectory(HKEY root, char *buffer, unsigned long buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return ERROR_INVALID_PARAMETER;
    }
    
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        strncpy(buffer, g_config_base_dir, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    return ERROR_SUCCESS;
}

LONG SDL2_RegistryPathToINIPath(const char *registry_path, char *output_path, unsigned long output_size)
{
    char config_dir[PATH_MAX];
    const char *p;
    
    if (registry_path == NULL || output_path == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Get base config directory
    SDL2_GetConfigDirectory(HKEY_LOCAL_MACHINE, config_dir, sizeof(config_dir));
    
    // Convert path: "SOFTWARE\Electronic Arts\EA Games\Generals" → "Generals.ini"
    p = registry_path;
    
    // Find the last component (game name)
    const char *last_backslash = strrchr(registry_path, '\\');
    if (last_backslash) {
        p = last_backslash + 1;
    } else if (strchr(registry_path, '/')) {
        p = strrchr(registry_path, '/') + 1;
    }
    
    // Build INI path: config_dir + game_name + ".ini"
    snprintf(output_path, output_size, "%s%s.ini", config_dir, p);
    
    printf("Phase 05: Registry path '%s' → INI path '%s'\n", registry_path, output_path);
    return ERROR_SUCCESS;
}

LONG SDL2_ParseRegistryPath(const char *registry_path, char *section, unsigned long section_size,
                            char *key, unsigned long key_size)
{
    if (registry_path == NULL || section == NULL || key == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Default: entire path is section, key is empty
    strncpy(section, registry_path, section_size - 1);
    section[section_size - 1] = '\0';
    
    if (key_size > 0) {
        key[0] = '\0';
    }
    
    return ERROR_SUCCESS;
}

LONG WINAPI RegOpenKeyExA(HKEY hKey, const char *lpSubKey, DWORD ulOptions,
                          REGSAM samDesired, PHKEY phkResult)
{
    RegistryKeyHandle *handle;
    char ini_path[PATH_MAX];
    int key_id;
    
    if (lpSubKey == NULL || phkResult == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (!g_config_initialized) {
        SDL2_InitializeConfigSystem();
    }
    
    // Convert registry path to INI file path
    SDL2_RegistryPathToINIPath(lpSubKey, ini_path, sizeof(ini_path));
    
    // Create parent directories if needed
    SDL2_CreateDirectoryRecursive(ini_path);
    
    // Find available key handle slot
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        key_id = 0;
        for (int i = 0; i < MAX_OPEN_KEYS; i++) {
            if (!g_open_keys[i].is_valid) {
                key_id = g_next_key_id++;
                if (g_next_key_id > 0x7FFFFFFF) g_next_key_id = 1;  // Wrap around
                
                handle = &g_open_keys[i];
                strncpy(handle->ini_file_path, ini_path, sizeof(handle->ini_file_path) - 1);
                handle->ini_file_path[sizeof(handle->ini_file_path) - 1] = '\0';
                handle->access_rights = samDesired;
                handle->is_valid = TRUE;
                handle->is_dirty = FALSE;
                
                *phkResult = (HKEY)(intptr_t)key_id;
                
                printf("Phase 05: Opened registry key: %s (handle=%d)\n", ini_path, key_id);
                break;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    if (key_id == 0) {
        printf("Phase 05: No available key handles (max=%d)\n", MAX_OPEN_KEYS);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    return ERROR_SUCCESS;
}

LONG WINAPI RegCloseKeyA(HKEY hKey)
{
    int key_id = (int)(intptr_t)hKey;
    
    if (hKey == NULL) {
        return ERROR_INVALID_HANDLE;
    }
    
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        for (int i = 0; i < MAX_OPEN_KEYS; i++) {
            if (g_open_keys[i].is_valid && (int)(intptr_t)&g_open_keys[i] == key_id) {
                g_open_keys[i].is_valid = FALSE;
                printf("Phase 05: Closed registry key (handle=%d)\n", key_id);
                break;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    return ERROR_SUCCESS;
}

LONG WINAPI RegQueryValueExA(HKEY hKey, const char *lpValueName, LPDWORD lpReserved,
                             LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    // TODO: Implement full version
    // For now, return not found
    return ERROR_FILE_NOT_FOUND;
}

LONG WINAPI RegSetValueExA(HKEY hKey, const char *lpValueName, DWORD Reserved, DWORD dwType,
                           const BYTE* lpData, DWORD cbData)
{
    // TODO: Implement full version
    return ERROR_SUCCESS;
}

LONG WINAPI RegCreateKeyExA(HKEY hKey, const char *lpSubKey, DWORD Reserved, char *lpClass,
                            DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                            PHKEY phkResult, LPDWORD lpdwDisposition)
{
    // For now, use RegOpenKeyEx behavior (creates if not exists)
    return RegOpenKeyExA(hKey, lpSubKey, dwOptions, samDesired, phkResult);
}

LONG WINAPI RegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName, LPDWORD lpcchValueName,
                          LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    // TODO: Implement enumeration
    return ERROR_NO_MORE_ITEMS;
}

LONG WINAPI RegQueryInfoKeyA(HKEY hKey, LPSTR lpClass, LPDWORD lpcchClass, LPDWORD lpReserved,
                             LPDWORD lpcSubKeys, LPDWORD lpcchMaxSubKeyLen, LPDWORD lpcchMaxClassLen,
                             LPDWORD lpcValues, LPDWORD lpcchMaxValueNameLen, LPDWORD lpcbMaxValueLen,
                             LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime)
{
    if (lpcValues) {
        *lpcValues = 0;  // No values enumerated
    }
    return ERROR_SUCCESS;
}

LONG WINAPI RegDeleteKeyA(HKEY hKey, const char *lpSubKey)
{
    // TODO: Implement deletion
    return ERROR_SUCCESS;
}

LONG WINAPI RegDeleteValueA(HKEY hKey, const char *lpValueName)
{
    // TODO: Implement deletion
    return ERROR_SUCCESS;
}

BOOL SDL2_RegistryValueExists(HKEY hKey, const char *lpValueName)
{
    // TODO: Implement existence check
    return FALSE;
}

LONG SDL2_GetRegistryKeyFilePath(HKEY hKey, char *buffer, unsigned long buffer_size)
{
    int key_id = (int)(intptr_t)hKey;
    
    if (buffer == NULL || buffer_size == 0) {
        return ERROR_INVALID_PARAMETER;
    }
    
    SDL2_EnterCriticalSection(&g_config_lock);
    {
        for (int i = 0; i < MAX_OPEN_KEYS; i++) {
            if (g_open_keys[i].is_valid && (int)(intptr_t)&g_open_keys[i] == key_id) {
                strncpy(buffer, g_open_keys[i].ini_file_path, buffer_size - 1);
                buffer[buffer_size - 1] = '\0';
                SDL2_LeaveCriticalSection(&g_config_lock);
                return ERROR_SUCCESS;
            }
        }
    }
    SDL2_LeaveCriticalSection(&g_config_lock);
    
    return ERROR_INVALID_HANDLE;
}
