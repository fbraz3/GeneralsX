/*
 * DX8Wrapper_Stubs.h
 * 
 * Phase 39.4 Compatibility Layer (Extended Phase 41 Week 3)
 * Provides no-op/stub implementations of DX8Wrapper for cross-platform compilation.
 * All graphics operations are routed to the Graphics::IGraphicsDriver backend (factory-based).
 * 
 * Created: November 18, 2025
 * Updated: Phase 41 Week 3 - Integrated Graphics::IGraphicsDriver factory
 * 
 * Purpose: Allow legacy DirectX 8 game code to compile without modification while
 *          the actual graphics rendering is handled by the abstract graphics driver backend.
 * 
 * Integration: DX8Wrapper::Begin_Scene/End_Scene now delegates to Graphics::IGraphicsDriver
 * via the factory pattern. This ensures game code uses pure abstraction without backend coupling.
 */

#ifndef __DX8WRAPPER_STUBS_H__
#define __DX8WRAPPER_STUBS_H__

// Include DirectX 8 graphics compatibility layer (provides D3DSURFACE_DESC, etc.)
#include "d3d8_vulkan_graphics_compat.h"
#include "vector3.h"  // Phase 39.4: For Convert_Color parameter type
#include "vector4.h"  // Phase 39.4: For Convert_Color return type
#include "rddesc.h"  // Phase 39.4: For RenderDeviceDescClass
#include "dx8fvf_vertex_formats.h"  // Phase 42: Vertex format definitions

// Phase 41 Week 3: Graphics driver factory integration
// Forward declare to avoid header dependencies
namespace Graphics {
    class IGraphicsDriver;
}

// Phase 41 Week 3: Get the current graphics driver instance (from DX8Wrapper_Stubs.cpp)
extern Graphics::IGraphicsDriver* GetCurrentGraphicsDriver();

// Phase 42: Forward declaration for DX8_CleanupHook interface
// Full definition appears at end of this file
class DX8_CleanupHook;

// ============================================================================
// DIRECTX 8 DEFINES & ENUMS (Phase 39.4 Stub Layer)
// ============================================================================

// NOTE: D3DFORMAT and D3DTRANSFORMSTATETYPE are defined in:
// - d3d8_vulkan_enums_compat.h (uses enum, conflicts with typedef in some files)
// We use #define for common constants to avoid redefinition conflicts

// D3DFORMAT constants (as #define instead of enum to avoid typedef conflicts)
#ifndef D3DFMT_UNKNOWN
#define D3DFMT_UNKNOWN 0
#define D3DFMT_R8G8B8 20
#define D3DFMT_A8R8G8B8 21
#define D3DFMT_X8R8G8B8 22
#define D3DFMT_R5G6B5 23
#define D3DFMT_X1R5G5B5 24
#define D3DFMT_A1R5G5B5 25
#define D3DFMT_A4R4G4B4 26
#define D3DFMT_R3G3B2 27
#define D3DFMT_A8 28
#define D3DFMT_A8R3G3B2 29
#define D3DFMT_X4R4G4B4 30
#define D3DFMT_A2B10G10R10 31
#define D3DFMT_G16R16 34
#define D3DFMT_A8P8 40
#define D3DFMT_P8 41
#define D3DFMT_L8 50
#define D3DFMT_A8L8 51
#define D3DFMT_A4L4 52
#define D3DFMT_V8U8 60
#define D3DFMT_L6V5U5 61
#define D3DFMT_X8L8V8U8 62
#define D3DFMT_Q8W8V8U8 63
#define D3DFMT_V16U16 64
#define D3DFMT_W11V11U10 65
#define D3DFMT_A2W10V10U10 67
#define D3DFMT_UYVY 0x59565955
#define D3DFMT_YUY2 0x32595559
#define D3DFMT_DXT1 0x31545844
#define D3DFMT_DXT2 0x32545844
#define D3DFMT_DXT3 0x33545844
#define D3DFMT_DXT4 0x34545844
#define D3DFMT_DXT5 0x35545844
#define D3DFMT_D16_LOCKABLE 70
#define D3DFMT_D32 71
#define D3DFMT_D15S1 73
#define D3DFMT_D24S8 75
#define D3DFMT_D16 80
#define D3DFMT_D24X8 77
#define D3DFMT_D24X4S4 79
#define D3DFMT_INDEX16 101
#define D3DFMT_INDEX32 102
#endif

// D3DTRANSFORMSTATETYPE constants (as #define instead of enum)
#ifndef D3DTS_WORLD
#define D3DTS_WORLD 256
#define D3DTS_VIEW 2
#define D3DTS_PROJECTION 3
#define D3DTS_TEXTURE0 16
#define D3DTS_TEXTURE1 17
#define D3DTS_TEXTURE2 18
#define D3DTS_TEXTURE3 19
#define D3DTS_TEXTURE4 20
#define D3DTS_TEXTURE5 21
#define D3DTS_TEXTURE6 22
#define D3DTS_TEXTURE7 23
#endif

