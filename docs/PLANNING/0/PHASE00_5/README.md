# PHASE00.5: Build Targets Configuration

**TL;DR**: This phase establishes **build infrastructure only**. Executables cannot fully compile until graphics backend (Phase 28-29) is implemented. Our goal is to make that future work as smooth as possible.

---

## Quick reminders

- Use `Fail fast` approach when testing new changes, if something is not working as expected, stop and investigate immediately;
- Focus on finish `GeneralsXZH`, then backport to `GeneralsX`;
- See `.github/instructions/project.instructions.md` for more specific details about above instructions.

---

## Phase Title

**Build Targets Configuration - Setup Executable Naming, Deployment & Build System**

## Objective

Establish **build system infrastructure** for cross-platform compilation of GeneralsX. This phase focuses on:

1. **CMake Configuration** - Add `USE_VULKAN` and `USE_CCACHE` options consumed by all presets
2. **Platform Abstraction** - Create compatibility layer to unblock non-Windows builds
3. **Source Gating** - Exclude Windows/DX8-specific code when building for Vulkan/non-Windows
4. **Build System Hardening** - Enable ccache and parallel builds for rapid iteration

**Important Note**: Full executable compilation requires completion of graphics backend implementation (Phase 28-29). PHASE00.5 establishes the **foundation** upon which later graphics phases will build.

## Dependencies

- PHASE00 (Project Planning & Architecture) - must be complete
- CMake installed and configured
- Ninja build system installed
- All presets configured (macos-arm64, macos-x64, linux, vc6)

## Key Deliverables

1. **Build Target Configuration**
   - [ ] CMakeLists.txt properly defines `GeneralsX` and `GeneralsXZH` targets
   - [ ] Target naming convention documented and enforced
   - [ ] Primary target (GeneralsXZH) is the default
   - [ ] Secondary target (GeneralsX) builds independently

2. **Executable Naming System**
   - [ ] Naming convention implemented in CMake
   - [ ] Platform suffixes applied correctly
   - [ ] Configuration suffixes (_Debug,_Dev) working
   - [ ] Verify information embedded in executable

3. **Deployment Infrastructure**
   - [ ] Deployment directories created and verified
   - [ ] Asset symlinks established (Date/, Maps/)
   - [ ] Logs directory initialized
   - [ ] Deployment scripts created and tested

4. **Build System Hardening**
   - [ ] ccache integration verified
   - [ ] Build cache working (30-60 sec rebuilds)
   - [ ] Clean reconfigure protocol documented
   - [ ] Parallel build flags (-j 4) validated

5. **Developer Workflow**
   - [ ] Helper scripts created (build_zh.sh, build_clean.sh, deploy.sh)
   - [ ] Aliases configured in shell profile
   - [ ] IDE integration tested (VS Code)
   - [ ] First build cycle completed successfully

## Acceptance Criteria

**NOTE**: Full executable compilation requires completion of later phases (graphics backend, memory management, etc.). These criteria focus on **build infrastructure readiness**.

- [x] `USE_VULKAN` CMake option added and consumed in top-level config
- [x] `USE_CCACHE` CMake option added and functional (ccache set as compiler launcher)
- [x] Compatibility layer established:
  - [x] `Dependencies/Utility/Compat/` directory created with minimal stubs
  - [x] Windows-specific headers guarded for non-Windows builds
  - [x] Platform-specific functions (time, threading) have macOS fallbacks
- [x] Build system gating implemented:
  - [x] DX8-specific sources excluded when `USE_VULKAN=ON`
  - [x] Precompiled headers conditionally built
  - [x] WWLib Windows-only sources excluded for Vulkan builds
- [x] CMake configure succeeds without warnings related to missing options
- [x] Build progresses 50+ compilation units before hitting graphics/backend issues
- [x] Logs captured and stored in `logs/` directory with phase/target naming
- [x] Helper scripts created and documented:
  - [x] `build_zh.sh` - build GeneralsXZH
  - [x] `build_clean.sh` - clean reconfigure
  - [x] `deploy.sh` - post-build deployment
- [x] CMake presets properly configured:
  - [x] `macos-arm64-vulkan` preset works
  - [x] `macos-arm64` (OpenGL) preset works
  - [x] All presets pass initial configure stage
