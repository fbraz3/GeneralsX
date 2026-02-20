# Current Investigation: Shell Map Not Loading (Static Background Instead of 3D)

**Updated**: 2026-02-19  
**Status**: Root cause hypothesis identified — needs verification in Session 49

## Problem Statement
- Game launches and shows the main menu successfully
- **Static background image** is displayed instead of the 3D animated shell map
- Behavior is identical to passing `-noshellmap` on the command line
- The 3D rotating battlefield scene (shell game) never starts
---

## Shell Map Flow

1. `GameClient::update()` → `TheShell->showShellMap(TRUE)` → `TheShell->showShell()`
   ([GameClient.cpp#L608](../../../GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp#L608))

2. `Shell::showShellMap(useShellMap)` at [Shell.cpp#L537](../../../GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp#L537):
   - If `useShellMap && TheGlobalData->m_shellMapOn` → sends `MSG_NEW_GAME(GAME_SHELL)` with `m_pendingFile = m_shellMapName`
   - Else → loads `Menus/BlankWindow.wnd` (**static background**)

3. Default: `m_shellMapOn = TRUE`, `m_shellMapName = "Maps\\ShellMap1\\ShellMap1.map"`
   ([GlobalData.cpp#L1008](../../../GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp#L1008))

4. `GameData.ini` (inside `INIZH.big`) overrides the name to the correct ZH value:
   ```
   ShellMapName = Maps\ShellMapMD\ShellMapMD.map
   ```

5. `Maps\ShellMapMD\ShellMapMD.map` **IS present** in `MapsZH.big` ✅

---

## Hypotheses (Prioritized)

### H1 (Most Likely): Map file load fails silently → fallback to static BG

The BIG VFS on Linux may fail to match the path `Maps\ShellMapMD\ShellMapMD.map` because
of backslash vs forward-slash handling inconsistency. If `MSG_NEW_GAME(GAME_SHELL)` is
dispatched but the map fails to load, the game likely falls back to the static background
without user-visible error.

**Where to check**: `StdBIGFileSystem::openFile()` path comparison — does it normalize
`\` → `/` before comparing against BIG entry names?

### H2: INI not read / wrong shellmap name loaded

If `INIZH.big` fails to load or `GameData.ini` parsing is skipped, `m_shellMapName`
stays as `Maps\ShellMap1\ShellMap1.map` (Generals, not ZH). That path doesn't exist
in any BIG archive → map load fails → **static background**.

**Where to check**: Print `m_shellMapName` and `m_shellMapOn` at runtime just before
`showShellMap(TRUE)` is called.

### H3: `m_shellMapOn` set to FALSE by unexpected code path

Some code path calls `parseNoShellMap()` or directly sets `m_shellMapOn = FALSE` before
`showShellMap(TRUE)` runs. Possible if Linux SDL command-line args are being mis-parsed
by the game's arg parser.

**Where to check**: Add log at entry of `parseNoShellMap()`.

### H4: `MSG_NEW_GAME(GAME_SHELL)` dispatched but game logic fails

Map IS found but the shell game logic (W3D terrain setup, heightmap, etc.) crashes or
is silently skipped on Linux. The shell then never enters the game mode.

---

## Debug Plan for Session 49

### Step 1 — Confirm values at runtime

Add before `showShellMap(TRUE)` in `GameClient.cpp`:
```cpp
fprintf(stderr, "[SHELL_DIAG] m_shellMapOn=%d m_shellMapName='%s'\n",
    (int)TheGlobalData->m_shellMapOn, TheGlobalData->m_shellMapName.str());
fflush(stderr);
```

### Step 2 — Trace showShellMap branch taken

Add at top of `Shell::showShellMap()`:
```cpp
fprintf(stderr, "[SHELL_DIAG] showShellMap(use=%d) shellMapOn=%d -> %s\n",
    useShellMap, TheGlobalData->m_shellMapOn,
    (useShellMap && TheGlobalData->m_shellMapOn) ? "3D MAP" : "STATIC BG");
fflush(stderr);
```

### Step 3 — If 3D path IS taken: trace MSG_NEW_GAME(GAME_SHELL)

Follow `GameLogic`'s handling of `GAME_SHELL` new game message.

### Step 4 — If static path IS taken: confirm H2 or H3

Check if `m_shellMapName` is the correct ZH path, and whether `m_shellMapOn` became FALSE.

---

## Relevant Files

| File | Purpose |
|------|---------|
| [GlobalData.cpp#L1008](../../../GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp#L1008) | Default init of shellMapName / shellMapOn |
| [Shell.cpp#L537](../../../GeneralsMD/Code/GameEngine/Source/GameClient/GUI/Shell/Shell.cpp#L537) | The showShellMap() decision branch |
| [GameClient.cpp#L608](../../../GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp#L608) | Initial showShellMap(TRUE) call |
| [CommandLine.cpp#L776](../../../GeneralsMD/Code/GameEngine/Source/Common/CommandLine.cpp#L776) | parseNoShellMap() — should NOT be called |
| `Core/GameEngineDevice/Source/StdDevice/Common/StdBIGFileSystem.cpp` | BIG VFS path comparison / normalization |

---

## What Is NOT the Cause

- `-noshellmap` CLI arg: NOT passed ✅
- Default `m_shellMapOn`: TRUE ✅
- Replay mode: NOT active ✅
- `ShellMapMD.map` in archive: File IS in `MapsZH.big` ✅
- `GameData.ini` value: Correct ZH path in `INIZH.big` ✅

---
**Updated**: 2026-02-19 | Session 48

### 2. GameEngine::init() IS Being Called ✅  
- Entry: [GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp](GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp#L46-L47)
- Invocation: `TheGameEngine->init();`
- Debug Output: "DEBUG: GameEngine::init() START" **APPEARS** in logs

###3. Initialization Sequence Maps Correctly ✅
```plaintext
main() [SDL3Main.cpp]
  ↓
GameMain() [GameMain.cpp]
  ↓
GameEngine::init() [GameEngine.cpp:359]   ← CALLED ✅
  ├─ TheSubsystemList = new SubsystemInterfaceList
  ├─ initSubsystem(TheLocalFileSystem...)
  ├─ initSubsystem(TheArchiveFileSystem...)
  ├─ initSubsystem(TheWritableGlobalData...)  ← **HANGS HERE** ⚠️
  │  └─ Calls GlobalData::parseSpecialFile() → generates ExeCRC
  └─ TheShell->push("Menus/MainMenu.wnd") [line 703] ← UNREACHED
```

### 4. generateExeCRC() Completes Successfully ✅
- Function: [GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp:1295](GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp#L1295)
- Linux Optimization: Skips 178MB binary read, uses version-based CRC
- Debug Output: "DEBUG: generateExeCRC() - END, about to execute return statement" **APPEARS**
- Status: **Function completes without error**

### 5. Hang Point EXACTLY Identified ⚠️
Debug traces confirmed:
```
✅ "DEBUG: GameEngine::init() START"
✅ "DEBUG: GameEngine::init() - INI created"  
✅ "DEBUG: GameEngine::init() - About to call initSubsystem(TheWritableGlobalData...)"
✅ "DEBUG: generateExeCRC() about to return CRC: 0x00000002"
✅ "DEBUG: generateExeCRC() - END, about to execute return statement"
❌ "DEBUG: GameEngine::init() - initSubsystem(TheWritableGlobalData) returned"  ← NEVER APPEARS
```

**Conclusion:** Program hangs AFTER `generateExeCRC()` returns but BEFORE `initSubsystem(TheWritableGlobalData)` completes.

## Probable Causes

### 1. Deadlock in INI Parsing (Most Likely)
- `initSubsystem()` triggers `GlobalData::parseSpecialFile()` or similar
- INI file parsing code may have synchronization issue
- Candidate: `std::lock_guard`, `CriticalSection`, or similar locking mechanism

### 2. Infinite Loop in INI Data Processing
- GlobalData initialization reads `Data\INI\Default\GameData` and `Data\INI\GameData`
- May be stuck parsing specific INI field
- Memory issues or string operations causing hang

### 3. File System or Archive Access Issue
- TheArchiveFileSystem (BIG file system) just initialized
- INI parsing may be querying archive for data files
- Possible race condition or deadlock between file systems

### 4. fprintf/fflush Buffering (Unlikely but Possible)
- stderr output stops abruptly after "END, about to execute return statement"
- Could indicate buffer not being flushed in signal handler context
- Less likely given that prior fprintf calls work fine

## Next Steps (Priority Order)

### Immediate (High Priority)
1. **Add debug to TheSubsystemList->initSubsystem()**
   - Trace exactly when it hangs
   - Check if it's in GlobalData parsing or elsewhere

2. **Isolate INI Parsing**
   - Add debug before/after GlobalData::parseSpecialFile()
   - Check which INI field causes hang

3. **Check for Lock/Mutex Issues**
   - Search for CriticalSection usage in GlobalData
   - Look for HANDLE or std::mutex acquisitions
   - Verify no recursive locks

### Secondary (Medium Priority)
4. **Disable INI parsing** (test-only)
   - Comment out initSubsystem(TheWritableGlobalData) temporarily
   - See if program continues to shell menu push

5. **Add stack trace on timeout**
   - Use GDB to get backtrace when timeout triggers
   - Shows exactly which function is executing when hang occurs

## Files to Investigate
- [ ] [SubsystemInterfaceList.cpp](Core/GameEngine/Source/SubsystemInterfaceList.cpp) - initSubsystem() implementation
- [ ] [GlobalData.cpp parseSpecialFile()](GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp) - INI parsing entry
- [ ] [CriticalSection.h](GeneralsMD/Code/CompatLib/Include/critical_section.h) - Synchronization primitives
- [ ] [INI parsing code](GeneralsMD/Code/GameEngine/Source/Common/INI.cpp) - Field parsing logic

## Known Working Code Paths
- SDL3 initialization ✅
- DXVK graphics device ✅
- Audio system (OpenAL) ✅
- File system initialization ✅
- Command line parsing ✅
- Version object creation ✅

## Blocked By
- **Deadlock mystery**: Program hangs synchronously; likely requires advanced debugging (GDB stack trace) to identify

## Notes for Next Session
- **Binary location**: `/home/felipe/Projects/GeneralsX/build/linux64-deploy/GeneralsMD/GeneralsXZH`
- **Build command**: `./scripts/docker-build-linux-zh.sh linux64-deploy`
- **Run command**: `timeout 30s ./build/linux64-deploy/GeneralsMD/GeneralsXZH -win 2>&1`
- **Debug markers** now in code at:
  - GameEngine::init() START/END
  - initSubsystem() call points
  - generateExeCRC() entry/exit

## Session Summary
Successfully narrowed hang from "entire initialization" down to single function call `initSubsystem(TheWritableGlobalData)`. This is major progress - we now have a precise location to investigate. Next session should focus on GDB debugging of that specific call stack.

---
**Investigator**: GitHub Copilot (Claude Haiku)  
**Date**: February 18, 2026  
**Status**: 🔴 Blocked - Requires GDB/stack tracing  
**Confidence**: 95% - Hang location precisely identified
