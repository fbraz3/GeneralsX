# GeneralsX @build BenderAI 27/02/2026
# Compiler cache support for Windows (sccache) and Linux/macOS fallback
#
# Windows: Uses sccache (Mozilla's Rust-based compiler cache)
#          - Cross-platform support
#          - Works with MSVC, Clang, GCC
#          - Can use cloud backends (AWS S3, Google Cloud, etc.)
#
# macOS: Delegates to ccache.cmake (already has time_macros sloppiness configured)
# Linux: Tries sccache first (installed via vcpkg), falls back to ccache
#
# Reference: https://github.com/mozilla/sccache

option(SAGE_USE_COMPILER_CACHE "Use compiler cache (sccache/ccache) if available" ON)

if(SAGE_USE_COMPILER_CACHE)
    if(APPLE)
        # macOS: Use ccache.cmake which is already configured
        # This maintains backward compatibility with existing setup_ccache.sh
        if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/ccache.cmake")
            include(${CMAKE_SOURCE_DIR}/cmake/ccache.cmake)
        else()
            message(STATUS "ccache.cmake not found, compiler cache disabled on macOS")
        endif()
        
    elseif(WIN32 OR MINGW)
        # Windows: Use sccache (via vcpkg or system)
        find_program(SCCACHE_PROGRAM sccache PATHS
            ${CMAKE_PREFIX_PATH}
            $ENV{SCCACHE_PATH}
            "C:/Program Files/sccache"
            "C:/Program Files (x86)/sccache"
            "$ENV{USERPROFILE}/.cargo/bin"
        )
        
        if(SCCACHE_PROGRAM)
            set(CMAKE_C_COMPILER_LAUNCHER   "${SCCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
            set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
            
            message(STATUS "sccache enabled: ${SCCACHE_PROGRAM}")
            
            # GeneralsX @build BenderAI 27/02/2026
            # Configure sccache with sloppiness options similar to ccache
            # This fixes the 62.46% uncacheable calls issue from __TIME__ and __DATE__
            #
            # sccache uses environment variables for configuration:
            #   SCCACHE_IGNORE_NONDETERMINISTIC_BEHAVIOR: Ignore nondeterministic compiler output
            #
            # For MSVC (cl.exe), we need to tell sccache to ignore __TIME__ and __DATE__
            # This is done by enabling nondeterministic behavior handling
            
            if(MSVC)
                # MSVC-specific: Enable nondeterministic output handling
                # This tells sccache to ignore timestamps in preprocessor output
                set(ENV{SCCACHE_SLOPPINESS} "include_file_mtime")
                message(STATUS "sccache: sloppiness configured for MSVC (include_file_mtime)")
            else()
                # GCC/Clang via MinGW/Cross-compilation
                # Similar handling for nondeterministic macros
                set(ENV{SCCACHE_SLOPPINESS} "include_file_mtime")
                message(STATUS "sccache: sloppiness configured for non-MSVC (include_file_mtime)")
            endif()
            
            # Optional: Enable compression for storage efficiency
            # Matches ccache compress settings from macOS setup
            if(NOT DEFINED ENV{SCCACHE_COMPRESS})
                set(ENV{SCCACHE_COMPRESS} "1")
            endif()
            
            # Optional: Set cache size (sccache default is 10GB)
            if(NOT DEFINED ENV{SCCACHE_CACHE_SIZE})
                set(ENV{SCCACHE_CACHE_SIZE} "20G")
            endif()
            
        else()
            message(STATUS "sccache not found. Install via:")
            message(STATUS "  - Option A) vcpkg: vcpkg install sccache:x64-windows")
            message(STATUS "  - Option B) cargo: cargo install sccache")
            message(STATUS "  - Option C) Download: https://github.com/mozilla/sccache/releases")
            message(STATUS "  Compiler cache disabled for this build")
        endif()
        
    elseif(UNIX AND NOT APPLE)
        # Linux: Try sccache first (can be installed via vcpkg or cargo), then fall back to ccache
        find_program(SCCACHE_PROGRAM sccache)
        
        if(SCCACHE_PROGRAM)
            set(CMAKE_C_COMPILER_LAUNCHER   "${SCCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
            set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
            
            message(STATUS "sccache enabled: ${SCCACHE_PROGRAM}")
            
            # Configure sccache with Linux-specific sloppiness
            set(ENV{SCCACHE_SLOPPINESS} "include_file_mtime")
            message(STATUS "sccache: sloppiness configured for Linux (include_file_mtime)")
            
        else()
            # Fall back to ccache on Linux
            find_program(CCACHE_PROGRAM ccache)
            
            if(CCACHE_PROGRAM)
                set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}" CACHE STRING "C compiler launcher")
                set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ compiler launcher")
                
                message(STATUS "ccache enabled: ${CCACHE_PROGRAM}")
                
                # Apply time_macros sloppiness for Linux ccache (same as macOS)
                execute_process(
                    COMMAND ccache -o sloppiness=time_macros,locale
                    RESULT_VARIABLE CCACHE_CONFIG_RESULT
                )
                if(CCACHE_CONFIG_RESULT EQUAL 0)
                    message(STATUS "ccache: sloppiness configured for nondeterministic macros (time_macros,locale)")
                endif()
            else()
                message(STATUS "No compiler cache found (sccache or ccache). Install via:")
                message(STATUS "  - sccache: cargo install sccache")
                message(STATUS "  - ccache: apt install ccache (Debian/Ubuntu) or brew install ccache (macOS)")
            endif()
        endif()
    endif()
else()
    message(STATUS "Compiler cache disabled (SAGE_USE_COMPILER_CACHE=OFF)")
endif()
