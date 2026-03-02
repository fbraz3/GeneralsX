# GitHub Actions: Deployment Bundle Strategy

**Date**: 28 Feb 2026  
**Branches**: `main` (Linux), `macos-build` (macOS)

---

## Problem Identified

### Initial GitHub Actions Issue
```
❌ No files were found with the provided path:
   build/linux64-deploy/GeneralsMD/z_generals
   build/linux64-deploy/Generals/g_generals
No artifacts will be uploaded.
```

### Root Causes

1. **Binary name mismatch**: CMake `OUTPUT_NAME` differs from target name
   - CMake target: `z_generals` 
   - Actual binary: `GeneralsXZH` (on Linux) / `GeneralsX` (Generals)
   - Workflow was looking for: `z_generals` (doesn't exist in build dir)

2. **Missing runtime dependencies**: Upload was bare binary without libraries
   - Binary alone cannot run (needs DXVK, SDL3, GameSpy libraries)
   - Local deploy scripts copy libs, set RPATH/DYLD_LIBRARY_PATH
   - Workflow was not mimicking this deployment pattern

---

## Solution: Deployment Bundle

Instead of uploading bare binary, create complete **deployment bundle** with:

```
GeneralsX-<game>-<os>-deploy/
├── GeneralsXZH (or GeneralsX)     [executable with correct name]
├── lib/
│   ├── libdxvk_d3d8.so(.2)        [DXVK DirectX wrapper]
│   ├── libdxvk_d3d9.so(.2)        [Optional Direct3D 9 compat]
│   ├── libSDL3.so(.0.0.so)        [SDL3 windowing/input]
│   ├── libSDL3_image.so.0         [SDL3 image loading]
│   ├── libgamespy.so              [Online multiplayer]
│   └── libMoltenVK.dylib          [macOS: Vulkan->Metal translation]
└── run.sh                         [Wrapper script with env setup]
```

### Bundle Contents by Platform

#### Linux Bundle
```
run.sh structure:
  - LD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${LD_LIBRARY_PATH}"
  - DXVK_WSI_DRIVER="SDL3"
  - DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
  - exec "./GeneralsXZH" "$@"
```

#### macOS Bundle
```
run.sh structure:
  - DYLD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${DYLD_LIBRARY_PATH}"  [macOS equivalent]
  - DXVK_WSI_DRIVER="SDL3"
  - DXVK_LOG_LEVEL="${DXVK_LOG_LEVEL:-info}"
  - exec "./GeneralsXZH" "$@"
```

---

## Implementation: Workflow Changes

### Step 1: Verify Binary With Correct Names

**Before** (incorrect):
```yaml
BINARY="build/${{ inputs.preset }}/GeneralsMD/z_generals"  # ❌ Wrong
```

**After** (correct):
```yaml
BINARY="build/${{ inputs.preset }}/GeneralsMD/GeneralsXZH"  # ✅ Correct CMake OUTPUT_NAME
```

### Step 2: Create Deployment Bundle

**New Step** (added after build succeeds):
```bash
# Copy executable to bundle
cp "${BINARY}" "${RUNTIME_DIR}/GeneralsXZH"
chmod +x "${RUNTIME_DIR}/GeneralsXZH"

# Copy all runtime libs to bundle/lib
mkdir -p "${RUNTIME_DIR}/lib"
cp "${DXVK_LIB_DIR}"/libdxvk_d3d8.so* "${RUNTIME_DIR}/lib/"
cp "${SDL3_LIB_DIR}"/libSDL3.so* "${RUNTIME_DIR}/lib/"
cp "${GAMESPY_LIB}" "${RUNTIME_DIR}/lib/"

# Create wrapper script with environment setup
cat > "${RUNTIME_DIR}/run.sh" << 'EOF'
#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${LD_LIBRARY_PATH}"
export DXVK_WSI_DRIVER="SDL3"
exec "${SCRIPT_DIR}/GeneralsXZH" "$@"
EOF
chmod +x "${RUNTIME_DIR}/run.sh"
```

### Step 3: Upload Bundle Instead of Bare Binary

**Before** (fails):
```yaml
- name: Upload Binary Artifact
  uses: actions/upload-artifact@v4
  with:
    name: binary-linux-${{ inputs.game }}-${{ inputs.preset }}
    path: |
      build/${{ inputs.preset }}/GeneralsMD/z_generals      # ❌ Wrong path/name
```

**After** (succeeds):
```yaml
- name: Upload Deployment Bundle
  uses: actions/upload-artifact@v4
  with:
    name: bundle-linux-${{ inputs.game }}-${{ inputs.preset }}
    path: /tmp/GeneralsX-${{ inputs.game }}-deploy/       # ✅ Complete bundle
```

---

## Artifact Names Changed

| Platform | Before | After |
|----------|--------|-------|
| **Linux** | `binary-linux-GeneralsMD-linux64-deploy` | `bundle-linux-GeneralsMD-linux64-deploy` |
| **macOS** | `binary-macos-GeneralsMD-macos-vulkan` | `bundle-macos-GeneralsMD-macos-vulkan` |

---

## Benefits of Bundle Approach

✅ **Runnable out-of-box**:
- Download bundle from Actions UI
- Extract to folder
- Run: `./run.sh -win`
- No additional setup needed

✅ **Matches local deploy scripts**:
- Mirrors `scripts/deploy-linux-zh.sh` pattern
- Consistent environment setup (LD_LIBRARY_PATH, DXVK vars)
- Easier debugging (all files in one place)

✅ **Enables future validation**:
- Can run smoke tests in CI (reach main menu)
- Can validate cross-platform behavior
- Can check for missing dependencies

✅ **Cross-platform consistency**:
- Linux and macOS use same bundle structure
- Only difference: LD_LIBRARY_PATH vs DYLD_LIBRARY_PATH

---

## Files Modified

### Linux (branch: `main`)
- `.github/workflows/build-linux.yml`
  - Fixed binary names: `z_generals` → `GeneralsXZH` / `g_generals` → `GeneralsX`
  - Added "Deploy Bundle" step
  - Changed artifact upload to bundle

**Commit**: `f92e87976eb4cf9b69fe8a4d8d2502183ff3540d`

### macOS (branch: `macos-build`)
- `.github/workflows/build-macos.yml`
  - Added GAME_DIR variable for consistency
  - Added "Deploy Bundle" step (copies dylibs via find loop)
  - Changed artifact upload to bundle

**Commit**: `f12b71dc9d37ae296045df64d0317294412f74bd`

---

## Testing Strategy

### Manual Test (Local)
1. Download bundle artifact from GitHub Actions UI
2. Extract to test folder
3. Run: `./run.sh -win`
4. Verify game launches and reaches main menu

### Automated Test (Future Phase)
1. After bundle upload, trigger smoke test job
2. Extract bundle
3. Run with headless flags: `./run.sh -noshellmap -headless`
4. Check for graphics/audio errors
5. Exit cleanly

---

## Next Steps

- [ ] Test Linux bundle upload in GitHub Actions (push to main)
- [ ] Test macOS bundle upload in GitHub Actions (push macos-build)
- [ ] Merge macos-build into main when both pass
- [ ] Add optional smoke test job (runs after bundle upload succeeds)
- [ ] Document bundle extraction/usage in README

---

## Alignment with Local Scripts

### Linux Deployment Script
**File**: `scripts/deploy-linux-zh.sh`

Pattern matched:
- ✅ Copy binary (OUTPUT_NAME = GeneralsXZH)
- ✅ Copy DXVK libs
- ✅ Copy SDL3 libs
- ✅ Copy GameSpy lib
- ✅ Create run.sh wrapper
- ✅ Set LD_LIBRARY_PATH
- ✅ Export DXVK environment variables

### macOS Deployment Script
**File**: `scripts/deploy-macos-zh.sh` (future)

Pattern to implement:
- ✅ Copy binary (OUTPUT_NAME = GeneralsXZH/GeneralsX)
- ✅ Copy framework dylibs
- ✅ Copy MoltenVK dylib
- ✅ Create run.sh wrapper with DYLD_LIBRARY_PATH
- ⏳ patchelf for RPATH (macOS uses install_name_tool instead)

---

## References

- **Deployment Script (Linux)**: `scripts/deploy-linux-zh.sh`
- **Linux Workflow**: `.github/workflows/build-linux.yml` (main branch)
- **macOS Workflow**: `.github/workflows/build-macos.yml` (macos-build branch)
- **CMakeLists.txt (Output Names)**:
  - `GeneralsMD/Code/Main/CMakeLists.txt` → OUTPUT_NAME GeneralsXZH
  - `Generals/Code/Main/CMakeLists.txt` → OUTPUT_NAME GeneralsX
