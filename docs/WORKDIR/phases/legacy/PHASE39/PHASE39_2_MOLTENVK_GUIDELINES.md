# MoltenVK Implementation Guidelines - GeneralsX Phase 39

**Document Status**: Active Implementation  
**Last Updated**: October 30, 2025  
**Phase**: 39.2 (Vulkan Backend Implementation)  
**Platform**: macOS (Apple Silicon & Intel), Linux, Windows

---

## 1. MoltenVK Recommended Usage Model

### Official Recommendation (from MoltenVK Documentation)

> "For desktop, iOS, and iPadOS applications, the **recommended usage model is to use the MoltenVK dynamic library in conjunction with the Vulkan Loader**. In this scenario the MoltenVK library takes on the role of the ICD from the point of view of the application and the Vulkan Loader."

### What This Means

**ICD = Installable Client Driver**

```
Application
    ↓ (links to)
Vulkan Loader (vulkan.so/vulkan.dylib/vulkan.dll)
    ↓ (discovers and delegates to)
MoltenVK ICD (libMoltenVK.dylib on macOS)
    ↓ (translates to)
Metal Framework (GPU.framework on macOS)
    ↓
GPU Hardware (Apple Silicon / Intel)
```

### Why This Architecture?

1. **Portability**: Same code works on Windows (NVIDIA/AMD ICD), Linux (Mesa ICD), macOS (MoltenVK ICD)
2. **Validation Layers**: Vulkan Loader enables validation layer inspection and debugging
3. **Flexibility**: Can swap ICDs or validation layers without recompiling application
4. **Standard**: Follows Vulkan ecosystem conventions across all platforms

---

## 2. Development vs Production Model

### ✅ Development Model (What We're Implementing)

**Linking**:
```bash
# Link ONLY to Vulkan Loader
-L/usr/local/lib -lvulkan

# Do NOT link directly to MoltenVK
# ❌ Do NOT use: -L/usr/local/lib -lMoltenVK
```

**Runtime Libraries** (from system Vulkan SDK during development):
```
/usr/local/lib/libvulkan.dylib          (Vulkan Loader)
/usr/local/lib/libMoltenVK.dylib        (MoltenVK ICD, auto-discovered)
```

**Configuration Files** (ICD manifests):
```
/usr/local/etc/vulkan/icd.d/MoltenVK_icd.json
```

**Validation Layers** (enabled for debugging):
```
/usr/local/etc/vulkan/explicit_layer.d/VK_LAYER_KHRONOS_validation.json
```

**Advantages**:
- ✅ Full access to Vulkan Validation Layers (critical for debugging)
- ✅ Can dynamically choose between Metal (via MoltenVK) and other backends
- ✅ Recommended by Khronos and LunarG (Vulkan SDK maintainers)
- ✅ Professional development environment

### ❌ NOT Recommended: Direct MoltenVK Linking

```bash
# ❌ NOT RECOMMENDED
-L/usr/local/lib -lMoltenVK

# ❌ Cannot use validation layers with direct linking
# ❌ Sacrifices portability across platforms
# ❌ Not a professional approach
```

---

## 3. GeneralsX Implementation Strategy

### Phase 39.2.2: CMake Configuration

**File**: `cmake/vulkan.cmake`

**Key Decisions**:

```cmake
# ✅ CORRECT: Find and link to Vulkan Loader
find_package(Vulkan REQUIRED)
# Sets: Vulkan::Loader target (use this)
#       Vulkan::Vulkan target (includes headers + loader)

# Link target configuration:
target_link_libraries(ww3d2 PUBLIC Vulkan::Loader)

# Enable validation layers in debug builds
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(VULKAN_VALIDATION_ENABLED=1)
endif()
```

**NOT Direct MoltenVK Linking**:

```cmake
# ❌ This is wrong and we DON'T do this:
find_package(MoltenVK REQUIRED)
target_link_libraries(ww3d2 PUBLIC MoltenVK::MoltenVK)  # ❌ NO!
```

### Phase 39.2.2: Graphics Backend Initialization

**File**: `graphics_backend_dxvk.cpp`

**Key Pattern**:

```cpp
// During Vulkan instance creation in CreateInstance()
VkApplicationInfo appInfo{};
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
appInfo.pApplicationName = "GeneralsX";
appInfo.apiVersion = VK_API_VERSION_1_2;  // ← Vulkan 1.2 or later

VkInstanceCreateInfo createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
createInfo.pApplicationInfo = &appInfo;

// Platform-specific extensions (Vulkan Loader discovers these)
#ifdef __APPLE__
    // Metal surface: vkCreateMetalSurfaceEXT() provided by Vulkan Loader
    createInfo.ppEnabledExtensionNames = 
        {VK_KHR_SURFACE_EXTENSION_NAME,
         VK_EXT_METAL_SURFACE_EXTENSION_NAME};
    createInfo.enabledExtensionCount = 2;
    
    // On macOS, Vulkan Loader delegates to MoltenVK automatically
#endif

// Create instance with Vulkan Loader
VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
// Vulkan Loader receives this call, discovers MoltenVK ICD,
// MoltenVK translates vkCreateInstance to Metal equivalents
```

