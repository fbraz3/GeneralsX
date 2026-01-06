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

### Phase 051: Project Foundation (COMPLETE ✅)
- [PHASE00: Project Planning & Architecture](../WORKDIR/phases/0/PHASE00/README.md) - Strategic decisions
- [PHASE00.5: Build System Infrastructure](../WORKDIR/phases/0/PHASE00_5/README.md) - CMake, ccache, cross-platform builds

### Phase 051-06: Cross-Platform Prerequisites
Core compatibility layers for cross-platform support.

- PHASE01: DirectX 8 Compatibility Layer
- PHASE02: SDL2 Window & Event Loop
- PHASE03: File I/O Wrapper Layer
- PHASE04: Memory & Threading
- PHASE05: Registry & Configuration
- PHASE06: Input System

### Phase 051-10: Advanced Graphics
Complex rendering features optimized for Vulkan.

- PHASE06: Material & Pipeline State
- PHASE07: Lighting System
- PHASE08: Camera & View Matrices
- PHASE09: Frustum Culling
- PHASE10: Mesh Loading (.w3d format)

### Phase 051-15: Game Rendering
Rendering game objects and units.

- PHASE11: Game Object Rendering
- PHASE12: Particle Effects
- PHASE13: Terrain Rendering
- PHASE14: Water & Special Effects
- PHASE15: UI Overlay Rendering

### Phase 051-20: Menu System
Main menu and UI infrastructure.

- PHASE16: Menu Definitions (INI parsing)
- PHASE17: Menu State Machine
- PHASE18: Menu Rendering
- PHASE19: Button Interaction
- PHASE20: Menu Audio Integration

### Phase 051-25: Game Logic
Core gameplay systems.

- PHASE21: GameObject System (entities)
- PHASE22: World Management (scene graph)
- PHASE23: Game Loop (main tick)
- PHASE24: Input Handling
- PHASE25: Unit Selection & Highlighting

### Phase 051-30: Advanced Gameplay
Complex game systems.

- PHASE26: Command System (move/attack)
- PHASE27: Pathfinding Integration
- PHASE28: Physics Integration
- PHASE29: AI & Behavior Trees
- PHASE30: Game State Transitions

### Phase 051-35: Audio & Polish
Sound and quality improvements.

- PHASE31: OpenAL Audio System
- PHASE32: Sound Effects Loading
- PHASE33: Music System
- PHASE34: Performance Profiling
- PHASE35: Memory Optimization

### Phase 051-40: Testing & Release
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

