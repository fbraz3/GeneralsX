# Phase 50: Vulkan-Only Clean Refactor

**Status**: In Progress
**Start Date**: October 2025
**Objective**: Remove multi-backend architecture (DirectX mock + OpenGL + Metal); implement clean Vulkan-only codebase using fighter19-dxvk-port as architectural reference.

## Problem Analysis

### Current State (v0.49-last-multibackend)
The codebase attempts to support multiple graphics backends:
- **Windows (VC6)**: DirectX 8 native (legacy)
- **macOS ARM64**: Vulkan/MoltenVK (intended exclusive) BUT with OpenGL/Metal fallback paths
- **Linux**: DXVK (DirectX→Vulkan wrapper)

**Architecture Problem**: Multiple conditional paths in same files:
```cpp
// dx8wrapper.cpp::_Create_GL_Texture() - 5700+ line file
#ifndef _WIN32
    if (g_useMetalBackend) {
        // Metal path
    } else {
        // OpenGL path (WRONG - should NOT exist in Vulkan-only mode)
        glGenTextures(1, &gl_texture);  // NULL when ENABLE_OPENGL=OFF
    }
#endif
```

**Session Discovery**: 
- Texture loading crash at line 3162 in `_Create_GL_Texture()` - calling `glGenTextures()` when ENABLE_OPENGL=OFF
- Added `#if defined(ENABLE_OPENGL)` guard around OpenGL calls
- Program progressed further, crash moved to `Set_Viewport()` - same pattern
- Realized patching approach insufficient - ENTIRE codebase needs guards, suggesting fundamental design flaw

**Root Cause**: Architecture conflates three concerns:
1. DirectX API mock (necessary for game to run)
2. Graphics backend selection (necessary for cross-platform)
3. Mixed implementation code (unnecessary - causes complexity)

### Reference: fighter19-dxvk-port Analysis

**Key Finding**: fighter19 has **NO OpenGL calls** in WW3D2 directory (tested with grep):
```bash
grep -r "glGenTextures\|glBindTexture\|glViewport" references/fighter19-dxvk-port/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/*.cpp
# Result: 0 matches
```

**fighter19 Architecture**:
- **CMakeLists.txt**: Clear separation - `SAGE_USE_DX8` option determines Windows (native DX8) vs non-Windows (DXVK)
- **For Windows**: Uses actual DirectX 8 SDK from fetch_content
- **For Linux**: Uses DXVK wrapper (DirectX API on top of Vulkan)
- **Graphics Code**: Pure DirectX API calls only, NO OpenGL/Metal fallback code
- **WW3D2 Structure**: 254 files, NO conditional graphics backend code

**Lesson from fighter19**: When targeting specific platform, implement FOR THAT PLATFORM, don't add fallback paths.

## Clean Refactor Strategy

### Phase 50.1: Preparation

**Goal**: Create clean branch, analyze current Vulkan code, document plan

**Steps**:
1. Create `vulkan-only-refactor` branch from main
2. Tag current state: `v0.49-last-multibackend` (✅ DONE)
3. Analyze Phase 48-49 Vulkan code to identify what's production-ready
4. Identify all OpenGL/Metal code paths for removal
5. Document new clean architecture

**Files to Analyze**:
- `GeneralsMD/Code/GameEngine/Graphics/` (game-side rendering)
- `Core/Libraries/Source/GraphicsAPI/` (graphics abstraction)
- CMakeLists.txt files (build configuration)
- `cmake/` directory (feature flag definitions)

### Phase 50.2: Code Removal

**Goal**: Delete all OpenGL/Metal code, keep Vulkan infrastructure

