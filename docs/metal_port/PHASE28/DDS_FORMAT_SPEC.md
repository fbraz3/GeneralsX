# DDS File Format Specification

**DirectDraw Surface (.dds)** - Microsoft texture format with GPU compression support

## File Structure

```
+-------------------+
|  Magic (4 bytes)  |  "DDS " (0x20534444)
+-------------------+
|  Header (124 B)   |  DDS_HEADER structure
+-------------------+
|  [DX10 Header]    |  Optional DDS_HEADER_DXT10 (if FourCC == "DX10")
+-------------------+
|  Pixel Data       |  Compressed or uncompressed texture data
|  (variable size)  |  Includes all mipmap levels
+-------------------+
```

## DDS_PIXELFORMAT Structure (32 bytes)

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 4 | DWORD | `dwSize` | Structure size (must be 32) |
| 4 | 4 | DWORD | `dwFlags` | Flags (DDPF_FOURCC, DDPF_RGB, etc.) |
| 8 | 4 | FOURCC | `dwFourCC` | Format code (e.g., "DXT1") |
| 12 | 4 | DWORD | `dwRGBBitCount` | Bits per pixel |
| 16 | 4 | DWORD | `dwRBitMask` | Red channel mask |
| 20 | 4 | DWORD | `dwGBitMask` | Green channel mask |
| 24 | 4 | DWORD | `dwBBitMask` | Blue channel mask |
| 28 | 4 | DWORD | `dwABitMask` | Alpha channel mask |

### Pixel Format Flags

```cpp
#define DDPF_ALPHAPIXELS  0x00000001  // Contains alpha channel
#define DDPF_ALPHA        0x00000002  // Alpha-only surface
#define DDPF_FOURCC       0x00000004  // FourCC code valid
#define DDPF_RGB          0x00000040  // Uncompressed RGB
#define DDPF_LUMINANCE    0x00020000  // Luminance data
```

## DDS_HEADER Structure (124 bytes)

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 4 | DWORD | `dwSize` | Structure size (must be 124) |
| 4 | 4 | DWORD | `dwFlags` | Feature flags |
| 8 | 4 | DWORD | `dwHeight` | Texture height (pixels) |
| 12 | 4 | DWORD | `dwWidth` | Texture width (pixels) |
| 16 | 4 | DWORD | `dwPitchOrLinearSize` | Bytes per scanline or total size |
| 20 | 4 | DWORD | `dwDepth` | Depth (for volume textures) |
| 24 | 4 | DWORD | `dwMipMapCount` | Number of mipmap levels |
| 28 | 44 | DWORD[11] | `dwReserved1` | Reserved (unused) |
| 72 | 32 | DDS_PIXELFORMAT | `ddspf` | Pixel format structure |
| 104 | 4 | DWORD | `dwCaps` | Surface complexity flags |
| 108 | 4 | DWORD | `dwCaps2` | Additional surface flags |
| 112 | 4 | DWORD | `dwCaps3` | Reserved |
| 116 | 4 | DWORD | `dwCaps4` | Reserved |
| 120 | 4 | DWORD | `dwReserved2` | Reserved |

### Header Flags

```cpp
#define DDSD_CAPS         0x00000001  // dwCaps valid
#define DDSD_HEIGHT       0x00000002  // dwHeight valid
#define DDSD_WIDTH        0x00000004  // dwWidth valid
#define DDSD_PITCH        0x00000008  // dwPitchOrLinearSize is pitch
#define DDSD_PIXELFORMAT  0x00001000  // ddspf valid
#define DDSD_MIPMAPCOUNT  0x00020000  // dwMipMapCount valid
#define DDSD_LINEARSIZE   0x00080000  // dwPitchOrLinearSize is total size
#define DDSD_DEPTH        0x00800000  // dwDepth valid (volume texture)
```

### Caps Flags

```cpp
#define DDSCAPS_COMPLEX   0x00000008  // Multiple surfaces (mipmaps/cubemaps)
#define DDSCAPS_TEXTURE   0x00001000  // Is a texture
#define DDSCAPS_MIPMAP    0x00400000  // Has mipmaps
```

### Caps2 Flags (Cubemaps & Volumes)

```cpp
#define DDSCAPS2_CUBEMAP           0x00000200  // Is a cubemap
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400  // +X face
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800  // -X face
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000  // +Y face
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000  // -Y face
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000  // +Z face
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000  // -Z face
#define DDSCAPS2_VOLUME            0x00200000  // Is a volume texture
```

## Block Compression Formats (BC1/BC2/BC3)

### BC1 (DXT1) - FourCC "DXT1"

**Compression**: 6:1 (4 bits per pixel)  
**Block Size**: 8 bytes (4x4 pixels)  
**Alpha**: 1-bit (opaque or transparent)

**Block Structure**:

```
+-------------+-------------+
| color0 (16) | color1 (16) |  RGB565 endpoints
+-------------+-------------+
| indices (32 bits)         |  2 bits per pixel (4x4 = 16 pixels)
+---------------------------+
```

**Calculation**:

```cpp
block_size = 8; // bytes
blocks_wide = max(1, (width + 3) / 4);
blocks_high = max(1, (height + 3) / 4);
data_size = blocks_wide * blocks_high * block_size;
```