### Platform-Specific Surface Creation

**macOS**:
```cpp
// CreateSurface() implementation
VkMetalSurfaceCreateInfoEXT createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
createInfo.pLayer = (__bridge CAMetalLayer*)metalLayer;

// Vulkan Loader routes this to MoltenVK
// MoltenVK wraps the Metal layer for presentation
VkResult result = vkCreateMetalSurfaceEXT(m_instance, &createInfo, nullptr, &m_surface);
```

**Linux**:
```cpp
// CreateSurface() implementation
VkXcbSurfaceCreateInfoKHR createInfo{};
createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
createInfo.connection = xcbConnection;
createInfo.window = xcbWindow;

// Vulkan Loader discovers native Vulkan ICD (Mesa, AMD, NVIDIA)
// ICD translates to GPU driver calls
VkResult result = vkCreateXcbSurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
```

---

## 4. Validation Layers Support

### Development Builds: Layers ENABLED

```cpp
// In graphics_backend_dxvk.cpp CreateInstance()

#if VALIDATION_LAYERS_ENABLED  // Set in debug builds
    const char* VALIDATION_LAYERS[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    VkInstanceCreateInfo createInfo{};
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
    
    // Vulkan Loader provides these layers during development
    // From: /usr/local/etc/vulkan/explicit_layer.d/
#endif
```

**Runtime Validation Configuration** (macOS):

```bash
# Enable all validation checks
export VK_LAYER_PATH=/usr/local/etc/vulkan/explicit_layer.d

# Run with validation
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/vulkan_validation.log

# Check for validation errors
grep "Validation" logs/vulkan_validation.log
```

### Production Builds: Layers DISABLED

```cpp
// Release builds disable validation for performance
#if VALIDATION_LAYERS_ENABLED == 0
    createInfo.enabledLayerCount = 0;
    // No performance overhead
#endif
```

---

## 5. Runtime Library Distribution

### Development (Current)

**Location**: System Vulkan SDK installation
```
/usr/local/lib/
  ├── libvulkan.dylib             (Vulkan Loader)
  └── libMoltenVK.dylib           (MoltenVK ICD)

/usr/local/etc/vulkan/
  ├── icd.d/
  │   └── MoltenVK_icd.json        (ICD manifest for autodiscovery)
  └── explicit_layer.d/
      └── VK_LAYER_KHRONOS_validation.json
```

**Application does NOT bundle these** - just links to Vulkan Loader.

### Production (Shipping GeneralsX on macOS)

**Location**: Inside application bundle
```
GeneralsX.app/
├── Contents/
│   ├── MacOS/
│   │   └── GeneralsXZH              (executable, links to Vulkan Loader only)
│   ├── Frameworks/
│   │   ├── libvulkan.dylib         (Vulkan Loader)
│   │   └── libMoltenVK.dylib       (MoltenVK ICD)
│   └── Resources/
│       └── vulkan/
│           ├── icd.d/
│           │   └── MoltenVK_icd.json
│           └── explicit_layer.d/
│               └── VK_LAYER_KHRONOS_validation.json (optional for debug builds)
```

**Install Name References** (rpath):
```bash
# Vulkan Loader references MoltenVK via relative path
install_name_tool -change /usr/local/lib/libMoltenVK.dylib \
  @loader_path/../Frameworks/libMoltenVK.dylib \
  Contents/Frameworks/libvulkan.dylib
```

---

## 6. Mixed Vulkan/Metal Usage (Optional)

### Advanced Feature: VK_EXT_metal_objects Extension

The documentation mentions:
> "Mixing of Vulkan and Metal capabilities can be done using the VK_EXT_metal_objects extension."

