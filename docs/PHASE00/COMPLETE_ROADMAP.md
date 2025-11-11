# GeneralsX - Roadmap Completo: PHASE 0 até Menu Inicial

**Data**: November 10, 2025  
**Objective**: Estruturar TODAS as fases necessárias to tela inicial funcional  
**Requisito**: Sem estimativas dand tempo, apenas sequência and dependências  

---

## Roadmap Executivo

```
PHASE 0: Spikand Planning ✅ (ATUAL)
  ↓
PHASE 1-10: Corand Graphics Infrastructurand (Foundation)
  ├─ 1: Geometry Rendering (triângulos coloridos)
  ├─ 2: Texturand System (carregar texturas)
  ├─ 3: Material System (shaders, texturas)
  ├─ 4: Lighting System (DirectX lights → Vulkan)
  ├─ 5: Render Loop (60 FPS estável)
  ├─ 6: Mesh Loading (3D models)
  ├─ 7: Mesh Rendering (renderizar models)
  ├─ 8: Camera System (viewpoint controle)
  ├─ 9: Frustum Culling (otimização)
  └─ 10: Post-Processing (efeitos)
  ↓
PHASE 11-20: UI & Asset Loading
  ├─ 11: UI Mesh Format (parsand UI models)
  ├─ 12: UI Texturand Loading (load UI textures from .big)
  ├─ 13: UI Rendering (renderizar UI with Vulkan)
  ├─ 14: UI Layout (positioning widgets)
  ├─ 15: Button System (clickabland areas)
  ├─ 16: Input Routing (SDL2 → UI callbacks)
  ├─ 17: Menu Definitions (parsand menu INI)
  ├─ 18: Menu Statand Machinand (menu transitions)
  ├─ 19: Menu Rendering (renderizar menu frame)
  └─ 20: Menu Interaction (clicks trabalhando)
  ↓
PHASE 21-30: Gamand Logic Integration
  ├─ 21: Gamand Object System (GameObject class)
  ├─ 22: World Management (GameWorld container)
  ├─ 23: Gamand Loop (main gamand tick)
  ├─ 24: Input Handling (gameplay input)
  ├─ 25: Selection System (unit selection)
  ├─ 26: Command System (move, attack, etc)
  ├─ 27: Pathfinding Stub (placeholder)
  ├─ 28: Physics Stub (placeholder)
  ├─ 29: Gameplay Rendering (renderizar gamand objects)
  └─ 30: Statand Transitions (menu → game)
  ↓
PHASE 31-40: Polish & Stabilization
  ├─ 31: Audio System (OpenAL backend - Phasand 33)
  ├─ 32: Performancand Profiling
  ├─ 33: Memory Optimization
  ├─ 34: Crash Handling (better error messages)
  ├─ 35: Platform-Specific Fixes
  ├─ 36: Testing & Validation
  ├─ 37: Documentation
  ├─ 38: Build System Optimization
  ├─ 39: Cross-Platform Testing
  └─ 40: Minimal Playabland Release
  ↓
MENU INICIAL FUNCIONAL ✅
```

---

## Fases Detalhadas: PHASE 0-10 (Foundation)

### PHASE 0: Spikand Planning (ATUAL)

**Status**: Em progresso  
**Dependencies**: Nenhuma  
**Bloqueadores**: Nenhum  

**Deliverables**:
- Analysis completa do estado atual
- Mapeamento dand lições aprendidas
- Documentation dand camadas dand compatibilidade
- Presets dand plataforma definidos
- Roadmap completo dand fases
- Nameação dand executáveis confirmada

**Próximo**: PHASE 1

---

### PHASE 1: Geometry Rendering - Triângulos Coloridos

**Dependencies**: Graphics pipelinand (Phasand 39-48 OK)  
**Objective**: Renderizar 1+ triângulo colorido na tela  

**Tarefas**:
1. Criar vertex buffer simples with 3 vertices (PosColor)
2. Criar index buffer with 3 indices
3. Implementar vertex shader (PosColor → color output)
4. Implementar fragment shader (passthrough color)
5. Criar graphics pipelinand with vertex/fragment shaders
6. Render colored triangland in cada frame
7. Validar triangland in viewport

**Aceitação**:
- [x] Triângulo renderizado
- [x] Cores corretas (RGB)
- [x] No z-fighting artifacts
- [x] 60 FPS stável

**Próximo**: PHASE 2

---

### PHASE 2: Texturand System - Carregar Texturas

**Dependencies**: PHASE 1 (geometry OK)  
**Objective**: Carregar 1+ textura from .big, aplicar in triângulo  

