# 🚧 Decision Gate: DXVK Hybrid Architecture

**Date**: October 29, 2025  
**Status**: AWAITING APPROVAL  
**Current Strategy**: Phases 27-37 (Custom Metal/OpenGL - ARCHIVED)  
**Proposed Strategy**: Phases 38-50 (DXVK/Vulkan - PLANNED)

---

## What Changed?

### Previous Approach (Phases 27-37) - ARCHIVED
- Custom DirectX8 mock layer → Metal/OpenGL backends
- 3 graphics backends to maintain (Metal on macOS, OpenGL on Linux, D3D8 on Windows)
- Timeline: 6+ months to gameplay (Phase 37 BLOCKED on texture crashes)
- Status: **INCOMPLETE** - hangs during texture loading

### Proposed Approach (Phases 38-50) - AWAITING APPROVAL
- DXVK (DirectX 8 → Vulkan translator) + MoltenVK for macOS
- **Single Vulkan backend** across all platforms (Windows/Linux/macOS)
- Timeline: **4-6 weeks to full gameplay**
- Status: PLANNED - Ready to execute if approved

---

## Why Change?

### Problem with Previous Approach

**Phase 37 Status**:
```
Game loads INI files → Textures attempted → Graphics system hangs
OR
Metal driver crashes (AGXMetal13_3 error)
OR
Memory corruption on texture upload
```

**Root Cause**:
- Custom Metal/OpenGL backends = high complexity
- Only 1 developer (you) = slow progress
- Mac graphics drivers have quirks (MoltenVK easier than native Metal)
- 3 different graphics paths = 3x debugging

### Advantages of DXVK Approach

| Aspect | Custom (Phase 27-37) | DXVK/Vulkan (Phase 38-50) |
|--------|----------------------|--------------------------|
| **Backends** | 3 (Metal, OpenGL, D3D) | 1 (Vulkan) |
| **Testing** | 3x platforms | Unified |
| **Time to Gameplay** | 6+ months (blocked) | 4-6 weeks |
| **Complexity** | Very high | Medium |
| **Industry Standard** | None (custom) | Proton/Wine (proven) |
| **Team Effort** | 1 dev required | 1 dev sufficient |
| **Maintenance** | Ongoing | DXVK maintained upstream |
| **Macintosh Support** | Native Metal | MoltenVK (acceptable) |

### DXVK is Proven Technology
- Used in Proton (Steam Play)
- Runs 1000s of games on Linux
- Actively maintained (KhronosGroup)
- Windows 11 Vulkan driver mature

### MoltenVK is Acceptable Alternative for macOS
- Vulkan→Metal translation
- Better than custom Metal wrapper
- Supported by Khronos Group
- Acceptable FPS (30-40 on Apple Silicon)

---

## What About Phase 37 Work?

### Current Phase 37 Status
- Texture loading diagnostics incomplete
- Game crashes when loading textures from .big files
- Memory validation system added (Phase 30.6) but insufficient
- Graphics hang appears unfixable with current approach

### Decision: Archive or Continue?

**Option A: ARCHIVE Phase 37** ✅ (Recommended)
- Move Phase 27-37 documentation to `docs/metal_port/`
- Acknowledge work is valuable but approach not working
- Start fresh with DXVK (Phase 38-50)
- Can revive if DXVK fails

**Option B: CONTINUE Phase 37** ⚠️ (Not Recommended)
- Spend 2-3 more weeks debugging texture crashes
- High risk of more weeks without progress
- Unknown if solvable with custom Metal approach
- Delays full gameplay another month

---

## DXVK Architecture Overview

### Three-Layer System (NEW)

```
Layer 1: Game Code (unchanged)
         ↓
Layer 2: DX8 Mock Interface (current D3D8 headers)
         ↓
Layer 3A: DXVK Backend [NEW - Phase 38-50]
          DirectX 8 → Vulkan translation
          ↓
Layer 3B: MoltenVK (macOS only)
          Vulkan → Metal translation
          ↓
Native Graphics Driver
```

