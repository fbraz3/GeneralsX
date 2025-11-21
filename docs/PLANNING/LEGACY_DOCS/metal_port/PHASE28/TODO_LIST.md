# Phase 28 - Detailed Task List

**Last Updated**: October 9, 2025
**Status**: In Progress (0/9 phases complete)

## Progress Overview

```
Phase 28.1 DDS Loader           [░░░░░░░░░░] 0%
Phase 28.2 TGA Loader           [░░░░░░░░░░] 0%
Phase 28.3 OpenGL Upload        [░░░░░░░░░░] 0%
Phase 28.4 Texture Cache        [░░░░░░░░░░] 0%
Phase 28.5 DX8 Integration      [░░░░░░░░░░] 0%
Phase 28.6 Font Atlas           [░░░░░░░░░░] 0%
Phase 28.7 UI Test              [░░░░░░░░░░] 0%
Phase 28.8 Skirmish Test        [░░░░░░░░░░] 0%
──────────────────────────────────────────────
Overall                         [░░░░░░░░░░] 0%
```

---

## Phase 28.1: DDS Loader Implementation

**Goal**: Parse DDS files with BC1/BC2/BC3 support
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dds_loader.cpp/h`
**Estimated Time**: 1-2 days

### Tasks

- [ ] **28.1.1** Create `dds_loader.h` with structures
  - [ ] `DDSPixelFormat` struct (32 bytes)
  - [ ] `DDSHeader` struct (124 bytes)
  - [ ] `DDSData` struct (parsed output)
  - [ ] `LoadDDS(file_path)` function signature

- [ ] **28.1.2** Implement DDS header parser
  - [ ] Validate magic number `"DDS "` (0x20534444)
  - [ ] Parse dimensions (width, height, depth)
  - [ ] Parse mipmap count (`dwMipMapCount`)
  - [ ] Parse pixel format flags (`dwFlags`, `dwFourCC`)
  - [ ] Detect BC1/BC2/BC3 via FourCC (`DXT1`/`DXT3`/`DXT5`)

- [ ] **28.1.3** Calculate compressed data sizes
  - [ ] BC1: `max(1, ((width+3)/4)) * max(1, ((height+3)/4)) * 8` bytes per mip
  - [ ] BC2/BC3: `max(1, ((width+3)/4)) * max(1, ((height+3)/4)) * 16` bytes per mip
  - [ ] Validate total file size matches expected data

- [ ] **28.1.4** Extract pixel data for each mipmap level
  - [ ] Read compressed blocks from file
  - [ ] Store pointers to each mip level
  - [ ] Handle single-mip and multi-mip textures

- [ ] **28.1.5** Error handling
  - [ ] Return nullptr for invalid magic
  - [ ] Log unsupported formats (BC4/BC5/BC6H/BC7)
  - [ ] Handle missing mipmaps gracefully

- [ ] **28.1.6** Unit test
  - [ ] Load `Data/English/Art/Textures/GUIButtons.dds`
  - [ ] Verify width/height
  - [ ] Verify FourCC matches expected format
  - [ ] Print first 16 bytes of mip 0 data

**Dependencies**: File I/O (`FileClass`), logging system
**Blockers**: None
**Status**: Not Started

---

## Phase 28.2: TGA Loader Implementation

**Goal**: Parse TGA files (24/32-bit, RLE support)
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/tga_loader.cpp/h`
**Estimated Time**: 1 day

### Tasks

- [ ] **28.2.1** Create `tga_loader.h` with structures
  - [ ] `TGAHeader` struct (18 bytes)
  - [ ] `TGAData` struct (parsed RGBA8 output)
  - [ ] `LoadTGA(file_path)` function signature

- [ ] **28.2.2** Implement TGA header parser
  - [ ] Parse image type (2=uncompressed RGB, 10=RLE RGB)
  - [ ] Parse dimensions (width, height)
  - [ ] Parse bits per pixel (24 or 32)
  - [ ] Parse image descriptor (alpha bits, origin)

