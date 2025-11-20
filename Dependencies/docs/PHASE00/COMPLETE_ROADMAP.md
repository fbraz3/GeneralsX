# GeneralsX - Complete Roadmap: PHASE 0 until Main Menu

**Date**: November 10, 2025  
**Objective**: Structure ALL necessary phases until initial screen working  
**Requirement**: No time estimates, only sequence and dependencies  

---

## Executive Roadmap

```
PHASE 0: Spike Planning ✅ (CURRENT)
  ↓
PHASE 1-10: Core Graphics Infrastructure (Foundation)
  ├─ 1: Geometry Rendering (colored triangles)
  ├─ 2: Texture System (load textures)
  ├─ 3: Material System (shaders, textures)
  ├─ 4: Lighting System (DirectX lights → Vulkan)
  ├─ 5: Render Loop (stable 60 FPS)
  ├─ 6: Mesh Loading (3D models)
  ├─ 7: Mesh Rendering (render models)
  ├─ 8: Camera System (viewpoint control)
  ├─ 9: Frustum Culling (optimization)
  └─ 10: Post-Processing (effects)
  ↓
PHASE 11-20: UI & Asset Loading
  ├─ 11: UI Mesh Format (parse UI models)
  ├─ 12: UI Texture Loading (load UI textures from .big)
  ├─ 13: UI Rendering (render UI with Vulkan)
  ├─ 14: UI Layout (positioning widgets)
  ├─ 15: Button System (clickable areas)
  ├─ 16: Input Routing (SDL2 → UI callbacks)
  ├─ 17: Menu Definitions (parse menu INI)
  ├─ 18: Menu State Machine (menu transitions)
  ├─ 19: Menu Rendering (render menu frame)
  └─ 20: Menu Interaction (clicks working)
  ↓
PHASE 21-30: Game Logic Integration
  ├─ 21: Game Object System (GameObject class)
  ├─ 22: World Management (GameWorld container)
  ├─ 23: Game Loop (main game tick)
  ├─ 24: Input Handling (gameplay input)
  ├─ 25: Selection System (unit selection)
  ├─ 26: Command System (move, attack, etc)
  ├─ 27: Pathfinding Stub (placeholder)
  ├─ 28: Physics Stub (placeholder)
  ├─ 29: Gameplay Rendering (render game objects)
  └─ 30: State Transitions (menu → game)
  ↓
PHASE 31-40: Polish & Stabilization
  ├─ 31: Audio System (OpenAL backend - Phase 33)
  ├─ 32: Performance Profiling
  ├─ 33: Memory Optimization
  ├─ 34: Crash Handling (better error messages)
  ├─ 35: Platform-Specific Fixes
  ├─ 36: Testing & Validation
  ├─ 37: Documentation
  ├─ 38: Build System Optimization
  ├─ 39: Cross-Platform Testing
  └─ 40: Minimal Playable Release
  ↓
INITIAL MENU WORKING ✅
```

---

## Detailed Phases: PHASE 0-10 (Foundation)

### PHASE 0: Spike Planning (CURRENT)

**Status**: In progress  
**Dependencies**: None  
**Blockers**: None  

**Deliverables**:
- Complete analysis of current state
- Mapping of lessons learned
- Documentation of compatibility layers
- Platform presets defined
- Complete roadmap of phases
- Executable naming confirmed

**Next**: PHASE 1

---

### PHASE 1: Geometry Rendering - Colored Triangles

**Dependencies**: Graphics pipeline (Phase 39-48 OK)  
**Objective**: Render 1+ colored triangle on screen  

**Tasks**:
1. Create simple vertex buffer with 3 vertices (PosColor)
2. Create index buffer with 3 indices
3. Implement vertex shader (PosColor → color output)
4. Implement fragment shader (passthrough color)
5. Create graphics pipeline with vertex/fragment shaders
6. Render colored triangle in each frame
7. Validate triangle in viewport

**Acceptance**:
- [x] Triangle rendered
- [x] Cores corretas (RGB)
- [x] No z-fighting artifacts
- [x] 60 FPS stable

**Next**: PHASE 2

---

### PHASE 2: Texture System - Load Textures

**Dependencies**: PHASE 1 (geometry OK)  
**Objective**: Load 1+ texture from .big, apply to triangle  

**Tasks**:
1. Fix texture loading pipeline (Phase 28.4 worked, validate)
2. Add texture coordinates (UV) to vertex buffer
3. Create texture sampler
4. Update fragment shader to use sampler
5. Load 1 texture from .big (INITIALIZE before render)
6. Bind texture in command buffer
7. Render textured triangle

