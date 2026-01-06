# Phase 28: Texture System Implementation (MVP Foundation)

**Status**: In Progress (Started: October 9, 2025)
**Goal**: Implement complete texture loading and rendering pipeline for OpenGL backend to enable playable game MVP

## Overview

Phase 28 delivers the **texture system** - the most critical missing piece for a Minimum Viable Product (MVP). Without textures, the UI is unreadable and gameplay is impractical. This phase enables:

- **UI Rendering**: Menus, buttons, cursors, icons with proper alpha blending
- **Font Rendering**: Text in HUD, tooltips, status displays
- **3D Models**: Terrain, units, buildings with textured materials
- **Particle Effects**: Explosions, smoke, weather effects

## Architecture

### 1. Texture Loading Pipeline

```
Asset Files (DDS/TGA) → Parser → Decoder → OpenGL Upload → Cache → Binding
     ↓                    ↓         ↓           ↓             ↓         ↓
  .big archives      Header     Pixel      glTexImage2D   Lookup   SetTexture
                    validation   data      + mipmaps      by name   (stage N)
```

### 2. File Format Support

| Format | Variants | Priority | Use Cases |
|--------|----------|----------|-----------|
| **DDS** | DXT1 (BC1), DXT3 (BC2), DXT5 (BC3) | **HIGH** | UI, models, terrain (compressed) |
| **TGA** | 24-bit RGB, 32-bit RGBA, RLE | **HIGH** | Fonts, uncompressed UI elements |
| BMP | 24-bit RGB | MEDIUM | Legacy fallback |
| PNG | RGBA8 | LOW | Future enhancement |

### 3. OpenGL Integration

**DirectX 8 → OpenGL 3.3 Mapping**:
- `IDirect3DTexture8::CreateTexture()` → `glGenTextures()` + `glTexImage2D()`
- `SetTexture(stage, texture)` → `glActiveTexture(GL_TEXTURE0 + stage)` + `glBindTexture()`
- `SetTextureStageState()` → Filtering/wrapping via `glTexParameteri()`
- Mipmaps: `D3DX_DEFAULT` → `glGenerateMipmap()` or manual upload

### 4. Cache Strategy

```cpp
class TextureCache {
    std::unordered_map<std::string, TextureEntry> m_cache; // Lookup by normalized path

    struct TextureEntry {
        GLuint gl_texture_id;
        int width, height;
        int ref_count;
        GLenum format; // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, etc.
    };
};
```

**Lifecycle**:
1. Request texture by name (case-insensitive, normalized path)
2. Check cache → return existing GLuint if found
3. Load from .BIG archive → parse → decode → upload
4. Store in cache with refcount = 1
5. On release: decrement refcount → delete if 0

## Implementation Plan

### Phase 28.1: DDS Loader (Days 1-2)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dds_loader.cpp/h`

- [ ] Parse DDS header (magic, dimensions, format, mipmap count)
- [ ] Validate BC1/BC2/BC3 formats (4x4 blocks, compressed data size)
- [ ] Extract pixel data for each mip level
- [ ] Return structured data: `{width, height, format, levels[], data[]}`
- [ ] Error handling: unsupported formats, corrupt headers

**Test**: Load `Data/English/Art/Textures/GUIButtons.dds` → validate header

### Phase 28.2: TGA Loader (Days 2-3)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/tga_loader.cpp/h`

- [ ] Parse TGA header (type, dimensions, bits per pixel)
- [ ] Decompress RLE (type 10: run-length encoded RGBA)
- [ ] Convert BGR → RGB, add alpha channel if 24-bit
- [ ] Return RGBA8 uncompressed data
- [ ] Handle footer/version 2.0 (if present)

**Test**: Load font atlas `Data/English/Fonts/Arial12Bold.tga` → validate pixels

### Phase 28.3: OpenGL Upload & Mipmaps (Days 3-4)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` (existing)

```cpp
#ifndef _WIN32
GLuint Create_GL_Texture_From_DDS(const DDSData& dds) {
    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // Upload compressed mips
    for (int i = 0; i < dds.num_mipmaps; i++) {
        glCompressedTexImage2D(GL_TEXTURE_2D, i,
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, // or DXT1/DXT3
            dds.mip_widths[i], dds.mip_heights[i], 0,
            dds.mip_sizes[i], dds.mip_data[i]);
    }

    // Filtering & wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

    return tex_id;
}
#endif
```

**Test**: Upload DDS → verify `glGetTexLevelParameteriv()` reports correct dimensions

### Phase 28.4: Texture Cache (Day 4)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture_cache.cpp/h`

- [ ] Hash map: normalized path → `{GLuint, width, height, refcount}`
- [ ] `GetTexture(name)` → load if missing, increment refcount
- [ ] `ReleaseTexture(name)` → decrement, delete if zero
- [ ] Case-insensitive lookup (macOS compatibility)
- [ ] Clear cache on level transition (optional memory management)

**Test**: Load same texture twice → verify single GLuint, refcount = 2

### Phase 28.5: DX8 Wrapper Integration (Days 5-6)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

**Changes**:
```cpp
// In DX8Wrapper::Set_Texture()
#ifndef _WIN32
    if (stage < MAX_TEXTURE_STAGES && texture) {
        const char* texture_name = texture->Get_Name(); // Assume method exists
        GLuint gl_tex = g_TextureCache.GetTexture(texture_name);

        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, gl_tex);

        GL_Current_Texture_Stages[stage] = gl_tex;
    }
#endif
```