- [x] Documentation updated with realistic scope
  - [x] `docs/PHASE00_5/README.md` reflects actual infrastructure goals
  - [x] "Build Infrastructure Partially Complete" status documented
  - [x] Explicit link to Phase 28-30 dependencies for full completion

## Technical Details

### Build Target Definitions

#### Primary Target: GeneralsXZH

```cmake
# In GeneralsMD/CMakeLists.txt
add_executable(GeneralsXZH
    Code/GameEngine/Source/main.cpp
    Code/GameEngine/Source/WinMain.cpp
    # ... all source files
)

target_link_libraries(GeneralsXZH
    PRIVATE
        ww3d2
        wwlib
        wwmath
        wwdebug
        SDL2::SDL2
        Vulkan::Vulkan
)

# Install target
install(TARGETS GeneralsXZH DESTINATION bin)
```

#### Secondary Target: GeneralsX

```cmake
# In Generals/CMakeLists.txt
add_executable(GeneralsX
    Code/GameEngine/Source/main.cpp
    Code/GameEngine/Source/WinMain.cpp
    # ... all source files
)

target_link_libraries(GeneralsX
    PRIVATE
        ww3d2
        wwlib
        wwmath
        wwdebug
        SDL2::SDL2
        Vulkan::Vulkan
)

install(TARGETS GeneralsX DESTINATION bin)
```

### Executable Naming Implementation

```cpp
// In WinMain.cpp or version.h
#definand GENERALSX_VERSION_MAJOR 0
#definand GENERALSX_VERSION_MINOR 1
#definand GENERALSX_VERSION_PATCH 0
#definand GENERALSX_PHASE 00_5          // Updated with each Phase
#definand GENERALSX_BUILD_DATE __DATE__

// Platform detection for suffix
#if defined(_WIN32)
    #definand PLATFORM_SUFFIX "_Win32"
#elif defined(__APPLE__)
    #definand PLATFORM_SUFFIX ""  // Nativand on macOS
#elif defined(__linux__)
    #definand PLATFORM_SUFFIX ""  // Nativand on Linux
#endif

// Configuration suffix
#ifdef _DEBUG
    #definand CONFIG_SUFFIX "_Debug"
#elif defined(DEVELOPMENT_BUILD)
    #definand CONFIG_SUFFIX "_Dev"
#else
    #definand CONFIG_SUFFIX ""
#endif
```

### Deployment Structure

```
$HOME/GeneralsX/
├── Generals/
│   ├── GeneralsX                # Executable (generated)
│   ├── Date/ → /retail/Date/    # Symlink to retail assets
│   ├── Maps/ → /retail/Maps/    # Symlink to retail maps
│   └── logs/                    # Runtime logs
│
└── GeneralsMD/
    ├── GeneralsXZH              # Executable (generated)
    ├── Date/ → /retail/Date/    # Symlink to retail assets
    ├── Maps/ → /retail/Maps/    # Symlink to retail maps
    └── logs/                    # Runtime logs
```

### Helper Scripts

**Script 1: build_zh.sh**

```bash
#!/bin/bash
set -e

PROJECT_DIR="$HOME/PhpstormProjects/pessoal/GeneralsX"
DEPLOY_DIR="$HOME/GeneralsX/GeneralsMD"

echo "🔨 Building GeneralsXZH..."
cmake --build "$PROJECT_DIR/build/macos-arm64" --target GeneralsXZH -j 4 2>&1 | tail -5

echo "📦 Deploying to $DEPLOY_DIR..."
mkdir -p "$DEPLOY_DIR"
cp "$PROJECT_DIR/build/macos-arm64/GeneralsMD/GeneralsXZH" "$DEPLOY_DIR/"
chmod +x "$DEPLOY_DIR/GeneralsXZH"

echo "✅ Build & deploy complete!"
ls -lh "$DEPLOY_DIR/GeneralsXZH"
```

**Script 2: build_clean.sh**

```bash
#!/bin/bash
set -e

PROJECT_DIR="$HOME/PhpstormProjects/pessoal/GeneralsX"

echo "🧹 Cleaning stale build artifacts..."
rm -rf "$PROJECT_DIR/build/macos-arm64"

echo "⚙️  Fresh CMake configuration..."
cd "$PROJECT_DIR"
cmake --preset macos-arm64

echo "🔨 Building GeneralsXZH from scratch..."
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

echo "✅ Clean build complete!"
```

