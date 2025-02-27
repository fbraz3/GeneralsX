# Microsoft Developer Studio Project File - Name="WorldBuilder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WorldBuilder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WorldBuilder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WorldBuilder.mak" CFG="WorldBuilder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WorldBuilder - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WorldBuilder - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "WorldBuilder - Win32 Internal" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/RTS/Code/Tools/WorldBuilder", RQAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WorldBuilder - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /I "../../Libraries/Source/Compression" /D "IG_DEBUG_STACKTRACE" /D "NDEBUG" /D "_RELEASE" /D "EDITOR" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "WIN32_LEAN_AND_MEAN" /D "_AFXDLL" /YX"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Benchmark.lib ddraw.lib dinput.lib d3dx8.lib mss32.lib d3d8.lib winmm.lib vfw32.lib dsound.lib wsock32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\WorldBuilder.pdb" /map:"..\..\..\Run\WorldBuilder.map" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\..\Run\WorldBuilder.exe" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\GameEngine" /OPT:REF /OPT:ICF
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "WorldBuilder - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /I "../../Libraries/Source/Compression" /D "WWDEBUG" /D "_DEBUG" /D "EDITOR" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "WIN32_LEAN_AND_MEAN" /D "_AFXDLL" /YX"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 BenchmarkD.lib ddraw.lib dinput.lib d3dx8.lib mss32.lib d3d8.lib winmm.lib vfw32.lib dsound.lib wsock32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\WorldBuilderD.pdb" /map:"..\..\..\Run\WorldBuilderD.map" /debug /machine:I386 /out:"..\..\..\Run\WorldBuilderD.exe" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "WorldBuilder - Win32 Internal"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Internal"
# PROP BASE Intermediate_Dir "Internal"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Internal"
# PROP Intermediate_Dir "Internal"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /D "EDITOR" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "WIN32_LEAN_AND_MEAN" /D "_AFXDLL" /YX"stdafx.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MD /W3 /GX /Zi /Od /I "./include" /I "./res" /I "../../libraries/include" /I "../../gameengine/include" /I "../../gameenginedevice/include/" /I "../../libraries/source/wwvegas" /I "../../libraries/source/wwvegas/ww3d2" /I "../../libraries/source/wwvegas/wwdebug" /I "../../libraries/source/wwvegas/wwlib" /I "../../libraries/source/wwvegas/wwmath" /I "../../libraries/source/wwvegas/wwsaveload" /I "../../Libraries/Source/Compression" /D "NDEBUG" /D "_INTERNAL" /D "EDITOR" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "REGEX_MALLOC" /D "STDC_HEADERS" /D "WIN32_LEAN_AND_MEAN" /D "_AFXDLL" /YX"stdafx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 d3dx8.lib d3d8.lib winmm.lib vfw32.lib ddraw.lib dinput.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBC" /out:"../../../run/WorldBuilder.exe" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\GameEngine"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 Benchmark.lib dxguid.lib ddraw.lib dinput8.lib d3dx8.lib mss32.lib d3d8.lib winmm.lib vfw32.lib dsound.lib wsock32.lib imm32.lib wininet.lib /nologo /subsystem:windows /pdb:"..\..\..\Run\WorldBuilderI.pdb" /map:"..\..\..\Run\WorldBuilderI.map" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"..\..\..\Run\WorldBuilderI.exe" /libpath:"..\..\Libraries\Lib" /libpath:"..\..\GameEngine\Lib" /libpath:"..\..\GameEngineDevice\Lib" /libpath:"..\..\GameEngine" /OPT:REF /OPT:ICF
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "WorldBuilder - Win32 Release"
# Name "WorldBuilder - Win32 Debug"
# Name "WorldBuilder - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\addplayerdialog.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\AutoEdgeOutTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BaseBuildProps.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\BlendEdgeTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BlendMaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BorderTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\brushoptions.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\BrushTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BuildList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BuildListTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CameraOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CButtonShowColor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CellWidth.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CFixTeamOwnerDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ContourOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CUndoable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DrawObject.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditAction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditCondition.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditCoordParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditObjectParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EditParameter.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\EulaDialog.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ExportScriptsOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EyedropperTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FeatherOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FeatherTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FenceOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FenceTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FloodFillTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GlobalLightOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GroveOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GroveTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HandScrollTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ImpassableOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LayersList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LightOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mapobjectprops.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\MapPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MapSettings.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\MeshMoldOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MeshMoldTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MoundOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MoundTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MyToolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NewHeightMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ObjectOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ObjectPreview.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ObjectTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\OpenMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\OptionsPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PickUnitDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playerlistdlg.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\PointerTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PolygonTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\propedit.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\RampOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RampTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RoadOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RoadTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RulerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RulerTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SaveMap.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ScorchOptions.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ScorchTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ScriptActionsFalse.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ScriptActionsTrue.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ScriptConditions.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\ScriptDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ScriptProperties.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\SelectMacrotexture.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ShadowOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TeamBehavior.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\TeamGeneric.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TeamIdentity.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\TeamObjectProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TeamReinforcement.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\teamsdialog.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\TerrainMaterial.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TerrainModal.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\TerrainSwatches.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TileTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Tool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WaterOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WaterTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WaypointOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WaypointTool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WBFrameWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WBHeightMap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WBPopupSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\src\wbview.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\wbview3d.cpp
# ADD CPP /I "..\res"
# End Source File
# Begin Source File

