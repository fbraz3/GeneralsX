# GeneralsX Phase Roadmap V2: Hybrid DXVK Architecture

**Revision Date**: October 29, 2025  
**Status**: Planning Phase (Pre-Phase 0)  
**Strategy**: DXVK backend + custom porting layer

---

## Executive Summary

GeneralsX transitions from **custom DirectX8→Metal/OpenGL** (Phases 27-37) to **DXVK-based Vulkan backend** with custom Win32/asset porting layer.

**Timeline**: 4-6 weeks to gameplay  
**Platforms**: Windows, Linux, macOS (via MoltenVK)  
**Shared Backend**: Single Vulkan path across all platforms

---

## Historical Phases (Archived)

These phases represent the artesanal DirectX→Graphics port. Documentation preserved in `docs/metal_port/`:

- **Phase 27**: W3D Graphics Engine Architecture (archived)
- **Phase 28**: Texture Loading - Post-DirectX Interception (archived)
- **Phase 29**: Metal Render States - Lighting, Fog, Stencil (archived)
- **Phase 30**: Metal Backend Success (archived)
- **Phase 31**: Texture System Integration (archived)
- **Phase 32**: Audio Pipeline Investigation (archived)
- **Phase 33**: OpenAL Audio Backend Stubs (archived)
- **Phase 34**: Game Logic & Gameplay Systems (archived)
- **Phase 35**: Multiplayer & Networking (archived)
- **Phase 36**: Polish & Optimization (archived)
- **Phase 37**: Metal Texture System - Discovered hangs (archived)

**Reason for Archive**: Replaced with DXVK approach. Preserved for:
- Historical reference
- Rollback if DXVK fails
- Learning material (what NOT to do)

---

## New Phase Structure: DXVK Hybrid (Phases 38-50)

### Phase 38: Graphics Backend Abstraction (Week 1)
**Goal**: Create abstraction layer enabling backend switching  
**Duration**: 3-5 days  
**Deliverables**:
- `graphics_backend.h` interface
- `graphics_backend_legacy.cpp` wrapping DX8Wrapper/metalwrapper
- `CMakeLists.txt` with `USE_DXVK` flag
- Test: `USE_DXVK=OFF` produces identical build
- Documentation: Architecture overview

**Key Tasks**:
```cpp
// Define interface
class IGraphicsBackend {
  virtual HRESULT SetTexture(unsigned stage, void* texture) = 0;
  virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) = 0;
  // ... ~30 virtual methods
};

// Wrap legacy
class LegacyGraphicsBackend : public IGraphicsBackend {
  // Delegates to DX8Wrapper:: and metalwrapper::
};

// Create global instance selector
#ifdef USE_DXVK
  IGraphicsBackend* g_backend = new DVKGraphicsBackend();
#else
  IGraphicsBackend* g_backend = new LegacyGraphicsBackend();
#endif
```

**Success Criteria**:
- ✅ `cmake -DUSE_DXVK=OFF` builds successfully
- ✅ Game runs identically to before abstraction
- ✅ No game code contains `#ifdef USE_DXVK`
- ✅ All graphics calls go through IGraphicsBackend

---

### Phase 39: DXVK Setup & Environment (Week 1-2)
**Goal**: Get DXVK building on development machines  
**Duration**: 3-5 days  
**Deliverables**:
- DXVK repository as Git submodule
- CMake DXVK detection & linking
- Build scripts for Windows/Linux/macOS
- Documentation: DXVK build environment
- CI/CD pipeline for DXVK builds

**Platform-Specific**:

**Linux**:
```bash
# DXVK setup simple on Linux
git submodule add https://github.com/doitsujin/dxvk.git hybrid_port/dxvk
cd hybrid_port/dxvk
./setup-meson.sh /path/to/install
meson compile -C build
meson install -C build
```

**macOS**:
```bash
# MoltenVK for macOS (replaces Metal wrapper)
git submodule add https://github.com/KhronosGroup/MoltenVK.git hybrid_port/molten-vk
cd hybrid_port/molten-vk
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j 4
```

**Windows**:
```bash
# DXVK on Windows via wine/proton
# Or use native Vulkan SDK
vcpkg install vulkan
```

**Success Criteria**:
- ✅ DXVK builds on at least Linux
- ✅ Vulkan loader accessible
- ✅ CMake finds DXVK/Vulkan automatically
- ✅ CI/CD pipeline green for build

---

