# WW3D2 library is now unified in Core/Libraries/Source/WWVegas/WW3D2
# This CMakeLists provides a compatibility interface library for GeneralsMD (Zero Hour expansion) that re-exports the core library

add_library(z_ww3d2 INTERFACE)

# Link against the unified core WW3D2 library
target_link_libraries(z_ww3d2 INTERFACE
    corei_ww3d2
    z_wwcommon
)
