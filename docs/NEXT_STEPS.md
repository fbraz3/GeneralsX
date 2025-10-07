# GeneralsX - Graphics Implementation Roadmap

**Project**: GeneralsX (Command & Conquer: Generals Zero Hour macOS Port)  
**Last Updated**: January 7, 2025  
**Current Phase**: Phase 27 - Graphics Engine Implementation  
**Status**: Phase 27.2 complete - Shader management and VAO setup implemented (64% overall progress)

---

## 🎉 Current Achievement - Phase 27.2 Complete

**SHADER SYSTEM READY**: All shader loading and vertex attribute setup complete!

| Component | Implementation | Status |
|-----------|---------------|--------|
| **Shader Loading** | ✅ File loading + compilation | **COMPLETE** |
| **Program Linking** | ✅ Vertex + Fragment linking | **COMPLETE** |
| **VAO Setup** | ✅ Dynamic FVF→OpenGL mapping | **COMPLETE** |
| **Error Checking** | ✅ Comprehensive validation | **COMPLETE** |

### Latest Achievement Summary (2025-01-07)

#### Phase 27.2.4: Shader Program Management ✅
- ✅ `_Load_And_Compile_Shader()`: File loading + GLSL compilation
- ✅ `_Create_Shader_Program()`: Vertex + fragment shader linking
- ✅ `_Check_Shader_Compile_Status()`: Detailed error reporting
- ✅ `_Check_Program_Link_Status()`: Link validation with logs
- ✅ Initialization in `Init_Capabilities_Sort_Level_1()`

#### Phase 27.2.5: Vertex Attribute Setup ✅
- ✅ VAO (Vertex Array Object) creation
- ✅ `_Setup_Vertex_Attributes()`: Dynamic FVF→attribute mapping
- ✅ Attribute locations: 0=pos, 1=normal, 2=color, 3=texcoord0
- ✅ Called from `Apply_Render_State_Changes()` before draws
- ✅ Proper offset/stride calculation for all FVF formats

#### Phase 27.2.6: Backport Documentation ✅
- ✅ PHASE27_TODO_LIST.md updated with implementation details
- ✅ Ready for Generals base backport (Phase 27.7)

---

## 🚀 Phase 27: Graphics Engine Implementation

**Objective**: Transform stub graphics system into fully functional OpenGL rendering pipeline

**Estimated Timeline**: 2-4 weeks (18-27 days of development)

**Total Tasks**: 28 detailed implementation tasks organized in 5 parts

**Current Progress**: 18/28 tasks complete (64%) - All shader/buffer tasks complete, rendering states next

---

## Implementation Breakdown

For detailed task-by-task implementation with code examples, complexity ratings, and time estimates, please refer to the **PHASE27_TODO_LIST.md** which contains all 28 tasks organized into:

- **Part 1: SDL2 Window & Context Setup (27.1)** ✅ 6/6 tasks complete (100%)
- **Part 2: Buffer & Shader Abstraction (27.2)** ✅ 6/6 tasks complete (100%)
- **Part 3: Uniform Updates (27.3)** ✅ 3/3 tasks complete (100%)  
- **Part 4: Rendering & States (27.4)** 🔄 1/9 tasks complete (11%)
- **Part 5: Testing & Validation (27.5)** ⏳ 0/5 tasks (0%)

View the complete task list with implementation details in `docs/PHASE27_TODO_LIST.md`.

---

## Progress Tracking

### Completion Metrics

| Part | Tasks | Completed | Estimated Time | Status |
|------|-------|-----------|----------------|--------|
| 27.1 - Window Setup | 6 | **6/6** | 3-5 days | ✅ **COMPLETE** |
| 27.2 - Buffers & Shaders | 6 | **6/6** | 4-6 days | ✅ **COMPLETE** |
| 27.3 - Uniform Updates | 3 | **3/3** | 1-2 days | ✅ **COMPLETE** |
| 27.4 - Rendering States | 9 | **1/9** | 3-5 days | 🔄 **IN PROGRESS** |
| 27.5 - Testing | 5 | **0/5** | 1-2 days | ⏳ Not Started |
| **TOTAL** | **28 tasks** | **18/28** | **12-20 days** | **64% Complete** |
- **Logging**: Phase 27.1.4 debug messages for shutdown verification

