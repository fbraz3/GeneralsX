# Microsoft Developer Studio Project File - Name="wwutil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wwutil - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwutil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwutil.mak" CFG="wwutil - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwutil - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wwutil - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "wwutil - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "wwutil - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/wwutil", ACDBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwutil - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /O2 /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "NDEBUG" /D "WIN32" /D "IG_DEBUG_STACKTRACE" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWUtil.lib"

!ELSEIF  "$(CFG)" == "wwutil - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gi /GX /Zi /O2 /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWUtilDebug.lib"

!ELSEIF  "$(CFG)" == "wwutil - Win32 Profile"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /Zi /O2 /Op /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D "WWDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWUtilProfile.lib"

!ELSEIF  "$(CFG)" == "wwutil - Win32 Internal"

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
# ADD BASE CPP /nologo /MD /W4 /WX /Gi /GX /O2 /Ob2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /Zi /O2 /I "..\wwlib" /I "..\wwdebug" /I "..\wwmath" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "_INTERNAL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWUtil.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWUtilInternal.lib"

!ENDIF 

# Begin Target

# Name "wwutil - Win32 Release"
# Name "wwutil - Win32 Debug"
# Name "wwutil - Win32 Profile"
# Name "wwutil - Win32 Internal"
# Begin Source File

SOURCE=.\mathutil.cpp
# End Source File
# Begin Source File

SOURCE=.\mathutil.h
# End Source File
# Begin Source File

SOURCE=.\miscutil.cpp
# End Source File
# Begin Source File

SOURCE=.\miscutil.h
# End Source File
# End Target
# End Project
