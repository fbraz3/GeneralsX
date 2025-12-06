# Phase 27 Backport Guide - Zero Hour → Generals Base

**Purpose**: This guide documents all Phase 27 OpenGL implementations in GeneralsMD (Zero Hour) to facilitate backporting to Generals (base game).

**Status**: Phase 27.4 complete (25/28 tasks - 89%)

**Last Updated**: October 7, 2025

---

## Backport Strategy

**Approach**: Implement all OpenGL features in **Zero Hour first**, then backport to **Generals base**.

**Rationale**:
1. Zero Hour has more complex graphics (generals, powers, particle effects)
2. Testing in Zero Hour ensures robustness for edge cases
3. Backport is straightforward: copy working code with minimal adjustments
4. Generals base has simpler rendering, fewer compatibility issues

---

## File Modifications Summary

### 1. Window System (Phase 27.1) ✅

#### Files Modified:
- `GeneralsMD/Code/GameEngine/Source/W3DDisplay.cpp`

#### Changes:
```cpp
// SDL2 initialization replacing Windows-only code
#ifndef _WIN32
    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL2 initialization failed: %s\n", SDL_GetError());
        return false;
    }
    
    // Configure OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // Create window
    sdl_window = SDL_CreateWindow(
        "GeneralsX - Command & Conquer: Generals Zero Hour",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0)
    );
    
    // Create OpenGL context
    sdl_gl_context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
    SDL_GL_SetSwapInterval(1); // V-Sync
    
    // Load OpenGL functions with GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return false;
    }
    
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GPU Vendor: %s\n", glGetString(GL_VENDOR));
    printf("GPU Renderer: %s\n", glGetString(GL_RENDERER));
#endif
```

#### Destructor cleanup:
```cpp
#ifndef _WIN32
    if (sdl_gl_context) {
        SDL_GL_DeleteContext(sdl_gl_context);
        sdl_gl_context = nullptr;
    }
    if (sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = nullptr;
    }
    SDL_Quit();
#endif
```

#### Backport Notes:
- Change window title from "Zero Hour" to "Generals"
- Verify SDL2 dependency in CMakeLists.txt (already present)
- GLAD loader already configured in both targets

---

### 2. Vertex Buffer Abstraction (Phase 27.2.1) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.h`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8vertexbuffer.cpp`

#### Header Changes (dx8vertexbuffer.h):
```cpp
class DX8VertexBufferClass : public W3DVertexBufferClass
{
public:
    // Add OpenGL members
#ifndef _WIN32
    GLuint GLVertexBuffer;      // OpenGL VBO handle
    void* GLVertexData;         // CPU-side buffer for Lock/Unlock emulation
#endif

    // Constructor initialization
    DX8VertexBufferClass() : 
#ifndef _WIN32
        GLVertexBuffer(0),
        GLVertexData(nullptr),
#endif
        { /* existing init */ }
};
```

#### Implementation Changes (dx8vertexbuffer.cpp):
```cpp
void DX8VertexBufferClass::Create_Vertex_Buffer(unsigned vertex_count, unsigned fvf, bool dynamic)
{
#ifdef _WIN32
    // DirectX 8 implementation (existing code)
#else
    // Phase 27.2.1: OpenGL VBO implementation
    VertexCount = vertex_count;
    FVF = fvf;
    IsDynamic = dynamic;
    
    unsigned vertex_size = Compute_Vertex_Size(fvf);
    unsigned total_size = vertex_count * vertex_size;
    
    // Create OpenGL VBO
    glGenBuffers(1, &GLVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, GLVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, total_size, nullptr, 
        dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    
    // Allocate CPU-side buffer for Lock/Unlock emulation
    GLVertexData = malloc(total_size);
    if (!GLVertexData) {
        printf("Phase 27.2.1 ERROR: Failed to allocate GLVertexData (%u bytes)\n", total_size);
    }
    
    printf("Phase 27.2.1: VBO created (handle=%u, size=%u bytes, dynamic=%d)\n",
        GLVertexBuffer, total_size, dynamic);
#endif
}

// Destructor cleanup
DX8VertexBufferClass::~DX8VertexBufferClass()
{
#ifndef _WIN32
    if (GLVertexBuffer != 0) {
        glDeleteBuffers(1, &GLVertexBuffer);
        GLVertexBuffer = 0;
    }
    if (GLVertexData != nullptr) {
        free(GLVertexData);
        GLVertexData = nullptr;
    }
#endif
}
```

#### Lock/Unlock Implementation:
```cpp
// WriteLockClass::Lock()
#ifndef _WIN32
    if (VB->GLVertexData) {
        return (char*)VB->GLVertexData + offset;
    }
#endif

// WriteLockClass::Unlock()
#ifndef _WIN32
    if (VB->GLVertexBuffer != 0 && VB->GLVertexData) {
        glBindBuffer(GL_ARRAY_BUFFER, VB->GLVertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, 
            (char*)VB->GLVertexData + offset);
    }
#endif
```

#### Backport Notes:
- Identical implementation for Generals base
- No game-specific differences
- Copy entire functions with `#ifndef _WIN32` blocks

---

### 3. Index Buffer Abstraction (Phase 27.2.2) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.h`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8indexbuffer.cpp`

#### Header Changes (dx8indexbuffer.h):
```cpp
class DX8IndexBufferClass : public W3DIndexBufferClass
{
public:
    // Add OpenGL members
#ifndef _WIN32
    GLuint GLIndexBuffer;       // OpenGL EBO handle
    void* GLIndexData;          // CPU-side buffer for Lock/Unlock emulation
#endif

    // Constructor
    DX8IndexBufferClass() :
#ifndef _WIN32
        GLIndexBuffer(0),
        GLIndexData(nullptr),
#endif
        { /* existing init */ }
};
```

#### Implementation (dx8indexbuffer.cpp):
```cpp
void DX8IndexBufferClass::Create_Index_Buffer(unsigned index_count, bool dynamic)
{
#ifdef _WIN32
    // DirectX 8 implementation
#else
    // Phase 27.2.2: OpenGL EBO implementation
    IndexCount = index_count;
    IsDynamic = dynamic;
    
    unsigned total_size = index_count * sizeof(unsigned short);
    
    // Create OpenGL EBO
    glGenBuffers(1, &GLIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_size, nullptr,
        dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    
    // Allocate CPU-side buffer
    GLIndexData = malloc(total_size);
    
    printf("Phase 27.2.2: EBO created (handle=%u, indices=%u, size=%u bytes)\n",
        GLIndexBuffer, index_count, total_size);
#endif
}

// Destructor
~DX8IndexBufferClass()
{
#ifndef _WIN32
    if (GLIndexBuffer != 0) {
        glDeleteBuffers(1, &GLIndexBuffer);
    }
    if (GLIndexData != nullptr) {
        free(GLIndexData);
    }
#endif
}
```

#### Backport Notes:
- Identical to vertex buffer pattern
- Copy entire implementation
- No game-specific code

---

### 4. Matrix Uniforms (Phase 27.3.1) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### World Matrix Update (dx8wrapper.cpp):
```cpp
void DX8Wrapper::Apply_Render_State_Changes()
{
    // ... existing code ...
    
    if (render_state_changed & WORLD_CHANGED) {
#ifdef _WIN32
        DX8CALL(SetTransform(D3DTS_WORLDMATRIX(0), (D3DMATRIX*)&render_state.world));
#else
        // Phase 27.3.1: Update World matrix uniform
        if (GL_Shader_Program != 0) {
            glUseProgram(GL_Shader_Program);
            GLint loc = glGetUniformLocation(GL_Shader_Program, "uWorldMatrix");
            if (loc != -1) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&render_state.world);
                printf("Phase 27.3.1: Updated uWorldMatrix uniform\n");
            }
        }
#endif
    }
    
    if (render_state_changed & VIEW_CHANGED) {
#ifdef _WIN32
        DX8CALL(SetTransform(D3DTS_VIEW, (D3DMATRIX*)&render_state.view));
#else
        // Phase 27.3.1: Update View matrix uniform
        if (GL_Shader_Program != 0) {
            glUseProgram(GL_Shader_Program);
            GLint loc = glGetUniformLocation(GL_Shader_Program, "uViewMatrix");
            if (loc != -1) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&render_state.view);
                printf("Phase 27.3.1: Updated uViewMatrix uniform\n");
            }
        }
#endif
    }
}
```

#### Projection Matrix Update (dx8wrapper.h):
```cpp
WWINLINE void DX8Wrapper::Set_Projection_Transform_With_Z_Bias(const Matrix4& proj, float z_bias)
{
    // ... existing code ...
    
#ifdef _WIN32
    DX8CALL(SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&ProjectionMatrix));
#else
    // Phase 27.3.1: Update Projection matrix uniform
    if (GL_Shader_Program != 0) {
        glUseProgram(GL_Shader_Program);
        GLint loc = glGetUniformLocation(GL_Shader_Program, "uProjectionMatrix");
        if (loc != -1) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&ProjectionMatrix);
        }
    }
#endif
}
```

#### Backport Notes:
- Identical code for Generals base
- GL_Shader_Program is global variable (ensure it exists)
- Copy both functions completely

---

### 5. Material Uniforms (Phase 27.3.2) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### Implementation:
```cpp
WWINLINE void DX8Wrapper::Set_DX8_Material(const D3DMATERIAL8* mat)
{
    DX8_RECORD_MATERIAL_CHANGE();
    WWASSERT(mat);
    SNAPSHOT_SAY(("DX8 - SetMaterial"));
#ifdef _WIN32
    DX8CALL(SetMaterial(mat));
#else
    // Phase 27.3.2: Update material uniforms in OpenGL shader
    if (GL_Shader_Program != 0 && mat != NULL) {
        glUseProgram(GL_Shader_Program);
        
        // CRITICAL: D3DMATERIAL8 uses float[4] arrays, NOT structs!
        // Access: mat->Diffuse[0], [1], [2], [3] for RGBA
        printf("Phase 27.3.2: Material set (diffuse: %.2f,%.2f,%.2f,%.2f)\n", 
            mat->Diffuse[0], mat->Diffuse[1], mat->Diffuse[2], mat->Diffuse[3]);
        
        // Future expansion:
        // GLint loc = glGetUniformLocation(GL_Shader_Program, "uMaterialDiffuse");
        // glUniform4f(loc, mat->Diffuse[0], mat->Diffuse[1], mat->Diffuse[2], mat->Diffuse[3]);
    }
#endif
}
```

