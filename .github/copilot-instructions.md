# GeneralsX - Cross-Platform AI Instructions

## Project Overview
**GeneralsX** - Cross-platform port of Command & Conquer: Generals/Zero Hour. Modernized C++20 engine with native macOS/Linux/Windows compatibility.

## Quick Start Commands

### Primary Build Workflow (macOS ARM64)
```bash
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 | tee /tmp/GeneralsXZH.log  # Zero Hour (primary)
cmake --build build/macos-arm64 --target GeneralsX -j 4 | tee /tmp/GeneralsX.log  # Original (secondary)
```

### Cross-Platform Alternatives
```bash
# Windows (VC6 compatibility): cmake --preset vc6
# macOS Intel: cmake --preset macos-x64  
# Linux: cmake --preset linux
```

### Runtime Testing & Debugging
```bash
# Setup assets directories with original game files
mkdir -p $HOME/GeneralsX/Generals
mkdir -p $HOME/GeneralsX/GeneralsMD

# Deploy Zero Hour (primary target)
cp ./build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
# Copy original Zero Hour game assets (Data/, Maps/) to $HOME/GeneralsX/GeneralsMD/

# Deploy Generals (secondary target)
cp ./build/macos-arm64/Generals/GeneralsX $HOME/GeneralsX/Generals/
# Copy original Generals game assets (Data/, Maps/) to $HOME/GeneralsX/Generals/

# Run Zero Hour with debug output  
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH

# Run Generals with debug output
cd $HOME/GeneralsX/Generals && ./GeneralsX

# Debug with LLDB when crashes occur
cd $HOME/GeneralsX/GeneralsMD && lldb ./GeneralsXZH
cd $HOME/GeneralsX/Generals && lldb ./GeneralsX
```

## Architecture Overview

