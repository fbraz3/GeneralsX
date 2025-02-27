# Microsoft Developer Studio Project File - Name="GameEngineDevice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GameEngineDevice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameEngineDevice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameEngineDevice.mak" CFG="GameEngineDevice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameEngineDevice - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameEngineDevice - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameEngineDevice - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE "GameEngineDevice - Win32 Profile" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/RTS/Code/GameEngineDevice", TNAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "../Libraries/Source/Benchmark" /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "_LIB" /D "_WINDOWS" /D "IG_DEBUG_STACKTRACE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Lib\GameEngineDevice.lib"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I "..\Libraries\Include\MSS" /I "../Libraries/Source/Benchmark" /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Lib\GameEngineDeviceDebug.lib"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

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
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /Od /I "../Libraries/Source/Benchmark" /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "_INTERNAL" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Lib\GameEngineDevice.lib"
# ADD LIB32 /nologo /out:"Lib\GameEngineDeviceInternal.lib"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "../Libraries/Source/Benchmark" /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "_LIB" /D "_WINDOWS" /D "IG_DEBUG_STACKTRACE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /O2 /Ob2 /I "../Libraries/Source/Benchmark" /I "..\Main" /I "Include" /I "..\GameEngine\Include" /I "..\Libraries\Include" /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WW3D2" /I "..\Libraries\Source\WWVegas\WWLib" /I "..\Libraries\Source\WWVegas\WWDebug" /I "..\Libraries\Source\WWVegas\WWMath" /I "..\Libraries\Source\WWVegas\WWSaveLoad" /I "..\Libraries\Include\Granny" /D "_LIB" /D "_WINDOWS" /D "IG_DEBUG_STACKTRACE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_RELEASE" /D "_PROFILE" /YX /FD /Gh /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Lib\GameEngineDevice.lib"
# ADD LIB32 /nologo /out:"Lib\GameEngineDeviceProfile.lib"

!ENDIF 

# Begin Target

# Name "GameEngineDevice - Win32 Release"
# Name "GameEngineDevice - Win32 Debug"
# Name "GameEngineDevice - Win32 Internal"
# Name "GameEngineDevice - Win32 Profile"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Group "W3DDevice"

# PROP Default_Filter ""
# Begin Group "GameLogic (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameLogic\W3DGameLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameLogic\W3DGhostObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameLogic\W3DTerrainLogic.cpp
# End Source File
# End Group
# Begin Group "Common (W3DDevice)"

# PROP Default_Filter ""
# Begin Group "Thing (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\Common\Thing\W3DModuleFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\Common\Thing\W3DThingFactory.cpp
# End Source File
# End Group
# Begin Group "System (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\Common\System\W3DFunctionLexicon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\Common\System\W3DRadar.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\W3DDevice\Common\W3DConvert.cpp
# End Source File
# End Group
# Begin Group "GameClient (W3DDevice)"

# PROP Default_Filter ""
# Begin Group "Drawable (W3DDevice)"

# PROP Default_Filter ""
# Begin Group "Draw (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DDebrisDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DDefaultDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DDependencyModelDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DLaserDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DModelDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DOverlordAircraftDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DOverlordTankDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DOverlordTruckDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DPoliceCarDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DProjectileStreamDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DPropDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DRopeDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DScienceModelDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DSupplyDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DTankDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DTankTruckDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DTracerDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DTreeDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Drawable\Draw\W3DTruckDraw.cpp
# End Source File
# End Group
# End Group
# Begin Group "GUI (W3DDevice)"

# PROP Default_Filter ""
# Begin Group "Gadget (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DCheckBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DHorizontalSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DPushButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DRadioButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DStaticText.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DTabControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DTextEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\Gadget\W3DVerticalSlider.cpp
# End Source File
# End Group
# Begin Group "GUICallbacks"

# PROP Default_Filter ""
# Begin Group "Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\GUICallbacks\W3DMainMenu.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\GUICallbacks\W3DControlBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\GUICallbacks\W3DMOTD.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\W3DGameFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\W3DGameWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\GUI\W3DGameWindowManager.cpp
# End Source File
# End Group
# Begin Group "Water"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Water\W3DWater.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Water\W3DWaterTracks.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Water\wave.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvp

"$(WkspDir)\..\Run\wave.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\wave.pso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvp

"$(WkspDir)\..\Run\Shaders\wave.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\wave.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvp

"$(WkspDir)\..\Run\Shaders\wave.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\wave.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvp

"$(WkspDir)\..\Run\wave.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\wave.pso

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Water\wave.nvv

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvv

"$(WkspDir)\..\Run\wave.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\wave.vso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvv

"$(WkspDir)\..\Run\Shaders\wave.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\wave.vso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvv

"$(WkspDir)\..\Run\Shaders\wave.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\wave.vso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Water\wave.nvv

