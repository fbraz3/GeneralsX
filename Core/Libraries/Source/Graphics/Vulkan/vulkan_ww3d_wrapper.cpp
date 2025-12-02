/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * Phase 39.3: WW3D Wrapper - Maps WW3D API calls to Vulkan backend
 *
 * This wrapper provides stub implementations of all WW3D functions that would normally
 * be in ww3d.cpp, shader.cpp, texture.cpp, etc. When using the Vulkan backend (USE_VULKAN),
 * these files are excluded from compilation, so we need to provide minimal implementations here.
 */

#include <cstdio>
#include <cstdlib>

 // Graphics driver system - required to initialize Vulkan backend
#include "../GraphicsDriverFactory.h"
#include "../IGraphicsDriver.h"
#include "../dx8buffer_compat.h"  // For SetGraphicsDriver

 // Forward declarations
class RenderObjClass;
class SceneClass;
class CameraClass;
class RenderInfoClass;
struct Vector2;

// Phase 6: VertexMaterial class (moved before WW3D namespace for use in Init/Shutdown)
class VertexMaterialClass {
public:
  // Static initialization methods (called by WW3D::Init/Shutdown)
  static void Init();
  static void Shutdown();

  // Instance methods
  virtual ~VertexMaterialClass() {}

  virtual void Compute_CRC() const {}
  virtual void Get_Ambient(void* out_vector) const {}
  virtual void Get_Diffuse(void* out_vector) const {}
  virtual void Get_Opacity() const {}
  virtual void Get_Emissive(void* out_vector) const {}
};
struct Vector3 {
  float X, Y, Z;
  Vector3() : X(0), Y(0), Z(0) {}
  Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
};

typedef int WW3DErrorType;
#define WW3D_ERROR_OK 0

namespace WW3D {
  // ============================================================================
  // Global State Variables (referenced by ww3d.h inline functions)
  // ============================================================================

  // Time synchronization variables
  float SyncTime = 0.0f;
  unsigned int PreviousSyncTime = 0;
  unsigned int FractionalSyncMs = 0;
  unsigned int LogicFrameTimeMs = 33;  // ~30 FPS default
  unsigned int FrameCount = 0;

  // Feature flags
  bool IsSortingEnabled = true;
  bool AreStaticSortListsEnabled = true;
  bool AreDecalsEnabled = true;
  bool IsTexturingEnabled = true;
  bool ThumbnailEnabled = false;
  bool IsScreenUVBiased = false;

  // Rendering mode variables
  int PrelitMode = 0;  // PrelitModeEnum
  int NPatchesLevel = 0;
  int NPatchesGapFillingMode = 0;
  bool MungeSortOnLoad = false;
  bool OverbrightModifyOnLoad = false;

  // Default screen size (used by render objects)
  Vector2* DefaultNativeScreenSize = nullptr;

  // Window handle (platform-specific)
  static void* g_windowHandle = nullptr;
  static int g_deviceWidth = 800;
  static int g_deviceHeight = 600;
  static int g_deviceBitDepth = 32;
  static bool g_deviceWindowed = true;
  static int g_textureReduction = 0;

  // Graphics driver instance (Phase 41)
  static Graphics::IGraphicsDriver* g_graphicsDriver = nullptr;

  // ============================================================================
  // Core WW3D Functions
  // ============================================================================

  WW3DErrorType Init(void* hwnd, char* defaultpal = nullptr, bool lite = false) {
    printf("[Vulkan WW3D Wrapper] WW3D::Init(%p, %p, %d) - Vulkan backend initialized\n",
      hwnd, defaultpal, lite);
    g_windowHandle = hwnd;

    // Phase 41: Initialize the graphics driver FIRST before any buffer creation
    // CreateDriver() calls SetGraphicsDriver() internally and initializes the backend
    fprintf(stderr, "[Vulkan WW3D Wrapper] Creating graphics driver via factory...\n");
    fflush(stderr);

    // Use platform default backend (Vulkan on Linux)
    g_graphicsDriver = Graphics::GraphicsDriverFactory::CreateDriver(
      Graphics::BackendType::Unknown,  // Auto-select best backend
      hwnd,                            // Window handle
      static_cast<uint32_t>(g_deviceWidth),   // Width
      static_cast<uint32_t>(g_deviceHeight),  // Height
      !g_deviceWindowed                // Fullscreen flag
    );

    if (g_graphicsDriver) {
      fprintf(stderr, "[Vulkan WW3D Wrapper] Graphics driver created and initialized: %p\n",
        static_cast<void*>(g_graphicsDriver));
      fflush(stderr);
    }
    else {
      fprintf(stderr, "[Vulkan WW3D Wrapper] ERROR: Failed to create graphics driver!\n");
      fflush(stderr);
    }

    // Initialize vertex material presets (Phase 6: Critical for terrain rendering)
    fprintf(stderr, "[Vulkan WW3D Wrapper] Calling VertexMaterialClass::Init()\n");
    fflush(stderr);
    VertexMaterialClass::Init();
    fprintf(stderr, "[Vulkan WW3D Wrapper] VertexMaterialClass::Init() completed\n");
    fflush(stderr);

    return WW3D_ERROR_OK;
  }

