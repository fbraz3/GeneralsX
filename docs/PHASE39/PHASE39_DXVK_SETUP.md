# Phase 39: DXVK/MoltenVK Graphics Backend Setup

**Status**: 🟡 **READY TO START** (Oct 30, 2025)

**Objective**: Install and configure DXVK/MoltenVK environment for Phase 39-40 backend implementation

**Timeline**: 1-2 days (environment setup, not graphics coding yet)

---

## What is DXVK/MoltenVK?

### DXVK
- **DirectX 8 → Vulkan** translator
- Intercepts DirectX API calls, converts to Vulkan commands
- Used in Proton (Steam Play) for Windows games on Linux
- Production-proven (1000+ games tested)

### MoltenVK
- **Vulkan → Metal** translator for macOS
- Converts Vulkan commands to Metal API
- Apple Silicon (ARM64) optimized
- Native Metal performance on macOS

### Architecture Flow
```
GeneralsX (DirectX 8 code)
    ↓
DXVK (DirectX → Vulkan)
    ↓
MoltenVK (Vulkan → Metal)
    ↓
Apple Silicon Metal Hardware
```

**Benefit**: Single codebase renders on Mac/Linux/Windows via Vulkan abstraction!

---

## Phase 39 Breakdown

### Phase 39.1: Environment Setup ✅ START HERE
- Install DXVK via Homebrew
- Install MoltenVK via Homebrew
- Verify Vulkan SDK (if needed)
- Configure CMake for DXVK compilation

### Phase 39.2: DXVKGraphicsBackend Implementation
- Create `graphics_backend_dxvk.cpp`
- Implement `IGraphicsBackend` interface
- Delegate to DXVK API instead of Metal

### Phase 39.3: CMake Integration
- Add DXVK dependency to CMakeLists.txt
- Create build preset: `cmake --preset macos-arm64-dxvk`
- Link DXVK libraries

### Phase 39.4: Environment Variable Selection
- Add `USE_DXVK` environment variable
- Factory pattern selects between:
  - `USE_DXVK=0` → LegacyGraphicsBackend (Phase 38 Metal/OpenGL)
  - `USE_DXVK=1` → DXVKGraphicsBackend (Phase 39 Vulkan)

### Phase 39.5: Testing
- Compare visual output (Metal vs DXVK)
- Verify performance (FPS consistency)
- Run 30+ minute gameplay test
- Document any visual differences

---

## Installation Commands (macOS)

### Step 1: Install DXVK via Homebrew

```bash
# Install DXVK (DirectX → Vulkan)
brew install dxvk

# Verify installation
ls /usr/local/Cellar/dxvk/
dxvk-config  # Should work if installed correctly
```

**Expected output**:
```
Installing: dxvk...
Build dxvk from source: yes
Compiling dxvk (may take 10-15 minutes)...
Installed to: /usr/local/Cellar/dxvk/1.x.x/
```

### Step 2: Install MoltenVK via Homebrew

```bash
# Install MoltenVK (Vulkan → Metal)
brew install molten-vk

# Verify installation
ls /usr/local/Cellar/molten-vk/
echo $DYLD_LIBRARY_PATH  # Should include MoltenVK path
```

**Expected output**:
```
Installing: molten-vk...
Build molten-vk from source: yes
Compiling molten-vk (may take 5-10 minutes)...
Installed to: /usr/local/Cellar/molten-vk/1.x.x/
```

### Step 3: Verify Vulkan SDK (Optional)

```bash
# Check if Vulkan SDK is available (may not be needed on macOS with MoltenVK)
brew install vulkan-tools

# Verify tools
vulkaninfo  # Lists Vulkan capabilities
```

### Step 4: Set Environment Variables

Add to your shell profile (`~/.zshrc` or `~/.bashrc`):

```bash
# DXVK paths
export DXVK_HOME=/usr/local/Cellar/dxvk/1.x.x
export DXVK_CONFIG_FILE=~/GeneralsX/dxvk.conf

# MoltenVK paths
export DYLD_LIBRARY_PATH=/usr/local/Cellar/molten-vk/1.x.x/lib:$DYLD_LIBRARY_PATH
export VK_ICD_FILENAMES=/usr/local/Cellar/molten-vk/1.x.x/lib/icd.d/MoltenVK_icd.json

# Reload shell
source ~/.zshrc
```

---

## CMake Configuration for Phase 39

### Option A: Update CMakeLists.txt (If needed)

```cmake
# Add DXVK support
option(USE_DXVK "Use DXVK graphics backend" OFF)

if(USE_DXVK)
    # Find DXVK package
    find_package(DXVK REQUIRED)
    
    # Add DXVK include directories
    include_directories(${DXVK_INCLUDE_DIRS})
    
    # Link DXVK libraries
    target_link_libraries(GeneralsXZH ${DXVK_LIBRARIES})
    
    # Add DXVK definition
    target_compile_definitions(GeneralsXZH PRIVATE USE_DXVK=1)
endif()
```

### Option B: Create New CMake Preset (Recommended)

