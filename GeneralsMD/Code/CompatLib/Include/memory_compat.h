#pragma once

// GeneralsX @build BenderAI 28/02/2026
// memory_compat.h is Linux-only - provides GlobalAlloc/GlobalFree/GlobalSize and MEMORYSTATUS
// Windows has these natively; on Windows, the real Windows.h provides them

#ifndef _WIN32

#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <malloc.h>
#elif __APPLE__
#include <malloc/malloc.h>
#endif

#define GMEM_FIXED 0

static void *GlobalAlloc(int, size_t size)
{
  return malloc(size);
}

static void GlobalFree(void *ptr)
{
  free(ptr);
}

static size_t GlobalSize(void *ptr)
{
#ifdef __linux__
  return malloc_usable_size(ptr);
#elif defined(__APPLE__)
  return malloc_size(ptr);
#endif
}

// MEMORYSTATUS - Windows memory status structure (for GlobalMemoryStatus)
// GeneralsX @build BenderAI 11/02/2026 Linux stub - memory profiling disabled
// GeneralsX @bugfix BenderAI 13/02/2026 Add conditional guard - DXVK may have defined it
// NOTE: Game uses this for DEBUG_LOG only (dwAvailPageFile, dwAvailPhys, dwAvailVirtual).
#ifndef _MEMORYSTATUS_DEFINED
typedef struct MEMORYSTATUS {
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
} MEMORYSTATUS;
#endif // _MEMORYSTATUS_DEFINED

// Pointer typedef for MEMORYSTATUS (always define)
typedef MEMORYSTATUS *LPMEMORYSTATUS;

// GlobalMemoryStatus stub - no-op on Linux (returns zeros)
// Used for debug logging only - safe to stub
static inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    if (lpBuffer) {
        memset(lpBuffer, 0, sizeof(MEMORYSTATUS));
        lpBuffer->dwLength = sizeof(MEMORYSTATUS);
    }
}

#endif // !_WIN32