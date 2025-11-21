# 🚀 Phase 38 Quick Start - Today's Tasks

**Current Date**: October 29, 2025  
**Phase**: 38 - Graphics Backend Abstraction  
**Subtask**: 38.1 - Define Graphics Backend Interface  
**Estimated Time**: Rest of today + tomorrow

---

## What You're About to Do

Create an **interface layer** that will let us switch between graphics backends (Metal/OpenGL vs DXVK/Vulkan) at build time.

### Big Picture
```
Game Code
    ↓
IGraphicsBackend interface ← YOU ARE HERE (creating this)
    ↓
USE_DXVK=OFF          USE_DXVK=ON
    ↓                      ↓
LegacyGraphicsBackend  DVKGraphicsBackend
    ↓                      ↓
Metal/OpenGL          DXVK/Vulkan
    ↓                      ↓
GPU
```

---

## Task 38.1: Create the Interface (TODAY)

### Step 1: Create the Header File

Create new file: `Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h`

```cpp
#pragma once

// Forward declarations for DirectX types
struct D3DRECT;
enum D3DRENDERSTATETYPE;
enum D3DFORMAT;

// Define the graphics backend interface
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() {}
    
    // ========================
    // Texture Management
    // ========================
    virtual HRESULT SetTexture(unsigned stage, void* texture) = 0;
    virtual HRESULT CreateTexture(unsigned width, unsigned height, unsigned format, void** texture) = 0;
    virtual HRESULT ReleaseTexture(void* texture) = 0;
    virtual HRESULT LockTexture(void* texture, void** data, unsigned* pitch) = 0;
    virtual HRESULT UnlockTexture(void* texture) = 0;
    
    // ========================
    // Render States
    // ========================
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value) = 0;
    virtual HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value) = 0;
    
    // ========================
    // Scene Rendering
    // ========================
    virtual HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil) = 0;
    virtual HRESULT BeginScene() = 0;
    virtual HRESULT EndScene() = 0;
    virtual HRESULT Present() = 0;
    
    // ========================
    // Vertex/Index Buffers
    // ========================
    virtual HRESULT CreateVertexBuffer(unsigned size, unsigned usage, unsigned format, void** buffer) = 0;
    virtual HRESULT ReleaseVertexBuffer(void* buffer) = 0;
    virtual HRESULT LockVertexBuffer(void* buffer, void** data) = 0;
    virtual HRESULT UnlockVertexBuffer(void* buffer) = 0;
    
    virtual HRESULT CreateIndexBuffer(unsigned size, unsigned format, void** buffer) = 0;
    virtual HRESULT ReleaseIndexBuffer(void* buffer) = 0;
    virtual HRESULT LockIndexBuffer(void* buffer, void** data) = 0;
    virtual HRESULT UnlockIndexBuffer(void* buffer) = 0;
    
    // ========================
    // Drawing
    // ========================
    virtual HRESULT SetVertexBuffer(unsigned stream, void* buffer, unsigned stride) = 0;
    virtual HRESULT SetIndexBuffer(void* buffer) = 0;
    virtual HRESULT DrawPrimitive(unsigned type, unsigned startVertex, unsigned primitiveCount) = 0;
    virtual HRESULT DrawIndexedPrimitive(unsigned type, unsigned baseVertexIndex, unsigned minVertexIndex, unsigned numVertices, unsigned startIndex, unsigned primitiveCount) = 0;
    
    // ========================
    // Other State
    // ========================
    virtual HRESULT SetViewport(unsigned x, unsigned y, unsigned width, unsigned height, float minZ, float maxZ) = 0;
    virtual HRESULT SetTransform(unsigned state, const float* matrix) = 0;
    virtual HRESULT SetLight(unsigned index, const void* light) = 0;
    virtual HRESULT SetMaterial(const void* material) = 0;
    
    // ========================
    // Debug/Utility
    // ========================
    virtual const char* GetBackendName() const = 0;
    virtual HRESULT GetLastError() const = 0;
};

// Global graphics backend instance
extern IGraphicsBackend* g_graphicsBackend;
```

