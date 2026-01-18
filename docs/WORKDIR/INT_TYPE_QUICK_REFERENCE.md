# Integer Type Definitions - Quick Reference

## Master Type Definition Table

| File | Location | Type | Defined As | Condition | Purpose |
|------|----------|------|-----------|-----------|---------|
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `int8_t` | `signed char` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `uint8_t` | `unsigned char` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `int16_t` | `short` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `uint16_t` | `unsigned short` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `int32_t` | `int` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `uint32_t` | `unsigned int` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `int64_t` | `__int64` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `uint64_t` | `unsigned __int64` | VC6 path | C99 compat |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `intptr_t` | `int` /(or `__int64` on 64-bit) | VC6 path | Pointer size |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | `uintptr_t` | `unsigned int` /(or `unsigned __int64` on 64-bit) | VC6 path | Pointer size |
| `stdint_adapter.h` | `Dependencies/Utility/Utility/` | All above | `<cstdint>` | Modern path | Standard C++ |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `Int` | `int32_t` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `UnsignedInt` | `uint32_t` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `Short` | `int16_t` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `UnsignedShort` | `uint16_t` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `Byte` | `char` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `UnsignedByte` | `unsigned char` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `Int64` | `int64_t` | Always | Legacy wrapper |
| `BaseTypeCore.h` | `Core/Libraries/Include/Lib/` | `UnsignedInt64` | `uint64_t` | Always | Legacy wrapper |

## Critical Inclusion Chain

```
Game Code (.cpp files)
   ↓
#include "Lib/BaseType.h"
   ↓
Generals/Code/Libraries/Include/Lib/BaseType.h
GeneralsMD/Code/Libraries/Include/Lib/BaseType.h
   ↓
#include "Lib/BaseTypeCore.h"
   ↓
Core/Libraries/Include/Lib/BaseTypeCore.h (Lines 35)
   ↓
#include <Utility/stdint_adapter.h>
   ↓
Dependencies/Utility/Utility/stdint_adapter.h (Line 25 check)
   ├─ IF: USING_STLPORT OR _MSC_VER < 1300
   │  └─ Manual typedefs (Lines 26-65)
   │     ├─ int8_t  = signed char
   │     ├─ uint8_t = unsigned char
   │     ├─ int16_t = short
   │     ├─ uint16_t = unsigned short
   │     ├─ int32_t = int
   │     ├─ uint32_t = unsigned int
   │     ├─ int64_t = __int64
   │     └─ uint64_t = unsigned __int64
   │
   └─ ELSE: (Modern Compiler)
      └─ #include <cstdint> (Line 173)
         └─ Standard C99 types from compiler
```

## Header Include Locations

### Standard `#include` statements found:

#### `stdint.h` (C-style header) - 17 occurrences:
- `Dependencies/Utility/Compat/stdint.h:2` - ✅ OUR shim
- `Core/GameEngineDevice/Include/GraphicsDevice/vulkan_stubs.h:33`
- `references/fighter19-dxvk-port/` - 7 files
- `references/jmarshall-win64-modern/` - 7 files

#### `cstdint` (C++ header) - 27 occurrences:
- `Dependencies/Utility/Utility/stdint_adapter.h:173` - ✅ OUR modern path
- `Dependencies/Utility/Utility/intrin_compat.h:72` - ✅ OUR modern path
- `references/fighter19-dxvk-port/GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/intrin_compat.h:3`
- `references/jmarshall-win64-modern/Code/Libraries/OpenAL/` - 20+ files

## Conflict Assessment Matrix

| Scenario | Risk | Status | Action |
|----------|------|--------|--------|
| VC6 + VC6 types | ❌ None | ✅ OK | No action needed |
| Modern + Standard types | ❌ None | ✅ OK | No action needed |
| Mixed types in same file | ⚠️ Low | ✅ OK | Type consistency review recommended |
| Multiple includes of stdint | ❌ None | ✅ OK | Header guards prevent re-inclusion |
| Legacy wrappers + standard types | ⚠️ Low | ✅ OK | Code style consistency issue, not functional |

## Files Defining or Using Integer Types

### Primary Definition Files (DO NOT MODIFY)
1. ✅ `Dependencies/Utility/Utility/stdint_adapter.h` - Core definition
2. ✅ `Dependencies/Utility/Compat/stdint.h` - Compatibility shim
3. ✅ `Core/Libraries/Include/Lib/BaseTypeCore.h` - Legacy wrappers

### Secondary Files (Reference/External)
4. `Generals/Code/Libraries/Include/Lib/BaseType.h` - Extension layer
5. `GeneralsMD/Code/Libraries/Include/Lib/BaseType.h` - Extension layer
6. `Core/GameEngineDevice/Include/GraphicsDevice/vulkan_stubs.h` - Vulkan types
7. `references/*/` - Reference implementations (not in main build)

## No Conflicts Found ✅

**Summary**: 
- Zero (0) conflicting type redefinitions
- Zero (0) header guard violations
- Zero (0) multiple inclusion issues
- 100% conditional compilation integrity verified

**Confidence Level**: HIGH - All type definitions use proper guards and conditional paths