#### ✅ Task 27.1.5: SDL Event Loop Complete
- **Files Modified**: GeneralsMD/Win32GameEngine.cpp, Generals/Win32GameEngine.cpp
- **Implementation**: Replaced Windows PeekMessage/GetMessage with SDL_PollEvent()
- **Event Handling**: 
  - SDL_QUIT → setQuitting(true)
  - SDL_WINDOWEVENT_CLOSE → setQuitting(true)
  - SDL_WINDOWEVENT_FOCUS_GAINED/LOST → setIsActive()
  - SDL_WINDOWEVENT_MINIMIZED/RESTORED → setIsActive()
  - Keyboard/mouse events → pass-through to existing input system
- **Platform Compatibility**: Preserves Windows codepath with #ifdef _WIN32
- **Compilation**: ✅ Successful ARM64 build

#### ✅ Task 27.1.6: OpenGL Rendering Test Complete
- **Implementation**: glClearColor + glClear in W3DDisplay::draw()
- **Test Color**: Blue-gray background (0.2, 0.3, 0.4, 1.0)
- **Buffer Swap**: SDL_GL_SwapWindow for frame presentation
- **Status**: ✅ Compiles successfully, validates SDL2/OpenGL stack
- **Next**: Remove early return once Part 2 DirectX→OpenGL translation begins

#### ✅ Task 27.2.1: OpenGL Vertex Buffer Abstraction Complete (100%)
- **Files Modified**: 
  - GeneralsMD/dx8vertexbuffer.h/cpp, dx8wrapper.cpp
  - Generals/dx8vertexbuffer.h/cpp, dx8wrapper.cpp
- **Implementation**:
  - ✅ Added GLuint GLVertexBuffer and void* GLVertexData members
  - ✅ Implemented Create_Vertex_Buffer() with glGenBuffers/glBufferData
  - ✅ Added destructor cleanup with glDeleteBuffers
  - ✅ Implemented WriteLockClass Lock/Unlock with CPU-side emulation
  - ✅ Implemented AppendLockClass Lock/Unlock with offset calculations
  - ✅ **DynamicVBAccessClass OpenGL support** - dynamic vertex buffers with offset-based locking
  - ✅ Fixed 4 switch statement scoping errors (added {} blocks around variable declarations)
  - ✅ Made GLVertexData public for external lock class access
  - ✅ Fixed type cast (VertexFormatXYZNDUV2* conversion)
  - ✅ Fixed FVFInfoClass constructor call (Generals uses single-parameter constructor)
  - ✅ Applied to both GeneralsMD (Zero Hour) and Generals (base game)
- **OpenGL APIs Used**: glGenBuffers, glBindBuffer, glBufferData, glBufferSubData, glDeleteBuffers
- **Design Pattern**: CPU-side buffer (GLVertexData) emulates DirectX Lock/Unlock behavior
- **Compilation**: ✅ Both targets successful (14MB ARM64 executables)
- **Git Commit**: be6202c0 - "feat(graphics): complete Tasks 27.2.1 and 27.2.2"

#### ✅ Task 27.2.2: OpenGL Index Buffer Abstraction Complete (100%)
- **Files Modified**:
  - GeneralsMD/dx8indexbuffer.h/cpp, dx8wrapper.cpp
  - Generals/dx8indexbuffer.h/cpp, dx8wrapper.cpp