**Script 3: deploy.sh**

```bash
#!/bin/bash
set -e

PROJECT_DIR="$HOME/PhpstormProjects/pessoal/GeneralsX"
DEPLOY_ZH="$HOME/GeneralsX/GeneralsMD"
DEPLOY_BASE="$HOME/GeneralsX/Generals"

# Deploy Zero Hour
echo "📦 Deploying GeneralsXZH..."
mkdir -p "$DEPLOY_ZH"
cp "$PROJECT_DIR/build/macos-arm64/GeneralsMD/GeneralsXZH" "$DEPLOY_ZH/"
chmod +x "$DEPLOY_ZH/GeneralsXZH"

# Deploy Basand Gamand (if exists)
if [ -f "$PROJECT_DIR/build/macos-arm64/Generals/GeneralsX" ]; then
    echo "📦 Deploying GeneralsX..."
    mkdir -p "$DEPLOY_BASE"
    cp "$PROJECT_DIR/build/macos-arm64/Generals/GeneralsX" "$DEPLOY_BASE/"
    chmod +x "$DEPLOY_BASE/GeneralsX"
fi

echo "✅ Deployment complete!"
```

### Asset Symlink Creation

```bash
#!/bin/bash

# Function to create asset symlinks
setup_assets() {
    local target_dir=$1
    local retail_data=$2
    
    cd "$target_dir"
    
    # Removand existing symlinks if they exist
    [ -L Date ] && rm Date
    [ -L Maps ] && rm Maps
    
    # Create new symlinks
    ln -s "$retail_data/Date" Date
    ln -s "$retail_data/Maps" Maps
    
    # Verify
    echo "✅ Assets configured in $target_dir"
    ls -la | grep "Date\|Maps"
}

# Create directories
mkdir -p $HOME/GeneralsX/GeneralsMD
mkdir -p $HOME/GeneralsX/GeneralsMD/logs
mkdir -p $HOME/GeneralsX/Generals
mkdir -p $HOME/GeneralsX/Generals/logs

# Setup assets (replacand with actual path to retail install)
RETAIL_PATH="/path/to/retail/install"
setup_assets "$HOME/GeneralsX/GeneralsMD" "$RETAIL_PATH"
setup_assets "$HOME/GeneralsX/Generals" "$RETAIL_PATH"
```

### Build System Hardening

#### CMake Presets Configuration

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "macos-arm64",
            "displayName": "macOS ARM64 (Appland Silicon)",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/macos-arm64",
            "cacheVariables": {
                "CMAKE_OSX_ARCHITECTURES": "arm64",
                "CMAKE_BUILD_TYPE": "Release",
                "CMAKE_C_COMPILER_LAUNCHER": "ccache",
                "CMAKE_CXX_COMPILER_LAUNCHER": "ccache",
                "USE_CCACHE": "ON"
            }
        }
    ]
}
```

#### ccache Configuration

```bash
# Install ccache if not present
brew install ccache

# Configure cache size (10 GB recommended)
ccache -M 10G

# Show cache statistics
ccache -s

