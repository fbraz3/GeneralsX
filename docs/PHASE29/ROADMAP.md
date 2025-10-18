# Phase 29: Lighting & Effects - Roadmap

**Project**: GeneralsX (Command & Conquer: Generals Zero Hour macOS Port)  
**Start Date**: October 18, 2025  
**Status**: Planning  
**Prerequisites**: Phase 27 (OpenGL Complete), Phase 28.6 (Debug Cleanup), Phase 30 (Metal Backend)

---

## Phase 29 Overview

### Objectives
Validate and enhance lighting, fog, and advanced rendering effects already implemented in Phase 27.4, and add Metal backend support where needed.

### Implementation Status

| Feature | Phase 27.4 (OpenGL) | Metal Backend | Status |
|---------|---------------------|---------------|--------|
| **Directional Lighting** | ✅ COMPLETE | ⏳ Pending | Uniforms implemented (uLightDirection, uLightColor, uAmbientColor) |
| **Fog Rendering** | ✅ COMPLETE | ⏳ Pending | LINEAR/EXP/EXP2 modes with color/density uniforms |
| **Stencil Buffer** | ✅ COMPLETE | ⏳ Pending | Complete operations (KEEP/ZERO/REPLACE/INCR/DECR/INVERT) |
| **Point Sprites** | ✅ COMPLETE | ⏳ Pending | Distance scaling with A/B/C coefficients + size clamping |

### Key Discovery
**Phase 27.4 already implemented all Phase 29 features for OpenGL!**  
- Fog rendering: `D3DRS_FOGENABLE`, `D3DRS_FOGCOLOR`, `D3DRS_FOGSTART`, `D3DRS_FOGEND`, `D3DRS_FOGDENSITY`
- Lighting: `uLightDirection`, `uLightColor`, `uAmbientColor`, `uUseLighting` uniforms
- Stencil: Full operations in `Set_DX8_Render_State()` switch cases
- Point sprites: Distance scaling implemented

**Phase 29 Focus**: Add Metal backend support for these features

---

## Phase 29.1: Metal Lighting Support (2-3 days)

### Objective
Port OpenGL lighting uniforms to Metal shaders

### Tasks
1. **Add lighting uniforms to `basic.metal`**:
   - `float3 uLightDirection`
   - `float3 uLightColor`
   - `float3 uAmbientColor`
   - `bool uUseLighting`

2. **Update MetalWrapper uniform functions**:
   - Add `SetLightDirection(float x, float y, float z)`
   - Add `SetLightColor(float r, float g, float b)`
   - Add `SetAmbientColor(float r, float g, float b)`
   - Add `SetUseLighting(bool enabled)`

3. **Integrate with `Set_DX8_Light()` in dx8wrapper.cpp**:
   ```cpp
   #ifdef __APPLE__
   if (g_useMetalBackend) {
       MetalWrapper::SetLightDirection(dir.x, dir.y, dir.z);
       MetalWrapper::SetLightColor(color.r, color.g, color.b);
   }
   #endif
   ```

4. **Test with game lighting scenarios**:
   - Day/night cycles
   - Indoor/outdoor lighting
   - Dynamic shadows

