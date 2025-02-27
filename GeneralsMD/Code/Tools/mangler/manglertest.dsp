# Microsoft Developer Studio Project File - Name="manglertest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=manglertest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "manglertest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "manglertest.mak" CFG="manglertest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "manglertest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "manglertest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "manglertest"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "manglertest - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "wlib" /I "wnet" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "manglertest - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /I "wlib" /I "wnet" /I "." /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386

!ENDIF 

# Begin Target

# Name "manglertest - Win32 Release"
# Name "manglertest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "wnet"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wnet\field.cpp
# End Source File
# Begin Source File

SOURCE=.\wnet\Makefile
# End Source File
# Begin Source File

SOURCE=.\wnet\packet.cpp
# End Source File
# Begin Source File

SOURCE=.\wnet\tcp.cpp
# End Source File
# Begin Source File

SOURCE=.\wnet\udp.cpp
# End Source File
# End Group
# Begin Group "wlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wlib\configfile.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\critsec.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\Makefile
# End Source File
# Begin Source File

SOURCE=.\wlib\monod.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\sem4.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\streamer.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\syslogd.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\threadfac.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\timezone.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\wdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\wstring.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\wtime.cpp
# End Source File
# Begin Source File

SOURCE=.\wlib\xtime.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\manglertest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "wnet.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wnet\field.h
# End Source File
# Begin Source File

SOURCE=.\wnet\packet.h
# End Source File
# Begin Source File

SOURCE=.\wnet\tcp.h
# End Source File
# Begin Source File

SOURCE=.\wnet\udp.h
# End Source File
# End Group
# Begin Group "wlib.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wlib\arraylist.h
# End Source File
# Begin Source File

SOURCE=.\wlib\configfile.h
# End Source File
# Begin Source File

SOURCE=.\wlib\critsec.h
# End Source File
# Begin Source File

SOURCE=.\wlib\dictionary.h
# End Source File
# Begin Source File

SOURCE=.\wlib\filed.h
# End Source File
# Begin Source File

SOURCE=.\wlib\linkedlist.h
# End Source File
# Begin Source File

SOURCE=.\wlib\mboxd.h
# End Source File
# Begin Source File

SOURCE=.\wlib\monod.h
# End Source File
# Begin Source File

SOURCE=.\wlib\odevice.h
# End Source File
# Begin Source File

SOURCE=.\wlib\sem4.h
# End Source File
# Begin Source File

SOURCE=.\wlib\stderrd.h
# End Source File
# Begin Source File

SOURCE=.\wlib\stdoutd.h
# End Source File
# Begin Source File

SOURCE=.\wlib\streamer.h
# End Source File
# Begin Source File

SOURCE=.\wlib\syslogd.h
# End Source File
# Begin Source File

SOURCE=.\wlib\threadfac.h
# End Source File
# Begin Source File

SOURCE=.\wlib\threadsafe.h
# End Source File
# Begin Source File

SOURCE=.\wlib\timezone.h
# End Source File
# Begin Source File

SOURCE=.\wlib\ustring.h
# End Source File
# Begin Source File

SOURCE=.\wlib\wdebug.h
# End Source File
# Begin Source File

SOURCE=.\wlib\wstring.h
# End Source File
# Begin Source File

SOURCE=.\wlib\wstypes.h
# End Source File
# Begin Source File

SOURCE=.\wlib\wtime.h
# End Source File
# Begin Source File

SOURCE=.\wlib\xtime.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\endian.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\manglertest.cfg
# End Source File
# Begin Source File

SOURCE=.\manglertest.log
# End Source File
# End Target
# End Project