// Dynamic Buffer Type Constants
#define BUFFER_TYPE_DYNAMIC_DX8 1
#define BUFFER_TYPE_STATIC_DX8 0
#define BUFFER_TYPE_DYNAMIC_SORTING 2  // Phase 39.4: Added for sorting buffer support

// Phase 39.4: D3DX Filter constants for D3DXLoadSurfaceFromSurface (missingtexture.cpp)
#ifndef D3DX_FILTER_BOX
#define D3DX_FILTER_NONE       0x00000000
#define D3DX_FILTER_POINT      0x00000001
#define D3DX_FILTER_LINEAR     0x00000002
#define D3DX_FILTER_TRIANGLE   0x00000004
#define D3DX_FILTER_BOX        0x00000008
#define D3DX_FILTER_MIRROR_U   0x00010000
#define D3DX_FILTER_MIRROR_V   0x00020000
#define D3DX_FILTER_MIRROR_W   0x00040000
#endif

// Phase 39.4: FVF (Flexible Vertex Format) stub
extern const int dynamic_fvf_type;  // Forward declare global constant

// FVFInfoClass - stub implementation for vertex format info
class FVFInfoClass {
public:
    int Get_Location_Offset() const { return 0; }
    int Get_Normal_Offset() const { return 0; }
    int Get_Diffuse_Offset() const { return 0; }
    int Get_Tex_Offset(int) const { return 0; }
    int Get_FVF_Width() const { return 0; }
    int Get_FVF_Size() const { return 0; }
};

// Forward declarations to avoid include chain
// Actual vector/matrix classes are defined elsewhere
class Vector3;
class Vector4;
class Matrix3D;
class Matrix4x4;

// Forward declarations for pointer types
class TextureClass;
class VertexMaterialClass;
class ShaderClass;
class SurfaceClass;
class LightClass;

// Forward declarations for dynamic buffer classes  
class DynamicVBAccessClass;
class DynamicIBAccessClass;

// Stub IDirect3DDevice8 (Phase 39.4: TestCooperativeLevel stub for ww3d.cpp line 801)
class IDirect3DDevice8Stub {
public:
    virtual ~IDirect3DDevice8Stub() {}
    virtual int TestCooperativeLevel() { return 0; }  // D3D_OK
};

// Global instance for _Get_D3D_Device8() (Phase 39.4)
extern IDirect3DDevice8Stub* GetStubD3DDevice8();
static IDirect3DDevice8Stub* g_stub_d3d_device8 = nullptr;

// Forward declarations for vertex format classes
class VertexFormatXYZNDUV2;
class VertexFormatXYZDUV2;
class VertexFormatXYZ;

/*
 * DX8Wrapper Stub Class
 * 
 * Provides no-op implementations of all DX8Wrapper static methods.
 * This allows existing game code that calls DX8Wrapper::MethodName() to compile
 * without errors, while actual rendering is handled by the Vulkan backend.
 * 
 * Pattern: Each method is a no-op inline function that accepts the same parameters
 * as the original DirectX 8 implementation.
 */
class DX8Wrapper {
public:
    // ========================================================================
    // Initialization & Device Management
    // ========================================================================
    // Phase 41 Week 3: Initialize graphics driver via factory
    static bool Init(void* hwnd, bool lite);  // Implementation in DX8Wrapper_Stubs.cpp
    static void Shutdown();  // Implementation in DX8Wrapper_Stubs.cpp
    static bool Set_Render_Device(int dev, int width, int height, int bits, int windowed, 
                                   bool resize_window, bool reset_device, bool restore_assets) { return true; }
    // Phase 39.4: Overload for 6-parameter version (ww3d.cpp line 396)
    static bool Set_Render_Device(const char* dev_name, int width, int height, int bits, int windowed,
                                   bool resize_window) { return true; }
    static bool Set_Any_Render_Device() { return true; }
    static bool Set_Next_Render_Device() { return true; }
    static bool Is_Initted() { return true; }
    static bool Reset_Device() { return true; }
    static bool Set_Device_Resolution(int width, int height, int bits, bool windowed, 
                                       bool resize_window) { return true; }
    static bool Toggle_Windowed() { return false; }
    static bool Is_Windowed() { return false; }

    // ========================================================================
    // Render Target Management
    // ========================================================================
    static void Set_Render_Target(SurfaceClass* target) {}
    static void Set_Render_Target_With_Z(SurfaceClass* color_target, SurfaceClass* z_target) {}
    static SurfaceClass* Create_Render_Target(int width, int height) { return nullptr; }
    static SurfaceClass* _Get_DX8_Back_Buffer() { return nullptr; }