- [ ] **28.2.3** Decompress RLE (type 10)
  - [ ] Read packet header (1 byte)
  - [ ] If MSB=1: repeat next pixel N times (run-length)
  - [ ] If MSB=0: read N raw pixels
  - [ ] Loop until width*height pixels decoded

- [ ] **28.2.4** Convert BGR → RGBA8
  - [ ] Swap B and R channels
  - [ ] Add alpha=255 if 24-bit
  - [ ] Copy alpha if 32-bit

- [ ] **28.2.5** Handle vertical flip
  - [ ] Check origin bit in descriptor
  - [ ] Flip rows if bottom-left origin

- [ ] **28.2.6** Unit test
  - [ ] Load `Data/English/Fonts/Arial12Bold.tga`
  - [ ] Verify dimensions
  - [ ] Check first pixel RGBA values
  - [ ] Test RLE decompression with known file

**Dependencies**: File I/O (`FileClass`)
**Blockers**: None
**Status**: Not Started

---

## Phase 28.3: OpenGL Upload & Mipmaps

**Goal**: Upload textures to GPU with filtering/wrapping
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
**Estimated Time**: 1 day

### Tasks

- [ ] **28.3.1** Implement `Create_GL_Texture_From_DDS()`
  - [ ] `glGenTextures(1, &tex_id)`
  - [ ] `glBindTexture(GL_TEXTURE_2D, tex_id)`
  - [ ] Loop mipmaps: `glCompressedTexImage2D()`
  - [ ] Set filtering: `GL_LINEAR_MIPMAP_LINEAR` + `GL_LINEAR`
  - [ ] Set anisotropy: `GL_TEXTURE_MAX_ANISOTROPY_EXT = 16.0`
  - [ ] Return `tex_id`

- [ ] **28.3.2** Implement `Create_GL_Texture_From_TGA()`
  - [ ] `glGenTextures(1, &tex_id)`
  - [ ] `glBindTexture(GL_TEXTURE_2D, tex_id)`
  - [ ] `glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data)`
  - [ ] `glGenerateMipmap(GL_TEXTURE_2D)` if mipmaps enabled
  - [ ] Set filtering and wrapping
  - [ ] Return `tex_id`

- [ ] **28.3.3** Handle wrapping modes
  - [ ] Map D3D wrap modes → GL: `REPEAT`, `CLAMP_TO_EDGE`, `MIRRORED_REPEAT`
  - [ ] `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S/T, mode)`

- [ ] **28.3.4** Error checking
  - [ ] `glGetError()` after each GL call
  - [ ] Log format/dimension mismatches
  - [ ] Fallback to 1x1 pink texture on failure

- [ ] **28.3.5** Unit test
  - [ ] Upload DDS texture → verify `glIsTexture(tex_id) == GL_TRUE`
  - [ ] Query mip 0 dimensions: `glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH)`
  - [ ] Verify no GL errors

**Dependencies**: DDS/TGA loaders (28.1, 28.2), GLAD
**Blockers**: None
**Status**: Not Started

---

## Phase 28.4: Texture Cache

