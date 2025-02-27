# Microsoft Developer Studio Project File - Name="profile" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=profile - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "profile.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "profile.mak" CFG="profile - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "profile - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "profile - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "profile - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "profile - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "profile"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "profile - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy release\profile.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "profile - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"_pch.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\profiledebug.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy debug\profiledebug.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "profile - Win32 Internal"

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
# ADD BASE CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Oa /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GF /Gs /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_INTERNAL" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Internal\profileinternal.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy internal\profileinternal.lib ..\..\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "profile - Win32 Profile"

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
# ADD BASE CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Oa /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GF /Gs /c
# ADD CPP /nologo /G6 /MD /W3 /WX /Zi /Ot /Og /Oi /Oy /Ob2 /Gy /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_PROFILE" /Yu"_pch.h" /FD /GF /Gs /c
# SUBTRACT CPP /Oa /Ow
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Profile\profileprofile.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copying
PostBuild_Cmds=copy profile\profileprofile.lib ..\..\lib
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "profile - Win32 Release"
# Name "profile - Win32 Debug"
# Name "profile - Win32 Internal"
# Name "profile - Win32 Profile"
# Begin Group "Precompiled header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\_pch.cpp
# ADD CPP /Yc"_pch.h"
# End Source File
# Begin Source File

SOURCE=.\_pch.h
# End Source File
# End Group
# Begin Group "Doxygen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\compile_doxygen.bat
# End Source File
# Begin Source File

SOURCE=.\profile.dox
# End Source File
# Begin Source File

SOURCE=.\profile_priv.dox
# End Source File
# End Group
# Begin Group "Function level"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal_funclevel.h
# End Source File
# Begin Source File

SOURCE=.\profile_funclevel.cpp

!IF  "$(CFG)" == "profile - Win32 Release"

!ELSEIF  "$(CFG)" == "profile - Win32 Debug"

!ELSEIF  "$(CFG)" == "profile - Win32 Internal"

!ELSEIF  "$(CFG)" == "profile - Win32 Profile"

# ADD CPP /FAcs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\profile_funclevel.h
# End Source File
# End Group
# Begin Group "High level"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal_highlevel.h
# End Source File
# Begin Source File

SOURCE=.\profile_highlevel.cpp
# End Source File
# Begin Source File

SOURCE=.\profile_highlevel.h
# End Source File
# End Group
# Begin Group "Command interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal_cmd.h
# End Source File
# Begin Source File

SOURCE=.\profile_cmd.cpp

!IF  "$(CFG)" == "profile - Win32 Release"

!ELSEIF  "$(CFG)" == "profile - Win32 Debug"

!ELSEIF  "$(CFG)" == "profile - Win32 Internal"

!ELSEIF  "$(CFG)" == "profile - Win32 Profile"

# ADD CPP /FAcs
# SUBTRACT CPP /Oa /Ow

!ENDIF 

# End Source File
# End Group
# Begin Group "Result functions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\internal_result.h
# End Source File
# Begin Source File

SOURCE=.\profile_result.cpp
# End Source File
# Begin Source File

SOURCE=.\profile_result.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\internal.h
# End Source File
# Begin Source File

SOURCE=.\profile.cpp
# End Source File
# Begin Source File

SOURCE=.\profile.h
# End Source File
# Begin Source File

SOURCE=.\profile_doc.h
# End Source File
# End Target
# End Project