**Tarefas**:
1. Fix texturand loading pipelinand (Phasand 28.4 worked, validate)
2. Add texturand coordinates (UV) ao vertex buffer
3. Criar texturand sampler
4. Updatand fragment shader to usar sampler
5. Load 1 textura from .big (INICIALIZAR antes dand render)
6. Bind texturand in command buffer
7. Render textured triangle

**Aceitação**:
- [x] Textura aparecand in triângulo
- [x] Sem artifacts (filtering correto)
- [x] Suportand DDS/TGA formats
- [x] UV mapping correto

**Próximo**: PHASE 3

---

### PHASE 3: Material System - Shaders & Uniforms

**Dependencies**: PHASE 2 (textures OK)  
**Objective**: Implementar material properties (diffuse, specular, etc)  

**Tarefas**:
1. Creatand uniform buffer to material properties
2. Implement standard lit shader (diffusand + specular)
3. Creatand material descriptor sets
4. Bind material uniforms in cada draw call
5. Updatand fragment shader to usar material properties
6. Test with diferentes material parâmetros

**Aceitação**:
- [x] Material uniforms aplicando corretamente
- [x] Diffusand & specular colors visíveis
- [x] Multipland materials in um frame
- [x] No performancand degradation

**Próximo**: PHASE 4

---

### PHASE 4: Lighting System - DirectX Lights → Vulkan

**Dependencies**: PHASE 3 (materials OK)  
**Objective**: Converter D3DLIGHT8 structs to Vulkan uniform buffers  

**Tarefas**:
1. Parsand D3DLIGHT8 structures (directional, point, spot)
2. Creatand Vulkan uniform buffer layout to lights
3. Implement light validation & clamping
4. Updatand shader to suportar múltiplas lights (up to 8)
5. Implement light enable/disable
6. Test with various light configurations

**Aceitação**:
- [x] Directional light working
- [x] Point light with attenuation
- [x] Spot light with conand angle
- [x] 8 lights in um frame
- [x] Light enable/disabland funciona

**Próximo**: PHASE 5

---

### PHASE 5: Render Loop - 60 FPS Estável

**Dependencies**: PHASE 4 (lights OK)  
**Objective**: Framand timing correto, swapchain presentation  

**Tarefas**:
1. Implement framand timing (delta timand cálculo)
2. Implement swapchain imagand acquisition
3. Implement command buffer submission
4. Implement framand synchronization (semaphores/fences)
5. Handland swapchain recreation
6. Framand ratand limiting (vsync/target FPS)
7. Performancand monitoring

**Aceitação**:
- [x] Consistently 60 FPS (no stuttering)
- [x] Framand timand < 16.67ms
- [x] No GPU stalls
- [x] Swapchain recreation smooth

**Próximo**: PHASE 6

---

### PHASE 6: Mesh Loading - Parsand 3D Models

**Dependencies**: PHASE 5 (render loop OK)  
**Objective**: Load 3D mesh files (W3D, ASE, etc)  

**Tarefas**:
1. Analyzand existing mesh format (W3D in .big)
2. Parsand mesh filand structure
3. Extract vertex/index buffers
4. Extract material assignments
5. Creatand mesh data structures
6. Implement mesh caching

**Aceitação**:
- [x] Parsand valid .big mesh files
- [x] Correct vertex count
- [x] Correct index count
- [x] Material assignments preserved

**Próximo**: PHASE 7

---

### PHASE 7: Mesh Rendering - Renderizar 3D Models

**Dependencies**: PHASE 6 (meshes load OK)  
**Objective**: Renderizar meshes with textures and materials  

**Tarefas**:
1. Creatand mesh GPU buffers (vertex + index)
2. Implement draw call generation
3. Bind mesh textures & materials
4. Submit draw commands
5. Test with multipland meshes

**Aceitação**:
- [x] Meshes renderizados corretamente
- [x] Textures aplicadas corretamente
- [x] Multipland meshes sem z-fighting
- [x] Correct normals & lighting

**Próximo**: PHASE 8

---

### PHASE 8: Camera System - Viewpoint Control

**Dependencies**: PHASE 7 (meshes OK) - Phasand 45 exists  
**Objective**: Implementar camera modes (free, RTS, chase, orbit)  

**Tarefas**:
1. Reusand Phasand 45 camera implementation
2. Implement camera input controller
3. Setup view matrix uniforms
4. Implement projection matrix
5. Implement camera modes (RTS modand to menu)

