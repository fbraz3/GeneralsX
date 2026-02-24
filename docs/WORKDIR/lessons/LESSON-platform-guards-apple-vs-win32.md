# LESSON: Platform Guards - Use `__APPLE__` not `_WIN32` for macOS Fixes

**Date**: 2026-02-24  
**Phase**: Phase 5 macOS Port  
**Discovered by**: Code audit before continuing Sprint 2+ work  

## The Problem

When porting code for macOS, a common mistake is using `#ifdef _WIN32 / #else` or `#ifndef _WIN32` guards. This is **incorrect** because the `else` branch covers **both Linux AND macOS**, potentially breaking the already-working Linux build.

Example of what **NOT to do**:
```cpp
// WRONG - #else covers Linux too, bypasses Linux's real malloc.h
#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>   // This runs on Linux too!
#endif
```

## The Correct Pattern

For macOS-specific workarounds, **always** use `#ifdef __APPLE__`:

```cpp
// CORRECT - explicitly targets only macOS
#ifdef __APPLE__
#include <stdlib.h>  // macOS: malloc.h does not exist
#else
#include <malloc.h>  // Linux/Windows: have native malloc.h
#endif
```

## Decision Tree for Platform Guards

```
Is the fix needed on macOS only?
  YES → #ifdef __APPLE__

Is the fix needed on Linux only?
  YES → #ifdef __linux__

Is the fix needed on all POSIX (Linux + macOS, not Windows)?
  YES → #ifndef _WIN32  (safe, both POSIX platforms)

Is it macOS + Linux (both need it differently)?
  YES → #ifdef __APPLE__ / #elif defined(__linux__) / #else
```

## Real Examples from This Codebase

### malloc.h - macOS has no `malloc.h`
```cpp
// macOS only needs the workaround; Linux has real malloc.h
#ifdef __APPLE__
#include <stdlib.h>   // macOS: malloc functions in stdlib.h
#else
#include <malloc.h>   // Linux/Windows: native malloc.h (has alloca, memalign, etc.)
#endif
```

### Timer APIs - Each platform has its own
```cpp
#ifdef __APPLE__
  // macOS: mach_absolute_time()
  uint64_t elapsed = mach_absolute_time();
  ...
#elif defined(__linux__)
  // Linux: CLOCK_BOOTTIME (uptime clock, Linux-only)
  clock_gettime(CLOCK_BOOTTIME, &ts);
  ...
#else
  // Other POSIX (BSD, etc.): CLOCK_MONOTONIC
  clock_gettime(CLOCK_MONOTONIC, &ts);
  ...
#endif
```

### Executable path - each platform has its own API
```cpp
#ifdef __linux__
  ssize_t count = readlink("/proc/self/exe", buffer, size);
#elif defined(__APPLE__)
  uint32_t bufsize = (uint32_t)size;
  int ret = _NSGetExecutablePath(buffer, &bufsize);
#else
  return false; // unsupported platform
#endif
```

## Why This Matters

**Linux build was already working.** Applying `#ifndef _WIN32` / `#else` to code that previously used `#include <malloc.h>` would:
1. Replace Linux's real `malloc.h` with a stub → missing `alloca()`, `memalign()`, `_alloca()` etc.
2. Potentially cause subtle runtime failures even if it compiles
3. Break the established regression baseline for Linux

## Files Fixed (Sprint 2 Audit, 2026-02-24)

| File | Wrong Guard | Correct Guard |
|---|---|---|
| `FastAllocator.h` | `#ifdef _WIN32` / `#else stdlib.h` | `#ifdef __APPLE__` / `#else malloc.h` |
| `ini.cpp` | `#ifdef _WIN32` / `#else stdlib.h` | `#ifdef __APPLE__` / `#else malloc.h` |
| `windows_compat.h` | `#ifndef _WIN32` malloc stub | `#ifdef __APPLE__` only |

## Summary

> **Always use `#ifdef __APPLE__` for macOS-specific code.**  
> Reserve `#ifndef _WIN32` for code that truly applies to ALL non-Windows platforms equally.
