# Microsoft Developer Studio Project File - Name="GameEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GameEngine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GameEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GameEngine.mak" CFG="GameEngine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GameEngine - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GameEngine - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "GameEngine - Win32 Internal" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/RTS/Code/GameEngine", FAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GameEngine - Win32 Release"

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
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /O2 /Ob2 /I "../Libraries/Source/Compression" /I "Include/Precompiled" /I "../Libraries/Source/WWVegas" /I "../Libraries/Source/WWVegas\WWLib" /I "Include" /I "../Libraries/Include" /I "../Libraries/Source/GameSpy" /D "IG_DEBUG_STACKTRACE" /D "NDEBUG" /D "_RELEASE" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "WIN32" /D "Z_PREFIX" /Fr /Yu"PreRTS.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Lib/GameEngine.lib"

!ELSEIF  "$(CFG)" == "GameEngine - Win32 Debug"

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
# ADD CPP /nologo /G6 /MDd /W3 /WX /Gm /GX /ZI /Od /I "..\Libraries\Source\WWVegas" /I "..\Libraries\Source\WWVegas\WWLib" /I "../Libraries/Source/Compression" /I "Include/Precompiled" /I "../Libraries/Source/WWVegas" /I "../Libraries/Source/WWVegas\WWLib" /I "Include" /I "../Libraries/Include" /I "../Libraries/Source/GameSpy" /D "_DEBUG" /D "BROWSER_DEBUG" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "WIN32" /D "Z_PREFIX" /Fr /Yu"PreRTS.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Lib/GameEngineDebug.lib"

!ELSEIF  "$(CFG)" == "GameEngine - Win32 Internal"

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
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "../Libraries/Source/WWVegas" /I "../Libraries/Source/WWVegas\WWLib" /I "Include" /I "../Libraries/Include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /WX /GX /Zi /O2 /I "../Libraries/Source/Compression" /I "Include/Precompiled" /I "../Libraries/Source/WWVegas" /I "../Libraries/Source/WWVegas\WWLib" /I "Include" /I "../Libraries/Include" /I "../Libraries/Source/GameSpy" /D "NDEBUG" /D "_INTERNAL" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "WIN32" /D "Z_PREFIX" /Fr /Yu"PreRTS.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Lib/GameEngineInternal.lib"

!ENDIF 

# Begin Target

# Name "GameEngine - Win32 Release"
# Name "GameEngine - Win32 Debug"
# Name "GameEngine - Win32 Internal"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Group "Thing"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\Thing\DrawModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Thing\Module.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Thing\ModuleFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Thing\Thing.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Thing\ThingFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Thing\ThingTemplate.cpp
# End Source File
# End Group
# Begin Group "System (Common)"

# PROP Default_Filter ""
# Begin Group "SaveGame (Common)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\System\SaveGame\GameState.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\SaveGame\GameStateMap.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\Common\System\ArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\ArchiveFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\AsciiString.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\BuildAssistant.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\CDManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Compression.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\CopyProtection.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\CriticalSection.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\DataChunk.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Directory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\DisabledTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\encrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\File.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\FileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\FunctionLexicon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\GameCommon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\GameMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\GameType.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Geometry.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\KindOf.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\List.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\LocalFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\LocalFileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\MemoryInit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\QuickTrig.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\QuotedPrintable.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Radar.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\RAMFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Snapshot.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\StackDump.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\StreamingArchiveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\String.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\SubsystemInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Trig.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\UnicodeString.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Upgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\Xfer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\XferCRC.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\XferLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\System\XferSave.cpp
# End Source File
# End Group
# Begin Group "INI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\INI\INI.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIAiData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIAudioEventInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INICommandButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INICommandSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIControlBarScheme.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INICrate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIDamageFX.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIDrawGroupInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIGameData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIMapCache.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIMapData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIMappedImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIMiscAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIMultiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIParticleSys.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INISpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INITerrain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INITerrainBridge.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INITerrainRoad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIVideo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIWater.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\INI\INIWebpageURL.cpp
# End Source File
# End Group
# Begin Group "Audio"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\Audio\AudioEventRTS.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Audio\AudioRequest.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Audio\GameAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Audio\GameMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Audio\GameSounds.cpp
# End Source File
# End Group
# Begin Group "RTS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\RTS\ActionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Energy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Handicap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\MissionStats.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Money.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\PlayerList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\PlayerTemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\ProductionPrerequisite.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\ResourceGatheringManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Science.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\ScoreKeeper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\SpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\Team.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RTS\TunnelTracker.cpp
# End Source File
# End Group
# Begin Group "Bezier"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Common\Bezier\BezFwdIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Bezier\BezierSegment.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\Common\BitFlags.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\CommandLine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\crc.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\CRCDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\DamageFX.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Dict.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\DiscreteCircle.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\GameEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\GameLOD.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\GameMain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\GlobalData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Language.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\MessageStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\MiniLog.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\MultiplayerSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\NameKeyGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\PartitionSolver.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\PerfTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\RandomValue.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\Recorder.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\SkirmishBattleHonors.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\StateMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\StatsCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\TerrainTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\UserPreferences.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Common\version.cpp
# End Source File
# End Group
# Begin Group "GameLogic"

