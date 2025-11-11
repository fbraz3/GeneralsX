# Roadmap Completo: Do Zero à Tela Inicial Funcional com Vulkan

**Data**: 11 de Novembro de 2025  
**Objetivo**: Mapear TODAS as fases necessárias para ter a tela inicial (menu principal) funcional com Vulkan  
**Estratégia**: Vulkan-only, cross-platform (macOS via MoltenVK, Linux/Windows nativo)  
**Status**: 🔵 Planejamento completo

---

## 1. ANÁLISE CRÍTICA DO ESTADO ATUAL

### 1.1 Lições Aprendidas (Consolidado de LESSONS_LEARNED.md)

**VFS Discovery (Outubro 17, 2025)**
- ✅ `.big` files armazenam assets (textures, sounds, INI)
- ✅ DirectX carrega via VFS automaticamente
- ✅ Interception pós-DirectX é mais confiável que modificar VFS
- ✅ Textures agora carregam para Metal backend com sucesso

**INI Parser Blocker (Outubro 19, 2025)**
- ❌ Blanket exception catching (`catch(...)`) engoliu exceções
- ❌ Todos os floats/strings leem como valores padrão (0.00, '')
- ✅ Solução: Remover `catch(...)` blanket, implementar re-throw com contexto
- 🔴 BLOQUEADOR: Audiomanager, music.ini todos quebrados até fix

**Metal Crashes (Outubro 24, 2025)**
- ❌ Storing local ARC objects em globals causava dangling pointers
- ✅ Solução: Deixar ARC gerenciar ciclo de vida automaticamente
- ✅ Pattern recognition from Phase 33.9 salvou horas de debug

**Vulkan Analysis (Outubro 30, 2025)**
- ✅ Phase 38 interface (47 métodos) mapeia 1:1 para Vulkan
- ✅ DXVK é viável em macOS ARM64
- ✅ MoltenVK + Vulkan SDK 1.4.328.1 testado e funcionando
- ✅ Estimativa: 18-26 horas para Phase 39 (DXVK implementation)

### 1.2 Estado Técnico Atual

**Build System**: ✅ Funcional
- CMake presets (macos-arm64, linux, windows) OK
- ccache integrado (30-60s rebuilds)
- Compatibility layer estabelecida
- Build progride ~56/908 compilation units antes de graphics blocker

**Graphics Stack**: 🟡 Parcial
- DirectX 8 mock layer em lugar
- Metal wrapper funcional (criação de texturas, render buffers)
- Vulkan SDK instalado e validado
- **FALTA**: Vulkan backend de verdade (Phase 39+)

**Asset Loading**: 🟡 Parcial
- ✅ VFS funciona (DirectX carrega de .big files)
- ✅ Texture interception pós-DirectX funciona (7 textures loaded)
- ❌ INI parsing quebrado (blanket exception catching)
- ❌ Audio system pronto mas untestable (volumes = 0.00)

**Menu System**: ❌ Não iniciado
- INI parsing para menu definitions não verificado
- Menu state machine não implementado
- Menu rendering não implementado
- Button interaction não implementado

---

## 2. CAMINHO CRÍTICO ATÉ TELA INICIAL

