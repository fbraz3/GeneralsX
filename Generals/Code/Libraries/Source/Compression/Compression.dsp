# Microsoft Developer Studio Project File - Name="Compression" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Compression - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Compression.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Compression.mak" CFG="Compression - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Compression - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Compression - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "Compression - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Compression"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Compression - Win32 Release"

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
# ADD CPP /nologo /W3 /WX /GX /O2 /Ob2 /I "..\..\include" /D "NDEBUG" /D "_RELEASE" /D "WIN32" /D "_MBCS" /D "_LIB" /D "Z_PREFIX" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Libraries\Lib\Compression.lib"

!ELSEIF  "$(CFG)" == "Compression - Win32 Debug"

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
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "Z_PREFIX" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Libraries\Lib\CompressionDebug.lib"

!ELSEIF  "$(CFG)" == "Compression - Win32 Internal"

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
# ADD CPP /nologo /W3 /WX /GX /O2 /I "..\..\include" /D "NDEBUG" /D "_INTERNAL" /D "WIN32" /D "_MBCS" /D "_LIB" /D "Z_PREFIX" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\..\Libraries\Lib\CompressionInternal.lib"

!ENDIF 

# Begin Target

# Name "Compression - Win32 Release"
# Name "Compression - Win32 Debug"
# Name "Compression - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ZLib\adler32.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\compress.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\crc32.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\deflate.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\gzio.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\infblock.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\infcodes.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\inffast.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\inflate.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\inftrees.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\infutil.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\maketree.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\trees.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\uncompr.c
# End Source File
# Begin Source File

SOURCE=.\ZLib\zutil.c
# End Source File
# End Group
# Begin Group "NoxLZH"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Hdec_g.tbl
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Hdec_s.tbl
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Hdisp.tbl
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Henc.tbl
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Huff.cpp
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Lz.cpp
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Lzhl.cpp
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibSource\Lzhl_tcp.cpp
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\NoxCompress.cpp
# End Source File
# End Group
# Begin Group "EAC"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EAC\btreeabout.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\btreedecode.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\btreeencode.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\huffabout.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\huffdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\huffencode.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\refabout.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\refdecode.cpp
# End Source File
# Begin Source File

SOURCE=.\EAC\refencode.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\CompressionManager.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Zlib.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ZLib\deflate.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\infblock.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\inffast.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\infutil.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\trees.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\zconf.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\zlib.h
# End Source File
# Begin Source File

SOURCE=.\ZLib\zutil.h
# End Source File
# End Group
# Begin Group "NoxLZH.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\LZHCompress\CompLibHeader\_huff.h
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibHeader\_lz.h
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibHeader\_lzhl.h
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibHeader\Lzhl.h
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\CompLibHeader\Lzhl_tcp.h
# End Source File
# Begin Source File

SOURCE=.\LZHCompress\NoxCompress.h
# End Source File
# End Group
# Begin Group "EAC.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\EAC\btreecodex.h
# End Source File
# Begin Source File

SOURCE=.\EAC\codex.h
# End Source File
# Begin Source File

SOURCE=.\EAC\gimex.h
# End Source File
# Begin Source File

SOURCE=.\EAC\huffcodex.h
# End Source File
# Begin Source File

SOURCE=.\EAC\refcodex.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Compression.h
# End Source File
# End Group
# End Target
# End Project
