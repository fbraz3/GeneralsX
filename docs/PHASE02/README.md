# PHASE02: Texture System

## Phase Title

Texture System - Load and Bind Textures

## Objective

Implement texture loading pipeline from DirectX format files (.dds, .tga) and bind them to GPU memory. This phase extends PHASE01's geometry rendering to support textured meshes from the game's .big archives.

## Dependencies

- PHASE01 (Geometry Rendering) - base graphics pipeline must work
- DirectX texture format parsers (DDS, TGA)
- Memory allocators for texture uploads
- Asset system understanding (.big file handling - see ASSET_SYSTEM.md)

## Key Deliverables

1. DDS and TGA file format parsing
2. GPU texture object creation
3. Texture binding in render pipeline
4. Mipmap generation (if applicable)
5. Texture sampling in shaders

## Acceptance Criteria

- [ ] DDS files parse correctly
- [ ] TGA files parse correctly
- [ ] Textures upload to GPU without crashes
- [ ] Textures loaded from .big archives (post-DirectX interception pattern - see ASSET_SYSTEM.md)
- [ ] Textured quad renders correctly
- [ ] No memory leaks in texture lifecycle
- [ ] Mipmaps generated where appropriate

## Technical Details

### Components to Implement

1. **File Format Parsers**
   - DDS header parsing (BC1/BC3 compression detection)
   - TGA header parsing
   - Pixel format conversion

2. **Texture Allocation**
   - GPU texture buffer allocation
   - Texture image view creation
   - Sampler setup

3. **Shader Integration**
   - Texture coordinate vertex attribute
   - Fragment shader sampling

### Code Location

```
Core/GameEngineDevice/Source/Texture/
GeneralsMD/Code/Graphics/TextureLoader/
```

### DDS Structure Support

```cpp
struct DDSHeader {
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipmapCount;
    uint32_t reserved[11];
};
```

### Texture Upload Pattern

```cpp
class TextureLoader {
public:
    static Texture* LoadFromFile(const std::string& path);
    static Texture* LoadFromMemory(const uint8_t* data, size_t size);
    
private:
    static bool ParseDDS(const uint8_t* data, TextureInfo& info);
    static bool ParseTGA(const uint8_t* data, TextureInfo& info);
};
```

## Estimated Scope

**MEDIUM** (2-3 days)

- File parsing: 1 day
- GPU upload: 0.5-1 day
- Shader integration: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Important References
- **Asset System**: `docs/PHASE00/ASSET_SYSTEM.md` - Complete .big file architecture and VFS patterns
- **Critical Lesson 1 (Phase 28.4)**: `docs/PHASE00/CRITICAL_LESSONS.md` - VFS Pattern & Post-DirectX Interception

## Testing Strategy

```bash
# Create test texture files (DDS, TGA)
# Render textured quad to validate
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
USE_METAL=1 ./GeneralsXZH 2>&1 | grep "Texture"
```

## Success Criteria

- Textured quad displays correctly
- No GPU errors
- Proper memory cleanup on shutdown

## Reference Documentation

- DDS format spec
- TGA format spec
- See `docs/MISC/BIG_FILES_REFERENCE.md` for texture archive structure