SOURCE=.\src\WHeightMapEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WorldBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WorldBuilderDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WorldBuilderView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\addplayerdialog.h
# End Source File
# Begin Source File

SOURCE=.\include\AutoEdgeOutTool.h
# End Source File
# Begin Source File

SOURCE=.\include\BaseBuildProps.h
# End Source File
# Begin Source File

SOURCE=.\include\BlendEdgeTool.h
# End Source File
# Begin Source File

SOURCE=.\include\BlendMaterial.h
# End Source File
# Begin Source File

SOURCE=.\include\BorderTool.h
# End Source File
# Begin Source File

SOURCE=.\include\brushoptions.h
# End Source File
# Begin Source File

SOURCE=.\include\BrushTool.h
# End Source File
# Begin Source File

SOURCE=.\include\BuildList.h
# End Source File
# Begin Source File

SOURCE=.\include\BuildListTool.h
# End Source File
# Begin Source File

SOURCE=.\include\CameraOptions.h
# End Source File
# Begin Source File

SOURCE=.\src\CButtonShowColor.h
# End Source File
# Begin Source File

SOURCE=.\include\CellWidth.h
# End Source File
# Begin Source File

SOURCE=.\include\CFixTeamOwnerDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\ContourOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\CUndoable.h
# End Source File
# Begin Source File

SOURCE=.\include\DrawObject.h
# End Source File
# Begin Source File

SOURCE=.\include\EditAction.h
# End Source File
# Begin Source File

SOURCE=.\include\EditCondition.h
# End Source File
# Begin Source File

SOURCE=.\include\EditCoordParameter.h
# End Source File
# Begin Source File

SOURCE=.\include\EditGroup.h
# End Source File
# Begin Source File

SOURCE=.\include\EditObjectParameter.h
# End Source File
# Begin Source File

SOURCE=.\include\EditParameter.h
# End Source File
# Begin Source File

SOURCE=.\include\euladialog.h
# End Source File
# Begin Source File

SOURCE=.\include\ExportScriptsOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\EyedropperTool.h
# End Source File
# Begin Source File

SOURCE=.\include\FeatherOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\FeatherTool.h
# End Source File
# Begin Source File

SOURCE=.\include\FenceOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\FenceTool.h
# End Source File
# Begin Source File

SOURCE=.\include\FloodFillTool.h
# End Source File
# Begin Source File

SOURCE=.\include\GlobalLightOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\GroveOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\GroveTool.h
# End Source File
# Begin Source File

SOURCE=.\include\HandScrollTool.h
# End Source File
# Begin Source File

SOURCE=.\include\ImpassableOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\LayersList.h
# End Source File
# Begin Source File

SOURCE=.\include\LightOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\MainFrm.h
# End Source File
# Begin Source File

SOURCE=..\..\gameengine\Include\Common\MapObject.h
# End Source File
# Begin Source File

SOURCE=.\include\mapobjectprops.h
# End Source File
# Begin Source File

SOURCE=.\include\MapPreview.h
# End Source File
# Begin Source File

SOURCE=.\include\MapSettings.h
# End Source File
# Begin Source File

SOURCE=.\include\MeshMoldOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\MeshMoldTool.h
# End Source File
# Begin Source File

SOURCE=.\include\MoundOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\MoundTool.h
# End Source File
# Begin Source File

SOURCE=.\include\MyToolbar.h
# End Source File
# Begin Source File

SOURCE=.\include\NewHeightMap.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectPreview.h
# End Source File
# Begin Source File

SOURCE=.\include\ObjectTool.h
# End Source File
# Begin Source File

SOURCE=.\include\OpenMap.h
# End Source File
# Begin Source File

SOURCE=.\include\OptionsPanel.h
# End Source File
# Begin Source File

SOURCE=.\include\PickUnitDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\playerlistdlg.h
# End Source File
# Begin Source File

SOURCE=.\include\PointerTool.h
# End Source File
# Begin Source File