  void Shutdown() {
    printf("[Vulkan WW3D Wrapper] WW3D::Shutdown() called\n");

    // Shutdown vertex material presets
    fprintf(stderr, "[Vulkan WW3D Wrapper] Calling VertexMaterialClass::Shutdown()\n");
    fflush(stderr);
    VertexMaterialClass::Shutdown();
    fprintf(stderr, "[Vulkan WW3D Wrapper] VertexMaterialClass::Shutdown() completed\n");
    fflush(stderr);

    // Phase 41: Destroy the graphics driver
    if (g_graphicsDriver) {
      fprintf(stderr, "[Vulkan WW3D Wrapper] Destroying graphics driver...\n");
      fflush(stderr);
      Graphics::GraphicsDriverFactory::DestroyDriver(g_graphicsDriver);
      g_graphicsDriver = nullptr;
      fprintf(stderr, "[Vulkan WW3D Wrapper] Graphics driver destroyed\n");
      fflush(stderr);
    }

    g_windowHandle = nullptr;
  }

  WW3DErrorType Sync(bool wait_for_vsync = true) {
    // Vulkan presents automatically
    FrameCount++;
    return WW3D_ERROR_OK;
  }

  // ============================================================================
  // Render Functions (Scene/Camera based)
  // ============================================================================

  void Render(SceneClass* scene, CameraClass* camera, bool clipping, bool sort, const Vector3& fog_color) {
    // Vulkan rendering - stub
  }

  void Render(RenderObjClass& obj, RenderInfoClass& render_info) {
    // Vulkan render single object - stub
  }

  void Flush(RenderInfoClass& render_info) {
    // Vulkan flush - stub
  }

  void Render_And_Clear_Static_Sort_Lists(RenderInfoClass& render_info) {
    // Vulkan render - stub
  }

  void Begin_Render(bool clear, bool clear_zbuffer, const Vector3& clear_color, float clear_z, void (*callback)()) {
    // Begin Vulkan render pass - stub
  }

  void End_Render(bool present) {
    // End Vulkan render pass - stub
  }

  void Set_Collision_Box_Display_Mask(int mask) {
    // Debug helper - no-op
  }

  // ============================================================================
  // Device/Resolution Functions
  // ============================================================================

  void* Get_Window() {
    return g_windowHandle;
  }

  void Get_Device_Resolution(int& width, int& height, int& bitDepth, bool& windowed) {
    width = g_deviceWidth;
    height = g_deviceHeight;
    bitDepth = g_deviceBitDepth;
    windowed = g_deviceWindowed;
  }

  void Get_Render_Target_Resolution(int& width, int& height, int& bitDepth, bool& windowed) {
    width = g_deviceWidth;
    height = g_deviceHeight;
    bitDepth = g_deviceBitDepth;
    windowed = g_deviceWindowed;
  }

  WW3DErrorType Set_Device_Resolution(int width, int height, int bitDepth, int device, bool windowed) {
    g_deviceWidth = width;
    g_deviceHeight = height;
    g_deviceBitDepth = bitDepth;
    g_deviceWindowed = windowed;
    return WW3D_ERROR_OK;
  }

  WW3DErrorType Set_Render_Device(int device, int resx, int resy, int bits, int windowed_mode,
    bool resize_window, bool reset_device, bool restore_assets) {
    g_deviceWidth = resx;
    g_deviceHeight = resy;
    g_deviceBitDepth = bits;
    g_deviceWindowed = (windowed_mode != 0);
    return WW3D_ERROR_OK;
  }

  void* Get_Render_Device_Desc(int device) {
    return nullptr;  // Stub - return null device desc
  }

  // ============================================================================
  // Texture Functions
  // ============================================================================

  int Get_Texture_Bitdepth() {
    return 32;
  }

  void Set_Texture_Bitdepth(int bits) {
    // Stub
  }

  int Get_Texture_Reduction() {
    return g_textureReduction;
  }

  void Set_Texture_Reduction(int reduction) {
    g_textureReduction = reduction;
  }

  void Set_Texture_Reduction(int reduction, int someflag) {
    g_textureReduction = reduction;
  }

  void Enable_Texturing(bool enable) {
    IsTexturingEnabled = enable;
  }

  // ============================================================================
  // Timing Functions
  // ============================================================================

  void Update_Logic_Frame_Time(float time_ms) {
    LogicFrameTimeMs = static_cast<unsigned int>(time_ms);
  }

  // ============================================================================
  // Static Sort List Functions
  // ============================================================================

  void Add_To_Static_Sort_List(RenderObjClass* obj, unsigned int sortLevel) {
    // TODO: Implement Vulkan-based static sort list
  }
}

// NOTE: TextureBaseClass::Set_Texture_Name and TextureBaseClass::Invalidate
// are implemented in phase43_surface_texture.cpp which is part of the WW3D2 library
// and has access to the full game header hierarchy

// Shader class stubs
class ShaderClass {
public:
  virtual ~ShaderClass() {}

  virtual int Get_SS_Category() const { return 0; }
  virtual int Guess_Sort_Level() const { return 0; }
};

// Texture class stubs
class TextureClass {
public:
  virtual ~TextureClass() {}

  virtual int Get_Texture_Memory_Usage() const { return 0; }
};

class TextureBaseClass : public TextureClass {
public:
  virtual void* Peek_D3D_Base_Texture() const { return nullptr; }
};

// Bezier segment stubs
class BezierSegment {
public:
  virtual ~BezierSegment() {}

  virtual void getSegmentPoints(int segments, void* points_vector) const {}
  virtual float getApproximateLength(float tolerance) const { return 0.0f; }
};

// Simple function symbols that might be needed
extern "C" {
  void _g_LastErrorDump() {
    printf("[Vulkan WW3D Wrapper] Crash dump requested\n");
  }
}

// WW3D static member definitions (linker support)
namespace WW3D {
  float DecalRejectionDistance = 1000000.0f;
  bool SnapshotActivated = false;
  bool IsColoringEnabled = false;
}

