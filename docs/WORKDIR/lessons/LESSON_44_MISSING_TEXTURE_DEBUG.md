# Lesson: Missing Textures Map to W3D Missing Texture Sentinel Color

**Session**: 44  
**Date**: 2026-02-19  
**Status**: Root cause identified, fix pending

---

## Problem

The Linux build main menu shows a fully magenta background. All terrain, sky, water, and some UI elements render as solid magenta.

## Root Cause

W3D engine has a built-in "missing texture" placeholder at `Core/Libraries/Source/WWVegas/WW3D2/missingtexture.cpp:97`:
```cpp
*buffer++ = 0x7FFF00FF;  // ARGB: A=127, R=255, G=0, B=255 = semi-transparent magenta
```

Any failed texture load call chain ends at `TextureLoadTaskClass::Apply_Missing_Texture()`, which fills the texture slot with this 128×128 magenta tile. At render scale, this becomes a solid magenta fill.

## Debugging Approach That Worked

1. Add `fprintf(stderr, "[TEX_MISSING] '%s'\n", ...)` to `Apply_Missing_Texture()` guarded by `#ifndef _WIN32`.
2. Run the game for ~18 sec, redirect stderr to file, `grep TEX_MISSING`.
3. Cross-reference each failing filename with BIG archive contents using Python struct parsing.

## What Is NOT the Issue

- DXVK Vulkan errors (d3d8.log is clean beyond patchsegments warning)
- Path separator mismatch (`\` vs `/`) — `ArchiveFileSystem` tokenizes with `"\\"` which covers both
- DDSFileClass not swapping extension — it already does `.tga` → `.dds` in the constructor
- Cross-extension fallback missing — `W3DFileSystem` already has `.tga` ↔ `.dds` fallback

## Investigation Technique: Python BIG Archive Parser

Quick one-liner to search BIG archives without extraction:
```python
import struct, sys

def search_big(path, needle):
    with open(path, 'rb') as f:
        magic = f.read(4)
        assert magic == b'BIGF'
        size = struct.unpack('<I', f.read(4))[0]
        num_files = struct.unpack('>I', f.read(4))[0]
        struct.unpack('>I', f.read(4))  # entry_offset
        for _ in range(num_files):
            offset = struct.unpack('>I', f.read(4))[0]
            fsize  = struct.unpack('>I', f.read(4))[0]
            name   = b''; c = f.read(1)
            while c != b'\x00': name += c; c = f.read(1)
            name = name.decode('latin-1', errors='replace')
            if needle.lower() in name.lower():
                print(f"  {name}  (offset={offset}, size={fsize})")

for bigfile in sys.argv[2:]:
    print(f"--- {bigfile} ---")
    search_big(bigfile, sys.argv[1])
```

Usage:
```bash
python3 search_big.py "trcobble" ~/GeneralsX/GeneralsMD/*.big
```

## Key BIG Archive Contents

| Archive | Path inside BIG | Note |
|---|---|---|
| `TexturesZH.big` | `Art\Textures\trcobblestones.dds` | Road textures stored as DDS |
| `TexturesZH.big` | `Art\Textures\mainmenuruleruserinterface.tga` | UI textures as TGA |
| `TerrainZH.big` | `Art\Terrain\PTBlossom01.tga` | Terrain tiles |
| `BrazilianZH.big` | `Data\Brazilian\generals.csf` | Localization |
| `noise0000.tga` | **NOT IN ANY ZH BIG** | May be in base Generals `Terrain.big` |
| `tscloud*.tga` | **NOT IN ANY ZH BIG** | Sky textures may be Generals-only BIG |

## Next Actions

1. Add logging before/after `loadBigFilesFromDirectory("", "*.big")` in `StdBIGFileSystem::init()` to confirm ZH BIG files load at startup.
2. Investigate why `doesFileExist("Art/Textures/trcobblestones.dds")` fails for a file confirmed to exist in `TexturesZH.big`.
3. Two candidate fixes:
   - Fix CWD issue: `loadBigFilesFromDirectory("", "*.big")` may scan the wrong directory if CWD != game dir at init time.
   - Fix `_TheFileFactory`: Ensure BIG-backed file factory is active when texture loading starts.

## Diagnostic Code Status

| File | Change | Remove when |
|---|---|---|
| `W3DDisplay.cpp:1886` | Clear color = green `Vector3(0,1,0)` | **Immediately** — revert to `(0,0,0)` |
| `textureloader.cpp:1274` | `[TEX_MISSING]` stderr logging | After fix is confirmed |
| `StdBIGFileSystem.cpp:59` | `[BIG]` logging on Generals BIG load | Keep until ZH BIG logging added |
