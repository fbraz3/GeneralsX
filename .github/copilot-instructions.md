# GeneralsX - AI Agent Instructions

## Project Overview
GeneralsX is a cross-platform port of Command & Conquer: Generals (2003) from Windows-only DirectX 8 to macOS/Linux/Windows using OpenGL and Metal backends. This is a **massive C++ game engine** (~500k+ LOC) being ported from Visual C++ 6 (C++98) to modern C++20 with platform abstraction layers.

**Critical Context**: This is NOT a greenfield project. You're working with 20+ year old game code that assumes Windows everywhere. Respect the existing architecture while adding cross-platform support.

## Architecture: The Three-Layer Compatibility System

### Layer 1: Core Compatibility (`Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`)
**2,295 lines** of Windows→POSIX type definitions, DirectX mock structures, and system API translations.
- Provides: `HWND`, `HRESULT`, `D3DFORMAT`, `BITMAPINFOHEADER`, registry stubs, file I/O wrappers
- **Never directly modify game code to remove Windows types** - add compatibility shims here instead
- Example: `MessageBox()` → macOS alert, `GetModuleFileName()` → `_NSGetExecutablePath()`

### Layer 2: DirectX 8 Mock Layer (`Core/Libraries/Source/WWVegas/WW3D2/d3d8.h`)
Fake DirectX 8 interfaces that redirect to OpenGL/Metal backends:
- `IDirect3DDevice8` → `DX8Wrapper` (Phase 29.x - Metal/OpenGL dispatch)
- `IDirect3DTexture8` → `MetalWrapper::CreateTextureFromMemory()` (Phase 28.4)
- All `D3DRS_*` render states mapped to Metal/OpenGL equivalents (Phase 29.1-29.4)

**Pattern**: Original game calls `device->SetRenderState(D3DRS_LIGHTING, TRUE)` → mock intercepts → calls `MetalWrapper::SetLightingEnabled(true)` → Metal shader uniform update.

### Layer 3: Game-Specific Extensions (`GeneralsMD/Code/`)
Zero Hour expansion code that extends base game with platform-specific fixes:
- INI parser hardening (Phase 22.7: `End` token protection, Phase 23.x: MapCache guards)
- Memory safety (Phase 30.6: `isValidMemoryPointer()` bounds checking)
- Texture interception (`texture.cpp::Apply_New_Surface()` - Phase 28.4 breakthrough)

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
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/run_$(date +%Y%m%d_%H%M%S).log
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

### LLDB for Metal Crashes
Metal driver crashes show `AGXMetal13_3` in backtrace - use LLDB:
```bash
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 lldb -o run -o bt -o quit ./GeneralsXZH 2>&1 | tee logs/lldb.log
```

### Process State Verification
Slow initialization != crash. Game takes 30+ seconds to load 15+ .big files and parse 200+ INI files.
```bash
ps aux | grep GeneralsXZH  # Still running?
top -pid $(pgrep GeneralsXZH)  # CPU usage? High = loading, Low = stuck
```

## Project-Specific Patterns

### Graphics Backend Selection (Runtime)
```bash
USE_METAL=1 ./GeneralsXZH   # macOS default (Phase 29.4+)
USE_OPENGL=1 ./GeneralsXZH  # Cross-platform fallback
```

Controlled by `g_useMetalBackend` flag in `WinMain.cpp`:
```cpp
#ifdef __APPLE__
    const char* use_opengl = getenv("USE_OPENGL");
    g_useMetalBackend = (use_opengl == nullptr);  // Metal default on macOS
#endif
```

### The .big File VFS Problem (Phase 28.4 Discovery)
Original assumption: Load textures from disk files → **WRONG**. Textures are inside `Textures.big` archives.

**Solution**: Intercept AFTER DirectX loads from .big via VFS:
```cpp
// texture.cpp::Apply_New_Surface() - runs AFTER DirectX unpacks from .big
IDirect3DSurface8* d3d_surface = /* DirectX loaded this from .big */;
D3DLOCKED_RECT lock;
d3d_surface->LockRect(&lock, NULL, D3DLOCK_READONLY);  // Get pixel data
TextureCache::Load_From_Memory(lock.pBits, width, height, format);  // Upload to Metal
```

