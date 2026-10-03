add_library(g_wwdownload STATIC)
set_target_properties(g_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_link_libraries(g_wwdownload PRIVATE
    corei_wwdownload
    gi_always
    gi_gameengine_include
)

# GeneralsX @bugfix GitHub Copilot 22/05/2026 FTP implementation requires Winsock symbols on MinGW link.
if(WIN32)
    target_link_libraries(g_wwdownload PRIVATE ws2_32)
endif()
