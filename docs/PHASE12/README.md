# PHASE12: UI Texture Loading

## Phase Title

UI Texture Loading - Load Textures from .big Files

## Objective

Load UI textures from game archives with proper format handling and caching.

## Dependencies

- PHASE02 (Texture System)
- PHASE11 (UI Mesh Format)

## Key Deliverables

1. UI texture extraction from .big archives
2. Format conversion if needed
3. Caching mechanism
4. Memory management

## Acceptance Criteria

- [ ] UI textures load from .big files
- [ ] Correct colors and alpha channels
- [ ] Caching prevents redundant loads
- [ ] Memory usage acceptable
- [ ] No texture bleeding at edges

## Technical Details

### Components to Implement

1. **Archive Integration**
   - Locate UI texture .big files
   - Extract texture streams
   - Handle different compression formats

2. **Cache Management**
   - LRU cache for active UI textures
   - Memory limits

### Code Location

```
GeneralsMD/Code/UI/TextureLoader/
GeneralsMD/Code/Graphics/TextureCache/
```

### CRITICAL: Asset System Pattern (Phase 28.4)
UI textures are in `.big` archives. Use post-DirectX interception:
1. DirectX loads from .big (VFS)
2. Intercept at D3D8 texture surface level
3. Extract and upload to Vulkan/Metal

See `docs/PHASE00/ASSET_SYSTEM.md` for pattern.

## Estimated Scope

**SMALL** (1-2 days)

- Archive integration: 0.5 day
- Caching: 0.5 day
- Testing: 1 day

## Status

**not-started**

## Important References
- **Asset System**: `docs/PHASE00/ASSET_SYSTEM.md` - Complete .big file architecture and VFS patterns
- **Critical Lesson 1**: `docs/PHASE00/CRITICAL_LESSONS.md` - Post-DirectX interception

## Testing Strategy

```bash
# Load UI and verify textures
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
```

## Success Criteria

- All UI textures render correctly
- No visual artifacts
- Memory usage stays within limits

## Reference Documentation

- PHASE02 texture system
- See PHASE11 for UI mesh integration
