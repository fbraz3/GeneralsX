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
**  FILE: win32_memory_compat.cpp
**
**  AUTHOR: Ported for GeneralsX
**
**  DATE: November 2025
**
**  PURPOSE: Memory & Performance Counter Implementation
**
******************************************************************************/

#include "win32_memory_compat.h"
#include "win32_thread_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * MEMORY TRACKING STATE
 * ============================================================================ */

static SDL2_CriticalSection g_memory_lock;
static SDL2_MemoryStats g_memory_stats = {0};
static bool g_memory_tracking_initialized = false;

int SDL2_InitializeMemoryTracking(void)
{
    if (g_memory_tracking_initialized) {
        return 0;
    }

    printf("Phase 04: Initializing memory tracking\n");

    g_memory_lock = SDL2_CreateCriticalSection();
    memset(&g_memory_stats, 0, sizeof(SDL2_MemoryStats));
    g_memory_tracking_initialized = true;

    printf("Phase 04: Memory tracking initialized successfully\n");
    return 0;
}

void* SDL2_Malloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    void* ptr = malloc(size);

    if (!ptr) {
        fprintf(stderr, "Phase 04: malloc failed (size: %zu)\n", size);
        return NULL;
    }

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);

        g_memory_stats.total_allocated += size;
        g_memory_stats.current_allocated += size;
        g_memory_stats.allocation_count++;
        g_memory_stats.active_allocations++;

        if (g_memory_stats.current_allocated > g_memory_stats.peak_allocated) {
            g_memory_stats.peak_allocated = g_memory_stats.current_allocated;
        }
    }

    return ptr;
}

void* SDL2_Realloc(void* ptr, size_t new_size)
{
    size_t old_size = 0;

    if (ptr && g_memory_tracking_initialized) {
        /* Note: We don't track individual allocation sizes without malloc wrapping,
           so we can't accurately know the old size here. In production, use a
           memory manager that tracks sizes. */
    }

    void* new_ptr = realloc(ptr, new_size);

    if (!new_ptr) {
        fprintf(stderr, "Phase 04: realloc failed (new_size: %zu)\n", new_size);
        return NULL;
    }

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);

        if (ptr && new_ptr != ptr) {
            /* Realloc moved the block - we count this as new allocation + old free */
            g_memory_stats.total_allocated += new_size;
            g_memory_stats.current_allocated += new_size;
            g_memory_stats.allocation_count++;
        } else if (!ptr) {
            /* New allocation */
            g_memory_stats.total_allocated += new_size;
            g_memory_stats.current_allocated += new_size;
            g_memory_stats.allocation_count++;
            g_memory_stats.active_allocations++;
        } else {
            /* In-place resize (estimate) */
            g_memory_stats.current_allocated += (new_size > old_size ? new_size - old_size : old_size - new_size);
        }

        if (g_memory_stats.current_allocated > g_memory_stats.peak_allocated) {
            g_memory_stats.peak_allocated = g_memory_stats.current_allocated;
        }
    }

    return new_ptr;
}

void SDL2_Free(void* ptr)
{
    if (!ptr) {
        return;
    }

    free(ptr);

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);

        /* Without tracking allocation sizes, we can't update current_allocated accurately */
        g_memory_stats.free_count++;
        if (g_memory_stats.active_allocations > 0) {
            g_memory_stats.active_allocations--;
        }
    }
}

void* SDL2_MallocAligned(size_t alignment, size_t size)
{
    if (size == 0 || alignment == 0) {
        return NULL;
    }

    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        ptr = NULL;
    }

    if (!ptr) {
        fprintf(stderr, "Phase 04: aligned malloc failed (alignment: %zu, size: %zu)\n", alignment, size);
        return NULL;
    }

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);

        g_memory_stats.total_allocated += size;
        g_memory_stats.current_allocated += size;
        g_memory_stats.allocation_count++;
        g_memory_stats.active_allocations++;

        if (g_memory_stats.current_allocated > g_memory_stats.peak_allocated) {
            g_memory_stats.peak_allocated = g_memory_stats.current_allocated;
        }
    }

    return ptr;
}

void SDL2_FreeAligned(void* ptr)
{
    if (!ptr) {
        return;
    }

    free(ptr);

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);

        g_memory_stats.free_count++;
        if (g_memory_stats.active_allocations > 0) {
            g_memory_stats.active_allocations--;
        }
    }
}

