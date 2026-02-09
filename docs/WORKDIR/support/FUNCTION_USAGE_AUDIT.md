# Function Usage Audit Report
## GeneralsMD & Generals Source Code Analysis

**Date:** February 8, 2026  
**Scope:** GeneralsMD and Generals source directories  
**Focus:** Windows-specific functions and Linux port compatibility risks

---

## Executive Summary

This audit identifies **10 search patterns** across the codebase. **5 are Windows-only** (high risk for Linux), **3 are math utilities** (platform-independent), and **2 patterns were not found**. Total findings: **260+ matches**, with most concentrated in debug/exception handling code.

---

## 1. StackDump() - Stack Trace Dumping

### Overview
Windows-specific debug function for outputting call stack traces via callback mechanism.

### Occurrences
- **Total matches:** 31 (13 unique in GeneralsMD/Generals, 18 in references)
- **Type:** Declaration + Implementation + Empty stub (conditional)

### Files Affected - Primary Codebase

| File | Lines | Usage Context |
|------|-------|---|
| [GeneralsMD/Code/GameEngine/Include/Common/StackDump.h](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L34) | 34, 53 | Function declaration + empty inline stub |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L60) | 60 | Function implementation |
| [Generals/Code/GameEngine/Include/Common/StackDump.h](Generals/Code/GameEngine/Include/Common/StackDump.h#L34) | 34, 53 | Function declaration + empty inline stub |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L60) | 60 | Function implementation |

### Risk Assessment: **HIGH - CRITICAL PATH**
- **Code Type:** Debug-only (guarded by `#if defined(RTS_DEBUG) || defined(IG_DEBUG_STACKTRACE)`)
- **Linux Impact:** **BLOCKING** - Requires complete reimplementation for Linux
- **Dependencies:**
  - Uses `DbgHelpLoader` for Windows symbol resolution
  - Uses Windows APIs directly (no POSIX equivalent in existing code)
  - Currently has empty stub implementations for non-Windows builds
- **Backcompat:** Windows builds unaffected (original code preserved behind `#ifdef`)

### Implementation Notes
- Empty inline stubs already exist for non-debug builds: `__inline void StackDump(...) {}`
- Reference implementations (fighter19) use `libunwind` or `backtrace()` for Linux
- Linux stub is acceptable for MVP Phase 1 (graphics-first)

---

## 2. StackDumpFromAddresses() - Stack Trace from Address Array

### Overview
Converts pre-filled stack address array into human-readable stack trace with symbol information.

### Occurrences
- **Total matches:** 28 (8 unique in GeneralsMD/Generals)
- **Type:** Declaration + Implementation + Empty stub

### Files Affected - Primary Codebase

| File | Lines | Usage Context |
|------|-------|---|
| [GeneralsMD/Code/GameEngine/Include/Common/StackDump.h](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L44) | 44, 59 | Declaration + empty stub |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L439) | 439 | Implementation |
| [Generals/Code/GameEngine/Include/Common/StackDump.h](Generals/Code/GameEngine/Include/Common/StackDump.h#L44) | 44, 59 | Declaration + empty stub |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L439) | 439 | Implementation |

### Risk Assessment: **HIGH - CRITICAL PATH**
- **Code Type:** Debug-only (guarded preprocessor blocks)
- **Linux Impact:** **BLOCKING** - No Linux implementation
- **Called By:**
  - `Debug.cpp` lines 314, 713, 808 (in references)
  - `GameMemory.cpp` lines 311, 1007 (address conversion)
- **Implementation Strategy:**
  - Wraps result of `FillStackAddresses()` with symbol details
  - Should use `addr2line` or `libunwind` on Linux
  - Reference: fighter19 DXVK port has implementation

---

## 3. FillStackAddresses() - Capture Stack Addresses

### Overview
Captures raw return addresses from call stack into array.

### Occurrences
- **Total matches:** 30 (10 unique in GeneralsMD/Generals)
- **Type:** Declaration + Implementation + Empty stub

### Files Affected - Primary Codebase

| File | Lines | Usage Context |
|------|-------|---|
| [GeneralsMD/Code/GameEngine/Include/Common/StackDump.h](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L41) | 41, 56 | Declaration + empty inline stub |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L317) | 317 | Implementation |
| [Generals/Code/GameEngine/Include/Common/StackDump.h](Generals/Code/GameEngine/Include/Common/StackDump.h#L41) | 41, 56 | Declaration + empty inline stub |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L317) | 317 | Implementation |

### Risk Assessment: **HIGH - CRITICAL PATH**
- **Code Type:** Debug-only infrastructure
- **Linux Impact:** **BLOCKING** - Requires system-specific implementation
- **Called By:**
  - `GameMemory.cpp` line 857, 1421 (memory debug tracking)
  - `Debug.cpp` (exception handlers)
- **Implementation Strategy:**
  - Currently uses Windows stack walking via `DbgHelpLoader`
  - Linux alternatives: `backtrace()` (glibc), `libunwind`, or GCC intrinsics
  - Empty stub acceptable for Phase 1

---

## 4. GetFunctionDetails() - Symbol Resolution

### Overview
Converts a single code address to function name, filename, and line number.

### Occurrences
- **Total matches:** 34 (10 unique in GeneralsMD/Generals)
- **Type:** Declaration + Implementation + Empty stub + Usage

### Files Affected - Primary Codebase

| File | Lines | Usage Context |
|------|-------|---|
| [GeneralsMD/Code/GameEngine/Include/Common/StackDump.h](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L46) | 46, 61 | Declaration + empty stub |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L248-468) | 248, 468 | Implementation + usage |
| [GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp](GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp#L269) | 269 | Used in exception handler |
| [Generals/Code/GameEngine/Include/Common/StackDump.h](Generals/Code/GameEngine/Include/Common/StackDump.h#L46) | 46, 61 | Declaration + empty stub |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L248-468) | 248, 468 | Implementation + usage |
| [Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp](Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp#L267) | 267 | Used in exception handler |

### Risk Assessment: **HIGH - CRITICAL PATH**
- **Code Type:** Debug-only symbol resolution
- **Linux Impact:** **BLOCKING** - Depends on `DbgHelpLoader`
- **Usage Pattern:**
  - Called once per stack frame in `StackDumpFromAddresses()`
  - Populates output: function name (256 chars), filename (256 chars), line number, address
- **Linux Strategy:**
  - Use `addr2line`, `dladdr()`, or libunwind for symbol lookup
  - fighter19 reference has implementation

---

## 5. DumpExceptionInfo() - Exception Dumping

### Overview
Windows-specific exception handler that outputs exception context and stack trace.

### Occurrences
- **Total matches:** 28 (10 unique in GeneralsMD/Generals)
- **Type:** Declaration + Implementation (Windows-only) + Empty stub + Usage

### Files Affected - Primary Codebase

| File | Lines | Usage Context |
|------|-------|---|
| [GeneralsMD/Code/GameEngine/Include/Common/StackDump.h](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L49-64) | 49, 64 | Declaration + empty inline stub |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L480-645) | 480, 645 | Implementation + empty stub |
| [GeneralsMD/Code/Main/WinMain.cpp](GeneralsMD/Code/Main/WinMain.cpp#L777) | 777 | **CRITICAL** - Structured Exception Handler (SEH) callback |
| [GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp](GeneralsMD/Code/Tools/WorldBuilder/src/WorldBuilder.cpp#L269) | 269 | Tool exception handler |
| [Generals/Code/GameEngine/Include/Common/StackDump.h](Generals/Code/GameEngine/Include/Common/StackDump.h#L49-64) | 49, 64 | Declaration + empty inline stub |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L480-645) | 480, 645 | Implementation + empty stub |
| [Generals/Code/Main/WinMain.cpp](Generals/Code/Main/WinMain.cpp#L755) | 755 | **CRITICAL** - Structured Exception Handler (SEH) callback |
| [Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp](Generals/Code/Tools/WorldBuilder/src/WorldBuilder.cpp#L267) | 267 | Tool exception handler |

### Risk Assessment: **CRITICAL - RUNTIME**
- **Code Type:** Exception handling (runtime-critical on Windows)
- **Linux Impact:** **BLOCKING** - Depends on EXCEPTION_POINTERS (Windows-only type)
- **Call Sites:**
  - **WinMain.cpp line 777/755**: `DumpExceptionInfo(e_info->ExceptionRecord->ExceptionCode, e_info);`
    - This is in the top-level SEH handler for unhandled exceptions
    - Windows-specific: `__try / __except` block
- **Parameters:**
  - Takes `EXCEPTION_POINTERS*` (platform-specific Windows type)
  - No cross-platform equivalent exists
- **Linux Strategy:**
  - Replace with POSIX signal handler (SIGSEGV, SIGABRT, etc.)
  - Use `sigaction()` instead of SEH
  - Can reuse `DumpExceptionInfo` logic but change parameter type
  - stub is acceptable (silent exception handling)

---

## 6. DbgHelpLoader:: - Debug Help Library Wrapper

### Overview
Static class for dynamic loading/unloading of Windows debug symbol library (dbghelp.dll).

### Occurrences
- **Total matches:** 100+ (reduced scope, ~50 in GeneralsMD/Generals)
- **Type:** Static method calls (thread-safe, singleton)

### Key Methods Called

| Method | Files | Count | Purpose |
|--------|-------|-------|---------|
| `DbgHelpLoader::isLoaded()` | StackDump.cpp | 2 | Check if dll loaded |
| `DbgHelpLoader::isFailed()` | StackDump.cpp | 2 | Check if load failed |
| `DbgHelpLoader::load()` | StackDump.cpp | 2 | Load dbghelp.dll |
| `DbgHelpLoader::unload()` | StackDump.cpp | 4 | Unload dbghelp.dll |
| `DbgHelpLoader::symSetOptions()` | StackDump.cpp | 2 | Configure symbol options |
| `DbgHelpLoader::symInitialize()` | StackDump.cpp | 2 | Init symbol engine |
| `DbgHelpLoader::symLoadModule()` | StackDump.cpp | 2 | Load module symbols |
| `DbgHelpLoader::stackWalk()` | StackDump.cpp | ~6 | Walk stack frames |
| `DbgHelpLoader::symGetSymFromAddr()` | StackDump.cpp | 2 | Get symbol from address |
| `DbgHelpLoader::symGetLineFromAddr()` | StackDump.cpp | 2 | Get source line info |
| `DbgHelpLoader::symFunctionTableAccess` | StackDump.cpp | ~4 | Function table callback |
| `DbgHelpLoader::symGetModuleBase` | StackDump.cpp | ~4 | Get module base address |

### Files Affected - Primary Codebase

| File | Lines | Usage |
|------|-------|-------|
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L34) | 34 | Include |
| [GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L123-407) | 123-407 | 40+ usages within StackDump functions |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L34) | 34 | Include |
| [Generals/Code/GameEngine/Source/Common/System/StackDump.cpp](Generals/Code/GameEngine/Source/Common/System/StackDump.cpp#L123-407) | 123-407 | 40+ usages within StackDump functions |

### Risk Assessment: **HIGH - CRITICAL DEPENDENCY**
- **Code Type:** Windows-only system library (dbghelp.dll)
- **Linux Impact:** **BLOCKING** - No direct Linux equivalent for dbghelp
- **Scope:** All concentrated in `StackDump.cpp` (debug initialization and symbol resolution)
- **Linux Strategy:**
  - Replace with libunwind + addr2line combination
  - Or use backtrace()/backtrace_symbols() for basic stack traces
  - Empty stub acceptable for Phase 1 (defer detailed symbol info)

### Header Location
[Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.h)

**Key Features:**
- Thread-safe singleton pattern (CriticalSection)
- Loads dbghelp.dll dynamically (not linked at compile time)
- Currently 223 lines of Windows-only code
- Already wrapped in `#ifdef _WIN32`

---

## 7. DbgHelpGuard - Debug Help Guard RAII

### Overview
RAII class to temporarily manage dbghelp.dll loading/unloading to prevent conflicts with AMD/ATI drivers.

### Occurrences
- **Total matches:** 10 (4 unique in GeneralsMD/Generals primary code)
- **Type:** Stack-allocated RAII guard object (activate/deactivate)

### Files Affected - Primary Codebase

| File | Lines | Usage |
|------|-------|-------|
| [GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L86) | 86 | Include directive |
| [GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L341) | 341 | `DbgHelpGuard dbgHelpGuard;` instantiation |
| [GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L599) | 599 | `DbgHelpGuard dbgHelpGuard;` instantiation |
| [GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L645) | 645 | `dbgHelpGuard.deactivate();` call |
| [Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L83) | 83 | Include directive |
| [Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L330) | 330 | Instantiation |
| [Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L575) | 575 | Instantiation |
| [Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp](Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp#L592) | 592 | deactivate() call |

### Risk Assessment: **MEDIUM - GRAPHICS LAYER**
- **Code Type:** Graphics initialization code (runtime)
- **Linux Impact:** **NOT BLOCKING** - Can be safely removed
- **Context:** Used in DirectX 8 device creation (dx8wrapper.cpp)
  - Line 341: Inside `GetDeviceCaps()` or similar DirectX init
  - Line 599: Inside graphics mode change sequence
- **Linux Strategy:**
  - DbgHelpGuard becomes no-op class (empty implementation)
  - Or: Remove entirely with conditional compilation
  - No functional impact on graphics output

### Header Location
[Core/Libraries/Source/WWVegas/WWLib/DbgHelpGuard.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpGuard.h) (44 lines)

**Purpose:** Prevents AMD/ATI drivers from loading old dbghelp.dll version during game initialization.

---

## 8. To_D3DMATRIX() - Matrix Type Conversion

### Overview
Inline conversion function from game math matrices (Matrix3D or Matrix4x4) to DirectX D3DMATRIX.

### Occurrences
- **Total matches:** 42 (21 unique in GeneralsMD/Generals)
- **Type:** Inline function calls (no definition found in matches, likely macro)

### Files Affected - Primary Codebase

| File | Count | Lines |
|------|-------|-------|
| [GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h#L1201-1280) | 8 | 1201, 1240, 1245, 1251, 1259, 1269, 1274, 1280 |
| [GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp](GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp#L1354-1623) | 3 | 1354, 1468, 1623 |
| [Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h](Generals/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h#L1184-1224) | 8 | 1184, 1189, 1195, 1203, 1213, 1218, 1224 |
| [Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp](Generals/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/W3DVolumetricShadow.cpp#L1248-1517) | 3 | 1248, 1362, 1517 |

### Risk Assessment: **MEDIUM - GRAPHICS ONLY**
- **Code Type:** Graphics rendering transformation matrix conversion
- **Linux Impact:** **MANAGEABLE** - Needs DXVK equivalent
- **Usage Pattern:**
  - Called during graphics rendering setup
  - Always converts game-internal matrices to DirectX matrices
  - Lines primarily in `dx8wrapper.h` (graphics abstraction)
  - Some in shadow volume rendering code
- **Linux Strategy:**
  - DXVK handles D3DMATRIX internally
  - May need adapter layer to convert to Vulkan matrix format
  - Not code-blocking (just needs graphics layer replacement)

### Pattern Notes
- Appears to be `#define To_D3DMATRIX(m) ...` or inline function
- 21 of 42 matches are in primary GeneralsMD/Generals
- Remaining are in references or duplicated declarations

---

## 9. To_D3DXMATRIX() - D3DX Matrix Conversion

### Occurrences
- **Total matches:** 0 (PATTERN NOT FOUND)

### Conclusion
This function/macro does not exist in the codebase. Only `To_D3DMATRIX()` exists.

---

## 10. Matrix3D / Matrix4x4 - Math Classes

### Overview
Core game math classes for 3D and 4x4 matrix operations. These are **platform-independent** game logic utilities.

### Occurrences
- **Total matches:** 100+ (representative sample)
- **Type:** Class instantiation, method calls, type conversions

### Sample File Distribution

| Category | File | Approx Lines |
|----------|------|------|
| Declarations | Core/Libraries/Source/WWVegas/WW3D2/matrix3d.h | N/A (not searched) |
| Game Logic | GeneralsMD/Code/GameEngine/Source/.../*.cpp | 20+ usages |
| Graphics | GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/.../*.cpp | 30+ usages |
| Camera/View | GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/camera.cpp | 10+ usages |
| Shadows | GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shadow/*.cpp | 15+ usages |
| Game Locomotion | GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Locomotor.cpp | 10+ usages |

### Risk Assessment: **NONE - PLATFORM INDEPENDENT**
- **Code Type:** Math utility classes (no platform dependencies)
- **Linux Impact:** **NO IMPACT** - Fully portable C++ code
- **Implementation:** Pure C++ math operations (no Win32/DirectX/Linux-specific code)
- **Note:** Matrix3D/Matrix4x4 can be directly used in DXVK Vulkan rendering path

### Key Attributes
- `Matrix3D`: 3x4 affine transformation matrix (position + 3D rotation/scale)
- `Matrix4x4`: Full 4x4 matrix (used for projection matrices, 3D transformations)
- Static methods: `Transform_Vector()`, `Rotate_Vector()`, `Inverse()`, etc.
- Used extensively: transforms, projections, shadows, camera math

---

## Database Overview

### Include Files (#include References)

| Header | Location | Primary Uses |
|--------|----------|---|
| `DbgHelpLoader.h` | [Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.h) | StackDump.cpp (debug symbol resolution) |
| `DbgHelpGuard.h` | [Core/Libraries/Source/WWVegas/WWLib/DbgHelpGuard.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpGuard.h) | dx8wrapper.cpp (graphics init) |
| `StackDump.h` | GeneralsMD/Code/GameEngine/Include/Common/ | Exception handlers, memory tracking |

### Conditional Compilation Guards

All debug functions wrapped in:
```cpp
#if defined(RTS_DEBUG) || defined(IG_DEBUG_STACKTRACE)
    // Full implementation
#else
    // Empty inline stubs
#endif
```

---

## Risk Summary by Impact

### 🔴 **BLOCKING (Linux Port)**
1. **StackDump()** - No Linux stack dumping
2. **StackDumpFromAddresses()** - Depends on above
3. **FillStackAddresses()** - Windows stack walk
4. **GetFunctionDetails()** - Symbol resolution
5. **DumpExceptionInfo()** - EXCEPTION_POINTERS (Windows-only type)
6. **DbgHelpLoader::** - dbghelp.dll dependency

### 🟡 **MANAGEABLE (Graphics Layer)**
7. **To_D3DMATRIX()** - Needs DXVK equivalent
8. **DbgHelpGuard** - Can be no-op class

### 🟢 **NO IMPACT (Platform Independent)**
9. **Matrix3D / Matrix4x4** - Pure C++ math
10. **To_D3DXMATRIX()** - Pattern not found

---

## Recommended Implementation Order (Phase 1-2)

### Phase 1: Graphics Only (Skip Debug/Exception Handling)
```
✓ No changes needed for:
  - Matrix3D / Matrix4x4 (use as-is)
  
⚠ Leave stubbed:
  - StackDump() → empty inline (already exists)
  - StackDumpFromAddresses() → empty inline (already exists)
  - FillStackAddresses() → empty inline (already exists)
  - GetFunctionDetails() → empty inline (already exists)
  - DumpExceptionInfo() → empty inline (already exists)
  - DbgHelpGuard → empty RAII class (no-op constructor/destructor)

🔧 Replace for graphics:
  - To_D3DMATRIX() → Add DXVK matrix conversion layer
```

### Phase 2: Audio (No Impact)
```
✓ No changes needed - no graphics/debugging dependencies
```

### Phase 3+: Debug/Exception Handling (Future)
```
When Linux exception handling is complete:
  - Implement POSIX signal handlers (SIGSEGV, SIGABRT)
  - Add libunwind or backtrace() for stack capturing
  - Create Linux equiv of DumpExceptionInfo()
```

---

## Windows Build Compatibility

**All changes are contained in conditional compilation blocks:**

```cpp
#if defined(RTS_DEBUG) || defined(IG_DEBUG_STACKTRACE)
    // Windows implementation
#else
    // Empty inline stubs (Linux will use these)
#endif

#ifdef _WIN32
    // Windows-only: dx8wrapper.cpp DbgHelpGuard usage
#endif
```

**Windows VC6/Win32 builds:** ✅ **ZERO IMPACT** - Original code preserved

---

## Appendix: Complete Function Reference Map

### Debug/Exception Functions (All Windows-only)
| Function | Header | Implementation | Debug-only? | Empty Stub? | Linux Risk |
|----------|--------|---|---|---|---|
| StackDump() | [StackDump.h#L34](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L34) | [StackDump.cpp#L60](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L60) | Yes | Yes | 🔴 HIGH |
| StackDumpFromAddresses() | [StackDump.h#L44](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L44) | [StackDump.cpp#L439](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L439) | Yes | Yes | 🔴 HIGH |
| FillStackAddresses() | [StackDump.h#L41](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L41) | [StackDump.cpp#L317](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L317) | Yes | Yes | 🔴 HIGH |
| GetFunctionDetails() | [StackDump.h#L46](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L46) | [StackDump.cpp#L248](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L248) | Yes | Yes | 🔴 HIGH |
| DumpExceptionInfo() | [StackDump.h#L49](GeneralsMD/Code/GameEngine/Include/Common/StackDump.h#L49) | [StackDump.cpp#L480](GeneralsMD/Code/GameEngine/Source/Common/System/StackDump.cpp#L480) | Yes | Yes | 🔴 CRITICAL |

### Graphics Helper Functions
| Function | Header | Usage | Linux Risk |
|----------|--------|-------|---|
| To_D3DMATRIX() | [dx8wrapper.h#L1201+](GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h#L1201) | Matrix conversion | 🟡 MEDIUM (graphics layer) |

### RAII/Helper Classes
| Class | Header | Usage | Linux Strategy |
|-------|--------|-------|---|
| DbgHelpLoader | [DbgHelpLoader.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpLoader.h) | Symbol resolution | Replace with libunwind |
| DbgHelpGuard | [DbgHelpGuard.h](Core/Libraries/Source/WWVegas/WWLib/DbgHelpGuard.h) | Graphics init guard | Empty RAII no-op |

### Math Classes (Platform-Independent)
| Class | Usages | Scope | Linux Risk |
|-------|--------|-------|---|
| Matrix3D | 100+ | Game logic + graphics | 🟢 NONE |
| Matrix4x4 | 50+ | Graphics projections | 🟢 NONE |

---

## Action Items for Phase 0

- [ ] Document Linux signal handler strategy (replace SEH)
- [ ] Design POSIX exception handling layer
- [ ] Plan libunwind or backtrace() integration
- [ ] Design DXVK matrix conversion layer
- [ ] Document To_D3DMATRIX() macro definition location
- [ ] Create conditional compilation test for Linux builds

