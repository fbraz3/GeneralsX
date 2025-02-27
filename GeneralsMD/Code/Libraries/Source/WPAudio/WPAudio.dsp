# Microsoft Developer Studio Project File - Name="WPAudio" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=WPAudio - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WPAudio.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WPAudio.mak" CFG="WPAudio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WPAudio - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "WPAudio - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "WPAudio - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "WPAudio"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WPAudio - Win32 Release"

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
# ADD CPP /nologo /W3 /WX /GX /O2 /Ob2 /I "..\..\include" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "WPAudio - Win32 Debug"

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
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "WPAudio - Win32 Internal"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /WX /GX /O2 /Ob1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_INTERNAL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "WPAudio - Win32 Release"
# Name "WPAudio - Win32 Debug"
# Name "WPAudio - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AUD_Assert.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Attributes.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Cache.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Channel.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Device.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_DSoundDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Events.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Level.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_List.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Lock.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Memory.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Profiler.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Source.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_StreamBuffering.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Streamer.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Time.cpp
# End Source File
# Begin Source File

SOURCE=.\AUD_Windows.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Include\WPAudio\Altypes.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Attributes.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Cache.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Channel.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Debug.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Defs.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Device.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Driver.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Errors.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Events.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Level.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\List.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Lock.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Memory.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Profiler.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Search.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Source.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\StreamBuffering.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Streamer.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Time.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\WPAudio\Win32.h
# End Source File
# End Group
# Begin Group "Asimp3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Asimp3\Asimp3asm.asm

!IF  "$(CFG)" == "WPAudio - Win32 Release"

# Begin Custom Build
InputDir=.\Asimp3
OutDir=.\Release
InputPath=.\Asimp3\Asimp3asm.asm

BuildCmds= \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /Fo$(OutDir)\asi_x86.obj $(InputPath) \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /DAMD /Fo$(OutDir)\asi_amd.obj $(InputPath) \
	

"$(OutDir)\asi_x86.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\asi_amd.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "WPAudio - Win32 Debug"

# Begin Custom Build
InputDir=.\Asimp3
OutDir=.\Debug
InputPath=.\Asimp3\Asimp3asm.asm

BuildCmds= \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /Fo$(OutDir)\asi_x86.obj $(InputPath) \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /DAMD /Fo$(OutDir)\asi_amd.obj $(InputPath) \
	

"$(OutDir)\asi_x86.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\asi_amd.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "WPAudio - Win32 Internal"

# Begin Custom Build
InputDir=.\Asimp3
OutDir=.\Internal
InputPath=.\Asimp3\Asimp3asm.asm

BuildCmds= \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /Fo$(OutDir)\asi_x86.obj $(InputPath) \
	$(InputDir)\bin\ml /nologo /coff /Cp /Zm /c /W2 /Zi /DAMD /Fo$(OutDir)\asi_amd.obj $(InputPath) \
	

"$(OutDir)\asi_x86.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(OutDir)\asi_amd.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Asimp3\Datatbl.h
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Imssapi.h
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Mp3api.cpp
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Mp3dec.cpp
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Mp3dec.h
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Mss.h
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Mssasi.h
# End Source File
# Begin Source File

SOURCE=.\Asimp3\Rib.h
# End Source File
# End Group
# End Target
# End Project
