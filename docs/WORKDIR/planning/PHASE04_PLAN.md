# PHASE 04: Timing & OS Services - Planning Document

**Objective**: Implement SDL2-based timing system and cross-platform OS service abstraction layer.

**Focus**: Windows testing (VC6), cross-platform architecture ready  
**Scope**: Timing, window management, clipboard, file system abstraction

---

## Win32 API Audit Results

### Timing APIs
| Win32 API | Usage | Replacement |
|-----------|-------|-------------|
| `GetTickCount()` | Frame timing, performance measurement | `SDL_GetTicks()` |
| `Sleep(ms)` | Thread sleep, frame pacing | `SDL_Delay()` |
| `SetTimer()` | Window timer callbacks (mostly tools) | `SDL_AddTimer()` or event-driven |
| `KillTimer()` | Kill window timers | `SDL_RemoveTimer()` |
| `QueryPerformanceCounter()` | High-res timing (not heavily used) | `SDL_GetPerformanceCounter()` |
| `timeGetTime()` | Alternative timing (in timingTest tool) | `SDL_GetTicks()` |

**Usage Frequency**:
- ✅ HIGH: `GetTickCount()` - Frame sync, performance tracking
- ✅ MEDIUM: `Sleep()` - Frame pacing, startup delays
- ⚠️ LOW: `SetTimer/KillTimer` - Tools mainly, game can use event loop instead

### Window Management APIs
| Win32 API | Usage | Replacement |
|-----------|-------|-------------|
| `ShowWindow()` | Show/hide window, minimize | `SDL_ShowWindow()` |
| `GetWindowRect()` | Get window bounds | `SDL_GetWindowSize()`, `SDL_GetWindowPosition()` |
| `GetClientRect()` | Get client area | `SDL_GetWindowSize()` (without title/borders) |
| `SetWindowPos()` | Position/resize/z-order | `SDL_SetWindowPosition()`, `SDL_SetWindowSize()` |
| `CreateWindow()` | Create window | Already using SDL2 (from PHASE 02) |

**Usage Frequency**:
- ⚠️ LOW-MEDIUM: Tools (WorldBuilder, etc) - mainly UI positioning
- ✅ MEDIUM: Game windowing - resizing, fullscreen transitions
- ⚠️ LOW: Z-order manipulation - not critical for primary game

### Clipboard APIs
| Win32 API | Usage | Replacement |
|-----------|-------|-------------|
| `OpenClipboard()` | Open clipboard | `SDL_SetClipboardText()` / `SDL_GetClipboardText()` |
| `CloseClipboard()` | Close clipboard | (automatic with SDL2) |
| `GetClipboardData()` | Get clipboard data | `SDL_GetClipboardText()` |
| `SetClipboardData()` | Set clipboard data | `SDL_SetClipboardText()` |

**Usage Frequency**:
- ❌ NONE: Clipboard not used in game code (only tools)
- Status: Can be stubbed for game, implement if needed for mod tools

### File Operations
| Win32 API | Usage | Replacement |
|-----------|-------|-------------|
| `CreateFile()` | Open/create file | `TheFileSystem` (use existing abstraction) |
| `ReadFile()` | Read from file | `TheFileSystem::Read()` |
| `WriteFile()` | Write to file | `TheFileSystem::Write()` |
| `CreateFileA()` | ANSI file operations | Use UTF-8 wrapper in `TheFileSystem` |

**Usage Frequency**:
- ✅ HIGH: All file I/O should go through `TheFileSystem`
- Status: Most game code already uses abstraction, tools use direct Win32

**Action**: Audit `TheFileSystem` implementation for cross-platform readiness (case sensitivity, path separators, etc)

---

## PHASE 04 Tasks

### Task 1: SDL2 Timing System Implementation

**Files to Create**:
- `Core/GameEngine/Include/Common/GameTiming.h`
- `Core/GameEngine/Source/Common/GameTiming.cpp`

**Implementation Details**:

1. **SDL_GetTicks() Wrapper**
   - Replace `GetTickCount()` calls
   - Returns milliseconds since SDL initialization
   - Platform-independent, works on Windows/macOS/Linux

2. **GameTiming Class**
   - Static interface: `TheGameTiming` singleton
   - Methods:
     - `getTicks()` → Current milliseconds
     - `getFrameTime()` → Milliseconds since last frame
     - `sleep(ms)` → Frame pacing
     - `getPerformanceCounter()` → Optional high-res timing

3. **Frame Timing Integration**
   - Track last frame time
   - Calculate delta time
   - Maintain frame rate consistency

**Success Criteria**:
- ✓ All `GetTickCount()` replaced with `GameTiming::getTicks()`
- ✓ Frame delta time calculated correctly
- ✓ No platform-specific calls in game code
- ✓ Compiles on Windows VC6

**Estimated Effort**: 3-4 hours

### Task 2: Window Management Layer

**Files to Create**:
- `Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Window.h`
- `Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Window.cpp`

**Implementation Details**:

1. **SDL2Window Class**
   - Wrapper around SDL_Window
   - Methods:
     - `getWidth()` / `getHeight()`
     - `setSize(w, h)`
     - `getPosition()` / `setPosition(x, y)`
     - `setFullscreen(bool)`
     - `minimize()` / `maximize()` / `restore()`
     - `setTitle()`
     - `show()` / `hide()`

2. **Integration Points**
   - Attach to existing SDL2GameEngine window
   - Provide interface for UI/rendering systems
   - Event handling for window resize, minimize, etc.