# PROP Default_Filter ""
# Begin Group "AI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AI.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIDock.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIGuard.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIPathfind.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AISkirmishPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AIStates.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\AITNGuard.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\Squad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\AI\TurretAI.cpp
# End Source File
# End Group
# Begin Group "Object"

# PROP Default_Filter ""
# Begin Group "Behavior"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\AutoHealBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\BehaviorModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\BridgeBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\BridgeScaffoldBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\BridgeTowerBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\DumbProjectileBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\FireWeaponWhenDamagedBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\FireWeaponWhenDeadBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\GenerateMinefieldBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\HelicopterSlowDeathUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\InstantDeathBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\JetSlowDeathBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\MinefieldBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\NeutronMissileSlowDeathUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\OverchargeBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\ParkingPlaceBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\PhysicsUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\PoisonedBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\POWTruckBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\PrisonBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\PropagandaCenterBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\PropagandaTowerBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\RebuildHoleBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\SlowDeathBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\SpawnBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\SupplyWarehouseCripplingBehavior.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Behavior\TechBuildingBehavior.cpp
# End Source File
# End Group
# Begin Group "Body"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\ActiveBody.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\BodyModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\HighlanderBody.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\HiveStructureBody.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\ImmortalBody.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\InactiveBody.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Body\StructureBody.cpp
# End Source File
# End Group
# Begin Group "Collide"

# PROP Default_Filter ""
# Begin Group "CrateCollide"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\ConvertToCarBombCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\ConvertToHijackedVehicleCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\CrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\HealCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\MoneyCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\SalvageCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\ShroudCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\UnitCrateCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CrateCollide\VeterancyCrateCollide.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\CollideModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\FireWeaponCollide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Collide\SquishCollide.cpp
# End Source File
# End Group
# Begin Group "Contain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\CaveContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\GarrisonContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\HealContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\MobNexusContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\OpenContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\OverlordContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\ParachuteContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\RailedTransportContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\TransportContain.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Contain\TunnelContain.cpp
# End Source File
# End Group
# Begin Group "Create"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\CreateModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\GrantUpgradeCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\PreorderCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\SpecialPowerCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\SupplyCenterCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\SupplyWarehouseCreate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Create\VeterancyGainCreate.cpp
# End Source File
# End Group
# Begin Group "Damage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Damage\BoneFXDamage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Damage\DamageModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Damage\TransitionDamageFX.cpp
# End Source File
# End Group
# Begin Group "Die"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\CreateCrateDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\CreateObjectDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\CrushDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\DamDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\DestroyDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\DieModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\EjectPilotDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\FXListDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\KeepObjectDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\RebuildHoleExposeDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\SpecialPowerCompletionDie.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Die\UpgradeDie.cpp
# End Source File
# End Group
# Begin Group "SpecialPower"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\BaikonurLaunchPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\CashBountyPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\CashHackSpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\CleanupAreaPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\DefectorSpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\DemoralizeSpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\OCLSpecialPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\SpecialAbility.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\SpecialPowerModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SpecialPower\SpyVisionSpecialPower.cpp
# End Source File
# End Group
# Begin Group "Update"

