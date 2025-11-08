# Phase 50: Compatibility Layers Modularization

**Date**: 7 de novembro de 2025  
**Status**: 🟡 IN-PROGRESS (Architecture complete, cleanup ongoing)  
**Target**: Clean separation of compatibility concerns for cross-platform compilation

## Overview

Phase 50 refactored the monolithic Windows compatibility layer (2,380 lines in `win32_compat.h`) into focused, modular headers. This improves maintainability, reduces header coupling, and clarifies Phase 50 vs Phase 51 responsibilities.

## New Architecture

### DirectX 8 Graphics (`d3d8.h` ecosystem)

**Master Header**: `d3d8.h` (27 lines)  
Umbrella that includes:

1. **`d3d8_graphics_compat.h`** (650+ lines) ✅ COMPLETE
   - **Purpose**: All DirectX 8 graphics types for Phase 50
   - **Content**:
     - Forward declarations: `IDirect3DDevice8`, `IDirect3DTexture8`, `IDirect3DSurface8`, etc.
     - `IDirect3DDevice8` stub interface with 10+ virtual methods (SetVertexShader, SetPixelShader, SetRenderState, etc.)
     - Enums: `D3DFORMAT` (16 formats), `D3DPOOL`, `D3DTRANSFORMSTATETYPE`, `D3DRENDERSTATETYPE` (33 states), `D3DTEXTURESTAGESTATETYPE`, `D3DLIGHTTYPE`, `D3DPRIMITIVETYPE`, `D3DCULL`, `D3DCMPFUNC`, `D3DBLEND`, `D3DFILLMODE`
     - Structs: `D3DVECTOR`, `D3DMATRIX`, `D3DCOLORVALUE`, `D3DLIGHT8`, `D3DCAPS8`, `D3DADAPTER_IDENTIFIER8`, `D3DDISPLAYMODE`, `D3DVIEWPORT8`, `D3DMATERIAL8`, `D3DLOCKED_RECT`, `D3DLOCKED_BOX`
     - Constants & Macros: `D3D_OK`, `D3DFVF_*` (FVF macros), `D3DCOLOR_ARGB()`, `D3DDP_MAXTEXCOORD=8`
   - **Dependencies**: `win32_compat_core.h` (for DWORD, LARGE_INTEGER, GUID)
   - **Includes**: `#include "win32_compat_core.h"` at top

2. **`d3d8_audio_compat.h`** (290+ lines) ⏳ PHASE 51 DEFERRED
   - **Purpose**: DirectSound→OpenAL audio mapping (not needed for Phase 50)
   - **Currently**: Disabled in `d3d8.h` with comment marker
   - **Content**:
     - DirectSound structures: `WAVEFORMATEX`, `DSBUFFERDESC`, `DS3DBUFFER`, `DS3DLISTENER`
     - Forward declarations: `IDirectSound8`, `IDirectSoundBuffer8`, `IDirectSound3DListener8`
     - Constants: `DS_OK`, `DSBCAPS_*`, `WAVE_FORMAT_*`
     - Future implementation notes for OpenAL mapping
   - **Phase 51 Work**: Map IDirectSound8 methods to OpenAL calls, implement 3D audio

### Windows API Core (`win32_compat_core.h`)

**Purpose**: Core Windows types used across all compatibility layers  
**Lines**: 300+  
**Content**:
- Integer types: `LONG`, `DWORD`, `WORD`, `BYTE`, `UINT`, `SHORT`, `CHAR`, `INT`, `FLOAT`, `DOUBLE`, `BOOL`, `__int64`, `SIZE_T`
- Pointer types: `LPVOID`, `LPCVOID`, `LPSTR`, `LPCSTR`, `LPWSTR`, `LPCWSTR`
- Windows object handles: `HANDLE`, `HMODULE`, `HINSTANCE`, `HWND`, `HDC`, `HKEY`, `HPEN`, `HBRUSH`, `HFONT`, `HBITMAP`, `HICON`, `HCURSOR`, `HMENU`
- Callback types: `FARPROC`, `WPARAM`, `LPARAM`, `LRESULT`
- Structures: `LARGE_INTEGER`, `ULARGE_INTEGER`, `GUID`
- Boolean constants: `TRUE=1`, `FALSE=0` (CRITICAL: defined BEFORE all usage)
- Calling conventions: `__cdecl`, `__stdcall`, `__fastcall` (no-ops on non-Windows)
- HRESULT codes: `S_OK`, `S_FALSE`, `E_FAIL`, `E_INVALIDARG`, `E_OUTOFMEMORY`, `E_NOTIMPL`

