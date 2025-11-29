# Phase 60: UI Rendering and Main Menu

**Phase**: 60
**Title**: User Interface Rendering and Main Menu Display
**Duration**: 5-7 days
**Status**: IN PROGRESS
**Dependencies**: Phase 59 complete (Render states working)

---

## Overview

Phase 60 focuses on getting the game's user interface to render correctly, culminating in a fully functional main menu. This is the first "visual milestone" where the game looks like a real game.

### Current State (Post-Phase 59)

- ✅ 3D geometry renders with textures
- ✅ Transforms and blend modes work
- ✅ Dynamic vertex/index buffer support (DrawIndexedPrimitiveUP)
- ✅ UI pipeline created for RHW vertices
- ⏳ UI elements rendering in progress
- ⏳ Main menu appearing (needs verification)
- ⏳ Text rendering not verified

### Goal

Display the main menu with all buttons, text, and UI elements functional.

---

## Implementation Progress (November 29, 2025)

### Completed

1. **UI Pipeline with RHW Vertex Support**
   - Created `CreateUIPipeline()` with separate shaders
   - UI vertex shader handles screen-space coordinates
   - Push constants: screenWidth, screenHeight, useTexture, alphaThreshold
   - Automatically selected when FVF has D3DFVF_XYZRHW flag

2. **Dynamic Buffer Support**
   - `DynamicVBAccessClass` now allocates real vertex memory
   - `DynamicIBAccessClass` now allocates real index memory
   - `Set_Vertex_Buffer(DynamicVBAccessClass&)` stores data pointer
   - `Set_Index_Buffer(DynamicIBAccessClass&)` stores data pointer

3. **DrawIndexedPrimitiveUP Integration**
   - `Draw_Triangles` detects dynamic buffers
   - Routes to `DrawIndexedPrimitiveUP` for immediate rendering
   - Creates temporary Vulkan buffers per-frame
   - Automatic cleanup in next frame's BeginFrame

4. **Testing Results**
   - Game runs 30+ seconds without crashes
   - `DrawIndexedPrimitiveUP` called with stride=44 (VertexFormatXYZNDUV2)
   - Multiple frames presented successfully

### Key Discovery

The game's `Render2DClass` uses **matrix-transformed XYZ vertices** (stride 44), not pre-transformed RHW vertices (stride 28). This means:
- The standard 3D pipeline handles UI rendering
- Orthographic projection matrix transforms screen coordinates
- RHW pipeline would be used for different rendering paths

---

## UI System Overview

### W3D UI Architecture

The game uses a custom UI system built on W3D:

```
GameWindow (root)
├── ShellMenuScheme (main menu background)
├── Button widgets
├── ListBox widgets
├── TextEntry widgets
├── Static image widgets
└── Animation frames
```

### UI Rendering Path

```
1. GameWindowManager processes windows
2. WindowVideoManager handles video backgrounds
3. W3DDisplay renders UI primitives
4. Direct3D draws quads with textures
```

---

## Technical Requirements

### 1. 2D Orthographic Projection

UI uses screen-space coordinates:

```cpp
void SetupOrthoProjection(float width, float height)
{
    D3DXMATRIX ortho;
    D3DXMatrixOrthoOffCenterLH(&ortho, 0, width, height, 0, 0, 1);
    SetTransform(D3DTS_PROJECTION, &ortho);
    
    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    SetTransform(D3DTS_WORLD, &identity);
    SetTransform(D3DTS_VIEW, &identity);
}
```

### 2. Pre-Transformed Vertices (RHW)

UI often uses pre-transformed vertices:

```cpp
// FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1
struct UIVertex {
    float x, y, z, rhw;  // Screen position
    DWORD color;         // Vertex color
    float u, v;          // Texture coord
};
```

For RHW vertices, bypass MVP transform:

```glsl
// In vertex shader
if (is_pretransformed) {
    gl_Position = vec4(inPosition.xy / screenSize * 2.0 - 1.0, 0.0, 1.0);
    gl_Position.y = -gl_Position.y;  // Flip Y
}
```

