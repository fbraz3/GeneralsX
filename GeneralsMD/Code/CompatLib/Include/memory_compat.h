#pragma once

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
#else
  #error "GlobalSize not implemented for this platform"
#endif
}

// MEMORYSTATUS - Windows memory status structure (for GlobalMemoryStatus)
// TheSuperHackers @build BenderAI 11/02/2026 Linux stub - memory profiling disabled
// TheSuperHackers @bugfix BenderAI 12/02/2026 Define unconditionally - guard pre-set in windows_compat.h
// NOTE: Game uses this for DEBUG_LOG only (dwAvailPageFile, dwAvailPhys, dwAvailVirtual).
// DXVK's windows_base.h has minimal version (2 fields). We provide full 8-field version.
// The _MEMORYSTATUS_DEFINED guard is pre-defined in windows_compat.h BEFORE windows_base.h,
// so DXVK skips its definition and only ours exists. Define unconditionally here.
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

// Pointer typedef for MEMORYSTATUS
typedef MEMORYSTATUS *LPMEMORYSTATUS;

// GlobalMemoryStatus stub - no-op on Linux (returns zeros)
// Used for debug logging only - safe to stub
static inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    if (lpBuffer) {
        memset(lpBuffer, 0, sizeof(MEMORYSTATUS));
        lpBuffer->dwLength = sizeof(MEMORYSTATUS);
    }
}