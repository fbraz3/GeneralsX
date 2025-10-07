# Phase 27: OpenGL Implementation - Complete Task List

**Total Tasks**: 28 tasks  
**Completed**: 16/28 (57%)  
**Status**: In Progress  
**Last Updated**: December 28, 2024

---

## Task Progress Overview

| Phase | Tasks | Completed | Status |
|-------|-------|-----------|--------|
| 27.1 - SDL2 Window System | 6 | 6/6 | ✅ COMPLETE |
| 27.2 - Buffer & Shader Abstraction | 6 | 3/6 | 🔄 IN PROGRESS |
| 27.3 - Uniform Updates | 3 | 3/3 | ✅ COMPLETE |
| 27.4 - Rendering & States | 9 | 1/9 | 🔄 IN PROGRESS |
| 27.5 - Testing & Validation | 5 | 0/5 | ⏳ NOT STARTED |
| 27.6-27.8 - Finalization | 3 | 0/3 | ⏳ NOT STARTED |
| **TOTAL** | **32** | **13/32 (41%)** | 🔄 **IN PROGRESS** |

**Note**: Task count increased from 28 to 32 with addition of backport guide update tasks (27.2.6, 27.4.9, 27.5.5) and finalization tasks (27.6-27.8).

---

## Phase 27.1: SDL2 Window System ✅ COMPLETE (6/6)

### ✅ Task 27.1.1 - Framework Selection

**Status**: Complete  
**Description**: Choose between SDL2 and GLFW for window management  
**Decision**: SDL2 (better cross-platform support, audio/input integration)  
**Files**: N/A (research task)

### ✅ Task 27.1.2 - SDL2/GLAD Dependencies

**Status**: Complete  
**Description**: Install SDL2 2.32.10 and generate GLAD OpenGL 3.3 loader  
**Files**: CMakeLists.txt, Dependencies/glad/  
**Commit**: aba77114

### ✅ Task 27.1.3 - SDL2 Window Creation

**Status**: Complete  
**Description**: Create SDL window with OpenGL 3.3 Core context  
**Files**: GeneralsMD/Code/GameEngine/Source/W3DDisplay.cpp  
**APIs**: SDL_CreateWindow, SDL_GL_CreateContext, gladLoadGLLoader

### ✅ Task 27.1.4 - SDL2 Cleanup

**Status**: Complete  
**Description**: Proper resource cleanup in destructor  
**Files**: GeneralsMD/Code/GameEngine/Source/W3DDisplay.cpp  
**APIs**: SDL_GL_DeleteContext, SDL_DestroyWindow, SDL_Quit

### ✅ Task 27.1.5 - SDL Event Loop

**Status**: Complete  
**Description**: Replace Windows message loop with SDL_PollEvent  
**Files**: GeneralsMD/Code/GameEngine/Source/Win32GameEngine.cpp  
**Events**: SDL_QUIT, SDL_WINDOWEVENT_*, keyboard/mouse pass-through

### ✅ Task 27.1.6 - OpenGL Rendering Test

**Status**: Complete  
**Description**: Basic glClear test with blue-gray background  
**Files**: GeneralsMD/Code/GameEngine/Source/W3DDisplay.cpp  
**APIs**: glClearColor, glClear, SDL_GL_SwapWindow

---

## Phase 27.2: Buffer & Shader Abstraction 🔄 IN PROGRESS (3/6)

### ✅ Task 27.2.1 - Vertex Buffer Abstraction

**Status**: Complete  
**Description**: OpenGL VBO implementation with Lock/Unlock emulation  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.h/cpp  
**APIs**: glGenBuffers, glBindBuffer(GL_ARRAY_BUFFER), glBufferData, glBufferSubData  
**Commit**: be6202c0

### ✅ Task 27.2.2 - Index Buffer Abstraction

**Status**: Complete  
**Description**: OpenGL EBO implementation with CPU-side buffer  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.h/cpp  
**APIs**: glGenBuffers, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER), glBufferData  
**Commit**: be6202c0

### ✅ Task 27.2.3 - Texture Creation and Binding