See `docs/PHASE28/CRITICAL_VFS_DISCOVERY.md` for why VFS integration failed.

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
Metal driver crashes from bad pointers. Add bounds checking:
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
- Phase 27: W3D Graphics Engine
- Phase 28: Texture Loading (4 sub-phases - DDS/TGA loaders, DirectX interception)
- Phase 29: Metal Render States (lighting, fog, stencil, point sprites)
- Phase 30: Metal Backend Success
- Phase 31: Texture System Integration
- Phase 32: Audio Pipeline Investigation (COMPLETE - INI parsing, event system, lifecycle)
- **Phase 33: OpenAL Audio Backend Implementation (Ready to start - October 2025)**
- Phase 34: Game Logic & Gameplay Systems (Planned - depends on Phase 33 audio)
- Phase 35: Multiplayer & Networking (Planned)

### Critical Reference Files
- `docs/MACOS_PORT_DIARY.md` - **UPDATE AFTER EVERY SESSION** - Technical development diary (project requirement)
- `docs/Misc/BIG_FILES_REFERENCE.md` - Asset structure (INI.big, INIZH.big contents)
- `docs/Misc/GRAPHICS_BACKENDS.md` - Metal vs OpenGL runtime selection
- `docs/Misc/CRITICAL_FIXES.md` - Emergency fixes (fullscreen lock, NULL crashes)
- `docs/Misc/AUDIO_BACKEND_STATUS.md` - Audio system status (OpenAL stub vs real implementation)
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

**Pattern**: Stuck on DirectX render state mapping? Check `references/dxgldotorg-dxgl/` for proven D3D→OpenGL translations.

**Audio Implementation**: For Phase 33 (OpenAL backend), refer to `references/jmarshall-win64-modern/Code/GameEngineDevice/Source/OpenALAudioDevice/` for complete working implementation.

## Testing Strategy

### Unit Tests in `tests/` Directory
```bash
# Metal initialization test
tests/metal_init_test.cpp

# Texture format tests (BC3 compression, DDS/TGA loading)
tests/test_bc3_*.cpp
tests/test_dds_loader.cpp
tests/test_tga_loader.cpp

# Run all OpenGL tests
tests/opengl/run_opengl_tests.sh
```

**Rule**: All new tests go in `tests/` - no test code in game directories.

### Integration Testing
```bash
# Run game with specific subsystem logging
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/subsystem_test.log

# Check subsystem initialization status
grep "initSubsystem" logs/subsystem_test.log
grep "Subsystem.*operational" logs/subsystem_test.log
```

## Common Pitfalls

1. **Assuming Desktop Paths Work**: Game expects `Data\` directory with backslashes from Windows. Use `TheFileSystem` for cross-platform file access, NOT raw `fopen()`.

2. **DirectX Enums Don't Map 1:1**: `D3DTEXTUREADDRESS_WRAP` (Windows) ≠ `GL_REPEAT` (OpenGL). Check `dx8wrapper.cpp` for proven mappings.

3. **Texture Formats Are Complex**: `D3DFMT_DXT1` = BC1 compression. Apple Silicon requires 256-byte aligned `bytesPerRow`. See `MetalWrapper::CreateTextureFromMemory()` for alignment logic.

4. **Build Errors After Git Pull**: Stale CMake cache. **Always reconfigure**:
   ```bash
   rm -rf build/macos-arm64
   cmake --preset macos-arm64
   ```

5. **"Game Hangs at Startup"**: NOT a hang - it's loading 15+ .big files. Wait 30-60 seconds or check CPU usage.

## Commit Message Convention

Follow conventional commits (project requirement):
```
feat(metal): implement texture upload pipeline for BC3 compressed formats

- Add MetalWrapper::CreateTextureFromMemory() with format conversion
- Handle bytesPerRow alignment (256 bytes for Apple Silicon)
- Support RGBA8, RGB8, DXT1/3/5 (BC1/2/3) formats

Refs: Phase 28.4 Post-DirectX Texture Interception
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

**I'm stuck on a crash** → Check `$HOME/Documents/.../ReleaseCrashInfo.txt` first, then LLDB if Metal-related

**I need to understand .big file structure** → Read `docs/Misc/BIG_FILES_REFERENCE.md`

**I need compilation commands** → Check `.github/instructions/project.instructions.md`

**I need proven cross-platform solution** → Compare `references/*/` submodules

**I need graphics backend info** → Read `docs/Misc/GRAPHICS_BACKENDS.md`

**Build fails mysteriously** → Delete `build/` and reconfigure with preset

**Game initialization seems stuck** → It's loading assets - wait 60 seconds, check CPU usage, grep logs AFTER completion

**Integration not working as expected** → Read `docs/Misc/LESSONS_LEARNED.md` for common architectural mismatches (VFS, pipeline execution)
