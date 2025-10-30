# GeneralsX Hybrid Architecture: DXVK + Custom Porting Layer

## Executive Summary

GeneralsX transitions from a custom DirectX8→Metal/OpenGL translation layer to a **hybrid architecture** combining:

- **DXVK for Graphics**: Industry-standard DirectX→Vulkan translation (proven, 15+ years optimized)
- **Custom Layer for Porting**: Win32→POSIX APIs, INI parsing, asset loading (already working well)

**Result**: Single backend (Vulkan) runs on Windows, Linux, and macOS (via MoltenVK)

---

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Generals Game Code                       │
│  (DirectX 8 API calls - unchanged from original Windows)    │
└────────────────────┬────────────────────────────────────────┘
                     │
        ┌────────────┴────────────┐
        │                         │
        ▼ (via #ifdef)            ▼
   
   ┌─────────────────┐      ┌──────────────────────┐
   │  LEGACY PATH    │      │   HYBRID PATH        │
   │ (artesanal)     │      │   (DXVK-based)      │
   └────────┬────────┘      └──────┬───────────────┘
            │                      │
            ▼                      ▼
   ┌─────────────────┐      ┌──────────────────────┐
   │ DX8Wrapper      │      │   DXVK               │
   │ (our mocks)     │      │   (DirectX→Vulkan)   │
   └────────┬────────┘      └──────┬───────────────┘
            │                      │
   ┌────────┴──────────┐          ▼
   │                   │      ┌──────────────┐
   ▼                   ▼      │   Vulkan     │
┌─────────┐        ┌─────────┐└──────┬───────┘
│ OpenGL  │        │ Metal   │       │
│ Backend │        │ Wrapper │   ┌───┴─────────┐
└─────────┘        └─────────┘   │             │
   (Khronos)         (Apple)      ▼             ▼
                              ┌────────┐   ┌─────────┐
                              │ Metal  │   │OpenGL   │
                              │(macOS) │   │(Linux)  │
                              └────────┘   └─────────┘
                                   │            │
        ┌──────────────────────────┼────────────┘
        │                          │
        └──────────────────────────┘
              Driver GPU
```

### Component Breakdown

#### Unchanged (Custom Layer - Reusable)
```cpp
✅ Core/Libraries/Source/WWVegas/WWBase/win32_compat.h
   - Win32 type definitions (HWND, HRESULT, etc)
   - POSIX API translations (file I/O, threading, memory)
   - Registry→Config translations

✅ Core/Libraries/Source/WWVegas/WW3D2/texture_cache.cpp
   - Asset loading from .big files
   - Texture format conversions
   - Memory management

✅ Core/GameEngine/Source/Common/GameMemory.cpp
   - Memory protection/validation
   - Safe pointer checks

✅ GeneralsMD/Code/Common/*.cpp
   - INI parser hardening
   - Configuration management
   - Audio system (audio engine logic)
```

#### Removed (Graphics-specific, replaced by DXVK)
```cpp
❌ Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp
   - DirectX 8 mock implementation
   - Render state translation
   - Texture binding emulation

❌ Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/cpp
   - Metal texture creation
   - Metal render pass setup
   - Metal shader management

❌ Core/Libraries/Source/WWVegas/WW3D2/texture.cpp (MODIFIED)
   - Removed Metal texture handle code
   - Removed graphics backend coupling
   - Keep only asset/memory logic
```

#### New (Graphics Abstraction Layer)
```cpp
🆕 Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h
   - IGraphicsBackend interface
   - Render state enums
   - Device initialization protocol

🆕 Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_legacy.cpp
   - Implementation of IGraphicsBackend
   - Wraps old DX8Wrapper + metalwrapper
   - For fallback/comparison testing

🆕 hybrid_port/graphics_backend_dxvk.cpp
   - DXVK-based implementation
   - Vulkan device management
   - Command buffer recording
```

---

## Development Workflow

### Build Configuration

```bash
# Legacy artesanal path (for testing/comparison)
cmake -DUSE_DXVK=OFF -DCMAKE_BUILD_TYPE=Release

# New DXVK hybrid path (production)
cmake -DUSE_DXVK=ON -DCMAKE_BUILD_TYPE=Release
```

### CMakeLists.txt Structure

```cmake
# In Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt

option(USE_DXVK "Use DXVK for graphics backend" ON)

if(USE_DXVK)
  # DXVK path
  find_package(DXVK REQUIRED)
  target_link_libraries(ww3d2 PUBLIC DXVK::DXVK)
  target_compile_definitions(ww3d2 PUBLIC USE_DXVK=1)
  target_sources(ww3d2 PRIVATE
    graphics_backend.h
    graphics_backend_dxvk.cpp
  )
else()
  # Legacy artesanal path
  target_sources(ww3d2 PRIVATE
    dx8wrapper.cpp
    metalwrapper.cpp
    graphics_backend_legacy.cpp
  )
  target_compile_definitions(ww3d2 PUBLIC USE_DXVK=0)
endif()
```

---

## Game Logic Interface

Game code uses abstract interface - doesn't know which backend:

```cpp
// In texture.cpp, render loops, etc
#include "graphics_backend.h"

class TextureClass {
  void Apply(unsigned stage) {
    if (WW3D::Is_Texturing_Enabled()) {
      // Backend-agnostic call
      DX8Wrapper::Set_DX8_Texture(stage, Peek_D3D_Base_Texture());
    }
  }
};

// DX8Wrapper becomes thin wrapper around backend
namespace DX8Wrapper {
  void Set_DX8_Texture(unsigned stage, IDirect3DBaseTexture8* tex) {
    #ifdef USE_DXVK
      // DXVK implementation
      gGraphicsBackend->SetTexture(stage, tex);
    #else
      // Legacy implementation
      glActiveTexture(GL_TEXTURE0 + stage);
      glBindTexture(GL_TEXTURE_2D, /* extract GL ID */);
    #endif
  }
}
```

---

## Single Backend Advantage: Windows, Linux, macOS

### Without DXVK (Current Approach)
```
Windows:  DirectX8 calls → D3D9 (native) → GPU Driver
Linux:    DirectX8 calls → Our OpenGL mock → GPU Driver  ⚠️ Different code path
macOS:    DirectX8 calls → Our Metal mock → GPU Driver   ⚠️ Different code path
```

**Problem**: Three different rendering paths → three sets of bugs

### With DXVK (Proposed)
```
Windows:  DirectX8 calls → DXVK → Vulkan → VK Driver
Linux:    DirectX8 calls → DXVK → Vulkan → VK Driver
macOS:    DirectX8 calls → DXVK → Vulkan → MoltenVK → Metal Driver
```

**Advantage**: Single Vulkan rendering path on all platforms

---

## Phase-by-Phase Implementation

### Phase 0: Abstraction & Setup (Week 1)
- Create graphics_backend.h interface
- Move existing code to graphics_backend_legacy.cpp
- Verify legacy path still works
- Create rollback strategy document

### Phase 1: DXVK Integration (Week 2-3)
- Setup DXVK build environment
- Implement graphics_backend_dxvk.cpp
- Basic D3D8→Vulkan translation
- Test on Linux first (simpler than macOS MoltenVK)

### Phase 2: MoltenVK Integration (Week 3-4)
- Setup MoltenVK for macOS
- Vulkan→Metal translation
- Metal validation layer debugging
- Performance profiling

### Phase 3: Asset & Game Logic (Week 4-6)
- Win32 file I/O → POSIX (already have)
- Registry → Config files (already have)
- Audio system porting (already analyzed in Phase 32)
- Game logic adjustments

---

## Rollback Strategy

If DXVK integration fails:

1. **Keep USE_DXVK=OFF** - revert to legacy graphics path
2. **No code removal** - legacy code remains in tree
3. **Hot-swap possible** - can switch at compile time
4. **Bisect capability** - can test individual game systems

**Example**:
```bash
# Something breaks in Phase 2
git bisect start
cmake -DUSE_DXVK=OFF  # Revert to working artesanal path
# Investigate issue in isolation
```

---

## Performance Considerations

### Expected Profile

| Operation | Artesanal | DXVK |
|-----------|-----------|------|
| Render state change | Direct Metal API | Vulkan intermediate + cache |
| Texture upload | Direct Metal | Vulkan staging buffer |
| Pipeline creation | Immediate | Compiled/cached by DXVK |
| Driver interaction | 1 layer | 2 layers (DXVK + Vulkan driver) |

**Net result**: DXVK overhead likely negligible on modern GPUs (2024+)

### Optimization Opportunities

1. **DXVK Shader Cache** - first frame slow, subsequent fast
2. **Memory pooling** - DXVK reuses Vulkan buffers
3. **Batch rendering** - Vulkan command buffer batching
4. **MoltenVK on macOS** - Apple Metal optimization built-in

---

## Testing & Validation

### Unit Tests
```bash
# Backend abstraction tests
tests/graphics_backend_test.cpp

