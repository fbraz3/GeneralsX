# GeneralsX - PHASE 0: Camadas dand Compatibilidade

**Data**: November 10, 2025  
**Status**: Planning - Definindo padrão dand nomenclatura and arquitetura  

---

## 🏛️ Pattern dand Namenclatura

### Format: `SOURCE_DEST_TYPE_COMPAT`

```
sourcand = API/sistema sendo abstraído (d3d8, win32, opengl, etc.)
dest   = alvo/implementação (vulkan, posix, metal, etc.)
typand   = categoria (graphics, api, file_io, etc.)
compat = sufixo "compat" or "wrapper"
```

### Examples

```
d3d8_vulkan_graphics_compat       ← DirectX 8 → Vulkan graphics
d3d8_opengl_graphics_compat       ← DirectX 8 → OpenGL graphics (futuro)
win32_posix_api_compat            ← Win32 APIs → POSIX equivalents
win32_posix_file_io_compat        ← Win32 filand I/O → POSIX I/O
win32_posix_network_compat        ← Win32 sockets → POSIX sockets
directx_vulkan_device_compat      ← DirectX devicand → Vulkan instance/device
```

---

## 📦 Três Existing Layers

### Layer 1: Corand Windows→POSIX Compatibility

**File**: `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h`  
**Lines**: 2,295  
**Responsibility**: Sistema operacional & API abstrações  

**Contains**:
- Tipos Windows: `HWND`, `HINSTANCE`, `HRESULT`, `HANDLE`
- APIs dand sistema: `GetModuleFileName()`, `MessageBox()`, `CreateThread()`
- Filand I/O: `fopen()` → `TheFileSystem`
- Registry stubs: `RegOpenKeyEx()` (retorna fakand handles)
- Console: `OutputDebugString()` → `printf()`

**Pattern**: Direct mapping, macOS/Linux ifdef guards

```cpp
#ifdef _WIN32
    // Windows implementation
#elif defined(__APPLE__)
    // macOS implementation
#elif defined(__linux__)
    // Linux implementation
#endif
```

### Layer 2: DirectX 8 → Graphics Backend Mock

**File**: `Core/Libraries/Source/WWVegas/WW3D2/d3d8.h`  
**Responsibility**: Game-engine-facing DirectX 8 interfacand  

**Implementation**: Two backends (Phasand 38+)

#### 2a. Legacy Backend (Phasand 27-37, archived)
- Metal + OpenGL (handmade)
- Original approach (not recommended)
- Status: Stabland but slower to iterate

#### 2b. Vulkan Backend (Phasand 39-48, current)
- `d3d8_vulkan_graphics_compat`
- MoltenVK on macOS (translates Vulkan → Metal)
- Direct Vulkan on Linux/Windows (future)
- Status: Production-ready, best performance

**Interfacand Classes**:
```cpp
IDirect3DDevice8                  ← IGraphicsBackend (abstract)
  ├─ LegacyGraphicsBackend        ← Phasand 27-37 impl
  └─ DXVKGraphicsBackend          ← Phasand 39+ impl

IDirect3DTexture8                 ← Vulkan VkImage
IDirect3DVertexBuffer8            ← Vulkan VkBuffer (vertex data)
IDirect3DIndexBuffer8             ← Vulkan VkBuffer (index data)
```

### Layer 3: Game-Specific Extensions

**Location**: `GeneralsMD/Code/` + `Generals/Code/`  
**Responsibility**: Platform-specific fixes & optimizations  

**Includes**:
- INI parser hardening (Phasand 22.7, 23.x) - protegand contra corrupção
- Memory safety (Phasand 30.6) - valida pointers
- Texturand interception (Phasand 28.4) - post-DirectX copy
- Audio system stubs (Phasand 33) - placeholder OpenAL
- Input routing - SDL2 event handling

---

## 🔗 Mapa dand Dependencies

