# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

# profiling library
#add_subdirectory(Source/profile) # Phase 50: Windows-only: _int64, __forceinline not available on macOS

# debugging library
#add_subdirectory(Source/debug) # Phase 50: Windows-only: __int64, windows.h dependency

add_subdirectory(Source/EABrowserDispatch)
add_subdirectory(Source/EABrowserEngine)
add_subdirectory(Source/Compression)