```
Start
  ↓
[PHASE 00.5] ✅ Build Infrastructure
  ↓
[PHASE 01] Vulkan Instance & Device Setup
  ├─ VkInstance creation
  ├─ Physical device enumeration
  ├─ VkDevice & VkQueue creation
  ├─ VkCommandPool initialization
  └─ Framebuffer + Swapchain setup
  ↓
[PHASE 02] Vulkan Command Buffers & Rendering
  ├─ VkCommandBuffer recording
  ├─ VkRenderPass definition
  ├─ Framebuffer attachment setup
  └─ Begin/End render pass cycle
  ↓
[PHASE 03] Shader System (SPIR-V)
  ├─ GLSL → SPIR-V compilation
  ├─ VkShaderModule creation
  ├─ Shader stages binding
  └─ Pipeline shader layout
  ↓
[PHASE 04-05] Buffers & Textures
  ├─ Vertex buffer creation (VkBuffer)
  ├─ Index buffer creation
  ├─ Texture loading (VkImage, VkImageView)
  ├─ Sampler creation (VkSampler)
  └─ Descriptor sets for texture binding
  ↓
[PHASE 06] Pipeline State Management
  ├─ VkGraphicsPipeline creation
  ├─ VkPipelineLayout setup
  ├─ Render state mapping (D3D → Vulkan)
  ├─ Dynamic state handling
  └─ Pipeline caching
  ↓
[PHASE 07] Basic Drawing & Rendering Loop
  ├─ Draw calls (vkCmdDraw, vkCmdDrawIndexed)
  ├─ Frame presentation (vkQueuePresentKHR)
  ├─ Synchronization (VkSemaphore, VkFence)
  ├─ Frame timing
  └─ Cleanup between frames
  ↓
[PHASE 08] Asset Loading from .big Files
  ├─ VFS integration verification
  ├─ DirectX texture loading → Vulkan conversion
  ├─ Mesh loading (.w3d format)
  ├─ Material parsing (IW.big files)
  └─ Error handling for missing assets
  ↓
[PHASE 09] INI Parser Fix & Hardening
  ├─ Remove blanket exception catching
  ├─ Implement proper exception re-throwing
  ├─ Field validation before parsing
  ├─ Test all INI types (float, string, int, etc.)
  └─ Verify all game configs parse correctly
  ↓
[PHASE 10] Window System & Input
  ├─ SDL2 window creation & management
  ├─ Vulkan surface creation (VkSurfaceKHR)
  ├─ Swapchain image management
  ├─ Input event handling (keyboard, mouse)
  └─ Window resize handling
  ↓
[PHASE 11] Camera System
  ├─ View matrix calculation
  ├─ Projection matrix setup (orthographic for menu, perspective for game)
  ├─ Matrix uniform buffer creation
  ├─ Camera movement & orientation
  └─ Frustum calculation
  ↓
[PHASE 12] Lighting System
  ├─ Light uniform buffers (position, color, intensity)
  ├─ Lighting shader implementation (Phong/PBR)
  ├─ Light updates per frame
  ├─ Shadow mapping foundation (if needed for menu)
  └─ Ambient + directional + point lights
  ↓
[PHASE 13] Mesh Rendering
  ├─ Mesh data upload to Vulkan buffers
  ├─ Vertex attribute binding setup
  ├─ Index buffer binding
  ├─ Multi-mesh rendering
  └─ Mesh transformation (model matrix)
  ↓
[PHASE 14] Texture & Material System
  ├─ Texture binding to shaders (descriptor sets)
  ├─ Material properties (color, shininess, etc.)
  ├─ Multi-texture support (diffuse, normal, specular)
  ├─ Texture filtering (linear, nearest)
  └─ Mipmap handling
  ↓
[PHASE 15] Game Object System (Entity Foundation)
  ├─ GameObject class hierarchy
  ├─ Transform components
  ├─ Mesh components
  ├─ Material components
  ├─ Rendering components
  └─ Update/render dispatch
  ↓
[PHASE 16] Menu INI Definitions Parser
  ├─ Parse MenuDefinition.ini from WINDOW.big
  ├─ Screen definitions (layout, position)
  ├─ ScreenTransition.ini parsing
  ├─ WindowTransitions.ini for animations
  ├─ NamedGameWindows.ini for named references
  └─ Error recovery for malformed INI
  ↓
[PHASE 17] Menu State Machine
  ├─ MainMenuScreen state
  ├─ SkirmishScreen state
  ├─ Multiplayer screen state (if needed for initial)
  ├─ Loading screen state
  ├─ State transitions (animations)
  ├─ Event dispatching per state
  └─ Resume/suspend state management
  ↓
[PHASE 18] Menu UI Rendering Pipeline
  ├─ Orthographic projection setup (2D menu coords)
  ├─ UI mesh generation from INI definitions
  ├─ Button geometry generation
  ├─ Text rendering (font loading, rasterization)
  ├─ Screen background image loading
  └─ UI layer sorting & composition
  ↓
[PHASE 19] Button & Interactive Element System
  ├─ Button hitbox calculation
  ├─ Mouse over detection (picking)
  ├─ Button state machine (normal, hover, pressed, disabled)
  ├─ Visual feedback (highlight, press animation)
  ├─ Button click callbacks
  └─ Keyboard navigation (tab through buttons)
  ↓
[PHASE 20] Audio System Integration
  ├─ OpenAL initialization
  ├─ Audio clip loading from .big files
  ├─ Background music playback
  ├─ Button click sound effects
  ├─ Volume control
  └─ Audio event triggering from buttons
  ↓
[PHASE 21] Main Menu Initial Screen Display
  ├─ Load menu INI (MenuDefinition.ini + refs)
  ├─ Create UI geometry from definitions
  ├─ Setup rendering pipeline for menu
  ├─ Load menu background image
  ├─ Load menu music
  ├─ Create button geometries & text
  ├─ Setup input event handlers
  └─ Display main menu screen
  ↓
🎉 TELA INICIAL FUNCIONAL!
```

