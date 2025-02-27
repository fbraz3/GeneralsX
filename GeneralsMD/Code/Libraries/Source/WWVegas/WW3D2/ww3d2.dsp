# Microsoft Developer Studio Project File - Name="ww3d2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ww3d2 - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ww3d2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ww3d2.mak" CFG="ww3d2 - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ww3d2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 ProfileE" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 DebugE" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "ww3d2 - Win32 DebugW3D" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/ww3d2", QFBEAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ww3d2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "NDEBUG" /D "WIN32" /D "IG_DEBUG_STACKTRACE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gi /GX /Zi /O2 /Ob2 /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "_DEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2Debug.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ww3d2___W"
# PROP BASE Intermediate_Dir "ww3d2___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\srsdk\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /Zi /O2 /Ob2 /I "..\srsdk1x\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_PROFILE" /YX /FD /Gh /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Release\ww3d2.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2Profile.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 ProfileE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ProfileE"
# PROP BASE Intermediate_Dir "ProfileE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ProfileE"
# PROP Intermediate_Dir "ProfileE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /Gi /GX /Zi /O2 /I "..\srsdk1x\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /Gm /Gi /GX /Zi /O2 /I "..\srsdk1x\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "PARAM_EDITING_ON" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Profile\ww3d2.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2ProfileE.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 DebugE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugE"
# PROP BASE Intermediate_Dir "DebugE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugE"
# PROP Intermediate_Dir "DebugE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /Gi /GX /ZI /Od /I "..\srsdk1x\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /Fr /YX /FD /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /Gi /GX /ZI /Od /I "..\srsdk1x\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "PARAM_EDITING_ON" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Debug\ww3d2.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2DebugE.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 Internal"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Internal"
# PROP BASE Intermediate_Dir "Internal"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Internal"
# PROP Intermediate_Dir "Internal"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Zi /O2 /Ob2 /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /O2 /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_INTERNAL" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\ww3d2.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2Internal.lib"

!ELSEIF  "$(CFG)" == "ww3d2 - Win32 DebugW3D"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugW3D"
# PROP BASE Intermediate_Dir "DebugW3D"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugW3D"
# PROP Intermediate_Dir "DebugW3D"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /WX /Gm /Gi /GR /GX /ZI /Od /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /Fr /YX /FD /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /Gi /GX /ZI /Od /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwcpuid" /I "..\wwsaveload" /I "..\wwaudio" /I "..\miles6\include" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\ww3d2Debug.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\ww3d2DebugW3D.lib"

!ENDIF 

# Begin Target

# Name "ww3d2 - Win32 Release"
# Name "ww3d2 - Win32 Debug"
# Name "ww3d2 - Win32 Profile"
# Name "ww3d2 - Win32 ProfileE"
# Name "ww3d2 - Win32 DebugE"
# Name "ww3d2 - Win32 Internal"
# Name "ww3d2 - Win32 DebugW3D"
# Begin Group "Source"

# PROP Default_Filter "cpp;c"
# Begin Source File

SOURCE=.\aabtree.cpp
# End Source File
# Begin Source File

SOURCE=.\aabtreebuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\agg_def.cpp
# End Source File
# Begin Source File

SOURCE=.\animatedsoundmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\animobj.cpp
# End Source File
# Begin Source File

SOURCE=.\assetmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\assetstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\bitmaphandler.cpp
# End Source File
# Begin Source File

SOURCE=.\bmp2d.cpp
# End Source File
# Begin Source File

SOURCE=.\boxrobj.cpp
# End Source File
# Begin Source File

SOURCE=.\bwrender.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\collect.cpp
# End Source File
# Begin Source File

SOURCE=.\coltest.cpp
# End Source File
# Begin Source File

SOURCE=.\composite.cpp
# End Source File
# Begin Source File

SOURCE=.\dazzle.cpp
# End Source File
# Begin Source File

SOURCE=.\ddsfile.cpp
# End Source File
# Begin Source File