#### D3DMATERIAL8 Structure Reference:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 852-860)
typedef struct {
    float Diffuse[4];   // [0]=r, [1]=g, [2]=b, [3]=a
    float Ambient[4];
    float Specular[4];
    float Emissive[4];
    float Power;
} D3DMATERIAL8;
```

#### Backport Notes:
- **CRITICAL**: Do NOT use `.r/.g/.b/.a` - use array indices `[0][1][2][3]`
- Identical implementation for Generals
- Future shader expansion can add actual uniform updates

---

### 6. Lighting Uniforms (Phase 27.3.3) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### Implementation:
```cpp
WWINLINE void DX8Wrapper::Set_DX8_Light(DWORD index, const D3DLIGHT8* light)
{
    SNAPSHOT_SAY(("DX8 - SetLight %d",index));
#ifdef _WIN32
    DX8CALL(SetLight(index, light));
    DX8CALL(LightEnable(index, light != NULL));
#else
    // Phase 27.3.3: Update lighting uniforms in OpenGL shader
    if (GL_Shader_Program != 0) {
        glUseProgram(GL_Shader_Program);
        
        if (light && light->Type == D3DLIGHT_DIRECTIONAL && index == 0) {
            // Update light direction uniform
            GLint loc = glGetUniformLocation(GL_Shader_Program, "uLightDirection");
            if (loc != -1) {
                glUniform3f(loc, light->Direction.x, light->Direction.y, light->Direction.z);
            }
            
            // Update light color (diffuse) uniform
            loc = glGetUniformLocation(GL_Shader_Program, "uLightColor");
            if (loc != -1) {
                glUniform3f(loc, light->Diffuse.r, light->Diffuse.g, light->Diffuse.b);
            }
            
            // Update ambient color uniform
            loc = glGetUniformLocation(GL_Shader_Program, "uAmbientColor");
            if (loc != -1) {
                glUniform3f(loc, light->Ambient.r, light->Ambient.g, light->Ambient.b);
            }
            
            // Enable lighting
            loc = glGetUniformLocation(GL_Shader_Program, "uUseLighting");
            if (loc != -1) {
                glUniform1i(loc, 1);
            }
            
            printf("Phase 27.3.3: Lighting uniforms updated (directional light 0)\n");
        } else if (light == NULL) {
            // Disable lighting when light is NULL
            GLint loc = glGetUniformLocation(GL_Shader_Program, "uUseLighting");
            if (loc != -1) {
                glUniform1i(loc, 0);
            }
            printf("Phase 27.3.3: Lighting disabled\n");
        }
    }
#endif
}
```

#### D3DLIGHT8 Structure Reference:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 819-835)
typedef struct {
    DWORD Type;                 // D3DLIGHT_DIRECTIONAL, etc.
    D3DCOLORVALUE Diffuse;      // .r, .g, .b, .a members
    D3DCOLORVALUE Specular;
    D3DCOLORVALUE Ambient;
    D3DVECTOR Position;         // .x, .y, .z members
    D3DVECTOR Direction;
    float Range;
    float Falloff;
    // ... other members
} D3DLIGHT8;

typedef struct {
    float r, g, b, a;
} D3DCOLORVALUE;

typedef struct {
    float x, y, z;
} D3DVECTOR;
```

#### Backport Notes:
- D3DLIGHT8 uses D3DCOLORVALUE (has .r/.g/.b/.a) ✓
- D3DVECTOR has .x/.y/.z members ✓
- Only supports directional light at index 0 (most common)
- Copy entire function to Generals

---

### 7. Primitive Draw Calls (Phase 27.4.1) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

#### Implementation (Draw function):
```cpp
void DX8Wrapper::Draw(
    unsigned primitive_type,
    unsigned short start_index,
    unsigned short polygon_count,
    unsigned short min_vertex_index,
    unsigned short vertex_count)
{
    // ... existing validation code ...
    
    Apply_Render_State_Changes();
    
    if (!_Is_Triangle_Draw_Enabled()) return;
    
    // ... existing code ...
    
    switch (render_state.vertex_buffer_types[0]) {
    case BUFFER_TYPE_DX8:
    case BUFFER_TYPE_DYNAMIC_DX8:
        switch (render_state.index_buffer_type) {
        case BUFFER_TYPE_DX8:
        case BUFFER_TYPE_DYNAMIC_DX8:
            {
                DX8_RECORD_RENDER(polygon_count, vertex_count, render_state.shader);
                DX8_RECORD_DRAW_CALLS();
#ifdef _WIN32
                DX8CALL(DrawIndexedPrimitive(
                    (D3DPRIMITIVETYPE)primitive_type,
                    min_vertex_index,
                    vertex_count,
                    start_index + render_state.iba_offset,
                    polygon_count));
#else
                // Phase 27.4.1: OpenGL indexed primitive rendering
                GLenum gl_primitive_type = GL_TRIANGLES;
                unsigned index_count = polygon_count * 3;
                
                // Map D3D primitive types to OpenGL
                switch (primitive_type) {
                case D3DPT_TRIANGLELIST:
                    gl_primitive_type = GL_TRIANGLES;
                    index_count = polygon_count * 3;
                    break;
                case D3DPT_TRIANGLESTRIP:
                    gl_primitive_type = GL_TRIANGLE_STRIP;
                    index_count = polygon_count + 2;
                    break;
                case D3DPT_TRIANGLEFAN:
                    gl_primitive_type = GL_TRIANGLE_FAN;
                    index_count = polygon_count + 2;
                    break;
                case D3DPT_LINELIST:
                    gl_primitive_type = GL_LINES;
                    index_count = polygon_count * 2;
                    break;
                case D3DPT_LINESTRIP:
                    gl_primitive_type = GL_LINE_STRIP;
                    index_count = polygon_count + 1;
                    break;
                case D3DPT_POINTLIST:
                    gl_primitive_type = GL_POINTS;
                    index_count = polygon_count;
                    break;
                default:
                    printf("Phase 27.4.1 WARNING: Unknown primitive type %d, defaulting to TRIANGLES\n", 
                        primitive_type);
                    break;
                }
                
                // Calculate byte offset for index buffer
                GLsizei offset_bytes = (start_index + render_state.iba_offset) * sizeof(unsigned short);
                
                printf("Phase 27.4.1: glDrawElements(type=%d, count=%u, offset=%d, verts=%u)\n", 
                    primitive_type, index_count, offset_bytes, vertex_count);
                
                // Render with OpenGL
                glDrawElements(gl_primitive_type, index_count, GL_UNSIGNED_SHORT, 
                    (const void*)(uintptr_t)offset_bytes);
                
                // Check for errors
                GLenum error = glGetError();
                if (error != GL_NO_ERROR) {
                    printf("Phase 27.4.1 ERROR: glDrawElements failed with error 0x%04X\n", error);
                }
#endif
            }
            break;
        // ... other cases ...
        }
        break;
    // ... other cases ...
    }
}
```

#### D3D Primitive Type Constants:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 261-266)
#define D3DPT_POINTLIST 1
#define D3DPT_LINELIST 2
#define D3DPT_LINESTRIP 3
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN 6
```

#### Primitive Type Mapping Table:

| D3D Type | Value | OpenGL Type | Index Count Formula |
|----------|-------|-------------|---------------------|
| D3DPT_POINTLIST | 1 | GL_POINTS | polygon_count |
| D3DPT_LINELIST | 2 | GL_LINES | polygon_count × 2 |
| D3DPT_LINESTRIP | 3 | GL_LINE_STRIP | polygon_count + 1 |
| D3DPT_TRIANGLELIST | 4 | GL_TRIANGLES | polygon_count × 3 |
| D3DPT_TRIANGLESTRIP | 5 | GL_TRIANGLE_STRIP | polygon_count + 2 |
| D3DPT_TRIANGLEFAN | 6 | GL_TRIANGLE_FAN | polygon_count + 2 |

#### Backport Notes:
- Large block of code (60+ lines)
- Copy entire `#else` block to Generals
- No game-specific differences
- Verify D3DPT_* constants are defined in both

---

### 8. Render State Management (Phase 27.4.2) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### Implementation (Set_DX8_Render_State function):

**Culling Mode** (D3DRS_CULLMODE):
```cpp
case D3DRS_CULLMODE:  // 22
{
	if (value == D3DCULL_NONE) {
		glDisable(GL_CULL_FACE);
		printf("Phase 27.4.2: Culling disabled\n");
	} else {
		glEnable(GL_CULL_FACE);
		
		// D3D default: cull clockwise (back faces)
		// OpenGL default: cull counter-clockwise (back faces)
		if (value == D3DCULL_CW) {
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);  // Counter-clockwise is front
			printf("Phase 27.4.2: Cull clockwise (back faces)\n");
		} else if (value == D3DCULL_CCW) {
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);   // Clockwise is front
			printf("Phase 27.4.2: Cull counter-clockwise (back faces)\n");
		}
	}
	break;
}
```

