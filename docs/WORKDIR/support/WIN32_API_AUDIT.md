# Win32 API Audit - Migration to SDL3

**Date**: 2026-02-10  
**Goal**: Identify Win32 APIs that can/should be migrated to SDL3 for Linux port

---

## Executive Summary

✅ **SDL3 Migration Score**: 45% complete  
⚠️ **Critical APIs Still Using Win32**: Window messaging, mouse capture, keyboard polling  
🎯 **Target for Phase 1**: Migrate GameEngineDevice layer to SDL3  

---

## Category 1: Window Management APIs

### 🔴 HIGH PRIORITY - MUST MIGRATE

| API | Location | SDL3 Replacement | Status |
|-----|----------|------------------|--------|
| `CreateWindow()` | `WinMain.cpp:728` | `SDL_CreateWindow()` | ✅ **DONE** (SDL3Main.cpp uses SDL) |
| `RegisterClass()` | `WinMain.cpp:705` | N/A (SDL handles) | ✅ **DONE** |
| `GetMessage()` | `Win32GameEngine.cpp:147` | `SDL_PollEvent()` | ⚠️ **TODO** |
| `PeekMessage()` | `Win32GameEngine.cpp:143` | `SDL_PollEvent()` | ⚠️ **TODO** |
| `DispatchMessage()` | `Win32GameEngine.cpp:164` | N/A (SDL handles) | ⚠️ **TODO** |
| `TranslateMessage()` | `Win32GameEngine.cpp:163` | N/A (SDL handles) | ⚠️ **TODO** |
| `DefWindowProc()` | `WinMain.cpp:681,685` | N/A (SDL handles) | ✅ **DONE** (WinMain deprecated) |

**Analysis**:
- ✅ **SDL3Main.cpp** already uses `SDL_CreateWindow()` - correct!
- ⚠️ **Win32GameEngine.cpp** still uses Win32 message loop - needs migration to `SDL_PollEvent()`
- 🎯 **Action**: Replace Win32GameEngine's message loop with SDL3 event loop (see SDL3GameEngine.cpp as reference)

---

## Category 2: Mouse APIs

### 🟡 MEDIUM PRIORITY - MIGRATE IN PHASE 1

| API | Location | SDL3 Replacement | Status |
|-----|----------|------------------|--------|
| `GetCursorPos()` | `Win32DIMouse.cpp:390`<br>`W3DMouse.cpp:505`<br>`wnd_compat.cpp:73` | `SDL_GetMouseState()` | ⚠️ **PARTIAL** (compat stub exists) |
| `SetCursorPos()` | `Win32DIMouse.cpp:354,446` | `SDL_WarpMouseInWindow()` | ⚠️ **TODO** |
| `ShowCursor()` | `Win32DIMouse.cpp:333,355`<br>`W3DMouse.cpp:116,402,423,499` | `SDL_ShowCursor()` | ⚠️ **TODO** |
| `SetCapture()` | `Win32DIMouse.cpp:496` | `SDL_CaptureMouse()` | ⚠️ **TODO** |
| `ReleaseCapture()` | `Win32DIMouse.cpp:506` | `SDL_CaptureMouse(SDL_FALSE)` | ⚠️ **TODO** |
| `ClipCursor()` | `Win32DIMouse.cpp:424`<br>`Win32Mouse.cpp:454,467` | `SDL_SetWindowMouseGrab()` | ⚠️ **TODO** |

**Analysis**:
- ⚠️ **Win32DIMouse.cpp** and **Win32Mouse.cpp** use DirectInput + Win32 APIs
- 🎯 **Action**: Create **SDL3Mouse.cpp** to replace Win32DIMouse/Win32Mouse (similar to existing SDL3GameEngine pattern)
- ℹ️ **Note**: `wnd_compat.cpp:73` has stub `GetCursorPos()` - this is **correct** for Core libraries that can't use SDL3!

---

## Category 3: Keyboard APIs

### 🟢 LOW PRIORITY - WORKS VIA WIN32 COMPAT

| API | Location | SDL3 Replacement | Status |
|-----|----------|------------------|--------|
| `GetKeyState()` | `Win32DIKeyboard.cpp:352,427` | `SDL_GetKeyboardState()` | ⚠️ **TODO** |
| `GetAsyncKeyState()` | `WorldBuilder/WorldBuilder.cpp:568,571,574`<br>Various tools | `SDL_GetKeyboardState()` | 🟢 **DEFER** (Tools only, not runtime) |

**Analysis**:
- ⚠️ **Win32DIKeyboard.cpp** uses DirectInput + Win32 APIs for caps lock detection
- 🎯 **Action**: Migrate to `SDL_GetKeyboardState()` + `SDL_GetModState()` for modifier keys
- 🟢 **Tools** (WorldBuilder, GUIEdit) use Win32 APIs but are editor-only - defer to Phase 2+

---

## Category 4: Timing APIs

### ✅ ALREADY MIGRATED - NO ACTION NEEDED

| API | Location | POSIX/SDL3 Replacement | Status |
|-----|----------|------------------------|--------|
| `timeGetTime()` | `systimer.h:136`<br>`windows_compat.h:118` | `gettimeofday()` | ✅ **DONE** (compat layer) |
| `timeBeginPeriod()` | Various | NOOP (Linux kernel controls) | ✅ **DONE** (compat layer) |
| `timeEndPeriod()` | Various | NOOP | ✅ **DONE** (compat layer) |
| `Sleep()` | Various | `usleep()` | ✅ **DONE** (compat layer) |
| `GetCurrentThreadId()` | `wwmemlog.cpp:399`<br>`windows_compat.h:106` | `pthread_self()` | ✅ **DONE** (compat layer) |