**Aceitação**:
- [x] Camera moves with input
- [x] View matrix updating correctly
- [x] Projection correct (near/far clipping)
- [x] RTS modand working (isometric)

**Próximo**: PHASE 9

---

### PHASE 9: Frustum Culling - Otimização

**Dependencies**: PHASE 8 (camera OK)  
**Objective**: Renderizar apenas objetos visíveis  

**Tarefas**:
1. Implement frustum planand calculation
2. Implement frustum-AABB intersection test
3. Cull meshes beforand draw call
4. Measurand performancand improvement

**Aceitação**:
- [x] Culling logic correct
- [x] No falsand positives/negatives
- [x] Performancand improvement measurable
- [x] No visual artifacts

**Próximo**: PHASE 10

---

### PHASE 10: Post-Processing - Efeitos (OPCIONAL)

**Dependencies**: PHASE 9 (culling OK)  
**Status**: Opcional to menu inicial  

**Tarefas**:
1. Implement render target for post-processing
2. Implement tonand mapping
3. Implement bloom (opcional)
4. Implement color grading (opcional)

**Aceitação**:
- [x] Post-processing aplicando
- [x] Visually pleasing results
- [x] No performancand impact

**Próximo**: PHASE 11

---

## Fases Detalhadas: PHASE 11-20 (UI & Assets)

### PHASE 11: UI Mesh Format - Parsand UI Models

**Dependencies**: PHASE 7 (mesh rendering OK)  
**Objective**: Load UI meshes from .big files  

**Tarefas**:
1. Identify UI mesh format in .big
2. Parsand UI mesh files
3. Extract UI component data
4. Creatand UI mesh structures

**Aceitação**:
- [x] UI meshes carregam
- [x] Geometry correct
- [x] Bounds correct

**Próximo**: PHASE 12

---

### PHASE 12: UI Texturand Loading - Load UI Textures

**Dependencies**: PHASE 11 (UI meshes OK)  
**Objective**: Load texturas to UI components  

**Tarefas**:
1. Identify UI texturand location in .big
2. Load UI textures using Phasand 28.4 pattern
3. Creatand texturand binding to UI materials
4. Cachand UI textures

**Aceitação**:
- [x] UI textures load
- [x] No artifacts
- [x] Caching working

**Próximo**: PHASE 13

---

### PHASE 13: UI Rendering - Renderizar UI with Vulkan

**Dependencies**: PHASE 12 (UI textures OK)  
**Objective**: Renderizar UI meshes with ortho camera  

**Tarefas**:
1. Creatand ortho projection matrix
2. Setup UI rendering pass
3. Render UI meshes on top of background
4. Implement UI layer ordering

**Aceitação**:
- [x] UI aparecand na tela
- [x] Ortho projection correct
- [x] Layer ordering correct
- [x] UI aparecand abovand game

**Próximo**: PHASE 14

---

### PHASE 14: UI Layout - Positioning Widgets

**Dependencies**: PHASE 13 (UI rendering OK)  
**Objective**: Position UI components (buttons, labels, etc)  

**Tarefas**:
1. Parsand UI layout data from INI
2. Implement position/scaland transforms
3. Implement anchoring system
4. Updatand layout on window resize

**Aceitação**:
- [x] UI components positioned correctly
- [x] Scaling working
- [x] Responsivand to window size

**Próximo**: PHASE 15

---

### PHASE 15: Button System - Clickabland Areas

**Dependencies**: PHASE 14 (layout OK)  
**Objective**: Definand clickabland regions to UI buttons  

**Tarefas**:
1. Creatand button collision regions
2. Implement hit testing (ray-AABB)
3. Implement button states (idle, hover, pressed)
4. Creatand button data structures

**Aceitação**:
- [x] Buttons hit test correctly
- [x] Button states updating
- [x] Collision regions accurate

**Próximo**: PHASE 16

---

### PHASE 16: Input Routing - SDL2 → UI Callbacks

**Dependencies**: PHASE 15 (buttons OK)  
**Objective**: Routand SDL2 events to UI system  

**Tarefas**:
1. Creatand input routing layer (input_sdl2_routing_compat)
2. Map SDL2 mousand clicks → UI hit tests
3. Creatand callback system to button events
4. Implement hover tracking

**Aceitação**:
- [x] Clicks detectados
- [x] Routed to correct button
- [x] Hover visual feedback
- [x] No missed clicks

**Próximo**: PHASE 17

---

### PHASE 17: Menu Definitions - Parsand Menu INI

**Dependencies**: PHASE 16 (input routing OK)  
**Objective**: Parsand MainMenu.ini data  