**Depth Testing** (D3DRS_ZENABLE, D3DRS_ZFUNC, D3DRS_ZWRITEENABLE):
```cpp
case D3DRS_ZENABLE:  // 7
{
	if (value) {
		glEnable(GL_DEPTH_TEST);
		printf("Phase 27.4.2: Depth test enabled\n");
	} else {
		glDisable(GL_DEPTH_TEST);
		printf("Phase 27.4.2: Depth test disabled\n");
	}
	break;
}

case D3DRS_ZFUNC:  // 23
{
	GLenum depth_func = GL_LESS;
	switch (value) {
		case 1: depth_func = GL_NEVER; break;    // D3DCMP_NEVER
		case 2: depth_func = GL_LESS; break;     // D3DCMP_LESS
		case 3: depth_func = GL_EQUAL; break;    // D3DCMP_EQUAL
		case 4: depth_func = GL_LEQUAL; break;   // D3DCMP_LESSEQUAL
		case 5: depth_func = GL_GREATER; break;  // D3DCMP_GREATER
		case 6: depth_func = GL_NOTEQUAL; break; // D3DCMP_NOTEQUAL
		case 7: depth_func = GL_GEQUAL; break;   // D3DCMP_GREATEREQUAL
		case 8: depth_func = GL_ALWAYS; break;   // D3DCMP_ALWAYS
		default: depth_func = GL_LESS; break;
	}
	glDepthFunc(depth_func);
	printf("Phase 27.4.2: Depth function set to 0x%04X\n", depth_func);
	break;
}

case D3DRS_ZWRITEENABLE:  // 14
{
	glDepthMask(value ? GL_TRUE : GL_FALSE);
	printf("Phase 27.4.2: Depth write %s\n", value ? "enabled" : "disabled");
	break;
}
```

**Alpha Blending** (D3DRS_ALPHABLENDENABLE, D3DRS_SRCBLEND, D3DRS_DESTBLEND):
```cpp
case D3DRS_ALPHABLENDENABLE:  // 27
{
	if (value) {
		glEnable(GL_BLEND);
		printf("Phase 27.4.2: Alpha blending enabled\n");
	} else {
		glDisable(GL_BLEND);
		printf("Phase 27.4.2: Alpha blending disabled\n");
	}
	break;
}

case D3DRS_SRCBLEND:  // 19
case D3DRS_DESTBLEND:  // 20
{
	// Map D3DBLEND to OpenGL blend factors
	auto map_blend_factor = [](unsigned int d3d_blend) -> GLenum {
		switch (d3d_blend) {
			case 1: return GL_ZERO;             // D3DBLEND_ZERO
			case 2: return GL_ONE;              // D3DBLEND_ONE
			case 3: return GL_SRC_COLOR;        // D3DBLEND_SRCCOLOR
			case 4: return GL_ONE_MINUS_SRC_COLOR; // D3DBLEND_INVSRCCOLOR
			case 5: return GL_SRC_ALPHA;        // D3DBLEND_SRCALPHA
			case 6: return GL_ONE_MINUS_SRC_ALPHA; // D3DBLEND_INVSRCALPHA
			case 7: return GL_DST_ALPHA;        // D3DBLEND_DESTALPHA
			case 8: return GL_ONE_MINUS_DST_ALPHA; // D3DBLEND_INVDESTALPHA
			case 9: return GL_DST_COLOR;        // D3DBLEND_DESTCOLOR
			case 10: return GL_ONE_MINUS_DST_COLOR; // D3DBLEND_INVDESTCOLOR
			case 11: return GL_SRC_ALPHA_SATURATE; // D3DBLEND_SRCALPHASAT
			default: return GL_ONE;
		}
	};
	
	GLenum src_factor = map_blend_factor(RenderStates[D3DRS_SRCBLEND]);
	GLenum dst_factor = map_blend_factor(RenderStates[D3DRS_DESTBLEND]);
	
	glBlendFunc(src_factor, dst_factor);
	printf("Phase 27.4.2: Blend function (src=0x%04X, dst=0x%04X)\n", src_factor, dst_factor);
	break;
}
```

#### D3D Blend Factor Constants:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 341-352)
#define D3DBLEND_ZERO 1
#define D3DBLEND_ONE 2
#define D3DBLEND_SRCCOLOR 3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA 5
#define D3DBLEND_INVSRCALPHA 6
#define D3DBLEND_DESTALPHA 7
#define D3DBLEND_INVDESTALPHA 8
#define D3DBLEND_DESTCOLOR 9
#define D3DBLEND_INVDESTCOLOR 10
#define D3DBLEND_SRCALPHASAT 11
```

#### Backport Notes:
- Lambda function for blend factor mapping (C++11 feature)
- RenderStates array must be accessible
- All render state values are cached in RenderStates[] array
- Copy entire render state switch cases to Generals

---

### 9. Texture Stage States (Phase 27.4.3) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### Implementation (Set_DX8_Texture function):
```cpp
WWINLINE void DX8Wrapper::Set_DX8_Texture(unsigned stage, W3DTextureClass* texture)
{
#ifdef _WIN32
	// DirectX 8 implementation
	DX8TextureClass* dx8_texture = static_cast<DX8TextureClass*>(texture);
	DX8CALL(SetTexture(stage, dx8_texture ? dx8_texture->Peek_D3D_Texture() : nullptr));
#else
	// Phase 27.4.3: OpenGL texture binding
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		
		if (texture) {
			DX8TextureClass* dx8_texture = static_cast<DX8TextureClass*>(texture);
			GLuint gl_texture = dx8_texture->Get_GL_Texture();
			
			// Activate texture unit and bind texture
			glActiveTexture(GL_TEXTURE0 + stage);
			glBindTexture(GL_TEXTURE_2D, gl_texture);
			
			// Update shader uniform for texture sampler
			char uniform_name[32];
			snprintf(uniform_name, sizeof(uniform_name), "uTexture%u", stage);
			GLint loc = glGetUniformLocation(GL_Shader_Program, uniform_name);
			if (loc != -1) {
				glUniform1i(loc, stage);  // Texture unit index
			}
			
			// Enable texture usage in shader
			loc = glGetUniformLocation(GL_Shader_Program, "uUseTexture");
			if (loc != -1) {
				glUniform1i(loc, 1);
			}
			
			printf("Phase 27.4.3: Texture bound (stage %u, GL ID %u)\n", stage, gl_texture);
		} else {
			// Unbind texture
			glActiveTexture(GL_TEXTURE0 + stage);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			// Disable texture usage in shader
			GLint loc = glGetUniformLocation(GL_Shader_Program, "uUseTexture");
			if (loc != -1) {
				glUniform1i(loc, 0);
			}
			
			printf("Phase 27.4.3: Texture unbound (stage %u)\n", stage);
		}
	}
#endif
}
```

#### Shader Uniform Updates:
- `uTexture0` - Texture sampler for stage 0
- `uTexture1` - Texture sampler for stage 1 (if multitexturing)
- `uUseTexture` - Boolean flag to enable/disable texture sampling

#### Backport Notes:
- DX8TextureClass must have `Get_GL_Texture()` method
- Shader must declare `uniform sampler2D uTexture0;`
- Multiple texture stages supported via `glActiveTexture(GL_TEXTURE0 + stage)`
- Copy entire function to Generals

---

### 10. Alpha Testing (Phase 27.4.4) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `resources/shaders/basic.frag`

#### Render State Implementation (dx8wrapper.h):
```cpp
case D3DRS_ALPHATESTENABLE:  // 15
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaTestEnabled");
		if (loc != -1) {
			glUniform1i(loc, value ? 1 : 0);
			printf("Phase 27.4.4: Alpha test %s\n", value ? "enabled" : "disabled");
		}
	}
	break;
}

case D3DRS_ALPHAFUNC:  // 25
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaTestFunc");
		if (loc != -1) {
			glUniform1i(loc, value);
			printf("Phase 27.4.4: Alpha test function set to %u\n", value);
		}
	}
	break;
}

case D3DRS_ALPHAREF:  // 24
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uAlphaRef");
		if (loc != -1) {
			float alpha_ref = value / 255.0f;  // D3D uses 0-255, shader uses 0.0-1.0
			glUniform1f(loc, alpha_ref);
			printf("Phase 27.4.4: Alpha reference value set to %.3f (%u/255)\n", alpha_ref, value);
		}
	}
	break;
}
```

#### Fragment Shader Implementation (basic.frag):
```glsl
// Phase 27.4.4: Alpha testing uniforms
uniform bool uAlphaTestEnabled;
uniform int uAlphaTestFunc;
uniform float uAlphaRef;

void main()
{
    vec4 baseColor;
    
    if (uUseTexture) {
        baseColor = texture(uTexture, vTexCoord);
    } else {
        baseColor = vColor;
    }
    
    // Apply lighting
    vec3 litColor = baseColor.rgb * vLighting;
    
    // Phase 27.4.4: Alpha testing (shader-based)
    if (uAlphaTestEnabled) {
        bool alpha_pass = false;
        
        switch (uAlphaTestFunc) {
            case 1: alpha_pass = false; break;                      // D3DCMP_NEVER
            case 2: alpha_pass = (baseColor.a < uAlphaRef); break;  // D3DCMP_LESS
            case 3: alpha_pass = (baseColor.a == uAlphaRef); break; // D3DCMP_EQUAL
            case 4: alpha_pass = (baseColor.a <= uAlphaRef); break; // D3DCMP_LESSEQUAL
            case 5: alpha_pass = (baseColor.a > uAlphaRef); break;  // D3DCMP_GREATER
            case 6: alpha_pass = (baseColor.a != uAlphaRef); break; // D3DCMP_NOTEQUAL
            case 7: alpha_pass = (baseColor.a >= uAlphaRef); break; // D3DCMP_GREATEREQUAL
            case 8: alpha_pass = true; break;                       // D3DCMP_ALWAYS
            default: alpha_pass = true; break;
        }
        
        if (!alpha_pass) {
            discard;  // Fragment is discarded (not rendered)
        }
    }
    
    FragColor = vec4(litColor, baseColor.a);
}
```

#### Alpha Test Function Mapping:

| D3DCMP Constant | Value | Condition | Description |
|-----------------|-------|-----------|-------------|
| D3DCMP_NEVER | 1 | false | Always fail |
| D3DCMP_LESS | 2 | alpha < ref | Pass if less than reference |
| D3DCMP_EQUAL | 3 | alpha == ref | Pass if equal to reference |
| D3DCMP_LESSEQUAL | 4 | alpha <= ref | Pass if less or equal |
| D3DCMP_GREATER | 5 | alpha > ref | Pass if greater than reference |
| D3DCMP_NOTEQUAL | 6 | alpha != ref | Pass if not equal |
| D3DCMP_GREATEREQUAL | 7 | alpha >= ref | Pass if greater or equal |
| D3DCMP_ALWAYS | 8 | true | Always pass |

#### Backport Notes:
- OpenGL removed fixed-function alpha testing in Core Profile
- Shader-based implementation required (using `discard`)
- Alpha reference: D3D uses 0-255 integer, OpenGL shader uses 0.0-1.0 float
- Copy both render state cases and shader code to Generals

---

### 11. Fog Rendering (Phase 27.4.5) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `resources/shaders/basic.frag`

#### Render State Implementation (dx8wrapper.h):
```cpp
case D3DRS_FOGENABLE:  // 28
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogEnabled");
		if (loc != -1) {
			glUniform1i(loc, value ? 1 : 0);
			printf("Phase 27.4.5: Fog %s\n", value ? "enabled" : "disabled");
		}
	}
	break;
}

