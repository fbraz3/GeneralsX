# PHASE10: Post-Processing

## Phase Title

Post-Processing - Tone Mapping and Effects (Optional)

## Objective

Implement optional post-processing effects including tone mapping for HDR support, bloom, and other visual enhancements. This phase is flexible and can be deferred.

## Dependencies

- PHASE05 (Render Loop)
- PHASE07 (Mesh Rendering)
- Framebuffer attachment creation

## Key Deliverables

1. Framebuffer setup for rendering to texture
2. Tone mapping shader implementation
3. Bloom effect (optional)
4. Post-process pipeline
5. Effect toggling

## Acceptance Criteria

- [ ] Tone mapping produces correct output
- [ ] Performance overhead less than 5% (10% acceptable)
- [ ] Bloom effect looks visually correct (if implemented)
- [ ] Effects can be enabled/disabled at runtime
- [ ] No color banding or artifacts

## Technical Details

### Components to Implement

1. **Framebuffer Management**
   - Off-screen render target creation
   - Blit to screen

2. **Tone Mapping**
   - Reinhard tone mapping
   - Exposure adjustment

3. **Optional Effects**
   - Bloom (downsampling, blur, combine)
   - Color grading
   - FXAA (anti-aliasing)

### Code Location

```
Core/GameEngineDevice/Source/PostProcessing/
GeneralsMD/Code/Graphics/PostProcessing/
```

## Estimated Scope

**MEDIUM** (3-4 days - optional, can be deferred)

- Framebuffer setup: 0.5 day
- Tone mapping: 1 day
- Bloom: 1.5 days
- Testing: 1 day

## Status

**not-started**

## Priority

**OPTIONAL** - Deferrable to post-MVP if performance is tight

## Testing Strategy

```bash
# Test post-processing effects
cmake --build build/macos-arm64 --target GeneralsXZH -j 4
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 ./GeneralsXZH 2>&1 | grep PostProcess
```

## Success Criteria

- Visual quality matches or exceeds original game
- Performance acceptable (60 FPS maintained)
- Effects enhance game appearance
- Can be toggled in options menu (PHASE14+)

## Reference Documentation

- Tone mapping algorithms
- Bloom effect implementation
- HDR rendering in modern graphics APIs