"$(WkspDir)\..\Run\wave.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\wave.vso

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Shaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\fterrain.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\fterrain.nvp

"$(WkspDir)\..\Run\Shaders\fterrain.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\fterrain.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\fterrain0.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\fterrain0.nvp

"$(WkspDir)\..\Run\Shaders\fterrain0.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\fterrain0.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\fterrainnoise.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\fterrainnoise.nvp

"$(WkspDir)\..\Run\Shaders\fterrainnoise.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\fterrainnoise.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\fterrainnoise2.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\fterrainnoise2.nvp

"$(WkspDir)\..\Run\Shaders\fterrainnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\fterrainnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\invmonochrome.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\invmonochrome.nvp

"$(WkspDir)\..\Run\Shaders\invmonochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\invmonochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\invmonochrome.nvp

"$(WkspDir)\..\Run\Shaders\invmonochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\invmonochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\invmonochrome.nvp

"$(WkspDir)\..\Run\Shaders\invmonochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\invmonochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\invmonochrome.nvp

"$(WkspDir)\..\Run\Shaders\invmonochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\invmonochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\monochrome.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\monochrome.nvp

"$(WkspDir)\..\Run\Shaders\monochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\monochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\monochrome.nvp

"$(WkspDir)\..\Run\Shaders\monochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\monochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\monochrome.nvp

"$(WkspDir)\..\Run\Shaders\monochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\monochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\monochrome.nvp

"$(WkspDir)\..\Run\Shaders\monochrome.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\monochrome.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\motionblur.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\motionblur.nvp

"$(WkspDir)\..\Run\Shaders\motionblur.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\motionblur.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\motionblur.nvp

"$(WkspDir)\..\Run\Shaders\motionblur.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\motionblur.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\motionblur.nvp

"$(WkspDir)\..\Run\Shaders\motionblur.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\motionblur.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\motionblur.nvp

"$(WkspDir)\..\Run\Shaders\motionblur.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\motionblur.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\MotionBlur.nvv

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\MotionBlur.nvv

"$(WkspDir)\..\Run\Shaders\MotionBlur.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\MotionBlur.vso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\MotionBlur.nvv

"$(WkspDir)\..\Run\Shaders\MotionBlur.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\MotionBlur.vso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\MotionBlur.nvv

"$(WkspDir)\..\Run\Shaders\MotionBlur.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\MotionBlur.vso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\MotionBlur.nvv

"$(WkspDir)\..\Run\Shaders\MotionBlur.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\MotionBlur.vso

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\roadnoise2.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\roadnoise2.nvp

"$(WkspDir)\..\Run\Shaders\roadnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\roadnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\roadnoise2.nvp

"$(WkspDir)\..\Run\Shaders\roadnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\roadnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\roadnoise2.nvp

"$(WkspDir)\..\Run\Shaders\roadnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\roadnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\roadnoise2.nvp

"$(WkspDir)\..\Run\Shaders\roadnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\roadnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\terrain.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrain.nvp

"$(WkspDir)\..\Run\Shaders\terrain.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrain.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrain.nvp

"$(WkspDir)\..\Run\Shaders\terrain.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrain.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrain.nvp

"$(WkspDir)\..\Run\Shaders\terrain.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrain.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrain.nvp

"$(WkspDir)\..\Run\Shaders\terrain.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrain.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\terrainnoise.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\terrainnoise2.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise2.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise2.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise2.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\terrainnoise2.nvp

"$(WkspDir)\..\Run\Shaders\terrainnoise2.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\terrainnoise2.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\Trees.nvp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvp

"$(WkspDir)\..\Run\Shaders\Trees.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvp

"$(WkspDir)\..\Run\Shaders\Trees.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvp

"$(WkspDir)\..\Run\Shaders\Trees.pso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.pso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shaders\Trees.nvv

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvv

"$(WkspDir)\..\Run\Shaders\Trees.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.vso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvv

"$(WkspDir)\..\Run\Shaders\Trees.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.vso 
	if ERRORLEVEL 7 ECHO Warning: Output File Read Only! 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

# Begin Custom Build
WkspDir=.
InputPath=.\Source\W3DDevice\GameClient\Shaders\Trees.nvv

"$(WkspDir)\..\Run\Shaders\Trees.vso" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\nvasm\nvasm -d $(InputPath) $(WkspDir)\..\Run\Shaders\Trees.vso

# End Custom Build

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

!ENDIF 

# End Source File
# End Group
# Begin Group "Shadow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shadow\W3DBufferManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shadow\W3DProjectedShadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shadow\W3DShadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\Shadow\W3DVolumetricShadow.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\BaseHeightMap.cpp

!IF  "$(CFG)" == "GameEngineDevice - Win32 Release"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Debug"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Internal"

!ELSEIF  "$(CFG)" == "GameEngineDevice - Win32 Profile"

