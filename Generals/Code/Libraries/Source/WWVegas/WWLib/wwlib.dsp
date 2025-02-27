# Microsoft Developer Studio Project File - Name="wwlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wwlib - Win32 DebugW3D
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wwlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wwlib.mak" CFG="wwlib - Win32 DebugW3D"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wwlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wwlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "wwlib - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE "wwlib - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Commando/Code/wwlib", KOGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wwlib - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /O2 /Ob2 /I "..\wwdebug" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "NDEBUG" /D "WIN32" /D "IG_DEBUG_STACKTRACE" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWLib.lib"

!ELSEIF  "$(CFG)" == "wwlib - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gi /GX /Zi /O2 /Ob2 /I "..\wwdebug" /D "_DEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "REGEX_MALLOC" /D "STDC_HEADERS" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWLibdebug.lib"

!ELSEIF  "$(CFG)" == "wwlib - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wwlib___"
# PROP BASE Intermediate_Dir "wwlib___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /WX /GX /Zi /O2 /Op /Ob2 /I "..\wwdebug" /D "NDEBUG" /D "WWDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "REGEX_MALLOC" /D "STDC_HEADERS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWLibProfile.lib"

!ELSEIF  "$(CFG)" == "wwlib - Win32 Internal"

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
# ADD BASE CPP /nologo /MD /W4 /Gi /GX /O2 /Ob2 /I "..\wwdebug" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "REGEX_MALLOC" /D "STDC_HEADERS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MD /W3 /WX /Gi /GX /Zi /O2 /I "..\wwdebug" /D "NDEBUG" /D WINVER=0x400 /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "_INTERNAL" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\..\Lib\WWLib.lib"
# ADD LIB32 /nologo /out:"..\..\..\Lib\WWLibInternal.lib"

!ENDIF 

# Begin Target

# Name "wwlib - Win32 Release"
# Name "wwlib - Win32 Debug"
# Name "wwlib - Win32 Profile"
# Name "wwlib - Win32 Internal"
# Begin Group "Source"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\_convert.cpp
# End Source File
# Begin Source File

SOURCE=.\_mono.cpp
# End Source File
# Begin Source File

SOURCE=.\b64pipe.cpp
# End Source File
# Begin Source File

SOURCE=.\b64straw.cpp
# End Source File
# Begin Source File

SOURCE=.\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\blit.cpp
# End Source File
# Begin Source File

SOURCE=.\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\buff.cpp
# End Source File
# Begin Source File

SOURCE=.\bufffile.cpp
# End Source File
# Begin Source File

SOURCE=.\chunkio.cpp
# End Source File
# Begin Source File

SOURCE=.\convert.cpp
# End Source File
# Begin Source File

SOURCE=.\cpudetect.cpp
# End Source File
# Begin Source File

SOURCE=.\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\cstraw.cpp
# End Source File
# Begin Source File

SOURCE=.\data.cpp
# End Source File
# Begin Source File

SOURCE=.\ddraw.cpp
# End Source File
# Begin Source File

SOURCE=.\dsurface.cpp
# End Source File
# Begin Source File

SOURCE=.\ffactory.cpp
# End Source File
# Begin Source File

SOURCE=.\gcd_lcm.cpp
# End Source File
# Begin Source File

SOURCE=.\hash.cpp
# End Source File
# Begin Source File

SOURCE=.\hsv.cpp
# End Source File
# Begin Source File

SOURCE=.\ini.cpp
# End Source File
# Begin Source File

SOURCE=.\int.cpp
# End Source File
# Begin Source File

SOURCE=.\jshell.cpp
# End Source File
# Begin Source File

SOURCE=.\keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\LaunchWeb.cpp
# End Source File
# Begin Source File

SOURCE=.\lcw.cpp
# End Source File
# Begin Source File

SOURCE=.\load.cpp
# End Source File
# Begin Source File

SOURCE=.\mono.cpp
# End Source File
# Begin Source File

SOURCE=.\mpmath.cpp
# End Source File
# Begin Source File

SOURCE=.\mpu.cpp
# End Source File
# Begin Source File

SOURCE=.\msgloop.cpp
# End Source File
# Begin Source File

SOURCE=.\multilist.cpp
# End Source File
# Begin Source File

SOURCE=.\mutex.cpp
# End Source File
# Begin Source File

SOURCE=.\nstrdup.cpp
# End Source File
# Begin Source File

SOURCE=.\palette.cpp
# End Source File
# Begin Source File

SOURCE=.\pcx.cpp
# End Source File
# Begin Source File

SOURCE=.\pipe.cpp
# End Source File
# Begin Source File

SOURCE=.\pk.cpp
# End Source File
# Begin Source File

SOURCE=.\ramfile.cpp
# End Source File
# Begin Source File

SOURCE=.\random.cpp
# End Source File
# Begin Source File

SOURCE=.\rawfile.cpp
# End Source File
# Begin Source File

