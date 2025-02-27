# Microsoft Developer Studio Project File - Name="max2w3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=max2w3d - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "max2w3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "max2w3d.mak" CFG="max2w3d - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "max2w3d - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "max2w3d - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "max2w3d - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "max2w3d - Win32 Max4Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "max2w3d - Win32 Max4Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Tools/max2w3d", SSJAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "max2w3d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\Pluglib" /I "D:\3dsmax4\maxsdk\Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Release" /libpath:"D:\3dsmax4\maxsdk\lib"
# SUBTRACT LINK32 /debug
# Begin Custom Build
TargetPath=.\Run\Max2w3d.dle
TargetName=Max2w3d
InputPath=.\Run\Max2w3d.dle
SOURCE="$(InputPath)"

"$(MAXDIR)\Plugins\Westwood\$(TargetName).dle" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Copy $(TargetPath) $(MAXDIR)\Plugins\Westwood\$(TargetName).dle

# End Custom Build

!ELSEIF  "$(CFG)" == "max2w3d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\Pluglib" /I "D:\3dsmax4\maxsdk\Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib pluglibd.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Debug" /libpath:"D:\3dsmax4\maxsdk\lib"
# Begin Custom Build
TargetPath=.\Run\Max2w3d.dle
TargetName=Max2w3d
InputPath=.\Run\Max2w3d.dle
SOURCE="$(InputPath)"

"$(MAXDIR)\Plugins\Westwood\$(TargetName).dle" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Copy $(TargetPath) $(MAXDIR)\Plugins\Westwood\$(TargetName).dle

# End Custom Build

!ELSEIF  "$(CFG)" == "max2w3d - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "max2w3d_"
# PROP BASE Intermediate_Dir "max2w3d_"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Hybrid"
# PROP Intermediate_Dir ".\Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Z7 /O2 /I "..\pluglib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /Od /I "..\Pluglib" /I "D:\3dsmax4\maxsdk\Include" /D "WIN32" /D "_WINDOWS" /U "_UNICODE _MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib core.lib geom.lib mesh.lib util.lib bmm.lib pluglib.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"c:/3dsmax2/plugins/w3d/max2w3d.dle" /libpath:"..\pluglib\lib"
# ADD LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Hybrid" /libpath:"D:\3dsmax4\maxsdk\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=.\Run\Max2w3d.dle
TargetName=Max2w3d
InputPath=.\Run\Max2w3d.dle
SOURCE="$(InputPath)"

"$(MAXDIR)\Plugins\Westwood\$(TargetName).dle" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Copy $(TargetPath) "$(MAXDIR)\Plugins\Westwood\$(TargetName).dle"

# End Custom Build

!ELSEIF  "$(CFG)" == "max2w3d - Win32 Max4Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Max4Hybrid"
# PROP BASE Intermediate_Dir "Max4Hybrid"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Max4Hybrid"
# PROP Intermediate_Dir "Max4Hybrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Z7 /Od /I "..\Pluglib" /D "WIN32" /D "_WINDOWS" /U "_UNICODE _MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /Od /I "..\Pluglib" /I "D:\3dsmax4\maxsdk\Include" /I "D:\3dsmax4\maxsdk\Include\Maxscrpt" /D "WIN32" /D "_WINDOWS" /D "W3D_MAX4" /U "_UNICODE _MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Hybrid"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Hybrid" /libpath:"D:\3dsmax4\maxsdk\lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
TargetPath=.\Run\Max2w3d.dle
TargetName=Max2w3d
InputPath=.\Run\Max2w3d.dle
SOURCE="$(InputPath)"

"D:\3dsmax4\Plugins\Westwood\$(TargetName).dle" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Copy $(TargetPath) D:\3dsmax4\Plugins\Westwood\$(TargetName).dle

# End Custom Build

!ELSEIF  "$(CFG)" == "max2w3d - Win32 Max4Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Max4Release"
# PROP BASE Intermediate_Dir "Max4Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Max4Release"
# PROP Intermediate_Dir "Max4Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\Pluglib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\Pluglib" /I "D:\3dsmax4\maxsdk\Include" /I "D:\3dsmax4\maxsdk\Include\Maxscrpt" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "W3D_MAX4" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Release"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 pluglib.lib mesh.lib maxutil.lib maxscrpt.lib bmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib core.lib geom.lib paramblk2.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Run\Max2w3d.dle" /libpath:"..\Pluglib\Release" /libpath:"D:\3dsmax4\maxsdk\lib"
# SUBTRACT LINK32 /debug
# Begin Custom Build
TargetPath=.\Run\Max2w3d.dle
TargetName=Max2w3d
InputPath=.\Run\Max2w3d.dle
SOURCE="$(InputPath)"

