/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/******************************************************************************
**
**  FILE: win32_memory_compat.h
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Cross-Platform Memory Allocation & Performance Counter Layer
**
**  DESCRIPTION:
**    Provides memory allocation hooks with statistics tracking and
**    cross-platform compatible performance counter abstraction for
**    timing and profiling operations.
**
**  ARCHITECTURE:
**    Phase 04: Threading & Synchronization Layer
**    - Layer 1: Core Compatibility (this file)
**    - Layer 2: Platform Backend (malloc/free, QueryPerformanceCounter, clock_gettime)
**    - Layer 3: Game-Specific Extensions (GeneralsMD/Code/)
**
******************************************************************************/

#ifndef WIN32_MEMORY_COMPAT_H
#define WIN32_MEMORY_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

/* ============================================================================
 * MEMORY ALLOCATION TRACKING
 * ============================================================================ */

/**
 * Memory allocation statistics structure
 */
typedef struct {
    uint64_t total_allocated;       /**< Total bytes allocated */
    uint64_t total_freed;           /**< Total bytes freed */
    uint64_t current_allocated;     /**< Current bytes in use */
    uint64_t peak_allocated;        /**< Peak bytes ever in use */
    uint32_t allocation_count;      /**< Total allocations made */
    uint32_t free_count;            /**< Total frees made */
    uint32_t active_allocations;    /**< Currently active allocations */
} SDL2_MemoryStats;

/**
 * Initialize memory tracking (call once at startup)
 * 
 * @return              0 on success, -1 on error
 */
int SDL2_InitializeMemoryTracking(void);

/**
 * Allocate memory with tracking
 * 
 * @param size          Number of bytes to allocate
 * @return              Pointer to allocated memory, NULL on error
 *
 * NOTE: Thread-safe allocation
 */
void* SDL2_Malloc(size_t size);

/**
 * Reallocate memory with tracking
 * 
 * @param ptr           Pointer to existing allocation
 * @param new_size      New size in bytes
 * @return              Pointer to reallocated memory, NULL on error
 */
void* SDL2_Realloc(void* ptr, size_t new_size);

/**
 * Free allocated memory
 * 
 * @param ptr           Pointer from SDL2_Malloc/SDL2_Realloc
 */
void SDL2_Free(void* ptr);

/**
 * Allocate aligned memory
 * 
 * @param alignment     Alignment in bytes (must be power of 2)
 * @param size          Number of bytes to allocate
 * @return              Pointer to aligned memory, NULL on error
 */
void* SDL2_MallocAligned(size_t alignment, size_t size);

/**
 * Free aligned memory
 * 
 * @param ptr           Pointer from SDL2_MallocAligned
 */
void SDL2_FreeAligned(void* ptr);

/**
 * Get current memory statistics
 * 
 * @param stats         Pointer to stats structure to fill
 * @return              0 on success, -1 on error
 */
int SDL2_GetMemoryStats(SDL2_MemoryStats* stats);

/**
 * Reset memory statistics
 */
void SDL2_ResetMemoryStats(void);

/**
 * Get current allocation count
 * 
 * @return              Number of active allocations
 */
uint32_t SDL2_GetAllocationCount(void);

/**
 * Get current memory usage
 * 
 * @return              Bytes currently allocated
 */
uint64_t SDL2_GetCurrentMemoryUsage(void);

/**
 * Get peak memory usage
 * 
 * @return              Peak bytes ever allocated
 */
uint64_t SDL2_GetPeakMemoryUsage(void);

/* ============================================================================
 * PERFORMANCE COUNTERS
 * ============================================================================ */

/**
 * Performance counter type for timing operations
 */
#ifdef _WIN32
typedef LARGE_INTEGER SDL2_PerformanceCounter;
#else
typedef struct {
    uint64_t seconds;
    uint64_t nanoseconds;
} SDL2_PerformanceCounter;
#endif

/**
 * Get current performance counter value
 * 
 * @return              Current counter value
 *
 * USAGE:
 *   SDL2_PerformanceCounter start = SDL2_GetPerformanceCounter();
 *   // ... do work ...
 *   SDL2_PerformanceCounter end = SDL2_GetPerformanceCounter();
 *   double ms = SDL2_PerformanceCounterToMilliseconds(start, end);
 */
SDL2_PerformanceCounter SDL2_GetPerformanceCounter(void);

/**
 * Get performance counter frequency (ticks per second)
 * 
 * @return              Frequency in Hz
 */
uint64_t SDL2_GetPerformanceFrequency(void);

/**
 * Convert performance counter difference to milliseconds
 * 
 * @param start         Start counter value
 * @param end           End counter value
 * @return              Elapsed milliseconds
 */
double SDL2_PerformanceCounterToMilliseconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end
);

/**
 * Convert performance counter difference to microseconds
 * 
 * @param start         Start counter value
 * @param end           End counter value
 * @return              Elapsed microseconds
 */
double SDL2_PerformanceCounterToMicroseconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end
);

/**
 * Convert performance counter difference to seconds
 * 
 * @param start         Start counter value
 * @param end           End counter value
 * @return              Elapsed seconds
 */
double SDL2_PerformanceCounterToSeconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end
);

/**
 * High-resolution sleep (more accurate than Sleep/usleep)
 * 
 * @param milliseconds  Time to sleep
 */
void SDL2_HighResolutionSleep(double milliseconds);

/* ============================================================================
 * PROFILING HELPERS
 * ============================================================================ */

/**
 * RAII timer for automatic profiling
 * 
 * USAGE:
 *   {
 *       SDL2_ScopedTimer timer("Operation");  // Prints elapsed time on scope exit
 *       // ... do work ...
 *   }
 */
class SDL2_ScopedTimer
{
    const char* name;
    SDL2_PerformanceCounter start;

public:
    SDL2_ScopedTimer(const char* operation_name);
    ~SDL2_ScopedTimer();

private:
    SDL2_ScopedTimer(const SDL2_ScopedTimer&) = delete;
    SDL2_ScopedTimer& operator=(const SDL2_ScopedTimer&) = delete;
};

#endif /* WIN32_MEMORY_COMPAT_H */