SOURCE=.\rc4.cpp
# End Source File
# Begin Source File

SOURCE=.\rcfile.cpp
# End Source File
# Begin Source File

SOURCE=.\readline.cpp
# End Source File
# Begin Source File

SOURCE=.\realcrc.cpp
# End Source File
# Begin Source File

SOURCE=.\refcount.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\rgb.cpp
# End Source File
# Begin Source File

SOURCE=.\rle.cpp
# End Source File
# Begin Source File

SOURCE=.\rndstrng.cpp
# End Source File
# Begin Source File

SOURCE=.\sampler.cpp
# End Source File
# Begin Source File

SOURCE=.\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\slnode.cpp
# End Source File
# Begin Source File

SOURCE=.\srandom.cpp
# End Source File
# Begin Source File

SOURCE=.\stimer.cpp
# End Source File
# Begin Source File

SOURCE=.\straw.cpp
# End Source File
# Begin Source File

SOURCE=.\strtok_r.cpp
# End Source File
# Begin Source File

SOURCE=.\systimer.cpp
# End Source File
# Begin Source File

SOURCE=.\tagblock.cpp
# End Source File
# Begin Source File

SOURCE=.\TARGA.CPP
# End Source File
# Begin Source File

SOURCE=.\textfile.cpp
# End Source File
# Begin Source File

SOURCE=.\thread.cpp
# End Source File
# Begin Source File

SOURCE=.\trim.cpp
# End Source File
# Begin Source File

SOURCE=.\vector.cpp
# End Source File
# Begin Source File

SOURCE=.\widestring.cpp
# End Source File
# Begin Source File

SOURCE=.\win.cpp
# End Source File
# Begin Source File

SOURCE=.\WWCOMUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\wwfile.cpp
# End Source File
# Begin Source File

SOURCE=.\wwfont.cpp
# End Source File
# Begin Source File

SOURCE=.\wwstring.cpp
# End Source File
# Begin Source File

SOURCE=.\xpipe.cpp
# End Source File
# Begin Source File

SOURCE=.\xstraw.cpp
# End Source File
# Begin Source File

SOURCE=.\xsurface.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\_mono.h
# End Source File
# Begin Source File

SOURCE=.\always.h
# End Source File
# Begin Source File

SOURCE=.\argv.h
# End Source File
# Begin Source File

SOURCE=.\b64pipe.h
# End Source File
# Begin Source File

SOURCE=.\b64straw.h
# End Source File
# Begin Source File

SOURCE=.\base64.h
# End Source File
# Begin Source File

SOURCE=.\binheap.h
# End Source File
# Begin Source File

SOURCE=.\bittype.h
# End Source File
# Begin Source File

SOURCE=.\blit.h
# End Source File
# Begin Source File

SOURCE=.\blitter.h
# End Source File
# Begin Source File

SOURCE=.\bool.h
# End Source File
# Begin Source File

SOURCE=.\borlandc.h
# End Source File
# Begin Source File

SOURCE=.\bound.h
# End Source File
# Begin Source File

SOURCE=.\BSEARCH.H
# End Source File
# Begin Source File

SOURCE=.\bsurface.h
# End Source File
# Begin Source File

SOURCE=.\BUFF.H
# End Source File
# Begin Source File

SOURCE=.\bufffile.h
# End Source File
# Begin Source File

SOURCE=.\CallbackHook.h
# End Source File
# Begin Source File

SOURCE=.\chunkio.h
# End Source File
# Begin Source File

SOURCE=.\Convert.h
# End Source File
# Begin Source File

SOURCE=.\cpudetect.h
# End Source File
# Begin Source File

SOURCE=.\CRC.H
# End Source File
# Begin Source File

SOURCE=.\cstraw.h
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\dsurface.h
# End Source File
# Begin Source File

SOURCE=.\ffactory.h
# End Source File
# Begin Source File

SOURCE=.\FONT.H
# End Source File
# Begin Source File

SOURCE=.\gcd_lcm.h
# End Source File
# Begin Source File

SOURCE=.\growmix.h
# End Source File
# Begin Source File

SOURCE=.\hash.h
# End Source File
# Begin Source File

SOURCE=.\hashcalc.h
# End Source File
# Begin Source File

SOURCE=.\HASHLIST.H
# End Source File
# Begin Source File

SOURCE=.\hashtemplate.h
# End Source File
# Begin Source File

SOURCE=.\IFF.H
# End Source File
# Begin Source File

SOURCE=.\INDEX.H
# End Source File
# Begin Source File

SOURCE=.\INI.H
# End Source File
# Begin Source File

SOURCE=.\inisup.h
# End Source File
# Begin Source File

SOURCE=.\INT.H
# End Source File
# Begin Source File

SOURCE=.\iostruct.h
# End Source File
# Begin Source File

SOURCE=.\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\LaunchWeb.h
# End Source File
# Begin Source File

SOURCE=.\LCW.H
# End Source File
# Begin Source File

