set(WW3D2_SRC
    #aabtree.cpp
    #aabtree.h
    #aabtreebuilder.cpp (Windows-only: Uses dx8wrapper.h)
    #aabtreebuilder.h
    #agg_def.cpp
    #agg_def.h
    #animatedsoundmgr.cpp
    #animatedsoundmgr.h
    #animobj.cpp (Windows-only: Uses dx8wrapper.h)
    #animobj.h
    #assetmgr.cpp (Windows-only: Asset manager with D3D dependencies)
    #assetmgr.h
    #assetstatus.cpp
    #assetstatus.h
    #bitmaphandler.cpp
    #bitmaphandler.h
    #bmp2d.cpp (Windows-only: Bitmap 2D with D3D texture loader)
    #bmp2d.h
    #boxrobj.cpp (Windows-only: Uses dx8wrapper.h)
    #boxrobj.h
    #bwrender.cpp
    #bwrender.h
    #camera.cpp (Windows-only: Uses dx8wrapper.h)
    #camera.h
    #classid.h
    #collect.cpp
    #collect.h
    #colorspace.h
    #coltest.cpp
    #coltest.h
    #coltype.h
    #composite.cpp
    #composite.h
    #dazzle.cpp (Windows-only: Uses dx8wrapper.h)
    #dazzle.h
    #ddsfile.cpp (Windows-only: DDS texture format loader with D3D)
    #ddsfile.h
    #decalmsh.cpp (Windows-only: Uses dx8wrapper.h)
    #decalmsh.h
    #decalsys.cpp
    #decalsys.h
    #distlod.cpp
    #distlod.h
    #dllist.h
    #dx8caps.cpp (Windows-only)
    #dx8caps.h
    #dx8fvf.cpp (Windows-only)
    #dx8fvf.h
    #dx8indexbuffer.cpp (Windows-only)
    #dx8indexbuffer.h
    #dx8list.h
    #dx8polygonrenderer.cpp
    #dx8polygonrenderer.h
    #dx8renderer.cpp (Windows-only)
    #dx8renderer.h
    #dx8texman.cpp (Windows-only)
    #dx8texman.h
    #dx8vertexbuffer.cpp (Windows-only)
    #dx8vertexbuffer.h
    #dx8webbrowser.cpp
    #dx8webbrowser.h
    #dx8wrapper.cpp (Windows-only)
    #dx8wrapper.h
    #dynamesh.cpp
    #dynamesh.h
    #font3d.cpp
    #font3d.h
    #formconv.cpp
    #formconv.h
    #FramGrab.cpp
    #framgrab.h
    #hanim.cpp
    #hanim.h
    hanimmgr.cpp
    hanimmgr.h
    #hcanim.cpp
    #hcanim.h
    hlod.cpp
    hlod.h
    #hmdldef.cpp
    #hmdldef.h
    hmorphanim.cpp
    hmorphanim.h
    hrawanim.cpp
    hrawanim.h
    #htree.cpp
    #htree.h
    htreemgr.cpp
    htreemgr.h
    #intersec.cpp
    #intersec.h
    #intersec.inl
    #inttest.h
    #layer.cpp
    #layer.h
    light.cpp
    light.h
    lightenvironment.cpp
    lightenvironment.h
    #line3d.cpp
    #line3d.h
    #mapper.cpp (Windows-only: Uses dx8wrapper.h)
    #mapper.h
    #matinfo.cpp
    #matinfo.h
    #matpass.cpp (Windows-only: Uses dx8wrapper.h)
    #matpass.h
    #matrixmapper.cpp (Windows-only: Uses dx8wrapper.h)
    #matrixmapper.h
    mesh.cpp
    mesh.h
    meshbuild.cpp
    meshbuild.h
    #meshdam.cpp
    #meshdam.h
    meshgeometry.cpp
    meshgeometry.h
    #meshmatdesc.cpp (Windows-only: Uses dx8wrapper.h)
    #meshmatdesc.h
    meshmdl.cpp
    meshmdl.h
    #meshmdlio.cpp (Windows-only: Uses dx8wrapper.h)
    #metalmap.cpp
    #metalmap.h
    #missingtexture.cpp
    #missingtexture.h
    motchan.cpp
    motchan.h
    nullrobj.cpp
    nullrobj.h
    #part_buf.cpp (Windows-only: Uses dx8wrapper.h)
    #part_buf.h
    part_emt.cpp
    part_emt.h
    part_ldr.cpp
    part_ldr.h
    #pivot.cpp
    #pivot.h
    #pointgr.cpp
    #pointgr.h
    #polyinfo.cpp
    #polyinfo.h
    #predlod.cpp
    #predlod.h
    #prim_anim.cpp
    #prim_anim.h
    #projector.cpp
    #projector.h
    #proto.cpp
    #proto.h
    #proxy.h
    #rddesc.h
    #render2d.cpp (Windows-only: Uses dx8wrapper.h)
    #render2d.h
    #render2dsentence.cpp
    #render2dsentence.h
    #renderobjectrecycler.cpp
    #renderobjectrecycler.h
    #rendobj.cpp
    #rendobj.h
    #rinfo.cpp (Windows-only: Uses dx8wrapper.h)
    #rinfo.h
    #ringobj.cpp
    #ringobj.h
    #robjlist.h
    #scene.cpp (Windows-only: Uses dx8wrapper.h)
    #scene.h
    #segline.cpp
    #segline.h
    #seglinerenderer.cpp
    #seglinerenderer.h
    #shader.cpp (Windows-only: Uses dx8wrapper.h)
    #shader.h
    #shattersystem.cpp
    #shattersystem.h
    #snappts.cpp
    #snapPts.h
    #sortingrenderer.cpp (Windows-only: Uses dx8wrapper.h)
    #sortingrenderer.h
    #soundlibrarybridge.h
    #soundrobj.cpp
    #soundrobj.h
    #sphereobj.cpp
    #sphereobj.h
    #static_sort_list.cpp
    #static_sort_list.h
    #statistics.cpp
    #statistics.h
    #streak.cpp
    #streak.h
    #streakRender.cpp
    #streakRender.h
    #stripoptimizer.cpp
    #stripoptimizer.h
    #surfaceclass.cpp
    #surfaceclass.h
    #texproject.cpp (Windows-only: Uses dx8wrapper.h)
    #texproject.h
    #textdraw.cpp
    #textdraw.h
    texture.cpp
    texture.h
    #texturefilter.cpp (Windows-only: Uses dx8wrapper.h)
    #texturefilter.h
    #textureloader.cpp (Windows-only: Uses dx8wrapper.h)
    #textureloader.h
    texturethumbnail.cpp
    texturethumbnail.h
    #vertmaterial.cpp (Windows-only: Uses dx8wrapper.h)
    #vertmaterial.h
    #visrasterizer.cpp
    #visrasterizer.h
    #w3d_dep.cpp
    #w3d_dep.h
    w3d_file.h
    #w3d_obsolete.h
    #w3d_util.cpp
    #w3d_util.h
    #w3derr.h
    #w3dexclusionlist.cpp
    #w3dexclusionlist.h
    #ww3d.cpp (Windows-only: Uses dx8wrapper.h)
    #ww3d.h
    #ww3dformat.cpp
    #ww3dformat.h
    #ww3dids.h
    #ww3dtrig.h
)

add_library(g_ww3d2 STATIC)
set_target_properties(g_ww3d2 PROPERTIES OUTPUT_NAME ww3d2)

target_sources(g_ww3d2 PRIVATE ${WW3D2_SRC})

target_compile_definitions(g_ww3d2 PRIVATE
    $<$<BOOL:${IS_VS6_BUILD}>:WINVER=0x0500>
)

target_precompile_headers(g_ww3d2 PRIVATE
    #dx8wrapper.h
    [["always.h"]]
    [["STLUtils.h"]]
    [["win.h"]]
    [["WWCommon.h"]]
    [["wwstring.h"]]
    # Note: windows.h is included via win.h conditionally (platform-specific)
)

target_link_libraries(g_ww3d2 PRIVATE
    corei_ww3d2
    g_wwcommon
)