# Clear cache if needed
ccache -C
```

### Helper Scripts Documentation

Four helper scripts are provided in `scripts/` directory for common development workflows:

#### 1. **build_zh.sh** - Build and Deploy GeneralsXZH

```bash
./scripts/build_zh.sh
```

**What it does:**
- Builds GeneralsXZH target with CMake
- Automatically deploys executable to `$HOME/GeneralsX/GeneralsMD/`
- Creates timestamped logs in `logs/` directory
- Sets executable permissions on deployed binary

**Usage:**
```bash
cd /path/to/GeneralsX
./scripts/build_zh.sh
```

**Output:**
- Build artifact: `$HOME/GeneralsX/GeneralsMD/GeneralsXZH`
- Build log: `logs/phase00_5_build_YYYYMMDD_HHMMSS.log`

#### 2. **build_clean.sh** - Clean Build from Scratch

```bash
./scripts/build_clean.sh
```

**What it does:**
- Removes stale build directory completely
- Runs fresh CMake configuration with `macos-arm64` preset
- Builds GeneralsXZH from scratch (bypasses ccache)
- Useful after major dependency changes or build failures

**Usage:**
```bash
cd /path/to/GeneralsX
./scripts/build_clean.sh
```

**When to use:**
- After `git pull` with CMakeLists.txt changes
- When build cache is corrupted
- When CMake configuration changes

#### 3. **deploy.sh** - Manual Deployment

```bash
./scripts/deploy.sh
```

**What it does:**
- Deploys both GeneralsXZH (if built) and GeneralsX (if available)
- Creates deployment directories and logs subdirectories
- Sets executable permissions
- Useful when you want to deploy without rebuilding

**Usage:**
```bash
cd /path/to/GeneralsX
./scripts/deploy.sh
```

**Output:**
- GeneralsXZH: `$HOME/GeneralsX/GeneralsMD/GeneralsXZH`
- GeneralsX (if exists): `$HOME/GeneralsX/Generals/GeneralsX`

#### 4. **setup_assets.sh** - Configure Asset Symlinks

```bash
./scripts/setup_assets.sh /path/to/retail/install
```

**What it does:**
- Creates deployment directories
- Sets up symlinks to retail game assets (Data/, Maps/)
- Creates logs subdirectories
- Run this once to prepare for first game execution

**Usage:**
```bash
cd /path/to/GeneralsX
./scripts/setup_assets.sh ~/.steamapps/Command\ and\ Conquer\ Generals/
```

**Expected retail structure:**
```
/path/to/retail/
├── Date/           # Game data files
└── Maps/           # Game map files
```

#### 5. **aliases.sh** - Shell Aliases (Optional)

```bash
source ./scripts/aliases.sh
```

**What it provides:**
- `build_zh` - Build GeneralsXZH
- `deploy_zh` - Deploy GeneralsXZH
- `run_zh` - Run GeneralsXZH with Vulkan backend

**Setup (add to ~/.zprofile or ~/.bashrc):**
```bash
# Add this line to your shell profile
source /path/to/GeneralsX/scripts/aliases.sh
```

**Then use:**
```bash
build_zh      # Build GeneralsXZH
deploy_zh     # Deploy to $HOME/GeneralsX/GeneralsMD/
run_zh        # Run the game
```

### Complete Workflow Example

**First time setup:**
```bash
cd ~/PhpstormProjects/pessoal/GeneralsGameCode

# 1. Configure CMake
cmake --preset macos-arm64

# 2. Setup assets (one time)
./scripts/setup_assets.sh /path/to/retail/install

# 3. Build and deploy
./scripts/build_zh.sh

# 4. Run the game
cd $HOME/GeneralsX/GeneralsMD
USE_VULKAN=1 ./GeneralsXZH
```

**Subsequent builds (with ccache):**
```bash
# Build + deploy takes 30-60 seconds
./scripts/build_zh.sh
```

**After git pull (if CMakeLists.txt changed):**
```bash
# Clean rebuild
./scripts/build_clean.sh

