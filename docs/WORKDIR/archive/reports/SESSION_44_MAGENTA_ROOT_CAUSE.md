# Session 44 — Magenta Background Root Cause Identified

**Date**: 2026-02-19  
**Phase**: 1.8 — Linux Asset Loading / Rendering Investigation  
**Status**: Root cause found, fix NOT yet applied. Diagnostic code still in tree.

---

## Session Goals

- Determine the root cause of the persistent magenta background on the main menu shell map
- Confirm previous fixes still hold (`-win` windowed mode, exit segfault)

---

## What Was Done

### 1. Diagnostic Test: Green Clear Color + Skip drawViews()

Modified `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp`:
- Changed `Begin_Render` clear color `Vector3(0,0,0)` → `Vector3(0,1,0)` (bright green)
- Commented out `drawViews()` call

**Status at session end**: `drawViews()` has been **restored**. The green clear color is **still active**.  
Both changes are marked with `// DIAG:` comments.

**Key finding**: `MissingTexture::_Get_Missing_Texture()` returns a texture filled with solid color `0x7FFF00FF` — **semi-transparent magenta** (A=127, R=255, G=0, B=255). ANY texture that fails to load renders as magenta. The background magenta = terrain textures failing.

### 2. Texture Failure Logging Added

Added `fprintf(stderr, "[TEX_MISSING] ...")` to `Apply_Missing_Texture()` in:
- `Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp` (line ~1274, guarded by `#ifndef _WIN32`)

**This log is still active** and should be kept until the fix is validated.

### 3. Missing Texture Log Analysis

Ran the game for ~18 seconds with `drawViews()` active and captured `/tmp/tex_full.log`.

**ALL of these textures failed to load**:
- Road: `trcobblestones.tga`, `trcracks.tga`, `trtwolane*.tga`, `trsidewalk*.tga` (68+ road textures)
- Sky: `tscloudwis.tga`, `tscloudsun.tga`, `tsstarfeld.tga`, `tsmorning*.tga`
- Water: `twwater01.tga`, `twalphaedge.tga`, `watersurfacebubbles.tga`
- Noise: `noise0000.tga`
- UI: `mainmenubackdropuserinterface.tga`, `titlescreenuserinterface.tga`, `mainmenuruleruserinterface.tga`

This explains the fully magenta screen — essentially all visible geometry has missing textures.

### 4. BIG Archive Path Investigation

Inspected `TexturesZH.big` and `TerrainZH.big` using Python struct parsing.

**Key discoveries**:

| BIG Archive | Files stored as | Example |
|---|---|---|
| `TerrainZH.big` | `Art\Terrain\*.tga` | `Art\Terrain\PTBlossom01.tga` |
| `TexturesZH.big` | `Art\Textures\*.dds` | `Art\Textures\trcobblestones.dds` |
| `TexturesZH.big` | `Art\Textures\*.tga` | `Art\Textures\mainmenuruleruserinterface.tga` |

**Critical insight**: Many textures are stored **as `.dds`** in the archives but the W3D mesh materials reference them **as `.tga`**.

Example: `trcobblestones.tga` (requested) vs `Art\Textures\trcobblestones.dds` (what exists in BIG).

### 5. DDSFileClass Already Has Extension Swap

`ddsfile.cpp` constructor already converts any `.tga` extension to `.dds`:
```cpp
// The name could be given in .tga or .dds format, so ensure we're opening .dds...
int len=strlen(Name);
Name[len-3]='d'; Name[len-2]='d'; Name[len-1]='s';
```

### 6. Cross-Extension Fallback in W3DFileSystem Already Exists

`W3DFileSystem.cpp` line 322 already has a cross-extension fallback from Session X:
```cpp
// GeneralsX @bugfix BenderAI 18/02/2026 - Cross-extension fallback for TGA/DDS mismatches.
```
It converts `.tga` → `.dds` if the `.tga` lookup fails, and retries.

### 7. What Happens in Practice (Call Chain)

For texture `trcobblestones.tga`:

1. `TextureLoadTaskClass::Begin_Load()` called
2. First: `Begin_Compressed_Load()` → `Get_Texture_Information("trcobblestones.tga", compressed=true)`
3. `DDSFileClass dds_file("trcobblestones.tga", 0)` — constructor swaps → opens `trcobblestones.dds`
4. `file_auto_ptr file(_TheFileFactory, "trcobblestones.dds")` → creates `GameFileClass("trcobblestones.dds")`
5. `GameFileClass::Set_Name("trcobblestones.dds")` tries:
   - `Data/Brazilian/Art/Textures/trcobblestones.dds` → NOT FOUND
   - `Art/Textures/trcobblestones.dds` → **SHOULD FIND** in `TexturesZH.big`
6. Still fails → `Apply_Missing_Texture` → magenta

