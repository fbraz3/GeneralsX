# Phase 39: Graphics Backend Strategy - REVISED FOR MACOS ARM64

**Status**: 🟡 **REVISED** (Oct 30, 2025) - Using Metal-native approach

**Objective**: Implement production-grade graphics backend for macOS Apple Silicon

**Timeline**: 1-2 days (architecture decision + implementation)

---

## Important Discovery: DXVK Not Available on macOS ARM64

### What We Found
- ✅ DXVK exists but requires Linux (main platform)
- ✅ MoltenVK installed but requires x86_64 pre-built binaries
- ❌ Rebuilding MoltenVK requires full Xcode.app (not just Command Line Tools)
- ⚠️ Vulkan on macOS is **not** performance-optimal for Apple Silicon

### Strategic Decision

Instead of forcing Vulkan/MoltenVK on macOS, **use Metal directly** (native, optimized):

```
Phase 38.4 abstraction layer stays (✅ proven working)
    ↓
Phase 39a: Optimize Metal backend (Phase 27-37) instead of replacing
    ↓
Phase 39b: Add fallback OpenGL for Linux
    ↓
Result: Best performance per platform (Metal on Mac, OpenGL on Linux)
```

---

## The Real Problem Phase 39 Should Solve

**Original Phase 38/39 goal**: "Eliminate Metal deadlock"

**Reality check**: 
- ✅ Phase 38.4 delegation works perfectly
- ✅ Metal hang happens only on extended runs (90+ seconds)
- ✅ Game initializes and runs fine for gameplay duration
- ⚠️ Root cause: Autoreleasepool timing in BeginFrame()

**Phase 39 Revised Goal**: Fix Metal backend properly (not replace)

---

## Phase 39 Breakdown (REVISED)

### Phase 39.1: Metal Backend Optimization ✅ START HERE
- Add proper Objective-C autoreleasepool wrapping
- Fix render encoder creation deadlock
- Implement frame timeout handling
- Optimize Metal render state caching

### Phase 39.2: Performance Profiling
- Measure FPS stability (target: 30 constant)
- Profile GPU utilization
- Identify bottlenecks
- Compare: Phase 27-37 vs Phase 39 optimized

### Phase 39.3: Extended Gameplay Testing  
- 60+ minute continuous session
- Monitor memory usage
- Verify no regressions
- Capture performance metrics

### Phase 39.4: Linux OpenGL Backend (Optional)
- Create fallback for Linux testing
- Use same IGraphicsBackend interface (Phase 38!)
- Test compilation on Linux VM if available

### Phase 39.5: Documentation
- Finalize Metal optimization findings
- Document performance baseline
- Create platform-specific guidelines
- Prepare for Phase 40 (gameplay features)

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

## Phase 39.1: Metal Backend Optimization Instructions

### Root Cause of Current Metal Hang

Current issue in `metalwrapper.mm`:
```cpp
// BeginFrame() creates render encoder
s_renderEncoder = [(id<MTLCommandBuffer>)s_cmdBuffer renderCommandEncoderWithDescriptor:pass];
// ↑ This can deadlock if autoreleasepool is not active
```

**Problem**: Metal framework creates internal Objective-C objects during encoder creation, but no autoreleasepool exists to clean them up.

### Phase 39.1 Task: Complete Metal Backend Hardening

**File**: `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.mm`

**Changes needed**:

1. **Add autoreleasepool to BeginFrame()** ✅ DONE (preliminary fix)
   - Wraps render encoder creation
   - Prevents autoreleasepool exhaustion

2. **Add timeout handling**
   - Detect if encoder creation hangs
   - Implement failsafe timeout
   - Log detailed diagnostics

3. **Optimize state transitions**
   - Batch Metal calls
   - Reduce per-frame allocations
   - Cache pipeline states

4. **Frame rate limiting**
   - Ensure 30 FPS cap
   - Prevent GPU stalls
   - Monitor frame times

### Implementation Checklist

- [ ] Review current autoreleasepool fix
- [ ] Add timeout detection (5-second watchdog)
- [ ] Implement metal state caching
- [ ] Add frame time profiling
- [ ] Test 60-minute gameplay session
- [ ] Document performance baseline

### Expected Results After Phase 39.1

✅ Game runs 30+ minutes without hang
✅ Consistent 30 FPS frame rate
✅ Metal backend optimized for Apple Silicon
✅ Ready for Phase 40 (gameplay features)

---

## Phase 39.1 Detailed Implementation

### Step 1: Verify Current Autoreleasepool Fix

Check that `metalwrapper.mm` has proper autoreleasepool:

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
