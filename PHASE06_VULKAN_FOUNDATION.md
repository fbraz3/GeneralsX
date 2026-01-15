# Phase 06: Vulkan Graphics Foundation & Audio Abstraction

**Status**: Starting (Rollback Complete to Phase 05 ✅)
**Current Date**: January 15, 2026
**Baseline**: Commit `d76cc6fdb` - Phase 05 Complete (VC6 + SDL2 + CD Protection Removal)

## 📋 Strategy Overview

After evaluating two architectural approaches:

### ❌ Rejected: "SDL2 Everywhere First" (Phases 11+)
```
VC6 + SDL2 (fragile - VC6 lacks C99 headers, intrinsics)
  → Win32Device keyboard/mouse enum casting issues
  → Multiple target churn (32, x64, ARM64 simultaneously)
  → Miles Audio blocker for x64 (no 64-bit version)
  → Too much technical debt, too many moving parts
```

### ✅ Accepted: "Complete One Target Right" (Phase 06+)
```
VC6 + SDL2 (working ✅)
  ↓
VC6 + Vulkan (graphics abstraction layer)
  ↓
VC6 + OpenAL (audio abstraction, replace Miles)
  ↓
[WINE READY + SHIP] ✅
  ↓
MSVC x64 + SDL2 + Vulkan + OpenAL (optimization)
  ↓
MSVC ARM64 + SDL2 + Vulkan + OpenAL (future)
```

## 🎯 Phase 06 Objectives

### Primary Goal: Graphics Abstraction Layer
- [ ] Investigate DirectX8 → Vulkan translation approach (DXVK vs custom abstraction)
- [ ] Create clean `GraphicsDevice` abstraction layer
- [ ] Implement Vulkan backend for rendering pipeline
- [ ] Validate all game rendering works (menus, 3D, UI)

### Secondary Goal: Audio Abstraction
- [ ] Create `AudioDevice` abstraction layer
- [ ] Implement OpenAL backend (cross-platform, 64-bit compatible)
- [ ] Replace Miles Audio dependency
- [ ] Validate all audio works (music, SFX, speech)

## 📊 Why This Order?

1. **VC6 is stable and proven**
   - Previous phases fixed keyboard input (DirectInput works perfectly)
   - SDL2 window management works
   - Asset loading works
   - Just need to abstract graphics layer

2. **Vulkan is the right graphics target**
   - Cross-platform (Windows, macOS, Linux)
   - Better than DirectX8 abstraction mess
   - DXVK reference shows the path
   - Solves Wine compatibility automatically

3. **OpenAL solves the audio blocker**
   - Miles Audio has no 64-bit official version
   - OpenAL is standard, cross-platform
   - Removes dependency chain blocker early
   - Audio can be done in parallel with graphics

4. **Wine ready = shipping point**
   - Once Vulkan + OpenAL work on VC6, we have a complete game
   - Can deploy and test on Windows hardware + Wine
   - THEN optimize for x64/ARM64 with full validation

## 📚 Reference Sources

- **`references/fighter19-dxvk-port/`** - DXVK integration example
  - Examine how DirectX8 calls are mapped to Vulkan
  - Review graphics pipeline abstraction
  - Study rendering state management

- **`references/jmarshall-win64-modern/`** - Windows 64-bit patterns
  - Use for audio abstraction patterns
  - Reference memory management fixes

- **`docs/Vulkan/`** - Local Vulkan SDK docs
  - API reference for implementation
  - Platform-specific notes for Windows/macOS/Linux

## 🔧 Technical Decisions

### Graphics Backend: Vulkan (not OpenGL)
- **Pros**: Better Windows performance, macOS via MoltenVK, Linux native
- **Cons**: More complex than OpenGL
- **Rationale**: DXVK shows it works, future-proof

### Audio Backend: OpenAL (not Miles)
- **Pros**: Cross-platform, 64-bit native, standard
- **Cons**: Need wrapper layer
- **Rationale**: Solves 64-bit blocker immediately

### Rendering Abstraction: Custom wrapper (not DXVK directly)
- **Pros**: Full control, understand the codebase
- **Cons**: More work than using DXVK
- **Rationale**: Learning valuable for Wine/cross-platform work

## 📅 Estimated Timeline

- **Week 1**: Graphics abstraction layer design & Vulkan investigation
- **Week 2**: Implement Vulkan rendering backend
- **Week 3**: Test all game visuals, fix rendering bugs
- **Week 4**: Audio abstraction + OpenAL implementation
- **Week 5**: Test all game audio, integration testing
- **Week 6**: Wine testing, performance validation
- **Week 7**: Optimization & hardening for deployment

## ✅ Success Criteria

### Graphics (Vulkan)
- [ ] All game menus render correctly
- [ ] 3D battlefield rendering works
- [ ] UI overlays display properly
- [ ] No visual artifacts or glitches
- [ ] Acceptable performance (30+ FPS on target hardware)

### Audio (OpenAL)
- [ ] Background music plays correctly
- [ ] Sound effects trigger at right moments
- [ ] Voice lines play cleanly
- [ ] No audio dropouts or latency issues

### Integration
- [ ] Game playable start-to-finish (intro → mission → score screen)
- [ ] Wine compatibility verified
- [ ] Build process clean (no warnings)
- [ ] Documentation updated

## 🚀 Next Steps

1. Read through `fighter19-dxvk-port/` reference implementation
2. Document DirectX8 → Vulkan mapping
3. Design `GraphicsDevice` abstraction interface
4. Prototype first Vulkan rendering
5. Test on Windows hardware with 3D acceleration

## 📝 Notes

- This phase prioritizes **correctness over performance**
- Focus on **finishing VC6 target completely**
- Wine compatibility is the key milestone
- 64-bit optimization comes AFTER validation
- Document all decisions for future reference

---

**Rollback Command**: `git reset --hard d76cc6fdb`  
**Current Executable**: `build/vc6/GeneralsMD/GeneralsXZH.exe` (6.02 MB)  
**Architecture**: VC6 32-bit, SDL2 windowing, DirectX 8 rendering, Miles Audio