# PROP Default_Filter ""
# Begin Group "ProductionExitUpdate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProductionExitUpdate\DefaultProductionExitUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProductionExitUpdate\QueueProductionExitUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProductionExitUpdate\SpawnPointProductionExitUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProductionExitUpdate\SupplyCenterProductionExitUpdate.cpp
# End Source File
# End Group
# Begin Group "DockUpdate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\DockUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\PrisonDockUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\RailedTransportDockUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\RepairDockUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\SupplyCenterDockUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DockUpdate\SupplyWarehouseDockUpdate.cpp
# End Source File
# End Group
# Begin Group "AIUpdate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\AssaultTransportAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\ChinookAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\DeliverPayloadAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\DeployStyleAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\DozerAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\EMPUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\HackInternetAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\JetAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\MissileAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\POWTruckAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\RailedTransportAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\RailroadGuideAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\SupplyTruckAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\TransportAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\WanderAIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate\WorkerAIUpdate.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AIUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AssistedTargetingUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AutoDepositUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\AutoFindHealingUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\BaseRenerateUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\BattlePlanUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\BoneFXUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\CheckpointUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\CleanupHazardUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\CommandButtonHuntUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DelayedWeaponSetUpgradeUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DeletionUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DemoTrapUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DynamicGeometryInfoUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\DynamicShroudClearingRangeUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\EnemyNearUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FireOCLAfterWeaponCooldownUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FireSpreadUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FirestormDynamicGeometryInfoUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FireWeaponUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FlammableUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\FloatUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\HeightDieUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\HijackerUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\HordeUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\LifetimeUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\MissileLauncherBuildingUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\MobMemberSlavedUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\NeutronMissileUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\OCLUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ParticleUplinkCannonUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\PilotFindVehicleUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\PointDefenseLaserUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\PowerPlantUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProductionUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProjectileStreamUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ProneUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\RadarUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\RadiusDecalUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\SlavedUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\SpecialAbilityUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\SpyVisionUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\StealthDetectorUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\StealthUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\StickyBombUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\StructureCollapseUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\StructureToppleUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\TensileFormationUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\ToppleUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\UpdateModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\WaveGuideUpdate.cpp
# End Source File
# End Group
# Begin Group "Upgrade"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\ActiveShroudUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\ArmorUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\CommandSetUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\CostModifierUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\DelayedUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\ExperienceScalarUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\LocomotorSetUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\MaxHealthUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\ObjectCreationUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\PowerPlantUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\RadarUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\StatusBitsUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\StealthUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\SubObjectsUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\UnpauseSpecialPowerUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\UpgradeModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\WeaponBonusUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Upgrade\WeaponSetUpgrade.cpp
# End Source File
# End Group
# Begin Group "Helper"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Helper\ObjectDefectionHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Helper\ObjectHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Helper\ObjectRepulsorHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Helper\ObjectSMCHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Helper\ObjectWeaponStatusHelper.cpp
# End Source File
# End Group
# Begin Group "Destroy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Destroy\DestroyModule.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Armor.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\ExperienceTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\FiringTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\GhostObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Locomotor.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\ObjectCreationList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\ObjectTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\PartitionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\SimpleObjectIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\WeaponSet.cpp
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\System\CaveSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\System\CrateSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\System\Damage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\System\GameLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\System\GameLogicDispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\System\RankInfo.cpp
# End Source File
# End Group
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameLogic\Map\PolygonTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Map\SidesList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Map\TerrainLogic.cpp
# End Source File
# End Group
# Begin Group "ScriptEngine"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\Source\GameLogic\ScriptEngine\ScriptActions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\ScriptEngine\ScriptConditions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\ScriptEngine\ScriptEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\ScriptEngine\Scripts.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\ScriptEngine\VictoryConditions.cpp
# End Source File
# End Group
# End Group
# Begin Group "GameClient"

# PROP Default_Filter ""
# Begin Group "MessageStream"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\CommandXlat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\GUICommandTranslator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\HintSpy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\HotKey.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\LookAtXlat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\MetaEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\PlaceEventTranslator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\SelectionXlat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MessageStream\WindowXlat.cpp
# End Source File
# End Group
# Begin Group "Drawable"

# PROP Default_Filter ""
# Begin Group "Update (Client)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\Drawable\Update\AnimatedParticleSysBoneClientUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Drawable\Update\BeaconClientUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameLogic\Object\Update\LaserUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Drawable\Update\SwayClientUpdate.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameClient\Drawable\DrawableManager.cpp
# End Source File
# End Group
# Begin Group "System (GameClient)"

# PROP Default_Filter ""
# Begin Group "Debug Displayers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\Source\GameClient\System\Debug Displayers\AudioDebugDisplay.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameClient\System\Anim2D.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\System\CampaignManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\System\DebugDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\System\Image.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\System\ParticleSys.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\System\RayEffect.cpp
# End Source File
# End Group
# Begin Group "GUI"

