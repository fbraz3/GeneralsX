#!/usr/bin/env python3
"""
dxvk-macos-patches.py - Apply macOS-specific patches to DXVK v2.6 source.

These patches are necessary because DXVK's "native" build targets Linux only.
macOS requires:
  1. util_win32_compat.h: __unix__ not defined on macOS, add __APPLE__ guard
  2. util_env.cpp:        pthread_setname_np macOS takes 1 arg (not 2 like Linux)
  3. util_small_vector.h: lzcnt(n-1) where n is size_t (unsigned long) is
                          ambiguous between uint32_t and uint64_t overloads
  4. util_bit.h:          Add uintptr_t overloads for tzcnt/lzcnt
  5. src/*/meson.build:   --version-script is GNU ld only, not macOS ld
  6. util_env.cpp:        getExePath() has no macOS branch; add _NSGetExecutablePath
                          (/proc/self/exe doesn't exist on macOS)
  7. vulkan_loader.cpp:   loadVulkanLibrary() only tries libvulkan.so/libvulkan.so.1 (Linux).
                          Add macOS branch: libvulkan.dylib, libvulkan.1.dylib, libMoltenVK.dylib
  8. dxvk_extensions.h +  MoltenVK requires VK_KHR_portability_enumeration instance extension
     dxvk_instance.cpp:   AND VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR in VkInstanceCreateInfo
                          or vkEnumeratePhysicalDevices returns VK_ERROR_INCOMPATIBLE_DRIVER.
  9. dxvk_adapter.cpp:   vkCreateDevice fails with VK_ERROR_FEATURE_NOT_PRESENT for valid
                          features (tessellationShader, shaderFloat64, robustBufferAccess2,
                          nullDescriptor) because:
                          (a) VK_KHR_portability_subset must be enabled + its features struct
                              added to pNext when the device supports it (MoltenVK requirement);
                          (b) core VkPhysicalDeviceFeatures must be masked against what the
                              device actually supports before calling vkCreateDevice.
 10. dxvk_adapter.cpp:   vkCreateDevice fails with VK_ERROR_FEATURE_NOT_PRESENT for the 1st
                          and 3rd flags in VkPhysicalDeviceRobustness2FeaturesKHR
                          (robustBufferAccess2, nullDescriptor). MoltenVK incorrectly reports
                          these as supported in vkGetPhysicalDeviceFeatures2 but rejects them
                          in vkCreateDevice when VK_KHR_portability_subset is active.
                          Fix: do not request robustness2 features on macOS.
 11. d3d8_batch.h:        D3D8Batcher::StateChange() restores D3D9 state post-flush by calling
                          m_device->SetStreamSource(0, GetD3D9Nullable(m_stream), ...) and
                          m_device->SetIndices(GetD3D9Nullable(m_indices)). Since m_stream is
                          always a D3D8BatchBuffer with nullptr D3D9 backing, GetD3D9Nullable
                          returns null, nullifying D3D9 stream 0. This races with the caller:
                          D3D8Device::SetStreamSource first sets a real D3D9 vertex buffer,
                          THEN triggers StateChange() via the batcher — StateChange nullifies
                          the just-set buffer, leaving DrawIndexedPrimitive with no stream
                          source → null VkBuffer → MVKBuffer::getMTLBuffer() null deref crash.
                          Fix: remove the post-draw SetStreamSource/SetIndices restore calls;
                          DrawIndexedPrimitiveUP already nulls them per D3D9 spec, and the
                          D3D8Device layer correctly re-sets state before any subsequent draw.

Usage: dxvk-macos-patches.py <dxvk_source_dir>
"""

import sys
import os
import re

