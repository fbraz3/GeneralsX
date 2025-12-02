// /*
// **  Command & Conquer Generals Zero Hour(tm)
// **  Copyright 2025 Electronic Arts Inc.
// **
// **  This program is free software: you can redistribute it and/or modify
// **  it under the terms of the GNU General Public License as published by
// **  the Free Software Foundation, either version 3 of the License, or
// **  (at your option) any later version.
// **
// **  This program is distributed in the hope that it will be useful,
// **  but WITHOUT ANY WARRANTY; without even the implied warranty of
// **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// **  GNU General Public License for more details.
// **
// **  You should have received a copy of the GNU General Public License
// **  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// */

// /**
//  * Phase 39.3 / Phase 54: WW3D Wrapper - Maps WW3D API calls to Vulkan backend
//  *
//  * This wrapper provides real implementations of all WW3D functions that integrate
//  * with the Vulkan graphics backend. All rendering is routed through IGraphicsDriver.
//  */

// #include <cstdio>
// #include <cstdlib>
// #include <cstring>

// // Graphics driver system
// #include "../GraphicsDriverFactory.h"
// #include "../IGraphicsDriver.h"
// #include "../dx8buffer_compat.h"  // For SetGraphicsDriver

// // Forward declarations
// class RenderObjClass;
// class SceneClass;
// class CameraClass;
// class RenderInfoClass;

// // Phase 6: VertexMaterial class (moved before WW3D namespace for use in Init/Shutdown)
// class VertexMaterialClass {
// public:
//   // Static initialization methods (called by WW3D::Init/Shutdown)
//   static void Init();
//   static void Shutdown();

//   // Instance methods
//   virtual ~VertexMaterialClass() {}

//   virtual void Compute_CRC() const {}
//   virtual void Get_Ambient(void* out_vector) const {}
//   virtual void Get_Diffuse(void* out_vector) const {}
//   virtual void Get_Opacity() const {}
//   virtual void Get_Emissive(void* out_vector) const {}
// };

// struct Vector2 {
//   float X, Y;
//   Vector2() : X(0), Y(0) {}
//   Vector2(float x, float y) : X(x), Y(y) {}
// };

// struct Vector3 {
//   float X, Y, Z;
//   Vector3() : X(0), Y(0), Z(0) {}
//   Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
// };

// typedef int WW3DErrorType;
// #define WW3D_ERROR_OK 0
// #define WW3D_ERROR_GENERIC 1

// namespace WW3D {
//   // ============================================================================
//   // Global State Variables (referenced by ww3d.h inline functions)
//   // ============================================================================

//   // Time synchronization variables
//   float SyncTime = 0.0f;
//   unsigned int PreviousSyncTime = 0;
//   unsigned int FractionalSyncMs = 0;
//   unsigned int LogicFrameTimeMs = 33;  // ~30 FPS default
//   unsigned int FrameCount = 0;

//   // Feature flags
//   bool IsSortingEnabled = true;
//   bool AreStaticSortListsEnabled = true;
//   bool AreDecalsEnabled = true;
//   bool IsTexturingEnabled = true;
//   bool ThumbnailEnabled = false;
//   bool IsScreenUVBiased = false;

//   // Rendering mode variables
//   int PrelitMode = 0;  // PrelitModeEnum
//   int NPatchesLevel = 0;
//   int NPatchesGapFillingMode = 0;
//   bool MungeSortOnLoad = false;
//   bool OverbrightModifyOnLoad = false;

//   // Default screen size (used by render objects)
//   Vector2* DefaultNativeScreenSize = nullptr;

//   // Window handle (platform-specific)
//   static void* g_windowHandle = nullptr;
//   static int g_deviceWidth = 800;
//   static int g_deviceHeight = 600;
//   static int g_deviceBitDepth = 32;
//   static bool g_deviceWindowed = true;
//   static int g_textureReduction = 0;

//   // Graphics driver instance (Phase 41/54)
//   static Graphics::IGraphicsDriver* g_graphicsDriver = nullptr;
  
//   // Rendering state
//   static bool g_isInRenderBlock = false;
//   static float g_clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

//   // ============================================================================
//   // Core WW3D Functions
//   // ============================================================================

