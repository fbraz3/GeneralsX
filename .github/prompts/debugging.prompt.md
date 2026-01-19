---
name: Debug GeneralsX Crashes
description: Comprehensive guide for debugging GeneralsXZH crashes and diagnosing runtime issues on Windows MSVC 2022 builds. Covers log analysis, WinDbg debugging, memory issues, and common crash patterns.
argument-hint: "[crash|logs|memory|windbg|freeze|segfault]"
agent: agent
---

# GeneralsX Crash Debugging and Analysis

You are assisting with debugging and diagnosing crashes in GeneralsX, a C++20 game engine port running on Windows 32-bit (x86) with MSVC Build Tools 2022, SDL2 windowing, and OpenAL audio.

## Debugging Workflow

### Phase 1: Initial Crash Observation

**Symptom Categories:**
- **Immediate crash on startup** (before menu appears)
- **Crash during gameplay** (mid-game)
- **Freeze/hang** (no CPU activity visible)
- **SDL2 initialization failure** (windowing/input error)
- **Memory access violation** (segmentation fault)
- **Audio subsystem crash** (OpenAL initialization failure)

### Phase 2: Log Collection

**Step 1: Enable logging output**
```powershell
# Run with full logging
cd $env:USERPROFILE\GeneralsX\GeneralsMD
.\GeneralsXZH.exe -win -noshellmap 2>&1 | Tee-Object -FilePath debug.log

# Keep running for 60+ seconds minimum
# Game takes 20-30 seconds to load .big files and initialize graphics
```

**Step 2: Check game crash logs**
- Location: `%USERPROFILE%\Documents\Command and Conquer Generals Zero Hour Data\`
- Look for: `crash*.txt`, `error.log`, or recently modified files

**Step 3: Collect build/runtime logs**
```powershell
# Build log analysis
Get-Content logs\build_zh_msvc2022.log | Select-String -Pattern "error|warning|undefined" | Select-Object -Last 50

# Runtime log analysis
Get-Content logs\runTerminal_msvc2022.log | Select-String -Pattern "error|exception|failed" | Select-Object -Last 100
```

### Phase 3: Memory Analysis

**Check for memory-related issues in GameMemory.cpp:**
```cpp
GameMemory::isValidMemoryPointer(ptr)  // Validates pointer
GameMemory::isValidMemoryRange(ptr, size)  // Checks if range accessible
```

**Symptoms indicating memory issues:**
- "Access violation" in crash logs
- "Segmentation fault"
- Pointer arithmetic failures in legacy VC6 code
- Bounds checking failures
- Use-after-free patterns

**Common memory problem locations:**
- `Core/GameEngine/Source/Common/System/GameMemory.cpp` - Memory allocation/validation
- `Core/Libraries/Source/WWVegas/WW3D2/` - DirectX 8 memory management
- `Core/Libraries/Source/WWMath/` - Vector/matrix with manual memory
- Asset loading code - .big file parsing

### Phase 4: Component-Specific Debugging

#### SDL2 Windowing Issues
**Symptoms:**
- "SDL2 initialization failed"
- "Could not create SDL window"
- Black screen, no rendering

**Debug approach:**
1. Check SDL2 initialization in `Dependencies/Utility/Compat/win32_sdl2_compat.h`
2. Verify SDL2 linked: `cmake --build build\win32 --target z_generals --config Release --verbose 2>&1 | Select-String SDL2`
3. Check CMake logs: `Select-String "SDL2" logs\build_zh_msvc2022.log`
4. Run with debug output: `.\GeneralsXZH.exe -win -noshellmap 2>&1 | Select-String SDL`

#### OpenAL Audio Issues
**Symptoms:**
- "Failed to initialize OpenAL device"
- No audio output
- Crash when sound plays

**Debug approach:**
1. Check OpenAL device enumeration
2. Verify OpenAL-Soft library linked correctly
3. Look for audio subsystem initialization errors in logs
4. Test audio device independently if possible

#### Game Asset Loading
**Symptoms:**
- Crash after splash screen
- "Cannot find .big file"
- Missing textures/models causing crashes

**Debug approach:**
1. Verify Data folder: `Test-Path $env:USERPROFILE\GeneralsX\GeneralsMD\Data`
2. Check .big files: `Get-ChildItem $env:USERPROFILE\GeneralsX\GeneralsMD\Data\*.big`
3. Verify asset symlink if used: `cmd /c dir $env:USERPROFILE\GeneralsX\GeneralsMD\Data | findstr /i "steam"`

### Phase 5: WinDbg Debugging (Advanced)

**Using VS Code debugging task:**
```powershell
# Task: "Debug GeneralsXZH (Windows MSVC2022)"
# Output: logs\debugTerminal_msvc2022.log
```

**WinDbg common commands:**
```
g           - Go (run until breakpoint/crash)
k           - Backtrace (call stack)
dv          - Dump variables in current frame
p           - Step over
t           - Step into
!analyze -v - Automatic crash analysis
```

**Setting breakpoints:**
```
bp function_name    - Breakpoint at function
bl                  - List breakpoints
bc *                - Clear all breakpoints
```

### Phase 6: Common Crash Patterns

#### Pattern 1: Immediate Crash (Before Menu)
**Likely Causes:**
- Missing Data folder with game assets
- Corrupted .big files
- INI configuration parsing error
- SDL2 or OpenAL initialization failure
- Missing runtime dependency (fmt.dll)

**Solution Steps:**
1. Verify Data folder exists: `Test-Path $env:USERPROFILE\GeneralsX\GeneralsMD\Data`
2. Check fmt.dll copied: `Test-Path $env:USERPROFILE\GeneralsX\GeneralsMD\fmt.dll`
3. Re-run CMake configure to ensure all dependencies linked
4. Verify .big files not corrupted: compare file sizes with original

#### Pattern 2: Crash During Gameplay
**Likely Causes:**
- Memory corruption in game logic
- Pointer arithmetic errors from VC6 era code
- Uninitialized memory access
- Asset loading during runtime
- AI subsystem crash

**Solution Steps:**
1. Enable memory validation: `GameMemoryEnable` in CMake
2. Test minimal scenario: skirmish vs 1 AI, single unit
3. Check GameMemory.cpp validation logs
4. Reproduce with `-noshellmap` to skip shell map initialization

#### Pattern 3: Freeze/Hang (No Crash)
**Likely Causes:**
- Infinite loop during initialization
- Deadlock waiting for resource
- Game waiting for missing asset
- Shader compilation stuck

**Solution Steps:**
1. Check CPU usage in Task Manager while frozen
   - High CPU = infinite loop or heavy processing
   - Low CPU = waiting for I/O or lock
2. Attach WinDbg and sample thread state
3. Look for I/O operations: `Get-Process GeneralsXZH.exe`
4. Increase timeout to verify it's not just slow

#### Pattern 4: Audio Subsystem Crash
**Likely Causes:**
- OpenAL device not available
- Audio buffer format mismatch
- Invalid audio file format

**Solution Steps:**
1. Check system audio device: `mmsys.cpl` (Audio settings)
2. Verify OpenAL library: `Test-Path C:\vcpkg\installed\x86-windows\lib\OpenAL32.lib`
3. Try running without audio if optional

### Phase 7: Log Analysis Patterns

**Search for error indicators:**
```powershell
# All error lines
Get-Content logs\runTerminal_msvc2022.log | Select-String -Pattern "error|fail|exception|crash"

