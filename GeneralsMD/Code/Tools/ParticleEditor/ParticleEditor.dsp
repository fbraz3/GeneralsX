# Microsoft Developer Studio Project File - Name="ParticleEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ParticleEditor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ParticleEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ParticleEditor.mak" CFG="ParticleEditor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ParticleEditor - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ParticleEditor - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ParticleEditor"
# PROP Scc_LocalPath "..\ParticleEditor"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ParticleEditor - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:"..\..\..\Run\ParticleEditor.pdb" /map:"..\..\..\Run\ParticleEditor.map" /debug /machine:I386 /out:"..\..\..\Run\ParticleEditor.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying        Release        DLL        to        Run\ 
PostBuild_Cmds=post-build-release.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ParticleEditor - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:"..\..\..\Run\ParticleEditorD.pdb" /map:"..\..\..\Run\ParticleEditorD.map" /debug /machine:I386 /out:"..\..\..\Run\ParticleEditorD.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ParticleEditor - Win32 Release"
# Name "ParticleEditor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CButtonShowColor.cpp
# End Source File
# Begin Source File

SOURCE=.\CColorAlphaDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CSwitchesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\EmissionTypePanels.cpp
# End Source File
# Begin Source File

SOURCE=.\MoreParmsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleEditor.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleEditor.def
# End Source File
# Begin Source File

SOURCE=.\ParticleEditor.rc
# End Source File
# Begin Source File

SOURCE=.\ParticleEditorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleTypePanels.cpp
# End Source File
# Begin Source File

SOURCE=.\ShaderTypePanels.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VelocityTypePanels.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CButtonShowColor.h
# End Source File
# Begin Source File

SOURCE=.\CColorAlphaDialog.h
# End Source File
# Begin Source File

SOURCE=.\CSwitchesDialog.h
# End Source File
# Begin Source File

SOURCE=.\EmissionTypePanels.h
# End Source File
# Begin Source File

SOURCE=.\ISwapablePanel.h
# End Source File
# Begin Source File

SOURCE=.\MoreParmsDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleEditor.h
# End Source File
# Begin Source File

SOURCE=.\ParticleEditorDialog.h
# End Source File
# Begin Source File

SOURCE=.\ParticleEditorExport.h
# End Source File
# Begin Source File

SOURCE=.\ParticleTypePanels.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ShaderTypePanels.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VelocityTypePanels.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ParticleEditor.rc2
# End Source File
# End Group
# End Target
# End Project