SOURCE=.\decalmsh.cpp
# End Source File
# Begin Source File

SOURCE=.\decalsys.cpp
# End Source File
# Begin Source File

SOURCE=.\distlod.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8caps.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8fvf.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8indexbuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8polygonrenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8rendererdebugger.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8texman.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8vertexbuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8webbrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\dx8wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\dynamesh.cpp
# End Source File
# Begin Source File

SOURCE=.\font3d.cpp
# End Source File
# Begin Source File

SOURCE=.\formconv.cpp
# End Source File
# Begin Source File

SOURCE=.\FramGrab.cpp
# End Source File
# Begin Source File

SOURCE=.\hanim.cpp
# End Source File
# Begin Source File

SOURCE=.\hanimmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\hcanim.cpp
# End Source File
# Begin Source File

SOURCE=.\hlod.cpp
# End Source File
# Begin Source File

SOURCE=.\hmdldef.cpp
# End Source File
# Begin Source File

SOURCE=.\hmorphanim.cpp
# End Source File
# Begin Source File

SOURCE=.\hrawanim.cpp
# End Source File
# Begin Source File

SOURCE=.\htree.cpp
# End Source File
# Begin Source File

SOURCE=.\htreemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\intersec.cpp
# End Source File
# Begin Source File

SOURCE=.\layer.cpp
# End Source File
# Begin Source File

SOURCE=.\light.cpp
# End Source File
# Begin Source File

SOURCE=.\lightenvironment.cpp
# End Source File
# Begin Source File

SOURCE=.\line3d.cpp
# End Source File
# Begin Source File

SOURCE=.\linegrp.cpp
# End Source File
# Begin Source File

SOURCE=.\mapper.cpp
# End Source File
# Begin Source File

SOURCE=.\matinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\matpass.cpp
# End Source File
# Begin Source File

SOURCE=.\matrixmapper.cpp
# End Source File
# Begin Source File

SOURCE=.\mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\meshbuild.cpp
# End Source File
# Begin Source File

SOURCE=.\meshdam.cpp
# End Source File
# Begin Source File

SOURCE=.\meshgeometry.cpp
# End Source File
# Begin Source File

SOURCE=.\meshmatdesc.cpp
# End Source File
# Begin Source File

SOURCE=.\meshmdl.cpp
# End Source File
# Begin Source File

SOURCE=.\meshmdlio.cpp
# End Source File
# Begin Source File

SOURCE=.\metalmap.cpp
# End Source File
# Begin Source File

SOURCE=.\missingtexture.cpp
# End Source File
# Begin Source File

SOURCE=.\motchan.cpp
# End Source File
# Begin Source File

SOURCE=.\nullrobj.cpp
# End Source File
# Begin Source File

SOURCE=.\part_buf.cpp
# End Source File
# Begin Source File

SOURCE=.\part_emt.cpp
# End Source File
# Begin Source File

SOURCE=.\part_ldr.cpp
# End Source File
# Begin Source File

SOURCE=.\pivot.cpp
# End Source File
# Begin Source File

SOURCE=.\pointgr.cpp
# End Source File
# Begin Source File

SOURCE=.\polyinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\predlod.cpp
# End Source File
# Begin Source File

SOURCE=.\prim_anim.cpp
# End Source File
# Begin Source File

SOURCE=.\projector.cpp
# End Source File
# Begin Source File

SOURCE=.\proto.cpp
# End Source File
# Begin Source File

SOURCE=.\render2d.cpp
# End Source File
# Begin Source File

SOURCE=.\render2dsentence.cpp
# End Source File
# Begin Source File

SOURCE=.\renderobjectrecycler.cpp
# End Source File
# Begin Source File

SOURCE=.\rendobj.cpp
# End Source File
# Begin Source File

SOURCE=.\rinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ringobj.cpp
# End Source File
# Begin Source File

SOURCE=.\scene.cpp
# End Source File
# Begin Source File

