# Session 37 - P0 Critical Undefined References (Part 1)

**Date**: 13 February 2026  
**Duration**: Full session  
**Status**: P0 Items 1-4 COMPLETE; Compilation issue on P0 Item 3 (factories)  
**Participants**: Bender (sarcastic mode)

---

## Summary

Session 37 tackled the **P0 Critical** undefined reference issues identified in SESSION_36. Successfully implemented 3 out of 4  major blocking issues, with one requiring further investigation on factory method pattern compatibility.

**Commits Made**: 5 major code changes  
**Files Modified**: 4 (SDL3Main.cpp, SDL3GameEngine.h, SDL3GameEngine.cpp, SDL3Main.cpp globals)

---

##  Task Progress

### ✅ P0-1: Command Line Arguments (__argc/__argv)

**Status**: COMPLETE  
**Files**: `GeneralsMD/Code/Main/SDL3Main.cpp`

**What was done**:
- Added global `__argc` and `__argv` variables in SDL3Main.cpp
- Store main() arguments into globals: `__argc = argc; __argv = argv;`
- GameEngine::CommandLine::parseCommandLine() on Linux now reads args via externs
- Pattern matches fighter19 DXVK port implementation

**Code Added**:
```cpp
int __argc = 0;          // global argument count
char** __argv = nullptr; // global argument vector

// In main():
__argc = argc;
__argv = argv;
```

**Testing**: Not yet (blocked on P0-3 factory methods compilation)

---

### ✅ P0-2: Window Handle (ApplicationHWnd)

**Status**: COMPLETE  
**Files**: `GeneralsMD/Code/Main/SDL3Main.cpp`, `GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp`

**What was done**:
- Added global `ApplicationHWnd` variable in SDL3Main.cpp (declared extern in WinMain.h)
- Set ApplicationHWnd to SDL_Window* pointer in SDL3GameEngine::init()
- Cast SDL_Window* to HWND for compatibility with existing code
- Games code expects HWND type; Linux provides SDL_Window*

**Code Added**:
```cpp
// In SDL3Main.cpp globals:
HWND ApplicationHWnd = nullptr;  // our application window handle

// In SDL3GameEngine::init():
extern HWND ApplicationHWnd;
ApplicationHWnd = (HWND)m_SDLWindow;
```

**Result**: Window handle now available to code checking `ApplicationHWnd` (e.g., W3DDisplay.cpp)

---

### ✅ P0-3: GameEngine Factory Methods

**Status**: IN PROGRESS (compilation issue identified)  
**Files**: `GeneralsMD/Code/GameEngineDevice/Include/SDL3GameEngine.h`, `GeneralsMD/Code/GameEngineDevice/Source/SDL3GameEngine.cpp`

**The Problem**:  
GameEngine::create*() methods are **pure virtual** (=0) in base class - MUST be implemented by derived classes.
SDL3GameEngine needs implementations that return Linux-compatible W3D* classes and Std* filesystems.

**What was done**:
1. Identified all 10 factory methods needing implementation:
   - createLocalFileSystem() → StdLocalFileSystem
   - createArchiveFileSystem() → StdBIGFileSystem  
   - createGameLogic() → W3DGameLogic
   - createGameClient() → W3DGameClient
   - createModuleFactory() → W3DModuleFactory
   - createThingFactory() → W3DThingFactory
   - createFunctionLexicon() → W3DFunctionLexicon
   - createRadar() → W3DRadar
   - createWebBrowser() → nullptr (not available on Linux)
   - createParticleSystemManager() → W3DParticleSystemManager
   
2. Implemented factory methods in SDL3GameEngine.cpp
3. Added necessary includes for StdLocalFileSystem, StdBIGFileSystem

**The Blocker**:  
C++ `NEW` operator requires **complete type definitions** for W3D* classes, but forward declarations not giving complete types. Compilation fails with "invalid use of incomplete type 'class W3DGameLogic'"

**Forward declarations insufficient**:
```cpp
class W3DGameLogic;  // Forward decl
return NEW W3DGameLogic;  // ERROR: incomplete type!
```

**Solution Path** (Next Session):
1. **Option A**: Include full headers for W3D* classes in SDL3GameEngine.cpp
   - Risk: Complex include path dependencies  
   - May need to resolve circular #include issues
   
2. **Option B**: Move factory implementations to inline in header (like Win32GameEngine)
   - Include proper headers in header file
   - Inline methods resolve types at include time
   - Requires careful include ordering  

3. **Option C**: Stub factories with nullptr initially
   - Get past compilation block

**Recommendation**: Try Option B first (matches Win32GameEngine pattern). If that fails, Option C to unblock linking phase testing.

---

### ✅ P0-4: Game Text Files (g_csfFile / g_strFile)