**Acceptance**:
- [x] Textura aparecand in triangle
- [x] No artifacts (filtering correto)
- [x] Suportand DDS/TGA formats
- [x] UV mapping correto

**Next**: PHASE 3

---

### PHASE 3: Material System - Shaders & Uniforms

**Dependencies**: PHASE 2 (textures OK)  
**Objective**: Implement material properties (diffuse, specular, etc)  

**Tasks**:
1. Create uniform buffer for material properties
2. Implement standard lit shader (diffuse + specular)
3. Create material descriptor sets
4. Bind material uniforms in each draw call
5. Update fragment shader to use material properties
6. Test with different material parameters

**Acceptance**:
- [x] Material uniforms aplicando corretamente
- [x] - [x] Diffuse & specular colors visible
- [x] Multipland materials in um frame
- [x] No performancand degradation

**Next**: PHASE 4

---

### PHASE 4: Lighting System - DirectX Lights → Vulkan

**Dependencies**: PHASE 3 (materials OK)  
**Objective**: Convert D3DLIGHT8 structs to Vulkan uniform buffers  

**Tasks**:
1. Parse D3DLIGHT8 structures (directional, point, spot)
2. Create Vulkan uniform buffer layout for lights
3. Implement light validation & clamping
4. Update shader to support multiple lights (up to 8)
5. Implement light enable/disable
6. Test with various light configurations

**Acceptance**:
- [x] Directional light working
- [x] Point light with attenuation
- [x] Spot light with conand angle
- [x] 8 lights in um frame
- [x] Light enable/disabland works

**Next**: PHASE 5

---

### PHASE 5: Render Loop - Stable 60 FPS

**Dependencies**: PHASE 4 (lights OK)  
**Objective**: Correct frame timing, swapchain presentation  

**Tasks**:
1. Implement frame timing (delta time calculation)
2. Implement swapchain image acquisition
3. Implement command buffer submission
4. Implement frame synchronization (semaphores/fences)
5. Handle swapchain recreation
6. Frame rate limiting (vsync/target FPS)
7. Performance monitoring

**Acceptance**:
- [x] Consistently 60 FPS (no stuttering)
- [x] Framand timand < 16.67ms
- [x] No GPU stalls
- [x] Swapchain recreation smooth

**Next**: PHASE 6

---

### PHASE 6: Mesh Loading - Parse 3D Models

**Dependencies**: PHASE 5 (render loop OK)  
**Objective**: Load 3D mesh files (W3D, ASE, etc)  

**Tasks**:
1. Analyze existing mesh format (W3D in .big)
2. Parse mesh file structure
3. Extract vertex/index buffers
4. Extract material assignments
5. Create mesh data structures
6. Implement mesh caching

**Acceptance**:
- [x] Parsand valid .big mesh files
- [x] Correct vertex count
- [x] Correct index count
- [x] Material assignments prebeved

**Next**: PHASE 7

---

### PHASE 7: Mesh Rendering - Render 3D Models

**Dependencies**: PHASE 6 (meshes load OK)  
**Objective**: Render meshes with textures and materials  

**Tasks**:
1. Create mesh GPU buffers (vertex + index)
2. Implement draw call generation
3. Bind mesh textures & materials
4. Submit draw commands
5. Test with multiple meshes

**Acceptance**:
- [x] Meshes rendereds corretamente
- [x] Textures aplicadas corretamente
- [x] Multipland meshes sem z-fighting
- [x] Correct normals & lighting

**Next**: PHASE 8

---

### PHASE 8: Camera System - Viewpoint Control

**Dependencies**: PHASE 7 (meshes OK) - Phase 45 exists  
**Objective**: Implement camera modes (free, RTS, chase, orbit)  

**Tasks**:
1. Reuse Phase 45 camera implementation
2. Implement camera input controller
3. Setup view matrix uniforms
4. Implement projection matrix
5. Implement camera modes (RTS mode for menu)

**Acceptance**:
- [x] Camera moves with input
- [x] View matrix updating correctly
- [x] Projection correct (near/far clipping)
- [x] RTS modand working (isometric)

**Next**: PHASE 9

---

### PHASE 9: Frustum Culling - Optimization

**Dependencies**: PHASE 8 (camera OK)  
**Objective**: Render only visible objects  

**Tasks**:
1. Implement frustum plane calculation
2. Implement frustum-AABB intersection test
3. Cull meshes before draw call
4. Measure performance improvement

**Acceptance**:
- [x] Culling logic correct
- [x] No falsand positives/negatives
- [x] Performancand improvement measurable
- [x] No visual artifacts

**Next**: PHASE 10

---

### PHASE 10: Post-Processing - Effects (OPTIONAL)

