# Phase 43.1: Texture & Surface System via Abstraction (CORRECTED APPROACH)

**Objective**: Resolve 25 undefined linker symbols related to Texture and Surface systems

**Scope**: TextureClass, SurfaceClass, TextureFilterClass implementations

**Expected Duration**: 1-2 days

**Success Criteria**:
- All 25 texture/surface symbols resolved  
- Compilation succeeds (linker errors reduced by 25)
- Symbol count: 130 → 105
- No stubs, real Vulkan/Metal implementations

---

## Strategic Correction

### Initial Approach (Failed)
- Try to compile surfaceclass.cpp, texture.cpp, texturefilter.cpp as-is
- Result: DirectX API incompatibilities (D3DXLoadSurfaceFromSurface not available)
- Conclusion: These files cannot be compiled without massive DirectX rewrites

### Correct Approach  
- Implement Texture/Surface systems through Vulkan graphics driver abstraction
- Create VulkanTextureManager, VulkanSurfaceManager implementations
- Leverage existing DX8Wrapper pattern for game code compatibility
- Provides cross-platform solution (Vulkan + Metal via MoltenVK)

---

## Symbols to Resolve

### Category 1: TextureClass (8 symbols)
1. `TextureClass::TextureClass(unsigned width, unsigned height, ...)`
2. `TextureClass::TextureClass(const char* name, ...)`
3. `TextureClass::TextureClass(SurfaceClass*, ...)`
4. `TextureClass::Init()`
5. `TextureClass::Apply_New_Surface(IDirect3DBaseTexture8*)`
6. `TextureClass::Get_Surface_Level(unsigned int)`
7. `TextureClass::Get_Level_Description(...)`
8. `TextureClass::Get_Texture_Format() const`

### Category 2: TextureBaseClass (4 symbols)
1. `TextureBaseClass::~TextureBaseClass()` - Virtual destructor
2. `TextureBaseClass::Invalidate()` - Force reload
3. `TextureBaseClass::Set_Texture_Name(const char*)`
4. `TextureBaseClass::Set_D3D_Base_Texture(IDirect3DBaseTexture8*)`

### Category 3: SurfaceClass (7 symbols)
1. `SurfaceClass::SurfaceClass(IDirect3DSurface8*)`
2. `SurfaceClass::SurfaceClass(const char* filename)`
3. `SurfaceClass::SurfaceClass(unsigned width, unsigned height, WW3DFormat)`
4. `SurfaceClass::Get_Description(SurfaceDescription&)`
5. `SurfaceClass::Lock(int*)`
6. `SurfaceClass::Unlock()`
7. `SurfaceClass::Is_Transparent_Column(unsigned int)`

### Category 4: Utilities (6 symbols)
1. `Load_Texture(ChunkLoadClass&)` - Load from chunk
2. `PixelSize(SurfaceClass::SurfaceDescription const&)`
3. `TextureFilterClass::Set_Mip_Mapping(FilterType)`
4. `TextureFilterClass::_Init_Filters(TextureFilterMode)`
5. `TextureLoader::Flush_Pending_Load_Tasks()`
6. `TextureLoader::Request_Foreground_Loading(TextureBaseClass*)`

---

## Implementation Files

### File 1: Core/Libraries/Source/Graphics/vulkan_texture_manager.h

```cpp
#pragma once

#include "texture.h"
#include "surfaceclass.h"

class VulkanTextureManager {
public:
    // Texture creation
    static IDirect3DTexture8* CreateTexture(
        unsigned width, unsigned height, WW3DFormat format, 
        TextureBaseClass::MipCountType mips, TextureBaseClass::PoolType pool);
    
    static IDirect3DTexture8* CreateTextureFromFile(const char* filename);
    static IDirect3DTexture8* CreateTextureFromSurface(IDirect3DSurface8* surface);
    
    // Texture operations
    static void InvalidateTexture(IDirect3DTexture8* tex);
    static void SetTextureName(IDirect3DTexture8* tex, const char* name);
    static unsigned GetTextureMemoryUsage(IDirect3DTexture8* tex);
    
    // Mipmap filtering
    static void SetMipMapping(IDirect3DTexture8* tex, bool enabled);
};
```

### File 2: Core/Libraries/Source/Graphics/vulkan_surface_manager.h