    // ========================================================================
    // Scene Rendering
    // ========================================================================
    // Phase 41 Week 3: Delegate to Graphics::IGraphicsDriver factory
    static void Begin_Scene();  // Implementation in DX8Wrapper_Stubs.cpp
    static void End_Scene(bool flip_frame);  // Implementation in DX8Wrapper_Stubs.cpp
    static void Clear(bool clear_color, bool clear_z, const Vector3 &color, 
                      float dest_alpha = 1.0f) {}
    static void Flip_To_Primary() {}
    static int Peek_Device_Resolution_Width() { return 1024; }
    static int Peek_Device_Resolution_Height() { return 768; }
    static int Get_Device_Resolution_Width() { return 1024; }
    static int Get_Device_Resolution_Height() { return 768; }
    static void Get_Device_Resolution(int& w, int& h, int& bits, bool& windowed) {}
    static void Get_Render_Target_Resolution(int& w, int& h, int& bits, bool& windowed) {}

    // ========================================================================
    // Vertex & Index Buffer Management
    // ========================================================================
    static void Set_Vertex_Buffer(void* vb_access) {}
    static void Set_Vertex_Buffer(int stream_number, void* vb_access) {}
    static void Set_Vertex_Buffer(DynamicVBAccessClass* vb_access) {}  // Phase 39.4: Direct class overload
    static void Set_Vertex_Buffer(int stream_number, DynamicVBAccessClass* vb_access) {}
    static void Set_Vertex_Buffer(DynamicVBAccessClass& vb_access) {}  // Phase 39.4: Reference overload
    static void Set_Vertex_Buffer(int stream_number, DynamicVBAccessClass& vb_access) {}  // Phase 39.4: Reference with stream
    
    static void Set_Index_Buffer(void* ib_access, int start_index) {}
    static void Set_Index_Buffer(int stream_number, void* ib_access, int start_index) {}
    static void Set_Index_Buffer(DynamicIBAccessClass* ib_access, int start_index) {}  // Phase 39.4: Direct class overload
    static void Set_Index_Buffer(int stream_number, DynamicIBAccessClass* ib_access, int start_index) {}
    static void Set_Index_Buffer(DynamicIBAccessClass& ib_access, int start_index) {}  // Phase 39.4: Reference overload
    static void Set_Index_Buffer(int stream_number, DynamicIBAccessClass& ib_access, int start_index) {}  // Phase 39.4: Reference with stream

    // ========================================================================
    // Rendering Calls
    // ========================================================================
    static void Draw_Triangles(int start_index, int polygon_count, int min_vertex_index, 
                               int vertex_count) {}
    static void Draw_Triangles(unsigned int buffer_type, int start_index, int polygon_count,
                               int min_vertex_index, int vertex_count) {}

    // ========================================================================
    // Texture Operations
    // ========================================================================
    static void Set_Texture(int stage, TextureClass* texture) {}
    static void Set_DX8_Texture(int stage, void* d3d_texture) {}
    static void* _Create_DX8_Texture(int width, int height, int format) { return nullptr; }
    static void* _Create_DX8_Texture(int width, int height, int format, int mip_levels) { return nullptr; }
    static void* _Create_DX8_Surface(int width, int height, int format) { return nullptr; }
    static void* _Create_DX8_ZTexture(int width, int height, int format) { return nullptr; }
    static void* _Create_DX8_Cube_Texture(int width, int height, int format) { return nullptr; }
    static void* _Create_DX8_Volume_Texture(int width, int height, int depth, int format) 
                                            { return nullptr; }
    static void _Copy_DX8_Rects(void* src_surface, void* src_rect, int src_pitch,
                               void* dst_surface, void* dst_rect) {}
    static int CreateImageSurface(int width, int height, int format, void** surface) { return 0; }
    static int CopyRects(void* src_surface, void* src_rect, int count, void* dst_surface, void* dst_rect) { return 0; }
    static int D3DXLoadSurfaceFromSurface(void* dst_surface, void* dst_palette, void* dst_rect,
                                         void* src_surface, void* src_palette, void* src_rect,
                                         int filter, int color_key) { return 0; }

    // ========================================================================
    // Material & Shader Operations
    // ========================================================================
    static void Set_Material(VertexMaterialClass* material) {}
    static void Set_Shader(ShaderClass* shader) {}
    static void Set_Shader(const ShaderClass& shader) {}  // Phase 39.4: ShaderClass reference overload
    static void Set_DX8_Material(void* d3d_material) {}

