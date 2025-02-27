# Microsoft Developer Studio Project File - Name="GUIEdit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GUIEdit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GUIEdit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GUIEdit.mak" CFG="GUIEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GUIEdit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GUIEdit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "GUIEdit - Win32 Internal" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "GUIEdit"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GUIEdit - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /YX"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Benchmark.lib WW3D2.lib WWDebug.lib WWLib.lib WWUtil.lib WWMath.lib GameEngine.lib GameEngineDevice.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib wsock32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\GUIEdit.pdb" /map:"..\..\..\Run\GUIEdit.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\GUIEdit.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"

!ELSEIF  "$(CFG)" == "GUIEdit - Win32 Debug"

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
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 BenchmarkD.lib WW3D2Debug.lib WWDebugDebug.lib WWUtilDebug.lib WWLibDebug.lib WWMathDebug.lib GameEngineDebug.lib GameEngineDeviceDebug.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib wsock32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\GUIEditD.pdb" /map:"..\..\..\Run\GUIEditD.map" /debug /machine:I386 /nodefaultlib:"libcd.lib" /out:"..\..\..\Run\GUIEditD.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GUIEdit - Win32 Internal"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /I ".\Include" /I ".\Resource" /I "..\..\Libraries\Include" /I "..\..\GameEngine\Include" /I "..\..\GameEngineDevice\Include" /I "..\..\Libraries\Source\WWVegas" /I "..\..\Libraries\Source\WWVegas\WWLib" /I "..\..\Libraries\Source\WWVegas\WWMath" /I "..\..\Libraries\Source\WWVegas\WWDebug" /I "..\..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\..\Libraries\Source\WWVegas\WW3D2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_INTERNAL" /YX"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 WW3D2.lib WWDebug.lib WWLib.lib WWUtil.lib WWMath.lib GameEngine.lib GameEngineDevice.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\GUIEdit.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"
# ADD LINK32 Benchmark.lib WW3D2Internal.lib WWDebugInternal.lib WWLibInternal.lib WWUtilInternal.lib WWMathInternal.lib GameEngineInternal.lib GameEngineDeviceInternal.lib wsock32.lib dxguid.lib dinput8.lib d3dx8.lib d3d8.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vfw32.lib winmm.lib dsound.lib comctl32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\GUIEditI.pdb" /map:"..\..\..\Run\GUIEditI.map" /debug /machine:I386 /nodefaultlib:"libc.lib" /out:"..\..\..\Run\GUIEditI.exe" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine"

!ENDIF 

# Begin Target

# Name "GUIEdit - Win32 Release"
# Name "GUIEdit - Win32 Debug"
# Name "GUIEdit - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Dialog Procedures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Source\Dialog Procedures\CallbackEditor.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\CheckBoxProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\ColorDialog.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\ComboBoxProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\GenericProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\GridSettings.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\ListboxProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\NewLayoutDialog.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\ProgressBarProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\PushButtonProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\RadioButtonProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\SliderProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\StaticTextProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\TabControlProperties.cpp"
# End Source File
# Begin Source File

SOURCE=".\Source\Dialog Procedures\TextEntryProperties.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\EditWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GUIEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GUIEditDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GUIEditWindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\HierarchyView.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\LayoutScheme.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Save.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\WinMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Include\DialogProc.h
# End Source File
# Begin Source File

SOURCE=.\Include\EditWindow.h
# End Source File
# Begin Source File

SOURCE=.\Include\GUIEdit.h
# End Source File
# Begin Source File

SOURCE=.\Include\GUIEditColor.h
# End Source File
# Begin Source File

SOURCE=.\Include\GUIEditDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Include\GUIEditWindowManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\HierarchyView.h
# End Source File
# Begin Source File

SOURCE=.\Include\LayoutScheme.h
# End Source File
# Begin Source File

SOURCE=.\Include\Properties.h
# End Source File
# Begin Source File

SOURCE=.\Include\WinMain.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Resource\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\Resource\GUIEdit.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\GUIEdit.rc
# End Source File
# Begin Source File

SOURCE=.\Resource\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\Resource\resource.h
# End Source File
# Begin Source File

SOURCE=.\Resource\small.ico
# End Source File
# End Group
# End Target
# End Project
