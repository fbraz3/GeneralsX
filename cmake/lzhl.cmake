# GeneralsX @build GitHubCopilot 13/04/2026 Allow Flatpak SDK builds to inject pre-fetched lzhl source.
if(DEFINED FETCHCONTENT_SOURCE_DIR_LZHL AND EXISTS "${FETCHCONTENT_SOURCE_DIR_LZHL}")
    set(LZHL_DIR ${FETCHCONTENT_SOURCE_DIR_LZHL})
else()
    set(LZHL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/lzhl-src/CompLibHeader)

    FetchContent_Populate(lzhl DOWNLOAD_EXTRACT_TIMESTAMP
        GIT_REPOSITORY https://github.com/TheSuperHackers/lzhl-1.0
        GIT_TAG        dfd96e2ca64adaddb35dd4ebadd6add7d5586783
        SOURCE_DIR     ${LZHL_DIR}
    )
endif()

add_library(liblzhl STATIC)

target_sources(liblzhl PRIVATE
    "${LZHL_DIR}/_huff.h"
    "${LZHL_DIR}/_lz.h"
    "${LZHL_DIR}/_lzhl.h"
    "${LZHL_DIR}/Huff.cpp"
    "${LZHL_DIR}/Lz.cpp"
    #"${LZHL_DIR}/Lzhl_tcp.cpp"
    #"${LZHL_DIR}/Lzhl_tcp.h"
    "${LZHL_DIR}/Lzhl.cpp"
    "${LZHL_DIR}/lzhl.h"
)

target_include_directories(liblzhl PUBLIC ${LZHL_DIR} ${LZHL_DIR}/..)
