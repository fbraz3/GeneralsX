# GeneralsX @build GitHub Copilot 18/05/2026 Add MinGW-w64 x86_64 Windows toolchain for the modern Windows64 path.
# MinGW-w64 64-bit (x86_64) Toolchain File
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# GeneralsX @bugfix GitHub Copilot 21/05/2026 Resolve MinGW tools to absolute paths so native Windows builds can find ar/ranlib/windres and GCC preprocessors.
if(DEFINED ENV{MSYSTEM_PREFIX} AND NOT "$ENV{MSYSTEM_PREFIX}" STREQUAL "")
	set(_MINGW_X64_BIN_HINTS "$ENV{MSYSTEM_PREFIX}/bin")
else()
	set(_MINGW_X64_BIN_HINTS
		"C:/msys64/mingw64/bin"
		"C:/msys64/ucrt64/bin"
	)
endif()

find_program(MINGW64_GCC NAMES x86_64-w64-mingw32-gcc gcc HINTS ${_MINGW_X64_BIN_HINTS})
find_program(MINGW64_GXX NAMES x86_64-w64-mingw32-g++ g++ HINTS ${_MINGW_X64_BIN_HINTS})
find_program(MINGW64_WINDRES NAMES x86_64-w64-mingw32-windres windres HINTS ${_MINGW_X64_BIN_HINTS})
find_program(MINGW64_AR NAMES x86_64-w64-mingw32-ar ar HINTS ${_MINGW_X64_BIN_HINTS})
find_program(MINGW64_RANLIB NAMES x86_64-w64-mingw32-ranlib ranlib HINTS ${_MINGW_X64_BIN_HINTS})
find_program(MINGW64_DLLTOOL NAMES x86_64-w64-mingw32-dlltool dlltool HINTS ${_MINGW_X64_BIN_HINTS})

if(NOT MINGW64_GCC)
	set(MINGW64_GCC x86_64-w64-mingw32-gcc)
endif()
if(NOT MINGW64_GXX)
	set(MINGW64_GXX x86_64-w64-mingw32-g++)
endif()
if(NOT MINGW64_WINDRES)
	set(MINGW64_WINDRES x86_64-w64-mingw32-windres)
endif()
if(NOT MINGW64_AR)
	set(MINGW64_AR x86_64-w64-mingw32-ar)
endif()
if(NOT MINGW64_RANLIB)
	set(MINGW64_RANLIB x86_64-w64-mingw32-ranlib)
endif()
if(NOT MINGW64_DLLTOOL)
	set(MINGW64_DLLTOOL x86_64-w64-mingw32-dlltool)
endif()

set(CMAKE_C_COMPILER ${MINGW64_GCC})
set(CMAKE_CXX_COMPILER ${MINGW64_GXX})
set(CMAKE_RC_COMPILER ${MINGW64_WINDRES})
set(CMAKE_AR ${MINGW64_AR})
set(CMAKE_RANLIB ${MINGW64_RANLIB})
set(CMAKE_DLLTOOL ${MINGW64_DLLTOOL})
set(CMAKE_C_COMPILER_AR ${MINGW64_AR})
set(CMAKE_CXX_COMPILER_AR ${MINGW64_AR})

if(MINGW64_GCC)
	string(APPEND CMAKE_RC_FLAGS_INIT " --preprocessor=${MINGW64_GCC}")
endif()

# Target environment
if(DEFINED ENV{MSYSTEM_PREFIX} AND NOT "$ENV{MSYSTEM_PREFIX}" STREQUAL "")
	set(CMAKE_FIND_ROOT_PATH "$ENV{MSYSTEM_PREFIX}")
else()
	set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
endif()

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Force 64-bit pointer size
set(CMAKE_SIZEOF_VOID_P 8)

# Disable MFC-dependent tools (not compatible with the MinGW modern path)
set(RTS_BUILD_CORE_TOOLS OFF CACHE BOOL "Disable MFC-dependent core tools for MinGW" FORCE)
set(RTS_BUILD_GENERALS_TOOLS OFF CACHE BOOL "Disable MFC-dependent Generals tools for MinGW" FORCE)
set(RTS_BUILD_ZEROHOUR_TOOLS OFF CACHE BOOL "Disable MFC-dependent Zero Hour tools for MinGW" FORCE)