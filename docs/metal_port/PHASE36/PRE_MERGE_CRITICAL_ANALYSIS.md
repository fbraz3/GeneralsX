# Phase 36 - Pre-Merge Critical File Analysis

**Date**: October 19, 2025  
**Branch**: phase35-backup (fac287ab)  
**Merge Target**: original/main (71 commits ahead)

## Purpose

This document identifies and analyzes files modified by BOTH our Phase 27-35 work AND upstream's 71 commits to prevent auto-merge incompatibilities.

## Statistics

- **Our changes**: 695 files modified (Phase 27-35: macOS/Metal port)
- **Upstream changes**: 2493 files modified (71 commits: bug fixes, refactoring, security)
- **Critical files (intersection)**: **208 files** requiring pre-analysis

## Critical Files Breakdown

### HIGH PRIORITY - Known Architectural Changes

These files require CODE REWRITING, not simple merging:

1. **GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp**
   - **Upstream change**: Removed FPS management members (`m_maxFPS`, `m_logicTimeScaleFPS`, etc.)
   - **Upstream change**: Removed methods (`setFramesPerSecondLimit()`, `getFramesPerSecondLimit()`)
   - **Our code**: Still references removed members and functions
   - **Action needed**: Rewrite FPS management code to use `FrameRateLimit` class
   - **API mapping required**: Old GameEngine API → New FrameRateLimit API

2. **Core/GameEngine/Source/Common/FrameRateLimit.cpp**
   - **Status**: ✅ Already resolved using A+B merge
   - **Upstream change**: `LARGE_INTEGER` → `Int64` for m_freq/m_start
   - **Our changes**: Debug logging, division-by-zero protection
   - **Resolution**: Successfully combined both improvements

3. **Core/Libraries/Source/WWVegas/WWDownload/Download.cpp**
   - **Build error**: Redefinition of `MulDiv()`, `compat_getsockname()`, `compat_listen()`
   - **Upstream change**: Added compatibility functions
   - **Our changes**: Functions already in `win32_compat.h`
   - **Action needed**: Remove duplicate definitions, ensure single source

### MEDIUM PRIORITY - File System & Archives

4. **Core/GameEngine/Source/Common/System/ArchiveFileSystem.cpp**
   - **OURS diff**: Platform guards (`#ifdef _WIN32`), POSIX file handling
   - **UPSTREAM diff**: Bug fixes, error handling improvements
   - **Expected merge**: Combine platform abstraction + upstream fixes

5. **Core/GameEngineDevice/Source/Win32Device/Common/Win32BIGFileSystem.cpp**
   - **OURS diff**: Windows-only guards, cross-platform file paths
   - **UPSTREAM diff**: Security improvements, memory handling
   - **Expected merge**: Combine platform guards + upstream security

### MEDIUM PRIORITY - Core Libraries

6. **Core/GameEngine/Source/Common/System/GameMemory.cpp**
   - **Our changes**: `isValidMemoryPointer()` bounds checking (Phase 30.6)
   - **Upstream changes**: Memory allocation improvements
   - **Expected merge**: Combine both safety improvements

7. **Core/Libraries/Source/WWVegas/WWLib/ini.cpp**
   - **Our changes**: `End` token protection (Phase 22.7-22.8 breakthrough)
   - **Upstream changes**: INI parsing bug fixes
   - **Expected merge**: Combine both parsing improvements

8. **Core/Libraries/Source/WWVegas/WWLib/TARGA.CPP**
   - **Our changes**: Texture loading for Metal backend
   - **Upstream changes**: TGA format handling fixes
   - **Expected merge**: Combine both improvements

### MEDIUM PRIORITY - Graphics & Rendering

9. **Core/Libraries/Source/WWVegas/WW3D2/w3d_dep.cpp**
   - **Our changes**: DirectX 8 mock layer, Metal/OpenGL dispatch
   - **Upstream changes**: W3D rendering fixes
   - **Expected merge**: Combine platform abstraction + upstream fixes

10. **Core/GameEngineDevice/Include/W3DDevice/GameClient/W3DVideoBuffer.h**
    - **Our changes**: Metal backend integration
    - **Upstream changes**: Video buffer API improvements
    - **Expected merge**: Combine both improvements

### LOW PRIORITY - Headers Only

11. **Core/GameEngine/Include/Common/AsciiString.h**
12. **Core/GameEngine/Include/Common/UnicodeString.h**
13. **Core/GameEngine/Include/Common/GameMemory.h**
14. **Core/Libraries/Source/WWVegas/WWLib/always.h**
15. **Core/Libraries/Source/WWVegas/WWLib/win.h**