Create `CMakePresets.json` preset for DXVK:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "macos-arm64-dxvk",
      "displayName": "macOS ARM64 with DXVK",
      "inherits": "macos-arm64",
      "cacheVariables": {
        "USE_DXVK": "ON",
        "DXVK_HOME": "/usr/local/Cellar/dxvk/1.x.x"
      }
    }
  ]
}
```

### Option C: Command-line Build (Simplest for Phase 39.1)

```bash
# Build with DXVK enabled
cmake --preset macos-arm64 \
  -DUSE_DXVK=ON \
  -DDXVK_HOME=/usr/local/Cellar/dxvk/1.x.x

cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

---

## Phase 39.1 Task Checklist

- [ ] **Install DXVK**
  ```bash
  brew install dxvk
  ```

- [ ] **Install MoltenVK**
  ```bash
  brew install molten-vk
  ```

- [ ] **Verify installations**
  ```bash
  ls /usr/local/Cellar/dxvk/
  ls /usr/local/Cellar/molten-vk/
  ```

- [ ] **Set environment variables**
  ```bash
  export DYLD_LIBRARY_PATH=/usr/local/Cellar/molten-vk/1.x.x/lib:$DYLD_LIBRARY_PATH
  export VK_ICD_FILENAMES=/usr/local/Cellar/molten-vk/1.x.x/lib/icd.d/MoltenVK_icd.json
  ```

- [ ] **Test Vulkan tools (optional)**
  ```bash
  vulkaninfo | head -20  # Should show Vulkan version and extensions
  ```

- [ ] **Create Phase 39.1 completion document** (after setup)

---

## Expected Outcomes After Phase 39.1

✅ DXVK available in `/usr/local/Cellar/dxvk/`
✅ MoltenVK available in `/usr/local/Cellar/molten-vk/`
✅ Environment variables configured
✅ Vulkan tools available (optional, for debugging)
✅ Ready for Phase 39.2 backend implementation

---

## Troubleshooting Phase 39.1

### Issue: "brew install dxvk" fails
**Solution**: Ensure Homebrew is up-to-date
```bash
brew update
brew upgrade
brew install dxvk
```

### Issue: "MoltenVK not found after installation"
**Solution**: Verify Homebrew tap
```bash
brew tap homebrew/cask  # May need cask tap
brew install molten-vk --cask  # Try cask installation
```

### Issue: "Vulkan tools missing"
**Solution**: MoltenVK includes vulkan-loader, tools optional for Phase 39.1
```bash
# Phase 39.1 doesn't require vulkan-tools, skip if issues
# Focus on DXVK + MoltenVK core libraries
```

### Issue: DYLD_LIBRARY_PATH not persisting
**Solution**: Add to shell profile permanently
```bash
# For zsh (macOS default):
echo 'export DYLD_LIBRARY_PATH=/usr/local/Cellar/molten-vk/1.x.x/lib:$DYLD_LIBRARY_PATH' >> ~/.zshrc
source ~/.zshrc

# For bash:
echo 'export DYLD_LIBRARY_PATH=/usr/local/Cellar/molten-vk/1.x.x/lib:$DYLD_LIBRARY_PATH' >> ~/.bash_profile
source ~/.bash_profile
```

---

## Reference Documentation

- **DXVK Project**: https://github.com/doitsujin/dxvk
- **MoltenVK Project**: https://github.com/KhronosGroup/MoltenVK
- **Vulkan Specification**: https://www.khronos.org/vulkan/
- **Proton/DXVK in Production**: Used in Steam Play for 1000+ games

---

## Phase 39.2 Preview (Next Phase)

After Phase 39.1 setup, Phase 39.2 will:

1. Create `graphics_backend_dxvk.cpp` implementing `IGraphicsBackend`
2. Add DXVK-specific initialization code
3. Build with `USE_DXVK=1` flag
4. Test with `USE_DXVK=1 ./GeneralsXZH`

**Expected Phase 39.2 development time**: 4-6 hours

---

## Success Criteria for Phase 39.1

✅ `brew list | grep dxvk` shows installed package
✅ `brew list | grep molten-vk` shows installed package
✅ `echo $DYLD_LIBRARY_PATH` includes MoltenVK path
✅ `vulkaninfo` works (or MoltenVK icd.json accessible)
✅ Ready to proceed to Phase 39.2 backend implementation

**Status After Phase 39.1**: Ready for DXVKGraphicsBackend coding ✅

---

## Why This Approach?

**Phase 38** created abstraction layer:
```cpp
IGraphicsBackend interface
    ↓
LegacyGraphicsBackend (Phase 27-37 Metal/OpenGL via DX8Wrapper)
    ↓
NEW: DXVKGraphicsBackend (Phase 39 Vulkan via DXVK)
```

**Phase 39** adds DXVK backend without touching game code!

This is why Phase 38 was necessary - to enable pluggable backends.

---

**Next Steps**:
1. ✅ Read this document
2. ⏳ Run Phase 39.1 installation commands
3. ⏳ Verify environment variables
4. ⏳ Document setup completion
5. ⏳ Start Phase 39.2 DXVKGraphicsBackend implementation

**Estimated time for Phase 39.1**: 30-45 minutes