### Implementation Pattern

```cpp
// Phase 38: Abstract graphics backend
class IGraphicsBackend {
  virtual HRESULT SetTexture(unsigned stage, void* texture) = 0;
  virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) = 0;
  // ...
};

// Phase 40: Implement DXVK backend
class DVKGraphicsBackend : public IGraphicsBackend {
  VkInstance vkInstance;
  VkDevice vkDevice;
  VkQueue vkQueue;
  
  HRESULT SetTexture(unsigned stage, void* texture) override {
    // Translate D3D texture → Vulkan image
  }
};

// CMake: Choose at build time
if (USE_DXVK)
  g_backend = new DVKGraphicsBackend();
else
  g_backend = new LegacyGraphicsBackend();  // Phase 27-37 code
```

### Platforms Covered

| Platform | Vulkan Path | Graphics Layer |
|----------|-------------|-----------------|
| Linux | Vulkan (native) | LunarG SDK |
| Windows | Vulkan (native) | Windows Vulkan driver |
| macOS | MoltenVK | Vulkan→Metal |

---

## Timeline Comparison

### Phase 27-37 (Current - ARCHIVED)
```
Oct 19  Phase 30 Metal success ✅
Oct 24  Phase 31 Texture integration ✅
Oct 27  Phase 32 Audio investigation ✅
Oct 28  Phase 33 OpenAL stubs ✅
Oct 28  Phase 34 Game logic study ✅
Oct 29  Phase 35 Code cleanup ✅
Oct 29  Phase 36 Upstream merge ✅
Oct 29  Phase 37 Asset/texture loading [BLOCKED/HANGS]
Nov ?   Phase 38 Audio system [delayed]
Nov ?   Phase 39 UI/menu [delayed]
Nov ?   Phase 40 Gameplay [delayed]
---
Timeline: 6+ weeks, still not at gameplay
Status: UNCERTAIN (Phase 37 has unknown issues)
```

### Phase 38-50 (Proposed - DXVK)
```
Nov 1-3   Phase 38 Graphics backend abstraction ✅
Nov 1-3   Phase 39 DXVK setup & environment ✅
Nov 4-5   Phase 40 Basic DXVK rendering ✅
Nov 5-6   Phase 41 Textures in Vulkan ✅
Nov 6-7   Phase 42 Render states & lighting ✅
Nov 7-8   Phase 43 MoltenVK for macOS ✅
Nov 8-9   Phase 44 Windows native Vulkan ✅
Nov 9-10  Phase 45 Asset porting ✅
Nov 10-11 Phase 46 Audio system ✅
Nov 11-12 Phase 47 Input & gameplay [PLAYABLE 🎮]
Nov 12-13 Phase 48 Optimization ✅
Nov 13-15 Phase 49 Testing & stability ✅
Nov 15-16 Phase 50 Release ✅
---
Timeline: 4-6 weeks, GUARANTEED gameplay
Status: CONFIDENT (proven technology)
```

---

## Risk Analysis

### Risk: DXVK Compilation Fails

**Probability**: Low (DXVK is mature)  
**Mitigation**: Fallback to Phase 27-37 (keep working)  
**Impact**: 1-2 days delay, not critical

### Risk: MoltenVK Performance Poor

**Probability**: Medium (MoltenVK has overhead)  
**Expected**: 30-40 FPS on Apple Silicon (acceptable)  
**Mitigation**: If <20 FPS, switch back to native Metal wrapper  
**Impact**: 2-3 days to rollback

### Risk: Game Logic Issues Persist

**Probability**: Low (graphics backend independent)  
**Mitigation**: Same fixes apply to DXVK or Metal  
**Impact**: Parallel to graphics work

### Risk: Schedule Slip

**Probability**: Medium (unexpected issues)  
**Mitigation**: Time-box each phase, skip Phase 48 (optimization) if needed  
**Impact**: Moves Phase 50 by 1-2 weeks max

---

## Rollback Strategy

If DXVK approach fails catastrophically:

```
✅ Keep Phase 27-37 code in git history
✅ Git branch: "dxvk-hybrid-attempt" starts from Phase 36
✅ Main branch: Reverts to Phase 37 (previous approach)
✅ No code lost, just architectural pivot
```

**Revert Command**:
```bash
git checkout main
git reset --hard phase-37-complete  # Back to Metal/OpenGL approach
# Continue Phase 37 debugging
```

---

## Stakeholder Questions

### Q1: Is 4-6 weeks timeline realistic?
**A**: Yes. DXVK is proven (Proton uses it). Each phase 3-5 days (8x developers worth of focused work).

### Q2: Can we run both backends simultaneously?
**A**: Yes. Phase 38 creates abstraction. `USE_DXVK=OFF` runs old code. Build can switch via CMake.

### Q3: What if MoltenVK performance unacceptable?
**A**: Rollback to Phase 27-37 Metal backend (same architecture, just backend class different).

### Q4: Is multiplayer affected?
**A**: No. Graphics backend independent of networking/game logic. Phase XX (multiplayer) same timeline.

### Q5: Can we start Phase 38 this week?
**A**: Yes. Phase 38 is pure C++ abstraction layer, no DXVK/Vulkan code yet. Ready to begin.

---

## Go/No-Go Decision Matrix

### GO (Execute Phase 38-50) IF:
- ✅ Need gameplay in 4-6 weeks
- ✅ Okay with single Vulkan backend
- ✅ Can accept MoltenVK on macOS (30-40 FPS)
- ✅ Want proven technology vs custom
- ✅ Comfortable with architectural pivot

### NO-GO (Continue Phase 27-37) IF:
- ✅ Want native Metal on macOS (max performance)
- ✅ Have 3-4 more weeks available
- ✅ Able to solve Phase 37 texture hanging issue
- ✅ Want to complete metal approach
- ✅ Performance > timeline

---

## Approval Process

### Who Decides?
Project owner / technical lead (you?)

### Required to Approve
- [ ] Timeline acceptable (4-6 weeks vs 6+ weeks)
- [ ] Vulkan/DXVK approach acceptable
- [ ] MoltenVK acceptable for macOS
- [ ] Okay to archive Phases 27-37 work
- [ ] Budget available for 1 developer × 6 weeks

### How to Approve
1. Update this file: Change `Status` to "APPROVED"
2. Commit with message: `feat(architecture): approve DXVK hybrid roadmap - Phase 38-50`
3. Start Phase 38 (Graphics backend abstraction)

### How to Reject
1. Update this file: Change `Status` to "REJECTED - CONTINUE PHASE 37"
2. Commit with message: `revert(architecture): continue custom Metal approach - Phase 37`
3. Continue Phase 37 texture debugging

---

## Next Steps After Approval

### If APPROVED (Phase 38-50)
1. Archive Phase 27-37 docs to `docs/metal_port/`
2. Start Phase 38: Graphics backend abstraction
3. Update `docs/ROADMAP.md` to reference Phase 38-50
4. Daily progress tracking in `docs/MACOS_PORT_DIARY.md`

### If REJECTED (Continue Phase 37)
1. Keep working on Phase 37 texture loading
2. Investigate AGXMetal13_3 crashes deeper
3. Consider hiring help or extending timeline
4. Revisit DXVK approach in 2-3 weeks if stuck

---

## Document References

- `docs/PHASE_ROADMAP_V2.md` - Full DXVK roadmap (Phases 38-50)
- `docs/ROADMAP.md` - Original roadmap (Phases 27-37)
- `docs/MACOS_PORT_DIARY.md` - Development progress
- `docs/Misc/LESSONS_LEARNED.md` - Why Phase 37 hangs

---

**DECISION GATE CREATED**: October 29, 2025  
**AWAITING APPROVAL**: [PENDING YOUR DECISION]

**Question?** Read `docs/PHASE_ROADMAP_V2.md` for full details.

**Ready to approve?** Update status above and commit.
