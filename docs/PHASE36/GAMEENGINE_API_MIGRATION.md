# Phase 36 - GameEngine API Migration Guide

**Date**: October 19, 2025  
**Issue**: Upstream refactored GameEngine class, moved FPS management to FrameRateLimit  
**Impact**: 13 files across GameEngine, GameClient, GameLogic referencing removed API

## Problem Summary

Upstream commit `original/main` refactored the GameEngine class:

- **Removed members**: `m_maxFPS`, `m_logicTimeScaleFPS`, `m_enableLogicTimeScale`
- **Removed methods**: `setFramesPerSecondLimit()`, `getFramesPerSecondLimit()`, `setLogicTimeScaleFps()`, `enableLogicTimeScale()`
- **New architecture**: FPS management moved to `FrameRateLimit` class (composition pattern)

Our Phase 27-35 backup code still uses the old API, causing compilation errors.

## Upstream Architecture Change

### Before (Our Backup Code)

```cpp
// GameEngine.h (old)
class GameEngine {
    Int m_maxFPS;                    ///< Maximum frames per second for rendering
    Int m_logicTimeScaleFPS;         ///< Maximum frames per second for logic time scale
    Bool m_enableLogicTimeScale;     ///< Enable logic time scaling
    
    virtual void setFramesPerSecondLimit(Int fps);   ///< Set max render and engine update fps
    virtual Int getFramesPerSecondLimit(void);       ///< Get max render and engine update fps
    virtual void setLogicTimeScaleFps(Int fps);      ///< Set logic time scale fps
    virtual void enableLogicTimeScale(Bool enable);  ///< Enable logic time scale
};
```

### After (Upstream Refactor)

```cpp
// GameEngine.h (upstream)
class GameEngine {
    FrameRateLimit m_frameRateLimit;  ///< Composition - FPS management delegated
    
    // m_maxFPS - REMOVED
    // m_logicTimeScaleFPS - REMOVED
    // m_enableLogicTimeScale - REMOVED
    // setFramesPerSecondLimit() - REMOVED
    // getFramesPerSecondLimit() - REMOVED
};

// FrameRateLimit.h (upstream)
class FrameRateLimit {
    Int64 m_freq;         // Performance counter frequency
    Int64 m_start;        // Performance counter start
    unsigned m_maxFps;    // Maximum frames per second
    
    void setMaxFps(unsigned fps);
    unsigned getMaxFps();
};
```

## Files Requiring Migration

Total: **13 files** across 4 subsystems

### 1. Core Engine (2 files)

#### `GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h`
**Lines affected**: 79-80, 87, 89, 127-128, 135

**Current code**:
```cpp
virtual void setFramesPerSecondLimit( Int fps );
virtual Int getFramesPerSecondLimit( void );
virtual void setLogicTimeScaleFps( Int fps );
virtual void enableLogicTimeScale( Bool enable );
Int m_maxFPS;
Int m_logicTimeScaleFPS;
Bool m_enableLogicTimeScale;
```

**Migration action**: REMOVE declarations (members moved to FrameRateLimit)

---

#### `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`
**Lines affected**: 285-286, 291, 352-361, 416-436, 459, 462, 1039, 1279, 1486

**Current code**:
```cpp
// Line 285-286: Constructor initialization
m_maxFPS = BaseFps;
m_logicTimeScaleFPS = LOGICFRAMES_PER_SECOND;

// Line 352-361: Setter/getter implementation
void GameEngine::setFramesPerSecondLimit( Int fps ) {
    DEBUG_LOG(("GameEngine::setFramesPerSecondLimit() - setting max fps to %d", fps));
    m_maxFPS = fps;
}

Int GameEngine::getFramesPerSecondLimit( void ) {
    return m_maxFPS;
}

// Line 1039: Registry initialization
setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

// Line 1279: Usage in update loop
const Int maxRenderFps = getFramesPerSecondLimit();

// Line 1486: Uncapped FPS logic
const UnsignedInt maxFps = allowFpsLimit ? getFramesPerSecondLimit() : RenderFpsPreset::UncappedFpsValue;
```

**Migration action**: UPDATE to use `m_frameRateLimit`

**New code**:
```cpp
// Constructor: Initialize FrameRateLimit instead
m_frameRateLimit.setMaxFps(BaseFps);

// Registry initialization: Delegate to FrameRateLimit
m_frameRateLimit.setMaxFps(TheGlobalData->m_framesPerSecondLimit);

// Usage: Call through FrameRateLimit
const Int maxRenderFps = m_frameRateLimit.getMaxFps();
const UnsignedInt maxFps = allowFpsLimit ? m_frameRateLimit.getMaxFps() : RenderFpsPreset::UncappedFpsValue;
```

