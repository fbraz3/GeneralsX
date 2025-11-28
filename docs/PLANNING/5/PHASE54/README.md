# Phase 54: Display and Rendering Initialization

**Phase**: 54
**Title**: Display Pipeline and Rendering Systems
**Duration**: 5-7 days
**Status**: IN PROGRESS - BLOCKED ON TEXTURE LOADING
**Dependencies**: Phase 53 complete (Objects/Templates working)

---

## Overview

Phase 54 handles display and rendering initialization. This is where Metal/Vulkan/OpenGL backends are fully tested with actual game content.

### Current Status: PARTIALLY COMPLETE

**Working:**

- Vulkan instance, device, swapchain, render pass, framebuffers
- W3DDisplay basic initialization
- WindowManager, InGameUI, Shell

**Blocked:**

- TextureLoader hangs during `W3DBibBuffer` initialization
- Specifically: `NEW_REF(TextureClass, ("TBBib.tga"))` never returns

---

## Subsystems Sequence

| # | Subsystem | Purpose | Backend |
|---|-----------|---------|---------|
| 37 | TheDisplay (via GameClient) | Main display manager | Metal/Vulkan |
| 38 | TheInGameUI | In-game user interface | Metal/Vulkan |
| 39 | W3DDisplay internal init | W3D rendering pipeline | Metal/Vulkan |
| 40 | Shader compilation | Game shaders | Metal/Vulkan |
| 41 | Texture loading | Game textures from .big | Metal/Vulkan |

---

## Critical Components

### W3DDisplay

The core rendering class that interfaces with:

- Metal (macOS)
- Vulkan (cross-platform, current unified backend)
- OpenGL (legacy fallback)

Key methods:

```cpp
W3DDisplay::Init()          // Initialize rendering
W3DDisplay::Reset()         // Reset display state  
W3DDisplay::Draw()          // Main render loop
W3DDisplay::SetResolution() // Resolution changes
```

### Texture System

Phase 28.4 established texture interception:

1. Game loads texture from .big archive
2. DirectX mock receives texture data
3. MetalWrapper/VulkanWrapper uploads to GPU

Test with:

```bash
grep -i "texture\|metal\|vulkan" logs/phase54_render.log
```

### Shader System

Shaders need to be translated or replaced:

- Original: DirectX HLSL
- macOS: Metal Shading Language
- Cross-platform: SPIR-V (Vulkan)

---

## High-Risk Areas

### Resolution and Fullscreen

Fullscreen on macOS requires:

- Proper window hints
- Screen mode switching
- Resolution enumeration

### Multi-Monitor Support

SDL2 handles monitors, but game may assume single display.

### HDR/Color Space

Metal supports HDR, but game may expect SDR.

---

## Tasks

### Task 54.1: Display Initialization

Verify W3DDisplay initializes with correct backend.

### Task 54.2: Windowed Mode Test

Test in windowed mode first (easier debugging):

```bash
# Set resolution in GeneralsXZH.ini
Resolution = 1280 720
Windowed = yes
```

### Task 54.3: Texture Loading Verification

Confirm textures load from .big archives correctly.

### Task 54.4: Shader Compilation

Verify shader compilation/translation works.

### Task 54.5: First Frame Render

Get first game frame to render (even if garbled).

### Task 54.6: Fullscreen Mode

Test fullscreen mode after windowed works.

---

## Debugging Commands

```bash
# Run with rendering debug output
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/phase54_render.log

# Check Metal/Vulkan initialization
grep -i "metal\|vulkan\|device\|adapter" logs/phase54_render.log

# Check texture loading
grep -i "texture\|surface\|format" logs/phase54_render.log
```

---

## Success Criteria

- [x] W3DDisplay initializes with Vulkan backend
- [x] Window appears (SDL2 window created)
- [ ] **BLOCKED**: Textures load from .big archives - hangs on first texture
- [ ] Shaders compile successfully
- [ ] First frame renders
- [ ] Fullscreen mode works
- [ ] Game progresses to game logic initialization

---

## Current Blocker: Texture Loading Hang

**Location**: `W3DBibBuffer::W3DBibBuffer()` constructor

**Code Path**:

```cpp
// W3DBibBuffer.cpp:236-237
m_bibTexture = NEW_REF(TextureClass, ("TBBib.tga"));  // <-- HANGS HERE
fprintf(stderr, "[W3DBibBuffer] m_bibTexture created: %p\n", m_bibTexture);  // Never reached
```

**Root Cause Analysis**:

1. `TextureClass` constructor calls `TextureLoader::Request_High_Priority_Loading()`
2. This calls `TextureLoadTaskClass::Begin_Texture_Load()`
3. Likely hanging in:
   - `DX8Wrapper::_Create_DX8_Texture()` - Vulkan texture creation
   - OR file loading from .big archive
   - OR threading issue in texture loader

**Files to Investigate**:

- `Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp` - Begin_Texture_Load()
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - _Create_DX8_Texture()
- `Core/Libraries/Source/WWVegas/WW3D2/VulkanGraphicsDriver.cpp` - CreateTexture()

**Log Evidence**:

```log
[GameClient::init] Creating TerrainVisual
[W3DBibBuffer] Constructor called
[W3DBibBuffer] About to create m_bibTexture
# === HANGS HERE - no further output ===
```

---

**Created**: November 2025
**Last Updated**: November 28, 2025