3. **Platform-Specific Handling**
   - Windows: Direct SDL2 calls
   - macOS/Linux: Same SDL2 calls (already cross-platform)

**Success Criteria**:
- ✓ Window resize/move operations work
- ✓ Fullscreen toggle works
- ✓ Minimize/maximize work
- ✓ All operations use SDL2, no Win32 calls

**Estimated Effort**: 2-3 hours

### Task 3: File System Abstraction Review

**Objective**: Audit existing `TheFileSystem` and improve for cross-platform compatibility

**Analysis**:
- Review `Core/GameEngine/Include/*/FileSystem.h`
- Check path handling: separators (`/` vs `\`), case sensitivity
- Verify `.big` file handling works on case-sensitive systems
- Ensure INI parser handles different line endings (CRLF vs LF)

**Expected Improvements**:
1. **Path Normalization**
   - Convert `\` to `/` internally
   - Consistent path representation
   
2. **Case-Insensitive Lookups** (Windows) / Case-Sensitive (Linux/macOS)
   - Implement proper lookup strategies
   - Document platform-specific behavior

3. **Big File System**
   - Verify memory mapping works cross-platform
   - Test offset calculations on 64-bit systems

**Success Criteria**:
- ✓ Path handling documented
- ✓ Case sensitivity strategy defined
- ✓ Line ending handling verified
- ✓ No hard-coded Windows paths

**Estimated Effort**: 2-3 hours (mostly analysis)

### Task 4: Clipboard & File Dialogs (Optional/Stub)

**Files to Create**:
- `Core/GameEngineDevice/Include/SDL2Device/GameClient/SDL2Clipboard.h`
- `Core/GameEngineDevice/Source/SDL2Device/GameClient/SDL2Clipboard.cpp`

**Implementation Details**:

1. **SDL2Clipboard Class** (if needed for mods)
   - `setText(const UnicodeString&)` → Copy to clipboard
   - `getText()` → Paste from clipboard
   - Use `SDL_SetClipboardText()` / `SDL_GetClipboardText()`

2. **File Dialogs** (Stub/Future)
   - Not used by game engine
   - Used by tools (WorldBuilder, W3DView)
   - Can implement using native OS dialogs or defer to tools

**Status**: OPTIONAL - Can be deferred if not blocking main game

**Success Criteria**:
- ✓ Clipboard works for text copy/paste (if implemented)
- ✓ No Win32 clipboard API calls
- ✓ Graceful fallback if no clipboard available

**Estimated Effort**: 1-2 hours

---

## Implementation Order

1. **Task 1** (GameTiming) - CRITICAL
   - Required for frame sync
   - Blocks most other timing work
   - ~3-4 hours

2. **Task 3** (FileSystem Audit) - PARALLEL with Task 1
   - Documentation/analysis focused
   - Can happen simultaneously
   - ~2-3 hours

3. **Task 2** (Window Management) - AFTER Task 1
   - Depends on framework understanding
   - ~2-3 hours

4. **Task 4** (Clipboard/Dialogs) - OPTIONAL
   - Can be deferred
   - ~1-2 hours if needed

**Total Estimated Effort**: 8-12 hours (with Task 4) or 7-10 hours (without)

---

## Integration with Existing Code

### Current Architecture
```
SDL2GameEngine (PHASE 02) 
├── SDL2Mouse (PHASE 03 Task 1)
├── SDL2Keyboard (PHASE 03 Task 2)  
├── SDL2IMEManager (PHASE 03 Task 3)
└── [PHASE 04 Services needed here]
    ├── GameTiming (frame sync)
    ├── SDL2Window (window ops)
    └── Clipboard (optional)
```

### Integration Points
1. **GameEngine Loop**
   - Add `GameTiming::getTicks()` for frame delta
   - Use `GameTiming::sleep()` for frame pacing
   - Call in `serviceSDL2OS()` or frame handler

2. **Window Events**
   - SDL2Window handles resize/move events
   - Integrate with rendering system
   - Update UI layout on resize

3. **File System**
   - Review existing code
   - Add path normalization if needed
   - Test on case-sensitive systems

---

## Testing Strategy

### Windows (VC6) Testing
1. Frame timing accuracy (compare with old GetTickCount)
2. Window resize/move operations
3. File system path handling
4. Clipboard operations (if implemented)

### Validation Checklist
- ✓ Frame rate stable (no stuttering from timing)
- ✓ Window operations responsive
- ✓ No Win32 API calls in new code
- ✓ Code compiles cleanly on VC6

### Documentation Output
- `PHASE04_TASK1_GAMETIMING_COMPLETE.md` (after implementation)
- `PHASE04_FILESYSTEM_AUDIT.md` (analysis results)
- `PHASE04_WINDOW_MANAGEMENT_COMPLETE.md` (after Task 2)
- Dev diary entry with session summary

---

## Decision Points

### If LZHL Fixed Before Task 1 Complete
→ Switch to PHASE 03 Task 4 (Integration Testing)

### If Compilation Issues Arise
→ Debug, fix root cause, document in dev diary

### If FileSystem Needs Major Refactoring
→ Create separate epic, prioritize for Quinary goals

---

## Next Actions

1. Start Task 1: Create GameTiming class with SDL_GetTicks()
2. Start parallel Task 3: Audit FileSystem implementation
3. Complete Task 2: Implement window management layer
4. Defer Task 4: Optional, can be added later
5. Document all changes and commit to git