### Step 2: Understand What You're Creating

- **IGraphicsBackend**: Pure virtual interface (abstract class)
- **~30 virtual methods**: Every graphics operation
- **Two implementations coming later**:
  - `LegacyGraphicsBackend` (Phase 38.2) - wraps old Metal/OpenGL code
  - `DVKGraphicsBackend` (Phase 39+) - new DXVK implementation

### Step 3: Test the Header

```bash
cd /Users/felipebraz/PhpstormProjects/pessoal/GeneralsGameCode

# Just verify it compiles
clang -I. -I./Core/Libraries/Source/WWVegas/WW3D2 \
  -I./Core/Libraries/Source/WWVegas \
  -c Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h

# Or let CMake find errors during full build:
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | head -50
```

---

## Task 38.2: Create Legacy Wrapper (TOMORROW)

After header is solid, create wrapper that delegates to old code:

```cpp
// Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_legacy.h
#pragma once
#include "graphics_backend.h"

class LegacyGraphicsBackend : public IGraphicsBackend {
public:
    HRESULT SetTexture(unsigned stage, void* texture) override;
    HRESULT CreateTexture(unsigned width, unsigned height, unsigned format, void** texture) override;
    // ... all other methods
};
```

```cpp
// Core/Libraries/Source/WWVegas/WW3D2/graphics_backend_legacy.cpp
#include "graphics_backend_legacy.h"
#include "dx8wrapper.h"      // or similar existing wrapper
#include "metalwrapper.h"     // or similar existing wrapper

HRESULT LegacyGraphicsBackend::SetTexture(unsigned stage, void* texture) {
    // Just delegate to existing code
    return DX8Wrapper::SetTexture(stage, (IDirect3DTexture8*)texture);
}

// ... implement all other methods similarly
```

---

## Right Now - TODAY

### Your Next Actions (In Order)

1. **CREATE** `Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h`
   - Copy-paste the interface definition above
   - Verify all ~30 methods are there
   - Include proper header guards

2. **VERIFY IT COMPILES**
   ```bash
   cmake --preset macos-arm64
   cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee /tmp/phase38_build.log
   grep -i "error:" /tmp/phase38_build.log | head -10
   ```

3. **UPDATE DIARY**
   ```bash
   echo "- Phase 38.1: Graphics backend interface created and compiling" >> docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)
   ```

4. **COMMIT**
   ```bash
   git add Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h
   git commit -m "feat(phase38.1): define graphics backend interface

- Create IGraphicsBackend abstract base class
- 30 virtual methods covering all graphics operations
- Support for texture, render state, drawing, viewport, etc
- Pure interface, no implementation yet
- Ready for LegacyGraphicsBackend wrapper in Phase 38.2"
   ```

---

## Troubleshooting

### If Header Won't Compile
- Check for circular includes
- Verify forward declarations for D3D types
- Make sure all virtual methods have `= 0`
- Check for typos in method names

### If Game Won't Link
- Header is just declaration, linking happens in Phase 38.2 when we create LegacyGraphicsBackend
- For now, just get header compiling is success

### If You're Unsure
- Keep it simple - just copy the interface above
- All methods are already in existing code, you're just exposing the interface
- Can adjust methods later as you find what's actually used

---

## Success Criteria for Today

✅ File `Core/Libraries/Source/WWVegas/WW3D2/graphics_backend.h` exists  
✅ It compiles without errors  
✅ Contains ~30 virtual methods  
✅ Game still builds (even if not linked yet)  
✅ You understand what IGraphicsBackend will do  

---

## Reference

- Phase specification: `docs/PLANNING/3/PHASE38/README.md`
- Roadmap overview: `docs/PHASE_ROADMAP_V2.md` (Phases 38-50)
- Current status: `docs/DEV_BLOG/YYYY-MM-DIARY.md (organized by month)`

---

## 🚀 LET'S GO!

Time to create the interface. Start with the header file creation. You've got this! 💪

**When done**: Update diary, commit, and move to Phase 38.2 tomorrow!