---

## 3. FASES DETALHADAS PARA TELA INICIAL

### Phase 01: Vulkan Instance & Device Setup (2-3 dias)

**Dependências**: PHASE00.5 (Build system)

**Objetivo**: Inicializar Vulkan, enumerar devices, criar queues

**Deliverables**:
- ✅ VkInstance criada com extensões necessárias (macOS: MoltenVK, Linux/Windows: nativas)
- ✅ Physical device enumeration (GPU selecionado)
- ✅ VkDevice criado com graphics queue
- ✅ VkCommandPool inicializado
- ✅ VkQueue acesso para submission

**Tecnicamente**:
```cpp
// Arquivos principais
Core/Libraries/Source/WWVegas/WW3D2/vulkan_instance.h/cpp  (novo)
Core/Libraries/Source/WWVegas/WW3D2/vulkan_device.h/cpp    (novo)
Core/Libraries/Source/WWVegas/WW3D2/vulkan_queue.h/cpp     (novo)
```

**Validação**:
- Imprimir versão Vulkan em startup
- Listar physical devices disponíveis
- Verificar queue family properties
- Confirmar command pool alocação

**Notas Críticas**:
- macOS requer MoltenVK (já instalado via Vulkan SDK)
- Physical device sempre GPU (não CPU em macOS)
- Command pool = thread-safe para múltiplas threads

---

### Phase 02: Vulkan Command Buffers & Rendering (2-3 dias)

**Dependências**: Phase 01

**Objetivo**: Setup basic rendering pipeline (record commands, render passes)

**Deliverables**:
- ✅ VkCommandBuffer allocation & recording
- ✅ VkRenderPass criado (color attachment)
- ✅ VkFramebuffer setup (backbuffer)
- ✅ Begin/EndRenderPass cycle
- ✅ Frame synchronization (semaphores, fences)

**Tecnicamente**:
```cpp
// Novos arquivos
Core/Libraries/Source/WWVegas/WW3D2/vulkan_render_pass.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_framebuffer.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_sync.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_command_buffer.h/cpp
```

**Validação**:
- Frame renders without crashes
- Semaphore/fence handling correct (no deadlocks)
- Clear color works (render black/white/color to screen)

---

### Phase 03: Shader System (SPIR-V) (2-3 dias)

**Dependências**: Phase 02

**Objetivo**: Compile GLSL → SPIR-V, create shader modules

**Deliverables**:
- ✅ GLSL shader files criados (vertex, fragment)
- ✅ GLSL → SPIR-V compilation (glslc ou shaderc)
- ✅ VkShaderModule creation
- ✅ Shader stage binding
- ✅ Pipeline shader layout

**Tecnicamente**:
```cpp
// Novos arquivos
Core/Libraries/Source/WWVegas/WW3D2/vulkan_shader.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/shaders/  (diretório)
  ├── basic.vert.glsl
  ├── basic.frag.glsl
  ├── ui.vert.glsl
  ├── ui.frag.glsl
  └── Makefile (compilation rules)
```

**Validação**:
- Shader compilation succeeds (glslc validates SPIR-V)
- VkShaderModule creates without errors
- Pipeline accepts shader stages

**Notas Críticas**:
- Deve suportar basic lighting (Phong) mais tarde
- UI shaders separados (orthographic vs perspective)
- Compilation como parte do build system

---

### Phase 04: Vertex & Index Buffers (2 dias)

**Dependências**: Phase 03

**Objetivo**: GPU buffer allocation & upload

**Deliverables**:
- ✅ VkBuffer creation (vertex buffer)
- ✅ VkBuffer creation (index buffer)
- ✅ GPU memory allocation (VmaAllocator para simplificar)
- ✅ CPU → GPU data transfer
- ✅ Vertex attribute binding