**Files to Modify**:
- `resources/shaders/basic.metal` - Add lighting uniforms and calculations
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/mm` - Add lighting API
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - Integrate Metal lighting

---

## Phase 29.2: Metal Fog Support (2-3 days)

### Objective
Port OpenGL fog rendering to Metal shaders

### Tasks
1. **Add fog uniforms to `basic.metal`**:
   - `float4 uFogColor`
   - `float uFogStart`
   - `float uFogEnd`
   - `float uFogDensity`
   - `int uFogMode` (0=LINEAR, 1=EXP, 2=EXP2)

2. **Implement fog calculation in fragment shader**:
   ```metal
   float fogFactor = 1.0;
   if (uFogMode == 0) { // LINEAR
       fogFactor = (uFogEnd - distance) / (uFogEnd - uFogStart);
   } else if (uFogMode == 1) { // EXP
       fogFactor = exp(-uFogDensity * distance);
   } else if (uFogMode == 2) { // EXP2
       fogFactor = exp(-uFogDensity * uFogDensity * distance * distance);
   }
   fogFactor = clamp(fogFactor, 0.0, 1.0);
   finalColor = mix(uFogColor, color, fogFactor);
   ```

3. **Update MetalWrapper fog functions**:
   - Add `SetFogColor(float r, float g, float b, float a)`
   - Add `SetFogRange(float start, float end)`
   - Add `SetFogDensity(float density)`
   - Add `SetFogMode(int mode)`

4. **Integrate with `Set_DX8_Render_State()` fog cases**:
   - `D3DRS_FOGENABLE`
   - `D3DRS_FOGCOLOR`
   - `D3DRS_FOGSTART` / `D3DRS_FOGEND`
   - `D3DRS_FOGDENSITY`
   - `D3DRS_FOGVERTEXMODE` / `D3DRS_FOGTABLEMODE`

**Files to Modify**:
- `resources/shaders/basic.metal` - Add fog calculations
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/mm` - Add fog API
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - Integrate Metal fog

---

## Phase 29.3: Metal Stencil Buffer (1-2 days)

### Objective
Port OpenGL stencil operations to Metal render pipeline

### Tasks
1. **Create MTLDepthStencilDescriptor in MetalWrapper**:
   - Stencil test enable/disable
   - Stencil read/write masks
   - Stencil reference value

2. **Map D3D stencil operations to Metal**:
   - `D3DSTENCILOP_KEEP` → `MTLStencilOperationKeep`
   - `D3DSTENCILOP_ZERO` → `MTLStencilOperationZero`
   - `D3DSTENCILOP_REPLACE` → `MTLStencilOperationReplace`
   - `D3DSTENCILOP_INCR` → `MTLStencilOperationIncrementClamp`
   - `D3DSTENCILOP_DECR` → `MTLStencilOperationDecrementClamp`
   - `D3DSTENCILOP_INVERT` → `MTLStencilOperationInvert`

3. **Update MetalWrapper stencil API**:
   - Add `SetStencilEnable(bool enabled)`
   - Add `SetStencilFunc(int func, int ref, uint mask)`
   - Add `SetStencilOp(int fail, int zfail, int pass)`
   - Add `SetStencilMask(uint mask)`

4. **Integrate with `Set_DX8_Render_State()` stencil cases**:
   - `D3DRS_STENCILENABLE`
   - `D3DRS_STENCILFUNC`
   - `D3DRS_STENCILREF`
   - `D3DRS_STENCILMASK`
   - `D3DRS_STENCILWRITEMASK`
   - `D3DRS_STENCILFAIL` / `D3DRS_STENCILZFAIL` / `D3DRS_STENCILPASS`

**Files to Modify**:
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/mm` - Add stencil API
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - Integrate Metal stencil

---

## Phase 29.4: Metal Point Sprites (1-2 days)

### Objective
Port OpenGL point sprite distance scaling to Metal

### Tasks
1. **Add point size uniforms to `basic.metal`**:
   - `float uPointSize`
   - `float3 uPointScaleFactors` (A, B, C coefficients)
   - `float uPointSizeMin`
   - `float uPointSizeMax`

2. **Implement distance-based point size calculation**:
   ```metal
   float distance = length(viewPosition);
   float attenuation = 1.0 / (uPointScaleFactors.x + 
                              uPointScaleFactors.y * distance + 
                              uPointScaleFactors.z * distance * distance);
   float pointSize = uPointSize * attenuation;
   pointSize = clamp(pointSize, uPointSizeMin, uPointSizeMax);
   ```

3. **Update MetalWrapper point sprite API**:
   - Add `SetPointSize(float size)`
   - Add `SetPointScaleFactors(float a, float b, float c)`
   - Add `SetPointSizeRange(float min, float max)`

4. **Integrate with `Set_DX8_Render_State()` point sprite cases**:
   - `D3DRS_POINTSIZE`
   - `D3DRS_POINTSCALE_A` / `D3DRS_POINTSCALE_B` / `D3DRS_POINTSCALE_C`
   - `D3DRS_POINTSIZE_MIN` / `D3DRS_POINTSIZE_MAX`

**Files to Modify**:
- `resources/shaders/basic.metal` - Add point sprite calculations
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/mm` - Add point sprite API
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - Integrate Metal point sprites

