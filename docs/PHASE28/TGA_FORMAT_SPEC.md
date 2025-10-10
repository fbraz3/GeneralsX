# TGA File Format Specification

**Targa Image File (.tga)** - Truevision raster graphics format

## File Structure

```
+-------------------+
|  Header (18 B)    |  TGA_HEADER structure
+-------------------+
|  [Image ID]       |  Optional (length in header.idLength)
+-------------------+
|  [Color Map]      |  Optional (if header.colorMapType == 1)
+-------------------+
|  Pixel Data       |  Image data (compressed or uncompressed)
|  (variable size)  |
+-------------------+
|  [Footer]         |  Optional (TGA 2.0 extension)
+-------------------+
```

## TGA_HEADER Structure (18 bytes)

| Offset | Size | Type | Field | Description |
|--------|------|------|-------|-------------|
| 0 | 1 | BYTE | `idLength` | Length of image ID field (0-255) |
| 1 | 1 | BYTE | `colorMapType` | 0=no colormap, 1=has colormap |
| 2 | 1 | BYTE | `imageType` | See Image Types below |
| 3 | 5 | BYTES | `colorMapSpec` | Color map specification |
| 8 | 2 | WORD | `xOrigin` | X origin (usually 0) |
| 10 | 2 | WORD | `yOrigin` | Y origin (usually 0) |
| 12 | 2 | WORD | `width` | Image width (pixels) |
| 14 | 2 | WORD | `height` | Image height (pixels) |
| 16 | 1 | BYTE | `pixelDepth` | Bits per pixel (8/16/24/32) |
| 17 | 1 | BYTE | `imageDescriptor` | Image descriptor flags |

## Image Types

| Value | Type | Description |
|-------|------|-------------|
| 0 | No Image Data | Empty image |
| 1 | Uncompressed Color-Mapped | 8-bit indexed |
| 2 | Uncompressed True-Color | **24/32-bit RGB/RGBA** |
| 3 | Uncompressed Grayscale | 8/16-bit luminance |
| 9 | RLE Color-Mapped | Compressed indexed |
| 10 | RLE True-Color | **Compressed 24/32-bit RGB/RGBA** |
| 11 | RLE Grayscale | Compressed luminance |

**Focus for Generals**: Types **2** (uncompressed RGB/RGBA) and **10** (RLE RGB/RGBA)

## Image Descriptor Flags (Byte 17)

```
Bits 0-3: Alpha channel depth (0=no alpha, 8=8-bit alpha)
Bits 4-5: Pixel order (origin)
          00 = bottom-left (default)
          01 = bottom-right
          10 = top-left
          11 = top-right
Bits 6-7: Reserved (must be 0)
```

**Origin Flag**:

```cpp
#define TGA_ORIGIN_MASK      0x30
#define TGA_ORIGIN_BOTTOM    0x00  // Bottom-left (flip Y)
#define TGA_ORIGIN_TOP       0x20  // Top-left (no flip)
```

## Pixel Formats

### 24-bit True-Color (Type 2/10)

**Layout**: BGR (Blue, Green, Red) - **not RGB!**

```
+------+------+------+
| Blue | Green| Red  |  3 bytes per pixel
+------+------+------+
```

**Conversion to RGBA8**:

```cpp
uint8_t* rgba = new uint8_t[width * height * 4];
for (int i = 0; i < width * height; i++) {
    rgba[i*4 + 0] = bgr_data[i*3 + 2]; // R
    rgba[i*4 + 1] = bgr_data[i*3 + 1]; // G
    rgba[i*4 + 2] = bgr_data[i*3 + 0]; // B
    rgba[i*4 + 3] = 255;                // A (opaque)
}
```

### 32-bit True-Color (Type 2/10)

**Layout**: BGRA (Blue, Green, Red, Alpha)

```
+------+------+------+-------+
| Blue | Green| Red  | Alpha |  4 bytes per pixel
+------+------+------+-------+
```

**Conversion to RGBA8**:

```cpp
uint8_t* rgba = new uint8_t[width * height * 4];
for (int i = 0; i < width * height; i++) {
    rgba[i*4 + 0] = bgra_data[i*4 + 2]; // R
    rgba[i*4 + 1] = bgra_data[i*4 + 1]; // G
    rgba[i*4 + 2] = bgra_data[i*4 + 0]; // B
    rgba[i*4 + 3] = bgra_data[i*4 + 3]; // A
}
```

## RLE Compression (Type 10)

**Run-Length Encoding**: Reduces file size by encoding repeated pixels.

### Packet Structure

Each packet consists of:

```
+----------------+
| Header (1 B)   |  Bit 7: type, Bits 0-6: count
+----------------+
| Pixel Data     |  1 pixel (RLE) or N pixels (raw)
+----------------+
```

**Header Byte**:

```
Bit 7 (MSB):
  0 = Raw packet (next N+1 pixels are uncompressed)
  1 = RLE packet (repeat next pixel N+1 times)
  
Bits 0-6:
  Count-1 (0-127, actual count is 1-128)
```

### RLE Decompression Algorithm

```cpp
uint8_t* DecompressRLE(FILE* file, int width, int height, int bytes_per_pixel) {
    uint8_t* output = new uint8_t[width * height * bytes_per_pixel];
    int output_index = 0;
    int total_pixels = width * height;
    
    while (output_index < total_pixels) {
        uint8_t packet_header;
        fread(&packet_header, 1, 1, file);
        
        int count = (packet_header & 0x7F) + 1; // Extract count (1-128)
        
        if (packet_header & 0x80) {
            // RLE packet: repeat next pixel 'count' times
            uint8_t pixel[4];
            fread(pixel, bytes_per_pixel, 1, file);
            
            for (int i = 0; i < count; i++) {
                memcpy(output + output_index * bytes_per_pixel, pixel, bytes_per_pixel);
                output_index++;
            }
        } else {
            // Raw packet: read 'count' pixels verbatim
            fread(output + output_index * bytes_per_pixel, bytes_per_pixel, count, file);
            output_index += count;
        }
    }
    
    return output;
}
```

