set(WWDOWNLOAD_SRC
    Download.cpp
    Download.h
    DownloadDebug.h
    downloaddefs.h
    FTP.cpp
    ftp.h
    ftpdefs.h
    registry.cpp
    Registry.h
    urlBuilder.cpp
    urlBuilder.h
)

add_library(corei_wwdownload INTERFACE)

target_sources(corei_wwdownload INTERFACE ${WWDOWNLOAD_SRC})

target_link_libraries(corei_wwdownload INTERFACE
    core_wwcommon
    # GeneralsX @build Mr. Meeseeks 15/11/2026 Link ws2_32 only on Windows
    $<$<BOOL:${WIN32}>:ws2_32>
)