SOURCE=.\segline.cpp
# End Source File
# Begin Source File

SOURCE=.\seglinerenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\shader.cpp
# End Source File
# Begin Source File

SOURCE=.\shattersystem.cpp
# End Source File
# Begin Source File

SOURCE=.\snappts.cpp
# End Source File
# Begin Source File

SOURCE=.\sortingrenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\soundrobj.cpp
# End Source File
# Begin Source File

SOURCE=.\sphereobj.cpp
# End Source File
# Begin Source File

SOURCE=.\static_sort_list.cpp
# End Source File
# Begin Source File

SOURCE=.\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\streak.cpp
# End Source File
# Begin Source File

SOURCE=.\streakRender.cpp
# End Source File
# Begin Source File

SOURCE=.\stripoptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\surfaceclass.cpp
# End Source File
# Begin Source File

SOURCE=.\texfcach.cpp
# End Source File
# Begin Source File

SOURCE=.\texproject.cpp
# End Source File
# Begin Source File

SOURCE=.\texture.cpp
# End Source File
# Begin Source File

SOURCE=.\texturefilter.cpp
# End Source File
# Begin Source File

SOURCE=.\textureloader.cpp
# End Source File
# Begin Source File

SOURCE=.\texturethumbnail.cpp
# End Source File
# Begin Source File

SOURCE=.\txt.cpp
# End Source File
# Begin Source File

SOURCE=.\vertmaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\visrasterizer.cpp
# End Source File
# Begin Source File

SOURCE=.\w3d_dep.cpp
# ADD CPP /GX
# End Source File
# Begin Source File

SOURCE=.\w3d_util.cpp
# End Source File
# Begin Source File

SOURCE=.\w3dexclusionlist.cpp
# End Source File
# Begin Source File

SOURCE=.\ww3d.cpp
# End Source File
# Begin Source File

SOURCE=.\ww3dformat.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\aabtree.h
# End Source File
# Begin Source File

SOURCE=.\aabtreebuilder.h
# End Source File
# Begin Source File

SOURCE=.\agg_def.h
# End Source File
# Begin Source File

SOURCE=.\animatedsoundmgr.h
# End Source File
# Begin Source File

SOURCE=.\animobj.h
# End Source File
# Begin Source File

SOURCE=.\assetmgr.h
# End Source File
# Begin Source File

SOURCE=.\assetstatus.h
# End Source File
# Begin Source File

SOURCE=.\bitmaphandler.h
# End Source File
# Begin Source File

SOURCE=.\bmp2d.h
# End Source File
# Begin Source File

SOURCE=.\boxrobj.h
# End Source File
# Begin Source File

SOURCE=.\bwrender.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\collect.h
# End Source File
# Begin Source File

SOURCE=.\colorspace.h
# End Source File
# Begin Source File

SOURCE=.\coltest.h
# End Source File
# Begin Source File

SOURCE=.\coltype.h
# End Source File
# Begin Source File

SOURCE=.\composite.h
# End Source File
# Begin Source File

SOURCE=.\dazzle.h
# End Source File
# Begin Source File

SOURCE=.\ddsfile.h
# End Source File
# Begin Source File

SOURCE=.\decalmsh.h
# End Source File
# Begin Source File

SOURCE=.\decalsys.h
# End Source File
# Begin Source File

SOURCE=.\distlod.h
# End Source File
# Begin Source File

SOURCE=.\dllist.h
# End Source File
# Begin Source File

SOURCE=.\dx8caps.h
# End Source File
# Begin Source File

SOURCE=.\dx8fvf.h
# End Source File
# Begin Source File

SOURCE=.\dx8indexbuffer.h
# End Source File
# Begin Source File

SOURCE=.\dx8list.h
# End Source File
# Begin Source File

SOURCE=.\dx8polygonrenderer.h
# End Source File
# Begin Source File

SOURCE=.\dx8renderer.h
# End Source File
# Begin Source File