"$(MAXDIR)\Plugins\Westwood\$(TargetName).dle" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	Copy $(TargetPath) $(MAXDIR)\Plugins\Westwood\$(TargetName).dle

# End Custom Build

!ENDIF 

# Begin Target

# Name "max2w3d - Win32 Release"
# Name "max2w3d - Win32 Debug"
# Name "max2w3d - Win32 Hybrid"
# Name "max2w3d - Win32 Max4Hybrid"
# Name "max2w3d - Win32 Max4Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\aabtreebuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\AlphaModifier.cpp
# End Source File
# Begin Source File

SOURCE=.\animationcompressionsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\bchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\boneicon.cpp
# End Source File
# Begin Source File

SOURCE=.\bpick.cpp
# End Source File
# Begin Source File

SOURCE=.\colboxsave.cpp
# End Source File
# Begin Source File

SOURCE=.\dazzlesave.cpp
# End Source File
# Begin Source File

SOURCE=.\dllmain.cpp
# End Source File
# Begin Source File

SOURCE=.\exportlog.cpp
# End Source File
# Begin Source File

SOURCE=.\floaterdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FormClass.cpp
# End Source File
# Begin Source File

SOURCE=.\gamemaps.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtl.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlForm.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlPassDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlShaderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlTextureDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GameMtlVertexMaterialDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\genlodextensiondialog.cpp
# End Source File
# Begin Source File

SOURCE=.\genmtlnamesdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\gennamesdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\geometryexporttask.cpp
# End Source File
# Begin Source File

SOURCE=.\gridsnapmodifier.cpp
# End Source File
# Begin Source File

SOURCE=.\hiersave.cpp
# End Source File
# Begin Source File

SOURCE=.\hlodsave.cpp
# End Source File
# Begin Source File

SOURCE=.\logdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\maxworldinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\meshbuild.cpp
# End Source File
# Begin Source File

SOURCE=.\meshcon.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeform.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformData.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSave.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSaveSet.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSet.cpp
# End Source File
# Begin Source File

SOURCE=.\MeshDeformUndo.cpp
# End Source File
# Begin Source File

SOURCE=.\meshsave.cpp
# End Source File
# Begin Source File

SOURCE=.\motion.cpp
# End Source File
# Begin Source File

SOURCE=.\namedsel.cpp
# End Source File
# Begin Source File

SOURCE=.\nullsave.cpp
# End Source File
# Begin Source File

SOURCE=.\PCToPS2Material.cpp
# End Source File
# Begin Source File

SOURCE=.\presetexportoptionsdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2GameMtl.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2GameMtlShaderDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\rcmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\simpdib.cpp
# End Source File
# Begin Source File

SOURCE=.\skin.cpp
# End Source File
# Begin Source File

SOURCE=.\skindata.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\TARGA.CPP
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\vchannel.cpp
# End Source File
# Begin Source File

SOURCE=.\vxl.cpp
# End Source File
# Begin Source File

SOURCE=.\vxldbg.cpp
# End Source File
# Begin Source File

SOURCE=.\vxllayer.cpp
# End Source File
# Begin Source File

SOURCE=.\w3dappdata.cpp
# End Source File
# Begin Source File

SOURCE=.\w3ddesc.cpp
# End Source File
# Begin Source File

SOURCE=.\w3ddlg.cpp
# End Source File
# Begin Source File

SOURCE=.\w3dexp.cpp
# End Source File
# Begin Source File

SOURCE=.\w3dmtl.cpp
# End Source File
# Begin Source File

SOURCE=.\w3dutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\aabtreebuilder.h
# End Source File
# Begin Source File

SOURCE=.\AlphaModifier.h
# End Source File
# Begin Source File

SOURCE=.\animationcompressionsettings.h
# End Source File
# Begin Source File

SOURCE=.\bchannel.h
# End Source File
# Begin Source File

SOURCE=.\boneicon.h
# End Source File
# Begin Source File

SOURCE=.\bpick.h
# End Source File
# Begin Source File

SOURCE=.\colboxsave.h
# End Source File
# Begin Source File

SOURCE=.\dazzlesave.h
# End Source File
# Begin Source File

SOURCE=.\dllmain.h
# End Source File
# Begin Source File

SOURCE=.\exportlog.h
# End Source File
# Begin Source File

SOURCE=.\floaterdialog.h
# End Source File
# Begin Source File