SOURCE=.\LISTNODE.H
# End Source File
# Begin Source File

SOURCE=.\mempool.h
# End Source File
# Begin Source File

SOURCE=.\mixfile.h
# End Source File
# Begin Source File

SOURCE=.\MONO.H
# End Source File
# Begin Source File

SOURCE=.\MONODRVR.H
# End Source File
# Begin Source File

SOURCE=.\MPMATH.H
# End Source File
# Begin Source File

SOURCE=.\MPU.H
# End Source File
# Begin Source File

SOURCE=.\msgloop.h
# End Source File
# Begin Source File

SOURCE=.\multilist.h
# End Source File
# Begin Source File

SOURCE=.\mutex.h
# End Source File
# Begin Source File

SOURCE=.\Notify.h
# End Source File
# Begin Source File

SOURCE=.\nstrdup.h
# End Source File
# Begin Source File

SOURCE=.\ntree.h
# End Source File
# Begin Source File

SOURCE=.\PALETTE.H
# End Source File
# Begin Source File

SOURCE=.\pcx.H
# End Source File
# Begin Source File

SOURCE=.\PIPE.H
# End Source File
# Begin Source File

SOURCE=.\PK.H
# End Source File
# Begin Source File

SOURCE=.\Point.h
# End Source File
# Begin Source File

SOURCE=.\RAMFILE.H
# End Source File
# Begin Source File

SOURCE=.\RANDOM.H
# End Source File
# Begin Source File

SOURCE=.\RAWFILE.H
# End Source File
# Begin Source File

SOURCE=.\rc4.h
# End Source File
# Begin Source File

SOURCE=.\rcfile.h
# End Source File
# Begin Source File

SOURCE=.\readline.h
# End Source File
# Begin Source File

SOURCE=.\realcrc.h
# End Source File
# Begin Source File

SOURCE=.\ref_ptr.h
# End Source File
# Begin Source File

SOURCE=.\refcount.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\RGB.H
# End Source File
# Begin Source File

SOURCE=.\RLE.H
# End Source File
# Begin Source File

SOURCE=.\RNDSTRAW.H
# End Source File
# Begin Source File

SOURCE=.\rndstrng.h
# End Source File
# Begin Source File

SOURCE=.\rng.h
# End Source File
# Begin Source File

SOURCE=.\rsacrypt.h
# End Source File
# Begin Source File

SOURCE=.\sampler.h
# End Source File
# Begin Source File

SOURCE=.\sha.h
# End Source File
# Begin Source File

SOURCE=.\shapeset.h
# End Source File
# Begin Source File

SOURCE=.\sharebuf.h
# End Source File
# Begin Source File

SOURCE=.\Signaler.h
# End Source File
# Begin Source File

SOURCE=.\simplevec.h
# End Source File
# Begin Source File

SOURCE=.\SLIST.H
# End Source File
# Begin Source File

SOURCE=.\SLNODE.H
# End Source File
# Begin Source File

SOURCE=.\srandom.h
# End Source File
# Begin Source File

SOURCE=.\stl.h
# End Source File
# Begin Source File

SOURCE=.\STRAW.H
# End Source File
# Begin Source File

SOURCE=.\strtok_r.h
# End Source File
# Begin Source File

SOURCE=.\Surface.h
# End Source File
# Begin Source File

SOURCE=.\surfrect.h
# End Source File
# Begin Source File

SOURCE=.\systimer.h
# End Source File
# Begin Source File

SOURCE=.\TagBlock.h
# End Source File
# Begin Source File

SOURCE=.\TARGA.H
# End Source File
# Begin Source File

SOURCE=.\textfile.h
# End Source File
# Begin Source File

SOURCE=.\thread.h
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\trackwin.h
# End Source File
# Begin Source File

SOURCE=.\trect.h
# End Source File
# Begin Source File

SOURCE=.\trim.h
# End Source File
# Begin Source File

SOURCE=.\uarray.h
# End Source File
# Begin Source File

SOURCE=.\Vector.H
# End Source File
# Begin Source File

SOURCE=.\verchk.h
# End Source File
# Begin Source File

SOURCE=.\visualc.h
# End Source File
# Begin Source File

SOURCE=.\WATCOM.H
# End Source File
# Begin Source File

SOURCE=.\widestring.h
# End Source File
# Begin Source File

SOURCE=.\win.h
# End Source File
# Begin Source File

SOURCE=.\WWCOMUtil.h
# End Source File
# Begin Source File

SOURCE=.\WWFILE.H
# End Source File
# Begin Source File

SOURCE=.\wwstring.h
# End Source File
# Begin Source File

SOURCE=.\xmouse.h
# End Source File
# Begin Source File

SOURCE=.\XPIPE.H
# End Source File
# Begin Source File

SOURCE=.\XSTRAW.H
# End Source File
# Begin Source File

SOURCE=.\xsurface.h
# End Source File
# End Group
# End Target
# End Project
