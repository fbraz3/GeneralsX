# Phase 37: Asset Loading & Content Rendering (FASE A - IMEDIATO)

**Status**: Ready to Start  
**Estimated Time**: 3-5 days  
**Priority**: 🚨 **CRITICAL** - Blocker for all visual content

## Overview

Diagnose and fix why assets (textures, meshes, UI) are not loading. Currently, game renders Metal blue screen but no content. Phase 37 validates the complete asset loading pipeline: .big file decompression → DirectX surface → Metal texture upload.

## Current Status

✅ **What's Working**:

- Metal backend operational (rendering blue screen)
- DirectX device created and functional
- Game loop executing normally
- .big files being read from disk

❌ **What's Missing**:

- Textures not rendering (only Metal blue screen visible)
- Assets not loading from .big archives into memory
- TextureCache not being populated
- DirectX surfaces not being transferred to Metal

## Tasks

### Phase 37.1: Texture Loading Pipeline Diagnostics (1 day)

**Objective**: Trace the complete path from .big file to Metal texture

**Checklist**:

- [ ] Add detailed logging to `TextureClass::Apply_New_Surface()`
  - [ ] Log when called (should happen during shader binding)
  - [ ] Log DirectX surface information (format, dimensions)
  - [ ] Log Metal texture creation results

- [ ] Verify `TextureCache::Load_From_Memory()` is being called
  - [ ] Add logging at entry point
  - [ ] Log pixel data size and format
  - [ ] Confirm Metal texture handle creation

- [ ] Check DirectX to Metal conversion in `MetalWrapper::CreateTextureFromMemory()`
  - [ ] Validate format conversion (D3DFMT → MTLPixelFormat)
  - [ ] Log bytesPerRow alignment (critical for Apple Silicon)
  - [ ] Verify texture buffer creation

**Files to Investigate**:

- `Core/Libraries/Source/WWVegas/WW3D2/texture.cpp` - Apply_New_Surface()
- `Core/GameEngineDevice/Source/MetalDevice/metalwrapper.mm` - CreateTextureFromMemory()
- `Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp` - DirectX surface handling

**Commands**:
```bash
# Build with debug logging
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee /tmp/phase37_build.log

# Run with extended logging timeout
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 30 ./GeneralsXZH 2>&1 | tee /tmp/phase37_diagnostics.log

# Search for texture loading errors
grep -i "texture\|surface\|format" /tmp/phase37_diagnostics.log | head -50
```

### Phase 37.2: Asset Loading from .big Files (1-2 days)

**Objective**: Verify .big file decompression and asset extraction works

**Checklist**:

- [ ] Confirm `Win32BIGFileSystem::openArchiveFile()` working
  - [ ] Log number of files found in each .big archive
  - [ ] Verify INI.big loads correctly (contains game configuration)
  - [ ] Check Textures.big contents (should contain .tga/.dds files)

- [ ] Validate texture file extraction from .big
  - [ ] Add logging to `Win32BIGFileSystem::loadBigFilesFromDirectory()`
  - [ ] Log each texture filename as it's decompressed
  - [ ] Confirm file sizes match expected values

- [ ] Test DirectX VFS integration
  - [ ] Verify `TheFileSystem->openFile(filename)` returns valid handle
  - [ ] Check `D3DXLoadSurfaceFromMemory()` receives correct data
  - [ ] Validate surface format (BC1/BC3 compressed vs uncompressed)

**Files to Investigate**:

- `Core/GameEngine/Source/Common/System/Win32BIGFileSystem.cpp` - Archive extraction
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - DirectX device methods
- `Core/GameEngine/Source/Common/FileSystem.cpp` - File access abstraction

**Commands**:
```bash
# Check .big file contents (reference)
ls -lah $HOME/GeneralsX/GeneralsMD/Data/*.big

# Count textures in archive
grep -c "\.tga\|\.dds" /tmp/phase37_diagnostics.log

# Look for BIG file loading messages
grep -i "bigfile\|archive\|loaded" /tmp/phase37_diagnostics.log
```

### Phase 37.3: DirectX to Metal Texture Transfer (1-2 days)

**Objective**: Validate texture data flows from DirectX surface to Metal texture

**Checklist**:

- [ ] Implement complete texture binding hookpoint
  - [ ] Hook `IDirect3DTexture8::LockRect()` to capture pixel data
  - [ ] Ensure `LockRect()` returns valid pointer to pixel buffer
  - [ ] Validate stride/pitch for compressed formats

- [ ] Test Metal texture creation from DirectX data
  - [ ] Create `MTLTexture` with correct pixel format
  - [ ] Validate `bytesPerRow` alignment (256 bytes minimum on Apple Silicon)
  - [ ] Test bytesPerRow for various texture sizes (64x64, 128x128, 256x256)