**To Delete**:
1. **GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/**
   - **EXCEPT**: Files that are pure DirectX mock wrappers with NO graphics backend code
   - Keep structure for game engine compatibility
   - Remove only OpenGL/Metal implementations
   - OR replace entire directory with clean Vulkan-only version

2. **Core/Graphics/Metal/** (entire directory)
   - Metal-specific backend code
   - Should be 0 files after refactor

3. **cmake/opengl.cmake** - Remove (OpenGL feature flag)

4. **cmake/metal.cmake** - Remove (Metal feature flag)

5. **Core/Libraries/Source/GraphicsAPI/OpenGLRenderer.cpp** - Remove

6. **Core/Libraries/Source/GraphicsAPI/OpenGLRendererW3D.cpp** - Remove

7. **References to ENABLE_OPENGL in CMakeLists.txt** - Remove all conditional logic

8. **References to ENABLE_METAL in CMakeLists.txt** - Remove all conditional logic

**To Keep**:
- `GeneralsMD/Code/GameEngine/Graphics/` (game-side interface)
- `Core/Libraries/Source/GraphicsAPI/GraphicsRenderer.cpp` (abstraction interface)
- `Core/Libraries/Source/GraphicsAPI/W3DRendererAdapter.cpp` (adapter pattern)
- Phase 48-49 Vulkan code (proven working)
- All game logic code (unrelated to graphics backend)

### Phase 50.3: CMakeLists Simplification

**Current State**:
```cmake
option(ENABLE_OPENGL 'ON')
option(ENABLE_METAL 'ON') 
option(USE_DXVK 'ON')
option(RTS_BUILD_ZEROHOUR 'ON')

if(ENABLE_OPENGL)
    add_definitions(-DENABLE_OPENGL)
endif()

if(ENABLE_METAL)
    # Metal framework
endif()
```

**Target State**:
```cmake
# Remove all graphics backend options
# Vulkan is now the ONLY graphics backend

option(RTS_BUILD_ZEROHOUR 'ON')
option(RTS_BUILD_GENERALS 'OFF')

# Vulkan is mandatory
find_package(Vulkan REQUIRED)
```

**CMakeLists to Modify**:
1. `/CMakeLists.txt` (root)
2. `/cmake/config.cmake`
3. `/GeneralsMD/Code/CMakeLists.txt`
4. `/GeneralsMD/Code/Libraries/CMakeLists.txt`
5. `/Core/Libraries/CMakeLists.txt`
6. `/Core/Libraries/Source/GraphicsAPI/CMakeLists.txt`

### Phase 50.4: Graphics Architecture Reorganization

**Target Structure**:
```
Core/
  Libraries/
    Source/
      GraphicsAPI/
        - GraphicsRenderer.cpp (abstraction interface)
        - W3DRendererAdapter.cpp (adapter pattern)
        
GeneralsMD/
  Code/
    GameEngine/
      Graphics/
        - game_renderer.cpp (game-side interface)
        - graphics_camera.cpp
      
      VulkanGraphics/  (NEW - Vulkan-only backend)
        - vulkan_device.cpp/h
        - vulkan_swapchain.cpp/h
        - vulkan_pipeline.cpp/h
        - vulkan_renderer.cpp/h
        - vulkan_texture.cpp/h
        - vulkan_buffer.cpp/h
        - vulkan_synchronization.cpp/h
```

**Move Phase 48-49 Vulkan Code**:
- Extract working Vulkan device creation
- Extract working Vulkan render pass setup
- Extract working Vulkan texture loading
- Extract working synchronization code
- Organize by functionality (device, swapchain, rendering, synchronization)

### Phase 50.5: Build Verification

**Build Commands**:
```bash
# Configure (macOS ARM64)
cmake --preset macos-arm64

# Build GeneralsXZH
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee logs/vulkan_only_build.log

# Verify no OpenGL includes
grep -r "#include <GL/" build/macos-arm64/CMakeFiles/ && echo "FAIL: OpenGL includes found" || echo "PASS: No OpenGL includes"

# Verify no Metal includes in GeneralsXZH
grep -r "#include <Metal/" build/macos-arm64/CMakeFiles/ && echo "FAIL: Metal includes found" || echo "PASS: No Metal includes"

# Verify Vulkan includes present
grep -r "#include <vulkan/" build/macos-arm64/CMakeFiles/ || echo "WARN: No Vulkan includes found"
```

**Runtime Test**:
```bash
cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
cd $HOME/GeneralsX/GeneralsMD
timeout 30s ./GeneralsXZH 2>&1 | tee logs/vulkan_only_run.log

# Check for Vulkan initialization
grep -E "Vulkan|vkCreateInstance|vkCreateDevice" logs/vulkan_only_run.log
```

## Implementation Plan

### Step 1: Create Refactor Branch
```bash
git checkout -b vulkan-only-refactor
```

### Step 2: Document Current State
- List all files with OpenGL/Metal code
- Map Vulkan code locations
- Create migration checklist

### Step 3: Remove Dead Code
1. Delete `cmake/opengl.cmake`
2. Delete `cmake/metal.cmake`
3. Delete `Core/Graphics/Metal/`
4. Delete OpenGL-specific files from Core/Libraries/Source/GraphicsAPI/
5. Remove backend conditional logic from CMakeLists.txt files

### Step 4: Organize Vulkan Code
1. Create clean `GeneralsMD/Code/GameEngine/VulkanGraphics/` structure
2. Move Phase 48-49 Vulkan code with clear organization
3. Document each component's responsibility
4. Ensure clean separation of concerns

### Step 5: Update CMakeLists
1. Remove all `ENABLE_OPENGL`, `ENABLE_METAL`, `USE_DXVK` options
2. Make Vulkan mandatory
3. Remove all conditional graphics code
4. Add clear Vulkan requirement message

### Step 6: Build & Test
1. Full rebuild
2. Verify no OpenGL/Metal includes
3. Verify Vulkan present
4. Test runtime initialization
5. Verify game loads assets

### Step 7: Documentation
1. Update MACOS_PORT_DIARY.md
2. Create Phase 50 completion summary
3. Document lessons learned

## Success Criteria

- ✅ All OpenGL/Metal code removed from codebase
- ✅ Build has ZERO references to ENABLE_OPENGL, ENABLE_METAL, USE_DXVK
- ✅ CMakeLists.txt is simplified (removed ~50+ lines of conditional logic)
- ✅ Vulkan is the ONLY graphics backend
- ✅ Program builds without warnings related to graphics backends
- ✅ Runtime initialization succeeds and creates Vulkan instance
- ✅ Codebase is ready for Phase 51+ feature development

## Risk Assessment

**Low Risk** (proven in fighter19):
- Removing OpenGL code (0 calls in fighter19's WW3D2)
- Removing Metal code (not used by fighter19)
- Simplifying CMakeLists (fighter19's model proven simple)

**Medium Risk** (requires verification):
- Moving Vulkan code to clean structure (must preserve functionality)
- Ensuring game logic code is unaffected (should be isolated)
- Build system reorganization (need to test multiple platforms)

**Mitigation**:
- Tag before each major step
- Test after each structural change
- Keep v0.49-last-multibackend as reference
- Use fighter19 as architecture guide

## Timeline

- **Phase 50.1 (Preparation)**: 1-2 hours - analysis + documentation
- **Phase 50.2 (Code Removal)**: 1-2 hours - delete files + update CMakeLists
- **Phase 50.3 (Organization)**: 1-2 hours - reorganize Vulkan code
- **Phase 50.4 (Build Verification)**: 1 hour - build + test
- **Phase 50.5 (Documentation)**: 30 minutes - update diary + create summary

**Total**: ~5-7 hours for complete clean refactor

## Next Steps

1. ✅ Create git tag: `v0.49-last-multibackend` (DONE)
2. ⏳ Create refactor branch: `git checkout -b vulkan-only-refactor`
3. ⏳ Delete cmake/opengl.cmake and cmake/metal.cmake
4. ⏳ Remove ENABLE_OPENGL/ENABLE_METAL from CMakeLists
5. ⏳ Delete Core/Graphics/Metal/ directory
6. ⏳ Delete OpenGL files from Core/Libraries/Source/GraphicsAPI/
7. ⏳ Build and verify clean Vulkan-only compilation
8. ⏳ Test runtime with clean build