# PROP Default_Filter ""
# Begin Group "Gadget"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetCheckBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetComboBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetHorizontalSlider.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetProgressBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetPushButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetRadioButton.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetStaticText.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetTabControl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetTextEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Gadget\GadgetVerticalSlider.cpp
# End Source File
# End Group
# Begin Group "GUICallbacks"

# PROP Default_Filter ""
# Begin Group "Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\CreditsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\DifficultySelect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\DisconnectWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\DownloadMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\EstablishConnectionsWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\GameInfoWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\KeyboardOptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\LanGameOptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\LanLobbyMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\LanMapSelectMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\MainMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\MapSelectMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\NetworkDirectConnect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\OptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupHostGame.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupJoinGame.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupLadderSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupPlayerInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupReplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\PopupSaveLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\QuitMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\ReplayMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\ScoreScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\SinglePlayerMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\SkirmishGameOptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\SkirmishMapSelectMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLBuddyOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLCustomScoreScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLGameSetupMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLLadderScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLLobbyMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLLocaleSelectPopup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLLoginMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLMapSelectMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLMessageWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLQMScoreScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLQuickMatchMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLStatusMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Menus\WOLWelcomeMenu.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\ControlBarCallback.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\ControlBarPopupDescription.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\Diplomacy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\ExtendedMessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\GeneralsExpPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\IMECandidate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\InGameChat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\InGamePopupMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\MessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GUICallbacks\ReplayControls.cpp
# End Source File
# End Group
# Begin Group "Shell"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Shell\Shell.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\Shell\ShellMenuScheme.cpp
# End Source File
# End Group
# Begin Group "ControlBar"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarBeacon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarCommandProcessing.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarMultiSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarOCLTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarPrintPositions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarResizer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarScheme.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarStructureInventory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ControlBar\ControlBarUnderConstruction.cpp
# End Source File
# End Group
# Begin Group "DisconnectMenu"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Source\GameClient\GUI\DisconnectMenu\DisconnectMenu.cpp
# End Source File
# End Group
# Begin Group "EstablishConnectionsMenu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\GUI\EstablishConnectionsMenu\EstablishConnectionsMenu.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameClient\GUI\AnimateWindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameFont.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindowGlobal.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindowManagerScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindowTransitions.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\GameWindowTransitionsStyles.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\HeaderTemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\IMEManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\LoadScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\ProcessAnimateWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\WindowLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\WindowVideoManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GUI\WinInstanceData.cpp
# End Source File
# End Group
# Begin Group "Input"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\Input\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Input\Mouse.cpp
# End Source File
# End Group
# Begin Group "Terrain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameClient\Terrain\TerrainRoads.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Terrain\TerrainVisual.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameClient\Color.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Credits.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\DisplayString.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\DisplayStringManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Drawable.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\DrawGroupInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Eva.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\FXList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GameClientDispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GameText.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GlobalLanguage.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\GraphDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\InGameUI.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\LanguageFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Line2D.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\MapUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\RadiusDecal.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\SelectionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\VideoPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\View.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameClient\Water.cpp
# End Source File
# End Group
# Begin Group "GameNetwork"

# PROP Default_Filter ""
# Begin Group "WOLBrowser"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\Source\GameNetwork\WOLBrowser\WebBrowser.cpp
# End Source File
# End Group
# Begin Group "GameSpy"