---

## Phase 29.5: Testing & Validation (2-3 days)

### Objective
Validate all Phase 29 Metal features with runtime testing

### Tasks
1. **Create test scenarios**:
   - Lighting: Day/night cycles, indoor/outdoor scenes
   - Fog: Different modes (LINEAR/EXP/EXP2), varying density
   - Stencil: Shadow volumes, UI masking
   - Point sprites: Particle effects, explosions

2. **Runtime validation**:
   - Compare Metal vs OpenGL rendering side-by-side
   - Measure performance (FPS, memory usage)
   - Check for visual artifacts

3. **Debug logging**:
   - Add Phase 29 debug logs for Metal feature activation
   - Track uniform updates and render state changes

4. **Documentation**:
   - Update MACOS_PORT.md with Phase 29 progress
   - Create PHASE29/COMPLETION_SUMMARY.md

**Files to Create/Modify**:
- `docs/PHASE29/COMPLETION_SUMMARY.md` - Phase 29 results
- `docs/MACOS_PORT.md` - Update with Phase 29 status
- `docs/NEXT_STEPS.md` - Mark Phase 29 complete

---

## Timeline Summary

| Phase | Description | Estimated Time | Dependencies |
|-------|-------------|----------------|--------------|
| **29.1** | Metal Lighting Support | 2-3 days | Phase 27.3 (lighting uniforms) |
| **29.2** | Metal Fog Support | 2-3 days | Phase 27.4 (fog rendering) |
| **29.3** | Metal Stencil Buffer | 1-2 days | Phase 27.4 (stencil ops) |
| **29.4** | Metal Point Sprites | 1-2 days | Phase 27.4 (point sprites) |
| **29.5** | Testing & Validation | 2-3 days | Phases 29.1-29.4 |
| **TOTAL** | **5 Phases** | **8-13 days** | Phase 27 OpenGL complete |

**Estimated Completion**: October 26 - November 3, 2025

---

## Key Files for Phase 29

### Metal Shaders
- `resources/shaders/basic.metal` - Add lighting, fog, point sprite calculations

### Metal Wrapper
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h` - API declarations
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm` - API implementations

### DirectX8 Wrapper
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - Integrate Metal features
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` - Function declarations

### Reference Implementation
- `references/fighter19-dxvk-port/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/` - DXVK patterns
- `references/jmarshall-win64-modern/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/` - Working examples

---

## Success Criteria

### Phase 29.1 - Lighting ✅
- [  ] Metal shaders accept lighting uniforms
- [  ] SetLightDirection/Color/Ambient functions work
- [  ] Day/night lighting renders correctly

### Phase 29.2 - Fog ✅
- [  ] Metal shaders calculate fog (LINEAR/EXP/EXP2)
- [  ] SetFogColor/Range/Density functions work
- [  ] Fog blending looks natural

### Phase 29.3 - Stencil ✅
- [  ] MTLDepthStencilDescriptor configured
- [  ] Stencil operations mapped to Metal
- [  ] Shadow volumes render correctly

### Phase 29.4 - Point Sprites ✅
- [  ] Metal shaders calculate distance-based size
- [  ] SetPointSize/ScaleFactors functions work
- [  ] Particle effects look correct

### Phase 29.5 - Validation ✅
- [  ] Metal matches OpenGL visually
- [  ] 30 FPS stable with all features enabled
- [  ] Zero crashes or memory leaks

---

## Next Steps After Phase 29

**Phase 30+**: Continue with audio, networking, and gameplay systems as documented in NEXT_STEPS.md
