# WW3D2 library is now unified in Core/Libraries/Source/WWVegas/WW3D2
# This CMakeLists provides a compatibility interface library for Generals that re-exports the core library

add_library(g_ww3d2 INTERFACE)

# Link against the unified core WW3D2 library
target_link_libraries(g_ww3d2 INTERFACE
    corei_ww3d2
    g_wwcommon
)
