# VC6 Portable Toolchain for CMake
# This toolchain file configures CMake to use Visual C++ 6.0 Portable

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Set VC6 paths
set(VC6_ROOT "C:/VC6/VC6SP6" CACHE PATH "VC6 Installation Root")

# Set compiler paths
set(CMAKE_C_COMPILER "${VC6_ROOT}/VC98/BIN/cl.exe" CACHE FILEPATH "C Compiler")
set(CMAKE_CXX_COMPILER "${VC6_ROOT}/VC98/BIN/cl.exe" CACHE FILEPATH "C++ Compiler")
set(CMAKE_RC_COMPILER "${VC6_ROOT}/Common/MSDev98/Bin/rc.exe" CACHE FILEPATH "Resource Compiler")
set(CMAKE_LINKER "${VC6_ROOT}/VC98/BIN/link.exe" CACHE FILEPATH "Linker")

# Set compiler ID to MSVC for VC6
set(CMAKE_C_COMPILER_ID "MSVC" CACHE STRING "C Compiler ID")
set(CMAKE_CXX_COMPILER_ID "MSVC" CACHE STRING "CXX Compiler ID")

# VC6 MSVC version
set(MSVC_VERSION 1200 CACHE STRING "MSVC Version")

# Include directories
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${VC6_ROOT}/VC98/INCLUDE;${VC6_ROOT}/VC98/ATL/INCLUDE" CACHE STRING "C Include Directories")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${VC6_ROOT}/VC98/INCLUDE;${VC6_ROOT}/VC98/ATL/INCLUDE" CACHE STRING "CXX Include Directories")

# Library directories - Add to linker search path
link_directories("${VC6_ROOT}/VC98/LIB" "${VC6_ROOT}/VC98/MFC/LIB")

# Library directories
set(CMAKE_C_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib" CACHE STRING "C Standard Libraries")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "kernel32.lib user32.lib" CACHE STRING "CXX Standard Libraries")

# Flags
set(CMAKE_C_FLAGS_INIT "/TC /W3 /nologo" CACHE STRING "C Flags")
set(CMAKE_CXX_FLAGS_INIT "/TP /W3 /nologo" CACHE STRING "CXX Flags")

# Set library path for linker
set(CMAKE_LIBRARY_PATH "${VC6_ROOT}/VC98/LIB;${VC6_ROOT}/VC98/MFC/LIB" CACHE STRING "Library Path")
link_directories("${VC6_ROOT}/VC98/LIB" "${VC6_ROOT}/VC98/MFC/LIB")

# Mark toolchain file as processed
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