# ADD CPP /FAcs

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\camerashakesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\FlatHeightMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\HeightMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\TerrainTex.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\TileData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DAssetManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DAssetManagerExposed.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DBibBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DBridgeBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DCustomEdging.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDebugDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDebugIcons.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDisplayString.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDisplayStringManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DDynamicLight.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DGameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DGranny.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DInGameUI.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DParticleSys.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DPoly.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DPropBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DRoadBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DScene.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DShaderManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DShroud.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DSmudge.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DSnow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DStatusCircle.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DTerrainBackground.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DTerrainTracks.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DTerrainVisual.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DTreeBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DVideoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DView.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3dWaypointBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\W3DWebBrowser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\W3DDevice\GameClient\WorldHeightMap.cpp
# End Source File
# End Group
# End Group
# Begin Group "Win32Device"

# PROP Default_Filter ""
# Begin Group "Common (Win32Device)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32BIGFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32BIGFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32CDManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32GameEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32LocalFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32LocalFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\Common\Win32OSDisplay.cpp
# End Source File
# End Group
# Begin Group "GameClient (Win32Device)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Win32Device\GameClient\Win32DIKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Win32Device\GameClient\Win32Mouse.cpp
# End Source File
# End Group
# End Group
# Begin Group "VideoDevice"

# PROP Default_Filter ""
# Begin Group "Bink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\VideoDevice\Bink\BinkVideoPlayer.cpp
# End Source File
# End Group
# End Group
# Begin Group "MilesAudioDevice"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\MilesAudioDevice\MilesAudioManager.cpp
# End Source File
# End Group
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Group "W3DDevice H"

# PROP Default_Filter ""
# Begin Group "GameLogic H (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\W3DDevice\GameLogic\W3DGameLogic.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameLogic\W3DGhostObject.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameLogic\W3DTerrainLogic.h
# End Source File
# End Group
# Begin Group "Common H (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\W3DDevice\Common\W3DConvert.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\Common\W3DFunctionLexicon.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\Common\W3DModuleFactory.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\Common\W3DRadar.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\Common\W3DThingFactory.h
# End Source File
# End Group
# Begin Group "GameClient H (W3DDevice)"

# PROP Default_Filter ""
# Begin Group "Module H (W3DDevice)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DAssetManagerExposed.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DDebrisDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DDefaultDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DDependencyModelDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DLaserDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DModelDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DOverlordAircraftDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DOverlordTankDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DOverlordTruckDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DPoliceCarDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DProjectileStreamDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DPropDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DRopeDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DScienceModelDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DSupplyDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DTankDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DTankTruckDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DTracerDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DTreeDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\Module\W3DTruckDraw.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\BaseHeightMap.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\camerashakesystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\FlatHeightMap.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\HeightMap.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\TerrainTex.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\TileData.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DAssetManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DBibBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DBridgeBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DBufferManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DCustomEdging.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DCustomScene.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDebugDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDebugIcons.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDisplayString.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDisplayStringManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DDynamicLight.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGadget.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGameClient.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGameFont.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGameWindow.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGameWindowManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGranny.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DGUICallbacks.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DInGameUI.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DMirror.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DMouse.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DParticleSys.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DPoly.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DProjectedShadow.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DRoadBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DScene.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DShaderManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DShadow.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DShroud.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DSmudge.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DSnow.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DStatusCircle.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DTerrainBackground.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DTerrainTracks.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DTerrainVisual.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DTreeBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DVideobuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DView.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DVolumetricShadow.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DWater.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DWaterTracks.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DWaypointBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DWebBrowser.h
# End Source File
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\WorldHeightMap.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\W3DDevice\GameClient\W3DPropBuffer.h
# End Source File
# End Group
# Begin Group "Win32Device H"

# PROP Default_Filter ""
# Begin Group "Common H (Win32Device)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32BIGFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32BIGFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32CDManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32GameEngine.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32LocalFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\Common\Win32LocalFileSystem.h
# End Source File
# End Group
# Begin Group "GameClient H (Win32Device)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Win32Device\GameClient\Win32DIKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\Include\Win32Device\GameClient\Win32Mouse.h
# End Source File
# End Group
# End Group
# Begin Group "VideoDevice H"

# PROP Default_Filter ""
# Begin Group "Bink H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\VideoDevice\Bink\BINK.H
# End Source File
# Begin Source File

SOURCE=.\Include\VideoDevice\Bink\BinkVideoPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Include\VideoDevice\Bink\RAD.H
# End Source File
# Begin Source File

SOURCE=.\Include\VideoDevice\Bink\radbase.h
# End Source File
# End Group
# End Group
# Begin Group "MilesAudioDevice H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\MilesAudioDevice\MilesAudioManager.h
# End Source File
# End Group
# End Group
# End Target
# End Project