**Synced to**: Both Core and GeneralsMD

### Future Modular Headers (Planned for later refactoring)

These extract functionality from current `win32_compat.h`:

- **`win32_compat_file.h`** (TODO)
  - File I/O: CreateFile, ReadFile, WriteFile, CopyFile
  - File discovery: FindFirstFile, FindNextFile, FindClose
  - Path operations: GetCurrentDirectory, SetCurrentDirectory, GetFileAttributes
  - Structures: WIN32_FIND_DATA

- **`win32_compat_memory.h`** (TODO)
  - Memory allocation: GlobalAlloc, LocalAlloc, malloc stubs
  - Memory status: GlobalMemoryStatus, MEMORYSTATUS struct
  - Memory utilities: GetProcessHeap

- **`win32_compat_registry.h`** (TODO)
  - Registry API: RegOpenKey, RegQueryValue, RegSetValue, RegDeleteValue, RegCloseKey
  - Registry types: HKEY, DWORD, REG_DWORD, REG_SZ
  - Registry constants: HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.

- **`win32_compat_ui.h`** (TODO)
  - Window creation: CreateWindow, CreateDialog, ShowWindow, DestroyWindow
  - Message boxes: MessageBox, MessageBoxEx
  - UI events: CreateEvent, SetEvent, WaitForSingleObject
  - Structures: WNDCLASS, HWND, PAINTSTRUCT
  - Constants: WS_VISIBLE, CW_USEDEFAULT, SW_SHOW, SW_HIDE

## Synchronization Status

All modular headers synced to both locations:
- ✅ `Core/Libraries/Source/WWVegas/WW3D2/` (source of truth)
- ✅ `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/` (synced copy)

## Fixes Applied in Phase 50

### Path Issues
- **Fixed**: `pointerremap.h:40` - Changed `#include "osdep/osdep.h"` → `#include "../WWMath/osdep.h"`
- **Root Cause**: Incorrect relative path in POSIX-only include guard

### Platform-Specific Headers
- **malloc.h**:  `Core/Libraries/Source/WWVegas/WWLib/FastAllocator.h:42`
  ```cpp
  #ifdef __MACH__
  #include <stdlib.h>  // macOS provides malloc via stdlib.h
  #else
  #include <malloc.h>  // Linux/Unix
  #endif
  ```

### Missing Enumerations
- **Added**: `D3DDP_MAXTEXCOORD = 8` (DirectX 8 max texture coordinate sets)
- **Added**: `D3DRS_FOGSTART`, `D3DRS_FOGEND` render states (were missing from original enum)

### IDirect3DDevice8 Stub
Created stub interface with 10+ virtual methods to allow header compilation:
```cpp
struct IDirect3DDevice8 {
    virtual int SetVertexShader(unsigned long shader) { return 0; }
    virtual int SetPixelShader(unsigned long shader) { return 0; }
    virtual int SetVertexShaderConstant(int reg, const void* data, int count) { return 0; }
    virtual int SetPixelShaderConstant(int reg, const void* data, int count) { return 0; }
    virtual int SetTransform(unsigned long state, const D3DMATRIX* matrix) { return 0; }
    virtual int GetTransform(unsigned long state, D3DMATRIX* matrix) { return 0; }
    virtual int SetMaterial(const D3DMATERIAL8* material) { return 0; }
    virtual int SetRenderState(unsigned long state, unsigned long value) { return 0; }
    virtual int GetRenderState(unsigned long state, unsigned long* value) { return 0; }
    virtual int SetTextureStageState(unsigned long stage, unsigned long state, unsigned long value) { return 0; }
    virtual int GetTextureStageState(unsigned long stage, unsigned long state, unsigned long* value) { return 0; }
    virtual int Release() { return 0; }
};
```

**Note**: Full implementation will be provided by Vulkan backend in Phase 52+

## Current Compilation Status

**Build Progress**: 22 compilation errors (down from 400+)

### Remaining Issues (Cleanup Phase)