**Tecnicamente**:
```cpp
// Novos arquivos
Core/Libraries/Source/WWVegas/WW3D2/vulkan_buffer.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_memory.h/cpp
Dependencies/  (add VulkanMemoryAllocator if needed)
```

**Validação**:
- Buffer memory allocation succeeds
- Data upload is correct (CPU → GPU)
- Binding works in render pass

---

### Phase 05: Texture Loading & Sampling (3 dias)

**Dependências**: Phase 04

**Objetivo**: Load textures, create samplers

**Deliverables**:
- ✅ VkImage creation
- ✅ VkImageView for each texture
- ✅ VkSampler creation (linear, nearest filtering)
- ✅ Descriptor set allocation (bind textures to shaders)
- ✅ Mipmap generation (if needed)

**Tecnicamente**:
```cpp
// Novos arquivos
Core/Libraries/Source/WWVegas/WW3D2/vulkan_texture.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_sampler.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_descriptor.h/cpp

// Integração com existente
Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp (apply_new_surface revisited)
```

**Validação**:
- Load test texture (branco, preto, gradient)
- Sampler filtering works (linear vs nearest)
- Descriptor sets bind correctly
- Texture appears in render output

---

### Phase 06: Pipeline State & Material System (3 dias)

**Dependências**: Phase 05

**Objetivo**: Graphics pipeline creation, material state management

**Deliverables**:
- ✅ VkGraphicsPipeline creation
- ✅ VkPipelineLayout setup
- ✅ Render state mapping (D3DRS_* → Vulkan equivalents)
- ✅ Dynamic state handling
- ✅ Pipeline caching (VkPipelineCache)

**Tecnicamente**:
```cpp
// Novos arquivos
Core/Libraries/Source/WWVegas/WW3D2/vulkan_pipeline.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_render_state.h/cpp
Core/Libraries/Source/WWVegas/WW3D2/vulkan_material.h/cpp

// Mapeamento crítico: D3D renderstate → Vulkan
// ex: D3DRS_ALPHABLENDENABLE → VkPipelineColorBlendAttachmentState
```

**Validação**:
- Pipeline creates without errors
- Material changes work (blending, culling, etc)
- No pipeline creation on every frame (cache hit)

**Notas Críticas**:
- Renderstate mapping deve ser completo (da análise VULKAN_ANALYSIS.md)
- Pipeline layout deve permitir descriptors + push constants
- Dynamic states: viewport, scissor, blend constants

---

### Phase 07: Basic Drawing & Frame Presentation (2 dias)

**Dependências**: Phase 06

**Objetivo**: Draw calls, present to screen

**Deliverables**:
- ✅ vkCmdDraw implementation
- ✅ vkCmdDrawIndexed implementation
- ✅ vkQueuePresentKHR submission
- ✅ Frame timing (FPS counter)
- ✅ Cleanup between frames

**Validação**:
- Triangle rendered to screen
- FPS counter displays (target 60 FPS)
- No memory leaks (vkProfiling layers)
- Smooth frames (no hitches)

---

### Phase 08: Asset Loading from .big Files (3-4 dias)

**Dependências**: Phase 07

**Objetivo**: Integrate DirectX texture loading → Vulkan upload

**Deliverables**:
- ✅ VFS verified working (already done in Phase 28.4)
- ✅ DirectX texture loader integration (apply_new_surface)
- ✅ Format conversion (DDS, TGA → Vulkan)
- ✅ Mesh loading (.w3d files)
- ✅ Error handling for missing assets

**Validação**:
- Load 10+ textures from TexturesZH.big
- Verify texture appearance in render output
- Load test mesh from game assets
- Handle missing texture gracefully

**Notas Críticas**:
- Reusa trabalho de Phase 28.4 (texture interception)
- Deve carregar textures comprimidas (BC1/2/3)
- .w3d loading é mais complexo (mesh format)

---

### Phase 09: INI Parser Fix & Hardening (2-3 dias)

**Dependências**: Phase 08

**Objetivo**: Fix blanket exception catching, enable config parsing

**Deliverables**:
- ✅ Remove `catch(...)` blanket blocks
- ✅ Implement specific exception handling
- ✅ Add field validation before parsing
- ✅ Test all INI types (float, string, int, etc)
- ✅ Verify game configs parse correctly