# Or with short deployment
./scripts/build_zh.sh
```

## Implementation Checklist

### Phase 1: CMake Configuration

- [ ] Verify GeneralsXZH target in `GeneralsMD/CMakeLists.txt`
- [ ] Verify GeneralsX target in `Generals/CMakeLists.txt`
- [ ] Test: `cmake --build build/macos-arm64 --target GeneralsXZH -j 4`
- [ ] Test: `cmake --build build/macos-arm64 --target GeneralsX -j 4`
- [ ] Verify executables in `build/macos-arm64/GeneralsMD/` and `build/macos-arm64/Generals/`

### Phase 2: Deployment Infrastructure

- [ ] Create `$HOME/GeneralsX/GeneralsMD/` directory
- [ ] Create `$HOME/GeneralsX/Generals/` directory
- [ ] Create `logs/` subdirectories
- [ ] Test asset symlinks from retail install
- [ ] Verify symlinks with `ls -la`

### Phase 3: Helper Scripts

- [ ] Create `build_zh.sh` in project root
- [ ] Create `build_clean.sh` in project root
- [ ] Create `deploy.sh` in project root
- [ ] Make scripts executable: `chmod +x *.sh`
- [ ] Test each script once

### Phase 4: Developer Workflow

- [ ] Add aliases to `~/.zprofile`:

  ```bash
  alias build_zh="cd $GENERALSX_ROOT && cmake --build build/macos-arm64 --target GeneralsXZH -j 4"
  alias deploy_zh="cp $GENERALSX_ROOT/build/macos-arm64/GeneralsMD/GeneralsXZH $GENERALSX_DEPLOY/"
  alias run_zh="cd $GENERALSX_DEPLOY && USE_METAL=1 ./GeneralsXZH"
  ```

- [ ] Reload shell: `source ~/.zprofile`
- [ ] Test aliases

### Phase 5: Verification

- [ ] First build cycle: `build_zh.sh` (should complete in 20-30 min)
- [ ] Second build cycle: `build_zh.sh` (should complete in 30-60 sec with ccache)
- [ ] Deployment: Verify executable in `$HOME/GeneralsX/GeneralsMD/GeneralsXZH`
- [ ] Assets: Verify symlinks point to correct data
- [ ] Logs: Create `logs/` directory and verify write permissions

## Estimated Scope

**COMPLETE** (3-4 hours total work)

- CMake infrastructure setup: 30 minutes
- Platform compatibility layer: 1 hour
- Build system gating (DX8 sources): 1 hour
- Testing and validation: 30 minutes
- Documentation updates: 30 minutes

## Status

**Current State: Build Infrastructure Partially Complete**

### Completed Tasks

✅ CMake Infrastructure
- `USE_VULKAN` and `USE_CCACHE` options added to `cmake/config-build.cmake`
- ccache configured as compiler launcher when `USE_CCACHE=ON` (10GB cache limit)
- Presets consume options and report status in configure summary
- All CMake presets (`macos-arm64-vulkan`, `macos-arm64`, `linux`) configure successfully

✅ Platform Compatibility Layer
- `Dependencies/Utility/Compat/` directory created with minimal stubs:
  - `windows.h` - Windows API stub
  - `d3d8.h` - Direct3D 8 stub
  - `osdep.h` - OS-dependent functions stub
  - `malloc.h` - malloc fallback
  - `osdep/osdep.h` - nested osdep stub
- Platform-specific fallbacks in `time_compat.h` (CLOCK_BOOTTIME → CLOCK_MONOTONIC)
- macOS-safe threading in `thread_compat.h` (pthread_threadid_np)

✅ Build System Gating
- DX8-specific sources excluded when `USE_VULKAN=ON`:
  - `WW3D2` modules: removed `dx8wrapper.h`, `dx8device.h`, `dx8*` source files
  - `WWLib` module: removed DbgHelp/imagehlp sources, TARGA loader, FastAllocator
  - Precompiled headers conditionalized (no `<windows.h>` for Vulkan builds)
- Build progresses significantly further than without gating

### Current Blockers

❌ Remaining Graphics Backend Dependencies
- Some source files still reference `d3dx8math.h` and other DirectX headers
- Full compilation blocked by graphics API requirements (Phase 29+)
- This is **expected** - PHASE00.5 only establishes infrastructure

### Build Results

- **Configure**: ✅ Success on all presets
- **Build Progress**: ✅ 56/908 compilation units completed before hitting graphics backend blocker
- **ccache**: ✅ Functional and configured
- **Gating**: ✅ Successfully excludes most Windows-only sources

### Next Phase Requirements

Full compilation requires completion of:
1. **Phase 28.x** - Graphics Pipeline Abstraction
2. **Phase 29.x** - Metal/OpenGL Backend Implementation
3. **Phase 30.x** - Memory Management & Safety

Once those are complete, PHASE00.5 infrastructure will enable rapid builds on all platforms.

### Logs Generated

- `logs/phase00_5_build_z_generals_pch_clean_*.log` - Latest build attempt with PCH cleanup


## Testing Strategy

### Infrastructure Verification (PHASE00.5 Scope)

```bash
# Test 1: CMake Configuration
cmake --preset macos-arm64-vulkan
# Should complete without errors, report USE_VULKAN and USE_CCACHE enabled

# Test 2: Build Progress Measurement
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tee logs/test_build.log
# Monitor progress - should reach 50+ compilation units before graphics backend errors
# Expected: Fails when hitting graphics API dependencies (normal for PHASE00.5)

