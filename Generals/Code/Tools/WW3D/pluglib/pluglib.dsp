# Microsoft Developer Studio Project File - Name="pluglib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pluglib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pluglib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pluglib.mak" CFG="pluglib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pluglib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pluglib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pluglib - Win32 Hybrid" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/Tools/pluglib", TSJAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pluglib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "D:\3dsmax4\maxsdk\Include" /I "D:\3dsmax4\maxsdk\Include\Maxscrpt" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /YX /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /D "_DEBUG" /D "_WINDOWS" /YX /FD "WIN32" /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Debug\pluglibd.lib"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pluglib_"
# PROP BASE Intermediate_Dir "pluglib_"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Hybrid"
# PROP Intermediate_Dir "Hybrid"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Z7 /Od /I "D:\3dsmax4\maxsdk\Include" /I "D:\3dsmax4\maxsdk\Include\Maxscrpt" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:".\Lib\pluglib.lib"
# ADD LIB32 /nologo /out:".\Hybrid\pluglib.lib"

!ENDIF 

# Begin Target

# Name "pluglib - Win32 Release"
# Name "pluglib - Win32 Debug"
# Name "pluglib - Win32 Hybrid"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\chunkio.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EULER.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HSV.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\JShell.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\matrix3d.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\matrix4.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nodefilt.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nodelist.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PALETTE.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RAWFILE.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\realcrc.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RGB.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Vector.CPP

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\w3dquat.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wwmath.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WWmatrix3.cpp

!IF  "$(CFG)" == "pluglib - Win32 Release"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Debug"

!ELSEIF  "$(CFG)" == "pluglib - Win32 Hybrid"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\aaplane.h
# End Source File
# Begin Source File

SOURCE=.\always.h
# End Source File
# Begin Source File

SOURCE=.\bool.h
# End Source File
# Begin Source File

SOURCE=.\borlandc.h
# End Source File
# Begin Source File

SOURCE=.\chunkio.h
# End Source File
# Begin Source File

SOURCE=.\errclass.h
# End Source File
# Begin Source File

SOURCE=.\EULER.H
# End Source File
# Begin Source File

SOURCE=.\hsv.h
# End Source File
# Begin Source File

SOURCE=.\iostruct.h
# End Source File
# Begin Source File

SOURCE=.\matrix3d.h
# End Source File
# Begin Source File

SOURCE=.\matrix4.h
# End Source File
# Begin Source File

SOURCE=.\nodefilt.h
# End Source File
# Begin Source File

SOURCE=.\nodelist.h
# End Source File
# Begin Source File

SOURCE=.\palette.h
# End Source File
# Begin Source File

SOURCE=.\PROGRESS.H
# End Source File
# Begin Source File

SOURCE=.\rawfile.h
# End Source File
# Begin Source File

SOURCE=.\realcrc.h
# End Source File
# Begin Source File

SOURCE=.\rgb.h
# End Source File
# Begin Source File

SOURCE=.\Vector.H
# End Source File
# Begin Source File

SOURCE=.\vector2.h
# End Source File
# Begin Source File

SOURCE=.\vector3i.h
# End Source File
# Begin Source File

SOURCE=.\visualc.h
# End Source File
# Begin Source File

SOURCE=.\w3dquat.h
# End Source File
# Begin Source File

SOURCE=.\watcom.h
# End Source File
# Begin Source File

SOURCE=.\win.h
# End Source File
# Begin Source File

SOURCE=.\wwfile.h
# End Source File
# Begin Source File

SOURCE=.\wwmath.h
# End Source File
# Begin Source File

SOURCE=.\WWmatrix3.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