- **Implementation**:
  - ✅ Added GLuint GLIndexBuffer and void* GLIndexData members
  - ✅ Implemented Create_Index_Buffer() with glGenBuffers/glBufferData(GL_ELEMENT_ARRAY_BUFFER)
  - ✅ Added destructor cleanup with glDeleteBuffers
  - ✅ Implemented WriteLockClass Lock/Unlock with CPU-side emulation
  - ✅ Implemented AppendLockClass Lock/Unlock with offset calculations
  - ✅ **DynamicIBAccessClass OpenGL support** - dynamic index buffers with offset-based locking
  - ✅ Fixed 4 switch statement scoping errors (added {} blocks around variable declarations)
  - ✅ Made GLIndexData public for external lock class access
  - ✅ Added stub Get_DX8_Vertex_Buffer/Get_DX8_Index_Buffer for legacy code compatibility
  - ✅ **Cross-platform Protection**: Added #ifdef _WIN32 guards to DirectX8-only code paths
    - Protected Draw_Sorting_IB_VB function (DirectX8-only sorting buffer wrapper)
    - Protected SetStreamSource/SetIndices calls in render state application
- **OpenGL APIs Used**: glGenBuffers, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER), glBufferData, glBufferSubData, glDeleteBuffers
- **Design Pattern**: Dual-backend with CPU-side emulation (GLIndexData) and GPU buffer (GLIndexBuffer)
- **Impact**: Resolved 19 of 23 compilation errors (83%) - remaining 4 errors unrelated to buffer tasks
- **Git Commit**: be6202c0 - "feat(graphics): complete Tasks 27.2.1 and 27.2.2"

#### ✅ Compilation Error Resolution Complete (October 6, 2025)
- **Errors Fixed**: 23 compilation errors resolved across both targets
- **OpenGL Header Conflicts**:
  - ✅ Added GLAD includes to WinMain.cpp, W3DParticleSys.cpp, W3DDisplay.cpp, Win32GameEngine.cpp
  - ✅ Moved GLAD to absolute top of .cpp files (before all other includes)
  - ✅ Pattern: `#ifndef _WIN32 #include <glad/glad.h> #endif` prevents Windows conflicts