**The mystery**: Step 5 should work because `TexturesZH.big` contains `Art\Textures\trcobblestones.dds`. The `ArchiveFileSystem::doesFileExist` should find it because the path tokenizer handles both `\` and `/`.

**Blocked on**: Confirming whether `TexturesZH.big` is actually loaded at init time (no explicit log for the first `loadBigFilesFromDirectory("", "*.big")` call).

### 8. CSF Loading Confirms File System Is Partially Working

The CSF file loads from `data/brazilian/generals.csf` which is in `BrazilianZH.big` → proves that at least one ZH BIG file is loaded and accessible.

---

## Root Cause Hypothesis

**Primary hypothesis**: The `loadBigFilesFromDirectory("", "*.big")` call in `StdBIGFileSystem::init()` IS loading the BIG files (since CSF works). However, the `DDSFileClass` path resolution chain may have a subtle bug:

- `_TheFileFactory` at texture load time might NOT be the `W3DFileSystem` (which routes through BIG archives), but instead the `_DefaultFileFactory` (which only looks at disk).
- This would cause `GameFileClass::Set_Name()` to fail for anything not on disk.
- The CSF file loads through `TheFileSystem->openFile()` directly, not through `_TheFileFactory`, so it works regardless.

**Secondary hypothesis**: `StdBIGFileSystem::loadBigFilesFromDirectory("", "*.big")` fails silently on Linux if the current working directory at init time is not the game directory (i.e., when games are launched via the `run.sh` wrapper from a different CWD).

---

## What Is NOT the Root Cause

- ❌ DXVK/Vulkan rendering errors (d3d8.log shows ONLY D3DRS_PATCHSEGMENTS warnings)
- ❌ Wrong clear color (WW3D clear is correctly opaque black `0xFF000000`, confirmed by `m_minWaterOpacity=1.0f`)
- ❌ Missing pixel/vertex shaders (11/11 load OK: terrain.pso, terrainnoise.pso, etc.)
- ❌ Window creation/transparency (windowed mode confirmed working)
- ❌ Path separator mismatch (`\` vs `/`) — the BIG file tokenizer handles both
- ❌ `mainmenubackdropuserinterface.tga` doesn't exist — it IS in `TexturesZH.big` at `Art\Textures\mainmenuruleruserinterface.tga`

---

## Files Modified This Session

### Active Diagnostic Changes (must be reverted before final build):

| File | Change | Status |
|---|---|---|
| `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` | Clear color changed to green `Vector3(0,1,0)` | **MUST REVERT** |
| `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp` | `drawViews()` restored (was skipped, now restored) | Already reverted ✅ |
| `Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp` | `[TEX_MISSING]` logging in `Apply_Missing_Texture` | Keep for now for debugging |

### Code Changes to Keep:
The `[TEX_MISSING]` logging is valuable — leave it until the fix is confirmed.

---

## Next Steps

### Step 1: Investigate `_TheFileFactory` at Texture Load Time

Add a debug log in `W3DFileSystem.cpp` to confirm the factory is set during texture loading:

```cpp
// In GameFileClass::Set_Name(), add temporarily:
fprintf(stderr, "[FILE_LOOKUP] Set_Name('%s') -> trying path '%s'\n", filename, m_filePath);
```

This will show whether the path lookup is even reaching the BIG archives.

### Step 2: Confirm `loadBigFilesFromDirectory` Loads ZH BIG Files

Add logging to `StdBIGFileSystem::init()` before the first call:
```cpp
fprintf(stderr, "[BIG] Loading ZH BIG files from CWD...\n");
loadBigFilesFromDirectory("", "*.big");
fprintf(stderr, "[BIG] ZH BIG files load done\n");
```

This confirms whether `TexturesZH.big` is even registered.

### Step 3: Revert Diagnostic Changes in W3DDisplay.cpp

Before shipping the fix, revert the clear color back to `Vector3(0,0,0)`:

```cpp
// Line ~1886 in GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp
// Remove the DIAG comment and change (0,1,0) back to (0,0,0)
WW3D::Begin_Render( true, true, Vector3( 0.0f, 0.0f, 0.0f ), TheWaterTransparency->m_minWaterOpacity )
```

### Step 4: Fix the Root Cause

Once confirmed, the likely fix is one of:
- Ensure `_TheFileFactory` is set to the W3DFileSystem adapter before texture loading starts
- Add `fprintf` debug to `StdBIGFileSystem::loadBigFilesFromDirectory` to confirm CWD and file count
- OR: Add an explicit `getFileListInDirectory` log to dump loaded BIG file names

### Step 5: Visual Confirmation

After fix, run the game and confirm the background is NOT magenta. Expected result:
- Shell map terrain renders correctly (sand/grass tiles)
- Sky visible (cloud texture or gradient)
- Main menu UI backdrop appears

---

## Key Code Locations

| What | Where |
|---|---|
| Missing texture color `0x7FFF00FF` | `Core/Libraries/Source/WWVegas/WW3D2/missingtexture.cpp:97` |
| `Apply_Missing_Texture` + log | `Core/Libraries/Source/WWVegas/WW3D2/textureloader.cpp:1274` |
| DDSFileClass extension swap | `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ddsfile.cpp:58-63` |
| Cross-extension fallback | `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DFileSystem.cpp:322` |
| BIG file init (ZH load) | `Core/GameEngineDevice/Source/StdDevice/Common/StdBIGFileSystem.cpp:53-87` |
| Path tokenizer (`\` and `/`) | `Core/GameEngine/Source/Common/System/ArchiveFileSystem.cpp:259-285` |
| Texture path lookup chain | `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DFileSystem.cpp:143-370` |
| W3DDisplay draw loop (DIAG) | `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp:1886` |

---

## Build State

- **Binary**: `~/GeneralsX/GeneralsMD/GeneralsXZH` (deployed, 185MB, Feb 19 20:12)
- **Config**: `linux64-deploy`, DXVK 2.6.0, AMD RADV POLARIS12
- **Has diag changes**: yes (green clear color, `[TEX_MISSING]` logging)
