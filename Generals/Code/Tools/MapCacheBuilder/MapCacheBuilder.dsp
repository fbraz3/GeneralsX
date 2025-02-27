# Microsoft Developer Studio Project File - Name="MapCacheBuilder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MapCacheBuilder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MapCacheBuilder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MapCacheBuilder.mak" CFG="MapCacheBuilder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MapCacheBuilder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MapCacheBuilder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "MapCacheBuilder - Win32 Internal" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "MapCacheBuilder"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MapCacheBuilder - Win32 Release"

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
# ADD LINK32 WW3D2.lib WWDebug.lib WWLib.lib WWUtil.lib WWMath.lib GameEngine.lib GameEngineDevice.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib mss32.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\MapCacheBuilder.pdb" /map:"..\..\..\Run\MapCacheBuilder.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\MapCacheBuilder.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MapCacheBuilder - Win32 Debug"

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
# ADD LINK32 WW3D2Debug.lib WWDebugDebug.lib WWUtilDebug.lib WWLibDebug.lib WWMathDebug.lib GameEngineDebug.lib GameEngineDeviceDebug.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\MapCacheBuilderD.pdb" /map:"..\..\..\Run\MapCacheBuilderD.map" /debug /machine:I386 /nodefaultlib:"libcd msvcrt" /out:"..\..\..\Run\MapCacheBuilderD.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MapCacheBuilder - Win32 Internal"

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
# ADD LINK32 Benchmark.lib WW3D2Internal.lib WWDebugInternal.lib WWLibInternal.lib WWUtilInternal.lib WWMathInternal.lib GameEngineInternal.lib GameEngineDeviceInternal.lib wsock32.lib wininet.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib mss32.lib /nologo /subsystem:windows /incremental:yes /pdb:"..\..\..\Run\MapCacheBuilderI.pdb" /map:"..\..\..\Run\MapCacheBuilderI.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\MapCacheBuilderI.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "MapCacheBuilder - Win32 Release"
# Name "MapCacheBuilder - Win32 Debug"
# Name "MapCacheBuilder - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Source\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resource\MapCacheBuilder.rc
# End Source File
# Begin Source File

SOURCE=.\Resource\Resource.h
# End Source File
# End Group
# End Target
# End Project
