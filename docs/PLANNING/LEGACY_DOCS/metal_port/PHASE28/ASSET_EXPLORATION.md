# Phase 28: Asset Exploration - Texture Files Analysis

**Date**: October 13, 2025  
**Purpose**: Identify test textures for Phase 28.1 DDS Loader implementation  
**Status**: Exploration complete - test candidates identified

---

## 📦 Available .BIG Files (Zero Hour)

### Texture Archives

| File | Size | Description | Priority |
|------|------|-------------|----------|
| **TexturesZH.big** | 212 MB | Main texture archive | 🔥 PRIMARY |
| TerrainZH.big | 8.3 MB | Terrain textures | 🟡 SECONDARY |
| WindowZH.big | 8.1 MB | UI textures | 🟢 TERTIARY |

### Other Assets

| File | Size | Description |
|------|------|-------------|
| W3DZH.big | 181 MB | 3D models |
| SpeechEnglishZH.big | 242 MB | Voice audio |
| AudioEnglishZH.big | 56 MB | Sound effects |
| MapsZH.big | 38 MB | Multiplayer maps |
| INIZH.big | 18 MB | Configuration files |

**Total Assets**: 19 .big files, ~800 MB

---

## 🎨 Test Texture Candidates

### DDS Files (Extracted to Data/English/Art/Textures/)

| File | Size | Format | Dimensions | Mipmaps | Use Case |
|------|------|--------|------------|---------|----------|
| **defeated.dds** | 256 KB | BC3 (DXT5) | 1024x256 | 0 | Game over screen |
| **gameover.dds** | 342 KB | BC3 (DXT5) | 1024x256 | 0 | Game over screen |
| **victorious.dds** | 256 KB | BC3 (DXT5) | 1024x256 | 0 | Victory screen |

**Analysis** (defeated.dds):
```
Magic: DDS 
Header Size: 124 bytes
Flags: 0x00001007
Dimensions: 1024x256
Pitch/LinearSize: 0 bytes
Mipmap Count: 0

Pixel Format:
  Size: 32
  Flags: 0x00000004
  FourCC: 'DXT5'
  Format: BC3 (DXT5) - RGBA interpolated alpha
  Mipmap 0: 1024x256 = 262144 bytes

Expected file size: 262272 bytes (128 header + 262144 data)
Actual file size: 262272 bytes ✓
```

**Header Breakdown** (hexdump):
```
00000000: 4444 5320 7c00 0000 0710 0000 0001 0000  DDS |...........
          ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
          Magic    dwSize   dwFlags  dwHeight dwWidth
          "DDS "   124      0x1007   256      1024

00000010: 0004 0000 0000 0000 0000 0000 0000 0000  ................
          ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
          Pitch    Depth    MipCount
          1024     0        0

00000050: 0400 0000 4458 5435 0000 0000 0000 0000  ....DXT5........
          ^^^^^^^^ ^^^^^^^^
          pfFlags  pfFourCC
          0x04     "DXT5"
```

### TGA Files (Data/WaterPlane/)

| File Pattern | Count | Size | Use Case |
|--------------|-------|------|----------|
| caust*.tga | 32 | 16 KB each | Water caustics animation |

**Example**: caust00.tga, caust01.tga, ... caust31.tga

**Analysis** (pending - Phase 28.2):
- Likely 64x64 or 128x128 uncompressed RGBA
- Used for animated water effects
- Good candidate for TGA loader testing

---

## 🔍 DDS Format Details

### BC3 (DXT5) Compression

**Block Structure**:
- **Block Size**: 16 bytes (128 bits)
- **Pixels per Block**: 4x4 = 16 pixels
- **Compression Ratio**: ~4:1 (compared to RGBA8)

