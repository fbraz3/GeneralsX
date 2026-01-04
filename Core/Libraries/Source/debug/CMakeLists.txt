set(DEBUG_SRC
    "debug_cmd.cpp"
    "debug_cmd.h"
    #"debug_debug.cpp"  # Phase 39.4: Windows-specific debugging, disabled for cross-platform
    "debug_debug.h"
    "debug_doc.h"
    #"debug_except.cpp"  # Phase 39.4: Windows API, disabled for cross-platform
    #"debug_getdefaultcommands.cpp"  # Phase 39.4: Windows API, disabled for cross-platform
    "debug_internal.cpp"
    #"debug_io_con.cpp"  # Phase 39.4: Windows console, disabled for cross-platform
    #"debug_io_flat.cpp"  # Phase 39.4: Windows file I/O, disabled for cross-platform
    #"debug_io_net.cpp"  # Phase 39.4: Windows socket, disabled for cross-platform
    #"debug_io_ods.cpp"  # Phase 39.4: Windows ODS, disabled for cross-platform
    "debug_io.h"
    "debug_macro.h"
    "debug_stack.cpp"
    "debug_stack.h"
    "debug_renamed.h"
    "internal_except.h"
    "internal_io.h"
    "internal.h"
)

add_library(core_debug STATIC)

target_sources(core_debug PRIVATE ${DEBUG_SRC})

target_include_directories(core_debug INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(core_debug PRIVATE
    "debug_renamed.h"
    "internal.h"
    "internal_except.h"
    "internal_io.h"
    <windows.h>
)

target_link_libraries(core_debug PRIVATE
    core_config
    core_wwcommon
    corei_always
)