SOURCE=.\dx8rendererdebugger.h
# End Source File
# Begin Source File

SOURCE=.\dx8texman.h
# End Source File
# Begin Source File

SOURCE=.\dx8vertexbuffer.h
# End Source File
# Begin Source File

SOURCE=.\dx8webbrowser.h
# End Source File
# Begin Source File

SOURCE=.\dx8wrapper.h
# End Source File
# Begin Source File

SOURCE=.\dynamesh.h
# End Source File
# Begin Source File

SOURCE=.\font3d.h
# End Source File
# Begin Source File

SOURCE=.\formconv.h
# End Source File
# Begin Source File

SOURCE=.\framgrab.h
# End Source File
# Begin Source File

SOURCE=.\hanim.h
# End Source File
# Begin Source File

SOURCE=.\hanimmgr.h
# End Source File
# Begin Source File

SOURCE=.\hcanim.h
# End Source File
# Begin Source File

SOURCE=.\hlod.h
# End Source File
# Begin Source File

SOURCE=.\hmdldef.H
# End Source File
# Begin Source File

SOURCE=.\hmorphanim.h
# End Source File
# Begin Source File

SOURCE=.\hrawanim.h
# End Source File
# Begin Source File

SOURCE=.\htree.h
# End Source File
# Begin Source File

SOURCE=.\htreemgr.h
# End Source File
# Begin Source File

SOURCE=.\intersec.h
# End Source File
# Begin Source File

SOURCE=.\intersec.inl
# End Source File
# Begin Source File

SOURCE=.\inttest.h
# End Source File
# Begin Source File

SOURCE=.\layer.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\lightenvironment.h
# End Source File
# Begin Source File

SOURCE=.\line3d.h
# End Source File
# Begin Source File

SOURCE=.\linegrp.h
# End Source File
# Begin Source File

SOURCE=.\mapper.h
# End Source File
# Begin Source File

SOURCE=.\matinfo.h
# End Source File
# Begin Source File

SOURCE=.\matpass.h
# End Source File
# Begin Source File

SOURCE=.\matrixmapper.h
# End Source File
# Begin Source File

SOURCE=.\mesh.h
# End Source File
# Begin Source File

SOURCE=.\meshbuild.h
# End Source File
# Begin Source File

SOURCE=.\meshdam.h
# End Source File
# Begin Source File

SOURCE=.\meshgeometry.h
# End Source File
# Begin Source File

SOURCE=.\meshmatdesc.h
# End Source File
# Begin Source File

SOURCE=.\meshmdl.h
# End Source File
# Begin Source File

SOURCE=.\metalmap.h
# End Source File
# Begin Source File

SOURCE=.\missingtexture.h
# End Source File
# Begin Source File

SOURCE=.\motchan.h
# End Source File
# Begin Source File

SOURCE=.\nullrobj.h
# End Source File
# Begin Source File

SOURCE=.\part_buf.h
# End Source File
# Begin Source File

SOURCE=.\part_emt.h
# End Source File
# Begin Source File

SOURCE=.\part_ldr.h
# End Source File
# Begin Source File

SOURCE=.\pivot.h
# End Source File
# Begin Source File

SOURCE=.\pointgr.h
# End Source File
# Begin Source File

SOURCE=.\polyinfo.h
# End Source File
# Begin Source File

SOURCE=.\predlod.h
# End Source File
# Begin Source File

SOURCE=.\prim_anim.h
# End Source File
# Begin Source File

SOURCE=.\projector.h
# End Source File
# Begin Source File

SOURCE=.\proto.h
# End Source File
# Begin Source File

SOURCE=.\proxy.h
# End Source File
# Begin Source File

SOURCE=.\rddesc.h
# End Source File
# Begin Source File

SOURCE=.\render2d.h
# End Source File
# Begin Source File

SOURCE=.\render2dsentence.h
# End Source File
# Begin Source File

SOURCE=.\renderobjectrecycler.h
# End Source File
# Begin Source File