```cpp
#pragma once

#include "surfaceclass.h"

class VulkanSurfaceManager {
public:
    // Surface creation
    static IDirect3DSurface8* CreateSurface(
        unsigned width, unsigned height, WW3DFormat format);
    
    static IDirect3DSurface8* CreateSurfaceFromFile(const char* filename);
    
    // Surface operations
    static void LockSurface(IDirect3DSurface8* surf, void*& pixelData, int& pitch);
    static void UnlockSurface(IDirect3DSurface8* surf);
    static void GetSurfaceDescription(
        IDirect3DSurface8* surf, SurfaceClass::SurfaceDescription& desc);
    
    // Pixel operations
    static bool IsTransparentColumn(IDirect3DSurface8* surf, unsigned column);
    static unsigned GetPixelSize(const SurfaceClass::SurfaceDescription& desc);
};
```

### File 3: Core/Libraries/Source/Graphics/vulkan_texture_manager.cpp

Implementation using Vulkan texture creation, memory management, and mipmap handling.

### File 4: Core/Libraries/Source/Graphics/vulkan_surface_manager.cpp

Implementation using Vulkan image buffers for surface data storage and CPU access.

---

## Integration with Game Code

### How Game Code Accesses Textures

**Before (DirectX, now commented)**:
```cpp
IDirect3DTexture8* tex = ...; // DirectX direct
```

**After (Through Wrapper)**:
```cpp
IDirect3DTexture8* tex = VulkanTextureManager::CreateTexture(...);
// VulkanTextureManager handles Vulkan internally
// Game code unchanged - still uses DX8 mock interface
```

### How Symbols Get Resolved

1. **Game calls**: `TextureClass::TextureClass(const char*)`
2. **TextureClass constructor** (stub) calls: `VulkanTextureManager::CreateTextureFromFile()`
3. **VulkanTextureManager** creates actual Vulkan texture
4. **Returns**: IDirect3DTexture8* mock interface wrapping Vulkan texture
5. **Result**: Symbol is now defined and working

---

## Build & CMakeLists.txt

Add to `Core/Libraries/Source/Graphics/CMakeLists.txt`:

```cmake
# Phase 43.1: Texture & Surface Manager (Vulkan implementation)
list(APPEND GRAPHICS_SOURCE_FILES
    "vulkan_texture_manager.cpp"
    "vulkan_surface_manager.cpp"
)
```

Add to `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`:

Keep these files COMMENTED (they're replaced by managers above):
- surfaceclass.cpp  
- texture.cpp
- texturefilter.cpp

---

## Compilation Strategy

### Step 1: Create Header Files
- vulkan_texture_manager.h
- vulkan_surface_manager.h
- (Use stubs for now - no implementation)

### Step 2: Create Stub Implementations
- vulkan_texture_manager.cpp (returns null/empty)
- vulkan_surface_manager.cpp (returns null/empty)

### Step 3: Test Linker
```bash
cmake --build build/macos-arm64-vulkan --target generalszh -j 4 2>&1 | tee logs/phase43_1_linker_test.log
```

### Step 4: Verify Symbols Resolved
```bash
grep "\".*\", referenced from:" logs/phase43_1_linker_test.log | wc -l
# Expected: ~25 fewer undefined symbols
```

### Step 5: Implement Real Functionality
- Add Vulkan texture creation
- Add Vulkan surface management
- Add mipmap handling
- Add pixel data access

---

## Verification

After Phase 43.1:

```bash
# Check undefined symbols reduced
grep "\".*\", referenced from:" logs/phase43_1_linker_test.log | \
    sed 's/.*"\([^"]*\)".*/\1/' | sort | uniq | wc -l
# Expected: 105 (was 130, reduced by 25)

# Verify no new undefined symbols added
grep "Undefined symbols for architecture" logs/phase43_1_linker_test.log
# Expected: No fatal error, continuing to next phase
```

---

## Key Points

1. **Don't Try to Fix surfaceclass.cpp** - It's DirectX-specific, leave commented
2. **Implement Through Managers** - VulkanTextureManager, VulkanSurfaceManager
3. **Use DX8Wrapper Pattern** - Game code calls DX8 mock interfaces
4. **Managers Handle Vulkan** - Translation layer internal to managers
5. **Cross-Platform** - Same code works on Linux via Vulkan, macOS via MoltenVK

---

## Next Phase

After Phase 43.1 is complete:

- Phase 43.2: Point Group System (19 symbols)
- Phase 43.3: Transport/Network Layer (12 symbols)  
- Phase 43.4: Rendering Classes (15+ symbols)
- Phase 43.5-43.7: Remaining systems

Each phase follows same pattern: Manager classes implementing symbols, no direct DirectX code.