### 2. Game Client (4 files)

#### `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/CommandXlat.cpp`
**Lines affected**: 193, 196, 220, 242, 245, 250

**Current code**:
```cpp
// Line 193: Read max FPS
UnsignedInt maxRenderFps = TheGameEngine->getFramesPerSecondLimit();

// Line 196: Set max FPS
TheGameEngine->setFramesPerSecondLimit(maxRenderFps);

// Line 220: Read for comparison
const UnsignedInt maxRenderFps = TheGameEngine->getFramesPerSecondLimit();

// Line 242, 250: Set logic time scale
TheGameEngine->setLogicTimeScaleFps(logicTimeScaleFps);

// Line 245: Enable logic time scale
TheGameEngine->enableLogicTimeScale(logicTimeScaleFps < maxRenderFps);
```

**Migration action**: UPDATE to use `m_frameRateLimit`

**New code**:
```cpp
UnsignedInt maxRenderFps = TheGameEngine->getFrameRateLimit()->getMaxFps();
TheGameEngine->getFrameRateLimit()->setMaxFps(maxRenderFps);
```

**Note**: Logic time scale might need separate implementation (check if still in GameEngine)

---

#### `GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp`
**Lines affected**: 6060

**Current code**:
```cpp
renderFpsLimit = (UnsignedInt)TheGameEngine->getFramesPerSecondLimit();
```

**Migration action**: UPDATE call

**New code**:
```cpp
renderFpsLimit = (UnsignedInt)TheGameEngine->getFrameRateLimit()->getMaxFps();
```

---

#### `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/QuitMenu.cpp`
**Lines affected**: 218

**Current code**:
```cpp
Int fps = TheGameEngine->getFramesPerSecondLimit();
```

**Migration action**: UPDATE call

**New code**:
```cpp
Int fps = TheGameEngine->getFrameRateLimit()->getMaxFps();
```

---

#### `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
**Lines affected**: 1276

**Current code**:
```cpp
unibuffer.format( L"%.2f/%d FPS, ", fps, TheGameEngine->getFramesPerSecondLimit());
```

**Migration action**: UPDATE call

**New code**:
```cpp
unibuffer.format( L"%.2f/%d FPS, ", fps, TheGameEngine->getFrameRateLimit()->getMaxFps());
```

### 3. Game Logic (4 files)

#### `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptActions.cpp`
**Lines affected**: 7132, 7136

**Current code**:
```cpp
// Line 7132: Reset to default
TheGameEngine->setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

// Line 7136: Script-controlled FPS
TheGameEngine->setFramesPerSecondLimit(pAction->getParameter(0)->getInt());
```

**Migration action**: UPDATE calls

**New code**:
```cpp
TheGameEngine->getFrameRateLimit()->setMaxFps(TheGlobalData->m_framesPerSecondLimit);
TheGameEngine->getFrameRateLimit()->setMaxFps(pAction->getParameter(0)->getInt());
```

---

#### `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptEngine.cpp`
**Lines affected**: 5269

**Current code**:
```cpp
TheGameEngine->setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);
```

**Migration action**: UPDATE call

**New code**:
```cpp
TheGameEngine->getFrameRateLimit()->setMaxFps(TheGlobalData->m_framesPerSecondLimit);
```

---

#### `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`
**Lines affected**: 439

**Current code**:
```cpp
TheGameEngine->setFramesPerSecondLimit(maxFPS);
```

**Migration action**: UPDATE call

**New code**:
```cpp
TheGameEngine->getFrameRateLimit()->setMaxFps(maxFPS);
```

---

#### `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp`
**Lines affected**: 4275, 4314

**Current code**:
```cpp
// Line 4275: Disable logic time scale
TheGameEngine->enableLogicTimeScale(FALSE);

// Line 4314: Enable logic time scale
TheGameEngine->enableLogicTimeScale(TRUE);
```

**Migration action**: CHECK if logic time scale still exists in GameEngine

**Investigation needed**: Search upstream for logic time scale implementation

## Migration Strategy

### Option A: Add Accessor to GameEngine (Recommended)

**Pros**:
- Minimal code changes (13 files)
- Maintains encapsulation
- Easy to revert if upstream changes again

**Implementation**:
```cpp
// GameEngine.h
class GameEngine {
    FrameRateLimit m_frameRateLimit;  // Existing upstream member
    
public:
    FrameRateLimit* getFrameRateLimit() { return &m_frameRateLimit; }  // ADD THIS
};
```