# PROP Default_Filter ""
# Begin Group "Thread"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\BuddyThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\GameResultsThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\PeerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\PersistentStorageThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\PingThread.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Thread\ThreadUtils.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\Chat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\DownloadManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpyOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\GSConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\LadderDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\LobbyUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\MainMenuUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\PeerDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy\StagingRoomGameInfo.cpp
# End Source File
# End Group
# Begin Group "GameSpyEvalImplementation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpy.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpyChat.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpyGameInfo.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpyGP.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameSpyPersistentStorage.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "InGameNetwork"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameNetwork\Connection.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\ConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\DisconnectManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\FileTransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\FirewallHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\FrameData.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\FrameDataManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\FrameMetrics.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NAT.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetCommandList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetCommandMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetCommandRef.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetCommandWrapperList.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetMessageStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetPacket.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\NetworkUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\Transport.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\udp.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\User.cpp
# End Source File
# End Group
# Begin Group "LAN"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\GameNetwork\LANAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\LANAPICallbacks.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\LANAPIhandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\LANGameInfo.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Source\GameNetwork\GameInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GameMessageParser.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\GUIUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GameNetwork\IPEnumeration.cpp
# End Source File
# End Group
# Begin Group "Precompiled"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Precompiled\PreRTS.cpp
# ADD CPP /Yc"PreRTS.h"
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Common H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Common\ActionManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ArchiveFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AsciiString.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioAffect.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioEventInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioEventRTS.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioHandleSpecialValues.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioRandomValue.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioRequest.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\AudioSettings.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BattleHonors.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BezFwdIterator.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BezierSegment.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BitFlags.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BitFlagsIO.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\BuildAssistant.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CDManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ClientUpdateModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CommandLine.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CopyProtection.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\crc.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CRCDebug.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CriticalSection.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\CustomMatchPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\DamageFX.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\DataChunk.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Debug.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Dict.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Directory.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\DisabledTypes.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\DiscreteCircle.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\DrawModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\encrypt.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Energy.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Errors.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\file.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\FileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\FunctionLexicon.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameAudio.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameCommon.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameEngine.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameLOD.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameMemory.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameMusic.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameSounds.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameSpeech.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameSpyMiscPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameState.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameStateMap.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GameType.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Geometry.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\GlobalData.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Handicap.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\IgnorePreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\INI.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\INIException.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\KindOf.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\LadderPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Language.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\LatchRestore.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\List.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\LocalFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\LocalFileSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MapObject.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MapReaderWriterInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MessageStream.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MiniLog.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MiscAudio.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MissionStats.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ModelState.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Module.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ModuleFactory.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Money.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\MultiplayerSettings.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\NameKeyGenerator.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\OSDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Overridable.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Override.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\PartitionSolver.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\PerfMetrics.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\PerfTimer.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Player.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\PlayerList.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\PlayerTemplate.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ProductionPrerequisite.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\QuickmatchPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\QuickTrig.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\QuotedPrintable.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Radar.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\RAMFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\RandomValue.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Recorder.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Registry.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ResourceGatheringManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Science.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ScoreKeeper.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SkirmishBattleHonors.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SkirmishPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Snapshot.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SparseMatchFinder.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SpecialPowerMaskType.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SpecialPowerType.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\StackDump.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\StateMachine.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\StatsCollector.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\STLTypedefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\StreamingArchiveFile.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\string.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SubsystemInterface.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\SystemInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Team.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Terrain.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\TerrainTypes.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Thing.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ThingFactory.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ThingSort.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\ThingTemplate.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\TunnelTracker.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\UnicodeString.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\UnitTimings.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Upgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\UserPreferences.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\version.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\WellKnownKeys.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\Xfer.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\XferCRC.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\XferDeepCRC.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\XferLoad.h
# End Source File
# Begin Source File

SOURCE=.\Include\Common\XferSave.h
# End Source File
# End Group
# Begin Group "GameLogic H"