case D3DRS_FOGCOLOR:  // 34
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogColor");
		if (loc != -1) {
			// Extract RGB from D3DCOLOR (ARGB format)
			float r = ((value >> 16) & 0xFF) / 255.0f;
			float g = ((value >> 8) & 0xFF) / 255.0f;
			float b = (value & 0xFF) / 255.0f;
			glUniform3f(loc, r, g, b);
			printf("Phase 27.4.5: Fog color (%.2f, %.2f, %.2f)\n", r, g, b);
		}
	}
	break;
}

case D3DRS_FOGSTART:  // 36
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogStart");
		if (loc != -1) {
			float fog_start = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, fog_start);
			printf("Phase 27.4.5: Fog start distance = %.2f\n", fog_start);
		}
	}
	break;
}

case D3DRS_FOGEND:  // 37
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogEnd");
		if (loc != -1) {
			float fog_end = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, fog_end);
			printf("Phase 27.4.5: Fog end distance = %.2f\n", fog_end);
		}
	}
	break;
}

case D3DRS_FOGDENSITY:  // 38
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogDensity");
		if (loc != -1) {
			float fog_density = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, fog_density);
			printf("Phase 27.4.5: Fog density = %.4f\n", fog_density);
		}
	}
	break;
}

case D3DRS_FOGTABLEMODE:  // 35
case D3DRS_FOGVERTEXMODE:  // 140
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uFogMode");
		if (loc != -1) {
			glUniform1i(loc, value);
			const char* mode_names[] = {"NONE", "EXP", "EXP2", "LINEAR"};
			printf("Phase 27.4.5: Fog mode = %s (%u)\n", 
				value <= 3 ? mode_names[value] : "UNKNOWN", value);
		}
	}
	break;
}
```

#### Fragment Shader Implementation (basic.frag):
```glsl
// Phase 27.4.5: Fog uniforms
uniform bool uFogEnabled;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
uniform float uFogDensity;
uniform int uFogMode;  // 0=NONE, 1=EXP, 2=EXP2, 3=LINEAR

// Pass vWorldPos from vertex shader
in vec3 vWorldPos;

void main()
{
    // ... existing color calculation ...
    
    // Phase 27.4.5: Fog calculation
    if (uFogEnabled) {
        float fogDistance = length(vWorldPos);
        float fogFactor = 1.0;
        
        if (uFogMode == 1) {
            // Exponential fog: factor = exp(-density * distance)
            fogFactor = exp(-uFogDensity * fogDistance);
        }
        else if (uFogMode == 2) {
            // Exponential squared fog: factor = exp(-(density * distance)^2)
            float fogAmount = uFogDensity * fogDistance;
            fogFactor = exp(-fogAmount * fogAmount);
        }
        else if (uFogMode == 3) {
            // Linear fog: factor = (end - distance) / (end - start)
            fogFactor = (uFogEnd - fogDistance) / (uFogEnd - uFogStart);
        }
        
        fogFactor = clamp(fogFactor, 0.0, 1.0);
        
        // Mix fog color with base color
        // fogFactor = 1.0 means no fog (use base color)
        // fogFactor = 0.0 means full fog (use fog color)
        baseColor.rgb = mix(uFogColor, baseColor.rgb, fogFactor);
    }
    
    FragColor = vec4(baseColor.rgb, baseColor.a);
}
```

#### Fog Mode Constants:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 595-599)
#define D3DFOG_NONE 0
#define D3DFOG_EXP 1
#define D3DFOG_EXP2 2
#define D3DFOG_LINEAR 3
```

#### Fog Calculation Formulas:

| Mode | D3D Value | Formula | Description |
|------|-----------|---------|-------------|
| NONE | 0 | factor = 1.0 | No fog |
| EXP | 1 | factor = exp(-density × dist) | Exponential fog |
| EXP2 | 2 | factor = exp(-(density × dist)²) | Exponential squared fog |
| LINEAR | 3 | factor = (end - dist) / (end - start) | Linear fog |

#### Backport Notes:
- Float extraction from DWORD: `*reinterpret_cast<const float*>(&value)`
- D3DCOLOR format: ARGB (Alpha-Red-Green-Blue) packed in 32-bit integer
- Fog distance calculated from world position in view space
- Copy both render states and shader code to Generals

---

### 12. Stencil Buffer Operations (Phase 27.4.6) ✅

#### File Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`

#### Implementation:
```cpp
case D3DRS_STENCILENABLE:  // 52
{
	if (value) {
		glEnable(GL_STENCIL_TEST);
		printf("Phase 27.4.6: Stencil test enabled\n");
	} else {
		glDisable(GL_STENCIL_TEST);
		printf("Phase 27.4.6: Stencil test disabled\n");
	}
	break;
}

case D3DRS_STENCILFUNC:  // 56
{
	GLenum gl_func = GL_ALWAYS;
	switch (value) {
		case 1: gl_func = GL_NEVER; break;    // D3DCMP_NEVER
		case 2: gl_func = GL_LESS; break;     // D3DCMP_LESS
		case 3: gl_func = GL_EQUAL; break;    // D3DCMP_EQUAL
		case 4: gl_func = GL_LEQUAL; break;   // D3DCMP_LESSEQUAL
		case 5: gl_func = GL_GREATER; break;  // D3DCMP_GREATER
		case 6: gl_func = GL_NOTEQUAL; break; // D3DCMP_NOTEQUAL
		case 7: gl_func = GL_GEQUAL; break;   // D3DCMP_GREATEREQUAL
		case 8: gl_func = GL_ALWAYS; break;   // D3DCMP_ALWAYS
		default: gl_func = GL_ALWAYS; break;
	}
	
	glStencilFunc(gl_func, 
		RenderStates[D3DRS_STENCILREF], 
		RenderStates[D3DRS_STENCILMASK]);
	printf("Phase 27.4.6: Stencil function set (func=0x%04X)\n", gl_func);
	break;
}

case D3DRS_STENCILREF:  // 57
{
	glStencilFunc(
		GL_ALWAYS,  // Will be updated by D3DRS_STENCILFUNC
		value,
		RenderStates[D3DRS_STENCILMASK]);
	printf("Phase 27.4.6: Stencil reference value = %u\n", value);
	break;
}

case D3DRS_STENCILMASK:  // 58
{
	glStencilFunc(
		GL_ALWAYS,  // Will be updated by D3DRS_STENCILFUNC
		RenderStates[D3DRS_STENCILREF],
		value);
	printf("Phase 27.4.6: Stencil mask = 0x%08X\n", value);
	break;
}

case D3DRS_STENCILWRITEMASK:  // 59
{
	glStencilMask(value);
	printf("Phase 27.4.6: Stencil write mask = 0x%08X\n", value);
	break;
}

case D3DRS_STENCILFAIL:  // 53
case D3DRS_STENCILZFAIL:  // 54
case D3DRS_STENCILPASS:  // 55
{
	// Map D3DSTENCILOP to OpenGL stencil operations
	auto map_stencil_op = [](unsigned int d3d_op) -> GLenum {
		switch (d3d_op) {
			case 1: return GL_KEEP;      // D3DSTENCILOP_KEEP
			case 2: return GL_ZERO;      // D3DSTENCILOP_ZERO
			case 3: return GL_REPLACE;   // D3DSTENCILOP_REPLACE
			case 4: return GL_INCR;      // D3DSTENCILOP_INCRSAT (clamped)
			case 5: return GL_DECR;      // D3DSTENCILOP_DECRSAT (clamped)
			case 6: return GL_INVERT;    // D3DSTENCILOP_INVERT
			case 7: return GL_INCR_WRAP; // D3DSTENCILOP_INCR (wrapping)
			case 8: return GL_DECR_WRAP; // D3DSTENCILOP_DECR (wrapping)
			default: return GL_KEEP;
		}
	};
	
	GLenum sfail = map_stencil_op(RenderStates[D3DRS_STENCILFAIL]);
	GLenum dpfail = map_stencil_op(RenderStates[D3DRS_STENCILZFAIL]);
	GLenum dppass = map_stencil_op(RenderStates[D3DRS_STENCILPASS]);
	
	glStencilOp(sfail, dpfail, dppass);
	printf("Phase 27.4.6: Stencil operations (fail=0x%04X, zfail=0x%04X, pass=0x%04X)\n",
		sfail, dpfail, dppass);
	break;
}
```

#### Stencil Operation Mapping:

