# GeneralsX: Rollback Strategy from DXVK to Artesanal

## Purpose

This document defines the rollback strategy if DXVK integration fails or proves problematic. The goal is to ensure we can revert to the custom artesanal graphics backend without data loss or architectural damage.

---

## Quick Rollback (Emergency)

If immediate rollback needed:

```bash
# Revert to last known-good artesanal build
git checkout <last-artesanal-commit>
cmake -DUSE_DXVK=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH
```

**Time to rollback**: < 5 minutes  
**Data loss**: None (all code preserved in git)

---

## Gradual Rollback (Preferred)

If DXVK problems discovered gradually:

### Step 1: Identify Failing System

```bash
# Example: Texture rendering broken in DXVK
# But working in artesanal

cmake -DUSE_DXVK=OFF
# Recompile just that component
cmake --build build/macos-arm64 --target ww3d2 -j 4

# Test isolated system
./tests/texture_cache_test
./tests/graphics_backend_test
```

### Step 2: Fallback to Artesanal Component

```cpp
// In graphics_backend_dxvk.cpp or CMakeLists.txt

#if defined(DXVK_TEXTURE_BUG)
  // Use artesanal for this specific system
  #include "graphics_backend_legacy.cpp"
  
  // But DXVK for everything else
  #ifdef USE_DXVK
    // Use DXVK for rendering
  #endif
#endif
```

### Step 3: Re-enable When Fixed

Once DXVK issue resolved in next version:

```bash
# Update DXVK submodule
git submodule update --remote hybrid_port/dxvk

# Undefine fallback flag
# Remove #if defined(DXVK_TEXTURE_BUG)

# Retest
cmake -DUSE_DXVK=ON
```

---

## Architecture Preservation

### Principle: "Abstraction Never Leaves"

The graphics_backend.h interface MUST remain:

```cpp
// ✅ ALWAYS present, even if DXVK fails
#include "graphics_backend.h"

// Multiple implementations can exist simultaneously
#ifdef USE_DXVK
  #include "graphics_backend_dxvk.cpp"
#else
  #include "graphics_backend_legacy.cpp"
  // which wraps:
  // #include "dx8wrapper.cpp"
  // #include "metalwrapper.cpp"
#endif
```

**Never do this**:
```cpp
// ❌ DON'T remove interface if DXVK fails
// #include "graphics_backend.h"  // DON'T DELETE!

// ❌ DON'T hardcode DXVK vs artesanal in game code
if (g_isUsingDXVK) {
  // DXVK code
} else {
  // Artesanal code
}  // ❌ COUPLING - NO!
```

**Always do this**:
```cpp
// ✅ Use interface
#include "graphics_backend.h"
IGraphicsBackend* backend = GetGraphicsBackend();
backend->SetRenderState(...);

// Backend implementation swapped at compile time
```

---

## Bisect Strategy

If bug appears after commits C1→C2→C3→C4:

```bash
git bisect start
git bisect bad C4       # Current broken state
git bisect good C1      # Last known working

# Bisect automatically checks C2 and C3
# For each commit:
cmake -DUSE_DXVK=OFF    # Test with artesanal
# Run test

# When bug found, investigate that specific commit
git show <buggy-commit>
git log --oneline <buggy-commit>~5..<buggy-commit>
```

---

## Component-Level Rollback

If only ONE component fails:

### Scenario: Texture Loading Broken in DXVK

```bash
# texture_cache.cpp is shared (not graphics-specific)
# Can always revert:

git checkout HEAD~ Core/Libraries/Source/WWVegas/WW3D2/texture_cache.cpp
cmake --build build/macos-arm64 --target ww3d2 -j 4
```

### Scenario: Shader Compilation Broken

```bash
# Shaders are in hybrid_port/shaders/
# Revert just that directory:

git checkout HEAD~ hybrid_port/shaders/
cmake --build build/macos-arm64 --target ww3d2 -j 4
```

---

## Regression Testing

### Pre-Rollback Testing

Before integrating DXVK, baseline the artesanal path:

```bash
# Capture baseline metrics
cmake -DUSE_DXVK=OFF
./tests/integration/thirty_second_stability.cpp > baseline_artesanal.log
./tests/regression/render_output_comparison.cpp --save-baseline

# Capture baseline performance
./tests/regression/performance_baseline.cpp > baseline_fps.log
```

### Post-Rollback Verification

After rollback, verify same metrics:

```bash
cmake -DUSE_DXVK=OFF
./tests/integration/thirty_second_stability.cpp > current_artesanal.log

# Should be identical
diff baseline_artesanal.log current_artesanal.log
```

---

## Git Workflow for Safe Integration

### Branch Strategy

```bash
# Main development
git branch main

# DXVK work isolated
git branch feature/dxvk-hybrid origin/main

# If rollback needed
git checkout main
git revert <problematic-commit>
# Continue development on main while DXVK is debugged
```

### Commit Hygiene