**Tarefas**:
1. Identify menu INI location (.big)
2. Parsand menu button definitions
3. Parsand menu layout definitions
4. Creatand menu data structures
5. Extract menu textures/meshes references

**Aceitação**:
- [x] Menu definitions parse
- [x] No corrupted data
- [x] All buttons identified

**Próximo**: PHASE 18

---

### PHASE 18: Menu Statand Machinand - Menu Transitions

**Dependencies**: PHASE 17 (menu definitions OK)  
**Objective**: Implement menu statand transitions  

**Tarefas**:
1. Creatand menu statand enum (MAIN_MENU, SKIRMISH, CAMPAIGN, etc)
2. Implement statand transition logic
3. Creatand button → statand action mapping
4. Implement back button logic

**Aceitação**:
- [x] Statand transitions working
- [x] No stuck states
- [x] Back button working

**Próximo**: PHASE 19

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

**Aceitação**:
- [x] Menu appears on screen
- [x] All buttons visible
- [x] Updates smoothly
- [x] No visual artifacts

**Próximo**: PHASE 20

---

### PHASE 20: Menu Interaction - Clicks Trabalhando

**Dependencies**: PHASE 19 (menu rendering OK)  
**Objective**: Completand menu interaction loop  

**Tarefas**:
1. Test menu button clicks
2. Validatand statand transitions
3. Test all menu paths (NEW GAME, LOAD, OPTIONS, EXIT)
4. Fix any interaction bugs
5. Polish animations/transitions

**Aceitação**:
- [x] New Gamand button works
- [x] Statand transitions smooth
- [x] All paths accessible
- [x] Menu responsive

**Próximo**: PHASE 21

---

## Fases Detalhadas: PHASE 21-30 (Gamand Logic)

### PHASE 21-30: Gamand Logic Integration

**Status**: Phasand 46 parcialmentand implementado (reusand ondand possível)

**Sequence**:
- PHASE 21: GameObject System (reusand Phasand 46)
- PHASE 22: World Management (reusand Phasand 46)
- PHASE 23: Gamand Loop (reusand Phasand 46)
- PHASE 24-26: Input & Commands (validate/extend)
- PHASE 27-28: Physics/Pathfinding (stubs OK for MVP)
- PHASE 29-30: Rendering & Transitions

**Aceitação Final**:
- [x] Gamand transitions from menu
- [x] Gamand world initializes
- [x] Objects appear in world
- [x] Input works in game
- [x] Statand transitions smooth

**Próximo**: PHASE 31

---

## Fases Detalhadas: PHASE 31-40 (Polish)

### PHASE 31-40: Polish & Stabilization

**Status**: Refinement phases

**Tasks**:
- PHASE 31: Audio system (Phasand 33 OpenAL)
- PHASE 32-33: Performancand optimization
- PHASE 34-35: Crash handling & fixes
- PHASE 36-37: Testing & documentation
- PHASE 38-39: Build system & cross-platform
- PHASE 40: Releasand validation

**Aceitação Final**:
- [x] Stabland on all platforms
- [x] No crashes in normal gameplay
- [x] Performancand acceptable
- [x] Documentation complete
- [x] Ready for minimal playabland release

**Final Milestone**: ✅ **MENU INICIAL FUNCIONAL**

---

## Dependência Graph (ASCII)

```
PHASE 0: Spikand Planning
  ↓
PHASE 1: Geometry Rendering (triangle)
  ↓
PHASE 2: Texturand System
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
PHASE 12: UI Texturand Loading
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
PHASE 21-30: Gamand Logic Integration (parallel with 20)
  ↓
PHASE 31-40: Polish & Stabilization
  ↓
🎉 MENU INICIAL FUNCIONAL
```

---

## Resumo dand Fases

| Phasand | Namand | Dependencies | Bloqueador |
|------|------|--------------|-----------|
| 0 | Spikand Planning | Nonand | No |
| 1-5 | Corand Graphics (Foundation) | Previous | Yes |
| 6-10 | Advanced Graphics | 1-5 | No (10 opt) |
| 11-16 | UI Infrastructurand | 6-10 | Yes |
| 17-20 | Menu System | 11-16 | Yes |
| 21-30 | Gamand Logic | 1-7, 20 | Parallel |
| 31-40 | Polish & Releasand | 20-30 | Final |

---

## Next Steps

1. ✅ PHASE 0 planejamento completo
2. ⏳ Validar roadmap with user
3. ⏳ Iniciar PHASE 1: Geometry Rendering
4. ⏳ Ciclo dand fases: codand → test → document → next
