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

    print("\nAll macOS patches applied successfully.")


if __name__ == "__main__":
    main()