# PROP Default_Filter ""
# Begin Group "Module H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ActiveBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ActiveShroudUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AITNGuard.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ArmorUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AssaultTransportAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AssistedTargetingUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AutoDepositUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AutoFindHealingUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\AutoHealBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BaikonurLaunchPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BaseRegenerateUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BattlePlanUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BehaviorModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BodyModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BoneFXDamage.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BoneFXUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BridgeBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BridgeScaffoldBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\BridgeTowerBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CashBountyPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CashHackSpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CaveContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CheckpointUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ChinookAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CleanupAreaPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CleanupHazardUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CollideModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CommandButtonHuntUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CommandSetUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ContainModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ConvertToCarBombCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ConvertToHijackedVehicleCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CostModifierUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CreateCrateDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CreateModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CreateObjectDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\CrushDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DamageModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DamDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DefaultProductionExitUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DefectorSpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DelayedUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DelayedWeaponSetUpgradeUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DeletionUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DeliverPayloadAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DemoralizeSpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DemoTrapUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DeployStyleAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DestroyDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DestroyModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DieModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DozerAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DumbProjectileBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DynamicGeometryInfoUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\DynamicShroudClearingRangeUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\EjectPilotDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\EMPUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\EnemyNearUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ExperienceScalarUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireOCLAfterWeaponCooldownUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireSpreadUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FirestormDynamicGeometryInfoUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireWeaponCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireWeaponUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireWeaponWhenDamagedBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FireWeaponWhenDeadBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FlammableUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FloatUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\FXListDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\GarrisonContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\GenerateMinefieldBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\GrantUpgradeCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HackInternetAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HealContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HealCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HeightDieUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HelicopterSlowDeathUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HighlanderBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HijackerUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HiveStructureBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\HordeUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ImmortalBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\InactiveBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\InstantDeathBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\JetAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\JetSlowDeathBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\KeepObjectDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\LifetimeUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\LocomotorSetUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MaxHealthUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MinefieldBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MissileAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MissileLauncherBuildingUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MobMemberSlavedUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MobNexusContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\MoneyCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\NeutronMissileSlowDeathUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\NeutronMissileUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectCreationUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectDefectionHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectRepulsorHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectSMCHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ObjectWeaponStatusHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\OCLSpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\OCLUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\OpenContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\OverchargeBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\OverlordContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ParachuteContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ParkingPlaceBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ParticleUplinkCannonUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PhysicsUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PilotFindVehicleUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PointDefenseLaserUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PoisonedBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PowerPlantUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PowerPlantUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\POWTruckAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\POWTruckBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PreorderCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PrisonBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PrisonDockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ProductionUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ProjectileStreamUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ProneUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PropagandaCenterBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\PropagandaTowerBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\QueueProductionExitUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RadarUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RadarUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RadiusDecalUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RailedTransportAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RailedTransportContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RailedTransportDockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RailroadGuideAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RebuildHoleBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RebuildHoleExposeDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\RepairDockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SalvageCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ShroudCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SlavedUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SlowDeathBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpawnBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpawnPointProductionExitUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpecialAbility.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpecialAbilityUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpecialPowerCompletionDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpecialPowerCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpecialPowerModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpyVisionSpecialPower.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SpyVisionUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SquishCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StatusBitsUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StealthDetectorUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StealthUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StealthUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StickyBombUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StructureBody.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StructureCollapseUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\StructureToppleUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SubObjectsUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyCenterCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyCenterDockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyCenterProductionExitUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyTruckAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyWarehouseCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyWarehouseCripplingBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\SupplyWarehouseDockUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TechBuildingBehavior.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TensileFormationUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\ToppleUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TransitionDamageFX.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TransportAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TransportContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\TunnelContain.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\UnitCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\UnpauseSpecialPowerUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\UpdateModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\UpgradeDie.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\UpgradeModule.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\VeterancyCrateCollide.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\VeterancyGainCreate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\WanderAIUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\WaveGuideUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\WeaponBonusUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\WeaponSetUpgrade.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\WorkerAIUpdate.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\GameLogic\AI.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AIDock.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AIGuard.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AIPathfind.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AIPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AISkirmishPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\AIStateMachine.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Armor.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ArmorSet.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\CaveSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\CrateSystem.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Damage.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ExperienceTracker.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\FiringTracker.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\FPUControl.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\GameLogic.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\GhostObject.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Locomotor.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\LocomotorSet.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\LogicRandomValue.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Object.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ObjectCreationList.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ObjectIter.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ObjectScriptStatusBits.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ObjectStatusBits.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ObjectTypes.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\PartitionManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\PolygonTrigger.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Powers.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\RankInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ScriptActions.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ScriptConditions.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\ScriptEngine.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Scripts.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\SidesList.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Squad.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\TerrainLogic.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\TurretAI.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\VictoryConditions.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Weapon.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\WeaponBonusConditionFlags.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\WeaponSet.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\WeaponSetFlags.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\WeaponSetType.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\WeaponStatus.h
# End Source File
# End Group
# Begin Group "GameClient H"