- **Analysis**: Header changes typically auto-merge cleanly
- **Action**: Review after merge for missing includes or type conflicts

## GameEngine Refactoring Deep Dive

### Old API (Our Code Still Uses)

```cpp
// GameEngine.h (pre-refactor)
class GameEngine {
    unsigned m_maxFPS;
    unsigned m_logicTimeScaleFPS;
    unsigned m_updateTime;
    bool m_enableLogicTimeScale;
    bool m_isTimeFrozen;
    bool m_isGameHalted;
    
    void setFramesPerSecondLimit(unsigned fps);
    unsigned getFramesPerSecondLimit();
    void enableLogicTimeScale(bool enable);
    void setLogicTimeScaleFps(unsigned fps);
};
```

### New API (Upstream Refactor)

```cpp
// FrameRateLimit.h (upstream)
class FrameRateLimit {
    Int64 m_freq;          // was: LARGE_INTEGER in our backup
    Int64 m_start;
    unsigned m_maxFps;     // moved from GameEngine
    
    void setMaxFps(unsigned fps);
    unsigned getMaxFps();
};

// GameEngine.h (upstream - FPS management removed)
class GameEngine {
    FrameRateLimit m_frameRateLimit;  // Composition instead of direct members
    // m_maxFPS removed
    // m_logicTimeScaleFPS removed
    // setFramesPerSecondLimit() removed
};
```

### Migration Strategy

**Option 1: Update our GameEngine.cpp to use FrameRateLimit**
- Pros: Aligns with upstream architecture
- Cons: More code changes required
- Example: `m_maxFPS` → `m_frameRateLimit.getMaxFps()`

**Option 2: Keep wrapper methods in GameEngine**
- Pros: Minimal code changes
- Cons: Diverges from upstream architecture
- Example: Add `setFramesPerSecondLimit() { m_frameRateLimit.setMaxFps(fps); }`

**Recommended**: Option 1 - Full alignment with upstream architecture

## Pre-Merge Action Plan

### Phase 1: Analyze Critical Files (Before Merge)

1. **For each critical file**:
   ```bash
   # Generate three-way diff view
   git diff original/main~71:path/to/file phase35-backup:path/to/file > OURS.diff
   git diff original/main~71:path/to/file original/main:path/to/file > UPSTREAM.diff
   ```

2. **Document in `logs/conflict_analysis/FILENAME_analysis.md`**:
   - Upstream architectural changes (like GameEngine refactor)
   - Our platform-specific changes
   - Expected merge strategy (A+B combine vs code rewrite)

### Phase 2: GameEngine Refactoring (Special Case)

1. **Create API migration guide**:
   - Map old API calls to new API
   - Identify all files calling removed functions
   - Plan code updates before merge

2. **Files requiring GameEngine updates**:
   ```bash
   # Find all files calling removed GameEngine methods
   grep -r "setFramesPerSecondLimit\|getFramesPerSecondLimit" GeneralsMD/Code/ --include="*.cpp" --include="*.h"
   grep -r "m_maxFPS\|m_logicTimeScaleFPS" GeneralsMD/Code/ --include="*.cpp" --include="*.h"
   ```

### Phase 3: Incremental Merge with Validation

1. **Resolve conflicts in batches of 5**
2. **Build after each batch**: `cmake --build build/macos-arm64 --target GeneralsXZH -j 4`
3. **If build fails**: Analyze, fix, document, then continue
4. **Commit after each successful batch**

### Phase 4: Post-Merge Validation

1. **Full rebuild**: All targets (GeneralsXZH + GeneralsX)
2. **Runtime test**: Launch game, verify subsystems operational
3. **Regression test**: Compare with Phase 35 functionality

## Lessons Applied

1. **Don't trust auto-merge for architectural changes**
2. **Pre-analyze files modified by both branches**
3. **Build incrementally (every 5-10 files)**
4. **Document API migrations before code changes**

## Next Steps

1. ✅ Create this pre-analysis document
2. ⏳ Analyze GameEngine refactoring in detail
3. ⏳ Create API migration guide
4. ⏳ Identify all files needing GameEngine updates
5. ⏳ Restart merge with critical files pre-analyzed
6. ⏳ Resolve conflicts incrementally with build validation

---

**Last Updated**: October 19, 2025  
**Merge Status**: Aborted, awaiting pre-analysis completion
