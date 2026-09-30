# Include the default x64-windows triplet for host dependencies
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Exclude compiler version from ABI hash so that minor MSVC updates on GitHub runners
# do not invalidate the binary cache for host tools (e.g. protoc, host abseil).
set(VCPKG_DISABLE_COMPILER_TRACKING ON)