```text
PHASE00 (Planning)
    ↓
PHASE00.5 (Build System)
    ↓
PHASE01 (DirectX Compatibility)
    ↓
PHASE02 (SDL2 Window & Events)
    ├→ PHASE03 (File I/O)
    ├→ PHASE04 (Memory & Threading)
    ├→ PHASE05 (Registry)
    └→ PHASE06 (Input System)
       ↓
    PHASE07 (Vulkan Instance & Device)
    ├→ PHASE08 (Swapchain)
    ├→ PHASE09 (Render Pass)
    ├→ PHASE10 (Command Buffers)
    └→ PHASE11 (Buffers)
       ↓
    PHASE12 (Textures)
       ├→ PHASE13 (Samplers)
       ├→ PHASE14 (Shaders)
       ├→ PHASE15 (Materials)
       └→ PHASE16 (Render Targets)
          ↓
       PHASE17 (Render Loop)
          ├→ PHASE18 (Culling)
          ├→ PHASE19 (Lighting)
          ├→ PHASE20 (Viewport)
          └→ PHASE21 (Draw Commands)
             ├→ PHASE22 (INI Parser)
             ├→ PHASE23 (Menu Rendering)
             ├→ PHASE24 (Menu Interaction)
             ├→ PHASE25 (Main Menu State)
             ├→ PHASE26 (Audio Integration)
             │
             └→ PHASE27 (GameObjects)
                ├→ PHASE28 (World Mgmt)
                ├→ PHASE29 (Game Loop)
                ├→ PHASE30 (Input)
                ├→ PHASE31 (Selection)
                ├→ PHASE32 (Commands)
                ├→ PHASE33 (Pathfinding)
                ├→ PHASE34 (Physics)
                ├→ PHASE35 (AI)
                └→ PHASE36 (State Trans.)
                   ↓
                PHASE37-41 (Audio & Polish & Testing)
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
cmake --preset macos

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

- [Project Architecture Overview](../WORKDIR/phases/0/PHASE00/README.md)
- [Build System Infrastructure](../WORKDIR/phases/0/PHASE00_5/README.md)
- [Build Instructions (macOS)](./MACOS_BUILD_INSTRUCTIONS.md)
- [Build Instructions (Linux)](./LINUX_BUILD_INSTRUCTIONS.md)
- [Development Diary](../DEV_BLOG/README.md)
- [Critical Files Reference](../WORKDIR/support/BIG_FILES_REFERENCE.md)
- [Reference Repositories](../../references/)

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

| # | Phase | Area | Scope | Status | Documentation |
|---|-------|------|-------|--------|---|
| 00 | Project Planning | Foundation | - | ✅ COMPLETE | - |
| 00.5 | Build System | Foundation | SMALL | ✅ COMPLETE | - |
| 01 | DirectX 8 Compatibility Layer | Prerequisites | LARGE | not-started | [docs/WORKDIR/phases/0/PHASE01/README.md](../WORKDIR/phases/0/PHASE01/README.md) |
| 02 | SDL2 Window & Event Loop | OS API | MEDIUM | not-started | [docs/WORKDIR/phases/0/PHASE02/README.md](../WORKDIR/phases/0/PHASE02/README.md) |
| 03 | File I/O Wrapper Layer | OS API | MEDIUM | not-started | [docs/WORKDIR/phases/0/PHASE03/README.md](../WORKDIR/phases/0/PHASE03/README.md) |
| 04 | Memory & Threading | OS API | MEDIUM | not-started | [docs/WORKDIR/phases/0/PHASE04/README.md](../WORKDIR/phases/0/PHASE04/README.md) |
| 05 | Registry & Configuration | OS API | SMALL | not-started | [docs/WORKDIR/phases/0/PHASE05/README.md](../WORKDIR/phases/0/PHASE05/README.md) |
| 06 | Input System | OS API | MEDIUM | not-started | [docs/WORKDIR/phases/0/PHASE06/README.md](../WORKDIR/phases/0/PHASE06/README.md) |
| 07 | Vulkan Instance & Device | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/0/PHASE07/README.md](../WORKDIR/phases/0/PHASE07/README.md) |
| 07 | Swapchain & Presentation | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/0/PHASE07/README.md](../WORKDIR/phases/0/PHASE07/README.md) |
| 08 | Render Pass & Pipeline | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/0/PHASE08/README.md](../WORKDIR/phases/0/PHASE08/README.md) |
| 09 | Command Buffers & Sync | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/0/PHASE09/README.md](../WORKDIR/phases/0/PHASE09/README.md) |
| 10 | Vertex & Index Buffers | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/1/PHASE10/README.md](../WORKDIR/phases/1/PHASE10/README.md) |
| 11 | Texture System | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE11/README.md](../WORKDIR/phases/1/PHASE11/README.md) |
| 12 | Sampler & Descriptor Sets | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE12/README.md](../WORKDIR/phases/1/PHASE12/README.md) |
| 13 | Shader System | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE13/README.md](../WORKDIR/phases/1/PHASE13/README.md) |
| 14 | Material System | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE14/README.md](../WORKDIR/phases/1/PHASE14/README.md) |
| 15 | Render Target Management | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE15/README.md](../WORKDIR/phases/1/PHASE15/README.md) |
| 16 | Render Loop Integration | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/1/PHASE16/README.md](../WORKDIR/phases/1/PHASE16/README.md) |
| 17 | Visibility & Culling | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE17/README.md](../WORKDIR/phases/1/PHASE17/README.md) |
| 18 | Lighting System | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE18/README.md](../WORKDIR/phases/1/PHASE18/README.md) |
| 19 | Viewport & Projection | Graphics | MEDIUM | not-started | [docs/WORKDIR/phases/1/PHASE19/README.md](../WORKDIR/phases/1/PHASE19/README.md) |
| 20 | Draw Command System | Graphics | MEDIUM | COMPLETE | [docs/WORKDIR/phases/2/PHASE20/README.md](../WORKDIR/phases/2/PHASE20/README.md) |
| 21 | INI Parser Hardening | Menu | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE21/README.md](../WORKDIR/phases/2/PHASE21/README.md) |
| 22 | Menu Rendering | Menu | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE22/README.md](../WORKDIR/phases/2/PHASE22/README.md) |
| 23 | Menu Interaction | Menu | SMALL | not-started | [docs/WORKDIR/phases/2/PHASE23/README.md](../WORKDIR/phases/2/PHASE23/README.md) |
| 24 | Main Menu State Machine | Menu | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE24/README.md](../WORKDIR/phases/2/PHASE24/README.md) |
| 25 | Audio Integration (Phase 02) | Menu | SMALL | not-started | [docs/WORKDIR/phases/2/PHASE25/README.md](../WORKDIR/phases/2/PHASE25/README.md) |
| 26 | Game Object System | Logic | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE26/README.md](../WORKDIR/phases/2/PHASE26/README.md) |
| 27 | World Management | Logic | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE27/README.md](../WORKDIR/phases/2/PHASE27/README.md) |
| 28 | Game Loop Integration | Logic | SMALL | not-started | [docs/WORKDIR/phases/2/PHASE28/README.md](../WORKDIR/phases/2/PHASE28/README.md) |
| 29 | Rendering Integration | Logic | MEDIUM | not-started | [docs/WORKDIR/phases/2/PHASE29/README.md](../WORKDIR/phases/2/PHASE29/README.md) |
| 30 | Camera System | Logic | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE30/README.md](../WORKDIR/phases/3/PHASE30/README.md) |
| 31 | Audio System (Phase 03) | Gameplay | LARGE | not-started | [docs/WORKDIR/phases/3/PHASE31/README.md](../WORKDIR/phases/3/PHASE31/README.md) |
| 32 | Input Handling | Gameplay | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE32/README.md](../WORKDIR/phases/3/PHASE32/README.md) |
| 33 | Pathfinding & Movement | Gameplay | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE33/README.md](../WORKDIR/phases/3/PHASE33/README.md) |
| 34 | Combat System | Gameplay | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE34/README.md](../WORKDIR/phases/3/PHASE34/README.md) |
| 35 | Game State Management | Gameplay | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE35/README.md](../WORKDIR/phases/3/PHASE35/README.md) |
| 36 | Test Infrastructure | Testing | LARGE | not-started | [docs/WORKDIR/phases/3/PHASE36/README.md](../WORKDIR/phases/3/PHASE36/README.md) |
| 37 | Cross-Platform Validation | Testing | LARGE | not-started | [docs/WORKDIR/phases/3/PHASE37/README.md](../WORKDIR/phases/3/PHASE37/README.md) |
| 38 | Optimization & Profiling | Testing | MEDIUM | not-started | [docs/WORKDIR/phases/3/PHASE38/README.md](../WORKDIR/phases/3/PHASE38/README.md) |
| 39 | Performance Profiling System | Profiling | LARGE | ✅ COMPLETE | [docs/WORKDIR/phases/3/PHASE39/README.md](../WORKDIR/phases/3/PHASE39/README.md) |
| 40 | Data-Driven Optimization | Optimization | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE40/README.md](../WORKDIR/phases/4/PHASE40/README.md) |
| 41 | Automated Bottleneck Fixing | Optimization | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE41/README.md](../WORKDIR/phases/4/PHASE41/README.md) |
| 42 | Performance Dashboard | Visualization | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE42/README.md](../WORKDIR/phases/4/PHASE42/README.md) |
| 43 | Cross-Build Comparison | Validation | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE43/README.md](../WORKDIR/phases/4/PHASE43/README.md) |
| 44 | ML-Based Regression Detection | Intelligence | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE44/README.md](../WORKDIR/phases/4/PHASE44/README.md) |
| 45 | Advanced Memory Management | Optimization | LARGE | not-started | [docs/WORKDIR/phases/planned/PHASE45/README.md](../WORKDIR/phases/planned/PHASE45/README.md) |
| 46 | Extended Testing Suite | QA | LARGE | not-started | [docs/WORKDIR/phases/planned/PHASE46/README.md](../WORKDIR/phases/planned/PHASE46/README.md) |
| 47 | Release & Distribution | DevOps | LARGE | not-started | [docs/WORKDIR/phases/4/PHASE47/README.md](../WORKDIR/phases/4/PHASE47/README.md) |

---

## Language

All documentation is in English as per project requirements.

## Last Updated

November 11, 2025 - Vulkan-only strategy adopted, old multi-backend phases removed