SOURCE=.\include\PolygonTool.h
# End Source File
# Begin Source File

SOURCE=.\include\propedit.h
# End Source File
# Begin Source File

SOURCE=.\include\RampOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\RampTool.h
# End Source File
# Begin Source File

SOURCE=.\include\RoadOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\RoadTool.h
# End Source File
# Begin Source File

SOURCE=.\include\RulerOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\RulerTool.h
# End Source File
# Begin Source File

SOURCE=.\include\SaveMap.h
# End Source File
# Begin Source File

SOURCE=.\include\ScorchOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\ScorchTool.h
# End Source File
# Begin Source File

SOURCE=.\include\ScriptActionsFalse.h
# End Source File
# Begin Source File

SOURCE=.\include\ScriptActionsTrue.h
# End Source File
# Begin Source File

SOURCE=.\include\ScriptConditions.h
# End Source File
# Begin Source File

SOURCE=.\include\ScriptDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\ScriptProperties.h
# End Source File
# Begin Source File

SOURCE=.\include\SelectMacrotexture.h
# End Source File
# Begin Source File

SOURCE=.\include\ShadowOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\SplashScreen.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\TeamBehavior.h
# End Source File
# Begin Source File

SOURCE=.\include\TeamGeneric.h
# End Source File
# Begin Source File

SOURCE=.\include\TeamIdentity.h
# End Source File
# Begin Source File

SOURCE=.\include\TeamReinforcement.h
# End Source File
# Begin Source File

SOURCE=.\include\teamsdialog.h
# End Source File
# Begin Source File

SOURCE=.\include\terrainmaterial.h
# End Source File
# Begin Source File

SOURCE=.\include\TerrainModal.h
# End Source File
# Begin Source File

SOURCE=.\include\TerrainSwatches.h
# End Source File
# Begin Source File

SOURCE=.\include\TileTool.h
# End Source File
# Begin Source File

SOURCE=.\include\Tool.h
# End Source File
# Begin Source File

SOURCE=.\include\WaterOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\WaterTool.h
# End Source File
# Begin Source File

SOURCE=.\include\WaypointOptions.h
# End Source File
# Begin Source File

SOURCE=.\include\WaypointTool.h
# End Source File
# Begin Source File

SOURCE=.\include\WBFrameWnd.h
# End Source File
# Begin Source File

SOURCE=.\include\WBHeightMap.h
# End Source File
# Begin Source File

SOURCE=.\include\wbview.h
# End Source File
# Begin Source File

SOURCE=.\include\wbview3d.h
# End Source File
# Begin Source File

SOURCE=.\include\WHeightMapEdit.h
# End Source File
# Begin Source File

SOURCE=.\include\WorldBuilder.h
# End Source File
# Begin Source File

SOURCE=.\include\WorldBuilderDoc.h
# End Source File
# Begin Source File

SOURCE=.\include\WorldBuilderView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cross.cur
# End Source File
# Begin Source File

SOURCE=.\res\folder.bmp
# End Source File
# Begin Source File

SOURCE=.\res\foldersc.bmp
# End Source File
# Begin Source File

SOURCE=.\res\IDB_DownArrow.bmp
# End Source File
# Begin Source File

SOURCE=.\res\IDB_ROAD_CORNERS.bmp
# End Source File
# Begin Source File

SOURCE=.\res\IDC_AUTO_EDGE_OUT.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_BLEND_EDGE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_BUILD_LIST_TOOL.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_BUILD_MOVE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_BUILD_POINTER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_BUILD_ROTATE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_CLIFF.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_EYEDROPPER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_FENCE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_FLOOD_FILL.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_GROVE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_HAND_SCROLL.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_MOLD_POINTER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_MOVE_POINTER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_PLACE_OBJECT.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_POINTER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_POLYGON.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_POLYGON_MOVE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_POLYGON_PLUS.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_RAMP.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_ROAD.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_ROTATE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_Scorch.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_WATER.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_WATER_MOVE.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_WATER_PLUS.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDC_WAYPOINT.cur
# End Source File
# Begin Source File

SOURCE=.\res\IDI_Hide.ico
# End Source File
# Begin Source File

SOURCE=.\res\IDI_Show.ico
# End Source File
# Begin Source File

SOURCE=.\res\IDI_Thumb.ico
# End Source File
# Begin Source File

SOURCE=.\res\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\TileTool.cur
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WorldBuilder.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WorldBuilder.ico
# End Source File
# Begin Source File

SOURCE=.\res\WorldBuilder.rc
# End Source File
# Begin Source File

SOURCE=.\res\WorldBuilder.rc2
# End Source File
# Begin Source File

SOURCE=.\res\WorldBuilderDoc.ico
# End Source File
# End Group
# End Target
# End Project
