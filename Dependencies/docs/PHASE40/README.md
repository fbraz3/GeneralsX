# Phase 40: Complete SDL2 Migration & Win32 API Removal

**Phase**: 40  
**Title**: Complete SDL2 Migration & Win32 API Removal  
**Duration**: 3-4 weeks  
**Status**: 🚀 Planned  
**Dependencies**: Phase 39 (all sub-phases complete)

---

## Overview

Phase 40 completes the cross-platform unification by making SDL2 the **only** windowing, event handling, and system abstraction layer across all platforms (Windows, macOS, Linux). All native Win32 API calls, wrapper functions, and platform-specific code paths are completely removed.

**Strategic Goal**: Single codebase that compiles identically on all three platforms without preprocessor conditionals for platform-specific window management or system APIs.

---

## Core Philosophy

- **SDL2 Only**: No DirectX, no Win32 APIs, no platform-specific conditionals
- **All gaps are OUR responsibility**: Don't defer issues to "future phases"
- **Não importa se os erros são oriundos de fases anteriores**: All Win32 debt must be cleared now
- **Fail Fast**: Identify architectural mismatches immediately, don't defer
- **Production Quality**: All code is production-grade (no temporary fixes)
- **Mark with X**: Complete each component fully before proceeding
- **Compile with `tee`**: All builds logged for analysis
- **No emojis**: Professional terminal output

---

## Current State Analysis

### Phase 39 Completion Status

From Phase 39 documentation:

- ✅ **Phase 39.2**: SDL2 event wrapper complete
- ✅ **Phase 39.3**: Vulkan graphics backend complete
- ✅ **Phase 39.4**: DirectX 8 mock layer minimized to compatibility stubs
- ✅ **Phase 39.5**: SDL2 unified across platforms (infrastructure layer)

### Remaining Win32 References

**To be addressed in Phase 40**:

1. **Native Win32 window management**
   - `hwnd` handles and related structures
   - Win32 window creation/destruction
   - Win32-specific message handling

2. **Win32 system APIs**
   - Registry operations (`RegOpenKeyEx`, `RegQueryValueEx`, etc.)
   - Path handling differences
   - Environment variables access
   - File system operations with Win32-specific behavior

3. **Platform-specific preprocessor blocks**
   - `#ifdef _WIN32` checks in game code
   - `#ifdef _APPLE` checks (should be replaced with SDL2 equivalents)
   - `#ifdef __linux__` checks (should be replaced with SDL2 equivalents)

4. **Win32-specific headers and includes**
   - `#include <windows.h>`
   - `#include <winreg.h>`
   - Platform-specific SDK headers

---

## Implementation Strategy

### Week 1: Audit & Identification

#### Day 1-2: Win32 API Audit

```bash
grep -r "#ifdef _WIN32" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_win32_ifdef.log

grep -r "#include <windows.h>" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_windows_h.log

grep -r "hwnd\|HWND\|GetModuleFileName\|RegOpenKey" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null | tee logs/phase40_win32_api.log
```

**Deliverable**: Complete inventory of Win32 API usage

#### Day 3-4: SDL2 Coverage Analysis

For each Win32 API call identified:
- Determine if SDL2 has equivalent
- Identify any gaps in SDL2 coverage
- Document migration path

**Key SDL2 APIs for Phase 40**:

- **Window management**: `SDL_CreateWindow`, `SDL_DestroyWindow`
- **Event handling**: `SDL_PollEvent` (already in Phase 39.5)
- **Timers**: `SDL_GetTicks`, `SDL_Delay`
- **File paths**: `SDL_GetBasePath`, `SDL_GetPrefPath`
- **Threading**: `SDL_CreateThread`, `SDL_WaitThread` (if needed)

#### Day 5: Registry/Configuration Migration

**Current Win32 approach**:
```cpp
// Old code (Phase 29)
RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\...", ...);
```

**SDL2 alternative**:
```cpp
// New approach
std::string config_path = SDL_GetPrefPath("SomeHouse", "GeneralsX");
std::string config_file = config_path + "generals.ini";
```

**Deliverable**: Configuration storage strategy for SDL2

---

### Week 2: Core API Replacement

#### Day 1-2: Window Management

Replace all `hwnd` usage with SDL2 window handles:

**Files to modify** (representative):
- `Generals/Code/GameEngine/GameLogic/GameClient/WinMain.cpp`
- `GeneralsMD/Code/GameEngine/...`
- `Core/GameEngine/...` (any window-related code)

**Changes**:
- Remove `HWND hwnd` parameters
- Use `SDL_Window*` instead
- Replace `CreateWindowEx` with `SDL_CreateWindow`
- Replace window proc with SDL_PollEvent loop (already done in Phase 39.5)

**Verification**: All window creation code uses SDL2 exclusively

#### Day 3-4: Registry/Configuration

Replace Windows registry access with file-based configuration:

**Strategy**:
1. Define standard config paths using `SDL_GetPrefPath`
2. Create INI file parser (or use existing if available)
3. Replace all `RegOpenKey*` calls with file I/O
4. Ensure paths are cross-platform

**Files to modify** (representative):
- Any code that reads/writes registry
- Configuration management code
- Game settings storage

**Verification**: Configuration persists correctly on all platforms

#### Day 5: Path Handling

Replace Win32 path handling with SDL2 equivalents:

**Changes**:
- `GetModuleFileName` → `SDL_GetBasePath`
- `GetCurrentDirectory` → `SDL_GetBasePath` or working directory
- Path separators: Use `/` uniformly (cross-platform)
- String paths: Use `std::string` and `std::filesystem` (C++17)

**Files to modify**:
- Asset loading code
- File path construction
- Resource location logic

**Verification**: All file paths work identically on Windows/macOS/Linux

---

### Week 3: System API Replacement

#### Day 1-2: Threading (if applicable)

If Phase 39.5 didn't fully address threading:

**Strategy**:
- Use `SDL_CreateThread` for all threading (or std::thread if C++17)
- Replace Windows `CRITICAL_SECTION` with cross-platform sync primitives
- Use `SDL_mutexes` or C++17 `std::mutex`

**Verification**: Threading works correctly on all platforms

#### Day 3-4: Timing & Sleep

Replace Win32 timing with SDL2:

**Changes**:
- `GetTickCount` → `SDL_GetTicks`
- `Sleep(ms)` → `SDL_Delay(ms)`
- `QueryPerformanceCounter` → `SDL_GetPerformanceCounter` / `SDL_GetPerformanceFrequency`

**Files affected**:
- Render loop timing
- Frame rate limiting
- Performance profiling code

**Verification**: Frame timing accurate and consistent on all platforms

#### Day 5: Keyboard/Mouse Input

Verify Phase 39.5 SDL2 event handling is complete:

- All keyboard input via SDL_KEYDOWN/SDL_KEYUP
- All mouse input via SDL_MOUSEMOTION/SDL_MOUSEBUTTONDOWN/UP
- No Win32-specific input code remaining

**Verification**: Input works identically on all platforms

---

### Week 4: Cleanup & Verification

#### Day 1-2: Preprocessor Conditional Removal

**Goal**: Zero `#ifdef _WIN32`, `#ifdef _APPLE`, `#ifdef __linux__` in game code

**Process**:
```bash
grep -r "#ifdef _WIN32" Generals/ GeneralsMD/ Core/GameEngine/ --include="*.cpp" --include="*.h" 2>/dev/null
```

For each occurrence:
- If it's SDL2 wrapper code → keep (it's in Core/Libraries)
- If it's game code → convert to SDL2 equivalent

**Exceptions**:
- Core/Libraries/Source/WWVegas/win32_compat.h (system compatibility layer - can stay)
- Build system conditionals (CMakeLists.txt - necessary)
- Platform-specific build targets (acceptable)

**Verification**: Zero platform conditionals in game code directories

#### Day 3-4: Cross-Platform Testing

Test builds on all three platforms:

**Windows**:
```bash
cmake --preset vc6  # or appropriate Windows preset
cmake --build build/windows --target z_generals -j 4 2>&1 | tee logs/phase40_win_build.log
```

**macOS ARM64**:
```bash
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase40_mac_build.log
```

**Linux**:
```bash
cmake --preset linux
cmake --build build/linux --target z_generals -j 4 2>&1 | tee logs/phase40_linux_build.log
```