SOURCE=.\rendobj.h
# End Source File
# Begin Source File

SOURCE=.\rinfo.h
# End Source File
# Begin Source File

SOURCE=.\ringobj.h
# End Source File
# Begin Source File

SOURCE=.\robjlist.h
# End Source File
# Begin Source File

SOURCE=.\scene.h
# End Source File
# Begin Source File

SOURCE=.\segline.h
# End Source File
# Begin Source File

SOURCE=.\seglinerenderer.h
# End Source File
# Begin Source File

SOURCE=.\shader.h
# End Source File
# Begin Source File

SOURCE=.\shattersystem.h
# End Source File
# Begin Source File

SOURCE=.\shdlib.h
# End Source File
# Begin Source File

SOURCE=.\snapPts.h
# End Source File
# Begin Source File

SOURCE=.\sortingrenderer.h
# End Source File
# Begin Source File

SOURCE=.\soundrobj.h
# End Source File
# Begin Source File

SOURCE=.\sphereobj.h
# End Source File
# Begin Source File

SOURCE=.\static_sort_list.h
# End Source File
# Begin Source File

SOURCE=.\statistics.h
# End Source File
# Begin Source File

SOURCE=.\streak.h
# End Source File
# Begin Source File

SOURCE=.\streakRender.h
# End Source File
# Begin Source File

SOURCE=.\stripoptimizer.h
# End Source File
# Begin Source File

SOURCE=.\surfaceclass.h
# End Source File
# Begin Source File

SOURCE=.\texfcach.h
# End Source File
# Begin Source File

SOURCE=.\texproject.h
# End Source File
# Begin Source File

SOURCE=.\texture.h
# End Source File
# Begin Source File

SOURCE=.\texturefilter.h
# End Source File
# Begin Source File

SOURCE=.\textureloader.h
# End Source File
# Begin Source File

SOURCE=.\texturethumbnail.h
# End Source File
# Begin Source File

SOURCE=.\txt.h
# End Source File
# Begin Source File

SOURCE=.\vertmaterial.h
# End Source File
# Begin Source File

SOURCE=.\visrasterizer.h
# End Source File
# Begin Source File

SOURCE=.\w3d_dep.h
# End Source File
# Begin Source File

SOURCE=.\w3d_file.h
# End Source File
# Begin Source File

SOURCE=.\w3d_obsolete.h
# End Source File
# Begin Source File

SOURCE=.\w3d_util.h
# End Source File
# Begin Source File

SOURCE=.\w3derr.h
# End Source File
# Begin Source File

SOURCE=.\w3dexclusionlist.h
# End Source File
# Begin Source File

SOURCE=.\ww3d.h
# End Source File
# Begin Source File

SOURCE=.\ww3dformat.h
# End Source File
# Begin Source File

SOURCE=.\ww3dids.h
# End Source File
# Begin Source File

SOURCE=.\ww3dtrig.h
# End Source File
# End Group
# Begin Group "RequiredAssets"

# PROP Default_Filter "*.tga;*.w3d"
# Begin Source File

SOURCE=.\RequiredAssets\AddProjectorGradient.tga
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\Dazzle.INI
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\MultProjectorGradient.tga
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\ShatterAVel.tbl
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\ShatterPlanes0.w3d
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\ShatterVel.tbl
# End Source File
# Begin Source File

SOURCE=.\RequiredAssets\w3d_missing_texture.tga
# End Source File
# End Group
# Begin Source File

SOURCE=".\DX8 Rationalizations.txt"
# End Source File
# Begin Source File

SOURCE=".\DX8 Status.txt"
# End Source File
# Begin Source File

SOURCE=.\DX8ToolModifications.txt
# End Source File
# Begin Source File

SOURCE=.\MAPPERS.TXT
# End Source File
# Begin Source File

SOURCE=.\RenderObjectGuide.txt
# End Source File
# Begin Source File

SOURCE=".\State Management.txt"
# End Source File
# End Target
# End Project