**Usage in all 13 files**:
```cpp
// Before
TheGameEngine->getFramesPerSecondLimit();

// After
TheGameEngine->getFrameRateLimit()->getMaxFps();
```

### Option B: Add Wrapper Methods to GameEngine

**Pros**:
- No changes to caller code
- API backward compatible

**Cons**:
- Diverges from upstream architecture
- More maintenance burden

**Implementation**:
```cpp
// GameEngine.h
class GameEngine {
    FrameRateLimit m_frameRateLimit;
    
public:
    // Wrapper methods (deprecated)
    void setFramesPerSecondLimit(Int fps) { m_frameRateLimit.setMaxFps(fps); }
    Int getFramesPerSecondLimit() { return m_frameRateLimit.getMaxFps(); }
};
```

**Usage**: No changes required (backward compatible)

### Recommended: Option A

**Rationale**:
1. Aligns with upstream architecture (composition pattern)
2. Makes dependency explicit (callers see they're accessing FrameRateLimit)
3. Future-proof if upstream adds more FrameRateLimit features
4. Only 13 files to update (manageable scope)

## Implementation Plan

### Pre-Merge (Before Starting Phase 36 Merge)

1. **Create feature branch for migration**:
   ```bash
   git checkout -b feature/gameengine-api-migration phase35-backup
   ```

2. **Add accessor method to GameEngine.h**:
   ```cpp
   FrameRateLimit* getFrameRateLimit() { return &m_frameRateLimit; }
   ```

3. **Update all 13 files** (one commit per subsystem):
   - Commit 1: Core Engine (GameEngine.cpp)
   - Commit 2: Game Client (CommandXlat, InGameUI, QuitMenu, W3DDisplay)
   - Commit 3: Game Logic (ScriptActions, ScriptEngine, GameLogicDispatch, GameLogic)

4. **Build and test**:
   ```bash
   cmake --build build/macos-arm64 --target GeneralsXZH -j 4
   ```

5. **Merge migration branch to phase35-backup**:
   ```bash
   git checkout phase35-backup
   git merge --no-ff feature/gameengine-api-migration
   ```

### During Merge (Phase 36)

1. **Start merge from migrated state**:
   ```bash
   git merge original/main --no-ff --no-commit
   ```

2. **GameEngine.h conflict**: Accept UPSTREAM (has FrameRateLimit composition)
3. **GameEngine.cpp conflict**: Use OURS (our calls are already migrated)
4. **No API mismatch errors** (migration done pre-merge)

## Verification Checklist

- [ ] All 13 files updated to use `getFrameRateLimit()->getMaxFps()`
- [ ] GameEngine.h has accessor method: `FrameRateLimit* getFrameRateLimit()`
- [ ] Build successful: `cmake --build build/macos-arm64 --target GeneralsXZH -j 4`
- [ ] No references to old API: `grep -r "setFramesPerSecondLimit" GeneralsMD/Code/`
- [ ] Logic time scale handling verified (investigate upstream implementation)
- [ ] Runtime test: Game launches and FPS limiting works

## Logic Time Scale Investigation

**TODO**: Check if upstream still has logic time scale feature:

```bash
# Search upstream for logic time scale
git show original/main:GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h | grep -i "logictime\|timescale"
git show original/main:GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp | grep -i "logictime\|timescale"
```

**If removed**: Update GameLogic.cpp to remove `enableLogicTimeScale()` calls  
**If moved**: Update calls to new location

## Timeline

**Estimated time**: 2-3 hours
- Pre-merge migration: 1-2 hours (13 files)
- Testing and verification: 30 minutes
- Logic time scale investigation: 30 minutes

**Blocker resolution**: Must complete before Phase 36 merge restart

---

**Last Updated**: October 25, 2025  
**Status**: ⚠️ BLOCKER DISCOVERED - Cannot implement before merge  
**Next Step**: Merge upstream first, then resolve API conflicts during merge

## CRITICAL DISCOVERY (October 25, 2025)

### Blocker: FramePacer Does Not Exist in Our Codebase

**Problem**: The migration strategy assumed we could pre-migrate our code to use `TheFramePacer` before merging upstream. However, this is **impossible** because:

1. **FramePacer.h/cpp do not exist** in our Phase 35 codebase
2. **TheFramePacer singleton** requires initialization infrastructure not present
3. **Creating a stub would be extremely risky** - wrong initialization order could cause subtle bugs
4. **Build fails immediately** with `fatal error: 'Common/FramePacer.h' file not found`

### Evidence from Implementation Attempt

Attempted to:
- ✅ Remove FPS methods from GameEngine.h (successful)
- ✅ Update GameEngine.cpp to delegate to TheFramePacer (successful editing)
- ✅ Update all 13 caller files with sed script (successful substitution)
- ❌ **BUILD FAILED**: `fatal error: 'Common/FramePacer.h' file not found`

Compilation errors:
```
GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:49:10: 
fatal error: 'Common/FramePacer.h' file not found
#include "Common/FramePacer.h"
         ^~~~~~~~~~~~~~~~~~~~~
```

Even with include removed:
```
error: use of undeclared identifier 'TheFramePacer'
TheFramePacer->setFramesPerSecondLimit(...)
^
```

### Why Stub Solution Won't Work

Creating a minimal FramePacer stub would require:
1. Implementing FramePacer class with all 15+ methods
2. Creating TheFramePacer singleton initialization (where? when?)
3. Ensuring initialization happens BEFORE GameEngine uses it
4. Risk of initialization order bugs (static initialization fiasco)
5. Risk of behavioral differences from upstream implementation
6. Would need to be removed/replaced during merge anyway

**Risk vs Benefit**: High risk, zero benefit (merge will replace it anyway)

## REVISED STRATEGY: Merge-Time Resolution

### New Approach: Accept Conflicts, Resolve During Merge

Instead of pre-migration, we will:

1. **Start merge directly**: `git merge original/main --no-ff --no-commit`
2. **Expect compilation failures** in the 13 identified files
3. **Use this guide as reference** during conflict resolution
4. **Resolve conflicts incrementally** with build validation every 5 files

### Why This is Better

✅ **No risk of creating incompatible stubs**  
✅ **Upstream FramePacer comes with correct initialization**  
✅ **Can see exact upstream implementation while resolving**  
✅ **Conflicts will show us exactly what needs changing**  
✅ **Build errors will guide us to missed files**  

### Updated Merge Workflow

```bash
# 1. Start merge (expect conflicts)
git merge original/main --no-ff --no-commit

# 2. For each conflict in our 13 files:
#    - Check this guide for expected changes
#    - Replace TheGameEngine->METHOD() with TheFramePacer->METHOD()
#    - Accept upstream's FramePacer.h/cpp
#    - Accept upstream's GameEngine.h (methods removed)

# 3. Build after each batch of 5 files
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# 4. Fix any additional files revealed by compilation errors
grep "error:.*getFramesPerSecondLimit\|setFramesPerSecondLimit" build.log

# 5. Commit when all files compile
git commit
```

### Files Still Requiring Manual Updates (During Merge)

The 13 files identified still need updates, but DURING merge, not before:

**Core Engine** (2 files):
- `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp` - Internal calls

**Game Client** (5 files):
- `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/CommandXlat.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/QuitMenu.cpp`
- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`
- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp` (discovered during testing)

**Game Logic** (4 files):
- `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptActions.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameLogic/ScriptEngine/ScriptEngine.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp`
- `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp`

**Additional Files** (2 files discovered):
- `GeneralsMD/Code/GameEngine/Source/GameClient/MessageStream/LookAtXlat.cpp` (uses getUpdateFps)

### Quick Reference for Merge Conflict Resolution

**Pattern 1: FPS Getters/Setters**
```cpp
// Before (conflict)
TheGameEngine->getFramesPerSecondLimit()
TheGameEngine->setFramesPerSecondLimit(fps)

// After (resolution)
TheFramePacer->getFramesPerSecondLimit()
TheFramePacer->setFramesPerSecondLimit(fps)
```

**Pattern 2: Update Time**
```cpp
// Before
TheGameEngine->getUpdateTime()
TheGameEngine->getUpdateFps()

// After
TheFramePacer->getUpdateTime()
TheFramePacer->getUpdateFps()
```

**Pattern 3: Logic Time Scale**
```cpp
// Before
TheGameEngine->setLogicTimeScaleFps(fps)
TheGameEngine->enableLogicTimeScale(enable)

// After
TheFramePacer->setLogicTimeScaleFps(fps)
TheFramePacer->enableLogicTimeScale(enable)
```

### Lessons Learned

1. **Pre-migration requires complete infrastructure** - can't migrate to non-existent classes
2. **Merge-time resolution is safer** when upstream introduces new architecture
3. **Analysis is still valuable** - this guide accelerates merge conflict resolution
4. **Build early, build often** - would have discovered blocker sooner with earlier build

---

**Implementation Attempt**: October 25, 2025 (reverted)  
**Discovery**: FramePacer cannot be stubbed safely  
**Recommendation**: Merge first, resolve conflicts during merge using this guide