//   WW3DErrorType Init(void* hwnd, char* defaultpal, bool lite) {
//     fprintf(stderr, "[WW3D Vulkan] Init(%p) - Starting Vulkan backend initialization\n", hwnd);
//     fflush(stderr);
    
//     g_windowHandle = hwnd;
    
//     if (!hwnd) {
//       fprintf(stderr, "[WW3D Vulkan] ERROR: NULL window handle passed to Init()\n");
//       fflush(stderr);
//       return WW3D_ERROR_GENERIC;
//     }

//     // Phase 54: Initialize the graphics driver
//     fprintf(stderr, "[WW3D Vulkan] Creating graphics driver via factory...\n");
//     fflush(stderr);

//     // Use Vulkan backend on Linux
//     g_graphicsDriver = Graphics::GraphicsDriverFactory::CreateDriver(
//       Graphics::BackendType::Vulkan,
//       hwnd,
//       static_cast<uint32_t>(g_deviceWidth),
//       static_cast<uint32_t>(g_deviceHeight),
//       !g_deviceWindowed
//     );

//     if (g_graphicsDriver) {
//       fprintf(stderr, "[WW3D Vulkan] Graphics driver created successfully: %p\n",
//         static_cast<void*>(g_graphicsDriver));
//       fprintf(stderr, "[WW3D Vulkan] Backend: %s, Version: %s\n",
//         g_graphicsDriver->GetBackendName(),
//         g_graphicsDriver->GetVersionString());
//       fflush(stderr);
      
//       // Set the global graphics driver for buffer compatibility layer
//       Graphics::SetGraphicsDriver(g_graphicsDriver);
//     }
//     else {
//       fprintf(stderr, "[WW3D Vulkan] ERROR: Failed to create graphics driver!\n");
//       fflush(stderr);
//       return WW3D_ERROR_GENERIC;
//     }

//     // Initialize vertex material presets
//     fprintf(stderr, "[WW3D Vulkan] Calling VertexMaterialClass::Init()\n");
//     fflush(stderr);
//     VertexMaterialClass::Init();
//     fprintf(stderr, "[WW3D Vulkan] VertexMaterialClass::Init() completed\n");
//     fflush(stderr);

//     return WW3D_ERROR_OK;
//   }

//   void Shutdown() {
//     fprintf(stderr, "[WW3D Vulkan] Shutdown() called\n");
//     fflush(stderr);

//     // Shutdown vertex material presets
//     VertexMaterialClass::Shutdown();

//     // Destroy the graphics driver
//     if (g_graphicsDriver) {
//       fprintf(stderr, "[WW3D Vulkan] Destroying graphics driver...\n");
//       fflush(stderr);
//       Graphics::GraphicsDriverFactory::DestroyDriver(g_graphicsDriver);
//       g_graphicsDriver = nullptr;
//       Graphics::SetGraphicsDriver(nullptr);
//       fprintf(stderr, "[WW3D Vulkan] Graphics driver destroyed\n");
//       fflush(stderr);
//     }

//     g_windowHandle = nullptr;
//   }

//   WW3DErrorType Sync(bool wait_for_vsync) {
//     FrameCount++;
//     return WW3D_ERROR_OK;
//   }

//   // ============================================================================
//   // Frame Rendering Functions - CRITICAL for display
//   // ============================================================================

//   WW3DErrorType Begin_Render(bool clear, bool clear_zbuffer, const Vector3& clear_color, float clear_z) {
//     if (!g_graphicsDriver) {
//       fprintf(stderr, "[WW3D Vulkan] Begin_Render: ERROR - No graphics driver!\n");
//       fflush(stderr);
//       return WW3D_ERROR_GENERIC;
//     }
    
//     if (!g_graphicsDriver->IsInitialized()) {
//       fprintf(stderr, "[WW3D Vulkan] Begin_Render: ERROR - Graphics driver not initialized!\n");
//       fflush(stderr);
//       return WW3D_ERROR_GENERIC;
//     }
    
//     if (g_isInRenderBlock) {
//       fprintf(stderr, "[WW3D Vulkan] Begin_Render: WARNING - Already in render block\n");
//       fflush(stderr);
//       return WW3D_ERROR_OK;  // Allow nested calls
//     }

//     // Begin frame on the graphics driver
//     if (!g_graphicsDriver->BeginFrame()) {
//       fprintf(stderr, "[WW3D Vulkan] Begin_Render: ERROR - BeginFrame() failed!\n");
//       fflush(stderr);
//       return WW3D_ERROR_GENERIC;
//     }
    