### Phase 40: Basic DXVK Graphics Backend (Week 2)
**Goal**: Minimal D3D8→Vulkan translation working  
**Duration**: 5-7 days  
**Deliverables**:
- `graphics_backend_dxvk.cpp` skeleton
- Vulkan device/instance creation
- Basic command buffer recording
- Simple vertex/index buffer handling
- Game renders (even if blue screen)
- No crashes after 10 seconds

**Implementation Pattern**:
```cpp
class DVKGraphicsBackend : public IGraphicsBackend {
  VkInstance vkInstance;
  VkDevice vkDevice;
  VkQueue vkQueue;
  VkCommandPool cmdPool;
  
  virtual HRESULT SetTexture(unsigned stage, void* texture) override {
    // Translate D3D texture handle to Vulkan
    // VkDescriptorSet binding
  }
  
  virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override {
    // Vulkan pipeline state modification
  }
};
```

**Linux-First Approach**:
- Test DXVK on Linux first (simpler than macOS MoltenVK)
- Vulkan development tools available
- If works on Linux, porting to macOS easier

**Success Criteria**:
- ✅ Game window opens
- ✅ No crashes after 10 seconds
- ✅ Render output (any color, not pure black)
- ✅ Framerate > 1 FPS (not frozen)

---

### Phase 41: Texture System in Vulkan (Week 3)
**Goal**: Textured geometry rendering (not just blue screen)  
**Duration**: 5-7 days  
**Deliverables**:
- Vulkan image/sampler creation
- Texture descriptor set binding
- Fragment shader texture sampling
- Textured geometry visible
- No crashes during texture-heavy scenes

**Key Challenges**:
```cpp
// Vulkan texture lifecycle more complex than DirectX
VkImage texImage = CreateTextureImage(pixelData, width, height, format);
VkImageView texImageView = CreateImageView(texImage, VK_IMAGE_VIEW_TYPE_2D);
VkSampler texSampler = CreateSampler(VK_FILTER_LINEAR);
VkDescriptorSet descSet = CreateDescriptorSet();
UpdateDescriptorSet(descSet, texImageView, texSampler);
// + layout transitions between copy/read/render
```

**Asset Reuse**:
- Texture pixel data: From existing TextureCache (unchanged)
- Format conversions: Already working
- Memory management: Reuse existing allocation patterns

**Success Criteria**:
- ✅ Textured geometry visible (not blue screen)
- ✅ Multiple texture stages work (at least 4)
- ✅ Texture filtering operational (linear/nearest)
- ✅ No graphics glitches in texture sampling

---

### Phase 42: Render States & Lighting (Week 3-4)
**Goal**: Game logic graphics state properly mapped  
**Duration**: 5-7 days  
**Deliverables**:
- 8 texture stages binding
- Lighting/fog/ambient colors
- Stencil/blend operations
- Material properties
- Render target management

**Complex Mappings**:
```cpp
// D3DRS_LIGHTING (boolean)
switch (value) {
  case TRUE:
    // Enable per-vertex lighting in shader
    break;
  case FALSE:
    // Use only ambient/emissive
    break;
}

// D3DRS_AMBIENTMATERIALSOURCE
// Maps to which material component used

// D3DRS_ZENABLE + D3DRS_ZFUNC
// Maps to VkCompareOp + depth test enable
```

**Reference Material**: Phases 29.1-29.4 (archived) show D3D render state requirements

**Success Criteria**:
- ✅ Lighting in game scenes correct
- ✅ Fog/transparency working
- ✅ Shadow rendering operational
- ✅ Special effects visible (holograms, heat distortion)

---

### Phase 43: MoltenVK Integration for macOS (Week 4)
**Goal**: DXVK works on macOS via MoltenVK  
**Duration**: 3-5 days  
**Deliverables**:
- MoltenVK linked to build
- Vulkan→Metal translation layer loaded
- Game renders identically on macOS
- Metal validation warnings debugged
- macOS-specific quirks documented

**macOS Specific Challenges**:
```cpp
// MoltenVK compatibility notes
// - Requires 10.13+ macOS SDK
// - Metal driver slightly different behavior
// - Performance may differ from native Metal
// - Validation layer available but verbose

// Common issues:
// VK_FORMAT_B8G8R8A8_SRGB may not be supported → use RGBA8
// Presentation (swapchain) behaves differently
// Metal synchronization different from Vulkan semantics
```