# Asset loading tests (unchanged)
tests/texture_cache_test.cpp

# Platform-specific tests
tests/dxvk_vulkan_test.cpp (Linux)
tests/molten_vk_test.cpp (macOS)
```

### Integration Tests
```bash
# Load game, render one frame
tests/integration/single_frame_render.cpp

# Load game, run 30 seconds
tests/integration/thirty_second_stability.cpp

# Load game, verify texture sampling
tests/integration/texture_sampling_verification.cpp
```

### Regression Tests
```bash
# Comparison: legacy vs DXVK rendering output
tests/regression/render_output_comparison.cpp

# Performance regression detection
tests/regression/performance_baseline.cpp
```

---

## Repository Structure

```
GeneralsGameCode/
├── Core/
│   └── Libraries/Source/WWVegas/WW3D2/
│       ├── graphics_backend.h              (🆕 interface)
│       ├── graphics_backend_legacy.cpp     (🆕 legacy impl)
│       ├── dx8wrapper.cpp                  (unchanged/legacy)
│       ├── metalwrapper.h/cpp              (unchanged/legacy)
│       ├── texture.cpp                     (✏️ modified - decoupled)
│       └── win32_compat.h                  (✅ reused)
├── hybrid_port/                            (🆕 NEW DIRECTORY)
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── graphics_backend_dxvk.cpp
│   ├── dxvk/                               (submodule)
│   ├── molten-vk/                          (submodule)
│   └── shaders/                            (GLSL → SPIR-V)
├── docs/
│   ├── DXVK_HYBRID_ARCHITECTURE.md         (📄 this file)
│   ├── ROLLBACK_STRATEGY.md                (📄 strategy)
│   ├── PHASE_ROADMAP_V2.md                 (📄 updated phases)
│   ├── metal_port/                         (📁 historical)
│   │   ├── PHASE27/ ... PHASE37/
│   │   └── README.md                       (📄 historical overview)
│   └── PHASE_XX/                           (🆕 NEW PHASES)
│       ├── PHASE38_GRAPHICS_ABSTRACTION/
│       ├── PHASE39_DXVK_SETUP/
│       ├── PHASE40_MOLTEN_VK/
│       ├── PHASE41_ASSET_PORTING/
│       └── PHASE42_GAME_LOGIC/
└── tests/
    ├── graphics_backend_test.cpp
    └── integration/