**Block Layout** (16 bytes):
```
Bytes 0-7:   Alpha block (8 bytes)
  Bytes 0-1: Alpha0, Alpha1 (2 reference values)
  Bytes 2-7: 3-bit alpha indices (16 pixels * 3 bits = 48 bits = 6 bytes)

Bytes 8-15:  Color block (8 bytes)
  Bytes 8-9:  Color0 (RGB565 - 16 bits)
  Bytes 10-11: Color1 (RGB565 - 16 bits)
  Bytes 12-15: 2-bit color indices (16 pixels * 2 bits = 32 bits = 4 bytes)
```

**Alpha Interpolation**:
```
if (alpha0 > alpha1):
  alpha_interp[0] = alpha0
  alpha_interp[1] = alpha1
  alpha_interp[2] = (6*alpha0 + 1*alpha1) / 7
  alpha_interp[3] = (5*alpha0 + 2*alpha1) / 7
  alpha_interp[4] = (4*alpha0 + 3*alpha1) / 7
  alpha_interp[5] = (3*alpha0 + 4*alpha1) / 7
  alpha_interp[6] = (2*alpha0 + 5*alpha1) / 7
  alpha_interp[7] = (1*alpha0 + 6*alpha1) / 7
else:
  alpha_interp[0] = alpha0
  alpha_interp[1] = alpha1
  alpha_interp[2] = (4*alpha0 + 1*alpha1) / 5
  alpha_interp[3] = (3*alpha0 + 2*alpha1) / 5
  alpha_interp[4] = (2*alpha0 + 3*alpha1) / 5
  alpha_interp[5] = (1*alpha0 + 4*alpha1) / 5
  alpha_interp[6] = 0x00
  alpha_interp[7] = 0xFF
```

**Color Interpolation**:
```
color_interp[0] = color0
color_interp[1] = color1
color_interp[2] = (2*color0 + color1) / 3
color_interp[3] = (color0 + 2*color1) / 3
```

### Metal Pixel Format Mapping

| DDS FourCC | Metal Pixel Format | Block Size |
|------------|-------------------|------------|
| DXT1 | MTLPixelFormatBC1_RGBA | 8 bytes |
| DXT3 | MTLPixelFormatBC2_RGBA | 16 bytes |
| **DXT5** | **MTLPixelFormatBC3_RGBA** | **16 bytes** |
| - (RGB) | MTLPixelFormatRGBA8Unorm | 4 bytes/pixel |
| - (RGBA) | MTLPixelFormatRGBA8Unorm | 4 bytes/pixel |

**Note**: BC1/BC2/BC3 require macOS 10.11+ (all supported)

---

## 🎯 Recommended Test Strategy

### Phase 28.1: DDS Loader Implementation

**Test File**: `defeated.dds` (256 KB, 1024x256, BC3/DXT5)

**Rationale**:
1. ✅ Simple format (BC3 only, no mipmaps)
2. ✅ Medium size (not too small, not too large)
3. ✅ Known location (Data/English/Art/Textures/)
4. ✅ Visible result (full-screen game over texture)
5. ✅ Real game asset (authentic test case)

**Implementation Steps**:
1. Parse DDS header (128 bytes)
   - Validate magic number "DDS "
   - Read dimensions (width, height)
   - Read mipmap count
   - Read FourCC ("DXT5")

2. Calculate data size
   - Blocks: (width/4) * (height/4)
   - Size: blocks * 16 bytes (BC3 block size)
   - Example: (1024/4) * (256/4) * 16 = 256 * 64 * 16 = 262,144 bytes

3. Create MTLTexture
   ```objc
   MTLTextureDescriptor* desc = [MTLTextureDescriptor new];
   desc.width = 1024;
   desc.height = 256;
   desc.pixelFormat = MTLPixelFormatBC3_RGBA;
   desc.textureType = MTLTextureType2D;
   desc.mipmapLevelCount = 1;
   desc.usage = MTLTextureUsageShaderRead;
   
   id<MTLTexture> texture = [g_device newTextureWithDescriptor:desc];
   ```