# PROP Default_Filter ""
# Begin Group "Module H (Client)"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\GameClient\Module\AnimatedParticleSysBoneClientUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Module\BeaconClientUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameLogic\Module\LaserUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Module\SwayClientUpdate.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\GameClient\Anim2D.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\AnimateWindowManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\CampaignManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\CDCheck.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ClientRandomValue.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Color.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\CommandXlat.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ControlBar.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ControlBarResizer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ControlBarScheme.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Credits.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DebugDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Diplomacy.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DisconnectMenu.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Display.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DisplayString.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DisplayStringManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Drawable.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DrawableInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DrawableManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\DrawGroupInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\EstablishConnectionsMenu.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Eva.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ExtendedMessageBox.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\FontDesc.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\FXList.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Gadget.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetCheckBox.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetComboBox.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetListBox.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetProgressBar.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetPushButton.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetRadioButton.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetSlider.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetStaticText.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetTabControl.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GadgetTextEntry.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameClient.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameFont.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameInfoWindow.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameText.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameWindow.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameWindowGlobal.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameWindowID.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameWindowManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GameWindowTransitions.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GlobalLanguage.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GraphDraw.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GUICallbacks.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\GUICommandTranslator.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\HeaderTemplate.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\HintSpy.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\HotKey.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Image.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\IMEManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\InGameUI.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\KeyDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\LanguageFilter.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Line2D.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\LoadScreen.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\LookAtXlat.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\MapUtil.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\MessageBox.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\MetaEvent.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ParticleSys.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\PlaceEventTranslator.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ProcessAnimateWindow.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\RadiusDecal.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\RayEffect.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\SelectionInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\SelectionXlat.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Shadow.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Shell.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ShellHooks.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\ShellMenuScheme.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Statistics.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\TerrainRoads.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\TerrainVisual.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\VideoPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\View.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\Water.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\WindowLayout.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\WindowVideoManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\WindowXlat.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameClient\WinInstanceData.h
# End Source File
# End Group
# Begin Group "GameNetwork.H"

# PROP Default_Filter ""
# Begin Group "WOLBrowser H"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\Include\GameNetwork\WOLBrowser\FEBDispatch.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\WOLBrowser\WebBrowser.h
# End Source File
# End Group
# Begin Group "GameSpyLibs.H"

# PROP Default_Filter ""
# Begin Group "md5"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\md5.h
# End Source File
# End Group
# Begin Group "queryreporting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\queryreporting\gqueryreporting.h
# End Source File
# End Group
# Begin Group "pinger"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\pinger\pinger.h
# End Source File
# End Group
# Begin Group "CEngine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\CEngine\goaceng.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\CEngine\gserver.h
# End Source File
# End Group
# Begin Group "hashtable"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\hashtable.h
# End Source File
# End Group
# Begin Group "chat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chat.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chatCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chatChannel.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\Chat\chatCrypt.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chatHandlers.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chatMain.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\chat\chatSocket.h
# End Source File
# End Group
# Begin Group "nonport"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\nonport.h
# End Source File
# End Group
# Begin Group "darray"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\darray.h
# End Source File
# End Group
# Begin Group "peer.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peer.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerCEngine.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerGlobalCallbacks.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerKeys.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerMain.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerMangle.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerOperations.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerPing.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerPlayers.h
# End Source File
# Begin Source File

SOURCE=..\Libraries\Source\GameSpy\GameSpy\peer\peerRooms.h
# End Source File
# End Group
# End Group
# Begin Group "GameSpy.H"

# PROP Default_Filter ""
# Begin Group "Thread.H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\BuddyThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\GameResultsThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PeerThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PersistentStorageThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PingThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\ThreadUtils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\BuddyDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\GSConfig.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\LadderDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\LobbyUtils.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\MainMenuUtils.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PeerDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PeerDefsImplementation.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\PersistentStorageDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy\StagingRoomGameInfo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\GameNetwork\Connection.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\ConnectionManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\DisconnectManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\DownloadManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\FileTransfer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\FirewallHelper.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\FrameData.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\FrameDataManager.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\FrameMetrics.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameMessageParser.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpy.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyChat.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyGameInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyGP.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyOverlay.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyPersistentStorage.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GameSpyThread.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\GUIUtil.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\IPEnumeration.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\LANAPI.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\LANAPICallbacks.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\LANGameInfo.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\LANPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NAT.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetCommandList.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetCommandMsg.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetCommandRef.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetCommandWrapperList.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetPacket.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetworkDefs.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetworkInterface.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\NetworkUtil.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\RankPointValue.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\Transport.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\udp.h
# End Source File
# Begin Source File

SOURCE=.\Include\GameNetwork\User.h
# End Source File
# End Group
# Begin Group "Precompiled H"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Precompiled\PreRTS.h
# End Source File
# End Group
# End Group
# End Target
# End Project