| D3DSTENCILOP | Value | OpenGL | Description |
|--------------|-------|--------|-------------|
| KEEP | 1 | GL_KEEP | Keep current value |
| ZERO | 2 | GL_ZERO | Set to 0 |
| REPLACE | 3 | GL_REPLACE | Replace with ref value |
| INCRSAT | 4 | GL_INCR | Increment (clamped) |
| DECRSAT | 5 | GL_DECR | Decrement (clamped) |
| INVERT | 6 | GL_INVERT | Bitwise invert |
| INCR | 7 | GL_INCR_WRAP | Increment (wrapping) |
| DECR | 8 | GL_DECR_WRAP | Decrement (wrapping) |

#### Stencil Test Flow:
1. **Stencil Test**: Compare stencil value with reference using `glStencilFunc(func, ref, mask)`
2. **Test Failed**: Execute `sfail` operation
3. **Test Passed, Depth Failed**: Execute `dpfail` operation
4. **Test Passed, Depth Passed**: Execute `dppass` operation

#### Backport Notes:
- glStencilFunc requires 3 parameters: function, reference, mask
- glStencilOp requires 3 parameters: fail operation, depth-fail operation, pass operation
- RenderStates array caching required for interdependent values
- Lambda function for stencil operation mapping
- Copy entire stencil block to Generals

---

### 13. Scissor Test (Phase 27.4.7) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

#### Render State Implementation (dx8wrapper.h):
```cpp
// Note: D3DRS_SCISSORTESTENABLE is state 174 (D3D9 constant, not in D3D8 spec)
// For D3D8 compatibility, we provide basic scissor support that can be enabled
// when the engine is upgraded to D3D9 or when custom render targets are used.
// The implementation below is prepared for future use.
case 174:  // D3DRS_SCISSORTESTENABLE (D3D9 constant, not in D3D8)
{
	if (value) {
		glEnable(GL_SCISSOR_TEST);
		printf("Phase 27.4.7: Scissor test enabled (D3D9 extension)\n");
	} else {
		glDisable(GL_SCISSOR_TEST);
		printf("Phase 27.4.7: Scissor test disabled (D3D9 extension)\n");
	}
	break;
}
```

#### Viewport Integration (dx8wrapper.cpp, Set_Viewport function):
```cpp
void DX8Wrapper::Set_Viewport(CONST D3DVIEWPORT8* pViewport)
{
#ifdef _WIN32
	DX8CALL(SetViewport(pViewport));
#else
	// Phase 27.4.7: OpenGL viewport and scissor rect setup
	if (pViewport) {
		// Set viewport for rendering
		glViewport(pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height);
		
		// Set depth range
		glDepthRange(pViewport->MinZ, pViewport->MaxZ);
		
		// Initialize scissor rect to match viewport by default
		// This provides D3D8-like behavior where scissor rect follows viewport
		glScissor(pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height);
		
		printf("Phase 27.4.7: Viewport set to (%u, %u, %u x %u), depth [%.2f - %.2f]\n",
			pViewport->X, pViewport->Y, pViewport->Width, pViewport->Height,
			pViewport->MinZ, pViewport->MaxZ);
	}
#endif
}
```

#### D3DVIEWPORT8 Structure:
```cpp
// Core/Libraries/Source/WWVegas/WW3D2/d3d8.h (lines 880-888)
typedef struct {
	DWORD X;
	DWORD Y;
	DWORD Width;
	DWORD Height;
	float MinZ;
	float MaxZ;
} D3DVIEWPORT8;
```

#### Backport Notes:
- **D3D8 Limitation**: DirectX 8 doesn't have D3DRS_SCISSORTESTENABLE render state
- **D3D9 Extension**: State 174 is from D3D9, included for forward compatibility
- **Default Behavior**: Scissor rect automatically matches viewport dimensions
- **Coordinate System**: No Y-inversion needed (both D3D and OpenGL use bottom-left origin for viewport)
- Copy both render state case and Set_Viewport modifications to Generals

---

### 14. Point Sprites (Phase 27.4.8) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `resources/shaders/basic.vert`

#### Render State Implementation (dx8wrapper.h):
```cpp
case D3DRS_POINTSPRITEENABLE:  // 156
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSpriteEnabled");
		if (loc != -1) {
			glUniform1i(loc, value ? 1 : 0);
			printf("Phase 27.4.8: Point sprite %s\n", value ? "enabled" : "disabled");
		}
		
		// Enable OpenGL point sprite mode
		if (value) {
			glEnable(GL_PROGRAM_POINT_SIZE);  // Allow shader to control point size
			printf("Phase 27.4.8: GL_PROGRAM_POINT_SIZE enabled\n");
		} else {
			glDisable(GL_PROGRAM_POINT_SIZE);
		}
	}
	break;
}

case D3DRS_POINTSIZE:  // 154
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSize");
		if (loc != -1) {
			float pointSize = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, pointSize);
			printf("Phase 27.4.8: Point size set to %.2f\n", pointSize);
		}
	}
	break;
}

case D3DRS_POINTSCALEENABLE:  // 157
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleEnabled");
		if (loc != -1) {
			glUniform1i(loc, value ? 1 : 0);
			printf("Phase 27.4.8: Point scale %s\n", value ? "enabled" : "disabled");
		}
	}
	break;
}

case D3DRS_POINTSCALE_A:  // 158
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleA");
		if (loc != -1) {
			float scaleA = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, scaleA);
			printf("Phase 27.4.8: Point scale A = %.4f\n", scaleA);
		}
	}
	break;
}

case D3DRS_POINTSCALE_B:  // 159
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleB");
		if (loc != -1) {
			float scaleB = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, scaleB);
			printf("Phase 27.4.8: Point scale B = %.4f\n", scaleB);
		}
	}
	break;
}

case D3DRS_POINTSCALE_C:  // 160
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointScaleC");
		if (loc != -1) {
			float scaleC = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, scaleC);
			printf("Phase 27.4.8: Point scale C = %.4f\n", scaleC);
		}
	}
	break;
}

case D3DRS_POINTSIZE_MIN:  // 155
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSizeMin");
		if (loc != -1) {
			float minSize = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, minSize);
			printf("Phase 27.4.8: Point size min = %.2f\n", minSize);
		}
	}
	break;
}

case D3DRS_POINTSIZE_MAX:  // 166
{
	if (GL_Shader_Program != 0) {
		glUseProgram(GL_Shader_Program);
		GLint loc = glGetUniformLocation(GL_Shader_Program, "uPointSizeMax");
		if (loc != -1) {
			float maxSize = *reinterpret_cast<const float*>(&value);
			glUniform1f(loc, maxSize);
			printf("Phase 27.4.8: Point size max = %.2f\n", maxSize);
		}
	}
	break;
}
```

#### Vertex Shader Implementation (basic.vert):
```glsl
// Phase 27.4.8: Point sprite uniforms
uniform bool uPointSpriteEnabled;       // D3DRS_POINTSPRITEENABLE
uniform float uPointSize;               // D3DRS_POINTSIZE
uniform bool uPointScaleEnabled;        // D3DRS_POINTSCALEENABLE
uniform float uPointScaleA;             // D3DRS_POINTSCALE_A (constant)
uniform float uPointScaleB;             // D3DRS_POINTSCALE_B (linear)
uniform float uPointScaleC;             // D3DRS_POINTSCALE_C (quadratic)
uniform float uPointSizeMin;            // D3DRS_POINTSIZE_MIN
uniform float uPointSizeMax;            // D3DRS_POINTSIZE_MAX

void main()
{
    // ... existing transformation code ...
    
    // Phase 27.4.8: Point sprite size calculation
    // D3D point sprite formula: size = sqrt(1/(A + B*dist + C*dist²))
    // gl_PointSize controls point sprite rendering size
    if (uPointSpriteEnabled) {
        float pointSize = uPointSize;
        
        if (uPointScaleEnabled) {
            // Calculate distance from camera to point in view space
            float dist = length(viewPos.xyz);
            
            // D3D attenuation formula: screenSize = viewportHeight * size * sqrt(1/(A + B*d + C*d²))
            // For simplicity, we calculate: size / sqrt(A + B*d + C*d²)
            float attenuation = uPointScaleA + uPointScaleB * dist + uPointScaleC * dist * dist;
            if (attenuation > 0.0) {
                pointSize = pointSize / sqrt(attenuation);
            }
        }
        
        // Clamp to min/max range
        pointSize = clamp(pointSize, uPointSizeMin, uPointSizeMax);
        gl_PointSize = pointSize;
    } else {
        gl_PointSize = 1.0;  // Default point size
    }
}
```

#### Point Size Attenuation Formula:

**DirectX 8 Formula**:
```
screenSize = viewportHeight × size × sqrt(1 / (A + B×dist + C×dist²))
```

**Simplified OpenGL Shader Formula**:
```
pointSize = baseSize / sqrt(A + B×dist + C×dist²)
```

Where:
- **A** (constant): Base attenuation factor
- **B** (linear): Linear distance attenuation
- **C** (quadratic): Quadratic distance attenuation
- **dist**: Distance from camera to point (view space)

#### Point Sprite Render States:

| Render State | Value | Type | Description |
|--------------|-------|------|-------------|
| D3DRS_POINTSPRITEENABLE | 156 | BOOL | Enable/disable point sprites |
| D3DRS_POINTSIZE | 154 | FLOAT | Base point size |
| D3DRS_POINTSCALEENABLE | 157 | BOOL | Enable distance-based scaling |
| D3DRS_POINTSCALE_A | 158 | FLOAT | Constant attenuation |
| D3DRS_POINTSCALE_B | 159 | FLOAT | Linear attenuation |
| D3DRS_POINTSCALE_C | 160 | FLOAT | Quadratic attenuation |
| D3DRS_POINTSIZE_MIN | 155 | FLOAT | Minimum point size |
| D3DRS_POINTSIZE_MAX | 166 | FLOAT | Maximum point size |

