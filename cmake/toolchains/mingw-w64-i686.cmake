# TheSuperHackers @build JohnsterID 05/01/2026 Add MinGW-w64 i686 cross-compilation toolchain
# MinGW-w64 32-bit (i686) Toolchain File
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-i686.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# GeneralsX @bugfix GitHub Copilot 21/05/2026 Resolve MinGW tools to absolute paths so native Windows builds can find ar/ranlib/windres and GCC preprocessors.
if(DEFINED ENV{MSYSTEM_PREFIX} AND NOT "$ENV{MSYSTEM_PREFIX}" STREQUAL "")
	set(_MINGW_I686_BIN_HINTS "$ENV{MSYSTEM_PREFIX}/bin")
else()
	set(_MINGW_I686_BIN_HINTS
		"C:/msys64/mingw32/bin"
		"C:/msys64/ucrt64/bin"
	)
endif()

find_program(MINGW32_GCC NAMES i686-w64-mingw32-gcc gcc HINTS ${_MINGW_I686_BIN_HINTS})
find_program(MINGW32_GXX NAMES i686-w64-mingw32-g++ g++ HINTS ${_MINGW_I686_BIN_HINTS})
find_program(MINGW32_WINDRES NAMES i686-w64-mingw32-windres windres HINTS ${_MINGW_I686_BIN_HINTS})
find_program(MINGW32_AR NAMES i686-w64-mingw32-ar ar HINTS ${_MINGW_I686_BIN_HINTS})
find_program(MINGW32_RANLIB NAMES i686-w64-mingw32-ranlib ranlib HINTS ${_MINGW_I686_BIN_HINTS})
find_program(MINGW32_DLLTOOL NAMES i686-w64-mingw32-dlltool dlltool HINTS ${_MINGW_I686_BIN_HINTS})

if(NOT MINGW32_GCC)
	set(MINGW32_GCC i686-w64-mingw32-gcc)
endif()
if(NOT MINGW32_GXX)
	set(MINGW32_GXX i686-w64-mingw32-g++)
endif()
if(NOT MINGW32_WINDRES)
	set(MINGW32_WINDRES i686-w64-mingw32-windres)
endif()
if(NOT MINGW32_AR)
	set(MINGW32_AR i686-w64-mingw32-ar)
endif()
if(NOT MINGW32_RANLIB)
	set(MINGW32_RANLIB i686-w64-mingw32-ranlib)
endif()
if(NOT MINGW32_DLLTOOL)
	set(MINGW32_DLLTOOL i686-w64-mingw32-dlltool)
endif()

set(CMAKE_C_COMPILER ${MINGW32_GCC})
set(CMAKE_CXX_COMPILER ${MINGW32_GXX})
set(CMAKE_RC_COMPILER ${MINGW32_WINDRES})
set(CMAKE_AR ${MINGW32_AR})
set(CMAKE_RANLIB ${MINGW32_RANLIB})
set(CMAKE_DLLTOOL ${MINGW32_DLLTOOL})
set(CMAKE_C_COMPILER_AR ${MINGW32_AR})
set(CMAKE_CXX_COMPILER_AR ${MINGW32_AR})

if(MINGW32_GCC)
	string(APPEND CMAKE_RC_FLAGS_INIT " --preprocessor=${MINGW32_GCC}")
endif()

# Target environment
if(DEFINED ENV{MSYSTEM_PREFIX} AND NOT "$ENV{MSYSTEM_PREFIX}" STREQUAL "")
	set(CMAKE_FIND_ROOT_PATH "$ENV{MSYSTEM_PREFIX}")
else()
	set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)
endif()

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Force 32-bit pointer size
set(CMAKE_SIZEOF_VOID_P 4)

# Disable MFC-dependent tools (not compatible with MinGW-w64)
set(RTS_BUILD_CORE_TOOLS OFF CACHE BOOL "Disable MFC-dependent core tools for MinGW" FORCE)
set(RTS_BUILD_GENERALS_TOOLS OFF CACHE BOOL "Disable MFC-dependent Generals tools for MinGW" FORCE)
set(RTS_BUILD_ZEROHOUR_TOOLS OFF CACHE BOOL "Disable MFC-dependent Zero Hour tools for MinGW" FORCE)
