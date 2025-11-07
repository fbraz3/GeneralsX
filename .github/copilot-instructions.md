# GeneralsX - AI Agent Instructions

## Project Overview
GeneralsX is a cross-platform port of Command & Conquer: Generals (2003) from Windows-only DirectX 8 to macOS/Linux/Windows using Vulkan (via MoltenVK on macOS). This is a **massive C++ game engine** (~500k+ LOC) being ported from Visual C++ 6 (C++98) to modern C++20 with platform abstraction layers.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code that assumes Windows everywhere. Phase 50 is a clean Vulkan-only refactor using platform-conditional compilation (not graphics-backend-conditional), based on the fighter19-dxvk-port architecture pattern.

## Architecture: The Three-Layer Compatibility System

### Layer 1: Core Compatibility (`Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`)
**2,295 lines** of Windows→POSIX type definitions, DirectX mock structures, and system API translations.
- Provides: `HWND`, `HRESULT`, `D3DFORMAT`, `BITMAPINFOHEADER`, registry stubs, file I/O wrappers
- **Never directly modify game code to remove Windows types** - add compatibility shims here instead
- Example: `MessageBox()` → macOS alert, `GetModuleFileName()` → `_NSGetExecutablePath()`

### Layer 2: DirectX 8 Mock Layer (`Core/Libraries/Source/WWVegas/WW3D2/d3d8.h`)
Fake DirectX 8 interfaces that redirect to Vulkan backend via MoltenVK:
- `IDirect3DDevice8` → `DX8Wrapper` (Vulkan dispatch)
- `IDirect3DTexture8` → `VulkanTextureManager::CreateTextureFromMemory()`
- All `D3DRS_*` render states mapped to Vulkan pipeline equivalents

**Pattern**: Original game calls `device->SetRenderState(D3DRS_LIGHTING, TRUE)` → mock intercepts → calls `VulkanGraphicsBackend::SetLightingEnabled(true)` → Vulkan shader uniform update via descriptor sets.

### Layer 3: Game-Specific Extensions (`GeneralsMD/Code/`)
Zero Hour expansion code that extends base game with platform-specific fixes:
- INI parser hardening (Phase 22.7: `End` token protection, Phase 23.x: MapCache guards)
- Memory safety (Phase 30.6: `isValidMemoryPointer()` bounds checking)
- Texture interception: DirectX surfaces → Vulkan image uploads (Phase 50 architecture)

## Critical Build Workflow

### Presets Are Everything
```bash
# Configure (creates build/macos-arm64/)
cmake --preset macos-arm64

# Build PRIMARY TARGET (Zero Hour expansion - most stable)
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 | tee logs/build.log

# Build SECONDARY TARGET (original game - less tested)
cmake --build build/macos-arm64 --target GeneralsX -j 4 | tee logs/build.log
```

**Why `-j 4` not `-j 8`?** This codebase has MASSIVE translation units (5MB+ object files). Half CPU cores prevents OOM kills. See `build_zh.sh` for production workflow.

### Compilation Cache ESSENTIAL
First build: ~20-30 minutes. With ccache: ~30-60 seconds for incremental changes.
```bash
brew install ccache  # macOS
ccache -M 10G        # Set 10GB cache
cmake --preset macos-arm64 -DUSE_CCACHE=ON
```

### The Asset Path Problem
Game requires **original .big files** from retail game:
```bash
# Assets MUST be here (symlink from Steam/GOG install)
$HOME/GeneralsX/GeneralsMD/Data/      # Contains INI.big, INIZH.big, Textures.big, etc.
$HOME/GeneralsX/GeneralsMD/Maps/      # Contains map files

# Deploy executable here too
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
```

**Why?** The engine uses relative paths: `Data\INI\Object\AirforceGeneral.ini` expects `Data/` directory adjacent to executable.

## Debugging Workflow (NOT Obvious from Code)

### The Log-First Philosophy
**NEVER run game with piped grep** - it hides initialization progress and causes apparent hangs.

```bash
# ✅ CORRECT: Full log to file, grep afterwards
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH 2>&1 | tee logs/run_$(date +%Y%m%d_%H%M%S).log
grep -A 5 -B 5 "error\|crash" logs/run_20251019_143022.log

# ❌ WRONG: Grep during execution hides startup messages
./GeneralsXZH 2>&1 | grep error  # Looks like hang, but game is loading .big files
```

### Crash Logs are NOT in Terminal
```bash
# Zero Hour crash info (ALWAYS check this first after crash)
cat "$HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt"

# Generals crash info
cat "$HOME/Documents/Command and Conquer Generals Data/ReleaseCrashInfo.txt"
```

