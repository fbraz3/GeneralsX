set(WW3D2_SRC
    aabtree.cpp
    aabtree.h
    #aabtreebuilder.cpp
    #aabtreebuilder.h
    agg_def.cpp
    agg_def.h
    animatedsoundmgr.cpp
    animatedsoundmgr.h
    #animobj.cpp
    #animobj.h
    #assetmgr.cpp
    #assetmgr.h
    assetstatus.cpp
    assetstatus.h
    bitmaphandler.cpp
    bitmaphandler.h
    #bmp2d.cpp
    #bmp2d.h
    #boxrobj.cpp
    #boxrobj.h
    bwrender.cpp
    bwrender.h
    #camera.cpp
    #camera.h
    #classid.h # unused
    collect.cpp
    collect.h
    colorspace.h
    coltest.cpp
    coltest.h
    coltype.h
    composite.cpp
    composite.h
    #dazzle.cpp
    #dazzle.h
    #ddsfile.cpp
    #ddsfile.h
    #decalmsh.cpp
    #decalmsh.h
    decalsys.cpp
    decalsys.h
    distlod.cpp
    distlod.h
    dllist.h
    #dx8caps.cpp
    #dx8caps.h
    #dx8fvf.cpp
    #dx8fvf.h
    #dx8indexbuffer.cpp
    #dx8indexbuffer.h
    dx8list.h
    dx8polygonrenderer.cpp
    dx8polygonrenderer.h
    #dx8renderer.cpp
    #dx8renderer.h
    #dx8rendererdebugger.cpp
    #dx8rendererdebugger.h
    #dx8texman.cpp
    #dx8texman.h
    #dx8vertexbuffer.cpp
    #dx8vertexbuffer.h
    dx8webbrowser.cpp
    dx8webbrowser.h
    #dx8wrapper.cpp
    #dx8wrapper.h
    ddsloader.cpp
    ddsloader.h
    tgaloader.cpp
    tgaloader.h
    texturecache.cpp
    texturecache.h
    texturedquad.cpp
    texturedquad.h
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
    #hanimmgr.cpp
    #hanimmgr.h
    hcanim.cpp
    hcanim.h
    #hlod.cpp
    #hlod.h
    hmdldef.cpp
    hmdldef.H
    #hmorphanim.cpp
    #hmorphanim.h
    #hrawanim.cpp
    #hrawanim.h
    htree.cpp
    htree.h
    #htreemgr.cpp
    #htreemgr.h
    intersec.cpp
    intersec.h
    intersec.inl
    inttest.h
    layer.cpp
    layer.h
    #light.cpp
    #light.h
    #lightenvironment.cpp
    #lightenvironment.h
    line3d.cpp
    line3d.h
    #linegrp.cpp
    #linegrp.h
    #mapper.cpp
    #mapper.h
    matinfo.cpp
    matinfo.h
    #matpass.cpp
    #matpass.h
    #matrixmapper.cpp
    #matrixmapper.h
    #mesh.cpp
    #mesh.h
    #meshbuild.cpp
    #meshbuild.h
    meshdam.cpp
    meshdam.h
    #meshgeometry.cpp
    #meshgeometry.h
    #meshmatdesc.cpp
    #meshmatdesc.h
    #meshmdl.cpp
    #meshmdl.h
    #meshmdlio.cpp
    metalmap.cpp
    metalmap.h
    missingtexture.cpp
    missingtexture.h
    #motchan.cpp
    #motchan.h
    #nullrobj.cpp
    #nullrobj.h
    #part_buf.cpp
    #part_buf.h
    #part_emt.cpp
    #part_emt.h
    #part_ldr.cpp
    #part_ldr.h
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
    #render2d.h
    render2dsentence.cpp
    render2dsentence.h
    renderobjectrecycler.cpp
    renderobjectrecycler.h
    rendobj.cpp
    rendobj.h
    #rinfo.cpp
    #rinfo.h
    ringobj.cpp
    ringobj.h
    robjlist.h
    #scene.cpp
    #scene.h
    segline.cpp
    segline.h
    seglinerenderer.cpp
    seglinerenderer.h
    #shader.cpp
    #shader.h
    shattersystem.cpp
    shattersystem.h
    #shdlib.h
    snappts.cpp
    snapPts.h
    #sortingrenderer.cpp
    #sortingrenderer.h
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
    surfaceclass.cpp
    surfaceclass.h
    #texproject.cpp
    #texproject.h
    #textdraw.cpp # unused
    textdraw.h
    #texture.cpp
    #texture.h
    #texturefilter.cpp
    #texturefilter.h
    #textureloader.cpp
    #textureloader.h
    #texturethumbnail.cpp
    #texturethumbnail.h
    #vertmaterial.cpp
    #vertmaterial.h
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
    #ww3d.cpp
    #ww3d.h
    ww3dformat.cpp
    ww3dformat.h
    ww3dids.h
    ww3dtrig.h
    
    # Cross-platform configuration system
    config_manager.cpp
    ini_parser.cpp
    
    # Cross-platform API compatibility (Phase 2)
    threading.cpp
    
    # Phase 5: Audio & Multimedia APIs (OpenAL-based DirectSound compatibility)
    dsound_compat.cpp
    multimedia_timers.cpp
    
    # Phase 38: Graphics Backend Abstraction Layer
    graphics_backend.h
    graphics_backend_legacy.h
    graphics_backend_legacy.cpp
    graphics_backend_init.cpp
    
    # Phase 39: Vulkan Backend Implementation (MoltenVK on macOS)
    graphics_backend_dxvk.h
    graphics_backend_dxvk.cpp
    graphics_backend_dxvk_device.cpp
    graphics_backend_dxvk_frame.cpp
    graphics_backend_dxvk_render.cpp
    graphics_backend_dxvk_buffers.cpp
    
    # Phase 41: Drawing Operations Implementation
    graphics_backend_dxvk_drawing.cpp
    
    # Phase 42: Texture System Implementation
    graphics_backend_dxvk_textures.cpp
    graphics_backend_dxvk_formats.cpp
    graphics_backend_dxvk_texture_cache.cpp
    
    # Phase 43: Render Loop & Frame Presentation
    graphics_backend_dxvk_render_loop.cpp
    graphics_backend_dxvk_frame_sync.cpp
    graphics_backend_dxvk_commands.cpp
    graphics_backend_dxvk_errors.cpp
    graphics_backend_dxvk_perf.cpp
)

