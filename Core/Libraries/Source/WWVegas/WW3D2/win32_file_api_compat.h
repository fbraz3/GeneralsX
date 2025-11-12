/**
 * @file win32_file_api_compat.h
 * @brief Win32 File API to SDL/POSIX compatibility layer
 * @details Provides abstract FILE* interface for cross-platform file I/O
 *
 * This header implements the File API compatibility layer that abstracts:
 * - Windows file operations (CreateFileA, ReadFile, WriteFile)
 * - Path normalization (backslashes → forward slashes)
 * - Virtual File System (VFS) integration with .big archives
 * - Binary and text mode file access
 *
 * Pattern: source_dest_type_compat
 * Source: Win32 File API
 * Destination: SDL/POSIX file operations
 * Type: File I/O compatibility
 */

#pragma once

#ifndef WIN32_FILE_API_COMPAT_H_INCLUDED
#define WIN32_FILE_API_COMPAT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * FILE PATH UTILITIES - Path normalization for cross-platform compatibility
 * ============================================================================ */

/**
 * @brief Normalize file path (backslashes → forward slashes)
 * @param path Input path (may contain backslashes)
 * @return Normalized path (forward slashes only)
 *
 * Example:
 *   Input:  "Data\INI\Object\AirforceGeneral.ini"
 *   Output: "Data/INI/Object/AirforceGeneral.ini"
 */
static inline char* Win32_NormalizeFilePath(const char* path)
{
    if (!path) return NULL;
    
    static char normalized_path[4096];
    strncpy(normalized_path, path, sizeof(normalized_path) - 1);
    normalized_path[sizeof(normalized_path) - 1] = '\0';
    
    /* Convert backslashes to forward slashes */
    for (size_t i = 0; normalized_path[i]; i++) {
        if (normalized_path[i] == '\\') {
            normalized_path[i] = '/';
        }
    }
    
    return normalized_path;
}

/* ============================================================================
 * FILE MODE CONVERSIONS - Convert Win32 file modes to POSIX modes
 * ============================================================================ */

/**
 * @brief Convert Win32 file access flags to POSIX mode string
 * @param access Win32-style access flags
 * @return POSIX mode string ("r", "w", "rb", "wb", etc.)
 *
 * Win32 access patterns:
 *   GENERIC_READ      → "rb" (binary read)
 *   GENERIC_WRITE     → "wb" (binary write)
 *   GENERIC_READ|WRITE → "r+b" (binary read/write)
 *
 * Note: Always defaults to binary mode for safety with .big files
 */
static inline const char* Win32_ConvertAccessToMode(int access)
{
    /* Simplified mapping - extend as needed */
    if ((access & 0x80000000) && (access & 0x40000000)) {
        /* GENERIC_READ | GENERIC_WRITE */
        return "r+b";
    } else if (access & 0x40000000) {
        /* GENERIC_WRITE */
        return "wb";
    } else if (access & 0x80000000) {
        /* GENERIC_READ (default) */
        return "rb";
    }
    return "rb"; /* Default to read binary */
}

/* ============================================================================
 * FILE POINTER WRAPPERS - Simplified FILE* operations
 * ============================================================================ */

/**
 * @brief Open a file with automatic path normalization
 * @param filename Input filename (may contain backslashes)
 * @param mode POSIX mode string ("r", "w", "rb", "wb", etc.)
 * @return FILE* pointer on success, NULL on failure
 *
 * Benefits:
 *   - Automatically normalizes paths
 *   - Unified interface across platforms
 *   - Works with standard fopen() internally
 */
static inline FILE* Win32_OpenFile(const char* filename, const char* mode)
{
    if (!filename || !mode) return NULL;
    
    char normalized[4096];
    strncpy(normalized, Win32_NormalizeFilePath(filename), sizeof(normalized) - 1);
    normalized[sizeof(normalized) - 1] = '\0';
    
    return fopen(normalized, mode);
}

/**
 * @brief Close a file pointer (standard fclose wrapper)
 * @param file File pointer to close
 * @return 0 on success, EOF on error
 */
static inline int Win32_CloseFile(FILE* file)
{
    if (!file) return EOF;
    return fclose(file);
}

/**
 * @brief Read bytes from file
 * @param buffer Output buffer
 * @param size Number of bytes to read
 * @param file File pointer
 * @return Number of bytes read, 0 on EOF, negative on error
 */
static inline int Win32_ReadFile(void* buffer, int size, FILE* file)
{
    if (!buffer || !file || size <= 0) return -1;
    
    size_t bytes_read = fread(buffer, 1, size, file);
    return (int)bytes_read;
}

/**
 * @brief Write bytes to file
 * @param buffer Input buffer
 * @param size Number of bytes to write
 * @param file File pointer
 * @return Number of bytes written, negative on error
 */
static inline int Win32_WriteFile(const void* buffer, int size, FILE* file)
{
    if (!buffer || !file || size <= 0) return -1;
    
    size_t bytes_written = fwrite(buffer, 1, size, file);
    return (int)bytes_written;
}

/**
 * @brief Seek to position in file
 * @param file File pointer
 * @param offset Byte offset
 * @param origin Seek origin (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return 0 on success, -1 on error
 */
static inline int Win32_SeekFile(FILE* file, long offset, int origin)
{
    if (!file) return -1;
    return fseek(file, offset, origin);
}

/**
 * @brief Get current file position
 * @param file File pointer
 * @return Current position in bytes, -1 on error
 */
static inline long Win32_TellFile(FILE* file)
{
    if (!file) return -1;
    return ftell(file);
}

/**
 * @brief Get file size in bytes
 * @param file File pointer (must be open)
 * @return File size in bytes, -1 on error
 */
static inline long Win32_GetFileSize(FILE* file)
{
    if (!file) return -1;
    
    long current = ftell(file);
    if (current < 0) return -1;
    
    if (fseek(file, 0, SEEK_END) != 0) return -1;
    long size = ftell(file);
    
    fseek(file, current, SEEK_SET);
    return size;
}

/**
 * @brief Check if file exists
 * @param filename Path to check
 * @return 1 if exists, 0 if not
 */
static inline int Win32_FileExists(const char* filename)
{
    if (!filename) return 0;
    
    char normalized[4096];
    strncpy(normalized, Win32_NormalizeFilePath(filename), sizeof(normalized) - 1);
    normalized[sizeof(normalized) - 1] = '\0';
    
    FILE* f = fopen(normalized, "rb");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

/* ============================================================================
 * VFS INTEGRATION PLACEHOLDERS
 * These will be filled in during Phase 03.1 for .big archive support
 * ============================================================================ */

/**
 * @brief Open file from VFS or physical filesystem
 * @param filename Path to file
 * @param mode File mode ("r", "w", "rb", "wb", etc.)
 * @return FILE* pointer on success, NULL on failure
 *
 * FUTURE: This will attempt to:
 *   1. Check if filename is in a loaded .big archive
 *   2. If found, extract to memory and return FILE* wrapper
 *   3. Otherwise, open from physical filesystem
 */
static inline FILE* Win32_OpenFileVFS(const char* filename, const char* mode)
{
    /* Phase 03.1: Integrate Win32BIGFileSystem here */
    /* For now, just delegate to physical filesystem */
    return Win32_OpenFile(filename, mode);
}

#ifdef __cplusplus
}
#endif

#endif /* WIN32_FILE_API_COMPAT_H_INCLUDED */