**Goal**: Manage texture lifetime and avoid duplicate loads
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture_cache.cpp/h`
**Estimated Time**: 0.5 days

### Tasks

- [ ] **28.4.1** Create `texture_cache.h` with `TextureCache` class
  - [ ] `std::unordered_map<std::string, TextureEntry> m_cache`
  - [ ] `TextureEntry`: `{GLuint id, int width, int height, int refcount, GLenum format}`
  - [ ] `GetTexture(const char* path)` → `GLuint`
  - [ ] `ReleaseTexture(const char* path)` → `void`
  - [ ] `ClearCache()` → delete all textures

- [ ] **28.4.2** Implement case-insensitive lookup
  - [ ] Normalize path to lowercase
  - [ ] Replace `\` with `/`
  - [ ] Remove `.big:` prefix if present

- [ ] **28.4.3** Implement `GetTexture()`
  - [ ] Normalize path
  - [ ] Check cache → return ID if found, increment refcount
  - [ ] If not found:
    - [ ] Detect extension (`.dds` or `.tga`)
    - [ ] Call appropriate loader
    - [ ] Upload to GL
    - [ ] Store in cache with refcount=1
  - [ ] Return GLuint

- [ ] **28.4.4** Implement `ReleaseTexture()`
  - [ ] Normalize path
  - [ ] Find in cache
  - [ ] Decrement refcount
  - [ ] If refcount==0: `glDeleteTextures()` + remove from map

- [ ] **28.4.5** Unit test
  - [ ] Load same texture twice → verify single GLuint
  - [ ] Verify refcount=2
  - [ ] Release once → refcount=1
  - [ ] Release twice → texture deleted

**Dependencies**: DDS/TGA loaders, OpenGL upload (28.3)
**Blockers**: None
**Status**: Not Started

---

## Phase 28.5: DX8 Wrapper Integration

**Goal**: Connect SetTexture() to OpenGL texture binding
**Files**: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp/h`
**Estimated Time**: 1 day

### Tasks

- [ ] **28.5.1** Add global texture cache instance
  - [ ] `extern TextureCache g_TextureCache;`
  - [ ] Initialize in `DX8Wrapper::Init()`

- [ ] **28.5.2** Update `DX8Wrapper::Set_Texture()`
  - [ ] Add OpenGL path:
    ```cpp
    #ifndef _WIN32
    if (stage < MAX_TEXTURE_STAGES && texture) {
        const char* name = texture->Get_Name();
        GLuint gl_tex = g_TextureCache.GetTexture(name);
        glActiveTexture(GL_TEXTURE0 + stage);
        glBindTexture(GL_TEXTURE_2D, gl_tex);
        GL_Current_Texture_Stages[stage] = gl_tex;
    }
    #endif
    ```

- [ ] **28.5.3** Update `TextureClass::Apply()`
  - [ ] Call `Set_Texture(m_stage, this)`
  - [ ] Apply texture stage states (filtering, wrapping)

- [ ] **28.5.4** Handle texture release
  - [ ] On `TextureClass` destructor: `g_TextureCache.ReleaseTexture(m_name)`

- [ ] **28.5.5** Integration test
  - [ ] Create `TextureClass` with `"GUIButtons.dds"`
  - [ ] Call `Apply(0)` (stage 0)
  - [ ] Verify `glGetIntegerv(GL_TEXTURE_BINDING_2D)` returns correct ID
  - [ ] Delete texture → verify `glIsTexture()` returns false

**Dependencies**: Texture cache (28.4), TextureClass API
**Blockers**: None
**Status**: Not Started

---

## Phase 28.6: Font Atlas Support

**Goal**: Render text using texture atlas
**Files**: `GeneralsMD/Code/GameEngine/Source/GameClient/GameFont.cpp`
**Estimated Time**: 1 day

### Tasks

- [ ] **28.6.1** Load font atlas texture
  - [ ] Parse font descriptor (`.fnt` or embedded in code)
  - [ ] Load atlas TGA (e.g., `Arial12Bold.tga`)
  - [ ] Upload to GL via texture cache

- [ ] **28.6.2** Parse glyph metrics
  - [ ] Character widths (array or file)
  - [ ] UV coordinates for each glyph in atlas
  - [ ] Baseline/ascent/descent

- [ ] **28.6.3** Update `Blit_Char()` for OpenGL
  - [ ] Calculate glyph UV rect
  - [ ] Generate quad vertices (position + UVs)
  - [ ] Batch into vertex buffer
  - [ ] Render with atlas texture bound

- [ ] **28.6.4** String rendering
  - [ ] Loop through characters
  - [ ] Advance X position by glyph width
  - [ ] Handle newlines, tabs, color codes

- [ ] **28.6.5** Unit test
  - [ ] Render "Command & Conquer" at (100, 100)
  - [ ] Verify glyphs are crisp (no blur)
  - [ ] Test color modulation (red, white)