**Dependencies**: PHASE 9 (culling OK)  
**Status**: Optional until initial menu  

**Tasks**:
1. Implement render target for post-processing
2. Implement tone mapping
3. Implement bloom (optional)
4. Implement color grading (optional)

**Acceptance**:
- [x] Post-processing aplicando
- [x] Visually pleasing results
- [x] No performancand impact

**Next**: PHASE 11

---

## Phases Detalhadas: PHASE 11-20 (UI & Assets)

### PHASE 11: UI Mesh Format - Parse UI Models

**Dependencies**: PHASE 7 (mesh rendering OK)  
**Objective**: Load UI meshes from .big files  

**Tasks**:
1. Identify UI mesh format in .big
2. Parse UI mesh files
3. Extract UI component data
4. Create UI mesh structures

**Acceptance**:
- [x] UI meshes carregam
- [x] Geometry correct
- [x] Bounds correct

**Next**: PHASE 12

---

### PHASE 12: UI Texture Loading - Load UI Textures

**Dependencies**: PHASE 11 (UI meshes OK)  
**Objective**: Load textures to UI components  

**Tasks**:
1. Identify UI texture location in .big
2. Load UI textures using Phase 28.4 pattern
3. Create texture binding to UI materials
4. Cache UI textures

**Acceptance**:
- [x] UI textures load
- [x] No artifacts
- [x] Caching working

**Next**: PHASE 13

---

### PHASE 13: UI Rendering - Render UI with Vulkan

**Dependencies**: PHASE 12 (UI textures OK)  
**Objective**: Render UI meshes with ortho camera  

**Tasks**:
1. Create ortho projection matrix
2. Setup UI rendering pass
3. Render UI meshes on top of background
4. Implement UI layer ordering

**Acceptance**:
- [x] UI aparecand na tela
- [x] Ortho projection correct
- [x] Layer ordering correct
- [x] UI aparecand abovand game

**Next**: PHASE 14

---

### PHASE 14: UI Layout - Positioning Widgets

**Dependencies**: PHASE 13 (UI rendering OK)  
**Objective**: Position UI components (buttons, labels, etc)  

**Tarefas**:
1. Parsand UI layout data from INI
2. Implement position/scaland transforms
3. Implement anchoring system
4. Updatand layout on window resize

**Acceptance**:
- [x] UI components positioned correctly
- [x] Scaling working
- [x] Responsivand to window size

**Next**: PHASE 15

---

### PHASE 15: Button System - Clickabland Areas

**Dependencies**: PHASE 14 (layout OK)  
**Objective**: Definand clickabland regions to UI buttons  

**Tarefas**:
1. Create button collision regions
2. Implement hit testing (ray-AABB)
3. Implement button states (idle, hover, pressed)
4. Create button data structures

**Acceptance**:
- [x] Buttons hit test correctly
- [x] Button states updating
- [x] Collision regions accurate

**Next**: PHASE 16

---

### PHASE 16: Input Routing - SDL2 → UI Callbacks

**Dependencies**: PHASE 15 (buttons OK)  
**Objective**: Routand SDL2 events to UI system  

**Tarefas**:
1. Create input routing layer (input_sdl2_routing_compat)
2. Map SDL2 mousand clicks → UI hit tests
3. Create callback system to button events
4. Implement hover tracking

**Acceptance**:
- [x] Clicks detectados
- [x] Routed to correct button
- [x] Hover visual feedback
- [x] No missed clicks

**Next**: PHASE 17

---

### PHASE 17: Menu Definitions - Parsand Menu INI

**Dependencies**: PHASE 16 (input routing OK)  
**Objective**: Parsand MainMenu.ini data  

**Tarefas**:
1. Identify menu INI location (.big)
2. Parsand menu button definitions
3. Parsand menu layout definitions
4. Create menu data structures
5. Extract menu textures/meshes references

**Acceptance**:
- [x] Menu definitions parse
- [x] No corrupted data
- [x] All buttons identified

**Next**: PHASE 18

---

### PHASE 18: Menu Statand Machinand - Menu Transitions

**Dependencies**: PHASE 17 (menu definitions OK)  
**Objective**: Implement menu statand transitions  

**Tarefas**:
1. Create menu statand enum (MAIN_MENU, SKIRMISH, CAMPAIGN, etc)
2. Implement statand transition logic
3. Create button → statand action mapping
4. Implement back button logic

**Acceptance**:
- [x] Statand transitions working
- [x] No stuck states
- [x] Back button working

**Next**: PHASE 19

---

### PHASE 19: Menu Rendering - Renderizar Menu Frame

