# Microsoft Developer Studio Project File - Name="ImagePacker" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ImagePacker - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ImagePacker.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ImagePacker.mak" CFG="ImagePacker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ImagePacker - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ImagePacker - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "ImagePacker - Win32 Internal" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ImagePacker"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ImagePacker - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 WW3D2.lib WWDebug.lib WWLib.lib WWUtil.lib WWMath.lib GameEngine.lib GameEngineDevice.lib Benchmark.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\ImagePacker.pdb" /map:"..\..\..\Run\ImagePacker.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\ImagePacker.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"

!ELSEIF  "$(CFG)" == "ImagePacker - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 WW3D2Debug.lib WWDebugDebug.lib WWUtilDebug.lib WWLibDebug.lib WWMathDebug.lib GameEngineDebug.lib GameEngineDeviceDebug.lib BenchmarkD.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\ImagePackerD.pdb" /map:"..\..\..\Run\ImagePackerD.map" /debug /machine:I386 /nodefaultlib:"libcd.lib" /out:"..\..\..\Run\ImagePackerD.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ImagePacker - Win32 Internal"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Internal"
# PROP BASE Intermediate_Dir "Internal"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Internal"
# PROP Intermediate_Dir "Internal"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /WX /GX /O2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 WW3D2.lib WWDebug.lib WWLib.lib WWUtil.lib WWMath.lib GameEngine.lib GameEngineDevice.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\ImagePacker.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# ADD LINK32 WW3D2Internal.lib WWDebugInternal.lib WWLibInternal.lib WWUtilInternal.lib WWMathInternal.lib GameEngineInternal.lib GameEngineDeviceInternal.lib Benchmark.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib /nologo /subsystem:windows /map:"..\Run\ImagePacker.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\ImagePacker.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ImagePacker - Win32 Release"
# Name "ImagePacker - Win32 Debug"
# Name "ImagePacker - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Window Procedures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Source\Window Procedures\DirectorySelect.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Window Procedures\ImageErrorProc.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Window Procedures\ImagePackerProc.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Window Procedures\PageErrorProc.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Window Procedures\PreviewProc.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\ImageInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ImagePacker.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\TexturePage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Include\ImageDirectory.h
# End Source File
# Begin Source File

SOURCE=.\Include\ImageInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\ImagePacker.h
# End Source File
# Begin Source File

SOURCE=.\Include\TexturePage.h
# End Source File
# Begin Source File

SOURCE=.\Include\WindowProc.h
# End Source File
# Begin Source File

SOURCE=.\Include\WinMain.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resource\ImagePacker.rc
# End Source File
# Begin Source File

SOURCE=.\Resource\Resource.h
# End Source File
# End Group
# End Target
# End Project
