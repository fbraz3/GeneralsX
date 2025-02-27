# Microsoft Developer Studio Project File - Name="wwshade" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wwshade - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwshade.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwshade.mak" CFG="wwshade - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwshade - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wwshade - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "wwshade - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "wwshade - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "wwshade - Win32 DebugW3D" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "wwshade"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwshade - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /Zi /O2 /I "..\wwsaveload" /I "..\ww3d2" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWShade.lib"

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\wwsaveload" /I "..\ww3d2" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /D "_MBCS" /D "_LIB" /D "WWDEBUG" /D "_DEBUG" /D "WIN32" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWShadeDebug.lib"

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\DirectX\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwsaveload" /I "..\ww3d2" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "..\DirectX\include" /I "$(ProjDir)/DXSDK\INCLUDE" /I "..\wwsaveload" /I "..\ww3d2" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "../STLPORT" /I "../../../../DXSDK/INCLUDE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_PROFILE" /YX /FD /Gh /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWShadeProfile.lib"

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\DirectX\include" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /I "..\wwsaveload" /I "..\ww3d2" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /O2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /I "..\wwsaveload" /I "..\ww3d2" /D WINVER=0x400 /D "WWDEBUG" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_INTERNAL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWShadeRelease.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWShadeInternal.lib"

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\wwsaveload" /I "..\ww3d2" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /D "_MBCS" /D "_LIB" /D "WWDEBUG" /D "_DEBUG" /D "WIN32" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GX /ZI /Od /I "..\wwsaveload" /I "..\ww3d2" /I "..\wwlib" /I "..\wwmath" /I "..\wwdebug" /D "_MBCS" /D "_LIB" /D "WWDEBUG" /D "_DEBUG" /D "WIN32" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWShadeDebug.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWShadeDebugW3D.lib"

!ENDIF 

# Begin Target

# Name "wwshade - Win32 Release"
# Name "wwshade - Win32 Debug"
# Name "wwshade - Win32 Profile"
# Name "wwshade - Win32 Internal"
# Name "wwshade - Win32 DebugW3D"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\shddef.cpp
# End Source File
# Begin Source File

SOURCE=.\shddeffactory.cpp
# End Source File
# Begin Source File

SOURCE=.\shddefmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\shdforcelinks.cpp
# End Source File
# Begin Source File

SOURCE=.\shdhwshader.cpp
# End Source File
# Begin Source File

SOURCE=.\shdinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\shdlib.cpp
# End Source File
# Begin Source File

SOURCE=.\shdloader.cpp
# End Source File
# Begin Source File

SOURCE=.\shdmesh.cpp
# End Source File
# Begin Source File

SOURCE=.\shdrenderer.cpp
# End Source File
# Begin Source File

SOURCE=.\shdsubmesh.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\shdclassids.h
# End Source File
# Begin Source File

SOURCE=.\shddef.h
# End Source File
# Begin Source File

SOURCE=.\shddeffactory.h
# End Source File
# Begin Source File

SOURCE=.\shddefmanager.h
# End Source File
# Begin Source File

SOURCE=.\shdforcelinks.h
# End Source File
# Begin Source File

SOURCE=.\shdhw_constants.h
# End Source File
# Begin Source File

SOURCE=.\shdhwshader.h
# End Source File
# Begin Source File

SOURCE=.\shdinterface.h
# End Source File
# Begin Source File

SOURCE=.\shdloader.h
# End Source File
# Begin Source File

SOURCE=.\shdmesh.h
# End Source File
# Begin Source File

SOURCE=.\shdrenderer.h
# End Source File
# Begin Source File

SOURCE=.\shdsubmesh.h
# End Source File
# End Group
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Group "Simple"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shdsimple.cpp
# End Source File
# Begin Source File

SOURCE=.\shdsimple.h
# End Source File
# End Group
# Begin Group "Bump Specular"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shd6bumpspec.cpp
# End Source File
# Begin Source File

SOURCE=.\shd6bumpspec.h
# End Source File
# Begin Source File

SOURCE=.\shd6bumpspec.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd6bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd6bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd6bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd6bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd6bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd6bumpspec_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpspec.cpp
# End Source File
# Begin Source File

SOURCE=.\shd7bumpspec.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpspec_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpspecpass0.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd7bumpspecpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd7bumpspecpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd7bumpspecpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd7bumpspecpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd7bumpspecpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd7bumpspecpass1.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd7bumpspecpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd7bumpspecpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd7bumpspecpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd7bumpspecpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd7bumpspecpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpspec.cpp
# End Source File
# Begin Source File

SOURCE=.\shd8bumpspec.h
# End Source File
# Begin Source File

SOURCE=.\shd8bumpspec.psh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8bumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8bumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8bumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8bumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8bumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpspec.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8bumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpspec_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd8ssbumpspec.psh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8ssbumpspec.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8ssbumpspec.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shdbumpspec.cpp
# End Source File
# Begin Source File

SOURCE=.\shdbumpspec.h
# End Source File
# End Group
# Begin Group "Bump Diffuse"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shd6bumpdiff.cpp
# End Source File
# Begin Source File

SOURCE=.\shd6bumpdiff.h
# End Source File
# Begin Source File

SOURCE=.\shd6bumpdiff.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd6bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd6bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd6bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd6bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd6bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd6bumpdiff_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpdiff.cpp
# End Source File
# Begin Source File

SOURCE=.\shd7bumpdiff.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpdiff_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd7bumpdiffpass0.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass0.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd7bumpdiffpass1.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd7bumpdiffpass1.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpdiff.cpp
# End Source File
# Begin Source File

SOURCE=.\shd8bumpdiff.h
# End Source File
# Begin Source File

SOURCE=.\shd8bumpdiff.psh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8bumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8bumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8bumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8bumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8bumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpdiff.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8bumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8bumpdiff_constants.h
# End Source File
# Begin Source File

SOURCE=.\shd8ssbumpdiff.psh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.psh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shd8ssbumpdiff.vsh

!IF  "$(CFG)" == "wwshade - Win32 Release"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Debug"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Profile"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 Internal"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "wwshade - Win32 DebugW3D"

# Begin Custom Build
InputPath=.\shd8ssbumpdiff.vsh

"$(InputPath)_code.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	shdpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shdbumpdiff.cpp
# End Source File
# Begin Source File

SOURCE=.\shdbumpdiff.h
# End Source File
# End Group
# Begin Group "Bump Cube Map"

# PROP Default_Filter ""
# End Group
# Begin Group "Bump Reflect"

# PROP Default_Filter ""
# End Group
# Begin Group "Gloss Mask"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shdglossmask.cpp
# End Source File
# Begin Source File

SOURCE=.\shdglossmask.h
# End Source File
# End Group
# Begin Group "Cube Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shdcubemap.cpp
# End Source File
# Begin Source File

SOURCE=.\shdcubemap.h
# End Source File
# End Group
# Begin Group "LegacyW3D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shdlegacyw3d.cpp
# End Source File
# Begin Source File

SOURCE=.\shdlegacyw3d.h
# End Source File
# End Group
# End Group
# End Target
# End Project
