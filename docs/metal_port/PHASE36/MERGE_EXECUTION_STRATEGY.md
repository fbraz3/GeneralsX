# Phase 36 - Merge Execution Strategy

**Created**: October 25, 2025  
**Status**: Ready to Execute  
**Strategy**: Direct merge with incremental conflict resolution

---

## Executive Summary

After discovering that pre-merge API migration is impossible (FramePacer doesn't exist in our codebase), we pivot to **direct merge strategy**:

1. ✅ Merge upstream/main directly (brings FramePacer into our code)
2. ✅ Resolve conflicts in batches of 5 files
3. ✅ Build after each batch to catch issues early
4. ✅ Use migration documentation as reference during conflict resolution

**Estimated Time**: 5-8 hours (31+ conflicts expected)

---

## Pre-Execution Checklist

- [x] Analysis complete (208 critical files documented)
- [x] Migration guide ready (GAMEENGINE_API_MIGRATION.md)
- [x] Clean state achieved (main branch, no uncommitted changes)
- [x] Pre-merge attempt reverted (lessons learned documented)
- [ ] **Remote configured**: `upstream` → `https://github.com/TheSuperHackers/GeneralsGameCode.git`
- [ ] **Fetch latest**: `git fetch upstream`
- [ ] **Start merge**: `git merge upstream/main --no-ff --no-commit`

---

## Batch Resolution Strategy

### Batch Size: 5 Files
- Small enough to track changes clearly
- Large enough to make progress efficiently
- Build validation after each batch

### Priority Order (from PRE_MERGE_CRITICAL_ANALYSIS.md)

**Batch 1: File System (MEDIUM priority)**
1. `Core/Libraries/Source/debug/debug_io_flat.cpp`
2. `Core/Libraries/Source/debug/debug_stack.cpp`
3. `Core/Libraries/Source/WWFile/ArchiveFileSystem.cpp`
4. `Core/Libraries/Source/WWFile/Win32BIGFileSystem.cpp`
5. `Core/Libraries/Source/WWFile/Win32LocalFileSystem.cpp`

**Strategy**: A+B merge (combine our macOS fixes + upstream improvements)

**Batch 2: Core Libraries**
- Next 5 files from `git status` conflict list
- Continue A+B merge strategy

**Batch 3: GameEngine (HIGH priority - CRITICAL)**
1. `GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h`
2. `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`

**Strategy**:
- **GameEngine.h**: ACCEPT UPSTREAM (FPS methods removed)
- **GameEngine.cpp**: CAREFUL MERGE
  - Accept upstream's FramePacer delegation
  - **PRESERVE** Phase 35.2 initialization order fixes:
    - UpgradeCenter/ThingFactory before SkirmishGameOptionsInit
    - Lazy validation pattern for Science.cpp

**Batch 4-N**: Continue with remaining conflicts in 5-file batches

---

## API Migration During Merge

### Files Requiring API Updates (from GAMEENGINE_API_MIGRATION.md)

**Game Client** (5 files):
- `CommandXlat.cpp` - 6 locations (lines 193, 196, 220, 242, 245, 250)
- `InGameUI.cpp` - 1 location (line 6060)
- `QuitMenu.cpp` - 1 location (line 218)
- `W3DDisplay.cpp` - 1 location (line 1276)
- `W3DView.cpp` - discovered during testing

**Game Logic** (4 files):
- `ScriptActions.cpp` - 2 locations (lines 7132, 7136)
- `ScriptEngine.cpp` - 1 location (line 5269)
- `GameLogicDispatch.cpp` - 1 location (line 439)
- `GameLogic.cpp` - 2 locations (lines 4275, 4314)

### Quick Reference: API Replacements

```cpp
// Pattern 1: FPS Getters/Setters
TheGameEngine->getFramesPerSecondLimit()  →  TheFramePacer->getFramesPerSecondLimit()
TheGameEngine->setFramesPerSecondLimit()  →  TheFramePacer->setFramesPerSecondLimit()

// Pattern 2: Update Time
TheGameEngine->getUpdateTime()  →  TheFramePacer->getUpdateTime()
TheGameEngine->getUpdateFps()   →  TheFramePacer->getUpdateFps()

// Pattern 3: Logic Time Scale
TheGameEngine->setLogicTimeScaleFps()    →  TheFramePacer->setLogicTimeScaleFps()
TheGameEngine->enableLogicTimeScale()    →  TheFramePacer->enableLogicTimeScale()
```

### Sed Script for Batch Updates

```bash
# Game Client files
for file in CommandXlat.cpp InGameUI.cpp QuitMenu.cpp W3DDisplay.cpp W3DView.cpp; do
  sed -i '' 's/TheGameEngine->getFramesPerSecondLimit()/TheFramePacer->getFramesPerSecondLimit()/g' \
    "GeneralsMD/Code/GameEngine/Source/GameClient/**/$file"
done

# Game Logic files
for file in ScriptActions.cpp ScriptEngine.cpp GameLogicDispatch.cpp GameLogic.cpp; do
  sed -i '' 's/TheGameEngine->setFramesPerSecondLimit(/TheFramePacer->setFramesPerSecondLimit(/g' \
    "GeneralsMD/Code/GameEngine/Source/GameLogic/**/$file"
  sed -i '' 's/TheGameEngine->enableLogicTimeScale(/TheFramePacer->enableLogicTimeScale(/g' \
    "GeneralsMD/Code/GameEngine/Source/GameLogic/**/$file"
done
```

---

## Build Workflow

### After Each Batch

```bash
# Add resolved files
git add <resolved_files>

# Build incrementally
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 | tee logs/merge_batch${N}_build.log

# Check for errors
grep -i "error:" logs/merge_batch${N}_build.log | wc -l

# If errors found, fix before continuing
grep -i "error:" logs/merge_batch${N}_build.log | head -20
```

### Expected Build Sequence

1. **After File System batch**: Clean build (no FramePacer issues yet)
2. **After GameEngine batch**: ~13 compilation errors (API calls in client/logic files)
3. **After Game Client API fixes**: ~4 errors remaining (Game Logic files)
4. **After Game Logic API fixes**: 0 errors (all API migrated)
5. **Final build**: 0 errors, warnings acceptable (check for new warnings)

---

## Conflict Resolution Patterns

### Pattern 1: A+B Merge (File System, Libraries)

```bash
# Three-way diff analysis
git diff :1:<file> :2:<file> > logs/conflict_ours.diff
git diff :1:<file> :3:<file> > logs/conflict_theirs.diff

# Manual merge: combine both improvements
# - Keep our macOS/Metal fixes
# - Add upstream's enhancements
# - Test that both work together
```

### Pattern 2: Accept Upstream (GameEngine.h)

```bash
git checkout --theirs GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h
git add GeneralsMD/Code/GameEngine/Include/Common/GameEngine.h
```

### Pattern 3: Careful Merge (GameEngine.cpp)

```bash
# Open in editor, manually merge:
# 1. Accept upstream's FramePacer delegation
# 2. Preserve Phase 35.2 initialization order:
#    - UpgradeCenter/ThingFactory BEFORE SkirmishGameOptionsInit
#    - Science lazy validation pattern
# 3. Build and test
```

---

## Validation Gates

### Build Validation
- ✅ 0 compilation errors
- ✅ Warnings ≤ baseline (26 warnings in Phase 35)
- ✅ All targets build successfully

### API Migration Validation
```bash
# Verify old API removed
grep -rn 'TheGameEngine->getFramesPerSecondLimit\|TheGameEngine->setFramesPerSecondLimit' \
  GeneralsMD/Code/ --include='*.cpp'
# Expected: 0 matches
```

### Runtime Validation
```bash
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 timeout 60 ./GeneralsXZH 2>&1 | tee logs/merge_runtime_test.log

# Check key metrics:
grep "TheFramePacer" logs/merge_runtime_test.log | head -5  # Initialization
grep "GameEngine.*Init" logs/merge_runtime_test.log         # Engine init
grep "ERROR\|FATAL\|crash" logs/merge_runtime_test.log      # No crashes
```

### Crash Log Verification
```bash
cat "$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt"
# Expected: No new crashes (compare with Phase 35 baseline)
```

---

## Incremental Commit Strategy

### Option A: Single Merge Commit (Recommended)
```bash
# After all conflicts resolved and validated
git commit -m "feat(phase36): merge 71 upstream commits with macOS/Metal preservation

Merged 71 commits from superhackers/GeneralsGameCode upstream:
- FramePacer class introduction (FPS/timing management)
- GameEngine refactoring (FPS methods removed)
- [List other major upstream changes]

Conflict Resolution:
- 31+ conflicts resolved using three-way merge strategy
- File System: A+B merge (our macOS fixes + upstream improvements)
- GameEngine: Careful merge (preserved Phase 35.2 initialization order)
- API Migration: 13 files updated (TheGameEngine → TheFramePacer)

Preserved Phase 27-35 Work:
- Metal/OpenGL graphics backends
- macOS platform compatibility (win32_compat.h)
- DirectX 8 mock layer (d3d8.h)
- Texture loading pipeline (BC3/DDS/TGA)
- Audio system investigation
- Code cleanup and protection removal

Build Status: ✅ 0 errors, 26 warnings (baseline)
Runtime Status: ✅ 60+ frames, no crashes

Refs: Phase 36 - Upstream Merge with FramePacer Integration"
```

### Option B: Batched Commits (if complex)
```bash
# After each major batch
git commit -m "feat(phase36): resolve File System conflicts (batch 1/N)"
git commit -m "feat(phase36): resolve GameEngine conflicts with FramePacer (batch 2/N)"
# ...
git commit -m "feat(phase36): complete API migration TheGameEngine→TheFramePacer"
```

---

## Emergency Rollback

If merge becomes too complex:

```bash
# Abort merge completely
git merge --abort

# Return to clean state
git reset --hard phase35-backup

# Re-analyze specific problematic files
git show upstream/main:<file> > /tmp/upstream_version.cpp
git show phase35-backup:<file> > /tmp/our_version.cpp
diff -u /tmp/our_version.cpp /tmp/upstream_version.cpp > logs/detailed_diff.txt
```

---

## Success Criteria

**Merge Complete When**:
- ✅ All conflicts resolved (git status shows no unmerged paths)
- ✅ Build succeeds with 0 errors
- ✅ Runtime test runs 60+ seconds without crash
- ✅ API migration verified (grep shows 0 old API usage)
- ✅ Phase 27-35 features still operational (Metal, textures, etc.)
- ✅ Documentation updated (MACOS_PORT_DIARY.md)
- ✅ Commit message comprehensive
- ✅ Ready to push to origin/main

**Estimated Timeline**:
- Conflict resolution: 3-5 hours (31+ files, batches of 5)
- API migration: 1-2 hours (13 files, semi-automated with sed)
- Build validation: 1 hour (incremental builds)
- Runtime validation: 30 minutes (initialization + crash log check)
- Documentation: 30 minutes (diary update, lessons learned)
- **Total**: 6-9 hours

---

## Next Steps After Merge

1. **Push to origin**: `git push origin main`
2. **Create Phase 37 planning**: Assess state after merge, plan next features
3. **Update reference repos**: `git submodule update --remote`
4. **Celebrate**: 🎉 71 commits merged with full preservation of 9 months of work!

---

**Ready to Execute**: October 25, 2025  
**Strategy Validated**: Yes (post-discovery revision)  
**Documentation Complete**: Yes (PRE_MERGE_CRITICAL_ANALYSIS.md, GAMEENGINE_API_MIGRATION.md)  
**First Command**: `git fetch upstream && git merge upstream/main --no-ff --no-commit`
