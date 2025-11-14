set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheAssemblyArmada/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

FetchContent_MakeAvailable(gamespy)

# Create patch header for macOS compatibility
file(WRITE "${gamespy_SOURCE_DIR}/include/gamespy/genx_macos_patch.h" "#ifndef _GENX_MACOS_PATCH_H_
#define _GENX_MACOS_PATCH_H_

#ifdef __APPLE__
extern char* _strlwr(char* string);
extern char* _strupr(char* string);
#endif

#endif
")

# Always apply patches - use file(STRINGS) to read and modify
if(APPLE)
    # Patch 1: Remove _strlwr/_strupr forward declarations from gsplatform.h
    file(READ "${gamespy_SOURCE_DIR}/include/gamespy/gsplatform.h" GS_PLATFORM_CONTENT)
    string(REPLACE "#if !defined(_WIN32)
char* _strlwr(char* string);
char* _strupr(char* string);
#endif" "" GS_PLATFORM_CONTENT "${GS_PLATFORM_CONTENT}")
    file(WRITE "${gamespy_SOURCE_DIR}/include/gamespy/gsplatform.h" "${GS_PLATFORM_CONTENT}")
    
    # Patch 2: Add patch header include to gscommon.h (force it every time)
    file(READ "${gamespy_SOURCE_DIR}/include/gamespy/gscommon.h" GS_COMMON_CONTENT)
    # Remove old include if exists to avoid duplicates
    string(REPLACE "#include \"genx_macos_patch.h\"" "" GS_COMMON_CONTENT "${GS_COMMON_CONTENT}")
    # Add the include before the final endif
    string(REPLACE "#endif // __GSCOMMON_H__" "#include \"genx_macos_patch.h\"

#endif // __GSCOMMON_H__" GS_COMMON_CONTENT "${GS_COMMON_CONTENT}")
    file(WRITE "${gamespy_SOURCE_DIR}/include/gamespy/gscommon.h" "${GS_COMMON_CONTENT}")
    
    message(STATUS "Applied GameSpy macOS patches")
endif()
