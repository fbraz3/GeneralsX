# GeneralsX @build BenderAI 25/02/2026 vcpkg overlay triplet for Apple Silicon
# Overrides the built-in arm64-osx triplet to pin VCPKG_OSX_DEPLOYMENT_TARGET,
# ensuring that vcpkg-installed packages (fontconfig, freetype, libpng, bz2, etc.)
# are compiled with the same macOS minimum version as the game itself (15.0).
# Without this the built-in triplet inherits the host SDK version (e.g. 26.0),
# producing hundreds of ld warnings: "was built for newer macOS version (26.0)
# than being linked (15.0)".
#
# AFTER changing this triplet, the installed vcpkg packages must be rebuilt:
#   rm -rf build/macos-vulkan/vcpkg_installed
#   cmake --preset macos-vulkan

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET "15.0")