**Dependencies**: Texture cache (28.4), TGA loader (28.2)
**Blockers**: Font file format unknown (may need reverse-engineering)
**Status**: Not Started

---

## Phase 28.7: UI Rendering Test

**Goal**: Validate textures in main menu
**Files**: N/A (integration test)
**Estimated Time**: 0.5 days

### Tasks

- [ ] **28.7.1** Prepare test assets
  - [ ] Copy `Data/English/Art/Textures/` from original game
  - [ ] Verify `.dds` files present (backgrounds, buttons, cursors)

- [ ] **28.7.2** Run menu test
  - [ ] Launch GeneralsXZH
  - [ ] Wait for main menu
  - [ ] Verify background renders
  - [ ] Verify buttons render with correct icons
  - [ ] Verify cursor texture

- [ ] **28.7.3** Alpha blending test
  - [ ] Hover over semi-transparent UI element
  - [ ] Verify smooth alpha transition
  - [ ] Check no hard edges

- [ ] **28.7.4** Mipmap quality test
  - [ ] Zoom UI in/out (if supported)
  - [ ] Verify no aliasing/shimmering
  - [ ] Check distant UI elements are smooth

- [ ] **28.7.5** Performance test
  - [ ] Measure FPS in menu
  - [ ] Target: > 60 FPS
  - [ ] Monitor GL errors in log

**Dependencies**: All previous phases
**Blockers**: Need original game assets
**Status**: Not Started

---

## Phase 28.8: Skirmish Test

**Goal**: Validate textures in gameplay
**Files**: N/A (integration test)
**Estimated Time**: 1 day

### Tasks

- [ ] **28.8.1** Prepare test assets
  - [ ] Copy terrain textures (`Desert.dds`, `Grass.dds`)
  - [ ] Copy unit textures (`Tank.dds`, `Infantry.dds`)
  - [ ] Copy building textures

- [ ] **28.8.2** Start skirmish match
  - [ ] Select map
  - [ ] Start game
  - [ ] Wait for level load

- [ ] **28.8.3** Verify terrain rendering
  - [ ] Check ground textures visible
  - [ ] Verify LOD transitions smooth
  - [ ] Move camera → check mipmaps

- [ ] **28.8.4** Verify unit/building rendering
  - [ ] Select unit → check if textured
  - [ ] Build structure → verify texture
  - [ ] Zoom in/out → check quality

- [ ] **28.8.5** Stress test
  - [ ] Run for 10+ minutes
  - [ ] Monitor memory usage (should be stable)
  - [ ] Check FPS (target > 30)
  - [ ] Look for GL errors in log

- [ ] **28.8.6** Edge cases
  - [ ] Load different map types (desert, snow, urban)
  - [ ] Build all unit types
  - [ ] Trigger particle effects (explosions)

**Dependencies**: All previous phases
**Blockers**: Need original game assets
**Status**: Not Started

---

## Completion Criteria

### Phase 28 Complete When:
- [x] All 8 sub-phases marked complete
- [ ] DDS BC1/BC2/BC3 textures load and render correctly
- [ ] TGA textures load and render correctly
- [ ] Texture cache reduces duplicate loads
- [ ] UI text is readable (font atlas working)
- [ ] Main menu renders without crashes
- [ ] Skirmish match runs for 10+ minutes without crashes
- [ ] FPS targets met (menu > 60, game > 30)
- [ ] No GL errors during normal operation
- [ ] Documentation updated in `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)`

### Definition of Done:
- [ ] Code reviewed and merged to main
- [ ] Unit tests pass (DDS/TGA parsing)
- [ ] Integration tests pass (menu + skirmish)
- [ ] Performance benchmarks recorded
- [ ] Known issues documented
- [ ] Phase 29 scope defined

---

## Notes

**Current Blockers**: None
**Risks**:
- Font format may require reverse-engineering (low risk)
- Some DDS files may use unsupported formats (medium risk - mitigated with fallback)
- macOS case-sensitivity may cause texture not found (low risk - already handled)

**Next Immediate Action**: Create `DDS_FORMAT_SPEC.md` with header layout