**OpenGL Format**: `GL_COMPRESSED_RGBA_S3TC_DXT1_EXT` (0x83F1)

### BC2 (DXT3) - FourCC "DXT3"

**Compression**: 4:1 (8 bits per pixel)  
**Block Size**: 16 bytes (4x4 pixels)  
**Alpha**: 4-bit explicit (16 levels)

**Block Structure**:

```
+---------------------------+
| alpha (64 bits)           |  4 bits per pixel (4x4 = 16 pixels)
+-------------+-------------+
| color0 (16) | color1 (16) |  RGB565 endpoints
+-------------+-------------+
| indices (32 bits)         |  2 bits per pixel
+---------------------------+
```

**Calculation**:

```cpp
block_size = 16; // bytes
blocks_wide = max(1, (width + 3) / 4);
blocks_high = max(1, (height + 3) / 4);
data_size = blocks_wide * blocks_high * block_size;
```

**OpenGL Format**: `GL_COMPRESSED_RGBA_S3TC_DXT3_EXT` (0x83F2)

### BC3 (DXT5) - FourCC "DXT5"

**Compression**: 4:1 (8 bits per pixel)  
**Block Size**: 16 bytes (4x4 pixels)  
**Alpha**: 8-bit interpolated (smooth gradients)

**Block Structure**:

```
+-------------+-------------+
| alpha0 (8)  | alpha1 (8)  |  Alpha endpoints
+---------------------------+
| alpha indices (48 bits)   |  3 bits per pixel (4x4 = 16 pixels)
+-------------+-------------+
| color0 (16) | color1 (16) |  RGB565 endpoints
+-------------+-------------+
| indices (32 bits)         |  2 bits per pixel
+---------------------------+
```

**Calculation**:

```cpp
block_size = 16; // bytes
blocks_wide = max(1, (width + 3) / 4);
blocks_high = max(1, (height + 3) / 4);
data_size = blocks_wide * blocks_high * block_size;
```

**OpenGL Format**: `GL_COMPRESSED_RGBA_S3TC_DXT5_EXT` (0x83F3)

## Mipmap Layout

Mipmaps are stored sequentially in memory, largest first:

```
+-------------------+
|  Mip 0 (full res) |  e.g., 1024x1024
+-------------------+
|  Mip 1            |  512x512
+-------------------+
|  Mip 2            |  256x256
+-------------------+
|  ...              |
+-------------------+
|  Mip N            |  1x1 (smallest)
+-------------------+
```

**Mip Dimensions**:

```cpp
mip_width = max(1, width >> mip_level);
mip_height = max(1, height >> mip_level);
```

**Example (1024x1024 DXT5)**:

| Mip | Width | Height | Blocks | Size (bytes) |
|-----|-------|--------|--------|--------------|
| 0 | 1024 | 1024 | 256x256 | 1,048,576 |
| 1 | 512 | 512 | 128x128 | 262,144 |
| 2 | 256 | 256 | 64x64 | 65,536 |
| 3 | 128 | 128 | 32x32 | 16,384 |
| 4 | 64 | 64 | 16x16 | 4,096 |
| 5 | 32 | 32 | 8x8 | 1,024 |
| 6 | 16 | 16 | 4x4 | 256 |
| 7 | 8 | 8 | 2x2 | 64 |
| 8 | 4 | 4 | 1x1 | 16 |
| 9 | 2 | 2 | 1x1 | 16 |
| 10 | 1 | 1 | 1x1 | 16 |

**Total**: 1,398,128 bytes (~1.33 MB)

## Validation Checklist

- [ ] Magic number is `0x20534444` ("DDS ")
- [ ] `dwSize` == 124
- [ ] `dwFlags` includes `DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT`
- [ ] `dwWidth` and `dwHeight` > 0
- [ ] If mipmaps: `dwFlags & DDSD_MIPMAPCOUNT` and `dwMipMapCount` > 0
- [ ] If compressed: `ddspf.dwFlags & DDPF_FOURCC`
- [ ] FourCC is "DXT1", "DXT3", or "DXT5"
- [ ] File size matches calculated data size

## Implementation Example

```cpp
struct DDSPixelFormat {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
};

struct DDSHeader {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDSPixelFormat ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
};

bool ValidateDDS(const DDSHeader& header) {
    if (header.dwSize != 124) return false;
    if (header.dwFlags & DDSD_HEIGHT == 0) return false;
    if (header.dwFlags & DDSD_WIDTH == 0) return false;
    if (header.dwWidth == 0 || header.dwHeight == 0) return false;
    
    // Check for compressed format
    if (header.ddspf.dwFlags & DDPF_FOURCC) {
        uint32_t fourcc = header.ddspf.dwFourCC;
        if (fourcc != MAKEFOURCC('D','X','T','1') &&
            fourcc != MAKEFOURCC('D','X','T','3') &&
            fourcc != MAKEFOURCC('D','X','T','5')) {
            return false; // Unsupported format
        }
    }
    
    return true;
}
```

## References

- [Microsoft DDS Documentation](https://docs.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds)
- [OpenGL S3TC Extension](https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_compression_s3tc.txt)
- [Block Compression Formats](https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats/)
