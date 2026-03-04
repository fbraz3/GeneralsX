# Terrain Shader Metal Compilation Fix — Session 67

**Status**: In Progress
**Priority**: CRITICAL — Blocking terrain rendering
**Root Cause**: DXVK Patch 13 needs to be re-applied to DXVK macOS

---

## The Problem

### Log Error
```log
[mvk-error] VK_ERROR_INITIALIZATION_FAILED: Shader library compile failed (Error code 3):
program_source:393:122: error: use of undeclared identifier 's0_2d_shadowSmplr'
```

### Why It Occurs
1. DXVK processes D3D8 pixel shaders (PS1.x) always using "implicit mode"
2. PS1.x forces `majorVersion() < 2` + `forceSamplerTypeSpecConstants` → emits ALL 5 sampler types in SPIR-V:
   - `s0_2d`, `s0_2d_shadow`
   - `s0_3d`
   - `s0_cube`, `s0_cube_shadow`
3. SPIRV-Cross generates a Metal shader entry point that passes ALL types as parameters:
   ```metal
   ps_main(v, ..., s0_2d, s0_2dSmplr, s0_2d_shadow, s0_2d_shadowSmplr, s0_3d, s0_3dSmplr, s0_cube, s0_cubeSmplr, ...)
   ```
4. MoltenVK MSL wrapper only initializes descriptors that are **actually used** (only 2D for terrain):
   ```metal
   s0_2d = spvDescriptorSet0.s0_2d         // OK
   s0_2dSmplr = spvDescriptorSet0.s0_2dSmplr   // OK
   // s0_2d_shadowSmplr, s0_3d, etc. DO NOT EXIST
   ```
5. Metal compiler sees `ps_main(..., s0_2d_shadow, s0_2d_shadowSmplr, ...)` as undeclared references → ERROR

---

## The Solution: DXVK Patch 13

**Location**: `cmake/dxvk-macos-patches.py` (line 596+)

**Target**: `build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp`

### Change 1: emitDclSampler() ~line 754
**Before**:
```cpp
const bool implicit = m_programInfo.majorVersion() < 2 || m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

**After**:
```cpp
// GeneralsX Patch 13: Remove PS1.x auto-trigger
const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

**Effect**: With `forceSamplerTypeSpecConstants=False` (default), PS1.x shaders now use the detected type (2D), not all 5 types.

### Change 2: emitTextureSample() ~line 3015
**Before**:
```cpp
if (m_programInfo.majorVersion() >= 2 && !m_moduleInfo.options.forceSamplerTypeSpecConstants) {
```

**After**:
```cpp
if (!m_moduleInfo.options.forceSamplerTypeSpecConstants) {
```

**Effect**: For PS1.x, does not emit an `OpSwitch` over spec constants; emits the detected type directly.

---

## Implementation Steps

### 1. Reconfigure CMake
```bash
cd /path/to/GeneralsX
rm -rf build/macos-vulkan
cmake --preset macos-vulkan
```
- CMake runs `FetchContent_MakeAvailable(dxvk)` → downloads pristine DXVK
- Then calls `add_custom_command` which runs `cmake/dxvk-macos-patches.py`
- Script applies Patch 13 + other patches

### 2. Recompile DXVK
```bash
cmake --build build/macos-vulkan --target dxvk_compiler --parallel
```

### 3. Recompile GeneralsXZH
```bash
cmake --build build/macos-vulkan --target z_generals --parallel
```

### 4. Deploy and Test
```bash
./scripts/deploy-macos-zh.sh
./scripts/run-macos-zh.sh -win
```

---

## Diagnostic Checks

### Verify Patch Applied
```bash
grep -A5 "GeneralsX Patch 13" build/macos-vulkan/_deps/dxvk-src/src/dxso/dxso_compiler.cpp
# Should show:
# const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;
```

### Verify Metal Shader Compilation
```bash
# In the Xcode build output, look for "Metal shader compiled"
# Terrain should render WITHOUT "undeclared identifier" errors
```

---

## Expected Result

- Terrain renders with visible textures
- Main menu loads without shader errors
- No `VK_ERROR_INITIALIZATION_FAILED` errors in shaders

---

## Related Documentation
- `docs/WORKDIR/lessons/2026-02-LESSONS.md` — Lesson: "DXVK PS1.x Sampler Spec Constants"
- `docs/DEV_BLOG/2026-02-DIARY.md` — Session 66 entry
