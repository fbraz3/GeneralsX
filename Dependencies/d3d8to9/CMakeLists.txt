# GeneralsX: d3d8to9 as a static library linked into generalszh.exe only (compatibility graphics: a PC with no Vulkan
# renders through Windows' own d3d9.dll). Added from GeneralsMD/Code/Main/CMakeLists.txt, which the SimID digest does
# not read. Upstream pin and patches: SOURCE.txt, patches/, scripts/test/d3d8to9-pin-test.sh.
add_library(gx_d3d8to9 STATIC
    source/d3d8to9.cpp
    source/d3d8to9_base.cpp
    source/d3d8to9_device.cpp
    source/d3d8to9_index_buffer.cpp
    source/d3d8to9_surface.cpp
    source/d3d8to9_swap_chain.cpp
    source/d3d8to9_texture.cpp
    source/d3d8to9_vertex_buffer.cpp
    source/d3d8to9_volume.cpp
    source/d3d8types.cpp
    source/interface_query.cpp
    source/gx_shader_tokens.cpp
)
set_target_properties(gx_d3d8to9 PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_definitions(gx_d3d8to9 PRIVATE D3D8TO9NOLOG NOMINMAX)
target_include_directories(gx_d3d8to9 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# No d3d9.lib: every d3d9 entry point is GetProcAddress'd from System32\d3d9.dll (patch 0001) and the rest are COM calls.
# Linking it would put d3d9.dll in the exe's import table, and the loader would then map the DXVK d3d9.dll that sits
# beside the exe into every start, Vulkan or not.
