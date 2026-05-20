add_library(z_wwdownload STATIC)
set_target_properties(z_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

target_link_libraries(z_wwdownload PRIVATE
    corei_wwdownload
    zi_always
    zi_gameengine_include
)

# GeneralsX @bugfix GitHub Copilot 20/05/2026 FTP implementation requires Winsock symbols on MinGW link.
if(WIN32)
    target_link_libraries(z_wwdownload PRIVATE ws2_32)
endif()