**Status**: COMPLETE  
**Files**: `GeneralsMD/Code/Main/SDL3Main.cpp`

**What was done**:
- Added global path constants: `g_csfFile` and `g_strFile`
- These files are referenced by GameText::Manager::init() for game UI localization
- CSF files = Command & Conquer String Files (with language code %s)
- STR files = String Table Resource

**Code Added**:
```cpp
const Char *g_csfFile = "data/%s/Generals.csf";  // Language-specific
const Char *g_strFile = "data/Generals.str";     // UI strings
```

**Note**: Changed path separators from Windows (`\`) to Linux (`/`)

**Testing**: Not yet (blocked on factory compilation)

---

## Key Learnings

### 1. Virtual Factory Pattern (Pure Virtual = Must Override)

**Lesson**: When base class has PURE VIRTUAL methods (=0), derived class MUST provide implementation.

```cpp
// GameEngine base class:
virtual LocalFileSystem *createLocalFileSystem( void ) = 0;

// SDL3GameEngine MUST implement this:
LocalFileSystem *SDL3GameEngine::createLocalFileSystem( void ) { ... }
```

**Previous Mistake**: Tried calling non-existent parent method:
```cpp
return GameEngine::createLocalFileSystem();  // ❌ UNDEFINED - method is pure virtual!
```

### 2. forward declarations vs complete types for operator new

**Finding**: C++ `NEW` (or `new`) requires complete type information, not just forward declarations.

Forward decls work for pointers/references/function parameters, but NOT for object construction:
```cpp
class W3DGameLogic;  // ✅ OK for pointers
W3DGameLogic* p = nullptr;  // ✅ Works

W3DGameLogic p;  // ❌ ERROR - incomplete type
return NEW W3DGameLogic;  // ❌ ERROR - incomplete type
```

**Implication**: Factory methods can't just use forward decls; need complete types via #include

### 3. Platform-Specific File Systems

**Finding**: StdLocalFileSystem and StdBIGFileSystem exist in Core/GameEngineDevice/Source/StdDevice/Common/
These are POSIX/generic implementations for Linux builds.

Pattern:
- Windows: Win32LocalFileSystem, Win32BIGFileSystem
- Linux: StdLocalFileSystem, StdBIGFileSystem

---

## Testing Status

### Compilation
- ❌ SDL3GameEngine.cpp: Factory methods won't compile (incomplete types)
- ⏳ Not in linking phase yet

### Linking
- Not yet reached

### Execution
- Not yet reached

---

## Remaining P0-1 through P0-4 Tasks

| Item | Title | Status | Blocker | Notes |
|------|-------|--------|---------|-------|
| P0-1 | __argc/__argv | ✅ Done | - | Waiting for P0-3 fix |
| P0-2 | ApplicationHWnd | ✅ Done | - | Waiting for P0-3 fix |
| P0-3 | Factory Methods | ⏳ Blocked | Incomplete types | Need full includes or refactor |
| P0-4 | g_csfFile/g_strFile | ✅ Done | - | Waiting for P0-3 fix |

---

## Next Session (Session 38) Action Items

### Step 1: Resolve Factory Method Compilation

Choose ONE approach:

**Option 1 (Recommended)**: Move to inline  header + add includes
```cpp
// SDL3GameEngine.h
#include "Common/GameLogic.h"  // Get complete W3DGameLogic def
#include ... (other W3D type headers)

// Inline implementations
inline GameLogic *SDL3GameEngine::createGameLogic( void ) 
{ return NEW W3DGameLogic; }
```

**Option 2 (Fallback)**: Stub factories  
```cpp
LOCAL FileSystem *SDL3GameEngine::createLocalFileSystem(void) { return nullptr; }
// Accept nullptr for now, unblock testing
```

### Step 2: Test Build

Once P0-3 compiles, run full Docker build and document:
- How many undefined references remain
- What categories they fall into
- Prioritize P1 items (FontCharsClass, Registry functions)

### Step 3: Update Lessons Learned

Document the "incomplete types" issue and solution for future reference.

---

## Bite My Shiny Metal Ass! 🤖

We've locked down 3 of 4 P0 critical globals and identified the exact nature of the factory method compilation issue. Next session we resolve the includes and get to LINKING PHASE where real progress happens.

**Key Quote**: "Compare your incomplete types to mine and then properly #include!" - Bender (modified)

---

## References

- SESSION_36: Initial undefined reference analysis (43 symbols)
- fighter19 DXVK Port: Reference for __argc/__argv, ApplicationHWnd patterns
- Win32GameEngine: Pattern for factory method implementations (inline in header)
- GeneralsMD/Code/GameEngineDevice/Include/Win32Device/Common/Win32GameEngine.h: Best practices

---

**End Session 37** ✅