```

---

## Success Criteria

### Phase 0 Complete
- ✅ Graphics backend interface defined
- ✅ Legacy code compiles to graphics_backend_legacy.cpp
- ✅ USE_DXVK=OFF produces identical output to before
- ✅ Rollback strategy documented and tested

### Phase 1 Complete  
- ✅ DXVK builds on Linux
- ✅ Basic D3D8 calls translate to Vulkan
- ✅ Game renders (even if blue screen or glitchy)
- ✅ No crashes after 30 seconds

### Phase 2 Complete
- ✅ MoltenVK setup on macOS
- ✅ Vulkan→Metal translation working
- ✅ Game renders on macOS (same as Linux)
- ✅ Performance acceptable (>30 FPS target)

### Phase 3 Complete
- ✅ All platforms use single Vulkan backend
- ✅ Asset loading working
- ✅ Game logic functional
- ✅ No platform-specific hacks needed

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|-----------|
| DXVK build complexity | High | Use Docker for reproducible builds |
| MoltenVK macOS issues | Medium | Test extensively, fallback to Metal |
| Performance regression | Medium | Profile against baseline |
| Development time underestimated | High | Time-box each phase |

---

## Decision Point: Go/No-Go

**Proceed with hybrid DXVK approach IF**:
- ✅ Single Vulkan backend desired over three platform paths
- ✅ 2-4 week timeline acceptable
- ✅ DXVK/Vulkan knowledge investment worthwhile
- ✅ Rollback to artesanal possible if needed

**Stay with artesanal approach IF**:
- ✅ Maximum control over rendering preferred
- ✅ Learning experience prioritized
- ✅ Can commit 6+ months to stability
- ✅ OK with platform-specific bugs

---

**Document Status**: Phase Planning Draft (Awaiting Decision)  
**Last Updated**: October 29, 2025  
**Next Steps**: Decision on go/no-go for Phase 0