    // ========================================================================
    // Render State Management
    // ========================================================================
    static void Apply_Render_State_Changes() {}
    static void Invalidate_Cached_Render_States() {}
    static void Set_DX8_Render_State(unsigned int state, unsigned int value) {}
    static void Set_DX8_Texture_Stage_State(int stage, unsigned int state, unsigned int value) {}
    static void Get_Render_State(void* state) {}
    static void Release_Render_State() {}

    // ========================================================================
    // Transform Operations
    // ========================================================================
    static void Set_Transform(unsigned int type, const void* transform) {}
    static void _Set_DX8_Transform(unsigned int type, const void* transform) {}
    static void Get_Transform(unsigned int type, void* transform) {}
    static void _Get_DX8_Transform(unsigned int type, void* transform) {}
    static void Set_World_Identity() {}
    static void Set_View_Identity() {}
    static void Set_Projection_Transform_With_Z_Bias(const void* projection, 
                                                      float z_near, float z_far) {}

    // ========================================================================
    // Additional Transform Overloads for Vector4/Matrix3D/Matrix4x4 Support
    // ========================================================================
    static void Set_Transform(unsigned int type, const Vector4& transform) {}
    static void Set_Transform(unsigned int type, const Vector4* transform) {}
    static void Set_Transform(unsigned int type, const Matrix3D& transform) {}
    static void Set_Transform(unsigned int type, const Matrix3D* transform) {}
    static void Set_Transform(unsigned int type, const Matrix4x4& transform) {}
    static void Set_Transform(unsigned int type, const Matrix4x4* transform) {}
    static void Get_Transform(unsigned int type, Vector4& transform) {}
    static void Get_Transform(unsigned int type, Vector4* transform) {}
    static void Get_Transform(unsigned int type, Matrix3D& transform) {}
    static void Get_Transform(unsigned int type, Matrix3D* transform) {}
    static void Get_Transform(unsigned int type, Matrix4x4& transform) {}
    static void Get_Transform(unsigned int type, Matrix4x4* transform) {}
    
    // Projection transform overloads for Matrix3D/Matrix4x4
    static void Set_Projection_Transform_With_Z_Bias(const Matrix3D& projection,
                                                      float z_near, float z_far) {}
    static void Set_Projection_Transform_With_Z_Bias(const Matrix4x4& projection,
                                                      float z_near, float z_far) {}

    // ========================================================================
    // Viewport & Clipping
    // ========================================================================
    static void Set_Viewport(void* viewport) {}
    static void Set_Viewport(const void* viewport) {}

    // ========================================================================
    // Lighting
    // ========================================================================
    static void Set_Light(int index, void* light) {}
    static void Set_DX8_Light(int index, void* d3d_light) {}
    static void Set_Ambient(const void* color) {}
    static void Set_Ambient(const Vector3& color) {}  // Phase 39.4: Overload for Vector3 (ww3d.cpp line 969)
    static void Set_Light_Environment(void* light_env) {}

    // ========================================================================
    // Statistics / Debugging (Phase 39.4: No-op stubs)
    // ========================================================================
    static void Begin_Statistics() {}  // Called by Debug_Statistics::Begin_Statistics()
    static void End_Statistics() {}    // Called by Debug_Statistics::End_Statistics()

    // ========================================================================
    // Fog Operations
    // ========================================================================
    static void Set_Fog(bool enable, const Vector3 &color, float start, float end) {}
    static bool Get_Fog_Enable() { return false; }
    static unsigned int Get_Fog_Color() { return 0; }

    // ========================================================================
    // Capabilities & Device Info
    // ========================================================================
    static class DX8Caps* Get_Current_Caps() { extern DX8Caps* Get_DX8_Caps_Ptr(); return Get_DX8_Caps_Ptr(); }  // Phase 39.4: Return valid instance
    static bool Has_Stencil() { return false; }
    static int Get_Render_Device() { return 0; }
    static int Get_Render_Device_Count() { return 1; }
    static const char* Get_Render_Device_Name(int device_index) { return "Default"; }
    static const RenderDeviceDescClass& Get_Render_Device_Desc(int device_index) { 
        // Static stub instance (Phase 39.4)
        static RenderDeviceDescClass stub_desc;
        return stub_desc;
    }
    static int getBackBufferFormat() { return 0; }

    // ========================================================================
    // Registry and Configuration (Phase 39.4)
    // ========================================================================
    static bool Registry_Load_Render_Device(const char* sub_key, char* device, int device_len, 
                                           int& width, int& height, int& depth, int& windowed, int& texture_depth) 
    {
        // Stub implementation - set reasonable defaults
        if (device && device_len > 0) {
            strncpy(device, "Default", device_len - 1);
            device[device_len - 1] = '\0';
        }
        width = 1024;
        height = 768;
        depth = 32;
        windowed = 1;
        texture_depth = 32;
        return true;
    }