**Status**: Partial (binding infrastructure ready, file loading deferred)  
**Description**: OpenGL texture creation and binding infrastructure  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp  
**APIs**: glGenTextures, glBindTexture, glTexImage2D  
**Note**: DDS/TGA file loading moved to Task 27.5.4

### ⏳ Task 27.2.4 - Shader Program Management

**Status**: Not Started  
**Description**: Load/compile vertex and fragment shaders, create program  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp/h, new shader loader  
**APIs**: glCreateShader, glShaderSource, glCompileShader, glCreateProgram, glLinkProgram  
**Dependencies**: resources/shaders/basic.vert, basic.frag (already created)  
**Critical**: Required for uniform updates to work properly

### ⏳ Task 27.2.5 - Vertex Attribute Setup

**Status**: Not Started  
**Description**: Create VAO and setup vertex attributes from FVF  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp, dx8vertexbuffer.cpp  
**APIs**: glGenVertexArrays, glBindVertexArray, glVertexAttribPointer, glEnableVertexAttribArray  
**Mapping**: FVF flags → attribute locations (0=position, 1=normal, 2=texcoord, 3=color)

### ⏳ Task 27.2.6 - Update Backport Guide (Phase 27.2)

**Status**: Not Started  
**Description**: Document Tasks 27.2.3-27.2.5 in PHASE27_BACKPORT_GUIDE.md  
**Files**: PHASE27_BACKPORT_GUIDE.md  
**Content**: Texture creation code, shader loading implementation, VAO setup, troubleshooting

---

## Phase 27.3: Uniform Updates ✅ COMPLETE (3/3)

### ✅ Task 27.3.1 - Matrix Uniforms

**Status**: Complete  
**Description**: Update uWorldMatrix, uViewMatrix, uProjectionMatrix  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp/h  
**APIs**: glUniformMatrix4fv, glGetUniformLocation  
**Commit**: 4ff9651f  
**Build**: 21:39

### ✅ Task 27.3.2 - Material Uniforms

**Status**: Complete  
**Description**: Material color logging with D3DMATERIAL8 array fix  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h  
**Fix**: Changed `.r/.g/.b/.a` to `[0][1][2][3]` for D3DMATERIAL8  
**Commit**: 4ff9651f  
**Build**: 22:56

### ✅ Task 27.3.3 - Lighting Uniforms

**Status**: Complete  
**Description**: Update uLightDirection, uLightColor, uAmbientColor, uUseLighting  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h  
**APIs**: glUniform3f, glUniform1i  
**Commit**: 4ff9651f

---

## Phase 27.4: Rendering & States 🔄 IN PROGRESS (1/9)

### ✅ Task 27.4.1 - Primitive Draw Calls

**Status**: Complete  
**Description**: glDrawElements with complete D3D primitive type mapping  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp  
**APIs**: glDrawElements, glGetError  
**Mapping**: 6 D3D types (TRIANGLELIST, TRIANGLESTRIP, TRIANGLEFAN, LINELIST, LINESTRIP, POINTLIST)  
**Commit**: 4ff9651f  
**Build**: 23:26

### ⏳ Task 27.4.2 - Render State Management

**Status**: Not Started  
**Description**: Map critical D3D render states to OpenGL  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp/h  
**States**:

- D3DRS_CULLMODE → glCullFace/glFrontFace
- D3DRS_ZENABLE → glDepthFunc/glEnable(GL_DEPTH_TEST)
- D3DRS_ALPHABLENDENABLE → glBlendFunc/glEnable(GL_BLEND)
- D3DRS_SRCBLEND/DESTBLEND → glBlendFunc parameters

### ⏳ Task 27.4.3 - Texture Stage States

**Status**: Not Started  
**Description**: Map D3D texture stage states to OpenGL sampler uniforms  
**Files**: GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp  
**States**: D3DTSS_COLOROP, D3DTSS_ALPHAOP (texture combine modes)

### ⏳ Task 27.4.4 - Alpha Testing

