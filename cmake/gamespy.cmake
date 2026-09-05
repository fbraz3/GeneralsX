set(GS_OPENSSL FALSE)
set(GAMESPY_SERVER_NAME "server.cnc-online.net")

FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/TheSuperHackers/GamespySDK.git
    GIT_TAG        07e3d15c500415abc281efb74322ab6d9c857eb8
)

FetchContent_MakeAvailable(gamespy)

# The GamespySDK guards its platform-specific code on _MACOSX / _LINUX / _WIN32,
# but nothing defines _MACOSX on Apple. In Debug the gsinterface target adds
# GSI_COMMON_DEBUG=1, which compiles gsDebugVaListPrint in gsdebug.c; without
# _MACOSX that falls through to the dead #else branch, which calls va_start in a
# function with fixed args and an undeclared gsDebugTTyPrint. Both are hard
# errors on modern clang, so Debug builds fail while Release (which does not
# define GSI_COMMON_DEBUG) succeeds. Defining _MACOSX selects the intended
# vprintf path.
if(APPLE)
    target_compile_definitions(gsinterface INTERFACE _MACOSX)
endif()