# Memory-related issues
Get-Content logs\build_zh_msvc2022.log | Select-String -Pattern "memory|malloc|new|delete"

# SDL2-specific issues
Get-Content logs\runTerminal_msvc2022.log | Select-String "SDL"

# Graphics/rendering issues
Get-Content logs\runTerminal_msvc2022.log | Select-String -Pattern "render|texture|shader"
```

## Debugging Best Practices

### ✅ Do This:
- **Use `-win -noshellmap` flags** for faster startup and easier debugging
- **Keep full unfiltered logs** before applying grep filters
- **Check Task Manager** while "frozen" to distinguish between slow loading and real hang
- **Wait 60+ seconds** before assuming startup crash (asset loading takes time)
- **Copy logs with timestamp** for comparison between runs
- **Test on hardware with 3D acceleration** if possible (VM limitations may hide GPU issues)

### ❌ Don't Do This:
- Don't assume crash without verifying process is still running
- Don't apply aggressive log filtering without checking full context
- Don't ignore "minor" errors in logs (may be root cause)
- Don't modify game code without identifying root cause first
- Don't use empty stubs or try/catch blocks for workarounds

## Memory Debugging Checklist

- [ ] Check `GameMemory::isValidMemoryPointer()` calls for failed validation
- [ ] Verify no buffer overflows in array access
- [ ] Check for use-after-free patterns (deleted objects still accessed)
- [ ] Verify pointer arithmetic (especially in VC6 legacy code)
- [ ] Check for uninitialized variables in struct constructors
- [ ] Verify vtable pointers valid before virtual function calls
- [ ] Check for race conditions if threading used

## Key Notes

- **Windowed mode crucial**: Use `-win` flag to prevent fullscreen lockups
- **Asset loading takes time**: Wait 30+ seconds before assuming frozen
- **Log filtering hides context**: Check full logs before filtering
- **First-time debugging**: Check logs before using WinDbg
- **Memory protection enabled**: Extensive validation in GameMemory.cpp
