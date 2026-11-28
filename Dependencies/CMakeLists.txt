add_subdirectory(Precompiled)

add_library(depsi_always INTERFACE)
target_link_libraries(depsi_always INTERFACE
    deps_precompiled # Gives force included headers. Link early.
    deps_config
)

if (IS_VS6_BUILD)
    # The original max sdk does not compile against a modern compiler.
    # If there is a desire to make this work, then a fixed max sdk needs to be created.
    add_subdirectory(MaxSDK)
endif()

add_subdirectory(Utility)
add_subdirectory(Bink)
add_subdirectory(Miles)

if (WIN32)
    add_subdirectory(DbgHelp)
    add_subdirectory(Usp10)
    # GeneralsX @build Copilot 25/11/2026 Static d3d8to9 compatibility fallback for Windows x64
    if (MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_subdirectory(d3d8to9)
    endif()
else()
    add_library(deps_dbghelp INTERFACE)
    target_include_directories(deps_dbghelp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/DbgHelp)
    target_sources(deps_dbghelp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/DbgHelp/DbgHelpGuard.cpp)
    add_library(deps_usp10 INTERFACE)
    target_include_directories(deps_usp10 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Usp10)
endif()