//     g_isInRenderBlock = true;

//     // Store clear color for later use
//     g_clearColor[0] = clear_color.X;
//     g_clearColor[1] = clear_color.Y;
//     g_clearColor[2] = clear_color.Z;
//     g_clearColor[3] = 1.0f;

//     // Clear the render target
//     if (clear || clear_zbuffer) {
//       g_graphicsDriver->Clear(
//         g_clearColor[0], g_clearColor[1], g_clearColor[2], g_clearColor[3],
//         clear_zbuffer
//       );
//     }

//     static int beginRenderCount = 0;
//     if (beginRenderCount < 5) {
//       fprintf(stderr, "[WW3D Vulkan] Begin_Render #%d: SUCCESS - Frame started (clear=%d, clearZ=%d)\n",
//         beginRenderCount, clear, clear_zbuffer);
//       fflush(stderr);
//       beginRenderCount++;
//     }

//     return WW3D_ERROR_OK;
//   }

//   void End_Render(bool present) {
//     if (!g_graphicsDriver) {
//       fprintf(stderr, "[WW3D Vulkan] End_Render: ERROR - No graphics driver!\n");
//       fflush(stderr);
//       return;
//     }
    
//     if (!g_isInRenderBlock) {
//       fprintf(stderr, "[WW3D Vulkan] End_Render: WARNING - Not in render block\n");
//       fflush(stderr);
//       return;
//     }

//     // End the frame
//     g_graphicsDriver->EndFrame();
    
//     // Present to screen if requested
//     if (present) {
//       if (!g_graphicsDriver->Present()) {
//         static int presentFailCount = 0;
//         if (presentFailCount < 5) {
//           fprintf(stderr, "[WW3D Vulkan] End_Render: WARNING - Present() returned false\n");
//           fflush(stderr);
//           presentFailCount++;
//         }
//       }
//     }

//     g_isInRenderBlock = false;

//     static int endRenderCount = 0;
//     if (endRenderCount < 5) {
//       fprintf(stderr, "[WW3D Vulkan] End_Render #%d: Frame ended (present=%d)\n",
//         endRenderCount, present);
//       fflush(stderr);
//       endRenderCount++;
//     }
//   }

//   // ============================================================================
//   // Render Functions (Scene/Camera based)
//   // ============================================================================

//   void Render(SceneClass* scene, CameraClass* camera, bool clipping, bool sort, const Vector3& fog_color) {
//     // Scene rendering - would iterate through scene objects
//     // For now, this is a stub as scene rendering requires full object hierarchy
//   }

//   void Render(RenderObjClass& obj, RenderInfoClass& render_info) {
//     // Single object render - stub
//   }

//   void Flush(RenderInfoClass& render_info) {
//     // Flush pending draw calls - stub
//   }

//   void Render_And_Clear_Static_Sort_Lists(RenderInfoClass& render_info) {
//     // Static sort list rendering - stub
//   }

//   void Set_Collision_Box_Display_Mask(int mask) {
//     // Debug helper - no-op
//   }

//   // ============================================================================
//   // Device/Resolution Functions
//   // ============================================================================

//   void* Get_Window() {
//     return g_windowHandle;
//   }

//   void Get_Device_Resolution(int& width, int& height, int& bitDepth, bool& windowed) {
//     width = g_deviceWidth;
//     height = g_deviceHeight;
//     bitDepth = g_deviceBitDepth;
//     windowed = g_deviceWindowed;
//   }

//   void Get_Render_Target_Resolution(int& width, int& height, int& bitDepth, bool& windowed) {
//     width = g_deviceWidth;
//     height = g_deviceHeight;
//     bitDepth = g_deviceBitDepth;
//     windowed = g_deviceWindowed;
//   }

//   WW3DErrorType Set_Device_Resolution(int width, int height, int bitDepth, int device, bool windowed) {
//     fprintf(stderr, "[WW3D Vulkan] Set_Device_Resolution(%d x %d, %d-bit, windowed=%d)\n",
//       width, height, bitDepth, windowed);
//     fflush(stderr);
    
//     g_deviceWidth = width;
//     g_deviceHeight = height;
//     g_deviceBitDepth = bitDepth;
//     g_deviceWindowed = windowed;
    
