set(WW3D2_SRC
    aabtree.cpp
    aabtree.h
    aabtreebuilder.cpp
    aabtreebuilder.h
    #agg_def.cpp
    agg_def.h
    animatedsoundmgr.cpp
    animatedsoundmgr.h
    animobj.cpp
    animobj.h
    assetmgr.cpp
    assetmgr.h
    assetstatus.cpp
    assetstatus.h
    bitmaphandler.cpp
    bitmaphandler.h
    bmp2d.cpp
    bmp2d.h
    boxrobj.cpp
    boxrobj.h
    bwrender.cpp
    bwrender.h
    camera.cpp
    camera.h
    #classid.h # unused
    collect.cpp
    collect.h
    colorspace.h
    coltest.cpp
    coltest.h
    coltype.h
    composite.cpp
    composite.h
    dazzle.cpp
    dazzle.h
    #ddsfile.cpp
    #ddsfile.h
    decalmsh.cpp
    decalmsh.h
    decalsys.cpp
    decalsys.h
    distlod.cpp
    distlod.h
    dllist.h
    #dx8caps.cpp
    #dx8caps.h
#    dx8fvf.cpp
#    dx8fvf.h
#    dx8indexbuffer.cpp
#    dx8indexbuffer.h
#    dx8list.h
#    dx8polygonrenderer.cpp
#    dx8polygonrenderer.h
#    dx8renderer.cpp
#    dx8renderer.h
#    dx8rendererdebugger.cpp
#    dx8rendererdebugger.h
#    dx8texman.cpp
#    dx8texman.h
#    dx8vertexbuffer.cpp
#    dx8vertexbuffer.h
#    dx8webbrowser.cpp
#    dx8webbrowser.h
#    dx8wrapper.cpp
#    dx8wrapper.h
    dynamesh.cpp
    dynamesh.h
    font3d.cpp
    font3d.h
    formconv.cpp
    formconv.h
    FramGrab.cpp
    framgrab.h
    hanim.cpp
    hanim.h
    hanimmgr.cpp
    hanimmgr.h
    hcanim.cpp
    hcanim.h
    #hlod.cpp
    #hlod.h
    hmdldef.cpp
    hmdldef.h
    hmorphanim.cpp
    hmorphanim.h
    hrawanim.cpp
    hrawanim.h
    htree.cpp
    htree.h
    htreemgr.cpp
    htreemgr.h
    intersec.cpp
    intersec.h
    intersec.inl
    inttest.h
    layer.cpp
    layer.h
    light.cpp
    light.h
    #lightenvironment.cpp
    #lightenvironment.h
    line3d.cpp
    line3d.h
    linegrp.cpp
    linegrp.h
    mapper.cpp
    mapper.h
    matinfo.cpp
    matinfo.h
    matpass.cpp
    matpass.h
    matrixmapper.cpp
    matrixmapper.h
    mesh.cpp
    mesh.h
    meshbuild.cpp
    meshbuild.h
    meshdam.cpp
    meshdam.h
    meshgeometry.cpp
    meshgeometry.h
    meshmatdesc.cpp
    meshmatdesc.h
    meshmdl.cpp
    meshmdl.h
    meshmdlio.cpp
    metalmap.cpp
    metalmap.h
    missingtexture.cpp
    missingtexture.h
    motchan.cpp
    motchan.h
    nullrobj.cpp
    nullrobj.h
    part_buf.cpp
    part_buf.h
    part_emt.cpp
    part_emt.h
    part_ldr.cpp
    part_ldr.h
    pivot.cpp
    pivot.h
    pointgr.cpp
    pointgr.h
    polyinfo.cpp
    polyinfo.h
    predlod.cpp
    predlod.h
    prim_anim.cpp
    prim_anim.h
    projector.cpp
    projector.h
    proto.cpp
    proto.h
    proxy.h
    rddesc.h
    #render2d.cpp
    render2d.h
    render2dsentence_stub.cpp  # Cross-platform stub for FontCharsClass and Render2DSentenceClass
    render2dsentence.h         # API header (works on all platforms)
    renderobjectrecycler.cpp
    renderobjectrecycler.h
    rendobj.cpp
    rendobj.h
    rinfo.cpp
    rinfo.h
    ringobj.cpp
    ringobj.h
    robjlist.h
    scene.cpp
    scene.h
    segline.cpp
    segline.h
    seglinerenderer.cpp
    seglinerenderer.h
    #shader.cpp
    shader.h
    shattersystem.cpp
    shattersystem.h
    #shdlib.h
    snappts.cpp
    snapPts.h
    #sortingrenderer.cpp
    sortingrenderer.h
    soundlibrarybridge.h
    soundrobj.cpp
    soundrobj.h
    sphereobj.cpp
    sphereobj.h
    static_sort_list.cpp
    static_sort_list.h
    statistics.cpp
    statistics.h
    streak.cpp
    streak.h
    streakRender.cpp
    streakRender.h
    stripoptimizer.cpp
    stripoptimizer.h
#    surfaceclass.cpp
    surfaceclass.h
#    texproject.cpp
    texproject.h
    #textdraw.cpp # unused
    textdraw.h
#    texture.cpp
    texture.h
#    texturefilter.cpp
    texturefilter.h
#    textureloader.cpp
    textureloader.h
#    texturethumbnail.cpp
    texturethumbnail.h
    vertmaterial.cpp
    vertmaterial.h
    visrasterizer.cpp
    visrasterizer.h
    w3d_dep.cpp
    w3d_dep.h
    #w3d_file.h
    w3d_obsolete.h
    w3d_util.cpp
    w3d_util.h
    w3derr.h
    w3dexclusionlist.cpp
    w3dexclusionlist.h
    ww3d.cpp
    ww3d.h
    ww3dformat.cpp
    ww3dformat.h
    ww3dids.h
    ww3dtrig.h
    # Phase 01: DirectX 8 Compatibility Layer
    d3dx8_vulkan_fvf_compat.h
    d3d8_vulkan_graphics_compat.h
    win32_sdl_types_compat.h
    # Phase 02: SDL2 Window & Event Loop compatibility layer
    win32_sdl_api_compat.h
    #win32_sdl_api_compat.cpp  # TODO: Fix SDL2/SDL3 compatibility issues
    # Phase 04: Cross-platform Threading & Memory Layer
    win32_thread_compat.h
    win32_thread_compat.cpp
    win32_memory_compat.h
    win32_memory_compat.cpp
    # Phase 05: Registry & Configuration Compatibility Layer
    #win32_config_compat.h  # TODO: Add Windows Registry types to win32_sdl_types_compat.h
    #win32_config_compat.cpp
    # Phase 06: Complete Input System - Gamepad & Joystick Support
    win32_gamepad_compat.h
    #win32_gamepad_compat.cpp  # TODO: Fix SDL2/SDL3 compatibility issues
    # Phase 07: Event Queue Integration & Configuration Persistence
    win32_gamepad_event_integration.h
    win32_gamepad_event_integration.cpp
    #win32_gamepad_config_compat.h  # TODO: Fix Windows Registry type dependencies
    #win32_gamepad_config_compat.cpp
    # Phase 08: Advanced Input Features - In-Game Configuration UI
    #win32_gamepad_config_ui.h  # TODO: Fix Windows Registry type dependencies
    #win32_gamepad_config_ui.cpp
    # Phase 08: Advanced Input Features - Combo Detection
    win32_gamepad_combo_detection.h
    win32_gamepad_combo_detection.cpp
    # Phase 08: Advanced Input Features - Macro Recording & Playback
    win32_gamepad_macro_system.h
    win32_gamepad_macro_system.cpp
    # Phase 08: Advanced Input Features - Force Feedback & Haptics
    win32_gamepad_force_feedback.h
    win32_gamepad_force_feedback.cpp
    # Phase 07/08 Integration: Macro System Persistence
    #win32_gamepad_macro_persistence.h  # TODO: Fix Windows Registry type dependencies
    #win32_gamepad_macro_persistence.cpp
    # Phase 07/08 Integration: Force Feedback Profiles Persistence
    #win32_gamepad_ff_persistence.h  # TODO: Fix Windows Registry type dependencies
    #win32_gamepad_ff_persistence.cpp
    # Phase 09: Render Pass & Graphics Pipeline
    d3d8_vulkan_render_pass.h
    d3d8_vulkan_render_pass.cpp
    # Phase 10: Command Buffers & GPU/CPU Synchronization
    d3d8_vulkan_command_buffer.h
    d3d8_vulkan_command_buffer.cpp
    # Phase 11: Vertex & Index Buffers
    d3d8_vulkan_buffer.h
    d3d8_vulkan_buffer.cpp
    # Phase 12: Texture System
    d3d8_vulkan_texture.h
    d3d8_vulkan_texture.cpp
    # Phase 13: Sampler & Descriptor Sets
    d3d8_vulkan_descriptor.h
    d3d8_vulkan_descriptor.cpp
    # Phase 14: Shader System
    d3d8_vulkan_shader.h
    d3d8_vulkan_shader.cpp
    # Phase 15: Material System
    d3d8_vulkan_material.h
    d3d8_vulkan_material.cpp
    # Phase 16: Render Target Management
    d3d8_vulkan_rendertarget.h
    d3d8_vulkan_rendertarget.cpp
    # Phase 17: Render Loop Integration
    d3d8_vulkan_renderloop.h
    d3d8_vulkan_renderloop.cpp
    # Phase 18: Visibility & Culling
    d3d8_vulkan_culling.h
    d3d8_vulkan_culling.cpp
    # Phase 19: Lighting System
    d3d8_vulkan_lighting.h
    d3d8_vulkan_lighting.cpp
    # Phase 20: Viewport & Projection
    d3d8_vulkan_viewport.h
    d3d8_vulkan_viewport.cpp
    # Phase 21: Draw Command System
    d3d8_vulkan_drawing.h
    d3d8_vulkan_drawing.cpp
)

# NOTE: DX8 files are kept for compilation - they provide compatibility layer
# Even with Vulkan backend, DX8Wrapper and related classes are needed for linking
# and wrapping the game's DirectX API calls to Vulkan equivalents

add_library(corei_ww3d2 INTERFACE)

target_sources(corei_ww3d2 INTERFACE ${WW3D2_SRC})

if (NOT IS_VS6_BUILD AND (WIN32 OR MSVC OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows"))
    target_link_libraries(corei_ww3d2 INTERFACE
        comsuppw
    )
endif()

target_link_libraries(corei_ww3d2 INTERFACE
    core_browserengine
    sdl2_compat
)
