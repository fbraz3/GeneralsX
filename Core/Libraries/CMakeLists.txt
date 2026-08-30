# WW common libraries decended from earliest C&C games
add_subdirectory(Source/WWVegas)

# profiling library
add_subdirectory(Source/profile)

# debugging library
add_subdirectory(Source/debug)

add_subdirectory(Source/EABrowserDispatch)
add_subdirectory(Source/EABrowserEngine)
add_subdirectory(Source/Compression)

# GeneralsX @build Copilot 30/08/2026 Isolate the native libdatachannel transport in the platform layer.
if(SAGE_USE_NATIVE_WEBRTC)
    add_subdirectory(Source/Platform/NativeWebRTC)
endif()