SOURCE=.\FormClass.h
# End Source File
# Begin Source File

SOURCE=.\gamemaps.h
# End Source File
# Begin Source File

SOURCE=.\gamemtl.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlDlg.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlForm.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlPassDlg.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlShaderDlg.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlTextureDlg.h
# End Source File
# Begin Source File

SOURCE=.\GameMtlVertexMaterialDlg.h
# End Source File
# Begin Source File

SOURCE=.\genlodextensiondialog.h
# End Source File
# Begin Source File

SOURCE=.\genmtlnamesdialog.h
# End Source File
# Begin Source File

SOURCE=.\gennamesdialog.h
# End Source File
# Begin Source File

SOURCE=.\geometryexportcontext.h
# End Source File
# Begin Source File

SOURCE=.\geometryexporttask.h
# End Source File
# Begin Source File

SOURCE=.\gridsnapmodifier.h
# End Source File
# Begin Source File

SOURCE=.\hiersave.h
# End Source File
# Begin Source File

SOURCE=.\hlodsave.h
# End Source File
# Begin Source File

SOURCE=.\logdlg.h
# End Source File
# Begin Source File

SOURCE=.\maxworldinfo.h
# End Source File
# Begin Source File

SOURCE=.\meshbuild.h
# End Source File
# Begin Source File

SOURCE=.\meshcon.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeform.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformData.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformDefs.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformPanel.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSave.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSaveDefs.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSaveSet.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformSet.h
# End Source File
# Begin Source File

SOURCE=.\MeshDeformUndo.h
# End Source File
# Begin Source File

SOURCE=.\meshsave.h
# End Source File
# Begin Source File

SOURCE=.\motion.h
# End Source File
# Begin Source File

SOURCE=.\namedsel.h
# End Source File
# Begin Source File

SOURCE=.\nullsave.h
# End Source File
# Begin Source File

SOURCE=.\presetexportoptionsdialog.h
# End Source File
# Begin Source File

SOURCE=.\PS2GameMtlShaderDlg.h
# End Source File
# Begin Source File

SOURCE=.\rcmenu.h
# End Source File
# Begin Source File

SOURCE=..\pluglib\realcrc.cpp
# End Source File
# Begin Source File

SOURCE=.\simpdib.h
# End Source File
# Begin Source File

SOURCE=.\skin.h
# End Source File
# Begin Source File

SOURCE=.\skindata.h
# End Source File
# Begin Source File

SOURCE=.\SnapPoints.h
# End Source File
# Begin Source File

SOURCE=.\TARGA.H
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\vchannel.h
# End Source File
# Begin Source File

SOURCE=.\vxl.h
# End Source File
# Begin Source File

SOURCE=.\vxldbg.h
# End Source File
# Begin Source File

SOURCE=.\vxllayer.h
# End Source File
# Begin Source File

SOURCE=.\w3d_file.h
# End Source File
# Begin Source File

SOURCE=.\w3d_obsolete.h
# End Source File
# Begin Source File

SOURCE=.\w3dappdata.h
# End Source File
# Begin Source File

SOURCE=.\w3ddesc.h
# End Source File
# Begin Source File

SOURCE=.\w3ddlg.h
# End Source File
# Begin Source File

SOURCE=.\w3dexp.h
# End Source File
# Begin Source File

SOURCE=.\w3dmtl.h
# End Source File
# Begin Source File

SOURCE=.\w3dutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Res\DISK12.ICO
# End Source File
# Begin Source File

SOURCE=.\max2w3d.def
# End Source File
# Begin Source File

SOURCE=.\max2w3d.rc
# End Source File
# Begin Source File

SOURCE=.\Res\one.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\orig.ico
# End Source File
# Begin Source File

SOURCE=.\Res\orig1.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Res\two.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\ww3d.bmp
# End Source File
# End Group
# Begin Group "MAXScript Extensions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AppData.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportAll.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportAllDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportAllDlg.h
# End Source File
# Begin Source File

SOURCE=.\InputDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InputDlg.h
# End Source File
# Begin Source File

SOURCE=.\SceneSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneSetupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SceneSetupDlg.h
# End Source File
# Begin Source File

SOURCE=.\SkinCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\Utility.cpp
# End Source File
# End Group
# Begin Group "MAXScripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Export.ms
# End Source File
# Begin Source File

SOURCE=.\ResetAndCollapse.ms
# End Source File
# Begin Source File

SOURCE=.\SceneSetup.ms
# End Source File
# End Group
# Begin Source File

SOURCE=.\changes.txt
# End Source File
# End Target
# End Project
