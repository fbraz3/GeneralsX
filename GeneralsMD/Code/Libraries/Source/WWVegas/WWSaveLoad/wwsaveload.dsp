# Microsoft Developer Studio Project File - Name="wwsaveload" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wwsaveload - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwsaveload.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwsaveload.mak" CFG="wwsaveload - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwsaveload - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 ProfileE" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 DebugE" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "wwsaveload - Win32 DebugW3D" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/wwsaveload", RPFCAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwsaveload - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D WINVER=0x400 /D "_WINDOWS" /D "NDEBUG" /D "WIN32" /D "IG_DEBUG_STACKTRACE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoad.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Zi /Od /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_DEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /D "WWDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadDebug.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /O2 /Op /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /I "$(ProjDir)/DXSDK\INCLUDE" /I "../STLPORT" /I "../DXSDK/INCLUDE" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /D "_PROFILE" /YX /FD /Gh /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadProfile.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 ProfileE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ProfileE"
# PROP BASE Intermediate_Dir "ProfileE"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ProfileE"
# PROP Intermediate_Dir "ProfileE"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /Zi /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /WX /Gm /GX /Zi /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /I "$(ProjDir)/DXSDK\INCLUDE" /I "../STLPORT" /I "../DXSDK/INCLUDE" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /D "PARAM_EDITING_ON" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\profile\wwsaveload.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadProfileE.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 DebugE"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /WX /Gm /GX /ZI /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /I "$(ProjDir)/DXSDK\INCLUDE" /I "../STLPORT" /I "../DXSDK/INCLUDE" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /D "PARAM_EDITING_ON" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\debug\wwsaveload.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadDebugE.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 Internal"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /O2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /D "_INTERNAL" /D "WWDEBUG" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoad.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadInternal.lib"

!ELSEIF  "$(CFG)" == "wwsaveload - Win32 DebugW3D"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Max4Release"
# PROP BASE Intermediate_Dir "Max4Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "DebugW3D"
# PROP Intermediate_Dir "DebugW3D"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /WX /Gm /GR /GX /ZI /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /WX /Gm /GX /ZI /Od /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /Fr # SUBTRACT CPP
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\release\wwsaveload.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWSaveLoadDebugW3D.lib"

!ENDIF 

# Begin Target

# Name "wwsaveload - Win32 Release"
# Name "wwsaveload - Win32 Debug"
# Name "wwsaveload - Win32 Profile"
# Name "wwsaveload - Win32 ProfileE"
# Name "wwsaveload - Win32 DebugE"
# Name "wwsaveload - Win32 Internal"
# Name "wwsaveload - Win32 DebugW3D"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\definition.cpp
# End Source File
# Begin Source File

SOURCE=.\definitionfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\definitionfactorymgr.cpp
# End Source File
# Begin Source File

SOURCE=.\definitionmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\parameter.cpp
# End Source File
# Begin Source File

SOURCE=.\persistfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\pointerremap.cpp
# End Source File
# Begin Source File

SOURCE=.\saveload.cpp
# End Source File
# Begin Source File

SOURCE=.\saveloadstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\saveloadsubsystem.cpp
# End Source File
# Begin Source File

SOURCE=.\twiddler.cpp
# End Source File
# Begin Source File

SOURCE=.\wwsaveload.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\definition.h
# End Source File
# Begin Source File

SOURCE=.\definitionclassids.h
# End Source File
# Begin Source File

SOURCE=.\definitionfactory.h
# End Source File
# Begin Source File

SOURCE=.\definitionfactorymgr.h
# End Source File
# Begin Source File

SOURCE=.\definitionmgr.h
# End Source File
# Begin Source File

SOURCE=.\editable.h
# End Source File
# Begin Source File

SOURCE=.\parameter.h
# End Source File
# Begin Source File

SOURCE=.\parameterlist.h
# End Source File
# Begin Source File

SOURCE=.\parametertypes.h
# End Source File
# Begin Source File

SOURCE=.\persist.h
# End Source File
# Begin Source File

SOURCE=.\persistfactory.h
# End Source File
# Begin Source File

SOURCE=.\pointerremap.h
# End Source File
# Begin Source File

SOURCE=.\postloadable.h
# End Source File
# Begin Source File

SOURCE=.\saveload.h
# End Source File
# Begin Source File

SOURCE=.\saveloadids.h
# End Source File
# Begin Source File

SOURCE=.\saveloadstatus.h
# End Source File
# Begin Source File

SOURCE=.\saveloadstatus.h
# End Source File
# Begin Source File

SOURCE=.\saveloadsubsystem.h
# End Source File
# Begin Source File

SOURCE=.\simpledefinitionfactory.h
# End Source File
# Begin Source File

SOURCE=.\simpleparameter.h
# End Source File
# Begin Source File

SOURCE=.\twiddler.h
# End Source File
# Begin Source File

SOURCE=.\wwsaveload.h
# End Source File
# End Group
# End Target
# End Project