### LLDB for Vulkan Crashes
Vulkan driver crashes (MoltenVK) in backtrace - use LLDB:
```bash
cd $HOME/GeneralsX/GeneralsMD && lldb -o run -o bt -o quit ./GeneralsXZH 2>&1 | tee logs/lldb.log
```

### Process State Verification
Slow initialization != crash. Game takes 30+ seconds to load 15+ .big files and parse 200+ INI files.
```bash
ps aux | grep GeneralsXZH  # Still running?
top -pid $(pgrep GeneralsXZH)  # CPU usage? High = loading, Low = stuck
```

## Project-Specific Patterns

### Graphics Backend (Vulkan/MoltenVK)
Phase 50 uses **Vulkan exclusively** on all platforms (macOS via MoltenVK, Linux native Vulkan, Windows native Vulkan). No runtime backend selection - single unified Vulkan code path.

```bash
./GeneralsXZH  # Vulkan via MoltenVK on macOS
```

Controlled by CMake configuration (`-DENABLE_VULKAN=ON`). Platform-specific Vulkan implementations are selected at compile time, not runtime.

### The .big File VFS Problem (Phase 28 History)
Historical lesson: Original assumption was Load textures from disk files → **WRONG**. Textures are inside `Textures.big` archives.

**Current Solution (Phase 50)**: DirectX surfaces are intercepted after DirectX unpacks from .big via VFS, then uploaded to Vulkan images:
```cpp
// texture.cpp::Apply_New_Surface() - runs AFTER DirectX unpacks from .big
IDirect3DSurface8* d3d_surface = /* DirectX loaded this from .big */;
D3DLOCKED_RECT lock;
d3d_surface->LockRect(&lock, NULL, D3DLOCK_READONLY);  // Get pixel data
VulkanTextureManager::UploadToGPU(lock.pBits, width, height, format);  // Upload to Vulkan
```

See `docs/Misc/LESSONS_LEARNED.md` for architectural insights.

### INI Parser Hardening Pattern (Phase 22-23)
Game crashes on malformed INI files. Protection pattern:
```cpp
// Before (crash on unexpected 'End' token)
field_parser->Parse(chunk, ini);

// After (Phase 22.7 fix)
if (strcmp(chunk->Get_Name(), "End") == 0) {
    printf("INI::initFromINIMulti - Skipping 'End' token\n");
    continue;  // Graceful skip instead of exception
}
```

Apply this to ALL INI parsing code: MapCache, MetaMap, CommandMap, LanguageFilter.

### Memory Protection Pattern (Phase 30.6)
Vulkan driver crashes from bad pointers. Add bounds checking:
```cpp
// GameMemory.cpp::isValidMemoryPointer()
bool isValidMemoryPointer(void* ptr, size_t minSize) {
    if (ptr == NULL) return false;
    if ((uintptr_t)ptr < 0x1000) return false;  // NULL-ish addresses
    if ((uintptr_t)ptr > 0x00007FFFFFFFFFFF) return false;  // Beyond user space
    // Add size validation for collections
    if (minSize > 1000000) return false;  // Suspiciously large
    return true;
}
```

## Documentation Organization

### Phase-Based Structure (`docs/PHASE##/`)
- Phase 27-31: W3D Graphics Engine, Texture Loading, Metal Render States (archived - superseded)
- Phase 32-49: Development history, lessons learned, architectural experiments
- **Phase 50: Vulkan-Only Clean Refactor (CURRENT)**
  - Removed Metal backend
  - Vulkan exclusive implementation
  - Platform-conditional compilation model (not graphics-backend-conditional)
  - Based on fighter19-dxvk-port architecture
- Phase 51: Audio Pipeline Implementation (Planned)
- Phase 52+: Game Logic, Gameplay, Multiplayer (Planned)

### Critical Reference Files
- `docs/MACOS_PORT_DIARY.md` - **UPDATE AFTER EVERY SESSION** - Technical development diary (project requirement)
- `docs/Misc/BIG_FILES_REFERENCE.md` - Asset structure (INI.big, INIZH.big contents)
- `docs/Misc/LESSONS_LEARNED.md` - Architectural insights and integration patterns
- `docs/Misc/CRITICAL_FIXES.md` - Emergency fixes and validation patterns
- `docs/Misc/AUDIO_BACKEND_STATUS.md` - Audio system status
- `.github/instructions/project.instructions.md` - Compilation/debugging commands

## Reference Repositories (Git Submodules)

