# Microsoft Developer Studio Project File - Name="Autorun English" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Autorun English - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Autorun English.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Autorun English.mak" CFG="Autorun English - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Autorun English - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Autorun English - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Renegade Setup/Autorun", SJEEAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Autorun English - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /GX /O2 /I "../../Libraries/Include/" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release/Autorun English.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib wsock32.lib imm32.lib wininet.lib vfw32.lib /nologo /subsystem:windows /machine:I386 /out:".\Release\Autorun.exe"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "Autorun English - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /W3 /Gm /GX /ZI /Od /I "../../Libraries/Include/" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Debug/Autorun English.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 
# ADD LINK32 dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib imm32.lib wininet.lib vfw32.lib /nologo /subsystem:windows /debug /machine:I386 
# SUBTRACT LINK32 

!ENDIF 

# Begin Target

# Name "Autorun English - Win32 Release"
# Name "Autorun English - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ARGS.CPP
# End Source File
# Begin Source File

SOURCE=.\ARGS.H
# End Source File
# Begin Source File

SOURCE=.\autorun.cpp
# End Source File
# Begin Source File

SOURCE=.\autorun.h
# End Source File
# Begin Source File

SOURCE=.\CallbackHook.h
# End Source File
# Begin Source File

SOURCE=.\CDCNTRL.CPP
# End Source File
# Begin Source File

SOURCE=.\CDCNTRL.H
# End Source File
# Begin Source File

SOURCE=.\DrawButton.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawButton.h
# End Source File
# Begin Source File

SOURCE=.\EZGIMEX.cpp
# End Source File
# Begin Source File

SOURCE=.\GameText.cpp
# End Source File
# Begin Source File

SOURCE=.\GameText.h
# End Source File
# Begin Source File

SOURCE=.\GETCD.CPP
# End Source File
# Begin Source File

SOURCE=.\GetCD.h
# End Source File
# Begin Source File

SOURCE=.\gimex.h
# End Source File
# Begin Source File

SOURCE=.\IGR.cpp
# End Source File
# Begin Source File

SOURCE=.\IGR.h
# End Source File
# Begin Source File

SOURCE=.\Jsupport.cpp
# End Source File
# Begin Source File

SOURCE=.\JSUPPORT.H
# End Source File
# Begin Source File

SOURCE=.\leanAndMeanAutorun.h
# End Source File
# Begin Source File

SOURCE=.\locale.cpp
# End Source File
# Begin Source File

SOURCE=.\locale.h
# End Source File
# Begin Source File

SOURCE=.\Locale_API.cpp
# End Source File
# Begin Source File

SOURCE=.\Locale_API.h
# End Source File
# Begin Source File

SOURCE=.\POINT.h
# End Source File
# Begin Source File

SOURCE=.\RECT.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\TTFont.cpp
# End Source File
# Begin Source File

SOURCE=.\TTFont.h
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\ViewHTML.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewHTML.h
# End Source File
# Begin Source File

SOURCE=.\WinFix.CPP
# End Source File
# Begin Source File

SOURCE=.\WinFix.H
# End Source File
# Begin Source File

SOURCE=.\wnd_file.cpp
# End Source File
# Begin Source File

SOURCE=.\Wnd_File.h
# End Source File
# Begin Source File

SOURCE=.\WSYS_File.cpp
# End Source File
# Begin Source File

SOURCE=.\WSYS_file.h
# End Source File
# Begin Source File

SOURCE=.\WSYS_FileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\WSYS_FileSystem.h
# End Source File
# Begin Source File

SOURCE=.\WSYS_RAMFile.cpp
# End Source File
# Begin Source File

SOURCE=.\WSYS_RAMFile.h
# End Source File
# Begin Source File

SOURCE=.\WSYS_StdFile.cpp
# End Source File
# Begin Source File

SOURCE=.\WSYS_StdFile.h
# End Source File
# Begin Source File

SOURCE=.\WSYS_StdFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\WSYS_StdFileSystem.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\Autorun.ICO
# End Source File
# Begin Source File

SOURCE=.\AUTORUN.RC
# End Source File
# End Group
# Begin Source File

SOURCE=.\ARButton_Reg.bmp
# End Source File
# Begin Source File

SOURCE=.\ARButton_Sel.bmp
# End Source File
# Begin Source File

SOURCE=..\arrow.cur
# End Source File
# Begin Source File

SOURCE=..\autorun.ico
# End Source File
# Begin Source File

SOURCE=.\Autorun_BG.bmp
# End Source File
# Begin Source File

SOURCE=.\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=".\License French.bmp"
# End Source File
# Begin Source File

SOURCE=".\License French2.bmp"
# End Source File
# Begin Source File

SOURCE=".\License German.bmp"
# End Source File
# Begin Source File

SOURCE=".\License German2.bmp"
# End Source File
# Begin Source File

SOURCE=".\License USA.bmp"
# End Source File
# Begin Source File

SOURCE=".\License USA2.bmp"
# End Source File
# Begin Source File

SOURCE=.\mouse.wav
# End Source File
# Begin Source File

SOURCE=.\English\Present.wav
# End Source File
# Begin Source File

SOURCE=".\English\Rock-n-Roll.wav"
# End Source File
# End Target
# End Project