**Analysis**:
- ✅ These APIs are used in **Core libraries** (WWLib, WWDebug) that cannot depend on SDL3
- ✅ POSIX compat layer in `windows_compat.h` is the **correct** approach
- ℹ️ **Why not SDL3?** Core libraries compile **before** GameEngine (no SDL3 dependency available)

---

## Category 5: DirectX APIs (Already Using DXVK)

### ✅ CORRECT ARCHITECTURE - NO MIGRATION NEEDED

| API | Location | Strategy | Status |
|-----|----------|----------|--------|
| `Direct3D8` interfaces | `dx8wrapper.cpp`, DXVK headers | DXVK translates DX8→Vulkan | ✅ **CORRECT** |
| `d3d8.dll` loading | `dx8wrapper.cpp:LoadLibrary` | Loads `libdxvk_d3d8.so` on Linux | ✅ **DONE** |

**Analysis**:
- ✅ DXVK approach is **correct** - maintains Windows compatibility while enabling Linux
- ℹ️ No SDL3 migration needed here (SDL3 handles window/context, DXVK handles rendering)

---

## Migration Priority Matrix

### 🔴 Phase 1 (CRITICAL - Required for Linux Build)

1. **Win32GameEngine → SDL3GameEngine message loop**
   - File: `GameEngineDevice/Source/Win32Device/Common/Win32GameEngine.cpp`
   - Replace: `GetMessage/PeekMessage/DispatchMessage` → `SDL_PollEvent()`
   - Reference: `SDL3GameEngine.cpp` already implements this correctly
   - **Impact**: HIGH - Required for Linux event handling

2. **Win32Mouse → SDL3Mouse**
   - Files: `Win32DIMouse.cpp`, `Win32Mouse.cpp`
   - Replace: `GetCursorPos/SetCursorPos/SetCapture/ClipCursor` → SDL3 equivalents
   - **Impact**: HIGH - Required for Linux input

3. **Win32Keyboard → SDL3Keyboard**
   - File: `Win32DIKeyboard.cpp`
   - Replace: `GetKeyState` → `SDL_GetKeyboardState()`
   - **Impact**: MEDIUM - Required for keyboard input on Linux

### 🟡 Phase 2 (ENHANCEMENT - Improve Cross-Platform)

4. **Win32 display/window management**
   - Abstract window creation/management via GameClient interfaces
   - Keep Win32 path for Windows builds, use SDL3 for Linux

### 🟢 Phase 3+ (OPTIONAL - Tools/Editors)

5. **WorldBuilder/GUIEdit tool Win32 APIs**
   - Defer - these are Windows-only tools for now
   - Consider Qt/wxWidgets migration if cross-platform tools needed

---

## Architecture Notes

### ✅ What's Already Correct

1. **Core libraries using POSIX** (`windows_compat.h`)
   - ✅ Correct! Core can't depend on SDL3
   - ✅ `timeGetTime()`, `Sleep()`, `GetCurrentThreadId()` via POSIX is proper layering

2. **SDL3Main.cpp as new entry point**
   - ✅ Correct! Replaces WinMain.cpp for Linux builds
   - ✅ Uses `SDL_CreateWindow()` + DXVK environment setup

3. **DXVK for DirectX translation**
   - ✅ Correct! No need to migrate to SDL_Renderer (DXVK maintains DX8 API)

### ⚠️ What Needs Migration

1. **Win32GameEngine.cpp** - Still uses Win32 message loop
   - Blocks Linux builds from processing events correctly
   - **Solution**: Use SDL3GameEngine for Linux builds (via factory pattern)

2. **Win32DIMouse.cpp / Win32Mouse.cpp** - DirectInput + Win32 cursor APIs
   - Blocks Linux mouse input
   - **Solution**: Create SDL3Mouse.cpp (similar to SDL3GameEngine pattern)

3. **Win32DIKeyboard.cpp** - DirectInput keyboard
   - Blocks Linux keyboard input
   - **Solution**: Create SDL3Keyboard.cpp

---

## Recommended Actions

### Immediate (This Session)

1. ✅ Continue fixing DXVK d3d8types.h integration (in progress)
2. ⚠️ Document that Win32GameEngine needs SDL3 migration (Phase 1.5)

### Phase 1.5 (Before Testing)

1. Implement **SDL3 event loop** in SDL3GameEngine (replace Win32GameEngine for Linux)
2. Implement **SDL3Mouse** (replace Win32DIMouse for Linux)
3. Implement **SDL3Keyboard** (replace Win32DIKeyboard for Linux)

### Build System Integration

```cmake
if(SAGE_USE_SDL3)
    # Use SDL3 implementations
    add_sources(SDL3GameEngine.cpp SDL3Mouse.cpp SDL3Keyboard.cpp)
else()
    # Use Win32 implementations
    add_sources(Win32GameEngine.cpp Win32DIMouse.cpp Win32DIKeyboard.cpp)
endif()
```

---

## Conclusion

**Migration Progress**: 45% complete  
- ✅ Entry point (SDL3Main.cpp)  
- ✅ Core timing/threading (POSIX compat)  
- ✅ DirectX rendering (DXVK)  
- ⚠️ Event loop (still Win32)  
- ⚠️ Mouse/Keyboard input (still Win32)  

**Next Steps**: After current build succeeds, implement SDL3 input layer (Phase 1.5)

---

**Generated by**: Bender's Win32 API Scanner 🤖  
**Note**: "Bite my shiny metal ass if you disagree with these recommendations!"