- [ ] Verify texture rendering in shader
  - [ ] Add test shader that uses transferred texture
  - [ ] Render simple quad with texture to Metal
  - [ ] Capture frame buffer to verify texture visible

- [ ] Test format conversions (critical issue from Phase 28)
  - [ ] RGB8 formats (most common for TGA)
  - [ ] RGBA8 with alpha
  - [ ] BC1 (DXT1) compressed
  - [ ] BC3 (DXT5) compressed

**Reference**: Phase 28.4 discovered that `Apply_New_Surface()` is the correct interception point, NOT VFS integration.

**Files to Modify**:

- `Core/Libraries/Source/WWVegas/WW3D2/texture.cpp` - Add logging + hookpoint
- `Core/GameEngineDevice/Source/MetalDevice/metalwrapper.mm` - Format conversion
- `Core/GameEngineDevice/Source/MetalDevice/shaders/*.metal` - Test texture sampling

**Commands**:
```bash
# Compile with texture debugging enabled
cmake --build build/macos-arm64 --target GeneralsXZH -j 4

# Run test focusing on texture operations
cd $HOME/GeneralsX/GeneralsMD && USE_METAL=1 timeout 30 ./GeneralsXZH 2>&1 | tee /tmp/phase37_texture_test.log

# Verify texture binding calls
grep -A 2 "Apply_New_Surface\|LockRect\|CreateTextureFromMemory" /tmp/phase37_texture_test.log
```

## Success Criteria

- ✅ At least 10+ textures loading from .big files
- ✅ Textures visible in Metal rendering (not just blue screen)
- ✅ No Metal validation errors for texture formats
- ✅ Frame rate stable (60+ FPS) with textures loaded
- ✅ All logging shows clear data flow: .big → DirectX → Metal

## Debugging Strategy

1. **Enable verbose logging** in three subsystems:
   - BIGFileSystem (archive reading)
   - DirectX surface management
   - Metal texture creation

2. **Use frame capture** to inspect actual textures in GPU memory:
   - Metal debugger can show uploaded textures
   - Compare expected vs actual formats
   - Identify bytesPerRow/alignment issues

3. **Compare with Phase 28.4 working state**:
   - Phase 28.4 proof shows 7 textures loaded successfully
   - Use those as reference for current debugging
   - See `docs/PLANNING/28/PHASE28/CRITICAL_VFS_DISCOVERY.md`

4. **Test with minimal scene**:
   - Load Shell map (main menu) - minimal texture requirements
   - Verify menu background renders
   - Validate UI element textures

## Known Issues from Previous Phases

1. **BytesPerRow Alignment**: Apple Silicon requires 256-byte aligned texture rows
   - Formula: `((width * bytesPerPixel + 255) / 256) * 256`
   - See Phase 28.3 implementation for reference

2. **Format Mapping Complexity**:
   - D3DFMT_DXT1 → MTLPixelFormatBC1_RGBA
   - D3DFMT_DXT5 → MTLPixelFormatBC3_RGBA
   - D3DFMT_R8G8B8 → MTLPixelFormatRGB8Unorm (needs padding to RGBA)

3. **DirectX Surface Locking**:
   - Must use `D3DLOCK_READONLY` for safe reading
   - Some formats require special handling (compressed)
   - Stride/pitch may differ from expected dimensions

## Files to Monitor

| File | Purpose | Phase | Status |
|------|---------|-------|--------|
| `texture.cpp` | Texture asset management | 28 | ⏳ Needs logging |
| `metalwrapper.mm` | Metal API integration | 30 | ⏳ Test format conversion |
| `Win32BIGFileSystem.cpp` | Archive extraction | 26 | ⏳ Add diagnostics |
| `dx8wrapper.cpp` | DirectX device mock | 29 | ⏳ Trace texture methods |

## References

- `docs/PLANNING/28/PHASE28/CRITICAL_VFS_DISCOVERY.md` - Why VFS integration failed, why Apply_New_Surface() works
- `docs/PLANNING/3/PHASE30/` - Metal backend architecture
- `docs/PLANNING/3/PHASE31/` - Texture system integration
- `references/dxgldotorg-dxgl/` - DirectX→OpenGL texture patterns (applicable to DirectX→Metal)

## Dependencies

✅ **Complete**:

- Phase 30: Metal Backend
- Phase 31: Texture System Framework
- Phase 28: DirectX Texture Interception

⏳ **Blocking**:

- None (Phase 37 unblocks Phase 38, 39, 40)

## Next Phase

**Phase 38**: Audio System Validation & Fixes (if Phase 37 reveals only minor texture issues)

OR

**Phase 37.4**: Extended Debugging (if texture loading issues are complex)

---

**Last Updated**: October 27, 2025  
**Created by**: GeneralsX Development Team  
**Status**: Ready for Phase 37.1 execution
