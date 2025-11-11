# GeneralsX Phase Documentation Index

**Date**: November 11, 2025  
**Strategy**: Vulkan-Only Cross-Platform Port (all OSes)

This directory contains comprehensive phase documentation for the GeneralsX cross-platform port of Command & Conquer: Generals.

## Strategic Decision: Vulkan-Only Backend

We are focusing on **Vulkan exclusively** for all platforms:
- **macOS** (ARM64 + x86_64): Vulkan via MoltenVK
- **Linux** (x86_64): Native Vulkan
- **Windows** (x86_64): Native Vulkan

This eliminates the complexity of maintaining multiple graphics backends (Metal, OpenGL) and allows us to:
1. Focus engineering effort on one proven API
2. Achieve feature parity across all platforms
3. Reduce testing and maintenance burden
4. Leverage Vulkan's superior performance and modern design

---

## Overview

The project is organized into phases grouped into logical development areas:

### Phase 00: Project Foundation (COMPLETE ✅)
- [PHASE00: Project Planning & Architecture](./PHASE00/README.md) - Strategic decisions
- [PHASE00.5: Build System Infrastructure](./PHASE00_5/README.md) - CMake, ccache, cross-platform builds

### Phase 01-05: Core Graphics Foundation
Graphics pipeline fundamentals using Vulkan.

- PHASE01: Vulkan Instance & Device Setup
- PHASE02: Vulkan Command Buffers & Rendering
- PHASE03: Shader System (SPIR-V)
- PHASE04: Vertex & Index Buffers
- PHASE05: Texture Loading & Sampling

### Phase 06-10: Advanced Graphics
Complex rendering features optimized for Vulkan.

- PHASE06: Material & Pipeline State
- PHASE07: Lighting System
- PHASE08: Camera & View Matrices
- PHASE09: Frustum Culling
- PHASE10: Mesh Loading (.w3d format)

### Phase 11-15: Game Rendering
Rendering game objects and units.

- PHASE11: Game Object Rendering
- PHASE12: Particle Effects
- PHASE13: Terrain Rendering
- PHASE14: Water & Special Effects
- PHASE15: UI Overlay Rendering

### Phase 16-20: Menu System
Main menu and UI infrastructure.

- PHASE16: Menu Definitions (INI parsing)
- PHASE17: Menu State Machine
- PHASE18: Menu Rendering
- PHASE19: Button Interaction
- PHASE20: Menu Audio Integration

### Phase 21-25: Game Logic
Core gameplay systems.

- PHASE21: GameObject System (entities)
- PHASE22: World Management (scene graph)
- PHASE23: Game Loop (main tick)
- PHASE24: Input Handling
- PHASE25: Unit Selection & Highlighting

### Phase 26-30: Advanced Gameplay
Complex game systems.

- PHASE26: Command System (move/attack)
- PHASE27: Pathfinding Integration
- PHASE28: Physics Integration
- PHASE29: AI & Behavior Trees
- PHASE30: Game State Transitions

### Phase 31-35: Audio & Polish
Sound and quality improvements.

- PHASE31: OpenAL Audio System
- PHASE32: Sound Effects Loading
- PHASE33: Music System
- PHASE34: Performance Profiling
- PHASE35: Memory Optimization

### Phase 36-40: Testing & Release
Validation and release preparation.

- PHASE36: Automated Testing Suite
- PHASE37: Cross-Platform Testing
- PHASE38: Crash Reporting & Debugging
- PHASE39: Documentation & API Polish
- PHASE40: MVP Release (v0.1.0)

---

## Documentation Structure

Each PHASE directory will contain:

- `README.md` - Complete phase documentation including:
  - Phase title and number
  - Clear objective/goal
  - Dependencies (which phases must be completed first)
  - Key deliverables
  - Acceptance criteria (checklist format)
  - Technical details and Vulkan-specific implementation
  - Code location references
  - Example Vulkan code where relevant
  - Estimated scope (SMALL/MEDIUM/LARGE)
  - Current status
  - Testing strategy
  - Success criteria
  - Reference documentation links

---

## Phase Dependency Graph