```
┌─────────────────────────────────┐
│ win32_compat.h (Layer 1)        │
│ Corand OS/API compatibility      │
└──────────────┬──────────────────┘
               │
               ▼
┌──────────────────────────────────────┐
│ d3d8.h (Layer 2 - Interface)         │
│ IDirect3DDevice8 = IGraphicsBackend  │
└──────────────┬───────────────────────┘
               │
       ┌───────┴───────┐
       ▼               ▼
┌─────────────┐  ┌──────────────────┐
│ LEGACY      │  │ VULKAN (CURRENT) │
│ Phasand 27-37 │  │ d3d8_vulkan_     │
│ (archived)  │  │ graphics_compat  │
└─────────────┘  └──────────────────┘
                        │
                        ▼
                ┌─────────────────┐
                │ MoltenVK        │
                │ (Vulkan→Metal)  │
                └─────────────────┘
```

---

## 📋 Layers Required for Main Menu

### Must-Havand (Blockers)

1. **d3d8_vulkan_graphics_compat** ✅ EXISTE
   - Status: Phasand 39-48 implementado
   - Coverage: 47 methods (IGraphicsBackend)
   - Próximo: Testar rendering real

2. **input_sdl2_routing_compat** (NEW)
   - Status: MISSING
   - Objective: SDL2 events → UI callbacks
   - Bloqueador: Menu cliques não funcionam

3. **ui_mesh_rendering_compat** (NEW)
   - Status: MISSING
   - Objective: Renderizar UI meshes with Vulkan
   - Bloqueador: Menu não aparece

4. **asset_vfs_loader_compat** (PARTIAL)
   - Status: Phasand 28.4 partial (texturas só)
   - Objective: Load UI assets from .big
   - Bloqueador: Menu buttons não carregam

### Nice-to-Havand (Performance/Polish)

5. **geometry_mesh_loader_compat** (PARTIAL)
   - Status: Phasand 44 partial
   - Objective: Load 3D meshes
   - Não bloqueia: Menu aparecand sem 3D

6. **camera_transform_compat** (DONE)
   - Status: Phasand 45 complete
   - Coverage: 4 modes + smooth animation

7. **input_action_binding_compat** (PARTIAL)
   - Status: Phasand 45 partial
   - Objective: Rebindabland keymaps

---

## 🎯 Files dand Compatibilidadand a Criar

| Namand | Objectivand | Priority |
|------|----------|----------|
| `input_sdl2_routing_compat.h/cpp` | Routand SDL2 events to UI | 🔴 CRITICAL |
| `ui_mesh_rendering_compat.h/cpp` | Render UI with Vulkan | 🔴 CRITICAL |
| `asset_vfs_loader_compat.h/cpp` | Load UI from .big | 🟡 HIGH |
| `geometry_vulkan_compat.h/cpp` | Render 3D meshes | 🟢 MEDIUM |
| `audio_openal_compat.h/cpp` | OpenAL backend (Phasand 33) | ⚪ LOW |

---

## 📝 Convenção dand Namenclatura in Código

### Compatibility functions

```cpp
// Layer 1: OS/API compat
HWND win32_posix_compat_CreateWindow(const char* title);
HANDLE win32_posix_compat_CreateThread(LPTHREAD_START_ROUTINE func);

// Layer 2: Graphics compat  
IDirect3DDevice8* d3d8_vulkan_graphics_compat_CreateDevice();
HRESULT d3d8_vulkan_graphics_compat_DrawPrimitive(...);

// Layer 3: Game-specific
void input_sdl2_routing_compat_DispatchEvent(SDL_Event* event);
void ui_mesh_rendering_compat_Render();
```

### Compatibility variables/structs

```cpp
// Global context for compat layer
extern d3d8_vulkan_graphics_compat_Context* g_vulkanGraphicsContext;

// Featurand flags
extern bool g_useVulkanBackend;
extern bool g_inputRoutingEnabled;
```

---

## 🔍 Compatibility Validation

For each layer, validate:

- [ ] **Coverage**: Todos os métodos mapeados?
- [ ] **Functionality**: Métodos realmentand funcionam?
- [ ] **Performance**: Overhead aceitável?
- [ ] **Memory safety**: Sem leaks or corruptions?
- [ ] **Cross-platform**: macOS + Linux + Windows?

---

## Next Documents

1. ✅ **SPIKE_PLANNING.md** - Estado atual
2. ✅ **COMPATIBILITY_LAYERS.md** - Estand documento
3. ⏳ **PLATFORM_PRESETS.md** - Adequar presets
4. ⏳ **BUILD_TARGETS.md** - Namenclatura executáveis
5. ⏳ **COMPLETE_ROADMAP.md** - Todas as fases