When stuck, compare with proven solutions:
```bash
git submodule update --init --recursive  # Initialize all references

# jmarshall-win64-modern: INI parser fixes, Win64 port (provided Phase 22.7-22.8 breakthrough)
references/jmarshall-win64-modern/

# fighter19-dxvk-port: Linux port with DirectX→Vulkan via DXVK
references/fighter19-dxvk-port/

# dxgldotorg-dxgl: DirectX7→OpenGL wrapper patterns (excellent for DirectX8 mocking)
references/dxgldotorg-dxgl/
```

**Pattern**: Stuck on DirectX render state mapping? Check `references/dxgldotorg-dxgl/` for proven D3D mapping patterns.

**Vulkan Architecture Template**: Use `references/fighter19-dxvk-port/` for platform-conditional compilation model - shows how to structure Vulkan-only code without fallbacks.

## Testing Strategy

### Unit Tests in `tests/` Directory
```bash
# Vulkan initialization test
tests/vulkan_init_test.cpp

# Texture format tests (BC3 compression, DDS/TGA loading)
tests/test_bc3_*.cpp
tests/test_dds_loader.cpp
tests/test_tga_loader.cpp

# Vulkan-specific tests
tests/vulkan/
```

**Rule**: All new tests go in `tests/` - no test code in game directories.

### Integration Testing
```bash
# Run game with subsystem logging
cd $HOME/GeneralsX/GeneralsMD
./GeneralsXZH 2>&1 | tee logs/subsystem_test.log

# Check subsystem initialization status
grep "initSubsystem" logs/subsystem_test.log
grep "Vulkan.*initialized" logs/subsystem_test.log
```

## Common Pitfalls

1. **Assuming Desktop Paths Work**: Game expects `Data\` directory with backslashes from Windows. Use `TheFileSystem` for cross-platform file access, NOT raw `fopen()`.

2. **DirectX Enums Don't Map 1:1**: `D3DTEXTUREADDRESS_WRAP` (Windows) needs Vulkan equivalent. Check `dx8wrapper.cpp` for proven mappings.

3. **Texture Formats Are Complex**: `D3DFMT_DXT1` = BC1 compression. Vulkan images require proper format specification and memory alignment. See `VulkanTextureManager` for implementation.

4. **Build Errors After Git Pull**: Stale CMake cache. **Always reconfigure**:
   ```bash
   rm -rf build/macos-arm64
   cmake --preset macos-arm64
   ```

5. **"Game Hangs at Startup"**: NOT a hang - it's loading 15+ .big files. Wait 30-60 seconds or check CPU usage.

## Commit Message Convention

Follow conventional commits (project requirement):
```
feat(vulkan): implement texture upload pipeline for BC3 compressed formats

- Add VulkanTextureManager::UploadToGPU() with format conversion
- Handle Vulkan image memory alignment requirements
- Support RGBA8, RGB8, DXT1/3/5 (BC1/2/3) formats

Refs: Phase 50 Vulkan-Only Refactor
```

**Rules**:
- All messages in **English** (code/commits)
- **Commit Time Restrictions**: Do NOT commit code during business hours on weekdays (Monday-Friday 09:00-18:00 local time). Weekends (Saturday-Sunday) allow commits at any time. This restriction can be overridden if explicitly requested by the user.

## When to Update Documentation

**ALWAYS update** `docs/MACOS_PORT_DIARY.md` (development diary) at session end with:
- Phase progress (e.g., "Phase 28.4 complete - 7 textures loaded")
- Critical discoveries (e.g., "VFS integration failed, switched to DirectX interception")
- Commit hashes for reference

**Document critical discoveries** in `docs/Misc/LESSONS_LEARNED.md`:
- Integration failures and their root causes
- Architectural mismatches discovered during development
- Debugging patterns that led to breakthroughs
- Timeline impacts and solution trade-offs

## AI Agent Quick Reference

**I'm stuck on a crash** → Check `$HOME/Documents/.../ReleaseCrashInfo.txt` first, then LLDB for backtrace

**I need to understand .big file structure** → Read `docs/Misc/BIG_FILES_REFERENCE.md`

**I need compilation commands** → Check `.github/instructions/project.instructions.md`

**I need proven Vulkan architecture** → Compare with `references/fighter19-dxvk-port/` (Linux DXVK port)

**I need architectural insights** → Read `docs/Misc/LESSONS_LEARNED.md` for common pitfalls and solutions

**Build fails mysteriously** → Delete `build/` and reconfigure with preset

**Game initialization seems stuck** → It's loading assets - wait 60 seconds, check CPU usage, grep logs AFTER completion

**Vulkan initialization failing** → Check Vulkan SDK installation, MoltenVK on macOS, and validation layer output
