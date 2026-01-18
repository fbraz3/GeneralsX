# Integer Type Definitions - Code Locations

## Complete File Index with Line Numbers

### 1. Primary Integer Type Definition Files

#### `Dependencies/Utility/Utility/stdint_adapter.h`
- **Purpose**: Master C99 integer type adapter
- **Path**: `Dependencies/Utility/Utility/stdint_adapter.h`
- **Type Definitions** (VC6 path):
  - Line 26: `typedef signed char int8_t;`
  - Line 27: `typedef unsigned char uint8_t;`
  - Line 28: `typedef short int16_t;`
  - Line 29: `typedef unsigned short uint16_t;`
  - Line 30: `typedef int int32_t;`
  - Line 31: `typedef unsigned int uint32_t;`
  - Line 32: `typedef __int64 int64_t;`
  - Line 33: `typedef unsigned __int64 uint64_t;`
  - Line 35-40: `intptr_t` and `uintptr_t` (64-bit aware)
  - Lines 41-106: Extended types (`int_least8_t`, `int_fast64_t`, etc.)
  - Lines 108-164: Limit macros (`INT8_MIN`, `INT32_MAX`, etc.)
- **Guard Condition**: Lines 25
  - `#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)`
- **Modern Compiler Path**: Line 173
  - `#include <cstdint>`

#### `Dependencies/Utility/Compat/stdint.h`
- **Purpose**: Compatibility shim to provide `<stdint.h>` compatibility
- **Path**: `Dependencies/Utility/Compat/stdint.h`
- **Content**:
  - Line 2-4: Comment explaining purpose
  - Line 8-11: Standard header guards
  - Line 13-14: Include `../Utility/stdint_adapter.h`
- **Note**: This allows code to `#include <stdint.h>` on all platforms

#### `Core/Libraries/Include/Lib/BaseTypeCore.h`
- **Purpose**: Legacy type wrappers built on standard integer types
- **Path**: `Core/Libraries/Include/Lib/BaseTypeCore.h`
- **Includes**:
  - Line 31: `#include <math.h>`
  - Line 32: `#include <string.h>`
  - Line 33: `#include <Utility/compat.h>`
  - Line 34: `#include <Utility/CppMacros.h>`
  - Line 35: `#include <Utility/stdint_adapter.h>` ⭐ **CRITICAL**
- **Type Definitions** (Lines 121-131):
  - Line 121: `typedef int32_t Int;`
  - Line 122: `typedef uint32_t UnsignedInt;`
  - Line 123: `typedef uint16_t UnsignedShort;`
  - Line 124: `typedef int16_t Short;`
  - Line 126: `typedef unsigned char UnsignedByte;`
  - Line 127: `typedef char Byte;`
  - Line 128: `typedef char Char;`
  - Line 129: `typedef bool Bool;`
  - Line 130: `typedef int64_t Int64;`
  - Line 131: `typedef uint64_t UnsignedInt64;`

### 2. Extension/Wrapper Files

#### `Generals/Code/Libraries/Include/Lib/BaseType.h`
- **Purpose**: Extends BaseTypeCore with utility functions
- **Path**: `Generals/Code/Libraries/Include/Lib/BaseType.h`
- **Includes**:
  - Line 32: `#include "Lib/BaseTypeCore.h"`
  - Line 33: `#include "Lib/trig.h"`
- **Note**: No additional type definitions, only utilities

#### `GeneralsMD/Code/Libraries/Include/Lib/BaseType.h`
- **Purpose**: Extends BaseTypeCore with utility functions (Zero Hour variant)
- **Path**: `GeneralsMD/Code/Libraries/Include/Lib/BaseType.h`
- **Includes**:
  - Line 32: `#include "Lib/BaseTypeCore.h"`
  - Line 33: `#include "Lib/trig.h"`
- **Note**: No additional type definitions, only utilities

### 3. Compatibility Shims

#### `Dependencies/Utility/Compat/immintrin.h`
- **Purpose**: Intel intrinsics stubs for VC6
- **Path**: `Dependencies/Utility/Compat/immintrin.h`
- **Content**: Macro stubs only, NO type definitions
- **Note**: For SIMD operations compatibility

#### `Dependencies/Utility/Utility/intrin_compat.h`
- **Purpose**: Intrinsics and assembly compatibility
- **Path**: `Dependencies/Utility/Utility/intrin_compat.h`
- **Type-relevant includes**:
  - Line 72-74: Conditional `#include <cstdint>` for modern compilers
  ```cpp
  #if !(defined(_MSC_VER) && _MSC_VER < 1300)
  #include <cstdint>
  ```
- **Note**: VC6 path has inline assembly stubs instead

#### `Dependencies/Utility/Utility/compat.h`
- **Purpose**: Cross-platform macro definitions
- **Path**: `Dependencies/Utility/Utility/compat.h`
- **Conditional includes** (non-Windows only):
  - Line 22: `#include <cstddef>` (for `size_t`)
  - Line 24: `#include <cctype>` (for `isdigit`)