### 3. Text Rendering

Game uses bitmap fonts:

```
Font texture: Contains all glyphs in grid
Character info: UV coordinates, width, advance
```

Text rendering = Textured quads with font texture.

---

## Implementation Plan

### Week 1: UI Infrastructure

#### Day 1-2: RHW Vertex Support

```cpp
// Detect RHW vertices
bool IsPreTransformed(uint32_t fvf)
{
    return (fvf & D3DFVF_XYZRHW) != 0;
}

// Shader variant for RHW
// Pass screen dimensions as uniform
```

#### Day 3: Vertex Color Support

Many UI elements use vertex colors:

```glsl
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;  // D3DFVF_DIFFUSE
layout(location = 2) in vec2 inTexCoord;

out vec4 fragColor;

void main() {
    fragColor = inColor;
}
```

Fragment shader:

```glsl
in vec4 fragColor;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    outColor = texColor * fragColor;  // Modulate by vertex color
}
```

#### Day 4-5: UI Texture Management

UI textures are often loaded dynamically:

- Button states (normal, hover, pressed)
- Menu backgrounds
- Icons and indicators

Ensure texture loading handles all UI formats.

### Week 2: Main Menu

#### Day 6: Shell Map Background

The main menu has a 3D background:

```cpp
// Shell map is a simplified 3D scene
// Loads: Maps/Shell/Shell.map (or similar)
```

Verify terrain and sky render correctly.

#### Day 7: Menu Buttons

Verify button textures load and display:

- Single Player
- Multiplayer
- Options
- Exit

Test hover states change appearance.

---

## Text Rendering Details

### Bitmap Font Format

```cpp
struct FontGlyph {
    float u1, v1, u2, v2;  // UV rect in font texture
    float width;           // Glyph width
    float advance;         // Spacing to next glyph
};

struct BitmapFont {
    uint32_t texture_handle;
    float line_height;
    std::map<char, FontGlyph> glyphs;
};
```

### Text Draw Function

```cpp
void DrawText(const char* text, float x, float y, BitmapFont& font)
{
    SetTexture(0, font.texture_handle);
    
    float cursor_x = x;
    for (const char* c = text; *c; ++c) {
        FontGlyph& g = font.glyphs[*c];
        
        // Create quad for glyph
        DrawQuad(cursor_x, y, g.width, font.line_height, g.u1, g.v1, g.u2, g.v2);
        
        cursor_x += g.advance;
    }
}
```

---

## Testing Strategy

### Test 1: UI Quad

Draw single UI quad at known position.

### Test 2: Vertex Colors

Verify vertex color modulation works.

### Test 3: Menu Background

Verify shell map background renders.

### Test 4: Button Display

Verify menu buttons appear correctly.

### Test 5: Text Rendering

Verify menu text is readable.

### Test 6: Mouse Interaction

Verify button hover states work.

---

## Success Criteria

- [ ] RHW vertices render at correct screen position
- [ ] Vertex colors modulate texture
- [ ] Shell map background visible
- [ ] Menu buttons display correctly
- [ ] Button hover states work
- [ ] Menu text readable
- [ ] Menu navigation works
- [ ] Exit button closes game

---

## Files to Modify

1. `vulkan_graphics_driver.cpp` - RHW vertex handling
2. `shaders/ui.vert` - UI vertex shader
3. `shaders/ui.frag` - UI fragment shader with vertex color

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| RHW coordinate errors | High | Test with known values |
| Vertex color format | Medium | Check BGRA vs RGBA |
| Font texture loading | Medium | Debug font texture |
| Z-order issues | Medium | Ensure proper depth |

---

## Beyond Phase 60

After main menu works:

- **Phase 61**: In-game HUD rendering
- **Phase 62**: Minimap rendering
- **Phase 63**: Unit selection UI
- **Phase 64**: Build menus

---

**Created**: November 28, 2025
**Author**: GeneralsX Development Team
