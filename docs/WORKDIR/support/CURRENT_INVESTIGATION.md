# Current Investigation: Shell Menu Not Rendering (Feb 18, 2026)

## Status
**HANG LOCATION IDENTIFIED** - Program halts inside `initSubsystem(TheWritableGlobalData)` call during GameEngine initialization.

## Problem Statement
- Game loads SDL3 window and initializes DXVK graphics successfully  
- `GameEngine::init()` starts executing
- Program halts during `initSubsystem(TheWritableGlobalData, ...)` call
- Shell menu never appears; screen stays magenta

## Key Findings

### 1. Shell Code IS Present and Uncommented ✅
- Location: [GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp](GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp#L703)
- Code: `TheShell->push( "Menus/MainMenu.wnd" );`
- Status: **CONFIRMED** - Line exists and is NOT commented out

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