**Success Criteria**:
- ✅ Game renders on macOS
- ✅ Output matches Linux DXVK version
- ✅ Performance acceptable (>30 FPS)
- ✅ No MoltenVK-specific crashes

---

### Phase 44: Windows Native Vulkan (Week 4-5)
**Goal**: Verify cross-platform Vulkan works on Windows too  
**Duration**: 2-3 days  
**Deliverables**:
- DXVK build on Windows
- Native Vulkan backend (not fallback to D3D)
- Performance comparison vs D3D native
- Documentation: Windows build process

**Windows Strategy**:
```cpp
// Option A: DXVK on Windows (like Proton)
// D3D8 → DXVK → Vulkan → Windows Vulkan Driver

// Option B: Native D3D on Windows (fallback)
// D3D8 → DirectX 8 Device → GPU

// We go with Option A for unified backend
```

**Success Criteria**:
- ✅ DXVK compiles on Windows
- ✅ Game plays on Windows with DXVK
- ✅ Graphics equivalent to Linux/macOS
- ✅ Performance reasonable

---

### Phase 45: Asset Porting - File I/O (Week 5)
**Goal**: Cross-platform asset loading without Windows registry  
**Duration**: 3-5 days  
**Deliverables**:
- Platform-specific file path resolution
- .big file loading on Linux/macOS
- Config file loading (instead of registry)
- Asset search path algorithm
- Documentation: Asset porting completed

**Reuse Existing Code**:
- `win32_compat.h` - file I/O wrappers already present
- `TheFileSystem` - cross-platform file access exists
- `texture_cache.cpp` - asset loading logic unchanged

**New Work**: Config file handling
```cpp
// Instead of Windows registry
// ~/.config/generals.ini (Linux)
// ~/Library/Preferences/Generals.plist (macOS)
// APPDATA\Generals\config.ini (Windows)

INIClass* config = new INIClass();
config->Load(GetConfigPath() + "generals.ini");
```

**Success Criteria**:
- ✅ .big files load on all platforms
- ✅ INI configuration works cross-platform
- ✅ No registry access attempts
- ✅ Asset paths resolve correctly

---

### Phase 46: Audio System Porting (Week 5-6)
**Goal**: Cross-platform audio using OpenAL  
**Duration**: 3-5 days  
**Deliverables**:
- OpenAL device initialization
- Audio event system mapping
- Sound file loading (legacy .aud files)
- 3D audio positioning
- Music system
- No audio driver crashes

**Reference**: Phase 32 analysis already complete - use those findings

**Simple Implementation**:
```cpp
// Audio layer simple once graphics backend done
ALCdevice* device = alcOpenDevice(NULL);
ALCcontext* context = alcCreateContext(device, NULL);
alcMakeContextCurrent(context);

// Load game sounds via existing infrastructure
// Minimal changes needed
```

**Success Criteria**:
- ✅ Audio plays without crashes
- ✅ All sound effects functional
- ✅ Music system works
- ✅ 3D audio positioning correct

---

### Phase 47: Input & Game Logic (Week 6)
**Goal**: Game playable, all input working  
**Duration**: 3-5 days  
**Deliverables**:
- SDL input mapping (keyboard/mouse/gamepad)
- UI rendering cross-platform
- Game logic execution verified
- 30+ minute gameplay without crashes
- All major features functional

**Remaining Platform-Specific Fixes**:
```cpp
// Keyboard layout handling (Linux/macOS different)
// Gamepad device detection
// Mouse capture vs relative mode
// Window resizing behavior
// Alt-tab / window focus handling
```

**Success Criteria**:
- ✅ Game fully playable
- ✅ All input methods work
- ✅ UI renders correctly
- ✅ 30+ minute gameplay stable

---

### Phase 48: Optimization & Profiling (Week 6-7)
**Goal**: Performance meets playability standards  
**Duration**: 5-7 days  
**Deliverables**:
- Frame time profiling per platform
- Shader compilation optimization
- Memory usage reduction
- Driver optimization flags
- Performance report

**Measurements**:
```
Target FPS: 30+ on target hardware
Max Frame Time: 33ms
Memory Usage: <500MB

Current (estimated after Phase 47):
- Windows: 60 FPS (Vulkan better than D3D)
- Linux: 45 FPS (Vulkan native)
- macOS: 30-40 FPS (MoltenVK overhead)
```

**Optimization Levers**:
- DXVK shader compilation caching
- Vulkan pipeline caching
- Reduced descriptor set updates
- Memory pool optimization

