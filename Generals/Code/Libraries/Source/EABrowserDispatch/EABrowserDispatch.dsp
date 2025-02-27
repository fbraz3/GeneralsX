# Microsoft Developer Studio Project File - Name="EABrowserDispatch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=EABrowserDispatch - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "EABrowserDispatch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EABrowserDispatch.mak" CFG="EABrowserDispatch - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EABrowserDispatch - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "EABrowserDispatch - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "EABrowserDispatch - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "EABrowserDispatch"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EABrowserDispatch - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "..\..\include" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\EABrowserDispatch.lib"

!ELSEIF  "$(CFG)" == "EABrowserDispatch - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\EABrowserDispatchD.lib"

!ELSEIF  "$(CFG)" == "EABrowserDispatch - Win32 Internal"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_INTERNAL" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\Lib\EABrowserDispatchI.lib"

!ENDIF 

# Begin Target

# Name "EABrowserDispatch - Win32 Release"
# Name "EABrowserDispatch - Win32 Debug"
# Name "EABrowserDispatch - Win32 Internal"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BrowserDispatch.idl

!IF  "$(CFG)" == "EABrowserDispatch - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Running MIDL on $(InputPath)
InputPath=.\BrowserDispatch.idl

BuildCmds= \
	midl.exe /out ..\..\Include\EABrowserDispatch /tlb "BrowserDispatch.tlb" /h "BrowserDispatch.h" /iid "BrowserDispatch_i.c" /mktyplib203 /win32 $(InputPath)

"..\..\Include\EABrowserDispatch\BrowserDispatch.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "EABrowserDispatch - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Running MIDL on $(InputPath)
InputPath=.\BrowserDispatch.idl

BuildCmds= \
	midl.exe /out ..\..\Include\EABrowserDispatch /tlb "BrowserDispatch.tlb" /h "BrowserDispatch.h" /iid "BrowserDispatch_i.c" /mktyplib203 /win32 $(InputPath)

"..\..\Include\EABrowserDispatch\BrowserDispatch.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "EABrowserDispatch - Win32 Internal"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Running MIDL on $(InputPath)
InputPath=.\BrowserDispatch.idl

BuildCmds= \
	midl.exe /out ..\..\Include\EABrowserDispatch /tlb "BrowserDispatch.tlb" /h "BrowserDispatch.h" /iid "BrowserDispatch_i.c" /mktyplib203 /win32 $(InputPath)

"..\..\Include\EABrowserDispatch\BrowserDispatch.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch_i.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"..\..\Include\EABrowserDispatch\BrowserDispatch.tlb" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Include\EABrowserDispatch\BrowserDispatch_i.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Include\EABrowserDispatch\BrowserDispatch.h
# End Source File
# End Group
# End Target
# End Project
