# PHASE05: macOS Port (MoltenVK)

**Status**: 📋 Not Started (Blocked by Phase 4)
**Type**: Platform port (reusing Linux code)
**Created**: 2026-02-08
**Prerequisites**: Phase 1-4 complete (Linux stable, Windows compatible)

## Goal

Port the Linux build to macOS using MoltenVK, reusing ~70-80% of the Linux codebase. Focus on minimal effort, maximum reusability.

## Progress Snapshot
⏳ Blocked by Phase 4 (Linux must be stable first)  
🎯 Estimated effort: 20-40 hours (not 100-200!)  
📚 MoltenVK is mature, used by AAA games (Dota 2, Valheim, No Man's Sky)

---

## Context

**Why macOS After Linux?**
- ✅ SDL3: Cross-platform (same API on Linux/macOS)
- ✅ Vulkan: MoltenVK implements Vulkan 1.1+ → Metal
- ✅ OpenAL: Cross-platform (same API on Linux/macOS)
- ✅ DXVK: Should compile for macOS with minor changes
- ✅ ~70-80% code reuse from Linux port

**Why Not Native Metal?**
- ❌ Would require 1000+ hours (complete rewrite of renderer)
- ❌ macOS-only code (not reusable for Linux)
- ✅ MoltenVK performance is acceptable (~10-20% overhead)
- ✅ MoltenVK is mature and officially supported by Khronos

**Performance Expectations**:
```
Windows (DX8 native)      → 100 FPS (baseline)
Linux (DXVK)              →  90 FPS (10% overhead)
macOS (DXVK+MoltenVK)     →  85 FPS (15% overhead)
```

This is **acceptable** for a community port.

---

## Scope

### In Scope ✅
- CMake preset for macOS (`macos-vulkan`)
- Adapt library loading (`.so` → `.dylib`)
- MoltenVK integration (link against SDK)
- Universal binary (Intel + Apple Silicon)
- Basic testing (launch, menu, skirmish)
- Documentation (`INSTALL_MACOS.md`)

### Out of Scope ❌
- Native Metal renderer (future optimization, if ever)
- macOS-specific optimizations (Phase 6+)
- iOS/iPadOS ports (different scope)
- App Store submission (future, requires code signing)

---

## Implementation Plan

### A. Prerequisites Check

**Before starting Phase 5**:
- [ ] Phase 1-4 complete (Linux port stable)
- [ ] No P0/P1 bugs on Linux
- [ ] Performance on Linux acceptable (<15% overhead vs Windows)
- [ ] Windows builds still working (VC6/Win32)
- [ ] Community feedback positive (>10 Linux beta testers)

**Developer Environment**:
- [ ] macOS 11.0+ (Big Sur or later)
- [ ] Vulkan SDK installed (`brew install --cask vulkan-sdk`)
- [ ] MoltenVK confirmed working (`vulkaninfo` shows MoltenVK)
- [ ] SDL3 installed (`brew install sdl3` or build from source)
- [ ] OpenAL installed (`brew install openal-soft`)

### B. CMake Configuration (Estimated: 2-4 hours)

**Create macOS preset** in `CMakePresets.json`:
```json
{
  "name": "macos-vulkan",
  "displayName": "macOS (MoltenVK + SDL3)",
  "inherits": "linux64-deploy",
  "description": "Native macOS build using MoltenVK for Vulkan",
  "cacheVariables": {
    "CMAKE_SYSTEM_NAME": "Darwin",
    "CMAKE_OSX_ARCHITECTURES": "arm64;x86_64",
    "CMAKE_OSX_DEPLOYMENT_TARGET": "11.0",
    "SAGE_USE_SDL3": "ON",
    "SAGE_USE_OPENAL": "ON",
    "SAGE_USE_MOLTENVK": "ON"
  }
}
```

**Update `cmake/config-build.cmake`**:
```cmake
option(SAGE_USE_MOLTENVK "Use MoltenVK for Vulkan on macOS" OFF)

if(APPLE AND SAGE_USE_MOLTENVK)
    # Find MoltenVK from Vulkan SDK
    find_package(Vulkan REQUIRED)
    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan SDK not found. Install: brew install --cask vulkan-sdk")
    endif()
    
    target_link_libraries(GameEngineDevice PRIVATE Vulkan::Vulkan)
    message(STATUS "Using MoltenVK for Vulkan on macOS")
endif()
```

**Update `cmake/dx8.cmake`**:
```cmake
# Platform-specific Vulkan loader
if(UNIX AND NOT APPLE)
    # Linux: Use system libvulkan.so
    set(VULKAN_LOADER "libvulkan.so.1")
elseif(APPLE)
    # macOS: Use MoltenVK from Vulkan SDK
    set(VULKAN_LOADER "libMoltenVK.dylib")
endif()
```

### C. Library Loading Adaptation (Estimated: 4-8 hours)

**Update `dx8wrapper.cpp`** (already has Linux ifdef, extend to macOS):
```cpp
#ifdef _WIN32
    D3D8Lib = LoadLibrary("D3D8.DLL");
#elif __APPLE__
    D3D8Lib = LoadLibrary("libdxvk_d3d8.dylib");
#else
    D3D8Lib = LoadLibrary("libdxvk_d3d8.so");
#endif
```

**Verify DXVK builds for macOS**:
- [ ] Check if dxvk-native supports macOS (likely needs minor patches)
- [ ] Build DXVK for macOS (`cmake --preset macos` in dxvk-native repo)
- [ ] Test loading `libdxvk_d3d8.dylib` manually
- [ ] If DXVK doesn't build: Investigate required patches (likely minimal)

**SDL3 windowing** (already portable, verify):
- [ ] Confirm `SDL_CreateWindow()` with `SDL_WINDOW_VULKAN` works on macOS
- [ ] Confirm `SDL_Vulkan_LoadLibrary()` loads MoltenVK correctly
- [ ] Test event loop on macOS (keyboard/mouse input)

**OpenAL audio** (already portable, verify):
- [ ] Confirm `alcOpenDevice()` works on macOS
- [ ] Test audio playback with OpenAL Soft or system OpenAL
- [ ] Verify source pooling and buffer management

### D. macOS-Specific Paths (Estimated: 2-4 hours)

**Environment variable setup** (same as Linux, verify):
```cpp
// SDL3Main.cpp (already has this for Linux)
#ifndef _WIN32
    setenv("DXVK_WSI_DRIVER", "SDL3", 1);
    // macOS may need additional env vars, test and document
#endif
```

**File paths** (case-sensitive on APFS, same as Linux):
- [ ] Verify case-insensitive lookup works (Phase 4 implementation reusable)
- [ ] Test with game assets (Data/Audio/, Data/Maps/, etc.)

**Home directory**:
```cpp
// macOS: ~/Library/Application Support/Generals/
// Linux: ~/.local/share/Generals/
#ifdef __APPLE__
    const char* home = getenv("HOME");
    snprintf(configPath, sizeof(configPath), "%s/Library/Application Support/Generals", home);
#else
    // Linux path
#endif
```

### E. Universal Binary (Intel + Apple Silicon) (Estimated: 2-4 hours)

**CMake configuration** (already in preset):
```cmake
CMAKE_OSX_ARCHITECTURES = "arm64;x86_64"
```

**Test both architectures**:
- [ ] Build universal binary: `cmake --build build/macos-vulkan`
- [ ] Test on Intel Mac (if available, or VM)
- [ ] Test on Apple Silicon Mac (M1/M2/M3)
- [ ] Verify `lipo -info GeneralsXZH` shows both architectures

**Performance profiling**:
- [ ] Compare Intel vs Apple Silicon performance
- [ ] Profile with Xcode Instruments (GPU, CPU, memory)
- [ ] Document any architecture-specific quirks

### F. Packaging & Distribution (Estimated: 4-8 hours)

**Option 1: .app Bundle** (recommended for macOS):
```bash
# Create .app bundle structure
mkdir -p GeneralsXZH.app/Contents/{MacOS,Resources,Frameworks}

# Copy binary
cp build/macos-vulkan/GeneralsXZH GeneralsXZH.app/Contents/MacOS/

# Bundle libraries (MoltenVK, SDL3, OpenAL, DXVK)
cp /path/to/libMoltenVK.dylib GeneralsXZH.app/Contents/Frameworks/
cp /path/to/libSDL3.dylib GeneralsXZH.app/Contents/Frameworks/
cp /path/to/libopenal.dylib GeneralsXZH.app/Contents/Frameworks/
cp /path/to/libdxvk_d3d8.dylib GeneralsXZH.app/Contents/Frameworks/

# Fix library paths with install_name_tool
install_name_tool -change ...

# Create Info.plist
cat > GeneralsXZH.app/Contents/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>GeneralsXZH</string>
    <key>CFBundleIdentifier</key>
    <string>com.ea.generalszerohour</string>
    <key>CFBundleName</key>
    <string>Generals Zero Hour</string>
    <key>CFBundleVersion</key>
    <string>1.04</string>
</dict>
</plist>
EOF
```

**Option 2: .dmg Installer**:
```bash
# Create .dmg with app bundle + README
hdiutil create -volname "Generals Zero Hour" -srcfolder GeneralsXZH.app -ov -format UDZO GeneralsXZH-v1.0-macOS.dmg
```

**Code signing** (optional, future):
- [ ] Not required for initial release
- [ ] Future: Sign with Apple Developer certificate for Gatekeeper
- [ ] Future: Notarize for macOS 10.15+

### G. Testing & Validation (Estimated: 4-8 hours)

**Smoke tests** (comprehensive):
- [ ] Launch game on Intel Mac (if available)
- [ ] Launch game on Apple Silicon Mac (M1/M2/M3)
- [ ] Navigate main menu (all buttons, settings)
- [ ] Start skirmish match (load map, units respond, audio works)
- [ ] Play for 10 minutes (no crashes, performance acceptable)
- [ ] Exit game gracefully

**Compatibility testing**:
- [ ] macOS 11.0 (Big Sur, minimum supported)
- [ ] macOS 12.0 (Monterey)
- [ ] macOS 13.0 (Ventura)
- [ ] macOS 14.0 (Sonoma, latest)
- [ ] Multiple GPUs: Intel integrated, AMD discrete, Apple Silicon GPU

**Performance benchmarks**:
- [ ] Measure FPS (same map as Linux/Windows tests)
- [ ] Compare to Linux performance (should be similar, ~5-10% difference)
- [ ] Profile with Instruments (identify bottlenecks)
- [ ] Target: >80% of Windows performance

### H. Documentation (Estimated: 2-4 hours)

**Create `docs/INSTALL_MACOS.md`**:
```markdown
# macOS Installation

## Requirements
- macOS 11.0+ (Big Sur or later)
- Intel Mac or Apple Silicon (M1/M2/M3)
- Vulkan SDK (included in .app bundle)
- Original Generals Zero Hour game files

## Installation
1. Download `GeneralsXZH-v1.0-macOS.dmg`
2. Open .dmg and drag GeneralsXZH.app to Applications
3. Right-click → Open (first launch only, to bypass Gatekeeper)
4. Copy original game assets to:
   `~/Library/Application Support/Generals/`

## Troubleshooting
- "App cannot be opened": Right-click → Open (Gatekeeper prompt)
- "Vulkan not supported": Update GPU drivers / macOS version
- "No audio": Check System Preferences → Sound
- Performance issues: Close background apps, check Activity Monitor
```

**Update `README.md`** (add macOS to supported platforms):
```markdown
## Supported Platforms
- ✅ Windows (DirectX 8, original game compatible)
- ✅ Linux (Vulkan via DXVK, native ELF)
- ✅ macOS (Vulkan via MoltenVK, universal binary)
```

---

## Acceptance Criteria (Phase 5)

Phase 5 is **COMPLETE** when:
- [ ] `macos-vulkan` CMake preset builds successfully
- [ ] Universal binary (Intel + Apple Silicon) created
- [ ] Game launches on macOS without crashes
- [ ] Main menu renders correctly (MoltenVK working)
- [ ] Skirmish match playable (graphics + audio + input)
- [ ] Performance within 20% of Linux (80+ FPS)
- [ ] No P0 crashes in 30-minute test session
- [ ] .app bundle or .dmg installer available
- [ ] Documentation complete (`INSTALL_MACOS.md`)
- [ ] Windows + Linux builds still working (no regressions)

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| DXVK doesn't build for macOS | High | Investigate patches, may need to fork or use alternative |
| MoltenVK incompatibility | Medium | Test early, MoltenVK is mature but may have quirks |
| Performance worse than expected | Medium | Profile early, may need to optimize DXVK settings |
| Code signing issues | Low | Not required for initial release, future enhancement |
| Universal binary issues | Low | Test on both architectures, use separate builds if needed |

---

## Timeline (Estimated)

- **Day 1-2**: CMake configuration, build setup (4-8 hours)
- **Day 3-5**: Library loading adaptation, DXVK macOS build (8-12 hours)
- **Day 6-7**: Testing, packaging, documentation (8-12 hours)
- **Total**: 20-40 hours over 1-2 weeks (part-time)

**Fast track**: If DXVK builds for macOS out-of-the-box, could be done in 1 week.

---

## Deliverables

- `CMakePresets.json`: `macos-vulkan` preset
- `cmake/config-build.cmake`: MoltenVK detection
- `dx8wrapper.cpp`: macOS library loading
- `GeneralsXZH.app`: macOS app bundle (universal binary)
- `GeneralsXZH-v1.0-macOS.dmg`: Installer
- `docs/INSTALL_MACOS.md`: User installation guide
- Dev blog update: Phase 5 completion notes

---

## Phase 5 → Phase 6 Handoff

**Prerequisites for Phase 6 (Multi-Platform Polish)**:
- Phase 5 complete (macOS port working)
- All 3 platforms stable (Windows, Linux, macOS)
- Community feedback collected (bugs, performance)

**Phase 6 Focus**:
- Cross-platform bug fixes
- Performance parity (all platforms within 10% of baseline)
- Multi-platform packaging (unified releases)
- Multiplayer testing (cross-platform compatibility)

---

## Notes

- **macOS is a "bonus" platform**: Don't compromise Linux or Windows for macOS
- **MoltenVK is mature**: Expect fewer issues than expected, but test thoroughly
- **Community demand uncertain**: Gauge interest during Linux beta (if low demand, defer)
- **Code reuse is key**: Most work is build configuration, not code changes
- **Performance is acceptable**: 10-20% overhead is fine for community port
- **Future optimization**: If performance is poor, consider native Metal renderer (Phase 7+)

---

**Status Tracking**:
- [ ] A. Prerequisites Check
- [ ] B. CMake Configuration
- [ ] C. Library Loading Adaptation
- [ ] D. macOS-Specific Paths
- [ ] E. Universal Binary
- [ ] F. Packaging & Distribution
- [ ] G. Testing & Validation
- [ ] H. Documentation

**Progress**: 0/8 sections complete