**Dependencies**: PHASE 18 (statand machinand OK)  
**Objective**: Render completand menu framand each tick  

**Tarefas**:
1. Implement menu framand rendering loop
2. Render background (using mesh/texture)
3. Render all UI components
4. Render button statand changes (hover highlight)
5. Updatand framand every tick

**Acceptance**:
- [x] Menu appears on screen
- [x] All buttons visible
- [x] Updates smoothly
- [x] No visual artifacts

**Next**: PHASE 20

---

### PHASE 20: Menu Interaction - Clicks Trabalhando

**Dependencies**: PHASE 19 (menu rendering OK)  
**Objective**: Completand menu interaction loop  

**Tarefas**:
1. Test menu button clicks
2. Validate statand transitions
3. Test all menu paths (NEW GAME, LOAD, OPTIONS, EXIT)
4. Fix any interaction bugs
5. Polish animations/transitions

**Acceptance**:
- [x] New Game button works
- [x] Statand transitions smooth
- [x] All paths accessible
- [x] Menu responsive

**Next**: PHASE 21

---

## Detailed Phases: PHASE 21-30 (Game Logic)

### PHASE 21-30: Game Logic Integration

**Status**: Phase 46 partially implemented (reuse where possible)

**Sequence**:
- PHASE 21: GameObject System (reuse Phase 46)
- PHASE 22: World Management (reuse Phase 46)
- PHASE 23: Game Loop (reuse Phase 46)
- PHASE 24-26: Input & Commands (validate/extend)
- PHASE 27-28: Physics/Pathfinding (stubs OK for MVP)
- PHASE 29-30: Rendering & Transitions

**Acceptance Final**:
- [x] Game transitions from menu
- [x] Game world initializes
- [x] Objects appear in world
- [x] Input works in game
- [x] Statand transitions smooth

**Next**: PHASE 31

---

## Detailed Phases: PHASE 31-40 (Polish)

### PHASE 31-40: Polish & Stabilization

**Status**: Refinement phases

**Tasks**:
- PHASE 31: Audio system (Phase 33 OpenAL)
- PHASE 32-33: Performance optimization
- PHASE 34-35: Crash handling & fixes
- PHASE 36-37: Testing & documentation
- PHASE 38-39: Build system & cross-platform
- PHASE 40: Release validation

**Final Acceptance**:
- [x] Stable on all platforms
- [x] No crashes in normal gameplay
- [x] Performance acceptable
- [x] Documentation complete
- [x] Ready for minimal playable release

**Final Milestone**: ✅ **INITIAL MENU WORKING**

---

## Dependency Graph (ASCII)

```
PHASE 0: Spike Planning
  ↓
PHASE 1: Geometry Rendering (triangle)
  ↓
PHASE 2: Texture System
  ↓
PHASE 3: Material System
  ↓
PHASE 4: Lighting System
  ↓
PHASE 5: Render Loop (60 FPS)
  ↓
PHASE 6: Mesh Loading
  ↓
PHASE 7: Mesh Rendering
  ↓
PHASE 8: Camera System
  ↓
PHASE 9: Frustum Culling
  ↓
PHASE 10: Post-Processing (opt)
  ↓
PHASE 11: UI Mesh Format
  ↓
PHASE 12: UI Texture Loading
  ↓
PHASE 13: UI Rendering
  ↓
PHASE 14: UI Layout
  ↓
PHASE 15: Button System
  ↓
PHASE 16: Input Routing
  ↓
PHASE 17: Menu Definitions
  ↓
PHASE 18: Menu Statand Machine
  ↓
PHASE 19: Menu Rendering
  ↓
PHASE 20: Menu Interaction
  ↓
PHASE 21-30: Game Logic Integration (parallel with 20)
  ↓
PHASE 31-40: Polish & Stabilization
  ↓
🎉 INITIAL MENU WORKING
```

---

## Summary of Phases

| Phase | Name | Dependencies | Blocker |
|------|------|--------------|-----------|
| 0 | Spike Planning | None | No |
| 1-5 | Core Graphics (Foundation) | Previous | Yes |
| 6-10 | Advanced Graphics | 1-5 | No (10 opt) |
| 11-16 | UI Infrastructure | 6-10 | Yes |
| 17-20 | Menu System | 11-16 | Yes |
| 21-30 | Game Logic | 1-7, 20 | Parallel |
| 31-40 | Polish & Release | 20-30 | Final |

---

## Next Steps

1. ✅ PHASE 0 planning complete
2. ⏳ Validate roadmap with user
3. ⏳ Start PHASE 1: Geometry Rendering
4. ⏳ Cycle of phases: code → test → document → next