**Validação**:
- AudioSettings.ini parses floats correctly (not 0.00)
- Music.ini parses filenames correctly (not '')
- No "UNIVERSAL PROTECTION" errors in logs
- All 50+ audio tracks have valid paths

**Notas Críticas**:
- Referência: `references/jmarshall-win64-modern/` tem versão comprovada
- Must handle malformed INI gracefully (debug info but don't crash)
- Fix não é apenas para audio - afeta TODAS as configs do game

---

### Phase 10: Window System & Input (2-3 dias)

**Dependências**: Phase 09

**Objetivo**: SDL2 window, Vulkan surface, input handling

**Deliverables**:
- ✅ SDL2_CreateWindow (use existing code)
- ✅ VkSurfaceKHR creation (SDL2/Vulkan integration)
- ✅ Swapchain setup (extent, format, presentation mode)
- ✅ Input event loop (keyboard, mouse)
- ✅ Window resize handling

**Validação**:
- Window appears and can be resized
- Vulkan surface created successfully
- Keyboard input captured
- Mouse position tracking works

---

### Phase 11: Camera System (2 dias)

**Dependências**: Phase 10

**Objetivo**: View & projection matrices

**Deliverables**:
- ✅ Camera class (position, rotation, FOV)
- ✅ View matrix calculation
- ✅ Projection matrix (perspective + orthographic)
- ✅ Uniform buffer for matrices
- ✅ Camera update per frame

**Validação**:
- Orthographic projection works for menu (2D)
- Perspective projection works for game (3D)
- Camera movement smooth
- Matrix math correct (no flipped geometry)

---

### Phase 12: Lighting System (3 dias)

**Dependências**: Phase 11

**Objetivo**: Lighting calculations in shaders

**Deliverables**:
- ✅ Light uniform buffers (position, color, intensity)
- ✅ Lighting shader implementation (Phong basic)
- ✅ Ambient + directional + point lights
- ✅ Light updates per frame
- ✅ Normal map support (if needed for initial menu)

**Validação**:
- Lit geometry appears correctly
- Light position changes affect shading
- Ambient lighting provides baseline
- No black geometry

---

### Phase 13: Mesh Rendering (2-3 dias)

**Dependências**: Phase 12

**Objetivo**: Multi-mesh rendering system

**Deliverables**:
- ✅ Mesh data upload to buffers
- ✅ Vertex attribute binding setup
- ✅ Index buffer binding
- ✅ Multi-mesh rendering loop
- ✅ Mesh transformation (model matrix)

**Validation**:
- Load & render 10+ meshes
- Transformations work (scale, rotate, translate)
- Performance adequate (100+ meshes at 60 FPS)

---

### Phase 14: Texture & Material System (2-3 dias)

**Dependências**: Phase 13

**Objetivo**: Bind textures to geometry, material properties

**Deliverables**:
- ✅ Texture binding to shaders (descriptor sets)
- ✅ Material properties (color, shininess, metallic)
- ✅ Multi-texture support (diffuse, normal, specular)
- ✅ Texture filtering (linear, nearest)
- ✅ Mipmap handling

**Validation**:
- Textured geometry renders correctly
- Material properties affect appearance
- Different texture filters work
- No texture binding errors

---

### Phase 15: Game Object System (Entity Foundation) (3 dias)

**Dependências**: Phase 14

**Objetivo**: GameObject abstraction for rendering

**Deliverables**:
- ✅ GameObject base class
- ✅ Transform component
- ✅ Mesh component
- ✅ Material component
- ✅ Rendering dispatcher

**Validation**:
- GameObject creation/deletion works
- Transform changes affect rendering
- Multiple GameObjects render correctly
- No crashes with 100+ objects

---

### Phase 16: Menu INI Definitions Parser (3-4 dias)

**Dependências**: Phase 15

**Objetivo**: Parse menu definitions from WINDOW.big

**Deliverables**:
- ✅ MenuDefinition.ini parser
- ✅ ScreenDefinition parsing (layout, position)
- ✅ ScreenTransition.ini parsing
- ✅ WindowTransitions.ini parsing
- ✅ NamedGameWindows.ini parsing
- ✅ Error recovery for malformed INI

**Validação**:
- Parse main menu INI successfully
- All screens defined correctly
- No missing field errors
- Graceful handling of unknown fields

**Notas Críticas**:
- Menu INI localizado em WINDOW.big
- Dependências: Phase 09 (INI fix) ser completo
- Estrutura de tela: posição, tamanho, imagem de fundo

---

### Phase 17: Menu State Machine (3 dias)

**Dependências**: Phase 16

**Objetivo**: Menu navigation & state transitions

**Deliverables**:
- ✅ MainMenuScreen state
- ✅ SkirmishScreen state (opcional para MVP)
- ✅ State transition system
- ✅ Event dispatching per state
- ✅ Animation/transition timing

**Validação**:
- Menu screens transition correctly
- No memory leaks during transitions
- Back button returns to previous screen
- Quit button exits cleanly

---

### Phase 18: Menu UI Rendering Pipeline (3-4 dias)

**Dependências**: Phase 17

**Objetivo**: Render menu geometry from definitions

**Deliverables**:
- ✅ Orthographic projection setup (2D menu)
- ✅ UI mesh generation from INI
- ✅ Button geometry generation
- ✅ Text rendering setup
- ✅ Screen background image loading

**Validação**:
- Menu background renders correctly
- Button geometry matches INI definitions
- Text appears in correct positions
- No scaling/positioning bugs

---

### Phase 19: Button & Interactive Element System (3 dias)

**Dependências**: Phase 18

**Objetivo**: Button interaction & visual feedback

**Deliverables**:
- ✅ Button hitbox calculation
- ✅ Mouse over detection
- ✅ Button state machine (normal, hover, pressed, disabled)
- ✅ Visual feedback (highlight on hover)
- ✅ Click callbacks
- ✅ Keyboard navigation (tab/arrow keys)

**Validação**:
- Button hover detection works
- Click triggers callback
- Visual feedback on hover/press
- Keyboard navigation functional

---

### Phase 20: Audio System Integration (3-4 dias)

**Dependências**: Phase 19

**Objetivo**: Audio playback from buttons & menu

**Deliverables**:
- ✅ OpenAL initialization
- ✅ Audio clip loading from Music.big
- ✅ Background music playback
- ✅ Button click sound effects
- ✅ Volume control
- ✅ Audio event triggering

**Validação**:
- Menu music plays on startup
- Button click sound plays on click
- Volume control works
- No audio crackling/artifacts

**Notas Críticas**:
- Dependência crítica: Phase 09 (INI parser fix)
- Audio foi 100% implementado em Phase 33 mas bloqueado por INI bug
- Será rápido uma vez que Phase 09 terminar

---

### Phase 21: Main Menu Initial Screen Display (2-3 dias)

**Dependências**: Phase 20

**Objetivo**: Tie everything together - display working menu

**Deliverables**:
- ✅ Load menu INI (MenuDefinition.ini + refs)
- ✅ Create UI geometry from definitions
- ✅ Setup rendering pipeline for menu
- ✅ Load menu background image
- ✅ Load menu music
- ✅ Create button geometries & text
- ✅ Setup input event handlers
- ✅ Display main menu screen ready for interaction

**Validação**:
- Menu appears on startup
- All buttons visible and clickable
- Menu music plays
- Button clicks produce feedback (sound + visual)
- Clean shutdown

---

## 4. SUMMARY: DEPENDENCY CHAIN

```
PHASE 00.5 (BUILD) ✅
   ↓
PHASE 01-07 (VULKAN CORE)           [2-3 weeks]
   ├─ 01: Instance/Device setup
   ├─ 02: Command buffers
   ├─ 03: Shaders (SPIR-V)
   ├─ 04: Vertex/Index buffers
   ├─ 05: Textures & samplers
   ├─ 06: Pipeline state
   └─ 07: Drawing & presentation
   ↓
PHASE 08 (ASSET LOADING)             [3-4 days]
   ├─ VFS integration
   ├─ DirectX → Vulkan conversion
   └─ Mesh/texture loading
   ↓
PHASE 09 (INI FIX - CRITICAL!)       [2-3 days]
   ├─ Remove exception swallowing
   ├─ Field validation
   └─ Audio/config unblocking
   ↓
PHASE 10-15 (RENDERING SYSTEMS)      [2-3 weeks]
   ├─ 10: Window & input
   ├─ 11: Camera system
   ├─ 12: Lighting
   ├─ 13: Mesh rendering
   ├─ 14: Materials & textures
   └─ 15: GameObject entity system
   ↓
PHASE 16-20 (MENU SYSTEM)            [2-3 weeks]
   ├─ 16: Menu INI parser
   ├─ 17: State machine
   ├─ 18: UI rendering
   ├─ 19: Button interaction
   └─ 20: Audio integration
   ↓
PHASE 21 (MAIN MENU MVP)             [2-3 days]
   └─ Tie everything together
   ↓
🎉 TELA INICIAL FUNCIONAL!
```

---

## 5. TOTAL TIMELINE ESTIMATE

| Group | Phases | Duration | Dependencies |
|-------|--------|----------|--------------|
| Build | 00.5 | ✅ Complete | None |
| Vulkan Core | 01-07 | 2-3 weeks | 00.5 |
| Asset Loading | 08 | 3-4 days | 01-07 |
| INI Fix (CRITICAL) | 09 | 2-3 days | 08 |
| Rendering | 10-15 | 2-3 weeks | 08, 09 |
| Menu System | 16-20 | 2-3 weeks | 10-15 |
| Final Integration | 21 | 2-3 days | 16-20 |
| **TOTAL** | **01-21** | **8-11 weeks** | Sequential |

**Breakdown**:
- Vulkan Core: 40% of effort (most complex)
- Rendering Systems: 30% of effort (many components)
- Menu System: 20% of effort (INI integration critical)
- Final Integration: 10% of effort (tying things together)

---

## 6. CRITICAL SUCCESS FACTORS

### 6.1 INI Parser Fix (Phase 09) é BLOQUEADOR

**Why Critical**:
- Audio system 100% done but untestable (Phase 33 audioManager)
- Menu definitions REQUIRE working INI parser
- Audio music playback depends on Music.ini parsing
- Cannot proceed with menu without this

**Timeline Impact**:
- If Phase 09 takes 1 week: Menu development blocked
- If Phase 09 takes 3 days: Menu development on schedule

**Risk**: 🔴 HIGH - Must be fixed early, cannot be deferred

### 6.2 Vulkan Foundation (Phases 01-07) é Foundation

**Why Critical**:
- All subsequent graphics depends on this
- If architecture wrong, major rework needed
- Must be performant (60 FPS target)

**Risk**: 🟡 MEDIUM - Clear path but implementation heavy

### 6.3 Asset Loading Integration (Phase 08)

**Why Critical**:
- Must verify VFS still working
- DirectX → Vulkan conversion must be efficient
- Must NOT break existing texture loading

**Risk**: 🟢 LOW - VFS already proven, Phase 28.4 de-risked this

---

## 7. NEXT IMMEDIATE ACTIONS

1. **Approve este plano** (Phase numbering & dependencies)
2. **Criar docs/PHASE01/README.md** - Vulkan Instance setup
3. **Criar docs/PHASE02-05/README.md** - Graphics core phases
4. **Actualizar PHASE_INDEX.md** com novas 21 fases
5. **Começar Phase 01 implementation**

---

## ANEXO A: Lições Críticas Consolidadas

### Problema: Exception Handling
- ❌ Nunca use `catch(...)` sem re-throw
- ✅ Sempre adicione contexto (arquivo, linha, campo)
- ✅ Se não pode lidar, re-lance com mais info

### Problema: VFS/Asset Loading
- ❌ Nunca assuma assets são loose files
- ✅ Sempre verifique se VFS está envolvido
- ✅ Intercept DEPOIS que VFS carregou

### Problema: ARC Management (macOS)
- ❌ Nunca store local ARC objects em globals
- ✅ Deixe ARC gerenciar ciclo de vida
- ✅ Se precisa armazenar, use `[obj retain]`

### Problema: Build Caching
- ❌ Nunca ignore stale CMake cache
- ✅ Sempre delete `build/` se configure falha
- ✅ ccache é CRÍTICO (20min → 30sec)

### Problema: Testing Methodology
- ❌ Nunca pipe output durante testes (logs viram ilegíveis)
- ✅ Sempre use `tee` para salvar logs
- ✅ Grep DEPOIS da execução completar

---

**Status**: ✅ ROADMAP COMPLETO  
**Próximo**: Criar Phase 01-21 README files com detalhes de implementação

