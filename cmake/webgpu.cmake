# GeneralsX @feature Codex 04/08/2026 Define the shared Emscripten WebGPU contract.
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "WebGPU browser builds require Emscripten")
endif()

# GeneralsX @build Codex 04/08/2026 Keep the browser runtime single-threaded until game work is explicitly moved to workers.
add_compile_options(-fexceptions)

add_library(webgpu_runtime INTERFACE)
target_compile_definitions(webgpu_runtime INTERFACE SAGE_USE_WEBGPU)
target_compile_options(webgpu_runtime INTERFACE
    --use-port=emdawnwebgpu
    -fexceptions
)
target_link_options(webgpu_runtime INTERFACE
    --use-port=emdawnwebgpu
    -fexceptions
	# GeneralsX @feature Codex 04/08/2026 Suspend synchronous legacy initialization while WebGPU requests resolve.
	"-sASYNCIFY=1"
    # GeneralsX @port Codex 04/08/2026 Match the native process stack required by legacy global-data initialization.
    "-sSTACK_SIZE=1048576"
    # GeneralsX @build Codex 05/08/2026 Fixed 2 GiB heap. Do NOT raise: growth detaches emdawnwebgpu heap views
    # (black canvas), resizable buffers break TextDecoder, and >2GiB heaps hit signed-pointer casts all over this
    # 2003 codebase (also black canvas). More memory = MEMORY64 rebuild + pointer audit, tracked separately.
    "-sINITIAL_MEMORY=2147483648"
    # GeneralsX @feature Codex 05/08/2026 IDBFS keeps the user data dir (Options.ini, saves) across page loads.
    "-lidbfs.js"
)