if(APPLE)
    list(APPEND WW3D2_SRC
        metalwrapper.mm
        metalwrapper.h
    )
endif()

add_library(corei_ww3d2 INTERFACE)

target_sources(corei_ww3d2 INTERFACE ${WW3D2_SRC})

if (NOT IS_VS6_BUILD)
    target_link_libraries(corei_ww3d2 INTERFACE
        $<$<PLATFORM_ID:Windows>:comsuppw>
    )
endif()

target_link_libraries(corei_ww3d2 INTERFACE
    core_browserengine
    core_wwlib
)

# ============================================================================
# Vulkan Backend Configuration (Phase 39)
# ============================================================================
# 
# Implements Vulkan graphics backend with MoltenVK on macOS, Vulkan SDK
# on Linux, and potential DXVK integration on Windows.
#
# Reference: cmake/vulkan.cmake (Vulkan SDK discovery)
# Reference: docs/PHASE39/PHASE39_2_MOLTENVK_GUIDELINES.md (Best practices)

# Include Vulkan CMake module
include(${CMAKE_SOURCE_DIR}/cmake/vulkan.cmake)

# Link Vulkan backend (via Vulkan Loader, NOT direct MoltenVK linking)
if(VULKAN_LOADER_LIBRARY)
    target_link_libraries(corei_ww3d2 INTERFACE ${VULKAN_LOADER_LIBRARY})
    
    # Add Vulkan include directories
    target_include_directories(corei_ww3d2 INTERFACE 
        ${Vulkan_INCLUDE_DIRS}
    )
    
    if(NOT WIN32)
        # On macOS/Linux, enable KHR extensions and validation layers
        target_compile_definitions(corei_ww3d2 INTERFACE
            VK_USE_PLATFORM_METAL_EXT
            VK_ENABLE_BETA_EXTENSIONS
        )
    endif()
    
    if(APPLE)
        # macOS-specific Vulkan settings
        target_compile_definitions(corei_ww3d2 INTERFACE
            USE_MOLTENVK=1
        )
    endif()
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Enable validation layers in debug builds
        target_compile_definitions(corei_ww3d2 INTERFACE
            VK_VALIDATION_ENABLED=1
        )
    endif()
elseif(NOT USE_DXVK)
    message(STATUS "DXVK/Vulkan backend disabled - using legacy Metal/OpenGL")
endif()

# ============================================================================

if(APPLE)
    # Link Metal frameworks
    add_metal_support(corei_ww3d2)
endif()

# Add include directory for config_manager.h and other headers
target_include_directories(corei_ww3d2 INTERFACE
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Include
)

# Link OpenAL for audio support on non-Windows platforms
if(NOT WIN32 AND TARGET OpenAL::OpenAL)
    target_link_libraries(corei_ww3d2 INTERFACE OpenAL::OpenAL)
endif()