```
PHASE00 (Planning)
    ↓
PHASE00.5 (Build System)
    ↓
PHASE01 (Vulkan Instance & Device)
    ├→ PHASE02 (Command Buffers)
    ├→ PHASE03 (Shaders)
    ├→ PHASE04 (Buffers)
    └→ PHASE05 (Textures)
       ↓
    PHASE06 (Materials & Pipeline)
       ↓
    PHASE07 (Lighting)
       ├→ PHASE08 (Camera)
       ├→ PHASE09 (Culling)
       ├→ PHASE10 (Mesh Loading)
       └→ PHASE11 (Game Rendering)
          ├→ PHASE12 (Particles)
          ├→ PHASE13 (Terrain)
          ├→ PHASE14 (Water/FX)
          └→ PHASE15 (UI)
             ├→ PHASE16 (Menu)
             │  ├→ PHASE17 (State Machine)
             │  ├→ PHASE18 (Rendering)
             │  ├→ PHASE19 (Interaction)
             │  └→ PHASE20 (Audio)
             │
             └→ PHASE21 (GameObjects)
                ├→ PHASE22 (World Mgmt)
                ├→ PHASE23 (Game Loop)
                ├→ PHASE24 (Input)
                ├→ PHASE25 (Selection)
                ├→ PHASE26 (Commands)
                ├→ PHASE27 (Pathfinding)
                ├→ PHASE28 (Physics)
                ├→ PHASE29 (AI)
                └→ PHASE30 (State Trans.)
                   ↓
                PHASE31-35 (Audio & Polish)
                   ↓
                PHASE36-40 (Testing & Release)
                   ↓
                🎉 MVP v0.1.0
```

---

## Key Vulkan Concepts Per Phase

### Early Phases (01-05): Foundation
- Instance creation & physical device selection
- Logical device & queues
- Swapchain & presentation
- Command pools & buffers
- Render passes & framebuffers
- Shader compilation (GLSL → SPIR-V)
- Basic vertex/index buffers
- Texture loading & image layouts

### Mid Phases (06-15): Advanced Graphics
- Pipeline state management
- Descriptor sets & layouts (uniforms, textures)
- Lighting calculations in shaders
- View/projection matrix setup
- Visibility culling
- Complex mesh formats
- Particle systems
- Special effects

### Late Phases (16-30): Integration
- High-level rendering coordination
- Game object rendering
- UI rendering (orthographic projection)
- Audio system integration
- Game logic loop

### Final Phases (31-40): Polish
- Performance profiling (Vulkan timestamps)
- Memory management optimization
- Crash handling
- Cross-platform validation
- Release preparation

---

## Estimated Timeline

| Phase Group | Phases | Estimated Time | Difficulty |
|-------------|--------|----------------|------------|
| Foundation | 00-00.5 | ✅ Complete | - |
| Graphics Foundation | 01-05 | 3-4 weeks | HIGH |
| Advanced Graphics | 06-10 | 4-5 weeks | HIGH |
| Game Rendering | 11-15 | 3-4 weeks | MEDIUM |
| Menu System | 16-20 | 2-3 weeks | MEDIUM |
| Game Logic | 21-30 | 4-6 weeks | MEDIUM |
| Audio & Polish | 31-35 | 2-3 weeks | MEDIUM |
| Testing & Release | 36-40 | 2-3 weeks | LOW |
| **TOTAL** | | **20-28 weeks** | - |

---

## Build Commands

### Configure (All Platforms - Vulkan Only)

```bash
# macOS ARM64
cmake --preset macos-arm64-vulkan

# macOS x86_64
cmake --preset macos-x64-vulkan

# Linux
cmake --preset linux-vulkan

# Windows
cmake --preset windows-vulkan
```

### Build

```bash
cmake --build build/<preset> --target GeneralsXZH -j 4
```

### Run

```bash
cd $HOME/GeneralsX/GeneralsMD
./GeneralsXZH
```

---

## Platform-Specific Notes

### macOS (ARM64 + x86_64)
- Use MoltenVK for Vulkan support
- Ensure Metal SDK is installed
- Native performance via Metal backend underneath

### Linux (x86_64)
- Native Vulkan drivers (NVIDIA, AMD, Intel)
- Ensure Vulkan SDK is installed

### Windows (x86_64)
- Native Vulkan drivers
- Use latest Vulkan SDK

---

## Status Tracking

Each phase tracks its implementation status:

- **not-started**: Phase planning complete, implementation not begun
- **in-progress**: Phase currently being developed
- **completed**: Phase fully implemented and tested

---

## Testing Strategy

### Per-Phase Testing
Each phase includes:
- Unit tests for isolated components
- Integration tests with dependent phases
- Rendering verification (visual validation)
- Performance benchmarks (where applicable)

### Cross-Platform Validation
All phases tested on:
- macOS ARM64 (primary)
- macOS x86_64 (secondary)
- Linux x86_64
- Windows x86_64

---

## Contributing

When implementing a phase:

1. Read the complete README for that phase
2. Review all dependencies
3. Implement acceptance criteria in order
4. Update the status to "completed" when done
5. Document any deviations from the plan
6. Update the development diary

---

## Quick Links

- [Project Architecture Overview](./PHASE00/README.md)
- [Build System Infrastructure](./PHASE00_5/README.md)
- [Build Instructions](../MACOS_BUILD_INSTRUCTIONS.md)
- [Development Diary](../MACOS_PORT_DIARY.md)
- [Critical Files Reference](./MISC/BIG_FILES_REFERENCE.md)
- [Reference Repositories](../references/)

---

## Vulkan Resources

### Official Documentation
- [Vulkan Specification](https://www.khronos.org/vulkan/)
- [Vulkan SDK](https://vulkan.lunarg.com/)
- [Vulkan Samples](https://github.com/khronos/Vulkan-Samples)

### Learning Resources
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [Learn OpenGL (Vulkan section)](https://learnopengl.com/)
- [Vulkan Best Practices](https://vulkan.org/resources/bestpractices)

### Platform-Specific
- **macOS**: [MoltenVK Documentation](https://github.com/KhronosGroup/MoltenVK)
- **Linux**: Driver-specific documentation (NVIDIA, AMD, Intel)
- **Windows**: Microsoft & Vulkan SDK documentation

---

## All Phases Summary

| # | Phase | Area | Scope | Status |
|---|-------|------|-------|--------|
| 00 | Project Planning | Foundation | - | ✅ COMPLETE |
| 00.5 | Build System | Foundation | SMALL | ✅ COMPLETE |
| 01 | Vulkan Instance & Device | Graphics | MEDIUM | not-started |
| 02 | Command Buffers | Graphics | MEDIUM | not-started |
| 03 | Shader System | Graphics | MEDIUM | not-started |
| 04 | Buffers (Vertex/Index) | Graphics | MEDIUM | not-started |
| 05 | Textures & Sampling | Graphics | MEDIUM | not-started |
| 06 | Materials & Pipeline | Graphics | LARGE | not-started |
| 07 | Lighting | Graphics | LARGE | not-started |
| 08 | Camera System | Graphics | MEDIUM | not-started |
| 09 | Frustum Culling | Graphics | MEDIUM | not-started |
| 10 | Mesh Loading | Graphics | MEDIUM | not-started |
| 11 | Game Object Rendering | Rendering | LARGE | not-started |
| 12 | Particle Effects | Rendering | MEDIUM | not-started |
| 13 | Terrain Rendering | Rendering | LARGE | not-started |
| 14 | Water & Effects | Rendering | MEDIUM | not-started |
| 15 | UI Overlay | Rendering | MEDIUM | not-started |
| 16 | Menu Definitions | Menu | SMALL | not-started |
| 17 | Menu State Machine | Menu | MEDIUM | not-started |
| 18 | Menu Rendering | Menu | MEDIUM | not-started |
| 19 | Button Interaction | Menu | MEDIUM | not-started |
| 20 | Menu Audio | Menu | SMALL | not-started |
| 21 | GameObject System | Logic | MEDIUM | not-started |
| 22 | World Management | Logic | LARGE | not-started |
| 23 | Game Loop | Logic | MEDIUM | not-started |
| 24 | Input Handling | Logic | MEDIUM | not-started |
| 25 | Selection System | Logic | MEDIUM | not-started |
| 26 | Command System | Logic | LARGE | not-started |
| 27 | Pathfinding | Logic | MEDIUM | not-started |
| 28 | Physics | Logic | LARGE | not-started |
| 29 | AI & Behavior | Logic | LARGE | not-started |
| 30 | State Transitions | Logic | MEDIUM | not-started |
| 31 | OpenAL Audio | Polish | LARGE | not-started |
| 32 | Sound Effects | Polish | MEDIUM | not-started |
| 33 | Music System | Polish | MEDIUM | not-started |
| 34 | Performance Profiling | Polish | MEDIUM | not-started |
| 35 | Memory Optimization | Polish | MEDIUM | not-started |
| 36 | Testing Suite | Polish | LARGE | not-started |
| 37 | Cross-Platform Tests | Polish | LARGE | not-started |
| 38 | Crash Handling | Polish | SMALL | not-started |
| 39 | Documentation | Polish | MEDIUM | not-started |
| 40 | MVP Release v0.1.0 | Release | MEDIUM | not-started |

---

## Language

All documentation is in English as per project requirements.

## Last Updated

November 11, 2025 - Vulkan-only strategy adopted, old multi-backend phases removed