**When to Use**:
- Interop between Vulkan render passes and Metal rendering
- Access Metal objects directly for performance optimization
- Use Metal-specific features (like Apple Silicon's tile-based deferred rendering)

**NOT a requirement for GeneralsX Phase 39**, but available for future optimization:

```cpp
// In CreateInstance(), enable extension (if available)
#ifdef VK_EXT_METAL_OBJECTS_EXTENSION_NAME
    const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_METAL_SURFACE_EXTENSION_NAME,
        VK_EXT_METAL_OBJECTS_EXTENSION_NAME,  // Optional for interop
    };
#endif
```

---

## 7. Checklist: MoltenVK Best Practices Compliance

### ✅ Implementation Compliance

- [x] **Vulkan Loader**: Link to `Vulkan::Loader` (NOT direct MoltenVK)
  - File: `cmake/vulkan.cmake` line ~148
  - Status: Configured in `find_package(Vulkan REQUIRED)`

- [x] **Validation Layers**: Enabled in debug builds via CMake
  - File: `cmake/vulkan.cmake` line ~177
  - Status: `VULKAN_VALIDATION_ENABLED` flag in debug builds

- [x] **Platform Extensions**: Metal, Win32, XCB/Xlib declared
  - File: `graphics_backend_dxvk.h` line ~45-63
  - Status: Included in instance creation

- [x] **MoltenVK ICD Discovery**: Automatic via Vulkan Loader
  - File: `graphics_backend_dxvk.cpp` line ~350-380
  - Status: Vulkan Loader manages discovery, not hardcoded

- [x] **Metal Framework Linkage** (macOS): Via Vulkan Loader, not direct
  - File: `cmake/vulkan.cmake` line ~95 (notes only, actual linking through Vulkan::Loader)
  - Status: Not directly linked, delegated to MoltenVK

- [x] **No Static MoltenVK Linking**: Pure dynamic linking
  - File: `cmake/vulkan.cmake` line ~96-99
  - Status: MoltenVK search is QUIET, not required for linking

### 🟡 Pending Implementation (Phase 39.2.3-5)

- [ ] **Device Creation**: Create VkDevice with appropriate queue families
  - Target: `CreateDevice()` in `graphics_backend_dxvk.cpp`

- [ ] **Surface Creation**: Platform-specific (Metal on macOS)
  - Target: `CreateSurface()` in `graphics_backend_dxvk.cpp`
  - Note: Will use `vkCreateMetalSurfaceEXT()` on macOS

- [ ] **Swapchain Management**: Handle presentation via Vulkan
  - Target: `CreateSwapchain()` in `graphics_backend_dxvk.cpp`

- [ ] **Command Buffer Recording**: Record Vulkan commands for frame rendering
  - Target: `BeginScene()`, `EndScene()`, `RecordFrameCommands()` in `graphics_backend_dxvk.cpp`

---

## 8. Troubleshooting MoltenVK Issues

### Issue: "Cannot find Vulkan Loader"

**Solution**:
```bash
# Install Vulkan SDK
brew install vulkan-sdk

# Verify installation
ls /usr/local/lib/libvulkan.dylib
ls /usr/local/lib/libMoltenVK.dylib

# Set environment for CMake
export VULKAN_SDK=/usr/local/share/vulkan
cmake --preset macos-arm64 -DVULKAN_SDK_PATH=$VULKAN_SDK
```

### Issue: "Validation layers not working"

**Solution**:
```bash
# Verify layer manifest
cat /usr/local/etc/vulkan/explicit_layer.d/VK_LAYER_KHRONOS_validation.json

# Ensure debug build
cmake --build build/macos-arm64 --config Debug

# Check CMake flag
grep "VULKAN_VALIDATION_ENABLED" build/macos-arm64/CMakeCache.txt
```

### Issue: "Metal surface creation failed"

**Diagnosis**:
1. Verify macOS version >= 10.11 (required for Metal)
2. Check Metal framework availability: `grep -r "CAMetalLayer" build/`
3. Enable Vulkan validation for error messages
4. Check LLDB backtrace for Metal framework symbols

**Solution**:
```bash
cd $HOME/GeneralsX/GeneralsMD
USE_METAL=1 lldb -o run -o bt -o quit ./GeneralsXZH 2>&1 | tee logs/metal_debug.log
grep "vkCreateMetalSurfaceEXT" logs/metal_debug.log
```

---

## 9. References

### Official MoltenVK Documentation
- MoltenVK Runtime User Guide: https://github.com/KhronosGroup/MoltenVK/blob/master/README.md
- Vulkan SDK Download: https://vulkan.lunarg.com/
- macOS Specific: https://vulkan.lunarg.com/doc/sdk/1.4.328.1/mac/

### GeneralsX Phase References
- Phase 38: Graphics Backend Abstraction (`docs/WORKDIR/phases/3/PHASE38/PHASE38_6_VULKAN_ANALYSIS.md`)
- Phase 39.1: DXVK Discovery (`docs/WORKDIR/phases/3/PHASE39/PHASE39_1_DXVK_DISCOVERY.md`)
- Phase 39.2: Vulkan Wrapper Plan (`docs/WORKDIR/phases/3/PHASE39/PHASE39_2_VULKAN_WRAPPER_PLAN.md`)

### CMake Configuration
- File: `cmake/vulkan.cmake` (created Oct 30, 2025)
- Integration: To be added to `Core/Libraries/Source/WWVegas/WW3D2/CMakeLists.txt`

---

## 10. Next Steps (Phase 39.2.3 Implementation)

### Immediate Actions
1. **Integrate cmake/vulkan.cmake** into WW3D2 CMakeLists.txt
2. **Implement CreateDevice()** with proper queue discovery
3. **Implement CreateSurface()** with platform-specific Metal surface
4. **Test ICD Discovery** via Vulkan Loader with validation enabled

### Testing Commands
```bash
# Rebuild with Vulkan support
cmake --preset macos-arm64
cmake --build build/macos-arm64 --target GeneralsXZH -j 4 2>&1 | tee logs/build_vulkan.log

# Run with debug output
USE_METAL=1 ./GeneralsXZH 2>&1 | tee logs/vulkan_run.log

# Check for MoltenVK initialization
grep -i "moltenvk\|metal.*surface" logs/vulkan_run.log
```

---

**Document Compliance**: ✅ All MoltenVK best practices documented and planned for implementation.