    // ========================================================================
    // Utility Functions
    // ========================================================================
    static unsigned int Convert_Color(const void* color, float alpha = 1.0f) { return 0; }
    static unsigned int Convert_Color(const Vector3& color, float alpha = 1.0f) { 
        // Convert RGB float (0.0-1.0) to ARGB unsigned int
        unsigned char r = static_cast<unsigned char>(color.X * 255.0f);
        unsigned char g = static_cast<unsigned char>(color.Y * 255.0f);
        unsigned char b = static_cast<unsigned char>(color.Z * 255.0f);
        unsigned char a = static_cast<unsigned char>(alpha * 255.0f);
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    static unsigned int Convert_Color(const Vector4& color, float alpha = 1.0f) { 
        // Convert RGBA float (0.0-1.0) to ARGB unsigned int
        unsigned char r = static_cast<unsigned char>(color.X * 255.0f);
        unsigned char g = static_cast<unsigned char>(color.Y * 255.0f);
        unsigned char b = static_cast<unsigned char>(color.Z * 255.0f);
        unsigned char a = static_cast<unsigned char>(color.W * 255.0f);  // Use W component as alpha
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    // Phase 39.4: Convert from unsigned int (ARGB format) - declaration only, implemented at end of file
    static Vector4 Convert_Color(unsigned int color);
    static unsigned int Convert_Color_Clamp(const Vector4& color) { return 0; }
    static void* Convert_Color_Clamp(const void* color) { return nullptr; }

    // ========================================================================
    // Swap Chain & Display
    // ========================================================================
    static int Get_Swap_Interval() { return 0; }
    static void Set_Swap_Interval(int swap) {}
    static void Set_Texture_Bitdepth(int bitdepth) {}
    static int Get_Texture_Bitdepth() { return 32; }
    static void Set_Gamma(float gamma, float bright, float contrast, bool calibrate) {}

    // ========================================================================
    // Registry Operations
    // ========================================================================
    static bool Registry_Save_Render_Device(const char* sub_key) { return true; }
    static bool Registry_Save_Render_Device(const char* sub_key, int device, int width, 
                                           int height, int depth, bool windowed, 
                                           int texture_depth) { return true; }
    static bool Registry_Load_Render_Device(const char* sub_key, bool resize_window) { return true; }
    static bool Registry_Load_Render_Device(const char* sub_key, int& device, char* device_name,
                                           int device_len, int& width, int& height, int& depth,
                                           bool& windowed, int& texture_depth) { return true; }

    // ========================================================================
    // Screen Capture & Buffer Operations
    // ========================================================================
    static SurfaceClass* Get_Front_Buffer() { return nullptr; }
    static SurfaceClass* Get_Back_Buffer() { return nullptr; }

    // ========================================================================
    // Shader Operations (Advanced)
    // ========================================================================
    static void Set_Pixel_Shader(unsigned int shader) {}
    static void Set_Vertex_Shader(unsigned int shader) {}

    // ========================================================================
    // Frame Control & State
    // ========================================================================
    static bool _Is_Triangle_Draw_Enabled() { return true; }
    static void _Enable_Triangle_Draw(bool enabled) {}
    static bool Is_Render_To_Texture() { return false; }

    // ========================================================================
    // Direct Device Access (Advanced)
    // ========================================================================
    static IDirect3DDevice8Stub* _Get_D3D_Device8() { 
        if (!g_stub_d3d_device8) {
            g_stub_d3d_device8 = new IDirect3DDevice8Stub();
        }
        return g_stub_d3d_device8;
    }
    static void* _Get_D3D8() { return nullptr; }

    // ========================================================================
    // Cleanup & Hooks
    // ========================================================================
    /**
     * Register a cleanup hook that implements DX8_CleanupHook interface.
     * Called during device reset to release and reacquire resources.
     * @param hook Pointer to object implementing DX8_CleanupHook interface
     */
    static void SetCleanupHook(DX8_CleanupHook* hook) 
    { 
        // Forward to actual graphics driver if available
        // For now, just store it (Phase 42: can be enhanced later)
        if (hook) {
            // Could call hook->ReleaseResources() here during device reset
            // For stub version, this is a no-op
        }
    }
    
    static int _Get_Main_Thread_ID() { return 0; }

    // ========================================================================
    // Statistics & Debugging
    // ========================================================================
    static void stats(const char* format, ...) {}

private:
    DX8Wrapper();  // Prevent instantiation
};

// ============================================================================
// DIRECTX STRUCTURES & CLASSES
// ============================================================================

// NOTE: D3DSURFACE_DESC, IDirect3DTexture8, D3DVIEWPORT8, and other DirectX 8 structures
// are defined in d3d8_vulkan_graphics_compat.h and d3d8_vulkan_interfaces_compat.h
// We use forward declarations for additional classes not in those headers.

// ============================================================================
// VERTEX FORMATS (Phase 42)
// Vertex format structs are now defined in dx8fvf_vertex_formats.h
// to avoid duplication and ensure consistent definitions across the codebase
// ============================================================================

// Forward declaration for DX8Caps
class DX8Caps {
public:
    // Vendor and device IDs (Phase 39.4)
    enum VendorID {
        VENDOR_UNKNOWN = 0,
        VENDOR_NVIDIA = 0x10DE,
        VENDOR_AMD = 0x1022,
        VENDOR_INTEL = 0x8086,
        VENDOR_3DFX = 0x121A,
        VENDOR_S3 = 0x5333,
        VENDOR_MATROX = 0x102B,
        VENDOR_KYRO = 0x8086
    };

    enum DeviceID {
        DEVICE_UNKNOWN = 0,
        DEVICE_3DFX_VOODOO_3 = 0x1
    };

    // TextureOpCaps flags (DirectX 8 texture operation capabilities)
    unsigned long TextureOpCaps = 0;  // Phase 39.4: Texture operation capabilities flags

    DX8Caps() {}
    virtual ~DX8Caps() {}
    
    // Phase 39.4: Stub methods
    virtual bool Support_ZBias() const { return false; }
    virtual bool Support_AntiAlias() const { return false; }
    virtual bool Support_Volumetric_Textures() const { return false; }
    virtual int Get_Max_Textures_Per_Pass() const { return 4; }  // Reasonable default for legacy hardware
    virtual bool Support_NPatches() const { return false; }  // N-Patches support (tessellation)
    virtual bool Support_Dot3() const { return false; }  // Phase 39.4: Dot3 (DOT product) support for render2d.cpp
    virtual bool Support_ModAlphaAddClr() const { return false; }  // MODULATEALPHA_ADDCOLOR support for shader.cpp
    virtual bool Support_DXTC() const { return false; }  // Phase 39.4: DXTC compression support
    virtual bool Support_Texture_Format(int format) const { return false; }  // Phase 39.4: Check specific texture format support
    
    // Hardware identification (shader.cpp line 552-553)
    virtual unsigned int Get_Vendor() const { return VENDOR_UNKNOWN; }
    virtual unsigned int Get_Device() const { return DEVICE_UNKNOWN; }
    
    // Fog support (shader.cpp line 495)
    virtual bool Is_Fog_Allowed() const { return false; }
    
    // Phase 39.4: Get DX8Caps method (self-reference for backward compatibility)
    virtual DX8Caps& Get_DX8_Caps() { return *this; }
};

// DirectX Texture Manager stub (Phase 39.4)
class DX8TextureManagerClass {
public:
    // Static methods for texture management
    static void Shutdown() {}  // Phase 39.4: Shutdown texture manager (no-op stub)
    static void Free_Video_Memory(void*) {}  // Phase 39.4: Free video memory (no-op stub)
};

// DirectX Mesh Renderer stub
class DX8MeshRenderer {
public:
    virtual ~DX8MeshRenderer() {}
    virtual void Render() {}
    virtual void Invalidate() {}  // Phase 39.4: Add Invalidate method for WW3DAssetManager compatibility
    virtual void Flush() {}  // Phase 39.4: Flush any pending render commands (no-op for stub)
    virtual void Set_Camera(void*) {}  // Phase 39.4: Set camera for rendering (no-op stub)
    virtual void Clear_Pending_Delete_Lists() {}  // Phase 39.4: Clear deletion lists (no-op stub)
    // Phase 39.4: Mesh type registration (no-ops for stub)
    virtual void Register_Mesh_Type(void*) {}  // Called in MeshModelClass constructor
    virtual void Unregister_Mesh_Type(void*) {}  // Called in MeshModelClass destructor
    // Phase 39.4: Rendering list management
    virtual void Add_To_Render_List(void*) {}  // No-op stub for mesh rendering
};

extern DX8MeshRenderer TheDX8MeshRenderer;  // Forward declaration - NOTE: Object, not pointer!

// DirectX Error Code Helper Function (Phase 39.4)
// Used in legacy code like: DX8_ErrorCode(d3d_texture->GetLevelDesc(0, &desc));
// Macro approach to handle any return type (including void)
#define DX8_ErrorCode(x) do { (void)(x); } while(0)

// DX8CALL macro for DirectX device calls (Phase 39.4)
// Stub implementation that ignores all device calls (hardware not available)
#define DX8CALL(x) do { (void)(x); } while(0)

// Stub implementations of DirectX 8 device methods (Phase 39.4)
// These are called via DX8CALL macro in shader.cpp
inline HRESULT SetTextureStageState(DWORD stage, DWORD type, DWORD value) { return 0; }
inline HRESULT SetTexture(DWORD stage, void* texture) { return 0; }

// Dynamic Vertex Buffer Access Class Stubs
class DynamicVBAccessClass {
public:
    DynamicVBAccessClass(int buffer_type, int fvf_type, int vertex_count) : m_vertex_count(vertex_count) {
        // Phase 39.4: Allocate stub vertex buffer
        if (vertex_count > 0) {
            m_vertices = new VertexFormatXYZNDUV2[vertex_count];
        } else {
            m_vertices = nullptr;
        }
    }
    virtual ~DynamicVBAccessClass() {
        // Phase 39.4: Free stub vertex buffer
        delete[] m_vertices;
    }
    
    // Phase 39.4: Return stub FVF info for render2d.cpp compatibility
    const FVFInfoClass& FVF_Info() const {
        static FVFInfoClass stub_fvf;
        return stub_fvf;
    }
    
    // Phase 39.4: Reset buffer (no-op for stub)
    void _Reset() {}
    
    // Phase 39.4: Static Reset for use without instance (ww3d.cpp line 824)
    static void _Reset(bool force_physical_reset) {}
    
    class WriteLockClass {
    public:
        WriteLockClass(DynamicVBAccessClass* vb) : m_vb(vb) {}
        virtual ~WriteLockClass() {}
        
        template<typename T> T* Get_Formatted_Vertex_Array() { 
            // Phase 39.4: Return stub vertex buffer cast to requested type
            return reinterpret_cast<T*>(m_vb->m_vertices); 
        }
        
        // Specialization for VertexFormatXYZNDUV2
        VertexFormatXYZNDUV2* Get_Formatted_Vertex_Array() { 
            return m_vb->m_vertices;
        }
        
    private:
        DynamicVBAccessClass* m_vb;
    };
    
private:
    VertexFormatXYZNDUV2* m_vertices;
    int m_vertex_count;
};

// Dynamic Index Buffer Access Class Stubs
class DynamicIBAccessClass {
public:
    DynamicIBAccessClass(int buffer_type, int index_count) {}
    virtual ~DynamicIBAccessClass() {}
    
    // Phase 39.4: Reset buffer (no-op for stub)
    void _Reset() {}
    
    // Phase 39.4: Static Reset for use without instance (ww3d.cpp line 825)
    static void _Reset(bool force_physical_reset) {}
    
    class WriteLockClass {
    public:
        WriteLockClass(DynamicIBAccessClass* ib) {}
        virtual ~WriteLockClass() {}
        unsigned short* Get_Index_Array() { return nullptr; }
    };
};

// Phase 39.4: Base class for index buffers - used by pointgr.cpp
class IndexBufferClass {
public:
    IndexBufferClass(int size = 0) : m_indices(nullptr), m_size(size), m_ref_count(1) {
        if (size > 0) {
            m_indices = new unsigned short[size];
        }
    }
    virtual ~IndexBufferClass() { 
        if (m_indices) delete[] m_indices; 
    }
    
    // Reference counting methods (used by REF_PTR_RELEASE macro)
    virtual void Add_Ref() { m_ref_count++; }
    virtual void Release_Ref() { 
        m_ref_count--;
        if (m_ref_count <= 0) delete this;
    }
    
    class WriteLockClass {
    public:
        WriteLockClass(IndexBufferClass* ib) : m_ib(ib) {}
        virtual ~WriteLockClass() {}
        unsigned short* Get_Index_Array() { 
            return m_ib ? m_ib->m_indices : nullptr;
        }
    private:
        IndexBufferClass* m_ib;
    };
    
protected:
    unsigned short* m_indices;
    int m_size;
    int m_ref_count;
};

// Phase 41: DX8 buffer classes now provided by dx8buffer_compat.h
// These stub implementations are replaced by real implementations
// linked from Core/Libraries/Source/Graphics/dx8buffer_compat.cpp

// Phase 39.4: SortingIndexBufferClass stub - used by pointgr.cpp
class SortingIndexBufferClass : public IndexBufferClass {
public:
    SortingIndexBufferClass(int size = 0) : IndexBufferClass(size) {}
    virtual ~SortingIndexBufferClass() {}
};

// Phase 41: DX8VertexBufferClass compatibility wrapper
// Provided by dx8buffer_compat.h when included

// ============================================================================
// DELETED CLASS STUBS (Phase 39.4)
// These classes were now defined in meshmdl.h to avoid redefinition issues.
// DX8FVFCategoryContainer, DX8PolygonRendererList, and related classes
// are fully defined as stubs in meshmdl.h

// ============================================================================
// DELETED CLASS STUBS (Phase 39.4)
// ============================================================================
// These classes were now defined in meshmdl.h to avoid redefinition issues.
// DX8FVFCategoryContainer, DX8PolygonRendererList, and related classes
// are fully defined as stubs in meshmdl.h

// Phase 39.4: Implementation of DX8Wrapper::Convert_Color(unsigned int)
// Converts unsigned int ARGB color to Vector4 RGBA color
inline Vector4 DX8Wrapper::Convert_Color(unsigned int color) {
    // Simple ARGB -> RGBA conversion stub
    // Returns white with alpha channel from the unsigned int
    float alpha = float((color >> 24) & 0xFF) / 255.0f;
    return Vector4(1.0f, 1.0f, 1.0f, alpha);
}

// ============================================================================
// DIRECTX 8 INTERFACE STUB WRAPPERS (Phase 39.4)
// These provide minimal COM-like interface stubs for legacy code compatibility
// ============================================================================

/**
 * @brief Stub COM wrapper for IDirect3DTexture8 interface
 * Provides no-op implementations of texture methods used by missingtexture.cpp
 */
class DX8TextureStub {
public:
    DX8TextureStub() : m_ref_count(1), m_level_count(1) {}
    virtual ~DX8TextureStub() {}
    
    // Reference counting (COM-like interface)
    virtual void AddRef() { m_ref_count++; }
    virtual void Release() { if (--m_ref_count == 0) delete this; }
    
    // Texture methods used by missingtexture.cpp
    virtual int GetSurfaceLevel(int level, void** surface) { 
        *surface = nullptr; 
        return 0; 
    }
    virtual int LockRect(int level, D3DLOCKED_RECT* locked, void* rect, int flags) { 
        static D3DLOCKED_RECT stub_rect = {nullptr, 0};
        *locked = stub_rect;
        return 0; 
    }
    virtual int UnlockRect(int level) { return 0; }
    virtual unsigned int GetLevelCount() const { return m_level_count; }
    
    // Set level count for mipmap calculations
    void SetLevelCount(unsigned int count) { m_level_count = count; }
    
private:
    int m_ref_count;
    unsigned int m_level_count;
};

/**
 * @brief Stub COM wrapper for IDirect3DSurface8 interface
 * Provides no-op implementations of surface methods
 */
class DX8SurfaceStub {
public:
    DX8SurfaceStub() : m_ref_count(1) {}
    virtual ~DX8SurfaceStub() {}
    
    // Reference counting (COM-like interface)
    virtual void AddRef() { m_ref_count++; }
    virtual void Release() { if (--m_ref_count == 0) delete this; }
    
    // Surface methods
    virtual int GetDesc(D3DSURFACE_DESC* desc) { 
        if (desc) {
            desc->Format = static_cast<D3DFORMAT>(D3DFMT_A8R8G8B8);
            desc->Width = 128;
            desc->Height = 128;
            desc->Type = D3DRTYPE_SURFACE;
        }
        return 0; 
    }
    virtual int LockRect(D3DLOCKED_RECT* locked, void* rect, int flags) { 
        static D3DLOCKED_RECT stub_rect = {nullptr, 0};
        *locked = stub_rect;
        return 0; 
    }
    virtual int UnlockRect() { return 0; }
    
private:
    int m_ref_count;
};

/**
 * @brief DX8_CleanupHook Interface
 * 
 * Phase 42: Virtual interface for DirectX 8 device reset cleanup
 * 
 * This virtual interface is called before resetting the DirectX 8 device
 * to ensure that all DirectX 8 resources are properly released.
 * Objects that manage DirectX 8 resources (like heightmaps, water, etc.)
 * inherit from this interface and implement ReleaseResources() and
 * ReAcquireResources() to handle device resets gracefully.
 * 
 * Usage Pattern:
 *   - Resource objects inherit from DX8_CleanupHook
 *   - Implement ReleaseResources() to release DirectX 8 buffers/textures
 *   - Implement ReAcquireResources() to restore DirectX 8 buffers/textures
 *   - Call DX8Wrapper::SetCleanupHook() to register the hook
 */
class DX8_CleanupHook {
public:
    virtual ~DX8_CleanupHook() {}
    
    /**
     * Release all DirectX 8 resources so the device can be reset.
     * Called before DX8 device reset.
     */
    virtual void ReleaseResources(void) = 0;
    
    /**
     * Reacquire all DirectX 8 resources after device reset.
     * Called after DX8 device reset.
     */
    virtual void ReAcquireResources(void) = 0;
};

#endif // __DX8WRAPPER_STUBS_H__