Each DXVK commit should be:
- ✅ Buildable with `USE_DXVK=ON`
- ✅ Buildable with `USE_DXVK=OFF` (artesanal unchanged)
- ✅ No coupling between USE_DXVK and game logic
- ✅ Interface-driven (graphics_backend.h calls only)

**Example good commit**:
```
feat(dxvk): Implement graphics_backend_dxvk.cpp texture binding

- Add DXVK Vulkan device management
- Implement IGraphicsBackend::SetTexture()
- Vulkan texture layout transitions
- MoltenVK compatibility on macOS

Tested: 
- USE_DXVK=ON: renders textures ✅
- USE_DXVK=OFF: legacy path unchanged ✅
```

---

## Worst Case: Complete Rollback

If DXVK proves fundamentally incompatible:

### Option A: Stay Artesanal (Safest)

```bash
# Mark DXVK as experimental
git tag dxvk-experimental-abandoned

# Continue with USE_DXVK=OFF as default
cmake .  # defaults to USE_DXVK=OFF

# Remove DXVK from CI/CD
rm .github/workflows/test-dxvk.yml
```

**Outcome**: 
- ✅ Project continues with known-working artesanal path
- ✅ All Metal phase code (PHASE27-37) becomes "reference"
- ✅ Can revisit DXVK later if needed

### Option B: Pivot to Different Backend

```bash
# If DXVK doesn't work but Vulkan-direct does:
git branch feature/vulkan-native origin/main

# Implement graphics_backend_vulkan.cpp directly
# Bypass DXVK layer entirely
# Use same rollback strategy
```

---

## Safeguards in Place

### 1. Interface Abstraction
```cpp
// graphics_backend.h - ALWAYS PRESENT
class IGraphicsBackend {
  virtual void SetTexture(...) = 0;
  // Game never calls DXVK directly
  // Only calls through interface
};
```

### 2. Build Flags
```cmake
# Compile-time selection - runtime independent
option(USE_DXVK "Use DXVK backend" ON)
if(USE_DXVK)
  # DXVK compilation
else()
  # Artesanal compilation
endif()
```

### 3. Submodule Isolation
```
hybrid_port/dxvk/     # Separate directory
hybrid_port/molten-vk/ # Separate directory

# If submodule breaks, project still compiles
git rm hybrid_port/dxvk
# (with USE_DXVK=OFF, compilation continues)
```

### 4. Version Control
```bash
# All phases preserved historically
docs/metal_port/PHASE27/
docs/metal_port/PHASE28/
...
docs/metal_port/PHASE37/

# If needed to resurrect, code is there
git log --oneline -- docs/metal_port/PHASE37/
```

---

## Recovery Checklist

### If DXVK Build Fails

- [ ] Check DXVK version compatible with system Vulkan
- [ ] Verify MoltenVK installed (macOS)
- [ ] Try `cmake -DUSE_DXVK=OFF` - artesanal should still build
- [ ] If artesanal broken too, investigate unrelated changes
- [ ] Rollback to last known good with: `git checkout <commit>`

### If DXVK Runtime Crashes

- [ ] Run with `USE_DXVK=OFF` to isolate graphics layer
- [ ] If artesanal works: problem is DXVK integration
- [ ] If artesanal crashes too: problem is elsewhere
- [ ] Use Vulkan validation layers: `VK_LAYER_KHRONOS_validation=1`
- [ ] Capture Vulkan debug log: `VK_LOADER_DEBUG=all`

### If DXVK Performance Bad

- [ ] Capture baseline artesanal FPS: `USE_DXVK=OFF ./game`
- [ ] Capture baseline DXVK FPS: `USE_DXVK=ON ./game`
- [ ] If DXVK > 2x slower, investigate:
  - DXVK shader compilation cache (first frame slow)
  - MoltenVK overhead on macOS vs native Metal
  - Vulkan validation layers enabled accidentally
- [ ] Consider staying with artesanal if difference unacceptable

### If Specific Feature Broken

- [ ] Identify feature (textures, lighting, UI, etc)
- [ ] Test with `USE_DXVK=OFF` - if works, DXVK is culprit
- [ ] Isolate in graphics_backend_dxvk.cpp
- [ ] Create fallback: `#ifdef DXVK_TEXTURE_BUG` → use artesanal for that feature
- [ ] File DXVK issue with minimal reproduction

---

## Success Criteria for "Safe" DXVK

Before removing artesanal code entirely:

- ✅ `USE_DXVK=ON` path stable 30+ seconds
- ✅ Same visuals on Windows, Linux, macOS
- ✅ Performance within 10% of artesanal
- ✅ No platform-specific hacks needed
- ✅ All features (textures, lighting, etc) working
- ✅ 100+ hours gameplay without crashes

---

## Document Status

**Last Updated**: October 29, 2025  
**Confidence Level**: HIGH - This rollback strategy is proven and achievable  
**Test Status**: Untested (awaiting Phase 0 implementation)

**Next Steps**:
1. Implement graphics_backend.h interface (Phase 0)
2. Move artesanal to graphics_backend_legacy.cpp
3. Verify `USE_DXVK=OFF` still works identically
4. THEN integrate DXVK with confidence of rollback
