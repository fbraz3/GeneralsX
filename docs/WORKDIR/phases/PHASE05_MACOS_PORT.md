# PHASE05: macOS Port (SDL3 + OpenAL + MoltenVK)

**Status**: 📋 Planning (Ready to start - Linux functional)
**Type**: Platform port (reusing Linux code)
**Created**: 2026-02-23
**Updated**: 2026-02-23 - Deep codebase audit complete (~70 locations, 8 files need changes, 40+ verified compatible)
**Prerequisites**: Linux build stable (SDL3.4 ✅, DXVK ✅, OpenAL ✅)

## Goal

Port GeneralsX/GeneralsXZH to macOS **natively** using:
- **SDL3** for windowing/input (reuse from Linux, works on macOS)
- **OpenAL** for audio (reuse from Linux, works on macOS)  
- **MoltenVK** for graphics (Vulkan → Metal bridge, 2-4 week prototype)

**Philosophy**: Don't rewrite. Reuse. Adapt. Ship.

## Progress Snapshot
🚀 Ready to start (Linux functional, all components proven)  
🎯 Estimated effort: **2-4 weeks** (not 6-12 months!)  
📚 MoltenVK is mature, used by AAA games (Dota 2, Valheim, No Man's Sky)

---

## Context

**Stack Reuse Strategy**:
```
Linux (FUNCTIONAL):              macOS (NEW):
├─ SDL3.4 ✅                      ├─ SDL3.4 (reuse, works on macOS)
├─ OpenAL ✅                      ├─ OpenAL (reuse, identical code)
├─ DXVK → Vulkan ✅             ├─ MoltenVK → Metal (new layer, thin)
├─ CompatLib POSIX ✅           ├─ CompatLib + macOS shims (adapt)
└─ CMake linux64-deploy ✅      └─ CMake macos64-moltenvk (copy+modify)
```

**Key Insight**: ~70-80% of Linux code works on macOS **right now**.

**MoltenVK Choice**:
- ✅ Vulkan API is same on Linux & macOS (via MoltenVK)
- ✅ Mature, battle-tested (AAA games use it)
- ✅ ~10-15% performance overhead (~acceptable for community port)
- ✅ DXVK doesn't need changes (Vulkan is Vulkan)
- ⏳ Future: Native Metal renderer if needed (Phase 7)

**Why Not Native Metal Now?**
- ❌ Would require 1000+ hours (complete graphics rewrite)
- ❌ No code reuse (Metal ≠ DirectX)
- ✅ MoltenVK gets us 80% there in 2-4 weeks

---

## What Actually Changes? (Codebase Audit Results)

> Deep audit performed on 2026-02-23 covering ~70 platform-specific code locations,
> all CompatLib files (28 headers, 8 sources), DXVK loading, SDL3 integration, and OpenAL.

### Files Requiring macOS-Specific Changes (8 files, ~75-100 new lines):

| # | File | Location | Issue | Fix Required |
|---|------|----------|-------|--------------|
| 1 | `CMakePresets.json` | — | No macOS preset exists | Add `macos-vulkan` preset inheriting `linux64-deploy` (~30 lines) |
| 2 | `cmake/dx8.cmake` | FetchContent block | Downloads `dxvk-native-2.6-steamrt-sniper.tar.gz` — **Linux ELF binaries only** | Add `if(APPLE)` branch: DXVK headers-only + compile DXVK from source with Meson, or provide macOS `.dylib` artifact |
| 3 | `cmake/sdl3.cmake` | ~L54 | **Hardcoded** `/usr/lib/x86_64-linux-gnu/libpng16.so.16` — breaks on macOS | Add `if(APPLE)` branch using Homebrew/system PNG paths |
| 4 | `cmake/config-build.cmake` | — | No MoltenVK detection | Add `SAGE_USE_MOLTENVK` option + `find_package(Vulkan)` for macOS |
| 5 | `GeneralsMD/Code/CompatLib/Source/module_compat.cpp` | L14 | `readlink("/proc/self/exe")` in `GetModuleFileName()` — `/proc` doesn't exist on macOS | Add `#elif defined(__APPLE__)` with `_NSGetExecutablePath()` from `<mach-o/dyld.h>` |
| 6 | `GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp` | L1319 | `readlink("/proc/self/exe")` for EXE CRC check — same `/proc` issue | Add `#elif defined(__APPLE__)` with `_NSGetExecutablePath()` |
| 7 | `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` | L337 | Loads `"libdxvk_d3d8.so"` (Linux `.so` extension) | Add `#ifdef __APPLE__` → `"libdxvk_d3d8.dylib"`, `#else` → `.so` |
| 8 | `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt` | L230 | `if(UNIX AND NOT APPLE)` **explicitly excludes** FreeType/Fontconfig from macOS | Add `$<$<PLATFORM_ID:Darwin>:Freetype::Freetype>` or use CoreText alternative |

### Files Already macOS-Compatible (Zero Changes Needed):

| File | Why It Works |
|------|-------------|
| `CompatLib/Include/memory_compat.h` | Already has `#elif __APPLE__` with `malloc_size()` vs `malloc_usable_size()` |
| `Core/GameEngine/Source/Common/ReplaySimulation.cpp` | Already has `__APPLE__` branch with `_NSGetExecutablePath()` |
| `Core/Libraries/Source/WWVegas/WWLib/bittype.h` | Already has `__APPLE__` branch for type definitions |
| `Dependencies/Utility/Utility/endian_compat.h` | Already has `__APPLE__` branch for endianness |
| `CompatLib/Include/file_compat.h` | All POSIX/C++17: `stat()`, `getcwd()`, `opendir()`, `std::filesystem` — macOS native |
| `CompatLib/Include/socket_compat.h` | POSIX sockets; `strlcpy/strlcat` declared `__attribute__((weak))` — macOS has native versions |
| `CompatLib/Include/windows_compat.h` | `#ifndef _WIN32` guards; uses `<pthread.h>`, `<unistd.h>`, `<sys/time.h>` — all macOS POSIX |
| `GeneralsMD/Code/Main/SDL3Main.cpp` | `#ifndef _WIN32`; SDL3 Vulkan init, `setenv("DXVK_WSI_DRIVER","SDL3")` — works on macOS |
| `GeneralsMD/Code/Main/LinuxStubs.cpp` | Guarded `#ifndef _WIN32` (not `#ifdef __linux__`) — compiles on macOS |
| All OpenAL files (6 files) | Standard `<AL/al.h>` / `<AL/alc.h>` — cross-platform, just link OpenAL framework |
| SDL3GameEngine (header + source) | `#ifndef _WIN32`; SDL3 cross-platform event handling |
| `CompatLib/Include/thread_compat.h` | pthreads — macOS native |
| `CompatLib/CMakeLists.txt` | `if(UNIX)` guard — CMake sets `UNIX=TRUE` on macOS |

### Platform Guard Analysis (~70 locations audited):

| Guard Pattern | Count | macOS Compatible? | Notes |
|---------------|-------|-------------------|-------|
| `#ifndef _WIN32` | ~40 | ✅ Yes | macOS doesn't define `_WIN32` — falls into these paths automatically |
| `#ifdef _UNIX` | ~30 | ✅ Yes | `cmake/config-build.cmake` defines `_UNIX` via `if(UNIX)` — macOS sets `UNIX=TRUE` |
| `#ifdef __linux__` | 3 | ❌ No | Need `#elif defined(__APPLE__)` branches (module_compat.cpp, GlobalData.cpp, memory_compat.h) |
| `if(UNIX AND NOT APPLE)` | 1 | ⚠️ Partial | WW3D2/CMakeLists.txt — intentionally excludes macOS from FreeType, needs resolution |

### DXVK Pre-Built Binary Analysis (Critical Blocker):

The Linux build fetches `dxvk-native-2.6-steamrt-sniper.tar.gz` which contains:
```
dxvk-src/
├── include/dxvk/       # Headers (d3d8.h, windows_base.h, unknwn.h) — PURE C/C++, usable on macOS ✅
├── lib/                # 64-bit Linux shared objects (.so) — ELF format, UNUSABLE on macOS ❌
└── lib32/              # 32-bit Linux shared objects (.so) — ELF format, UNUSABLE on macOS ❌
```

**macOS strategy**: Use DXVK **headers** from the tarball for type definitions (they're plain C/C++),
but compile DXVK-native **from source** using Meson to produce macOS `.dylib` files.
DXVK upstream confirms SDL3 WSI support (`-DDXVK_WSI_SDL3` in meson.build, merged via PR #4404).
However, DXVK-native on macOS with MoltenVK is **experimental** — multiple GitHub issues exist
for Apple Silicon (M1 Pro crashes, M2 Pro issues). This is the **highest risk item**.

### Total New Code: ~75-100 lines
### Total Modified Files: ~8 files
### Total Files Verified Compatible As-Is: ~40+ files
### Total Reused Architecture: **~99,000+ lines from Linux port** ✅

---

## Scope

### In Scope ✅
- **CMake preset** for macOS (`macos-vulkan`)
- **MoltenVK integration** (Vulkan → Metal bridge)
- **Library loading** for .dylib (reuse Linux pattern)
- **macOS paths** (~/Library/Application Support)
- **Universal binary** (Intel x86_64 + Apple Silicon arm64)
- **Smoke tests** (launch, menu, skirmish)
- **Documentation** (INSTALL_MACOS.md)

### Out of Scope ❌
- App Store submission (future, code signing/notarization)
- Native Metal renderer (Phase 7+)
- macOS-specific optimizations (Phase 6 polish)
- iOS/iPadOS (out of scope entirely)

### NOT Changing (Reusing from Linux)
- ✅ SDL3.4 windowing/input (works as-is)
- ✅ OpenAL audio (works as-is)
- ✅ DXVK Vulkan wrapper (works as-is)
- ✅ CompatLib POSIX shims (95% reusable)
- ✅ Game logic, physics, AI (platform-agnostic)

---

## Implementation Plan (4 Sprints)

### Sprint 1: Environment & Build Setup (Days 1-2)

**Goal**: Get macOS build environment ready, CMake preset configured, DXVK building.

> ⚠️ **Key Blocker Identified by Audit**: `cmake/dx8.cmake` fetches a pre-built DXVK tarball
> (`dxvk-native-2.6-steamrt-sniper.tar.gz`) containing **Linux-only ELF `.so` files**.
> macOS MUST compile DXVK from source using Meson, or use a headers-only approach.
>
> ⚠️ **Secondary Blocker**: `cmake/sdl3.cmake` has hardcoded Linux path
> `/usr/lib/x86_64-linux-gnu/libpng16.so.16` at ~L54 that breaks on macOS.

**Tasks**:
- [ ] 1a. Verify Vulkan SDK installed on macOS
  ```bash
  # MoltenVK comes with Vulkan SDK for macOS
  # Apple Silicon path:
  ls ~/VulkanSDK/*/macOS/lib/libMoltenVK.dylib
  # Or Homebrew:
  brew install --cask vulkan-sdk
  vulkaninfo  # Should show MoltenVK device
  ```

- [ ] 1b. Verify SDL3 builds/available on macOS
  ```bash
  # FetchContent handles it (cmake/sdl3.cmake fetches SDL3 v3.4.2)
  # BUT: must fix hardcoded Linux libpng path first
  # Option B: brew install sdl3 (if packaged)
  ```

- [ ] 1c. Verify OpenAL available on macOS
  ```bash
  # macOS has system OpenAL framework (OpenAL.framework)
  # Or for openal-soft:
  brew install openal-soft
  ```

- [ ] 1d. Fix `cmake/sdl3.cmake` — Remove hardcoded Linux paths
  ```cmake
  # CURRENT (BREAKS on macOS):
  # set(PNG_SHARED_LIBRARY "/usr/lib/x86_64-linux-gnu/libpng16.so.16")
  
  # FIX: Platform-conditional PNG path
  if(APPLE)
      # Let CMake find_package handle it, or use Homebrew
      find_package(PNG QUIET)
  elseif(UNIX)
      set(PNG_SHARED_LIBRARY "/usr/lib/x86_64-linux-gnu/libpng16.so.16" CACHE FILEPATH "" FORCE)
  endif()
  ```

- [ ] 1e. Fix `cmake/dx8.cmake` — Add macOS DXVK path
  ```cmake
  # CURRENT: Downloads pre-built Linux tarball
  # FIX: Add macOS branch
  if(APPLE)
      # Option A: Compile DXVK from source using Meson
      FetchContent_Declare(dxvk
          GIT_REPOSITORY https://github.com/doitsujin/dxvk.git
          GIT_TAG v2.6
      )
      # Build with: meson setup build --cross-file ... -Ddxvk_wsi=sdl3
      # Option B: Headers-only + pre-compiled macOS .dylib artifact
  else()
      # Existing Linux pre-built tarball path
  endif()
  ```

- [ ] 1f. Create `macos-vulkan` preset in `CMakePresets.json`
  ```json
  {
    "name": "macos-vulkan",
    "displayName": "macOS (MoltenVK + SDL3 + OpenAL)",
    "inherits": "linux64-deploy",
    "cacheVariables": {
      "CMAKE_OSX_ARCHITECTURES": "arm64;x86_64",
      "CMAKE_OSX_DEPLOYMENT_TARGET": "11.0",
      "SAGE_USE_SDL3": "ON",
      "SAGE_USE_OPENAL": "ON",
      "SAGE_USE_DX8": "OFF",
      "SAGE_USE_MOLTENVK": "ON"
    }
  }
  ```
  Note: Do NOT set `CMAKE_SYSTEM_NAME` (CMake auto-detects Darwin on macOS).
  The preset inherits `linux64-deploy` which already sets `SAGE_USE_GLM=ON`, Ninja generator, vcpkg toolchain.

- [ ] 1g. Update `cmake/config-build.cmake` to detect MoltenVK
  ```cmake
  option(SAGE_USE_MOLTENVK "Use MoltenVK for Vulkan on macOS" OFF)
  
  if(APPLE AND SAGE_USE_MOLTENVK)
      find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
      if(NOT Vulkan_FOUND)
          message(FATAL_ERROR "MoltenVK not found. Install: brew install --cask vulkan-sdk")
      endif()
  endif()
  ```

- [ ] 1h. Test CMake configuration
  ```bash
  cmake --preset macos-vulkan
  # Should succeed without errors
  # Watch for: DXVK fetch, SDL3 build, OpenAL detection
  ```

**Checkpoint**: `cmake --preset macos-vulkan` runs without errors ✓

---

### Sprint 2: Library Loading & Platform Detection (Days 3-5)

**Goal**: Adapt library loading to find .dylib files on macOS, fix `/proc/self/exe` references.

> Audit found exactly **3 source files** with `#ifdef __linux__` that need `__APPLE__` branches,
> plus **1 CMakeLists.txt** that explicitly excludes macOS from FreeType.

**Tasks**:
- [ ] 2a. Fix `dx8wrapper.cpp` L337 — Library extension for macOS
  ```cpp
  // File: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp
  // CURRENT (line 334-337):
  #ifdef _WIN32
      D3D8Lib = LoadLibrary("D3D8.DLL");
  #else
      D3D8Lib = LoadLibrary("libdxvk_d3d8.so");
  #endif
  
  // FIX:
  #ifdef _WIN32
      D3D8Lib = LoadLibrary("D3D8.DLL");
  #elif defined(__APPLE__)
      D3D8Lib = LoadLibrary("libdxvk_d3d8.dylib");
  #else
      D3D8Lib = LoadLibrary("libdxvk_d3d8.so");
  #endif
  ```

- [ ] 2b. Fix `module_compat.cpp` L14 — `GetModuleFileName()` for macOS
  ```cpp
  // File: GeneralsMD/Code/CompatLib/Source/module_compat.cpp
  // CURRENT: Uses readlink("/proc/self/exe") which doesn't exist on macOS
  
  // FIX: Add __APPLE__ branch (pattern already exists in ReplaySimulation.cpp)
  #ifdef __linux__
      ssize_t len = readlink("/proc/self/exe", buffer, size - 1);
  #elif defined(__APPLE__)
      #include <mach-o/dyld.h>
      uint32_t bufsize = (uint32_t)size;
      int ret = _NSGetExecutablePath(buffer, &bufsize);
      ssize_t len = (ret == 0) ? strlen(buffer) : -1;
  #endif
  ```

- [ ] 2c. Fix `GlobalData.cpp` L1319 — EXE CRC check for macOS
  ```cpp
  // File: GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp
  // CURRENT: readlink("/proc/self/exe") for computing executable CRC
  
  // FIX: Same _NSGetExecutablePath() pattern
  #ifdef __linux__
      ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
  #elif defined(__APPLE__)
      #include <mach-o/dyld.h>
      uint32_t bufsize = sizeof(exePath);
      int ret = _NSGetExecutablePath(exePath, &bufsize);
      ssize_t len = (ret == 0) ? strlen(exePath) : -1;
  #endif
  ```

- [ ] 2d. Decide on FreeType/Fontconfig for macOS
  ```cmake
  # File: Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt
  # CURRENT (L230): if(UNIX AND NOT APPLE) — excludes macOS
  
  # Option A: Include FreeType on macOS via vcpkg/Homebrew
  if(UNIX)
      find_package(Freetype REQUIRED)
      if(NOT APPLE)
          find_package(Fontconfig REQUIRED)  # macOS uses CoreText instead
      endif()
  endif()
  
  # Option B: Skip FreeType on macOS, use CoreText (more work, future)
  ```

- [ ] 2e. Update file paths for macOS conventions
  - Search for `~/.local/share/` or XDG paths in source code
  - Add macOS case for `~/Library/Application Support/Generals/`:
    ```cpp
    #ifdef __APPLE__
        const char* home = getenv("HOME");
        snprintf(configPath, sizeof(configPath),
                 "%s/Library/Application Support/Generals", home);
    #elif defined(_UNIX)
        // Existing Linux XDG path
    #endif
    ```

- [ ] 2f. Verify SDL3 linkage works on macOS (CMakeLists)
  - SDL3 auto-detects Cocoa backend on macOS (no code changes)
  - Check `cmake/sdl3.cmake` for correct framework linking
  - May need: `find_library(COCOA Cocoa)`, `find_library(IOKIT IOKit)`

- [ ] 2g. Verify OpenAL linkage on macOS
  - macOS has system `OpenAL.framework` (deprecated but functional)
  - Or link `openal-soft` from Homebrew
  - Audit confirmed: all 6 OpenAL source files use standard `<AL/al.h>` — zero changes needed

- [ ] 2h. Build test
  ```bash
  cmake --build build/macos-vulkan --config Release 2>&1 | tee logs/phase5_build_macos.log
  # Expected: may have warnings, but should link successfully
  # Watch for: /proc/self/exe errors, .so loading, FreeType link errors
  ```

**Checkpoint**: Binary builds without linker errors ✓

---

### Sprint 3: Testing & Validation (Days 6-8)

**Goal**: Verify game launches, menu works, basic gameplay runs.

**Tasks**:
- [ ] 3a. Smoke test: Launch game
  ```bash
  ./build/macos-vulkan/GeneralsMD/GeneralsXZH -win
  # Should show window (may be black initially)
  ```

- [ ] 3b. Smoke test: Main menu renders
  - Look for menu buttons, background
  - If black screen: Check MoltenVK error logs
  - If crashes: Collect gdb backtrace

- [ ] 3c. Smoke test: Input works
  - Keyboard: Test arrow keys, space, etc.
  - Mouse: Move mouse, click buttons
  - Should not freeze

- [ ] 3d. Smoke test: Audio
  - Start skirmish or load map
  - Listen for in-game sound effects
  - If silent: Check OpenAL initialization errors

- [ ] 3e. Smoke test: Gameplay (10 minutes)
  - Load skirmish map
  - Place units, attack
  - Should not crash
  - Performance should be playable (>30 FPS)

- [ ] 3f. Performance baseline
  ```bash
  # Run same map on Linux & macOS, compare FPS
  # Target: macOS FPS ≥ Linux FPS × 0.85 (15% overhead acceptable)
  ```

**Checkpoint**: Game launches, menu works, skirmish playable ✓

---

### Sprint 4: Packaging & Documentation (Days 9-10)

**Goal**: Create .app bundle, document installation.

**Tasks**:
- [ ] 4a. Create .app bundle structure
  ```bash
  mkdir -p GeneralsXZH.app/Contents/{MacOS,Resources,Frameworks}
  
  # Copy binary
  cp build/macos-vulkan/GeneralsMD/GeneralsXZH GeneralsXZH.app/Contents/MacOS/
  
  # Copy assets (if bundled)
  cp -r Data GeneralsXZH.app/Contents/Resources/
  ```

- [ ] 4b. Bundle necessary libraries
  ```bash
  # Copy MoltenVK, SDL3, OpenAL, DXVK
  cp /usr/local/lib/libMoltenVK.dylib GeneralsXZH.app/Contents/Frameworks/
  cp /usr/local/lib/libSDL3.dylib GeneralsXZH.app/Contents/Frameworks/
  cp /usr/local/lib/libopenal.dylib GeneralsXZH.app/Contents/Frameworks/
  # DXVK dylib (if external)
  ```

- [ ] 4c. Fix library paths with `install_name_tool`
  ```bash
  # Make app self-contained (no system dependencies)
  install_name_tool -change /usr/local/lib/libMoltenVK.dylib \
    @executable_path/../Frameworks/libMoltenVK.dylib \
    GeneralsXZH.app/Contents/MacOS/GeneralsXZH
  # Repeat for SDL3, OpenAL, etc.
  ```

- [ ] 4d. Create Info.plist
  ```bash
  cat > GeneralsXZH.app/Contents/Info.plist <<'EOF'
  <?xml version="1.0" encoding="UTF-8"?>
  <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
  <plist version="1.0">
  <dict>
      <key>CFBundleExecutable</key>
      <string>GeneralsXZH</string>
      <key>CFBundleIdentifier</key>
      <string>com.ea.generalszerohour</string>
      <key>CFBundleName</key>
      <string>Generals: Zero Hour</string>
      <key>CFBundleShortVersionString</key>
      <string>1.04</string>
      <key>CFBundleVersion</key>
      <string>2026.02</string>
      <key>NSHighResolutionCapable</key>
      <true/>
  </dict>
  </plist>
  EOF
  ```

- [ ] 4e. Test .app bundle
  ```bash
  open GeneralsXZH.app  # Should launch game via Finder
  ```

- [ ] 4f. Create installation guide (`docs/INSTALL_MACOS.md`)
  ```markdown
  # macOS Installation Guide
  
  ## Requirements
  - macOS 11.0+ (Big Sur or later)
  - Apple Silicon (M1+) or Intel Mac
  
  ## Installation
  1. Download GeneralsXZH.app
  2. Copy to Applications folder
  3. Right-click → Open (first launch, bypass Gatekeeper)
  4. Copy original game assets to ~/Library/Application Support/Generals/
  
  ## Troubleshooting
  - Black screen: Update GPU drivers, check MoltenVK
  - No audio: Check System Preferences → Sound
  - Crashes: File bug report with crash logs
  ```

- [ ] 4g. Update dev blog
  - Document Sprint results
  - List any issues found
  - Performance metrics

**Checkpoint**: .app bundle runs, documentation complete ✓

---

## Detailed Sprint Checklist

## Detailed Sprint Checklist

### Sprint 1 Checklist: Environment Setup
- [ ] 1a. Vulkan SDK installed and MoltenVK found
- [ ] 1b. SDL3 available (Homebrew or FetchContent)
- [ ] 1c. OpenAL installed (system or Homebrew)
- [ ] 1d. `cmake/sdl3.cmake` fixed — hardcoded Linux libpng path removed
- [ ] 1e. `cmake/dx8.cmake` fixed — macOS DXVK path added (source build or headers-only)
- [ ] 1f. `macos-vulkan` preset added to CMakePresets.json
- [ ] 1g. `cmake/config-build.cmake` updated with MoltenVK detection
- [ ] 1h. `cmake --preset macos-vulkan` succeeds
- [ ] ✅ **CHECKPOINT**: CMake configuration works

### Sprint 2 Checklist: Library Loading & Platform Code
- [ ] 2a. `dx8wrapper.cpp` L337 — `.dylib` extension for macOS library loading
- [ ] 2b. `module_compat.cpp` L14 — `_NSGetExecutablePath()` replaces `/proc/self/exe`
- [ ] 2c. `GlobalData.cpp` L1319 — `_NSGetExecutablePath()` for EXE CRC check
- [ ] 2d. FreeType on macOS decided (include via vcpkg or skip for CoreText)
- [ ] 2e. File paths updated for `~/Library/Application Support/`
- [ ] 2f. SDL3 linkage verified on macOS (Cocoa framework)
- [ ] 2g. OpenAL linkage verified on macOS (OpenAL.framework or openal-soft)
- [ ] 2h. `cmake --build build/macos-vulkan` produces binary
- [ ] ✅ **CHECKPOINT**: Binary links without errors

### Sprint 3 Checklist: Testing & Validation
- [ ] 3a. Game launches with `-win` flag (no crashes on startup)
- [ ] 3b. Main menu renders correctly (logo, buttons visible)
- [ ] 3c. Input works (keyboard/mouse responsive)
- [ ] 3d. Audio plays (sound effects in menu/gameplay)
- [ ] 3e. Skirmish map loads and is playable for 10 minutes
- [ ] 3f. Performance baseline: macOS FPS ≥ Linux FPS × 0.85
- [ ] 3g. Windows builds still working (VC6/Win32, no regressions)
- [ ] 3h. Linux builds still working (no accidental breakage)
- [ ] ✅ **CHECKPOINT**: Game playable, no major bugs

### Sprint 4 Checklist: Packaging & Distribution
- [ ] 4a. .app bundle directory structure created
- [ ] 4b. Binary copied to GeneralsXZH.app/Contents/MacOS/
- [ ] 4c. Libraries (MoltenVK, SDL3, OpenAL) bundled
- [ ] 4d. install_name_tool used to fix library paths
- [ ] 4e. Info.plist created with correct metadata
- [ ] 4f. .app bundle launches via Finder (double-click)
- [ ] 4g. Installation guide written (docs/INSTALL_MACOS.md)
- [ ] 4h. Dev blog updated with Phase 5 completion notes
- [ ] ✅ **CHECKPOINT**: Distribution package ready

---

## Acceptance Criteria (Phase 5 Complete)

Phase 5 is **COMPLETE** when ALL of the following are TRUE:

- [ ] **Build Success**
  - [ ] `cmake --preset macos-vulkan` configures without errors
  - [ ] `cmake --build build/macos-vulkan` produces binary
  - [ ] Binary is universal (Intel x86_64 + Apple Silicon arm64)

- [ ] **Functionality**
  - [ ] Game launches and reaches main menu
  - [ ] Main menu renders (not just black screen)
  - [ ] Buttons are clickable (input works)
  - [ ] Skirmish map loads without crashes
  - [ ] Gameplay is playable for ≥10 minutes
  - [ ] Audio plays correctly (no crackling/silence)

- [ ] **Performance**
  - [ ] FPS ≥ 80% of Linux performance (≥68 FPS if Linux is 85 FPS)
  - [ ] No stuttering or frame drops during gameplay
  - [ ] Apple Silicon and Intel paths perform similarly

- [ ] **Compatibility**
  - [ ] Tested on macOS 11.0+ (Big Sur minimum)
  - [ ] Tested on both Intel and Apple Silicon (if hardware available)
  - [ ] .app bundle launches via Finder

- [ ] **No Regressions**
  - [ ] Windows VC6 build still compiles and runs
  - [ ] Windows Win32 build still compiles and runs
  - [ ] Linux linux64-deploy build still compiles and runs
  - [ ] No new bugs introduced in existing platforms

- [ ] **Documentation**
  - [ ] `docs/INSTALL_MACOS.md` created with installation steps
  - [ ] `README.md` updated to list macOS as supported platform
  - [ ] Dev blog entry documenting Phase 5 completion

- [ ] **Distribution**
  - [ ] GeneralsXZH.app bundle created and tested
  - [ ] Libraries bundled and path-fixed with install_name_tool
  - [ ] User can download, extract, and run game without manual setup

---

## Timeline

| Sprint | Days | Focus | Deliverables |
|--------|------|-------|--------------|
| **1: Setup** | 1-2 | CMake, Vulkan SDK, environment | `macos-vulkan` preset |
| **2: Code** | 3-5 | Library loading, platform code | Binary builds |
| **3: Test** | 6-8 | Validation, smoke tests, perf | Working game on macOS |
| **4: Package** | 9-10 | .app bundle, documentation | .app & docs ready |
| **TOTAL** | ~10 days | **~40-50 hours of work** | **Playable macOS port** |

**Actual time will depend on**:
- DXVK/MoltenVK compatibility (may need patches)
- Unknown integration issues (always some surprises)
- Testing thoroughness (smoke tests vs. full QA)

---

## Risk Register (Audit-Informed)

| # | Risk | Likelihood | Impact | Evidence | Mitigation |
|---|------|-----------|--------|----------|------------|
| R1 | **DXVK-native won't compile on macOS** | High | Critical | Pre-built tarball is Linux ELF only; 56 macOS-related issues on DXVK GitHub; M1/M2 Pro crashes reported | Test DXVK Meson build with MoltenVK in Sprint 1 day 1; fallback: headers-only + stub renderer |
| R2 | MoltenVK Vulkan subset incompatible with DXVK output | Medium | High | MoltenVK doesn't support all Vulkan extensions; DXVK may use unsupported features | Run `vulkaninfo` on macOS to check extension support; test with simple Vulkan triangle first |
| R3 | `cmake/sdl3.cmake` hardcoded Linux paths break configure | **Confirmed** | Medium | Hardcoded `/usr/lib/x86_64-linux-gnu/libpng16.so.16` at ~L54 | Add `if(APPLE)` branch in Sprint 1 task 1d |
| R4 | FreeType text rendering missing on macOS | Medium | Medium | `WW3D2/CMakeLists.txt` L230: `if(UNIX AND NOT APPLE)` explicitly excludes macOS | Include FreeType via vcpkg, or implement CoreText fallback |
| R5 | `/proc/self/exe` crashes on macOS | **Confirmed** | High | 2 locations use `readlink("/proc/self/exe")`: module_compat.cpp L14, GlobalData.cpp L1319 | Replace with `_NSGetExecutablePath()` in Sprint 2 (pattern exists in ReplaySimulation.cpp) |
| R6 | Library path issues (.dylib loading) | Low | Medium | dx8wrapper.cpp L337 loads `.so` — needs `.dylib` for macOS | Trivial `#ifdef __APPLE__` fix in Sprint 2 |
| R7 | Performance unacceptable (>20% slower) | Low | Medium | MoltenVK overhead typically 10-15% | Profile in Sprint 3; Apple Silicon Metal perf is well-optimized |
| R8 | Apple Silicon (arm64) binary issues | Low | High | No arm64-specific code found in audit; SDL3/OpenAL support arm64 | Test universal binary in Sprint 3; vcpkg has arm64-osx triplet |
| R9 | Xcode/Clang version incompatibility | Low | Low | Code is C++20 compatible (Win32 preset uses MSVC 2022 C++20) | Test on Xcode 14+ (macOS 13+) |

---

## Future Work (Phase 6+)

**Phase 6: Cross-Platform Polish**
- Performance parity across all platforms
- Bug fixes from community feedback
- Multiplayer testing (cross-platform)

**Phase 7: Native Metal Renderer (Optional)**
- If MoltenVK performance is insufficient
- Direct Metal graphics backend (1000+ hours)
- Should only be considered if community demand is high

**Emerging Idea: Universal SDL3 Renderer**
- Instead of DXVK (Windows) + MoltenVK (macOS), use SDL3 Graphics API
- Would be a long-term modernization goal (not Phase 5)
- Benefit: Single graphics code path for all platforms

---

## Notes & Lessons (from Codebase Audit)

- **Reuse is key**: 70-80% of code comes from Linux port — audit confirmed ~40+ files need zero changes
- **MoltenVK is safe but DXVK compilation is the wildcard**: MoltenVK itself is mature (AAA games), but DXVK-native building on macOS is uncharted territory with known Apple Silicon issues
- **DXVK headers are platform-agnostic**: The `include/dxvk/` headers (d3d8.h, windows_base.h, unknwn.h) are pure C/C++ and compile on any platform — only the `.so` libraries are Linux-specific
- **Pre-guard pattern works**: `windows_compat.h` already uses `_MEMORYSTATUS_DEFINED` / `_IUNKNOWN_DEFINED` guards that prevent DXVK type conflicts — same pattern applies on macOS
- **`#ifndef _WIN32` is our friend**: ~40 code locations use this guard, and macOS falls into ALL of them automatically
- **`_UNIX` define already works on macOS**: `cmake/config-build.cmake` has `if(UNIX)` which is TRUE on macOS — ~30 code locations get this for free
- **Only 3 `__linux__` guards exist**: Minimal macOS-specific work (module_compat.cpp, GlobalData.cpp, memory_compat.h — and memory_compat.h is already fixed)
- **SDL3 WSI confirmed in DXVK upstream**: PR #4404 merged, meson.build has `-DDXVK_WSI_SDL3` — DXVK native can use SDL3 for window system integration on macOS
- **OpenAL audit was cleanest**: All 6 files use standard `<AL/al.h>` / `<AL/alc.h>` with zero platform specifics — truly cross-platform
- **The `strlcpy` weak symbol trick is clever**: `socket_compat.h` declares `strlcpy`/`strlcat` with `__attribute__((weak))` — macOS has these natively, so the weak symbols gracefully defer to the native implementation
- **Performance overhead acceptable**: 10-15% is fine for community port
- **Test on real hardware**: Simulators and remote VMs may hide issues

---

## Status Tracking

**Checklists to Complete**:
- [ ] A. Sprint 1: Environment Setup (Days 1-2)
- [ ] B. Sprint 2: Library Loading & Code (Days 3-5)
- [ ] C. Sprint 3: Testing & Validation (Days 6-8)
- [ ] D. Sprint 4: Packaging & Documentation (Days 9-10)

**Overall Progress**: 0/4 sprints complete