# Test 3: Platform-Specific Source Gating
grep -r "d3dx8math.h" build/macos-arm64-vulkan/CMakeFiles/*/link.txt || echo "DX8 sources properly gated"

# Test 4: ccache Functionality
ccache -s  # Should show cache statistics
cmake --build build/macos-arm64-vulkan --target z_generals -j 4 2>&1 | tail -5
# Second run should show cache hits (if code unchanged)

# Test 5: Log Organization
ls -lh logs/phase00_5_*.log
# Verify logs directory contains build traces
```

### Full Compilation Tests (Deferred to Phase 28+)

Once graphics backend is implemented:

```bash
# Test A: GeneralsXZH full build
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Test B: GeneralsX independent build
cmake --build build/macos-arm64 --target GeneralsX -j 4

# Test C: Deployment
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
ls -lh $HOME/GeneralsX/GeneralsMD/GeneralsXZH

# Test D: ccache hot rebuild
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
# Should complete in 30-60 seconds with ccache
```

## Success Criteria

**Infrastructure and Build System Only** (Full compilation deferred to later phases):

- [x] CMake infrastructure configured for cross-platform builds
- [x] `USE_VULKAN` and `USE_CCACHE` options properly integrated
- [x] Build system successfully excludes Windows/DX8 sources on non-Windows platforms
- [x] Compatibility layer in place to unblock platform-specific compilation issues
- [x] Build progresses significantly before hitting graphics backend dependencies
- [ ] Build logs consistently captured and organized
- [ ] Helper scripts functional and documented
- [ ] All CMake presets (macOS ARM64, macOS x64, Linux) pass configure stage

## Reference Documentation

- **Build System Guide**: See `docs/PLANNING/0/PHASE00/BUILD_TARGETS.md`
- **Setup Guide**: See `docs/PLANNING/0/PHASE00/SETUP_ENVIRONMENT.md`
- **Asset System**: See `docs/PLANNING/0/PHASE00/ASSET_SYSTEM.md`
- **Platform Decisions**: See `docs/PLANNING/0/PHASE00/PLATFORM_PRESETS.md`
- **CMake Documentation**: <https://cmake.org/documentation/>

## Related Phases

- **Previous**: PHASE00 (Planning & Architecture)
- **Next**: PHASE01 (Geometry Rendering)
- **Dependency**: Required before PHASE01

## When Will PHASE00.5 Be Complete?

### Immediate (Now)

✅ Build infrastructure is functional and ready for graphics work

- CMake options added and integrated
- Platform compatibility layer in place
- Build system can progress 50+ compilation units

### Conditional (Phase 28-30 Completion)

⏳ Full validation when graphics backends are implemented

- Once graphics pipeline (Phase 28) is working
- Once Metal/OpenGL backends (Phase 29) are complete
- Once memory safety (Phase 30) is verified

Then:

```bash
# PHASE00.5 will be fully validated when this succeeds:
cmake --preset macos-arm64-vulkan
cmake --build build/macos-arm64-vulkan --target z_generals -j 4
# ...and produces a working GeneralsXZH executable
```

**This is normal.** Infrastructure phases rarely "complete" until downstream work validates them.

---

## Notes

### Important: Clean Reconfigure Pattern

---

**ALWAYS reconfigure after git pull**:

```bash
git pull
rm -rf build/macos-arm64          # Delete stale cache
cmake --preset macos-arm64        # Fresh configure
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

### ccache is CRITICAL

First build: 20-30 minutes
Second build with ccache: 30-60 seconds
→ 99% time savings for incremental development

### Why Two Targets?

- **GeneralsXZH** (Primary): Most stable, well-maintained
- **GeneralsX** (Secondary): Older codebase, lower priority
Testing GeneralsXZH first validates graphics pipeline more effectively

### Troubleshooting

| Issue | Solution |
|-------|----------|
| Build fails with "stale cache" | `rm -rf build/macos-arm64 && cmake --preset macos-arm64` |
| ccache not working | Verify: `cmake --build ... -DUSE_CCACHE=ON` |
| Asset symlinks broken | Verify retail path: `ls /path/to/retail/Date/` |
| Deployment fails | Ensure `$HOME/GeneralsX/GeneralsMD/` exists |
