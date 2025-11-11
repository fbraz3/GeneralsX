# PHASE11: UI Mesh Format

## Phase Title

UI Mesh Format - Parse UI Models

## Objective

Parse and load UI mesh format from .big archives. UI uses a specialized format different from gameplay meshes.

## Dependencies

- PHASE02 (Texture System)
- PHASE06 (Mesh Loading)
- .big file reading

## Key Deliverables

1. UI mesh format parser
2. UI vertex buffer creation
3. Material reference handling
4. Bounds calculation for hit testing
5. Format validation

## Acceptance Criteria

- [ ] UI meshes parse from .big without crashes
- [ ] Vertex data is correct
- [ ] Material IDs reference correctly
- [ ] No memory leaks
- [ ] Parsing is fast (< 10ms per file)

## Technical Details

### CRITICAL: Asset System Pattern
UI meshes are stored in `.big` archives. Follow the post-DirectX interception pattern from Phase 28.4:
- DirectX handles .big file loading and decompression
- Intercept at D3D8 surface/resource level
- Extract to GPU format

See `docs/PHASE00/ASSET_SYSTEM.md` for complete documentation.

### Components to Implement

1. **Mesh Format Definition**
   - Vertex layout (position, UV, color)
   - Index buffer structure
   - Material reference system

### Code Location

```
GeneralsMD/Code/UI/MeshLoader/
GeneralsMD/Code/Graphics/UIRenderer/
```

## Estimated Scope

**SMALL** (1-2 days)

- Format analysis: 0.5 day
- Parser implementation: 0.5 day
- Testing/validation: 1 day

## Status

**not-started**

## Important References
- **Asset System**: `docs/PHASE00/ASSET_SYSTEM.md` - .big file structure and loading patterns
- **Critical Lesson 1**: `docs/PHASE00/CRITICAL_LESSONS.md` - Post-DirectX interception pattern

## Testing Strategy

```bash
# Load sample UI meshes
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep "UI Mesh"
```

## Success Criteria

- All UI elements load successfully
- No parsing errors
- Fast loading times
- Correct vertex counts

## Reference Documentation

- See PHASE06 for general mesh loading patterns
- See `docs/MISC/BIG_FILES_REFERENCE.md` for UI mesh locations
