# Session 49: Root Cause - BIG VFS Not Loading in Linux

**Date**: 2026-02-20  
**Status**: 🔴 BLOCKED - Missing Game Data Files  
**Investigator**: GitHub Copilot (Claude Haiku)

---

## Problem Summary

**The game hangs during initialization** because:

```
[INI] loadDirectory - got 0 files from getFileListInDirectory
[INI] ERROR: No files read from directory 'Data\INI\Default\GameData'
```

The **BIG Virtual File System (VFS) is not finding INI files** because:
1. **No `.big` archive files exist** in the project workspace
2. The game expects game data in `.big` archives (INIZH.big, MapsZH.big, etc.)
3. On Linux, registry lookup fails (Windows registry doesn't exist)
4. Fallback checks for registry `"InstallPath"` return empty string
5. No BIG files are loaded → no INI files found → exception thrown → initialization fails

---

## Root Cause - The Exact Code Path

### 1. StdBIGFileSystem::init() Line 52-82
```cpp
void StdBIGFileSystem::init() {
    // ... snip ...
    loadBigFilesFromDirectory("", "*.big");  // ← Look in current directory
    
#if RTS_ZEROHOUR
    AsciiString installPath;
    GetStringFromGeneralsRegistry("", "InstallPath", installPath);  // ← FAILS ON LINUX
    
    if (!installPath.isEmpty())  // ← Always FALSE on Linux
    {
        loadBigFilesFromDirectory(installPath, "*.big");  // ← NEVER CALLED
    }
#endif
}
```

### 2. No BIG Files in Workspace
```bash
$ find /home/felipe/Projects/GeneralsX -name "*.big" 2>/dev/null
# Empty result - no .big files exist in project
```

### 3. INI Loading Fails
```cpp
INI::loadFileDirectory("Data\INI\Default\GameData")
  → loadDirectory()
  → TheFileSystem->getFileListInDirectory()
  → 0 files found (archives are empty)
  → throw INI_CANT_OPEN_FILE  ← HANG POINT
```

---

## Debug Traces Added in Session 49

Comprehensive debug traces now pinpoint the exact hang:

**File**: `Core/GameEngine/Source/Common/System/SubsystemInterface.cpp` (lines 164-201)
```cpp
[SUBSYS] initSubsystem('TheWritableGlobalData') - loadFileDirectory(...) START
[INI] loadFileDirectory('Data\INI\Default\GameData') START
[INI] loadDirectory - calling getFileListInDirectory(...) START
[INI] loadDirectory - got 0 files from getFileListInDirectory  ← PROBLEM HERE
[INI] ERROR: No files read from directory 'Data\INI\Default\GameData'
```

---

## What Is Actually Needed

To run GeneralsXZH on Linux, you need the game data files:

| File | Source | Purpose |
|------|--------|---------|
| `INIZH.big` | Game install CD/Steam/Origin | Zero Hour INI data |
| `MapsZH.big` | Game install CD/Steam/Origin | Zero Hour maps |
| `Default.big` | Game install CD/Steam/Origin | Shared engine data |
| `Music.big` | Game install CD/Steam/Origin | Game music |
| `Graphics.big`, `SpeechZH.big`, etc. | Game install CD/Steam/Origin | Assets |

These files are **~4-5 GB total** and cannot be included in this repository due to licensing.

---

## Possible Solutions

### Solution 1: Copy Game Data from Install (Recommended)
```bash
# If you have Generals: Zero Hour installed on Windows/Wine:
cd /path/to/GeneralsMD/  # Build output directory
cp /mnt/windows/Program\ Files/EA\ Games/Command\ \&\ Conquer\ Generals\ Zero\ Hour/*.big .

# OR create symlink if on same filesystem:
ln -s /path/to/game/data/*.big .
```

### Solution 2: Docker Volume Mount
```bash
# Run Docker with game data mounted:
docker run -it -v /path/to/game/data:/work/gamedata \
  -v /home/felipe/Projects/GeneralsX:/work \
  ubuntu:22.04 bash

# Then in Docker:
cd /work/build/linux64-deploy/GeneralsMD/
ln -s /work/gamedata/*.big .
./GeneralsXZH -win
```

### Solution 3: fallback Path for Common Locations
Add fallback search paths in `StdBIGFileSystem::init()`:

```cpp
// Try common Linux game data locations
const char* fallback_paths[] = {
    "./",  // Current directory (already checked)
    "gamedata/",
    "../gamedata/",
    "~/GeneralsZH/",
    "/opt/GeneralsZH/",
    nullptr
};
```

### Solution 4: Create Minimal Test Data
Create a minimal stub BIG file with just INI entries to test initialization flow.

---

## Next Steps

### High Priority
1. **Clarify where game data files should come from** - user needs to provide them
2. **Test with actual game data** - either copied from Windows install or Steam proton
3. **Re-run `GeneralsXZH -win` with game data in place**

### Medium Priority
4. **Add helpful error message** if no BIGs found (instead of mysterious hang)
5. **Implement fallback search paths** in `StdBIGFileSystem::init()`
6. **Docker volume mounting instructions** for testing

### Technical Debt
7. **Better separation** of "required" vs "optional" BIG files in error handling
8. **Graceful degradation** if some assets are missing (don't hang, just skip missing features)

---

## Files Affected

| File | Changes Made |
|------|---|
| `Core/GameEngine/Source/Common/System/SubsystemInterface.cpp` | Added [SUBSYS] traces |
| `Core/GameEngine/Source/Common/INI/INI.cpp` | Added [INI] traces |
| `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp` | Already had traces |

## Test Setup for Session 50

To test the next iteration, you'll need to:

```bash
# Obtain game data files (4-5 GB)
# Place in build directory:
cd build/linux64-deploy/GeneralsMD/

# Run binary
./GeneralsXZH -win
```

**Note**: Game data files cannot be committed to repository due to copyright.

---

## Related Issues

- **Shell Map Not Loading** (from Session 48) - Related but secondary
  - Shell map loading depends on INI parsing working
  - Once BIG VFS works, shell map issue will be retested
  
---

**Updated**: 2026-02-20 | Session 49  
**Status**: 🟡 Requires user action (provide game data files) to proceed