**Test**: Set UI texture → verify `glGetIntegerv(GL_TEXTURE_BINDING_2D)` returns correct ID

### Phase 28.6: Font Atlas Support (Day 6)
**Files**: `GeneralsMD/Code/GameEngine/Source/GameClient/GameFont.cpp`

- [ ] Load font atlas TGA (e.g., `Arial12Bold.tga`)
- [ ] Parse font metrics (glyph widths, positions in atlas)
- [ ] Update `Blit_Char()` to use GL texture coordinates
- [ ] Render strings via quad batches with atlas sampling

**Test**: Display "Command & Conquer" in menu → verify crisp text

### Phase 28.7: UI Rendering Test (Day 7)
**Integration Test**: Main menu rendering

- [ ] Load background texture (`MainMenuBackground.dds`)
- [ ] Load button textures (`GUIButtons.dds`)
- [ ] Load cursor texture (`Cursor.tga`)
- [ ] Verify alpha blending (semi-transparent overlays)
- [ ] Check mipmap quality (no aliasing when zoomed)

**Success Criteria**:
- Menu displays with correct colors and transparency
- No crashes during texture load/bind
- FPS > 30 with all UI textures loaded

### Phase 28.8: Skirmish Test (Day 8)
**Gameplay Test**: Start skirmish match

- [ ] Load terrain textures (`Desert.dds`, `Grass.dds`)
- [ ] Load unit textures (`Tank.dds`, `Infantry.dds`)
- [ ] Move camera → verify LOD mipmaps
- [ ] Select units → verify HUD icons
- [ ] Run for 10+ minutes → monitor memory/performance

**Success Criteria**:
- No crashes for 10+ minutes
- All textures visible (no pink/magenta placeholders)
- FPS > 30 in battle scenarios
- Memory usage stable (no leaks)

## File Structure

```
docs/WORKDIR/28/PHASE28/
├── README.md (this file)
├── TODO_LIST.md (detailed task tracking)
├── DDS_FORMAT_SPEC.md (DDS header layout, BC1/BC2/BC3 details)
├── TGA_FORMAT_SPEC.md (TGA header, RLE algorithm)
├── OPENGL_TEXTURE_API.md (glTexImage2D, compressed formats, extensions)
└── TESTING.md (test cases, validation procedures)

GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/
├── dds_loader.cpp/h (new)
├── tga_loader.cpp/h (new)
├── texture_cache.cpp/h (new)
├── dx8wrapper.cpp/h (update)
└── texture.cpp/h (update - TextureClass integration)
```

## Dependencies

- **OpenGL Extensions**: `GL_EXT_texture_compression_s3tc` (DXT1/3/5)
- **GLAD**: Already integrated (Phase 27.1)
- **File I/O**: Existing `FileClass` for reading from .BIG archives
- **Math**: Existing `Vector2` for UV coordinates

## Success Metrics

| Metric | Target | Status |
|--------|--------|--------|
| DDS load time | < 50ms for 2048x2048 | Not Tested |
| TGA load time | < 30ms for 1024x1024 | Not Tested |
| Cache hit rate | > 95% during gameplay | Not Tested |
| Memory overhead | < 200MB for full UI | Not Tested |
| Menu render FPS | > 60 FPS | Not Tested |
| Game render FPS | > 30 FPS | Not Tested |

## Risks & Mitigations

### Risk 1: Unsupported DDS Formats
**Impact**: High (blocks UI rendering)
**Mitigation**:
- Implement BC1/BC2/BC3 (covers 95% of assets)
- Log warnings for unsupported formats
- Fallback to solid color or pink texture

### Risk 2: Case-Sensitive Paths (macOS)
**Impact**: Medium (texture not found)
**Mitigation**:
- Normalize all paths to lowercase in cache
- Use existing `FileSystem::Open()` with case-insensitive search

### Risk 3: Memory Leaks
**Impact**: Medium (degrades performance over time)
**Mitigation**:
- Refcount all textures
- Automated tests: load 1000 textures → release → verify 0 leaks

### Risk 4: Mipmap Quality
**Impact**: Low (visual artifacts)
**Mitigation**:
- Use pre-generated mipmaps from DDS
- If missing, `glGenerateMipmap()` with trilinear filter

## Timeline

| Phase | Days | Deliverable |
|-------|------|-------------|
| 28.1 | 1-2 | DDS loader with BC1/BC2/BC3 |
| 28.2 | 2-3 | TGA loader with RLE |
| 28.3 | 3-4 | OpenGL upload + mipmaps |
| 28.4 | 4 | Texture cache |
| 28.5 | 5-6 | DX8 wrapper integration |
| 28.6 | 6 | Font atlas support |
| 28.7 | 7 | UI rendering test |
| 28.8 | 8 | Skirmish test |

**Total Estimated Time**: 8 days (aggressive) / 12 days (realistic)

## Next Steps

1. **Immediate**: Create `DDS_FORMAT_SPEC.md` with header layout
2. **Day 1**: Implement DDS parser (header validation only)
3. **Day 1**: Write unit test for DDS header parsing
4. **Day 2**: Add BC1/BC2/BC3 decompression (or use GL native)
5. **Day 2**: Test with real asset from `Data/English/Art/Textures/`

## References

- [DDS File Format (Microsoft)](https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds)
- [OpenGL S3TC Extension](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_compression_s3tc.txt)
- [TGA Format Specification](http://www.paulbourke.net/dataformats/tga/)
- Phase 27 Documentation: `../Misc/OPENGL_SUMMARY.md`