**Success Criteria**:
- ✅ >30 FPS on all platforms
- ✅ No frame rate drops below 25 FPS
- ✅ Memory usage stable
- ✅ Load times acceptable

---

### Phase 49: Testing & Stability (Week 7-8)
**Goal**: Game production-ready  
**Duration**: 5-7 days  
**Deliverables**:
- Stress testing (10+ hour sessions)
- Regression testing (replay old save games)
- Cross-platform compatibility testing
- Known issues documented
- Release notes prepared

**Test Coverage**:
- Single player campaigns
- Skirmish mode
- Map editor
- Replay system
- Multiplayer connections (if implemented)

**Success Criteria**:
- ✅ 10+ hours stable (no crashes)
- ✅ All game modes functional
- ✅ Consistent experience on all platforms
- ✅ Known issues << 10

---

### Phase 50: Release & Documentation (Week 8)
**Goal**: GeneralsX launched and documented  
**Duration**: 3-5 days  
**Deliverables**:
- Official release build
- User documentation (README, system requirements)
- Developer documentation (architecture, contribution guide)
- Build instructions for each platform
- Source code repository cleaned up
- Release notes & changelog

**Release Artifacts**:
```
GeneralsX-1.0-Windows.exe (Vulkan backend)
GeneralsX-1.0-Linux.tar.gz (Vulkan backend)
GeneralsX-1.0-macOS.dmg (MoltenVK backend)

+ Full source on GitHub
+ Build instructions for each platform
+ Known issues list
+ Troubleshooting guide
```

**Success Criteria**:
- ✅ Release builds for all platforms
- ✅ Documentation complete
- ✅ Community can build from source
- ✅ Launch ready

---

## Comparison: Old vs New Roadmap

| Aspect | Phases 27-37 (Archived) | Phases 38-50 (New) |
|--------|----------------------|-------------------|
| **Graphics Backend** | Custom Metal/OpenGL | DXVK/Vulkan |
| **Timeline** | 6+ months (incomplete) | 4-6 weeks |
| **Platforms** | Metal (macOS), OpenGL (Linux), D3D (Windows) | Vulkan (unified) |
| **Stability** | Hangs/crashes during Phase 37 | Expected stable in Phase 47 |
| **Maintainability** | Platform-specific code | Single Vulkan path |
| **Developer Skills** | Metal/OpenGL required | Vulkan/DXVK |
| **Industry Standard** | Custom layer | Proven technology (DXVK) |

---

## Risk Mitigation

### Risk: DXVK Compilation Fails
**Mitigation**: Keep `USE_DXVK=OFF` path working. Always revertible.

### Risk: MoltenVK Performance Poor
**Mitigation**: If <20 FPS on macOS, fallback to native Metal wrapper (Phase 27-37 code)

### Risk: Game Logic Issues
**Mitigation**: Asset/Win32 porting independent of graphics backend

### Risk: Schedule Slip
**Mitigation**: Time-box each phase. Skip optimization (Phase 48) if needed.

---

## Decision Gate

**Before Phase 38**: Stakeholder approval

- ✅ DXVK approach acceptable?
- ✅ 4-6 week timeline realistic?
- ✅ Single Vulkan backend desired?
- ✅ Okay to archive Phases 27-37?

**If NO**: Remain with Phase 27-37 (Phases 38+ not executed)  
**If YES**: Proceed with Phase 38

---

## Historical Archive

All previous phases preserved for reference:

```
docs/metal_port/PHASE27/  - W3D Graphics Engine
docs/metal_port/PHASE28/  - Texture Loading
docs/metal_port/PHASE29/  - Metal Render States
docs/metal_port/PHASE30/  - Metal Backend Success
docs/metal_port/PHASE31/  - Texture System Integration
docs/metal_port/PHASE32/  - Audio Pipeline
docs/metal_port/PHASE33/  - OpenAL Backend
docs/metal_port/PHASE34/  - Game Logic
docs/metal_port/PHASE35/  - Multiplayer
docs/metal_port/PHASE36/  - Polish
docs/metal_port/PHASE37/  - Texture Hang (Stopped Here)
```

**Access**: `git log --oneline docs/metal_port/`

---

**Document Status**: Phase Planning Complete  
**Approved By**: [TBD]  
**Next Step**: Phase 38 kickoff or decision gate

**Questions**?
1. Timeline feasible?
2. DXVK approach acceptable?
3. Rollback strategy sufficient?
4. Any platform-specific concerns?
