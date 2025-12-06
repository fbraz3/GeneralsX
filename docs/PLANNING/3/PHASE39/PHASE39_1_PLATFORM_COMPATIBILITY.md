# Phase 39.1: Platform Compatibility & Windows API Abstraction

**Status**: IN PROGRESS  
**Date**: November 14, 2025  
**Target**: Resolve all Windows-specific API dependencies to enable macOS ARM64 compilation

## Overview

Phase 39.1 extends Phase 39 (Vulkan Backend) by addressing platform compatibility issues that emerge during actual build execution. While Phase 39 established the Vulkan graphics backend infrastructure, Phase 39.1 focuses on systematically identifying and abstracting Windows-only APIs that prevent cross-platform compilation.

**Key Philosophy**: Apply "Fail Fast" approach - identify root causes rather than band-aid fixes. Use `#ifdef _WIN32` judiciously only when essential; prefer abstraction through compatibility layers.

## Changes Made This Session

### 1. DirectX Math Compatibility Layer

**File**: `Core/Libraries/Source/WWVegas/WW3D2/d3dx8math.h` (NEW)

Created a thin wrapper that redirects to the Vulkan-compatible math implementation:

```cpp
// d3dx8math.h - DirectX 8 Math compatibility wrapper
#include "d3dx8_vulkan_math_compat.h"
```

**Why**: Centralized header inclusion for DirectX 8 math functions (D3DXMATRIX, D3DXVECTOR*, etc.)

### 2. Macro Trailing Comma Fix (Variadic Arguments)

**Files Modified**:
- `GeneralsMD/Code/GameEngine/Include/GameClient/GameText.h`
- `Generals/Code/GameEngine/Include/GameClient/GameText.h`

**Issue**: Variadic macros with empty `__VA_ARGS__` leave trailing commas:

```cpp
// Before: #define FETCH_OR_SUBSTITUTE_FORMAT(...) fetchOrSubstituteFormat(..., __VA_ARGS__)
// Called with 2 args: fetchOrSubstituteFormat(..., ); // <- trailing comma error!

// After: #define FETCH_OR_SUBSTITUTE_FORMAT(...) fetchOrSubstituteFormat(..., ##__VA_ARGS__)
// Called with 2 args: fetchOrSubstituteFormat(...); // <- comma removed by ##
```

**Solution**: Use `##__VA_ARGS__` GCC/Clang extension that removes preceding comma when VA_ARGS is empty

### 3. C++20 Standard Requirement

**File Modified**: `cmake/compilers.cmake`

Added explicit C++ standard configuration:

```cmake
set(CMAKE_CXX_STANDARD 20)  # C++20 for variadic macros and modern features
```

**Why**: Precompiled headers were using wrong C++ version (< C++11), causing macro expansion issues

### 4. Windows File API Abstraction

**Files Modified**:
- `GeneralsMD/Code/GameEngine/Source/GameClient/System/Image.cpp`
- `Generals/Code/GameEngine/Source/GameClient/System/Image.cpp`

**Issue**: `FindFirstFile` and `WIN32_FIND_DATA` are Windows-specific filesystem APIs

**Solution**: Wrapped user-custom-images loading logic with `#ifdef _WIN32`:

```cpp
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    // ... user custom MappedImages loading ...
#endif
```

**Rationale**: Custom image loading is optional feature; on macOS simply skip this block and use default MappedImages

### 5. 64-bit Pointer Casting Fixes

**Files Modified**:
- `GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIStates.cpp` (line 1237)
- `Generals/Code/GameEngine/Source/GameLogic/AI/AIStates.cpp` (line 1232)
- `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Collide/CrateCollide/SabotageInternetCenterCrateCollide.cpp` (lines 121, 132)

**Issue**: Casting `void*` directly to `UnsignedInt` (32-bit) loses information on 64-bit ARM64

**Solution**: Use `uintptr_t` intermediate cast:

```cpp
// Before: (AbleToAttackType)(UnsignedInt)userData;
// After:  (AbleToAttackType)(uintptr_t)userData;
```

### 6. Windows CRT Function Stubs

**File Modified**: `Dependencies/Utility/Compat/msvc_types_compat.h`

Added cross-platform implementations:

#### itoa() - Integer to ASCII conversion

```cpp
inline char* itoa(int value, char* buffer, int radix) {
    // Uses snprintf for base 10 conversion
}
```

#### isnan() and isfinite() - Floating-point classification

```cpp
#define _isnan(x) std::isnan(x)
#define _finite(x) std::isfinite(x)
```

#### std::min() and std::max() - Integer comparison

```cpp
#define __min(a, b) std::min((a), (b))
#define __max(a, b) std::max((a), (b))
```

## Build Status

### Current Errors (After Changes)
1. ✅ FIXED: `d3dx8math.h` file not found → Wrapper created
2. ✅ FIXED: FETCH_OR_SUBSTITUTE_FORMAT macro error → Trailing comma removed with ##__VA_ARGS__
3. ✅ FIXED: GetLocalTime, HKL, GetKeyboardLayout undefined → Stubs in msvc_types_compat.h
4. ✅ FIXED: 64-bit pointer casting → uintptr_t intermediate cast
5. ✅ FIXED: itoa undefined → Stub implementation
6. ✅ FIXED: _isnan undefined → Macro wrapper
7. ✅ FIXED: __min undefined → Macro wrapper
8. ⏳ IN PROGRESS: PartitionManager.cpp static declaration conflicts

### Remaining Issues
- Static declaration reordering in PartitionManager.cpp (multiple overloaded hLine* functions)
- Private member access issues in PartitionManager.cpp lambda functions
- Additional Windows API dependencies likely in remaining ~15% of codebase

## Approach Summary

### Fail-Fast Principle Applied
1. **Identify Root Cause**: Not just fix compiler errors, understand WHY they fail
   - `_isnan` is Windows CRT → Map to standard C99 `std::isnan`
   - `FindFirstFile` is Windows API → Condition compilation with `#ifdef _WIN32`
   - Pointer casting precision loss on 64-bit → Use `uintptr_t` intermediate

2. **Targeted Solutions**: Apply fixes at appropriate abstraction level
   - Global API stubs in compatibility layer (msvc_types_compat.h)
   - Windows-only code with platform guards where feature is optional
   - Pointer-safe casting wherever void* is used for type punning

3. **Avoid Band-aids**: Don't just add `#ifdef _WIN32` everywhere
   - Evaluate if functionality is truly Windows-only
   - Consider if feature is optional or core
   - Use compatibility layer for truly cross-platform functions

## Next Steps

1. **Resolve PartitionManager.cpp issues**
   - Investigate static function declaration conflicts
   - Fix lambda function's private member access
   - May require refactoring of test/helper function declarations

2. **Systematic API Audit**
   - Scan remaining codebase for Windows-specific includes
   - Identify all undeclared functions/types
   - Add stubs incrementally as compilation reveals gaps

3. **Validation**
   - Compile both `GeneralsXZH` (Zero Hour) and `GeneralsX` (base game)
   - Verify binary works with Vulkan backend on macOS
   - Test with both Metal and OpenGL fallback paths

## References

- **Previous Phases**: See `docs/PHASE39/` for Vulkan backend implementation
- **Compatibility Patterns**: `Dependencies/Utility/Compat/msvc_types_compat.h` for established patterns
- **Project Instructions**: `.github/copilot-instructions.md` for platform-specific build guidelines