**Status**: Not Started  
**Description**: Shader-based alpha testing with discard  
**Files**: dx8wrapper.cpp, resources/shaders/basic.frag  
**Uniforms**: uAlphaTestEnabled, uAlphaTestFunc, uAlphaRef  
**States**: D3DRS_ALPHATESTENABLE, D3DRS_ALPHAFUNC, D3DRS_ALPHAREF

### ⏳ Task 27.4.5 - Fog Rendering

**Status**: Not Started  
**Description**: Shader-based fog (linear/exp/exp2 modes)  
**Files**: dx8wrapper.cpp, resources/shaders/basic.frag  
**Uniforms**: uFogEnabled, uFogColor, uFogStart, uFogEnd, uFogDensity  
**States**: D3DRS_FOGENABLE, D3DRS_FOGCOLOR, D3DRS_FOGSTART, D3DRS_FOGEND

### ⏳ Task 27.4.6 - Stencil Buffer Operations

**Status**: Not Started  
**Description**: Map D3D stencil states to OpenGL  
**Files**: dx8wrapper.cpp  
**APIs**: glStencilFunc, glStencilOp, glEnable(GL_STENCIL_TEST)  
**States**: D3DRS_STENCILENABLE, D3DRS_STENCILFUNC, D3DRS_STENCILPASS/FAIL/ZFAIL

### ⏳ Task 27.4.7 - Scissor Test

**Status**: Not Started  
**Description**: Viewport clipping with scissor test  
**Files**: dx8wrapper.cpp  
**APIs**: glScissor, glEnable(GL_SCISSOR_TEST)  
**States**: D3DRS_SCISSORTESTENABLE

### ⏳ Task 27.4.8 - Point Sprites

**Status**: Not Started  
**Description**: Point sprite rendering for particles  
**Files**: dx8wrapper.cpp, vertex shader  
**APIs**: gl_PointSize in vertex shader or geometry shader  
**States**: D3DRS_POINTSPRITEENABLE, D3DRS_POINTSIZE, D3DRS_POINTSCALEENABLE

### ⏳ Task 27.4.9 - Update Backport Guide (Phase 27.4)

**Status**: Not Started  
**Description**: Document Tasks 27.4.2-27.4.8 in PHASE27_BACKPORT_GUIDE.md  
**Files**: PHASE27_BACKPORT_GUIDE.md  
**Content**: Render state mapping tables, shader code for alpha/fog/sprites, troubleshooting

---

## Phase 27.5: Testing & Validation ⏳ NOT STARTED (0/5)

### ⏳ Task 27.5.1 - Basic Runtime Testing

**Status**: Not Started  
**Description**: Deploy and test engine startup with OpenGL  
**Location**: $HOME/GeneralsX/GeneralsMD/  
**Assets**: Copy Data/, Maps/ from original Zero Hour installation  
**Logs**: $HOME/Documents/Command and Conquer Generals Zero Hour Data/ReleaseCrashInfo.txt  
**Validation**: OpenGL context, shader compilation, no GL errors

### ⏳ Task 27.5.2 - Shader Debugging

**Status**: Not Started  
**Description**: Comprehensive GL error checking and shader validation  
**Tasks**:

- Add glGetError wrapper after each OpenGL call
- Verify uniform locations (log when -1)
- Check shader compilation with glGetShaderiv(GL_COMPILE_STATUS)
- Enable GL_DEBUG_OUTPUT for advanced debugging
- Verbose logging for matrix/uniform updates

### ⏳ Task 27.5.3 - Performance Baseline

**Status**: Not Started  
**Description**: Measure and document performance metrics  
**Metrics**:

- Frame time (avg/min/max)
- Draw calls per frame
- Vertex/triangle count
- FPS in different scenarios (menu, gameplay, effects)
**Tools**: Instruments.app (Time Profiler, GPU Driver)  
**Comparison**: DirectX8 baseline if available

### ⏳ Task 27.5.4 - Texture File Loading

**Status**: Not Started  
**Description**: Implement DDS and TGA file loading (deferred from 27.2.3)  
**Files**: texture.cpp, new texture loader  
**Formats**:

- DDS: Header parsing, DXT1/3/5 decompression
- TGA: RLE decompression
**Features**: Texture cache system, load from Data/ directory  
**Testing**: UI textures, terrain textures, unit textures

### ⏳ Task 27.5.5 - Update Backport Guide (Phase 27.5)

**Status**: Not Started  
**Description**: Document Tasks 27.5.1-27.5.4 in PHASE27_BACKPORT_GUIDE.md  
**Files**: PHASE27_BACKPORT_GUIDE.md  
**Content**: Testing procedures, shader debugging, profiling setup, texture loading, optimization tips

---

## Phase 27.6-27.8: Finalization ⏳ NOT STARTED (0/3)

### ⏳ Task 27.6 - Final Documentation Update

**Status**: Not Started  
**Description**: Update all documentation with Phase 27 completion  
**Files**: MACOS_PORT.md, OPENGL_SUMMARY.md, NEXT_STEPS.md, .github/copilot-instructions.md  
**Content**: Performance metrics, known issues, limitations, implementation summary

### ⏳ Task 27.7 - Generals Base Backport Execution

**Status**: Not Started  
**Description**: Execute complete backport using PHASE27_BACKPORT_GUIDE.md  
**Process**:

1. Copy shaders (basic.vert, basic.frag)
2. Copy SDL2 window code
3. Copy buffer abstractions
4. Copy uniform updates
5. Copy draw calls
6. Copy render states
7. Compile Generals target
8. Runtime testing
9. Document differences
10. Update backport guide with lessons learned

### ⏳ Task 27.8 - Git Commits and Push

**Status**: Not Started  
**Description**: Final commits and GitHub release  
**Commits**:

1. feat(opengl): complete Phase 27 OpenGL implementation
2. docs(phase27): final documentation update
3. feat(opengl): backport Phase 27 to Generals base
**Actions**: Push to origin/main, create GitHub release tag for Phase 27

---

## Backport Guide Update Strategy

To ensure the backport guide stays current, **three update tasks** have been added:

1. **Task 27.2.6**: Update after completing shader/texture/VAO work (Phase 27.2)
2. **Task 27.4.9**: Update after completing render states and advanced features (Phase 27.4)
3. **Task 27.5.5**: Update after completing testing and texture loading (Phase 27.5)

Each update task includes:

- ✅ Code examples from implemented tasks
- ✅ Troubleshooting sections based on issues encountered
- ✅ D3D→OpenGL mapping tables
- ✅ Shader code additions
- ✅ Testing procedures

This ensures the backport guide is always ready for the Generals base backport (Task 27.7).

---

## Critical Dependencies

**Task Dependencies**:

- 27.2.4 (Shader Management) → Required for 27.2.5 (VAO Setup)
- 27.2.4 (Shader Management) → Required for uniform updates to work
- 27.2.5 (VAO Setup) → Required for rendering to work
- 27.4.1 (Draw Calls) → Required for visible output
- 27.5.4 (Texture Loading) → Required for textured rendering
- All Phase 27.1-27.5 → Required for 27.7 (Backport)

**Build Order**:

1. Complete Phase 27.2 (buffers, shaders, VAO)
2. Complete Phase 27.4 (render states)
3. Complete Phase 27.5 (testing, texture loading)
4. Update documentation (27.6)
5. Execute backport (27.7)
6. Finalize (27.8)

---

## Progress Tracking

**Completed Phases**: 27.1 (100%), 27.3 (100%)  
**In Progress**: 27.2 (50%), 27.4 (11%)  
**Pending**: 27.5 (0%), 27.6-27.8 (0%)

**Next Immediate Tasks**:

1. Task 27.2.4 - Shader Program Management (CRITICAL)
2. Task 27.2.5 - Vertex Attribute Setup
3. Task 27.2.6 - Update Backport Guide

**Estimated Time to Phase 27 Completion**: 2-3 weeks of focused development

---

**Document Version**: 1.0  
**Last Updated**: December 28, 2024  
**Maintained By**: GeneralsX Development Team