//     // Update viewport if driver is initialized
//     if (g_graphicsDriver && g_graphicsDriver->IsInitialized()) {
//       Graphics::Viewport vp;
//       vp.x = 0;
//       vp.y = 0;
//       vp.width = width;
//       vp.height = height;
//       vp.minZ = 0.0f;
//       vp.maxZ = 1.0f;
//       g_graphicsDriver->SetViewport(vp);
//     }
    
//     return WW3D_ERROR_OK;
//   }

//   WW3DErrorType Set_Render_Device(int device, int resx, int resy, int bits, int windowed_mode,
//     bool resize_window, bool reset_device, bool restore_assets) {
//     fprintf(stderr, "[WW3D Vulkan] Set_Render_Device(device=%d, %d x %d, %d-bit, windowed=%d)\n",
//       device, resx, resy, bits, windowed_mode);
//     fflush(stderr);
    
//     g_deviceWidth = resx;
//     g_deviceHeight = resy;
//     g_deviceBitDepth = bits;
//     g_deviceWindowed = (windowed_mode != 0);
    
//     // If graphics driver exists but needs re-initialization with new resolution
//     if (g_graphicsDriver) {
//       // For now, just update the viewport
//       Graphics::Viewport vp;
//       vp.x = 0;
//       vp.y = 0;
//       vp.width = resx;
//       vp.height = resy;
//       vp.minZ = 0.0f;
//       vp.maxZ = 1.0f;
//       g_graphicsDriver->SetViewport(vp);
      
//       fprintf(stderr, "[WW3D Vulkan] Set_Render_Device: Viewport updated to %d x %d\n", resx, resy);
//       fflush(stderr);
//     }
    
//     return WW3D_ERROR_OK;
//   }

//   void* Get_Render_Device_Desc(int device) {
//     return nullptr;  // Device descriptor not implemented yet
//   }

//   // ============================================================================
//   // Texture Functions
//   // ============================================================================

//   int Get_Texture_Bitdepth() {
//     return 32;
//   }

//   void Set_Texture_Bitdepth(int bits) {
//     // Stub - texture bit depth management
//   }

//   int Get_Texture_Reduction() {
//     return g_textureReduction;
//   }

//   void Set_Texture_Reduction(int reduction) {
//     g_textureReduction = reduction;
//   }

//   void Set_Texture_Reduction(int reduction, int someflag) {
//     g_textureReduction = reduction;
//   }

//   void Enable_Texturing(bool enable) {
//     IsTexturingEnabled = enable;
//   }

//   // ============================================================================
//   // Timing Functions
//   // ============================================================================

//   void Update_Logic_Frame_Time(float time_ms) {
//     LogicFrameTimeMs = static_cast<unsigned int>(time_ms);
//   }

//   // ============================================================================
//   // Static Sort List Functions
//   // ============================================================================

//   void Add_To_Static_Sort_List(RenderObjClass* obj, unsigned int sortLevel) {
//     // TODO: Implement static sort list for Vulkan
//   }
  
//   // ============================================================================
//   // WW3D static member definitions
//   // ============================================================================
//   float DecalRejectionDistance = 1000000.0f;
//   bool SnapshotActivated = false;
//   bool IsColoringEnabled = false;
// }

// // ============================================================================
// // External symbol support
// // ============================================================================

// // Shader class stubs
// class ShaderClass {
// public:
//   virtual ~ShaderClass() {}
//   virtual int Get_SS_Category() const { return 0; }
//   virtual int Guess_Sort_Level() const { return 0; }
// };

// // Texture class stubs
// class TextureClass {
// public:
//   virtual ~TextureClass() {}
//   virtual int Get_Texture_Memory_Usage() const { return 0; }
// };

// class TextureBaseClass : public TextureClass {
// public:
//   virtual void* Peek_D3D_Base_Texture() const { return nullptr; }
// };

// // Bezier segment stubs
// class BezierSegment {
// public:
//   virtual ~BezierSegment() {}
//   virtual void getSegmentPoints(int segments, void* points_vector) const {}
//   virtual float getApproximateLength(float tolerance) const { return 0.0f; }
// };

// extern "C" {
//   void _g_LastErrorDump() {
//     fprintf(stderr, "[WW3D Vulkan] Crash dump requested\n");
//   }
// }

