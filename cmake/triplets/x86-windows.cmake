# Include the default x86-windows triplet
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

# Exclude compiler version from ABI hash so that minor MSVC updates on GitHub runners
# do not invalidate the binary cache.
set(VCPKG_DISABLE_COMPILER_TRACKING ON)