4. Upload compressed data
   ```objc
   [texture replaceRegion:MTLRegionMake2D(0, 0, 1024, 256)
              mipmapLevel:0
                withBytes:ddsData
              bytesPerRow:0  // Compressed format - ignored
            bytesPerImage:0  // Single slice - ignored
   ];
   ```

5. Validate
   - Check texture created successfully
   - Verify texture dimensions
   - Test texture binding to fragment shader
   - Render textured quad (Phase 28.3)

### Expected Results

**Success Criteria**:
- ✅ DDS file parsed without errors
- ✅ MTLTexture created with BC3 format
- ✅ Compressed data uploaded correctly
- ✅ No Metal validation errors
- ✅ Texture ID returned to game code

**Visual Validation** (Phase 28.3):
- Render 1024x256 quad with defeated.dds texture
- Should display "DEFEATED" text graphic
- Colors should match original texture

---

## 📊 File Statistics

### DDS Files Analyzed: 3

| Metric | Value |
|--------|-------|
| Total Size | 854 KB |
| Format | BC3 (DXT5) - 100% |
| Dimensions | 1024x256 (all) |
| Mipmaps | 0 (all) |
| Compression | RGBA interpolated alpha |

### TGA Files Found: 32+

| Metric | Value |
|--------|-------|
| Total Size | ~512 KB |
| Location | Data/WaterPlane/ |
| Use Case | Water caustics animation |
| Format | TBD (pending analysis) |

---

## 🚀 Next Actions

### Immediate (Phase 28.1):
1. ✅ Test texture identified: `defeated.dds`
2. ⏳ Create `ddsloader.h/cpp` with header parser
3. ⏳ Implement BC3 format detection
4. ⏳ Add Metal texture creation in `metalwrapper.mm`
5. ⏳ Test with defeated.dds file

### Future (Phase 28.2):
1. ⏳ Analyze TGA file format (caust00.tga)
2. ⏳ Create `tgaloader.h/cpp` with RLE support
3. ⏳ Test with water caustics textures

### Integration (Phase 28.3):
1. ⏳ Implement texture cache (hash map)
2. ⏳ Bind textures to fragment shader
3. ⏳ Render textured quad for validation

**Estimated Time**: 3-4 days for Phase 28.1 complete

---

## 📝 Tools Created

### analyze_dds.py

**Location**: `/tmp/analyze_dds.py` (temporary)

**Purpose**: Parse DDS headers and display format information

**Usage**:
```bash
python3 /tmp/analyze_dds.py <file.dds>
```

**Output**:
- Magic number validation
- Header size
- Flags
- Dimensions (width x height)
- Mipmap count
- Pixel format (FourCC, compression type)
- Expected vs actual file size

**Example**:
```bash
$ python3 /tmp/analyze_dds.py defeated.dds
Magic: DDS 
Dimensions: 1024x256
Format: BC3 (DXT5) - RGBA interpolated alpha
Expected file size: 262272 bytes
Actual file size: 262272 bytes
```

---

## 📚 References

### DDS Format Specification
- Microsoft DirectDraw Surface (.dds) format
- FourCC codes: DXT1, DXT3, DXT5
- BC1/BC2/BC3 block compression

### Metal Texture Formats
- `MTLPixelFormatBC1_RGBA` - DXT1 (macOS 10.11+)
- `MTLPixelFormatBC2_RGBA` - DXT3 (macOS 10.11+)
- `MTLPixelFormatBC3_RGBA` - DXT5 (macOS 10.11+)

### Game Assets
- Data/English/Art/Textures/ - UI textures (DDS)
- Data/WaterPlane/ - Effect textures (TGA)
- TexturesZH.big - Main texture archive (212 MB)

---

**Status**: Asset exploration complete ✅  
**Next Step**: Begin Phase 28.1 DDS Loader implementation  
**Test File**: defeated.dds (256 KB, 1024x256, BC3/DXT5)