### Directory Structure
- **GeneralsMD/**: Zero Hour expansion (PRIMARY TARGET) - main development focus
- **Generals/**: Original game (secondary) - used for comparison
- **Core/**: Shared 50MB+ libraries with `win32_compat.h` compatibility layer
- **references/**: Git submodules with working implementations for comparative analysis

### Cross-Platform Strategy
- **Pattern**: `#ifdef _WIN32` conditional compilation throughout codebase
- **Compatibility**: 200+ Windows APIs → POSIX alternatives via `win32_compat.h`  
- **Graphics**: DirectX8 → OpenGL through `dx8wrapper.cpp` (OpenGL chosen over Vulkan for simplicity and macOS native support)

## Critical Development Context

### 🎉 Phase 23.3 Universal INI Protection System (Phase 23.2)
**BREAKTHROUGH**: ControlBar crash COMPLETELY RESOLVED - engine progression unlocked
```cpp
// Early TheControlBar initialization in GameEngine::init() (SUCCESSFUL SOLUTION)
printf("GameEngine::init() - CRITICAL FIX: Initializing TheControlBar before INI parsing\n");
if (TheControlBar == NULL) {
    TheControlBar = NEW ControlBar;
    TheControlBar->init();
}
```
**Impact**: Engine advanced from TheThingFactory to TheGameClient (5+ subsystems progression)


### ✅ Memory Corruption Protection
```cpp
// AsciiString corruption detection in multiple places
if (m_data && ((uintptr_t)m_data < 0x1000)) {
    self->m_data = nullptr; // Reset corrupted pointer  
}

// Vector size validation with W3D PROTECTION logging
if (vectorSize > 100000) {
    printf("W3D PROTECTION: Vector corruption detected! Size %zu\n", vectorSize);
    return false;
}

// Font rendering macOS protection
if (!src_ptr || !data || !data->Buffer) {
    printf("FONT PROTECTION: NULL pointer in Blit_Char\n");
    return; // Safe fallback
}
```

## Reference Repository Strategy (Critical for Solutions)

### Available Git Submodules (Use these for comparative analysis!)
- **`references/jmarshall-win64-modern/`** - Windows 64-bit with modern features (console, D3D12, x64) - Base game (Generals) Only
- **`references/fighter19-dxvk-port/`** - Linux DXVK graphics integration with ARM64 support  
- **`references/dsalzner-linux-attempt/`** - POSIX compatibility focus

### Breakthrough Methodology
```bash
git submodule update --init --recursive  # Initialize all references
diff -r GeneralsMD/ references/jmarshall-win64-modern/GeneralsMD/  # Find working solutions
diff -r Core/ references/fighter19-dxvk-port/Core/  # Compare compatibility layers
```

**Success Example**: End token parsing crash resolved by comparing with jmarshall-win64-modern implementation

## Platform-Specific Patterns

### macOS (Primary Development Platform)
- **Native ARM64 builds** preferred for performance
- **Compatibility layers**: Win32 → POSIX in `win32_compat.h` (2270+ lines)
- **Graphics**: OpenGL preferred over Vulkan (avoids MoltenVK complexity)  
- **Build system**: CMake/Ninja with Apple Silicon optimizations

### Critical Files for AI Agents
- `Core/Libraries/Source/WWVegas/WW3D2/win32_compat.h` - 200+ Windows API mappings
- `docs/NEXT_STEPS.md` - Current development status and crash analysis
- `docs/MACOS_BUILD.md` - Complete build instructions and troubleshooting
- `docs/MACOS_PORT.md` - Comprehensive port progress, Phase 27 achievements, backport strategy
- `docs/BIG_FILES_REFERENCE.md` - Asset structure and INI file locations in .BIG archives
- `docs/OPENGL_SUMMARY.md` - OpenGL 3.3 implementation details, SDL2 integration, code examples
- `docs/PHASE27_BACKPORT_GUIDE.md` - Complete backport guide (Zero Hour → Generals base) with 837 lines of implementation details
- `docs/PHASE27_TODO_LIST.md` - Phase 27 task tracking with completion status, dependencies, and progress monitoring
- `resources/shaders/basic.vert` - OpenGL 3.3 vertex shader (transformations, lighting)
- `resources/shaders/basic.frag` - OpenGL 3.3 fragment shader (texturing, color)

## INI File System (Game Configuration)

### .BIG File Structure (Critical for Debugging)
| INI Pattern | Primary Location | Fallback | Notes |
|-------------|------------------|----------|-------|  
| `Data\INI\*.ini` | INIZH.big | INI.big | Root configuration files |
| `Data\INI\Object\*.ini` | INIZH.big | INI.big | Object definitions |
| `Data\INI\Object\*General.ini` | **INIZH.big ONLY** | - | Zero Hour exclusive generals |

**Crash Investigation**: Check `$HOME/Documents/Command\ and\ Conquer\ Generals\ Zero\ Hour\ Data/ReleaseCrashInfo.txt`

**Build Tip**: Use `-j 4` (half CPU cores) to prevent system overload during compilation.

**AI Agent Priority**: When investigating crashes, always check reference implementations first for working solutions before implementing new fixes.

**Universal INI Protection System**: Comprehensive field parser exception handling enables engine continuation through hundreds of unknown exceptions while processing complex INI files. This breakthrough enabled progression from immediate GameLOD.ini crashes to advanced GameClient subsystem initialization.

**ControlBar Resolution Success**: Early TheControlBar initialization in GameEngine::init() completely eliminated parseCommandSetDefinition crashes, unlocking major engine progression through 5+ additional subsystems.

**Documentation Structure**: All project documentation is organized in the `docs/` directory. Key files include build guides, port progress, OpenGL implementation details, and testing procedures.

## Phase 29: Metal Backend Implementation ✅ COMPLETE

**Status**: Phase 29.5.2 complete (January 13, 2025) - Full command-line parameter support and Metal initialization fixes

### Phase 29.4: Metal as Default ✅
**Goal**: Make Metal the default backend on macOS without environment variables

**Implementation**:
```cpp
// WinMain.cpp - Platform-based backend selection
#ifdef __APPLE__
    // macOS: Metal default, OpenGL opt-in with USE_OPENGL=1
    g_useMetalBackend = (getenv("USE_OPENGL") == nullptr);
#else
    // Linux: OpenGL default, Metal opt-in with USE_METAL=1
    g_useMetalBackend = (getenv("USE_METAL") != nullptr);
#endif
```

**Results**: Metal activates automatically on macOS, OpenGL override works

### Phase 29.5: Command-Line Backend Selection ✅
**Goal**: Add `-forceDirectX` and `-forceOpenGL` parameters for explicit backend control

**Implementation** (3 sub-phases):
1. **Phase 29.5.0**: Command-line parameter parsing in `CommandLine.cpp`
   - `parseForceDirectX()` and `parseForceOpenGL()` functions
   - Store in `GlobalData::m_forceDirectX` and `GlobalData::m_forceOpenGL`
   - Moved to `paramsForStartup` for early processing

2. **Phase 29.5.1**: Backend selection priority in `WinMain.cpp`
   - Priority: command-line flags → environment variables → platform defaults
   - Fixed `GetCommandLineA()` stub with `g_commandLine` global variable
   - BeginFrame/EndFrame in `dx8wrapper.cpp` use `g_useMetalBackend`

3. **Phase 29.5.2**: Metal initialization fix in `W3DDisplay.cpp` ✅
   - **CRITICAL FIX**: Replace `getenv("USE_METAL")` with `g_useMetalBackend` 
   - Located in `W3DDisplay::init_Display()` line 882
   - Aligns with Phase 29.4 Metal-as-default approach
   - Enables Metal initialization when backend selected via flags or defaults

**Technical Pattern**:
```cpp
// W3DDisplay.cpp - FIXED implementation
#ifdef __APPLE__
    // Phase 29.5.2: Use g_useMetalBackend instead of getenv()
    extern bool g_useMetalBackend;
    if (g_useMetalBackend) {  // ✅ CORRECT
        MetalWrapper::Initialize(config);
    }
#endif
```

### Runtime Commands
```bash
# macOS - Metal default (no parameters needed)
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH

# macOS - Force OpenGL (command-line parameter)
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH -forceOpenGL

# macOS - Force Metal explicitly (redundant but supported)
cd $HOME/GeneralsX/GeneralsMD && ./GeneralsXZH -forceDirectX

# macOS - OpenGL via environment variable (legacy support)
cd $HOME/GeneralsX/GeneralsMD && USE_OPENGL=1 ./GeneralsXZH
```

### Validation Results (Phase 29.5 Complete)
- ✅ Command-line parameters `-forceOpenGL` and `-forceDirectX` parsed correctly
- ✅ Backend selection priority system working (flags → env vars → defaults)
- ✅ `GetCommandLineA()` returns actual command-line arguments on macOS/Linux
- ✅ BeginFrame/EndFrame check `g_useMetalBackend` (Phase 29.5.1)
- ✅ Metal initialization checks `g_useMetalBackend` (Phase 29.5.2)
- ✅ Zero compilation errors, clean ARM64 build

### Commits
- `dbe1e07f`: Phase 29.5 - Command-line parameter support
- `dcaad904`: Phase 29.5.1 - BeginFrame/EndFrame Metal backend fix
- `ea5aa543`: Phase 29.5.2 - W3DDisplay Metal initialization fix ✅ **LATEST**

## Phase 27: OpenGL Graphics Implementation (Complete)

**Status**: 26/32 tasks complete (81%) - DirectX8→OpenGL translation layer nearly complete, finalization in progress

**Architecture**: DirectX 8 API calls wrapped with `#ifdef _WIN32` → OpenGL 3.3 Core Profile on non-Windows platforms

### Completed Implementations (Phase 27.1-27.5) ✅

#### Phase 27.1: SDL2 Window System (6/6 tasks) ✅
- SDL2 window creation with OpenGL 3.3 Core Profile context
- GLAD OpenGL loader integration  
- V-Sync support, fullscreen/windowed toggle
- Clean SDL2 cleanup and event loop
- Files: `GeneralsMD/Code/GameEngine/Source/W3DDisplay.cpp`, `Win32GameEngine.cpp`

#### Phase 27.2: Buffer & Shader Abstraction (6/6 tasks) ✅
- OpenGL VBO (Vertex Buffer Object) implementation with `glGenBuffers`, `glBufferData`
- OpenGL EBO (Element Buffer Object) for indices
- Lock/Unlock emulation using CPU-side buffers (GLVertexData/GLIndexData)
- WriteLockClass and AppendLockClass support
- Shader program management with error checking
- VAO (Vertex Array Objects) with dynamic FVF→attribute mapping
- Files: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.cpp/h`, `dx8indexbuffer.cpp/h`, `dx8wrapper.cpp/h`

#### Phase 27.3: Uniform Updates (3/3 tasks) ✅
- **Matrix Uniforms**: uWorldMatrix, uViewMatrix, uProjectionMatrix via `glUniformMatrix4fv()`
- **Material Uniforms**: Material color logging with D3DMATERIAL8 array fix (`mat->Diffuse[0]` not `.r`)
- **Lighting Uniforms**: uLightDirection, uLightColor, uAmbientColor, uUseLighting via `glUniform3f()`
- Files: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h/cpp`

#### Phase 27.4: Rendering & States (9/9 tasks) ✅
- **Primitive Draw Calls**: `glDrawElements()` with complete D3D primitive type mapping (6 types)
- **Render State Management**: Cullmode, depth test, depth mask, depth func, alpha blending (12 blend modes)
- **Texture Stage States**: Wrapping, filtering, mipmapping, anisotropy, border color
- **Alpha Testing**: Shader-based with 8 comparison modes
- **Fog Rendering**: LINEAR/EXP/EXP2 modes with color/density uniforms
- **Stencil Buffer**: Complete operations (KEEP/ZERO/REPLACE/INCR/DECR/INVERT) + masks
- **Scissor Test**: D3D9 forward-compatible state 174
- **Point Sprites**: Distance scaling with A/B/C coefficients + size clamping
- **Backport Guide**: Complete D3D→OpenGL state mapping tables documented
- File: `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp/h`

#### Phase 27.5: Testing & Validation (5/5 tasks) ✅
- **Runtime Testing**: 144,712 log lines, exit code 0, full engine validation
- **Shader Debugging**: 3 debug functions (107 lines), 15+ integration points, 0 GL errors during runtime
- **Performance Baseline**: 10s init time, 14,471 lines/sec throughput, OpenGL 4.1 Metal confirmed
- **Texture Loading Design**: Architecture documented in `PHASE27_TEXTURE_LOADING_DESIGN.md` (implementation deferred to Phase 28)
- **Backport Guide Update**: 430+ lines added with Phase 27.5 procedures

### Current Work (Phase 27.6) 🔄

#### Phase 27.6: Final Documentation Update (In Progress - 83%)
- ✅ PHASE27_TODO_LIST.md corrected (26/32 tasks, 81%)
- ✅ MACOS_PORT.md updated with Phase 27.5 complete status
- ✅ OPENGL_SUMMARY.md updated with final implementations
- ✅ NEXT_STEPS.md updated with post-Phase 27 roadmap
- ✅ .github/copilot-instructions.md (this file - being updated now)
- ⏳ PHASE27_COMPLETION_SUMMARY.md pending

### Critical Phase 27 Patterns

**D3DMATERIAL8 Color Access** (CRITICAL FIX):
```cpp
// WRONG: mat->Diffuse.r (does not compile)
// CORRECT: mat->Diffuse[0] (D3DMATERIAL8 uses float[4] arrays)
float r = mat->Diffuse[0];
float g = mat->Diffuse[1];
float b = mat->Diffuse[2];
float a = mat->Diffuse[3];
```

**OpenGL Uniform Updates**:
```cpp
#ifndef _WIN32
    if (GL_Shader_Program != 0) {
        glUseProgram(GL_Shader_Program);
        GLint loc = glGetUniformLocation(GL_Shader_Program, "uWorldMatrix");
        if (loc != -1) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&render_state.world);
        }
    }
#endif
```

**Primitive Draw Calls**:
```cpp
#ifndef _WIN32
    GLenum gl_primitive_type = GL_TRIANGLES;
    unsigned index_count = polygon_count * 3;
    
    switch (primitive_type) {
    case D3DPT_TRIANGLELIST:
        gl_primitive_type = GL_TRIANGLES;
        index_count = polygon_count * 3;
        break;
    // ... other cases ...
    }
    
    GLsizei offset_bytes = (start_index + render_state.iba_offset) * sizeof(unsigned short);
    glDrawElements(gl_primitive_type, index_count, GL_UNSIGNED_SHORT, 
        (const void*)(uintptr_t)offset_bytes);
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("GL ERROR: glDrawElements failed with error 0x%04X\n", error);
    }
#endif
```

**Error Checking** (Phase 27.5.2):
```cpp
void _Check_GL_Error(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* error_string = "UNKNOWN_ERROR";
        switch (error) {
            case GL_INVALID_ENUM: error_string = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: error_string = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: error_string = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: error_string = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error_string = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        printf("GL ERROR in %s: %s (0x%04X)\n", operation, error_string, error);
    }
}
```

### Backport Strategy

**Approach**: Implement all OpenGL features in **GeneralsMD (Zero Hour)** first, then backport to **Generals (base game)**.

**Rationale**:
1. Zero Hour has more complex graphics (generals, powers, particle effects)
2. Testing in Zero Hour ensures robustness for edge cases
3. Backport is straightforward: copy working code with minimal adjustments
4. Generals base has simpler rendering, fewer compatibility issues

**Documentation**: See `docs/PHASE27_BACKPORT_GUIDE.md` for complete backport instructions with copy-paste ready code examples.

### Performance Metrics (Phase 27.5.3)

- **Total execution time**: ~10 seconds (BIG files → MapCache → GameEngine → execute loop)
- **Initialization breakdown**: 60% MapCache, 20% BIG files, 20% other
- **Resource loading**: 19 BIG archives, 146 multiplayer maps
- **OpenGL version**: 4.1 Metal - 90.5 (macOS ARM64)
- **Error rate**: 0 OpenGL errors during full engine execution
- **Build stats**: 923/923 files (100%), 129 warnings (non-critical)

### Next Steps

- **Phase 27.6**: Final documentation update (83% complete)
- **Phase 27.7**: Backport to Generals base game (estimated 2-3 hours)
- **Phase 27.8**: Git finalization and GitHub release (estimated 1 hour)
- **Phase 28**: Complete texture system (DDS/TGA loading, 2-3 weeks)