int SDL2_GetMemoryStats(SDL2_MemoryStats* stats)
{
    if (!stats) {
        return -1;
    }

    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);
        memcpy(stats, &g_memory_stats, sizeof(SDL2_MemoryStats));
        return 0;
    }

    memset(stats, 0, sizeof(SDL2_MemoryStats));
    return -1;
}

void SDL2_ResetMemoryStats(void)
{
    if (g_memory_tracking_initialized) {
        SDL2_CriticalSectionLock lock(&g_memory_lock);
        memset(&g_memory_stats, 0, sizeof(SDL2_MemoryStats));
        printf("Phase 04: Memory statistics reset\n");
    }
}

uint32_t SDL2_GetAllocationCount(void)
{
    if (!g_memory_tracking_initialized) {
        return 0;
    }

    SDL2_CriticalSectionLock lock(&g_memory_lock);
    return g_memory_stats.active_allocations;
}

uint64_t SDL2_GetCurrentMemoryUsage(void)
{
    if (!g_memory_tracking_initialized) {
        return 0;
    }

    SDL2_CriticalSectionLock lock(&g_memory_lock);
    return g_memory_stats.current_allocated;
}

uint64_t SDL2_GetPeakMemoryUsage(void)
{
    if (!g_memory_tracking_initialized) {
        return 0;
    }

    SDL2_CriticalSectionLock lock(&g_memory_lock);
    return g_memory_stats.peak_allocated;
}

/* ============================================================================
 * PERFORMANCE COUNTERS
 * ============================================================================ */


static uint64_t g_perf_frequency = 1000000000LL;  /* nanoseconds per second */

SDL2_PerformanceCounter SDL2_GetPerformanceCounter(void)
{
    SDL2_PerformanceCounter counter;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    counter.seconds = ts.tv_sec;
    counter.nanoseconds = ts.tv_nsec;

    return counter;
}

uint64_t SDL2_GetPerformanceFrequency(void)
{
    return g_perf_frequency;  /* 1 billion (nanoseconds per second) */
}

double SDL2_PerformanceCounterToMilliseconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end)
{
    uint64_t start_ns = start.seconds * 1000000000ULL + start.nanoseconds;
    uint64_t end_ns = end.seconds * 1000000000ULL + end.nanoseconds;
    uint64_t diff = end_ns - start_ns;

    return (double)diff / 1000000.0;  /* Convert to milliseconds */
}

double SDL2_PerformanceCounterToMicroseconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end)
{
    uint64_t start_ns = start.seconds * 1000000000ULL + start.nanoseconds;
    uint64_t end_ns = end.seconds * 1000000000ULL + end.nanoseconds;
    uint64_t diff = end_ns - start_ns;

    return (double)diff / 1000.0;  /* Convert to microseconds */
}

double SDL2_PerformanceCounterToSeconds(
    SDL2_PerformanceCounter start,
    SDL2_PerformanceCounter end)
{
    uint64_t start_ns = start.seconds * 1000000000ULL + start.nanoseconds;
    uint64_t end_ns = end.seconds * 1000000000ULL + end.nanoseconds;
    uint64_t diff = end_ns - start_ns;

    return (double)diff / 1000000000.0;  /* Convert to seconds */
}

void SDL2_HighResolutionSleep(double milliseconds)
{
    struct timespec ts;
    uint64_t ns = (uint64_t)(milliseconds * 1000000.0);

    ts.tv_sec = ns / 1000000000;
    ts.tv_nsec = ns % 1000000000;

    nanosleep(&ts, NULL);
}


/* ============================================================================
 * SCOPED TIMER
 * ============================================================================ */

SDL2_ScopedTimer::SDL2_ScopedTimer(const char* operation_name)
    : name(operation_name), start(SDL2_GetPerformanceCounter())
{
    printf("Phase 04: Timer started: %s\n", name ? name : "unnamed");
}

SDL2_ScopedTimer::~SDL2_ScopedTimer()
{
    SDL2_PerformanceCounter end = SDL2_GetPerformanceCounter();
    double ms = SDL2_PerformanceCounterToMilliseconds(start, end);

    printf("Phase 04: Timer finished: %s - %.3f ms\n", name ? name : "unnamed", ms);
}