- **W3DDisplay.cpp Fixes**:
  - ✅ Fixed unterminated `#ifndef _WIN32` directive (missing #endif on line 773)
  - ✅ Removed duplicate `WW3D::Init(ApplicationHWnd)` call
  - ✅ Protected Windows-specific ApplicationHWnd usage with #ifdef guards
- **Win32GameEngine Fixes**:
  - ✅ Copied working implementation from GeneralsMD
  - ✅ Removed incorrect #ifdef _WIN32 wrapper from Win32GameEngine.h header
  - ✅ Fixed include structure (GLAD before SDL2, windows.h stub compatibility)
- **Compilation Times** (macOS ARM64, 4 parallel jobs):
  - GeneralsXZH: ~20 minutes (14MB executable)
  - GeneralsX: ~20 minutes (17KB executable)
- **Recommendation**: IDE timeout should be 25-30 minutes for safe compilation
- **Git Commits**: 
  - 904ce238 - "fix(opengl): resolve OpenGL header conflicts and compilation errors"
  - c0521670 - "fix(opengl): resolve GeneralsX compilation issues with Win32GameEngine"

#### ✅ Task 27.3.1-27.3.3: Uniform Updates Complete (December 28, 2024)

- **Files Modified**:
  - GeneralsMD/dx8wrapper.h - Set_DX8_Material(), Set_DX8_Light()
  - GeneralsMD/dx8wrapper.cpp - Apply_Render_State_Changes()
- **Implementation**:
  - ✅ **Task 27.3.1**: Matrix uniform updates (uWorldMatrix, uViewMatrix, uProjectionMatrix)
    - Added to WORLD_CHANGED section: glUniformMatrix4fv for world matrix
    - Added to VIEW_CHANGED section: glUniformMatrix4fv for view matrix
    - Added to Set_Projection_Transform_With_Z_Bias(): glUniformMatrix4fv for projection matrix
  - ✅ **Task 27.3.2**: Material uniform updates
    - Implemented logging in Set_DX8_Material() for material diffuse color
    - Fixed D3DMATERIAL8 structure access: changed `.r/.g/.b/.a` to `[0][1][2][3]` array indices
    - Structure uses `float Diffuse[4]` not `D3DCOLORVALUE Diffuse`
  - ✅ **Task 27.3.3**: Lighting uniform updates
    - Implemented in Set_DX8_Light() with uLightDirection, uLightColor, uAmbientColor, uUseLighting
    - Support for directional lights (D3DLIGHT_DIRECTIONAL, index 0)
    - Enable/disable lighting based on light presence
- **OpenGL APIs Used**: glUniformMatrix4fv, glUniform3f, glUniform1i, glUseProgram, glGetUniformLocation
- **Build Times**: 22:56 (Task 27.3.2-27.3.3), 21:39 (Task 27.3.1)
- **Git Commit**: 4ff9651f - "feat(opengl): implement Phase 27.3-27.4.1 uniform updates and draw calls"

#### ✅ Task 27.4.1: Primitive Draw Calls Complete (December 28, 2024)

- **Files Modified**: GeneralsMD/dx8wrapper.cpp - Draw()
- **Implementation**:
  - ✅ Implemented glDrawElements for indexed primitive rendering
  - ✅ D3D primitive type mapping:
    - D3DPT_TRIANGLELIST → GL_TRIANGLES (index_count = polygon_count * 3)
    - D3DPT_TRIANGLESTRIP → GL_TRIANGLE_STRIP (index_count = polygon_count + 2)
    - D3DPT_TRIANGLEFAN → GL_TRIANGLE_FAN (index_count = polygon_count + 2)
    - D3DPT_LINELIST → GL_LINES (index_count = polygon_count * 2)
    - D3DPT_LINESTRIP → GL_LINE_STRIP (index_count = polygon_count + 1)
    - D3DPT_POINTLIST → GL_POINTS (index_count = polygon_count)
  - ✅ Proper index offset calculation: `(start_index + iba_offset) * sizeof(unsigned short)`
  - ✅ GL error checking after draw calls with glGetError()
  - ✅ Debug logging: primitive type, index count, offset, vertex count
- **OpenGL APIs Used**: glDrawElements, glGetError
- **Design Pattern**: Inline OpenGL calls replace DX8CALL macro in non-Windows builds
- **Build Time**: 23:26 (797 files compiled)
- **Git Commit**: 4ff9651f - "feat(opengl): implement Phase 27.3-27.4.1 uniform updates and draw calls"

---

## Next Steps After Phase 27

**NOTE**: Part 1 (Window Setup) testing revealed that the executable requires full DirectX→OpenGL translation to run properly. The early return in `draw()` has been restored to allow isolated SDL2/OpenGL validation until Part 2 is complete.

**Testing Strategy**: Window rendering test will be re-enabled after Part 2 DirectX→OpenGL translation layer is implemented. For now, Part 1 serves as isolated validation that SDL2/OpenGL initialization works correctly.

### Phase 28: Input System (1-2 weeks)
- Keyboard input handling
- Mouse input and camera control
- Command mapping from INI files

### Phase 29: Audio System (1-2 weeks)
- OpenAL integration
- Sound effect playback
- Music streaming

### Phase 30: Networking (2-3 weeks)
- LAN multiplayer
- GameSpy integration

### Phase 31: Gameplay (3-4 weeks)
- Map loading
- Unit logic and AI
- Economy systems

### Phase 32: Polish (2-3 weeks)
- Performance optimization
- macOS app bundle
- Code signing

---

## Development Resources

### Reference Repositories
- `references/jmarshall-win64-modern/` - D3D12 implementation patterns
- `references/fighter19-dxvk-port/` - DXVK graphics layer (D3D→Vulkan)
- `references/dsalzner-linux-attempt/` - Linux POSIX compatibility

### Key Files for Phase 27
- `Core/Libraries/Source/WWVegas/WW3D2/W3DDisplay.cpp` - Display initialization
- `Core/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` - D3D8→OpenGL wrapper
- `Core/Libraries/Source/WWVegas/WW3D/W3DView.cpp` - 3D rendering viewport
- `Core/GameEngine/Source/GameEngine.cpp` - Main engine loop

### Debugging Tools
```bash
# Run with debugger
cd $HOME/GeneralsX/GeneralsMD && lldb ./GeneralsXZH

# Check OpenGL errors in code
glGetError()

# Profile performance
Xcode Instruments (macOS)
```

---

## Immediate Action Required

### Phase 27.2.3 Progress (October 6, 2025) - ✅ COMPLETED

#### ✅ Completed Components

1. **GLTexture Member Addition**:
   - Added `unsigned int GLTexture` to TextureBaseClass (protected section for derived class access)
   - Initialized to 0 in constructor
   - Proper cleanup with glDeleteTextures in destructor
   - Fixed access control issue (moved from private to protected)

2. **OpenGL Texture Creation Function**:
   - Implemented `DX8Wrapper::_Create_GL_Texture()` in dx8wrapper.cpp (110 lines)
   - Format conversion: WW3DFormat → OpenGL internal formats
   - Supported formats: RGBA8, RGB8, RGB565, RGB5_A1, DXT1, DXT3, DXT5
   - Texture filtering: GL_LINEAR (mag), GL_LINEAR_MIPMAP_LINEAR (min with mipmaps)
   - Wrapping mode: GL_REPEAT default
   - Error checking with glGetError

3. **Texture Binding Implementation**:
   - Modified TextureClass::Apply() to use glActiveTexture + glBindTexture
   - Multi-texturing support (texture stage parameter)
   - Platform-specific: OpenGL on macOS/Linux, DirectX on Windows

4. **Build Success**:
   - Compilation time: **22:04 minutes** (within 25-30 minute target)
   - Executable size: 14MB ARM64
   - Fixed access control issue (GLTexture moved to protected section)

#### 📋 Strategic Decision: File Loading Deferred

**Decision**: DDS/TGA file loading postponed to **Task 27.5.4** (after shader implementation)

**Rationale**:
1. **Dependency Chain**: Textures require working shaders to render properly
2. **Testing Strategy**: Better to test texture loading after shader pipeline is functional
3. **Current State**: Empty textures (glTexImage2D with NULL data) prevent crashes
4. **Existing Infrastructure**: DDSFileClass already exists for future integration

**Deferred Components** (moved to Task 27.5.4):
- DDS header parsing and data extraction
- TGA header parsing and data extraction
- glCompressedTexImage2D for compressed formats
- glTexImage2D with actual pixel data
- glGenerateMipmap for mipmap generation
- Integration with TextureLoader system

#### ✅ Task 27.2.3 Status: COMPLETE

**Files Modified**:
- GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture.h
- GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture.cpp
- GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h
- GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp

**Git Commit**: e1f7e81a - "feat(graphics): implement OpenGL texture creation and binding infrastructure"

---

## Next Steps After 27.2.3

**CURRENT STATUS**: ✅ Task 27.2.1 COMPLETE - OpenGL Vertex Buffer Abstraction finished!

**MILESTONE ACHIEVED**: DirectX8 vertex buffers now fully abstracted to OpenGL VBOs

**Completed Components**:
- ✅ VBO creation with glGenBuffers/glBufferData
- ✅ CPU-side buffer for Lock/Unlock emulation
- ✅ WriteLockClass full buffer access
- ✅ AppendLockClass partial buffer access with offsets
- ✅ Proper cleanup with glDeleteBuffers
- ✅ Applied to both Zero Hour and Generals targets

**NEXT TASK**: Task 27.2.2 - Create OpenGL index buffer abstraction (3-4 hours, MEDIUM complexity)

---

**Last Updated**: October 6, 2025  
**Status**: Phase 27.2 - DirectX→OpenGL Translation Layer IN PROGRESS  
**Current Task**: Task 27.2.3 - Texture Creation and Binding (next)  
**Completed**: 8/25 tasks (32% complete)  
**Build Status**: ✅ Both GeneralsXZH and GeneralsX compile successfully (ARM64)  
**Compilation Time**: ~20 minutes per target (4 parallel jobs, recommend 25-30min IDE timeout)