The original `win32_compat.h` (2,380 lines) contains overlapping definitions from multiple integration attempts. Cleanup needed:

1. **Typedef Redefinitions**
   - `FARPROC`: conflict between `void*` and `void (*)()`
   - `LARGE_INTEGER`: conflict between union and struct
   - `GUID`: multiple struct definitions
   - `D3DLOCKED_BOX`: struct vs typedef conflict

2. **Missing DirectX Device Methods**
   - `SetLight()`, `LightEnable()` (lighting)
   - `SetClipPlane()` (clip planes)
   - `SetTexture()` (texture binding)
   - `CopyRects()` (surface copy)
   - Others (SetVertexBuffer, SetIndexBuffer, etc.)

3. **Type Confusion**
   - `D3DXMATRIX` vs `D3DMATRIX` (math extensions)
   - Incomplete types: `IDirect3DBaseTexture8`

### Solution Strategy

Rather than trying to patch `win32_compat.h`, the cleaner approach is:

**Phase 50.x** (Next iteration):
1. Extract non-overlapping sections into focused headers
2. Remove duplicates from `win32_compat.h`
3. Convert to minimal umbrella that includes modular headers
4. Resolve remaining DirectX method stubs

**Rationale**: The current 2,380-line monolith was created by merging multiple sources (fbraz3, jmarshall-win64-modern, etc.). Modularization naturally identifies and resolves duplicates.

## Files Modified

### Core (Source of Truth)
- `Core/Libraries/Source/WWVegas/WW3D2/d3d8.h` - Master DirectX 8 header
- `Core/Libraries/Source/WWVegas/WW3D2/d3d8_graphics_compat.h` - Graphics types (NEW)
- `Core/Libraries/Source/WWVegas/WW3D2/d3d8_audio_compat.h` - Audio stubs (NEW, Phase 51)
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat_core.h` - Core types (NEW)
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h` - Master Windows API header (refactored)
- `Core/Libraries/Source/WWVegas/WWLib/FastAllocator.h` - Fixed malloc.h conditional
- `Core/Libraries/Source/WWVegas/WWSaveLoad/pointerremap.h` - Fixed osdep.h include

### GeneralsMD (Synced Copies)
- All headers copied to `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/`

## Build Commands

```bash
# Configure (creates build/macos-arm64/)
cmake --preset macos-arm64

# Build z_generals (Zero Hour - PRIMARY TARGET)
cmake --build build/macos-arm64 --target z_generals -j 4 2>&1 | tee logs/build.log

# Monitor progress
tail -f logs/build.log | grep -E "error:|Built target"
```

## Phase 50 Completion Criteria

- [x] Remove all graphics backend conditionals (OpenGL/Metal/DXVK)
- [x] Create modular compatibility layer structure
- [ ] Resolve all 22 remaining compilation errors
- [ ] Achieve successful z_generals binary linking
- [ ] Verify Vulkan initialization (runtime test)
- [ ] Document Phase 50 completion and commit

## Next Steps

1. **Resolve Typedef Conflicts** (Priority 1)
   - Review win32_compat.h lines with redefinition errors
   - Keep best/cleanest definition, remove duplicates

2. **Add Missing DirectX Device Methods** (Priority 2)
   - Extend IDirect3DDevice8 stub with remaining methods
   - Methods needed: SetLight, LightEnable, SetClipPlane, SetTexture, CopyRects, SetVertexBuffer, SetIndexBuffer, DrawPrimitive, etc.

3. **Resolve Type Confusion** (Priority 3)
   - Fix D3DXMATRIX vs D3DMATRIX
   - Ensure complete type definitions for all DirectX structs

4. **Continue Modularization** (Priority 4)
   - Extract file I/O, memory, registry, UI into separate headers
   - Reduce win32_compat.h footprint further

## Reference

- **Phase 50 Architecture**: Vulkan-only graphics, platform-conditional compilation (not backend-conditional)
- **Modularization Pattern**: Each header focused on single domain (graphics, audio, file I/O, etc.)
- **Syncing Strategy**: Core is source of truth, GeneralsMD gets copies for build consistency
- **Future Audio**: Phase 51 will implement DirectSound→OpenAL mapping using d3d8_audio_compat.h stubs

---

**Author**: Phase 50 Refactor Session  
**Updated**: 7 de novembro de 2025