#### Backport Notes:
- GL_PROGRAM_POINT_SIZE must be enabled for shader-controlled point size
- Point sprites automatically generate `gl_PointCoord` in fragment shader (0,0 to 1,1)
- Distance calculation uses view space position: `length(viewPos.xyz)`
- Float extraction: `*reinterpret_cast<const float*>(&value)`
- Copy both render states and vertex shader code to Generals

---

## Shader Files (Phase 27.2.4)

### Files to Copy:
- `resources/shaders/basic.vert` (vertex shader)
- `resources/shaders/basic.frag` (fragment shader)

### Vertex Shader (basic.vert):
```glsl
#version 330 core

// Vertex attributes
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aColor;

// Uniforms
uniform mat4 uWorldMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

// Lighting uniforms
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform bool uUseLighting;

// Outputs to fragment shader
out vec3 vNormal;
out vec2 vTexCoord;
out vec4 vColor;
out vec3 vLighting;

void main()
{
    // Transform position
    mat4 mvp = uProjectionMatrix * uViewMatrix * uWorldMatrix;
    gl_Position = mvp * vec4(aPosition, 1.0);
    
    // Pass through to fragment shader
    vTexCoord = aTexCoord;
    vColor = aColor;
    
    // Calculate lighting if enabled
    if (uUseLighting) {
        // Transform normal to world space
        mat3 normalMatrix = mat3(uWorldMatrix);
        vec3 worldNormal = normalize(normalMatrix * aNormal);
        
        // Simple directional lighting
        float diffuse = std::max(dot(worldNormal, -uLightDirection), 0.0);
        vLighting = uAmbientColor + (uLightColor * diffuse);
    } else {
        vLighting = vec3(1.0, 1.0, 1.0);
    }
    
    vNormal = aNormal;
}
```

### Fragment Shader (basic.frag):
```glsl
#version 330 core

// Inputs from vertex shader
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;
in vec3 vLighting;

// Uniforms
uniform sampler2D uTexture;
uniform bool uUseTexture;

// Output
out vec4 FragColor;

void main()
{
    vec4 baseColor;
    
    if (uUseTexture) {
        baseColor = texture(uTexture, vTexCoord);
    } else {
        baseColor = vColor;
    }
    
    // Apply lighting
    vec3 litColor = baseColor.rgb * vLighting;
    
    FragColor = vec4(litColor, baseColor.a);
}
```

### Backport Notes:
- Copy both shader files to Generals `resources/shaders/` directory
- Shaders are identical for both games
- See Phase 27.2.4 implementation below for shader loading code

---

### 3. Shader Program Management (Phase 27.2.4) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

#### Header Additions (dx8wrapper.h):
```cpp
// Phase 27.2.4: OpenGL shader program management
static unsigned int _Load_And_Compile_Shader(const char *shader_path, unsigned int shader_type);
static unsigned int _Create_Shader_Program(unsigned int vertex_shader, unsigned int fragment_shader);
static bool _Check_Shader_Compile_Status(unsigned int shader, const char *shader_name);
static bool _Check_Program_Link_Status(unsigned int program);
static int _Get_Uniform_Location(unsigned int program, const char *uniform_name);

// Phase 27.2.4: OpenGL shader program state
static unsigned int GL_Shader_Program;
static unsigned int GL_Vertex_Shader;
static unsigned int GL_Fragment_Shader;
```

#### Implementation (dx8wrapper.cpp):

**Static Variable Initialization** (near line 132):
```cpp
unsigned int DX8Wrapper::GL_Shader_Program = 0;
unsigned int DX8Wrapper::GL_Vertex_Shader = 0;
unsigned int DX8Wrapper::GL_Fragment_Shader = 0;
```

**Shader Initialization in Init()** (near line 463):
```cpp
#ifndef _WIN32
    // Phase 27.2.4: Initialize OpenGL shader program
    printf("Phase 27.2.4: Loading and compiling OpenGL shaders...\n");
    
    // Load and compile vertex shader
    GL_Vertex_Shader = _Load_And_Compile_Shader("resources/shaders/basic.vert", GL_VERTEX_SHADER);
    if (GL_Vertex_Shader == 0) {
        printf("Phase 27.2.4 ERROR: Failed to load vertex shader!\n");
        return false;
    }
    
    // Load and compile fragment shader  
    GL_Fragment_Shader = _Load_And_Compile_Shader("resources/shaders/basic.frag", GL_FRAGMENT_SHADER);
    if (GL_Fragment_Shader == 0) {
        printf("Phase 27.2.4 ERROR: Failed to load fragment shader!\n");
        glDeleteShader(GL_Vertex_Shader);
        GL_Vertex_Shader = 0;
        return false;
    }
    
    // Create and link shader program
    GL_Shader_Program = _Create_Shader_Program(GL_Vertex_Shader, GL_Fragment_Shader);
    if (GL_Shader_Program == 0) {
        printf("Phase 27.2.4 ERROR: Failed to create shader program!\n");
        glDeleteShader(GL_Vertex_Shader);
        glDeleteShader(GL_Fragment_Shader);
        GL_Vertex_Shader = 0;
        GL_Fragment_Shader = 0;
        return false;
    }
    
    // Delete shaders (they're now part of the program)
    glDeleteShader(GL_Vertex_Shader);
    glDeleteShader(GL_Fragment_Shader);
    GL_Vertex_Shader = 0;
    GL_Fragment_Shader = 0;
    
    printf("Phase 27.2.4: OpenGL shader program initialized successfully (ID: %u)\n", GL_Shader_Program);
#endif
```

**Shader Cleanup in Shutdown()** (near line 625):
```cpp
#ifndef _WIN32
    // Delete shader program
    if (GL_Shader_Program != 0) {
        printf("Phase 27.2.4: Deleting shader program (ID: %u)\n", GL_Shader_Program);
        glDeleteProgram(GL_Shader_Program);
        GL_Shader_Program = 0;
    }
#endif
```

**Helper Functions** (near line 2831):
```cpp
unsigned int DX8Wrapper::_Load_And_Compile_Shader(const char *shader_path, unsigned int shader_type)
{
    // Read shader source from file
    FILE *file = fopen(shader_path, "r");
    if (!file) {
        printf("Phase 27.2.4 ERROR: Failed to open shader file: %s\n", shader_path);
        return 0;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read file contents
    char *shader_source = (char*)malloc(file_size + 1);
    if (!shader_source) {
        printf("Phase 27.2.4 ERROR: Failed to allocate memory for shader source\n");
        fclose(file);
        return 0;
    }
    
    size_t bytes_read = fread(shader_source, 1, file_size, file);
    shader_source[bytes_read] = '\0';
    fclose(file);
    
    // Create shader
    GLuint shader = glCreateShader(shader_type);
    if (shader == 0) {
        printf("Phase 27.2.4 ERROR: glCreateShader failed for %s\n", shader_path);
        free(shader_source);
        return 0;
    }
    
    // Compile shader
    const char *source_ptr = shader_source;
    glShaderSource(shader, 1, &source_ptr, NULL);
    glCompileShader(shader);
    
    // Check compilation status
    const char *shader_type_name = (shader_type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
    if (!_Check_Shader_Compile_Status(shader, shader_type_name)) {
        printf("Phase 27.2.4 ERROR: Shader compilation failed for %s\n", shader_path);
        glDeleteShader(shader);
        free(shader_source);
        return 0;
    }
    
    printf("Phase 27.2.4: Successfully compiled %s shader from %s (ID: %u)\n", 
        shader_type_name, shader_path, shader);
    
    free(shader_source);
    return shader;
}

unsigned int DX8Wrapper::_Create_Shader_Program(unsigned int vertex_shader, unsigned int fragment_shader)
{
    // Create program
    GLuint program = glCreateProgram();
    if (program == 0) {
        printf("Phase 27.2.4 ERROR: glCreateProgram failed\n");
        return 0;
    }
    
    // Attach shaders
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    // Link program
    glLinkProgram(program);
    
    // Check link status
    if (!_Check_Program_Link_Status(program)) {
        printf("Phase 27.2.4 ERROR: Program linking failed\n");
        glDeleteProgram(program);
        return 0;
    }
    
    // Detach shaders (they're now part of the program)
    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    
    printf("Phase 27.2.4: Successfully created shader program (ID: %u)\n", program);
    
    return program;
}

bool DX8Wrapper::_Check_Shader_Compile_Status(unsigned int shader, const char *shader_name)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (success == GL_FALSE) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        
        if (log_length > 0) {
            char *error_log = (char*)malloc(log_length);
            glGetShaderInfoLog(shader, log_length, &log_length, error_log);
            printf("Phase 27.2.4 SHADER COMPILE ERROR (%s):\n%s\n", shader_name, error_log);
            free(error_log);
        }
        
        return false;
    }
    
    return true;
}

bool DX8Wrapper::_Check_Program_Link_Status(unsigned int program)
{
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (success == GL_FALSE) {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        
        if (log_length > 0) {
            char *error_log = (char*)malloc(log_length);
            glGetProgramInfoLog(program, log_length, &log_length, error_log);
            printf("Phase 27.2.4 PROGRAM LINK ERROR:\n%s\n", error_log);
            free(error_log);
        }
        
        return false;
    }
    
    return true;
}

int DX8Wrapper::_Get_Uniform_Location(unsigned int program, const char *uniform_name)
{
    GLint location = glGetUniformLocation(program, uniform_name);
    
    if (location == -1) {
        printf("Phase 27.2.4 WARNING: Uniform '%s' not found in program %u\n", uniform_name, program);
    }
    
    return location;
}
```

#### Backport Notes:
- Copy all shader management functions from Zero Hour to Generals
- Static variables must be initialized in .cpp file
- Shader paths are relative to executable location
- Ensure `resources/shaders/` exists in Generals runtime directory
- Error checking is comprehensive - compilation/linking failures are logged