**Verification checklist**:
- [ ] Zero compilation errors on all platforms
- [ ] Zero linker errors (missing Win32 APIs)
- [ ] Game executable runs without crashes
- [ ] Window created successfully
- [ ] Vulkan rendering active (verified in Phase 39)
- [ ] Input works (keyboard/mouse)
- [ ] Configuration loaded/saved correctly
- [ ] Game can load assets and maps

#### Day 5: Documentation & Final Checklist

Create COMPLETION_CHECKLIST.md documenting:
- All Win32 APIs removed
- All SDL2 migrations completed
- All platforms tested successfully
- Any workarounds or known limitations

---

## Critical Files to Address

### Game Code Directories

**Generals/Code/**:
- WinMain.cpp (window creation, main loop)
- GameLogic/ (timing, input handling)
- Libraries/ (system APIs)

**GeneralsMD/Code/**:
- Same categories as Generals/

**Core/GameEngine/**:
- Main game loop
- Event handling
- System API usage

### Core/Libraries/Source/** (Compatibility Layer)

**Files to KEEP**:
- `WWVegas/WW3D2/win32_compat.h` (system type definitions - not game code)

**Files to EVALUATE**:
- Any remaining Win32-specific wrappers
- Registry abstraction layers
- File I/O wrappers that are Win32-specific

---

## Success Criteria

### Must Have (Phase 40 Completion)

- [ ] All `hwnd` references removed from game code
- [ ] All `#include <windows.h>` removed from game code
- [ ] All `#ifdef _WIN32` removed from game code directories
- [ ] All registry operations replaced with file-based config
- [ ] All Win32 path handling replaced with cross-platform equivalents
- [ ] SDL2 used exclusively for window management, events, timers
- [ ] Game compiles cleanly on Windows, macOS, Linux
- [ ] Game runs without crashes on all three platforms
- [ ] Vulkan rendering works on all platforms (from Phase 39)
- [ ] Configuration persists across sessions
- [ ] Input handling works identically on all platforms

### Should Have

- [ ] Zero Win32 API calls in game code
- [ ] Performance equivalent to or better than Win32 version
- [ ] All assets load successfully
- [ ] All game features work identically on all platforms

### Known Limitations (Document if applicable)

- Any SDL2 APIs that don't have exact Win32 equivalents
- Platform-specific performance considerations
- Any intentional workarounds with explanations

---

## Compilation & Testing Commands

### Clean build on all platforms

```bash
# macOS ARM64
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/phase40_macos_arm64_build.log

# macOS Intel (if applicable)
cmake --preset macos-x64
cmake --build build/macos-x64 --target z_generals -j 4 2>&1 | tee logs/phase40_macos_x64_build.log

# Linux
cmake --preset linux
cmake --build build/linux --target z_generals -j 4 2>&1 | tee logs/phase40_linux_build.log

# Windows VC6
cmake --preset vc6
cmake --build build/vc6 --target z_generals -j 4 2>&1 | tee logs/phase40_windows_build.log
```

### Runtime verification

```bash
# macOS
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase40_runtime.log

# Linux
export DISPLAY=:0  # if running on display server
./GeneralsXZH 2>&1 | tee logs/phase40_runtime.log

# Windows
GeneralsXZH.exe 2>&1 | tee logs/phase40_runtime.log
```

---

## Integration with Phase 41

Upon Phase 40 completion:

- Clean SDL2-only codebase ready for Phase 41
- No Win32 API dependencies
- Game code fully compatible with pluggable graphics backends
- Foundation for abstract graphics driver layer (Phase 41)

---

## References

- Phase 39 documentation (especially Phase 39.5 SDL2 consolidation)
- SDL2 API documentation: https://wiki.libsdl.org/
- CMakeLists.txt configurations for cross-platform builds
- Existing SDL2 wrapper code in Core/Libraries

---

## Status Tracking

Phase 40 tasks to mark as complete:

- [ ] Week 1: Audit & Identification - Complete
- [ ] Week 2: Core API Replacement - Complete
- [ ] Week 3: System API Replacement - Complete
- [ ] Week 4: Cleanup & Verification - Complete
- [ ] All platforms compile successfully
- [ ] Game runs without crashes on all platforms
- [ ] Configuration properly migrated
- [ ] Documentation complete

**Next Phase**: Phase 41 - Graphics Driver Abstraction Layer

---

**Created**: November 19, 2025  
**Last Updated**: November 19, 2025  
**Status**: 🚀 Ready for Implementation