- **Includes other compat headers**:
  - Line 65: `#include "mem_compat.h"`
  - Line 66: `#include "string_compat.h"`
  - Line 67: `#include "tchar_compat.h"`
  - Line 68: `#include "wchar_compat.h"`
  - Line 69: `#include "time_compat.h"`
  - Line 70: `#include "thread_compat.h"`

### 4. Reference Implementation Files (Not in main build)

#### `references/fighter19-dxvk-port/GeneralsMD/Code/Libraries/Include/Lib/BaseType.h`
- **Includes**: `#include <stdint.h>` (line 125)
- **Purpose**: Reference for Linux port
- **Note**: Uses standard C99 header directly

#### `references/jmarshall-win64-modern/Code/Libraries/Include/Lib/BaseType.h`
- **Includes**: `#include <stdint.h>`
- **Purpose**: Reference for Windows 64-bit modernization
- **Note**: Uses standard C99 header directly

### 5. Vulkan and Graphics Files

#### `Core/GameEngineDevice/Include/GraphicsDevice/vulkan_stubs.h`
- **Path**: `Core/GameEngineDevice/Include/GraphicsDevice/vulkan_stubs.h`
- **Include**: Line 33 - `#include <stdint.h>`
- **Purpose**: Vulkan stubs for graphics initialization
- **Note**: Uses standard C header for Vulkan compatibility

## Type Definition Dependency Graph

```
User Code (Generals/GeneralsMD modules)
    │
    ├─→ #include <Lib/BaseType.h>
    │       │
    │       └─→ #include <Lib/BaseTypeCore.h> (Line 32)
    │               │
    │               └─→ #include <Utility/stdint_adapter.h> (Line 35)
    │                       │
    │                       ├─→ VC6 Path (Line 25 condition TRUE)
    │                       │   └─→ Manual typedefs (Lines 26-165)
    │                       │
    │                       └─→ Modern Path (Line 173)
    │                           └─→ #include <cstdint>
    │
    └─→ Other includes
            └─→ #include <Utility/compat.h>
                    └─→ Various platform-specific headers
```

## Compilation Paths

### Path A: VC6 Compiler (Visual C++ 6 or older MSVC)
```
Condition: defined(_MSC_VER) && _MSC_VER < 1300
Location: stdint_adapter.h line 25

Result:
├─ Uses manual typedefs (lines 26-106)
├─ Uses __int64 for 64-bit integers
├─ NO #include <cstdint>
└─ All types defined via typedef statements
```

### Path B: Modern Compiler (MSVC 2022, Clang, GCC)
```
Condition: NOT (defined(_MSC_VER) && _MSC_VER < 1300)
Location: stdint_adapter.h line 173

Result:
├─ #include <cstdint>
├─ All types from C++ standard library
├─ No manual typedefs needed
└─ No conflicts with system headers
```

### Path C: STLPORT Build
```
Condition: defined(USING_STLPORT)
Location: stdint_adapter.h line 25 (alternative condition)

Status: OBSOLETE - likely not used in current builds
```

## File Modification Restrictions

### ⛔ DO NOT MODIFY
1. `Dependencies/Utility/Utility/stdint_adapter.h` - Core system file
2. `Dependencies/Utility/Compat/stdint.h` - System compatibility shim
3. `Core/Libraries/Include/Lib/BaseTypeCore.h` - Base type definitions

### ✓ SAFE TO MODIFY (for refactoring)
1. `Generals/Code/Libraries/Include/Lib/BaseType.h` - Extension only
2. `GeneralsMD/Code/Libraries/Include/Lib/BaseType.h` - Extension only

### Reference Files (informational only)
1. `references/fighter19-dxvk-port/**` - Not part of main build
2. `references/jmarshall-win64-modern/**` - Not part of main build

## Quick Navigation Links

| Need | File | Line Range |
|------|------|------------|
| VC6 type defs | `stdint_adapter.h` | 26-106 |
| Modern type defs | `stdint_adapter.h` | 173 |
| Legacy wrappers | `BaseTypeCore.h` | 121-131 |
| Limit macros | `stdint_adapter.h` | 108-164 |
| Guard condition | `stdint_adapter.h` | 25 |
| All includes | `BaseTypeCore.h` | 31-35 |

## Cross-Reference: Where Types Are Used

### Types Defined in `stdint_adapter.h`
- Used by: `BaseTypeCore.h` (to create legacy wrappers)
- Used by: Game code through type includes
- Used by: Graphics layer (`vulkan_stubs.h`)
- Used by: Intrinsics layer (`intrin_compat.h`)

### Types Defined in `BaseTypeCore.h`
- Used by: `BaseType.h` (extended)
- Used by: All game code via BaseType includes
- Used by: Legacy codebase expecting `Int`, `UnsignedInt`, etc.

### Types from `<cstdint>`
- Used by: Modern compiler paths
- Used by: Reference implementations
- Used by: OpenAL library
- Used by: Graphics subsystems

## No Circular Includes ✅

- `stdint_adapter.h` → includes nothing from our codebase
- `BaseTypeCore.h` → includes `stdint_adapter.h` (one-way)
- `BaseType.h` → includes `BaseTypeCore.h` (one-way)
- All includes are properly guarded with `#pragma once`

**Verification Status**: COMPLETE - No circular dependencies found