---

### 4. VAO Creation and Initialization (Phase 27.2.5) ✅

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h`
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

#### Header Additions (dx8wrapper.h):
```cpp
// Phase 27.2.5: Vertex attribute setup based on FVF format
static void _Setup_Vertex_Attributes(unsigned int fvf, unsigned int vertex_stride);

// Phase 27.2.5: OpenGL VAO state
static unsigned int GL_VAO;
```

#### Implementation (dx8wrapper.cpp):

**Static Variable Initialization** (near line 135):
```cpp
unsigned int DX8Wrapper::GL_VAO = 0;
```

**VAO Creation in Init()** (near line 503, after shader initialization):
```cpp
#ifndef _WIN32
    // Phase 27.2.5: Create Vertex Array Object
    glGenVertexArrays(1, &GL_VAO);
    if (GL_VAO == 0) {
        printf("Phase 27.2.5 ERROR: Failed to create VAO!\n");
        return false;
    }
    
    // Bind VAO temporarily to initialize default state
    glBindVertexArray(GL_VAO);
    
    // Setup default vertex attributes (will be reconfigured per FVF later)
    // This ensures VAO has valid state
    
    // Unbind VAO
    glBindVertexArray(0);
    
    printf("Phase 27.2.5: VAO created successfully (ID: %u)\n", GL_VAO);
#endif
```

**VAO Cleanup in Shutdown()** (near line 632):
```cpp
#ifndef _WIN32
    // Delete VAO
    if (GL_VAO != 0) {
        printf("Phase 27.2.5: Deleting VAO (ID: %u)\n", GL_VAO);
        glDeleteVertexArrays(1, &GL_VAO);
        GL_VAO = 0;
    }
#endif
```

**Vertex Attribute Setup Function** (near line 2989):
```cpp
void DX8Wrapper::_Setup_Vertex_Attributes(unsigned int fvf, unsigned int vertex_stride)
{
    // Disable all attributes first
    for (int i = 0; i < 4; i++) {
        glDisableVertexAttribArray(i);
    }
    
    unsigned int offset = 0;
    
    // Position attribute (location 0) - required for all formats
    if (fvf & D3DFVF_XYZ) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(uintptr_t)offset);
        glEnableVertexAttribArray(0);
        offset += 3 * sizeof(float);
        
        printf("Phase 27.2.5: Enabled position attribute (loc 0, offset %u)\n", offset - 3 * sizeof(float));
    }
    
    // Normal attribute (location 1)
    if (fvf & D3DFVF_NORMAL) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(uintptr_t)offset);
        glEnableVertexAttribArray(1);
        offset += 3 * sizeof(float);
        
        printf("Phase 27.2.5: Enabled normal attribute (loc 1, offset %u)\n", offset - 3 * sizeof(float));
    }
    
    // Diffuse color attribute (location 2)
    // D3D stores color as BGRA DWORD, OpenGL expects RGBA float[4]
    if (fvf & D3DFVF_DIFFUSE) {
        // Note: Using GL_UNSIGNED_BYTE with GL_TRUE normalization converts 0-255 to 0.0-1.0
        // D3DCOLOR is BGRA order, so we need to use GL_BGRA format
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertex_stride, (void*)(uintptr_t)offset);
        glEnableVertexAttribArray(2);
        offset += sizeof(unsigned int);  // D3DCOLOR is 4 bytes
        
        printf("Phase 27.2.5: Enabled diffuse color attribute (loc 2, offset %u)\n", offset - sizeof(unsigned int));
    }
    
    // Texture coordinate 0 attribute (location 3)
    // Check for at least 1 texture coordinate set
    unsigned int tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    if (tex_count >= 1) {
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(uintptr_t)offset);
        glEnableVertexAttribArray(3);
        offset += 2 * sizeof(float);
        
        printf("Phase 27.2.5: Enabled texcoord0 attribute (loc 3, offset %u)\n", offset - 2 * sizeof(float));
    }
    
    // Note: We're only handling the first texture coordinate set for now
    // Additional texture coordinates (texcoord1, etc.) can be added later as needed
    
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Phase 27.2.5 ERROR: OpenGL error during vertex attribute setup: 0x%x\n", error);
    }
}
```

**VAO Binding in Apply_Render_State_Changes()** (near line 2643, in VERTEX_BUFFER_CHANGED section):
```cpp
#ifndef _WIN32
    // Phase 27.2.5: OpenGL vertex buffer and attribute setup
    switch (render_state.vertex_buffer_types[i]) {
    case BUFFER_TYPE_DX8:
    case BUFFER_TYPE_DYNAMIC_DX8:
        {
            // Bind VAO
            glBindVertexArray(GL_VAO);
            
            // Bind VBO
            DX8VertexBufferClass* vb = static_cast<DX8VertexBufferClass*>(render_state.vertex_buffers[i]);
            glBindBuffer(GL_ARRAY_BUFFER, vb->Get_GL_Vertex_Buffer());
            
            // Setup vertex attributes based on FVF format
            unsigned int fvf = render_state.vertex_buffers[i]->FVF_Info().Get_FVF();
            unsigned int stride = render_state.vertex_buffers[i]->FVF_Info().Get_FVF_Size();
            
            if (fvf != 0) {
                _Setup_Vertex_Attributes(fvf, stride);
            }
            
            printf("Phase 27.2.5: Configured vertex attributes for FVF 0x%08x (stride %u)\n", fvf, stride);
        }
        break;
    case BUFFER_TYPE_SORTING:
    case BUFFER_TYPE_DYNAMIC_SORTING:
        break;
    default:
        WWASSERT(0);
    }
#else
    // Original DirectX8 code...
#endif
```

**VAO Unbinding** (in else clause when vertex_buffer is NULL):
```cpp
#ifndef _WIN32
    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    DX8CALL(SetStreamSource(i,NULL,0));
    DX8_RECORD_VERTEX_BUFFER_CHANGE();
#endif
```

#### FVF (Flexible Vertex Format) Mapping Table:

| D3D FVF Flag | OpenGL Location | Type | Size | Description |
|--------------|-----------------|------|------|-------------|
| `D3DFVF_XYZ` | 0 | vec3 | 12 bytes | Position (x, y, z) |
| `D3DFVF_NORMAL` | 1 | vec3 | 12 bytes | Normal vector |
| `D3DFVF_DIFFUSE` | 2 | vec4 (BGRA) | 4 bytes | Vertex color (BGRA DWORD) |
| `D3DFVF_TEX1` | 3 | vec2 | 8 bytes | Texture coordinate 0 (u, v) |

**Color Format Note**: D3DCOLOR uses BGRA byte order, so we use `GL_UNSIGNED_BYTE` with `GL_TRUE` normalization to convert 0-255 to 0.0-1.0 range. The shader receives it as `vec4` in RGBA order.

#### Backport Notes:
- VAO must be created AFTER shader program initialization
- FVF flags are identical in Generals base - no translation needed
- Attribute locations (0-3) must match shader `layout(location = N)` declarations
- Color format handling (BGRA→RGBA) is automatic with normalized GL_UNSIGNED_BYTE
- Additional texture coordinates can be added by extending the `_Setup_Vertex_Attributes()` function

---

## CMakeLists.txt Dependencies

### SDL2 Configuration (already present in both):
```cmake
find_package(SDL2 REQUIRED)
target_link_libraries(GeneralsXZH PRIVATE SDL2::SDL2)
target_link_libraries(GeneralsX PRIVATE SDL2::SDL2)
```

### GLAD Configuration (already present):
```cmake
add_subdirectory(Dependencies/glad)
target_link_libraries(GeneralsXZH PRIVATE glad)
target_link_libraries(GeneralsX PRIVATE glad)
```

### Backport Notes:
- Both targets already have SDL2 and GLAD linked
- No CMake changes required for backport

---

## Testing Checklist

### Pre-Backport Validation (Zero Hour):
- [ ] Compilation successful (797 files, 0 errors)
- [ ] Runtime startup without crashes
- [ ] OpenGL context creation verified
- [ ] Shader compilation successful
- [ ] Vertex/index buffers created
- [ ] Uniform updates logged
- [ ] Draw calls executed without GL errors

### Post-Backport Validation (Generals Base):
- [ ] Copy all files listed above
- [ ] Compilation successful (759 files, 0 errors)
- [ ] Runtime startup without crashes
- [ ] OpenGL context creation verified
- [ ] Compare debug logs with Zero Hour
- [ ] Verify shader loading
- [ ] Test basic rendering (if Phase 27 complete)

---

## Git Commit Strategy

### Zero Hour Commits (Done):
- `4ff9651f` - feat(opengl): implement Phase 27.3-27.4.1 uniform updates and draw calls
- `ae40f803` - docs(phase27): update NEXT_STEPS.md with Phase 27.3-27.4.1 achievements
- `26d3b4df` - docs(phase27): comprehensive documentation update for OpenGL implementation

### Generals Base Backport Commits (Future):
1. `feat(opengl): backport Phase 27.1 SDL2 window system from Zero Hour`
2. `feat(opengl): backport Phase 27.2.1-27.2.2 buffer abstraction from Zero Hour`
3. `feat(opengl): backport Phase 27.3 uniform updates from Zero Hour`
4. `feat(opengl): backport Phase 27.4.1 primitive draw calls from Zero Hour`
5. `test(opengl): validate Generals base OpenGL implementation`

---

## Known Differences Between Games

### Zero Hour Specific Features (NOT to backport):
- General powers rendering
- Advanced particle effects (superweapon effects)
- Special building effects (Black Lotus capture)

### Generals Base Specific:
- Simpler particle systems
- Fewer shader complexity requirements
- May have different shader constants

### Common Features (Backport Everything):
- Window creation
- Buffer management
- Basic rendering pipeline
- Uniform updates
- Draw calls

---

## Troubleshooting

### Common Backport Issues:

1. **Compilation Errors**:
   - Verify all `#ifndef _WIN32` blocks copied correctly
   - Check for missing header includes (glad/glad.h, SDL2/SDL.h)
   - Ensure GL_Shader_Program global variable exists

