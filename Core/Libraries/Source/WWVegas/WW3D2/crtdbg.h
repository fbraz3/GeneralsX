/**
 * C Runtime Debug compatibility header for non-Windows platforms (macOS, Linux)
 * 
 * This header provides compatibility for _crtdbg.h on non-Windows platforms.
 * The C Runtime Debug library is Windows-specific and not available on macOS/Linux.
 * 
 * Purpose: Allow cross-platform compilation when crtdbg.h is included
 */

#ifndef CRTDBG_H_COMPAT
#define CRTDBG_H_COMPAT

#ifdef _WIN32
    // On Windows, use the real CRT debug header
    #include <crtdbg.h>
#else
    // On non-Windows platforms, provide minimal stubs for CRT debug functions
    
    // Memory allocation debug macros - just redirect to normal functions on non-Windows
    #define _malloc_dbg(size, type, file, line) malloc(size)
    #define _calloc_dbg(count, size, type, file, line) calloc(count, size)
    #define _realloc_dbg(ptr, size, type, file, line) realloc(ptr, size)
    #define _free_dbg(ptr, type) free(ptr)
    
    // Debug report function - no-op on non-Windows
    #define _CrtCheckMemory() 1
    #define _CrtSetDbgFlag(flags) (flags)
    #define _CrtSetBreakAlloc(addr) (addr)
    #define _CrtSetReportMode(type, mode) (mode)
    #define _CrtSetReportFile(type, file) (file)
    
    // Debug flags - no-op on non-Windows
    #define _CRTDBG_ALLOC_MEM_DF 0x01
    #define _CRTDBG_DELAY_FREE_MEM_DF 0x02
    #define _CRTDBG_CHECK_ALWAYS_DF 0x04
    #define _CRTDBG_RESERVED_DF 0x08
    #define _CRTDBG_CHECK_CRT_DF 0x10
    #define _CRTDBG_LEAK_CHECK_DF 0x20
    #define _CRTDBG_MAP_ALLOC 0
    
    // Report types
    #define _CRT_WARN 0
    #define _CRT_ERROR 1
    #define _CRT_ASSERT 2
    
    // Report file destinations
    #define _CRTDBG_FILE_STDOUT 0
    #define _CRTDBG_FILE_STDERR 1
    #define _CRTDBG_REPORT_FILE 2
    
#endif // _WIN32

#endif // CRTDBG_H_COMPAT
