# Microsoft Developer Studio Project File - Name="wwdebug" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wwdebug - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwdebug.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwdebug.mak" CFG="wwdebug - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwdebug - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wwdebug - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "wwdebug - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "wwdebug - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "wwdebug - Win32 DebugW3D" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/wwdebug", OPNAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwdebug - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /O2 /Ob2 /I "..\wwlib" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "NDEBUG" /D "WIN32" /D "IG_DEBUG_STACKTRACE" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWDebug.lib"

!ELSEIF  "$(CFG)" == "wwdebug - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gi /Zi /Od /Ob2 /I "..\wwlib" /D "_DEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "WWDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWDebugDebug.lib"

!ELSEIF  "$(CFG)" == "wwdebug - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wwdebug_"
# PROP BASE Intermediate_Dir "wwdebug_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /Zi /O2 /Op /Ob2 /I "..\wwlib" /I "$(ProjDir)/DXSDK\INCLUDE" /I "../STLPORT" /I "../DXSDK/INCLUDE" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_PROFILE" /YX /FD /Gh /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\release\wwdebug.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWDebugProfile.lib"

!ELSEIF  "$(CFG)" == "wwdebug - Win32 Internal"

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
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\wwlib" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /Zi /O2 /I "..\wwlib" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_INTERNAL" /D "WWDEBUG" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWDebug.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWDebugInternal.lib"

!ELSEIF  "$(CFG)" == "wwdebug - Win32 DebugW3D"

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
# ADD BASE CPP /nologo /MDd /W3 /WX /Gm /Gi /GR /GX /ZI /Od /I "..\wwlib" /D "_DEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /Fr /YX /FD /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gi /GX /Zi /O2 /I "..\wwlib" /D "_DEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "WWDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\Libs\Max4Release\wwdebug.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWDebugDebugW3D.lib"

!ENDIF 

# Begin Target

# Name "wwdebug - Win32 Release"
# Name "wwdebug - Win32 Debug"
# Name "wwdebug - Win32 Profile"
# Name "wwdebug - Win32 Internal"
# Name "wwdebug - Win32 DebugW3D"
# Begin Group "Source"

# PROP Default_Filter "cpp;c"
# Begin Source File

SOURCE=.\wwdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\wwmemlog.cpp
# End Source File
# Begin Source File

SOURCE=.\wwprofile.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\wwdebug.h
# End Source File
# Begin Source File

SOURCE=.\wwhack.h
# End Source File
# Begin Source File

SOURCE=.\wwmemlog.h
# End Source File
# Begin Source File

SOURCE=.\wwprofile.h
# End Source File
# End Group
# End Target
# End Project