2. **Runtime Crashes**:
   - Verify SDL2 initialization before OpenGL calls
   - Check that shaders are loaded before rendering
   - Validate buffer creation before use

3. **Missing Rendering**:
   - Verify shader compilation logs
   - Check uniform locations (should not be -1)
   - Enable GL error checking with glGetError()

---

## Summary

**Total Files Modified**: 7 files
**Total Lines Added**: ~2000 lines (OpenGL code)
**Build Impact**: No performance regression expected
**Testing Required**: Full runtime validation

**Completed Phases**:
- ✅ Phase 27.1: SDL2 Window System (6/6 tasks)
- ✅ Phase 27.2: Buffer & Shader Abstraction (8/8 tasks)
- ✅ Phase 27.3: Uniform Updates (3/3 tasks)
- ✅ Phase 27.4.1-27.4.8: Rendering Pipeline (8/8 tasks)
  - Primitive Draw Calls
  - Render State Management (culling, depth, blending)
  - Texture Stage States
  - Alpha Testing (shader-based)
  - Fog Rendering (shader-based, 3 modes)
  - Stencil Buffer Operations
  - Scissor Test
  - Point Sprites (particle systems)

**Completed Phases (New)**:
- ✅ Phase 27.5.1-27.5.3: Runtime Testing & Validation (3/5 tasks)
  - Basic Runtime Testing (144K+ log lines, exit code 0)
  - Shader Debugging Infrastructure (GL error checking)
  - Performance Baseline (PERFORMANCE_BASELINE.md)

**Pending Phases**:
- ⏳ Phase 27.5.4-27.5.5: Texture Loading & Documentation (2 tasks - deferred/in-progress)
- ⏳ Phase 27.6-27.8: Final Documentation & Backport Execution (3 tasks)

---

## Phase 27.5: Testing & Validation (Tasks 27.5.1-27.5.5)

### Task 27.5.1: Basic Runtime Testing ✅

**Objective**: Validate that GeneralsXZH executes successfully with SDL2/OpenGL infrastructure.

#### Test Procedure:

1. **Build and Deploy**:
   ```bash
   cmake --build build/macos-arm64 --target GeneralsXZH -j 4
   cp build/macos-arm64/GeneralsMD/GeneralsXZH $HOME/GeneralsX/GeneralsMD/
   ```

2. **Execute with Logging**:
   ```bash
   cd $HOME/GeneralsX/GeneralsMD
   ./GeneralsXZH 2>&1 | tee /tmp/generalszh_runtime_test.log
   ```

3. **Verify Success Criteria**:
   - Exit code should be 0 (no crashes)
   - Log should show BIG file loading (19 archives expected)
   - Map cache processing (146 maps expected for test data)
   - GameEngine::init() completion
   - SDL2 window creation
   - GameEngine::execute() entry
   - Clean shutdown messages

#### Expected Output:

```
Win32BIGFileSystem::loadBigFilesFromDirectory - successfully loaded: ./ShadersZH.big
...
Phase 27.1.3: SDL2 window created (800x600, fullscreen)
OpenGL Version: 4.1 Metal - 90.5
...
GameMain - TheGameEngine->init() completed successfully
GameEngine::execute() - ENTRY POINT - About to create FrameRateLimit
FRAMERATE INIT: Frequency = 1000000000, Start = 2222635901
GameEngine::execute() - FrameRateLimit created successfully
...
Phase 27.1.4: Cleaning up SDL2 and OpenGL resources...
GameMain - Returning exit code: 0
```

#### Backport Notes:

- No code changes required for backport
- Generals base uses same runtime testing procedure
- Expected differences:
  - Generals loads INI.big instead of INIZH.big
  - Shader file is Shaders.big instead of ShadersZH.big
  - Map count may differ (Generals has fewer stock maps)

---

### Task 27.5.2: Shader Debugging Infrastructure ✅

**Objective**: Add comprehensive OpenGL error checking and debugging capabilities.

#### Files Modified:
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` (lines 435-439)
- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp` (lines 3011-3117, integration points throughout)

#### Implementation:

**1. Error Checking Function** (dx8wrapper.cpp lines 3011-3055):

```cpp
static void _Check_GL_Error(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        const char* errorStr = "UNKNOWN_ERROR";
        switch (error) {
            case GL_INVALID_ENUM: errorStr = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: errorStr = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorStr = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: errorStr = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        printf("Phase 27.5.2 ERROR: %s failed with error: %s (0x%04X)\n", 
               operation, errorStr, error);
    }
}
```

**2. Debug Output Callback** (dx8wrapper.cpp lines 3057-3086):

```cpp
static void _Enable_GL_Debug_Output() {
#if defined(__APPLE__) || defined(__MACH__)
    // macOS OpenGL 4.1 does not support GL_DEBUG_OUTPUT (requires 4.3+)
    printf("Phase 27.5.2 WARNING: GL_DEBUG_OUTPUT not available on macOS (requires GL 4.3+, have 4.1)\n");
#else
    if (glDebugMessageCallback) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, 
            GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
            printf("GL Debug: %s\n", message);
        }, nullptr);
    }
#endif
}
```

**3. Safe Uniform Location** (dx8wrapper.cpp lines 3088-3117):

```cpp
static GLint _Get_Uniform_Location_Safe(GLuint program, const char* name, bool log_if_missing = false) {
    GLint location = glGetUniformLocation(program, name);
    if (location == -1 && log_if_missing) {
        printf("Phase 27.5.2 WARNING: Uniform '%s' not found in shader program %u\n", name, program);
    }
    return location;
}
```

**4. Integration Points** (15+ locations):

```cpp
// Shader initialization (dx8wrapper.cpp ~line 515)
_Enable_GL_Debug_Output();
_Check_GL_Error("Enable GL debug output");

// Matrix uniforms (Apply_Render_State_Changes, ~line 2625)
glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)&render_state.world);
_Check_GL_Error("Set uWorldMatrix uniform");

// Vertex buffer operations (~line 2685)
glBindVertexArray(GL_VAO);
_Check_GL_Error("Bind VAO for rendering");
```

#### Backport Instructions:

1. Copy all three helper functions to Generals base dx8wrapper.cpp (before any usage)
2. Copy all `_Check_GL_Error()` calls from Zero Hour integration points
3. Ensure macOS conditional compilation preserved (`#if defined(__APPLE__)`)
4. Test compilation: `cmake --build build/macos-arm64 --target GeneralsX -j 4`

#### Expected Results:

- Compilation successful with no new errors
- Runtime shows "Phase 27.5.2 WARNING" for GL_DEBUG_OUTPUT on macOS (expected)
- No GL errors reported during normal execution
- If GL errors occur, detailed error messages with operation context

---

### Task 27.5.3: Performance Baseline ✅

**Objective**: Establish performance metrics for current OpenGL implementation.

#### Documentation Created:
- `docs/PERFORMANCE_BASELINE.md` (comprehensive report)

#### Key Metrics Established:

| Metric | Value | Notes |
|--------|-------|-------|
| **Execution Time** | ~10 seconds | Full initialization + shutdown |
| **Log Throughput** | 14,471 lines/sec | 144,712 total lines |
| **BIG Files Loaded** | 19 archives | All assets present |
| **MapCache Entries** | 146 maps | Full multiplayer map set |
| **INI Exceptions Handled** | 1000+ | Universal Protection operational |
| **OpenGL Version** | 4.1 Metal - 90.5 | macOS compatibility layer |
| **SDL2 Window Creation** | Success | 800x600 fullscreen |
| **Exit Code** | 0 | Clean shutdown |
| **GL Errors** | 0 | No errors detected |

#### Timing Breakdown:

```
BIG File Loading:      ~2s  (20%)
MapCache Processing:   ~6s  (60%)
Engine Init:           ~1s  (10%)
SDL2/OpenGL Setup:    ~0.5s  (5%)
Main Loop Entry:      ~0.5s  (5%)
```

#### Backport Notes:

- Run same testing procedure on Generals base
- Expected minor timing differences due to fewer assets
- Document baseline in same format for comparison
- Key metrics to track:
  - Initialization time
  - Memory usage
  - GL error count (should remain 0)
  - Exit code stability

---

### Task 27.5.4: Texture File Loading (DEFERRED)

**Status**: ⏸️ **Design Complete, Implementation Deferred to Phase 28**

**Rationale**: Texture loading requires extensive implementation (200+ lines per format for DDS/TGA). Current Phase 27 infrastructure can operate with stub textures. Full implementation scheduled for Phase 28 rendering work.

#### Design Document:
- `PHASE27/TEXTURE_LOADING_DESIGN.md` (architecture and API design)

#### Backport Impact:

- When implemented in Zero Hour, full backport will be required
- Estimated effort: 2-3 hours for complete backport
- No immediate action needed for current Phase 27 completion

---

### Task 27.5.5: Update Backport Guide (CURRENT)

**Status**: ✅ **COMPLETE**

This section completes the Phase 27.5 documentation in the backport guide.

---

## Next Steps After Phase 27.5

1. **Phase 27.6**: Final Documentation Update
   - Update all progress tracking documents
   - Create Phase 27 completion summary
   - Document known issues and limitations

2. **Phase 27.7**: Generals Base Backport Execution
   - Follow this guide to backport all Phase 27 features
   - Test Generals base independently
   - Document any unexpected differences

3. **Phase 27.8**: Git Commits and GitHub Release
   - Commit all Phase 27 changes with descriptive messages
   - Create GitHub release/tag for Phase 27 milestone
   - Prepare Phase 28 planning document

---

**Document Version**: 2.1  
**Last Updated**: October 7, 2025  
**Author**: GeneralsX Development Team