def patch_file(path, transform_fn, description):
    """Apply a transformation to a file and report the result."""
    if not os.path.exists(path):
        print(f"SKIP (not found): {os.path.basename(path)}")
        return False
    content = open(path).read()
    patched = transform_fn(content)
    if patched == content:
        print(f"OK (already patched): {os.path.basename(path)} - {description}")
        return True
    open(path, 'w').write(patched)
    print(f"PATCHED: {os.path.basename(path)} - {description}")
    return True


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <dxvk_source_dir>")
        sys.exit(1)

    src = sys.argv[1]
    if not os.path.isdir(src):
        print(f"ERROR: {src} is not a directory")
        sys.exit(1)

    # Patch 1: util_win32_compat.h
    # macOS does NOT define __unix__, so LoadLibraryA/GetProcAddress/FreeLibrary
    # stubs are never compiled. Add __APPLE__ to the guard.
    patch_file(
        os.path.join(src, "src/util/util_win32_compat.h"),
        lambda c: c.replace(
            "#if defined(__unix__)",
            "#if defined(__unix__) || defined(__APPLE__)"
        ),
        "__unix__ guard: add __APPLE__"
    )

    # Patch 2: util_env.cpp
    # macOS pthread_setname_np only takes 1 argument: pthread_setname_np(name)
    # Linux signature:                                 pthread_setname_np(thread, name)
    patch_file(
        os.path.join(src, "src/util/util_env.cpp"),
        lambda c: c.replace(
            "    ::pthread_setname_np(pthread_self(), posixName.data());",
            "#ifdef __APPLE__\n"
            "    ::pthread_setname_np(posixName.data());\n"
            "#else\n"
            "    ::pthread_setname_np(pthread_self(), posixName.data());\n"
            "#endif"
        ),
        "pthread_setname_np 1-arg macOS signature"
    )

    # Patch 3: util_small_vector.h
    # size_t is 'unsigned long' on macOS arm64, distinct from both uint32_t and
    # uint64_t. Cast explicitly to avoid ambiguous overload resolution.
    patch_file(
        os.path.join(src, "src/util/util_small_vector.h"),
        lambda c: c.replace(
            "bit::lzcnt(n - 1)",
            "bit::lzcnt(static_cast<uint64_t>(n - 1))"
        ),
        "lzcnt: cast size_t→uint64_t to resolve overload ambiguity"
    )

    # Patch 4: util_bit.h
    # Add uintptr_t overloads for tzcnt/lzcnt.
    # On macOS arm64, uintptr_t is 'unsigned long' (neither uint32_t=unsigned int
    # nor uint64_t=unsigned long long). Template overload resolution fails.
    def patch_util_bit(c):
        if "uintptr_t n" in c:
            return c  # already patched
        extra = (
            "\n#if defined(__APPLE__)\n"
            "  // macOS arm64: uintptr_t is 'unsigned long', distinct from\n"
            "  // uint32_t (unsigned int) and uint64_t (unsigned long long).\n"
            "  // Explicit overloads to resolve template ambiguity.\n"
            "  inline uint32_t tzcnt(uintptr_t n) { return tzcnt(static_cast<uint64_t>(n)); }\n"
            "  inline uint32_t lzcnt(uintptr_t n) { return lzcnt(static_cast<uint64_t>(n)); }\n"
            "#endif\n"
            "\n"
        )
        # Insert before the last closing brace of the namespace
        last_brace = c.rfind('\n}')
        if last_brace == -1:
            return c
        return c[:last_brace] + extra + c[last_brace:]

    patch_file(
        os.path.join(src, "src/util/util_bit.h"),
        patch_util_bit,
        "uintptr_t overloads for tzcnt/lzcnt (macOS arm64)"
    )

    # Patch 5: meson.build files
    # macOS ld does not support --version-script. Guard the flag with a
    # platform != 'darwin' check.
    meson_files = [
        "src/dxgi/meson.build",
        "src/d3d8/meson.build",
        "src/d3d9/meson.build",
        "src/d3d10/meson.build",
        "src/d3d11/meson.build",
    ]
    for relpath in meson_files:
        patch_file(
            os.path.join(src, relpath),
            lambda c: re.sub(
                r"( +)(.*'-Wl,--version-script'.*\n)",
                r"  if platform != 'darwin'\n\1\2  endif\n",
                c
            ),
            "--version-script: guard with platform != 'darwin'"
        )

    # Patch 6: util_env.cpp - getExePath() has no macOS branch
    # getExePath() only handles _WIN32, __linux__, __FreeBSD__.
    # On macOS the function body is empty → compiler inserts brk 1 (UB trap).
    # Fix: add __APPLE__ include and __APPLE__ implementation branch.
    def patch_util_env_getexepath(c):
        # Already patched?
        if "_NSGetExecutablePath" in c:
            return c
        # Add macOS include after the FreeBSD include block
        c = c.replace(
            "#elif defined(__FreeBSD__)\n"
            "#include <sys/sysctl.h>\n"
            "#include <unistd.h>\n"
            "#include <limits.h>\n"
            "#endif",
            "#elif defined(__FreeBSD__)\n"
            "#include <sys/sysctl.h>\n"
            "#include <unistd.h>\n"
            "#include <limits.h>\n"
            "#elif defined(__APPLE__)\n"
            "#include <mach-o/dyld.h>\n"
            "#include <limits.h>\n"
            "#endif"
        )
        # Add macOS getExePath() branch before the closing #endif of getExePath
        c = c.replace(
            "    return std::string(exePath);\n"
            "#endif\n"
            "  }\n"
            "\n"
            "\n"
            "  void setThreadName",
            "    return std::string(exePath);\n"
            "#elif defined(__APPLE__)\n"
            "    // macOS: _NSGetExecutablePath from <mach-o/dyld.h>\n"
            "    // /proc/self/exe does not exist on macOS.\n"
            "    char buf[PATH_MAX] = {};\n"
            "    uint32_t size = sizeof(buf);\n"
            "    if (_NSGetExecutablePath(buf, &size) != 0)\n"
            "      return std::string();\n"
            "    return std::string(buf);\n"
            "#endif\n"
            "  }\n"
            "\n"
            "\n"
            "  void setThreadName"
        )
        return c

    patch_file(
        os.path.join(src, "src/util/util_env.cpp"),
        patch_util_env_getexepath,
        "getExePath(): add __APPLE__ branch using _NSGetExecutablePath"
    )

    # Patch 7: vulkan_loader.cpp
    # loadVulkanLibrary() only tries libvulkan.so / libvulkan.so.1 on non-Windows.
    # On macOS the Vulkan loader from LunarG SDK is libvulkan.dylib.
    # DXVK succeeds at dlopen only when the library name matches exactly.
    def patch_vulkan_loader(c):
        old = (
            "  static std::pair<HMODULE, PFN_vkGetInstanceProcAddr> loadVulkanLibrary() {\n"
            "    static const std::array<const char*, 2> dllNames = {{\n"
            "#ifdef _WIN32\n"
            "      \"winevulkan.dll\",\n"
            "      \"vulkan-1.dll\",\n"
            "#else\n"
            "      \"libvulkan.so\",\n"
            "      \"libvulkan.so.1\",\n"
            "#endif\n"
            "    }};\n"
        )
        new = (
            "  static std::pair<HMODULE, PFN_vkGetInstanceProcAddr> loadVulkanLibrary() {\n"
            "#ifdef _WIN32\n"
            "    static const std::array<const char*, 2> dllNames = {{\n"
            "      \"winevulkan.dll\",\n"
            "      \"vulkan-1.dll\",\n"
            "    }};\n"
            "#elif defined(__APPLE__)\n"
            "    // macOS: try the Vulkan loader (from Vulkan SDK) and MoltenVK directly.\n"
            "    // LoadLibraryA() maps to dlopen() on non-Windows DXVK native builds.\n"
            "    static const std::array<const char*, 4> dllNames = {{\n"
            "      \"libvulkan.dylib\",\n"
            "      \"libvulkan.1.dylib\",\n"
            "      \"libvulkan.1.4.341.dylib\",\n"
            "      \"libMoltenVK.dylib\",\n"
            "    }};\n"
            "#else\n"
            "    static const std::array<const char*, 2> dllNames = {{\n"
            "      \"libvulkan.so\",\n"
            "      \"libvulkan.so.1\",\n"
            "    }};\n"
            "#endif\n"
        )
        return c.replace(old, new)

    patch_file(
        os.path.join(src, "src/vulkan/vulkan_loader.cpp"),
        patch_vulkan_loader,
        "loadVulkanLibrary(): add macOS-specific Vulkan/MoltenVK dylib names"
    )

    # Patch 8a: dxvk_extensions.h
    # MoltenVK requires VK_KHR_portability_enumeration as an Optional instance
    # extension. Without it, VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
    # cannot be set and vkEnumeratePhysicalDevices returns VK_ERROR_INCOMPATIBLE_DRIVER.
    def patch_extensions_h(c):
        old = (
            "  struct DxvkInstanceExtensions {\n"
            "    DxvkExt extDebugUtils                   = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,                      DxvkExtMode::Optional };\n"
            "    DxvkExt extSurfaceMaintenance1          = { VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,            DxvkExtMode::Optional };\n"
            "    DxvkExt khrGetSurfaceCapabilities2      = { VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,       DxvkExtMode::Optional };\n"
            "    DxvkExt khrSurface                      = { VK_KHR_SURFACE_EXTENSION_NAME,                          DxvkExtMode::Required };\n"
            "  };"
        )
        new = (
            "  struct DxvkInstanceExtensions {\n"
            "    DxvkExt extDebugUtils                   = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,                      DxvkExtMode::Optional };\n"
            "    DxvkExt extSurfaceMaintenance1          = { VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME,            DxvkExtMode::Optional };\n"
            "    DxvkExt khrGetSurfaceCapabilities2      = { VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,       DxvkExtMode::Optional };\n"
            "    DxvkExt khrPortabilityEnumeration       = { VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,          DxvkExtMode::Optional };\n"
            "    DxvkExt khrSurface                      = { VK_KHR_SURFACE_EXTENSION_NAME,                          DxvkExtMode::Required };\n"
            "  };"
        )
        return c.replace(old, new)

    patch_file(
        os.path.join(src, "src/dxvk/dxvk_extensions.h"),
        patch_extensions_h,
        "DxvkInstanceExtensions: add khrPortabilityEnumeration (Optional) for MoltenVK"
    )

    # Patch 8b: dxvk_instance.cpp (two sub-patches)
    def patch_instance_cpp(c):
        # Sub-patch 8b-1: Add khrPortabilityEnumeration to getExtensionList()
        old1 = (
            "  std::vector<DxvkExt*> DxvkInstance::getExtensionList(DxvkInstanceExtensions& ext, bool withDebug) {\n"
            "    std::vector<DxvkExt*> result = {{\n"
            "      &ext.extSurfaceMaintenance1,\n"
            "      &ext.khrGetSurfaceCapabilities2,\n"
            "      &ext.khrSurface,\n"
            "    }};"
        )
        new1 = (
            "  std::vector<DxvkExt*> DxvkInstance::getExtensionList(DxvkInstanceExtensions& ext, bool withDebug) {\n"
            "    std::vector<DxvkExt*> result = {{\n"
            "      &ext.extSurfaceMaintenance1,\n"
            "      &ext.khrGetSurfaceCapabilities2,\n"
            "      &ext.khrSurface,\n"
            "      // MoltenVK/macOS: VK_KHR_portability_enumeration must be enabled or\n"
            "      // vkEnumeratePhysicalDevices returns VK_ERROR_INCOMPATIBLE_DRIVER (-9).\n"
            "      &ext.khrPortabilityEnumeration,\n"
            "    }};"
        )
        c = c.replace(old1, new1)
        # Sub-patch 8b-2: Set VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR in VkInstanceCreateInfo
        old2 = (
            "      VkInstanceCreateInfo info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };\n"
            "      info.pApplicationInfo         = &appInfo;\n"
            "      info.enabledLayerCount        = layerList.count();\n"
            "      info.ppEnabledLayerNames      = layerList.names();\n"
            "      info.enabledExtensionCount    = m_extensionNames.count();\n"
            "      info.ppEnabledExtensionNames  = m_extensionNames.names();\n"
            "\n"
            "      VkResult status = m_vkl->vkCreateInstance(&info, nullptr, &instance);"
        )
        new2 = (
            "      // MoltenVK/macOS requires VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR\n"
            "      // when VK_KHR_portability_enumeration is enabled, otherwise\n"
            "      // vkEnumeratePhysicalDevices returns VK_ERROR_INCOMPATIBLE_DRIVER.\n"
            "      VkInstanceCreateFlags createFlags = 0;\n"
            "      if (m_extensionSet.supports(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))\n"
            "        createFlags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;\n"
            "\n"
            "      VkInstanceCreateInfo info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };\n"
            "      info.flags                    = createFlags;\n"
            "      info.pApplicationInfo         = &appInfo;\n"
            "      info.enabledLayerCount        = layerList.count();\n"
            "      info.ppEnabledLayerNames      = layerList.names();\n"
            "      info.enabledExtensionCount    = m_extensionNames.count();\n"
            "      info.ppEnabledExtensionNames  = m_extensionNames.names();\n"
            "\n"
            "      VkResult status = m_vkl->vkCreateInstance(&info, nullptr, &instance);"
        )
        return c.replace(old2, new2)

    patch_file(
        os.path.join(src, "src/dxvk/dxvk_instance.cpp"),
        patch_instance_cpp,
        "DxvkInstance: add portability_enumeration extension + flag for MoltenVK"
    )

    # Patch 9: dxvk_adapter.cpp
    # vkCreateDevice fails with VK_ERROR_FEATURE_NOT_PRESENT on MoltenVK because:
    #   (a) VK_KHR_portability_subset must be enabled when the device supports it and
    #       VkPhysicalDevicePortabilitySubsetFeaturesKHR must be in the pNext chain.
    #   (b) core VkPhysicalDeviceFeatures (tessellationShader, shaderFloat64, etc.)
    #       must be masked against actual device capabilities before vkCreateDevice.
    # VK_ENABLE_BETA_EXTENSIONS must be defined before including vulkan_core.h to
    # expose the portability subset sType enum and struct (they are in vulkan_beta.h).
    def patch_adapter_portability_subset(c):
        # Sub-patch 9a: define VK_ENABLE_BETA_EXTENSIONS + include vulkan_beta.h
        old_top = (
            '#include <cstring>\n'
            '#include <unordered_set>\n'
            '\n'
            '#include "dxvk_adapter.h"\n'
            '#include "dxvk_device.h"\n'
            '#include "dxvk_instance.h"'
        )
        new_top = (
            '// macOS: Enable Vulkan beta extensions to expose VK_KHR_portability_subset types\n'
            '// (VkPhysicalDevicePortabilitySubsetFeaturesKHR, VK_STRUCTURE_TYPE_*_PORTABILITY_SUBSET_*).\n'
            '// These are guarded by VK_ENABLE_BETA_EXTENSIONS in vulkan_core.h and vulkan_beta.h.\n'
            '// This define MUST appear before any Vulkan header is pulled in.\n'
            '#ifdef __APPLE__\n'
            '#define VK_ENABLE_BETA_EXTENSIONS\n'
            '#endif\n'
            '\n'
            '#include <cstring>\n'
            '#include <unordered_set>\n'
            '\n'
            '#include "dxvk_adapter.h"\n'
            '#include "dxvk_device.h"\n'
            '#include "dxvk_instance.h"\n'
            '\n'
            '// macOS: VkPhysicalDevicePortabilitySubsetFeaturesKHR lives in vulkan_beta.h.\n'
            '// Included after dxvk headers (which pull in vulkan.h) but VK_ENABLE_BETA_EXTENSIONS\n'
            '// defined above ensures the portability types are active in vulkan_core.h too.\n'
            '#ifdef __APPLE__\n'
            '#include <vulkan/vulkan_beta.h>\n'
            '#endif'
        )
        c = c.replace(old_top, new_top)

        # Sub-patch 9b: enable VK_KHR_portability_subset extension + mask core features
        old_ext = (
            '    // Enable additional extensions if necessary\n'
            '    extensionsEnabled.merge(m_extraExtensions);\n'
            '    DxvkNameList extensionNameList = extensionsEnabled.toNameList();\n'
            '\n'
            '    // Always enable robust buffer access\n'
            '    enabledFeatures.core.features.robustBufferAccess = VK_TRUE;'
        )
        new_ext = (
            '    // Enable additional extensions if necessary\n'
            '    extensionsEnabled.merge(m_extraExtensions);\n'
            '\n'
            '    // macOS/MoltenVK: VK_KHR_portability_subset MUST be enabled when the device supports it.\n'
            '    // Without it, vkCreateDevice rejects valid features (robustBufferAccess2, nullDescriptor, etc.)\n'
            '    // with VK_ERROR_FEATURE_NOT_PRESENT due to MoltenVK\'s portability subset enforcement.\n'
            '    const bool hasPortabilitySubset =\n'
            '      m_deviceExtensions.supports(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) != 0u;\n'
            '    if (hasPortabilitySubset)\n'
            '      extensionsEnabled.add(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);\n'
            '\n'
            '    DxvkNameList extensionNameList = extensionsEnabled.toNameList();\n'
            '\n'
            '    // macOS/MoltenVK: mask all core features against actual device capabilities.\n'
            '    // Prevents requesting tessellationShader, shaderFloat64, etc. that M1 reports as 0,\n'
            '    // which MoltenVK rejects with VK_ERROR_FEATURE_NOT_PRESENT in vkCreateDevice.\n'
            '    {\n'
            '      const VkBool32* src = reinterpret_cast<const VkBool32*>(&m_deviceFeatures.core.features);\n'
            '      VkBool32*       dst = reinterpret_cast<VkBool32*>(&enabledFeatures.core.features);\n'
            '      const size_t  count = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);\n'
            '      for (size_t i = 0; i < count; ++i)\n'
            '        dst[i] = dst[i] & src[i];\n'
            '    }\n'
            '\n'
            '    // Always enable robust buffer access\n'
            '    enabledFeatures.core.features.robustBufferAccess = VK_TRUE;'
        )
        c = c.replace(old_ext, new_ext)

        # Sub-patch 9c: inject VkPhysicalDevicePortabilitySubsetFeaturesKHR into pNext chain
        old_chain = (
            '    // Create pNext chain for additional device features\n'
            '    initFeatureChain(enabledFeatures, devExtensions, instance->extensions());\n'
            '\n'
            '    // Log feature support info an extension list'
        )
        new_chain = (
            '    // Create pNext chain for additional device features\n'
            '    initFeatureChain(enabledFeatures, devExtensions, instance->extensions());\n'
            '\n'
            '    // macOS/MoltenVK: when VK_KHR_portability_subset is enabled, the Vulkan spec requires\n'
            '    // VkPhysicalDevicePortabilitySubsetFeaturesKHR in VkDeviceCreateInfo.pNext set to\n'
            '    // device-queried values. Without this, MoltenVK may reject features it reported as\n'
            '    // supported in vkGetPhysicalDeviceFeatures2 (e.g. robustBufferAccess2, nullDescriptor).\n'
            '    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilityFeatures = {\n'
            '      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR };\n'
            '    if (hasPortabilitySubset) {\n'
            '      VkPhysicalDeviceFeatures2 query = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };\n'
            '      query.pNext = &portabilityFeatures;\n'
            '      m_vki->vkGetPhysicalDeviceFeatures2(m_handle, &query);\n'
            '      portabilityFeatures.pNext = std::exchange(enabledFeatures.core.pNext, &portabilityFeatures);\n'
            '    }\n'
            '\n'
            '    // Log feature support info an extension list'
        )
        return c.replace(old_chain, new_chain)

    patch_file(
        os.path.join(src, "src/dxvk/dxvk_adapter.cpp"),
        patch_adapter_portability_subset,
        "dxvk_adapter: portability_subset extension + feature masking for MoltenVK"
    )

    # Patch 10: dxvk_adapter.cpp
    # MoltenVK bug: vkGetPhysicalDeviceFeatures2 reports robustBufferAccess2 and
    # nullDescriptor as supported (returns 1), but vkCreateDevice rejects them with
    # VK_ERROR_FEATURE_NOT_PRESENT when VK_KHR_portability_subset is active.
    # Fix: on macOS, do not request these two robustness2 features.
    def patch_adapter_robustness2(c):
        old = (
            '    // Require robustBufferAccess2 since we use the robustness alignment\n'
            '    // info in a number of places, and require null descriptor support\n'
            '    // since we no longer have a fallback for those in the backend\n'
            '    enabledFeatures.extRobustness2.robustBufferAccess2 = VK_TRUE;\n'
            '    enabledFeatures.extRobustness2.robustImageAccess2 = m_deviceFeatures.extRobustness2.robustImageAccess2;\n'
            '    enabledFeatures.extRobustness2.nullDescriptor = VK_TRUE;'
        )
        new = (
            '    // Require robustBufferAccess2 since we use the robustness alignment\n'
            '    // info in a number of places, and require null descriptor support\n'
            '    // since we no longer have a fallback for those in the backend\n'
            '    // macOS/MoltenVK: despite reporting these features as supported via\n'
            '    // vkGetPhysicalDeviceFeatures2, vkCreateDevice rejects them with\n'
            '    // VK_ERROR_FEATURE_NOT_PRESENT when VK_KHR_portability_subset is active.\n'
            '#ifdef __APPLE__\n'
            '    enabledFeatures.extRobustness2.robustBufferAccess2 = VK_FALSE;\n'
            '    enabledFeatures.extRobustness2.robustImageAccess2 = VK_FALSE;\n'
            '    enabledFeatures.extRobustness2.nullDescriptor = VK_FALSE;\n'
            '#else\n'
            '    enabledFeatures.extRobustness2.robustBufferAccess2 = VK_TRUE;\n'
            '    enabledFeatures.extRobustness2.robustImageAccess2 = m_deviceFeatures.extRobustness2.robustImageAccess2;\n'
            '    enabledFeatures.extRobustness2.nullDescriptor = VK_TRUE;\n'
            '#endif'
        )
        return c.replace(old, new)

    patch_file(
        os.path.join(src, "src/dxvk/dxvk_adapter.cpp"),
        patch_adapter_robustness2,
        "dxvk_adapter: do not request robustness2 features on macOS (MoltenVK portability_subset bug)"
    )

    # Patch 11: d3d8_batch.h
    # D3D8Batcher::StateChange() restores D3D9 stream state after flushing batched
    # draws via DrawIndexedPrimitiveUP by calling:
    #   m_device->SetStreamSource(0, D3D8VertexBuffer::GetD3D9Nullable(m_stream), 0, stride)
    #   m_device->SetIndices(D3D8IndexBuffer::GetD3D9Nullable(m_indices))
    # However m_stream is always a D3D8BatchBuffer constructed with nullptr as its D3D9
    # backing buffer, so GetD3D9Nullable always returns null. This call:
    #   1. Is a no-op (DrawIndexedPrimitiveUP already unbinds stream 0 + index buffer per D3D9 spec)
    #   2. CORRUPTS state when StateChange() is triggered from D3D8Device::SetStreamSource:
    #      D3D8Device sets D3D9 stream 0 to a real buffer first, THEN calls batcher->SetStream
    #      which calls StateChange(), which nullifies the just-set D3D9 stream 0.
    #      The following DrawIndexedPrimitive then finds stream 0 = null → null VkBuffer
    #      → CRASH in MVKBuffer::getMTLBuffer() (ldr x0, [x0, #0x58] with x0=null).
    # Fix: remove the post-draw SetStreamSource and SetIndices restore calls in StateChange().
    def patch_batcher_state_restore(c):
        old = (
            '        m_device->SetStreamSource(0, D3D8VertexBuffer::GetD3D9Nullable(m_stream), 0, m_stride);\n'
            '        m_device->SetIndices(D3D8IndexBuffer::GetD3D9Nullable(m_indices));\n'
        )
        new = (
            '        // GeneralsX Patch 11: do NOT restore D3D9 state here.\n'
            '        // m_stream is a CPU-only D3D8BatchBuffer with null D3D9 backing;\n'
            '        // GetD3D9Nullable returns null and would overwrite the real D3D9\n'
            '        // stream source already set by D3D8Device::SetStreamSource before\n'
            '        // triggering this StateChange(). DrawIndexedPrimitiveUP has already\n'
            '        // unbound stream 0 and indices per D3D9 spec. The D3D8Device layer\n'
            '        // is responsible for re-setting correct state before the next draw.\n'
        )
        return c.replace(old, new)

    patch_file(
        os.path.join(src, "src/d3d8/d3d8_batch.h"),
        patch_batcher_state_restore,
        "d3d8_batch: remove post-flush SetStreamSource/SetIndices null-overwrite (null VkBuffer crash)"
    )

    # Patch 12: dxvk_context.cpp - updateVertexBufferBindings passes VK_NULL_HANDLE to vkCmdBindVertexBuffers2
    # When a vertex buffer slot has no buffer bound (length == 0), DXVK sets:
    #   buffers[i] = VK_NULL_HANDLE
    # and then calls cmdBindVertexBuffers with that null handle.
    # Per Vulkan spec, passing VK_NULL_HANDLE requires nullDescriptor feature (VK_EXT_robustness2).
    # MoltenVK does NOT handle VK_NULL_HANDLE gracefully: it casts the handle directly to MVKBuffer*,
    # resulting in a null pointer dereference in MVKBuffer::getMTLBuffer() (crash: ldr x0, [x0, #0x58]).
    # DXVK already uses dummyResources().bufferHandle() for xfb null slots (same file, nearby).
    # Fix: use the same dummy buffer handle for unbound vertex buffer slots.
    def patch_null_vertex_buffer(c):
        old = (
            '      } else {\n'
            '        buffers[i] = VK_NULL_HANDLE;\n'
            '        offsets[i] = 0;\n'
            '        lengths[i] = 0;\n'
            '        strides[i] = 0;\n'
            '      }\n'
        )
        new = (
            '      } else {\n'
            '        // GeneralsX Patch 12: MoltenVK crashes on VK_NULL_HANDLE in\n'
            '        // vkCmdBindVertexBuffers2 (no nullDescriptor support). Use the\n'
            '        // DXVK dummy buffer instead — same pattern as xfb null slots.\n'
            '        buffers[i] = m_common->dummyResources().bufferHandle();\n'
            '        offsets[i] = 0;\n'
            '        lengths[i] = 0;\n'
            '        strides[i] = 0;\n'
            '      }\n'
        )
        return c.replace(old, new)

    patch_file(
        os.path.join(src, "src/dxvk/dxvk_context.cpp"),
        patch_null_vertex_buffer,
        "dxvk_context: use dummyResources().bufferHandle() for unbound vertex slots (null VkBuffer MoltenVK crash)"
    )

    # Patch 13: dxso_compiler.cpp - PS1.x shaders (majorVersion < 2) emit all sampler type variants
    # via spec constants in BOTH the declaration AND the sample instruction paths.
    # SPIRV-Cross (MoltenVK) generates ps_main() with all type variants as params for every OpSwitch
    # that uses SpecSamplerType. Metal entry-point wrapper only initializes 2D descriptors, leaving
    # shadow/3D/cube variants undeclared → "use of undeclared identifier 's0_cubeSmplr'" etc.
    #
    # Fix TWO locations in dxso_compiler.cpp:
    #   1. emitDclSampler (~line 754): `implicit` declaration — removes auto-trigger for PS1.x
    #   2. emitTextureSample (~line 3015): sample instruction path — removes PS1.x spec-const OpSwitch
    # With forceSamplerTypeSpecConstants=False (default), both paths use the detected 2D type directly.
    def patch_ps1x_sampler_implicit(c):
        # Part A: fix declaration path (emitDclSampler)
        old_decl = (
            '    const bool implicit = m_programInfo.majorVersion() < 2 || m_moduleInfo.options.forceSamplerTypeSpecConstants;\n'
        )
        new_decl = (
            '    // GeneralsX Patch 13: MoltenVK/SPIRV-Cross does not declare dummy variables for inactive\n'
            '    // sampler type variants in the MSL entry-point wrapper. Removing the majorVersion() < 2\n'
            '    // auto-trigger prevents DXVK from emitting all-type SPIR-V for PS1.x shaders. Only\n'
            '    // forceSamplerTypeSpecConstants=True (explicit opt-in) enables multi-type mode.\n'
            '    const bool implicit = m_moduleInfo.options.forceSamplerTypeSpecConstants;\n'
        )
        # Part B: fix sample instruction path (emitTextureSample)
        old_sample = (
            '    if (m_programInfo.majorVersion() >= 2 && !m_moduleInfo.options.forceSamplerTypeSpecConstants) {\n'
            '      DxsoSamplerType samplerType =\n'
            '        SamplerTypeFromTextureType(sampler.type);\n'
            '\n'
            '      SampleType(samplerType);\n'
            '    }\n'
            '    else {'
        )
        new_sample = (
            '    // GeneralsX Patch 13b: Remove majorVersion() < 2 auto-trigger for spec-constant sampler\n'
            '    // type switching. Same rationale as Patch 13: for PS1.x + forceSamplerTypeSpecConstants=False,\n'
            '    // emit a direct 2D sample call instead of an OpSwitch over {2D, 3D, Cube} spec constants.\n'
            '    // SPIRV-Cross generates ps_main() params for every OpSwitch case label even when those\n'
            '    // samplers were not declared as SPIR-V bindings → "undeclared identifier" in Metal MSL.\n'
            '    if (!m_moduleInfo.options.forceSamplerTypeSpecConstants) {\n'
            '      DxsoSamplerType samplerType =\n'
            '        SamplerTypeFromTextureType(sampler.type);\n'
            '\n'
            '      SampleType(samplerType);\n'
            '    }\n'
            '    else {'
        )
        c = c.replace(old_decl, new_decl)
        c = c.replace(old_sample, new_sample)
        return c

    patch_file(
        os.path.join(src, "src/dxso/dxso_compiler.cpp"),
        patch_ps1x_sampler_implicit,
        "dxso_compiler: remove PS1.x majorVersion<2 sampler spec-const auto-trigger in declaration AND sample paths (MoltenVK MSL undeclared identifier fix)"
    )

    print("\nAll macOS patches applied successfully.")


if __name__ == "__main__":
    main()
