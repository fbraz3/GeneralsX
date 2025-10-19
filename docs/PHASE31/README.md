# Phase 31: Texture System

**Estimated Time**: 10-14 days  
**Priority**: HIGH - Critical for menu graphics and game rendering

## Overview

Implement complete texture loading and rendering system for Metal backend, supporting DDS (compressed) and TGA (uncompressed) formats used by Command & Conquer Generals.

## Tasks

### Phase 31.1: DDS Texture Loader (3-4 days)

- BC1/BC2/BC3 (DXT1/3/5) compressed format support
- RGB8/RGBA8 uncompressed fallback
- Mipmap level extraction
- Header validation and error handling

### Phase 31.2: TGA Texture Loader (2 days)

- RLE compressed TGA support
- Uncompressed 24/32-bit TGA
- Vertical flip handling
- Color format conversion

### Phase 31.3: Texture Upload & Binding (2-3 days)

- MTLTexture creation from DDS/TGA data
- Texture cache system integration
- Sampler state configuration
- Texture coordinate handling

### Phase 31.4: UI Rendering Validation (2 days)

- Menu background textures
- Button and UI element rendering
- Texture filtering verification
- Performance optimization

### Phase 31.5: Testing & Documentation (1-2 days)

- Test with game menu textures
- Validate texture formats
- Performance profiling
- Update documentation

## Files to Modify

- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/textureloader.cpp` - Texture loading pipeline
- `Core/Libraries/Source/WWVegas/WW3D2/metalwrapper.h/mm` - MTLTexture API
- `tests/test_dds_loader.cpp` - DDS format tests
- `tests/test_tga_loader.cpp` - TGA format tests

## Success Criteria

- ✅ Game menu backgrounds render correctly
- ✅ UI buttons display textures
- ✅ Both DDS and TGA formats supported
- ✅ No texture corruption or artifacts
- ✅ Acceptable performance (< 100ms texture load time)

## Dependencies

- Phase 30: Metal Backend ✅ COMPLETE
- Phase 29: Metal Render States ✅ COMPLETE
- .BIG file system (already functional)

## Notes

This phase addresses the critical texture loading gap preventing menu graphics from displaying. The game stores textures in .big archives which are already accessible via the VFS system.