# GeneralsX @build BenderAI 03/03/2026 - z_wwdownload: explicit static library compiling WWDownload sources.
# Cannot be an empty STATIC lib (vcpkg/MSVC rejects it) and a pure INTERFACE chain causes
# the linker to search for corei_wwdownload.lib (which doesn't exist for INTERFACE libs).
# Solution: compile WWDownload sources explicitly using absolute paths to Core.
add_library(z_wwdownload STATIC)
set_target_properties(z_wwdownload PROPERTIES OUTPUT_NAME wwdownload)

# Use Core WWDownload sources directly (mirrors WWDOWNLOAD_SRC in Core's CMakeLists)
target_sources(z_wwdownload PRIVATE
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDownload/Download.cpp"
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDownload/FTP.cpp"
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDownload/registry.cpp"
    "${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDownload/urlBuilder.cpp"
)

target_link_libraries(z_wwdownload PRIVATE
    zi_always
    zi_gameengine_include
    core_wwcommon
)