### Example

**Input** (10 bytes):

```
81 FF 00 00  // Header: 0x81 (RLE, count=2), Pixel: 0x0000FF (blue)
02 AA BB CC DD EE FF  // Header: 0x02 (raw, count=3), 3 pixels
```

**Output** (15 bytes):

```
FF 00 00  FF 00 00  FF 00 00  // Blue repeated 3 times (count=0x81 & 0x7F + 1 = 2+1 = 3)
AA BB CC  DD EE FF              // 3 raw pixels
```

## Vertical Flip

TGA files may have origin at **bottom-left** or **top-left**. OpenGL expects **top-left**.

```cpp
bool IsBottomOrigin(uint8_t imageDescriptor) {
    return (imageDescriptor & TGA_ORIGIN_MASK) == TGA_ORIGIN_BOTTOM;
}

void FlipVertical(uint8_t* data, int width, int height, int bytes_per_pixel) {
    int row_size = width * bytes_per_pixel;
    uint8_t* temp_row = new uint8_t[row_size];
    
    for (int y = 0; y < height / 2; y++) {
        uint8_t* row1 = data + y * row_size;
        uint8_t* row2 = data + (height - 1 - y) * row_size;
        
        memcpy(temp_row, row1, row_size);
        memcpy(row1, row2, row_size);
        memcpy(row2, temp_row, row_size);
    }
    
    delete[] temp_row;
}
```

## Validation Checklist

- [ ] `imageType` is 2 or 10 (true-color)
- [ ] `pixelDepth` is 24 or 32
- [ ] `width` and `height` > 0 and < 65536
- [ ] `colorMapType` == 0 (no color map for true-color)
- [ ] File size >= 18 + (width * height * bytes_per_pixel)

## Complete Loading Algorithm

```cpp
struct TGAData {
    int width;
    int height;
    uint8_t* rgba_data; // RGBA8 format
};

TGAData* LoadTGA(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return nullptr;
    
    // 1. Read header
    TGAHeader header;
    fread(&header, sizeof(TGAHeader), 1, file);
    
    // 2. Validate
    if ((header.imageType != 2 && header.imageType != 10) ||
        (header.pixelDepth != 24 && header.pixelDepth != 32)) {
        fclose(file);
        return nullptr;
    }
    
    // 3. Skip image ID
    if (header.idLength > 0) {
        fseek(file, header.idLength, SEEK_CUR);
    }
    
    // 4. Read pixel data
    int bytes_per_pixel = header.pixelDepth / 8;
    uint8_t* raw_data;
    
    if (header.imageType == 10) {
        // RLE compressed
        raw_data = DecompressRLE(file, header.width, header.height, bytes_per_pixel);
    } else {
        // Uncompressed
        int data_size = header.width * header.height * bytes_per_pixel;
        raw_data = new uint8_t[data_size];
        fread(raw_data, 1, data_size, file);
    }
    
    fclose(file);
    
    // 5. Convert BGR(A) to RGBA
    TGAData* result = new TGAData;
    result->width = header.width;
    result->height = header.height;
    result->rgba_data = new uint8_t[header.width * header.height * 4];
    
    for (int i = 0; i < header.width * header.height; i++) {
        result->rgba_data[i*4 + 0] = raw_data[i*bytes_per_pixel + 2]; // R
        result->rgba_data[i*4 + 1] = raw_data[i*bytes_per_pixel + 1]; // G
        result->rgba_data[i*4 + 2] = raw_data[i*bytes_per_pixel + 0]; // B
        result->rgba_data[i*4 + 3] = (bytes_per_pixel == 4) ? raw_data[i*bytes_per_pixel + 3] : 255; // A
    }
    
    delete[] raw_data;
    
    // 6. Flip if bottom-origin
    if (IsBottomOrigin(header.imageDescriptor)) {
        FlipVertical(result->rgba_data, header.width, header.height, 4);
    }
    
    return result;
}
```

## Font Atlas Example

Generals fonts are typically stored as TGA with:

- **Format**: 32-bit RGBA (alpha channel for anti-aliasing)
- **Compression**: Usually RLE to reduce file size
- **Layout**: Grid of glyphs (e.g., 16x16 characters)
- **Usage**: Sample texture coordinates for each character

**Example**: `Arial12Bold.tga`

```
+---+---+---+---+---+---+---+---+
| ! | " | # | $ | % | & | ' | ( |  8 chars wide
+---+---+---+---+---+---+---+---+
| ) | * | + | , | - | . | / | 0 |
+---+---+---+---+---+---+---+---+
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
+---+---+---+---+---+---+---+---+
...
```

**UV Calculation**:

```cpp
char c = 'A'; // ASCII 65
int chars_per_row = 16;
int char_width = 16;
int char_height = 16;

int col = (c - 32) % chars_per_row;
int row = (c - 32) / chars_per_row;

float u0 = (col * char_width) / (float)atlas_width;
float v0 = (row * char_height) / (float)atlas_height;
float u1 = u0 + (char_width / (float)atlas_width);
float v1 = v0 + (char_height / (float)atlas_height);
```

## References

- [TGA Format Specification (Paul Bourke)](http://www.paulbourke.net/dataformats/tga/)
- [Truevision TGA File Format](https://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf)
- [RLE Compression Explanation](https://en.wikipedia.org/wiki/Run-length_encoding)
