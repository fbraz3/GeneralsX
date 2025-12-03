/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// GameEngine.cpp /////////////////////////////////////////////////////////////////////////////////
// Implementation of the Game Engine singleton
// Author: Michael S. Booth, April 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include <filesystem>

#include "Common/ActionManager.h"
#include "Common/AudioAffect.h"
#include "Common/BuildAssistant.h"
#include "Common/CRCDebug.h"
#include "Common/FramePacer.h"
#include "Common/Radar.h"
#include "Common/PlayerTemplate.h"
#include "Common/Team.h"
#include "Common/PlayerList.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/INI.h"
#include "Common/INIException.h"
#include "Common/MessageStream.h"
#include "Common/ThingFactory.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/CDManager.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/RandomValue.h"
#include "Common/NameKeyGenerator.h"
#include "Common/ModuleFactory.h"
#include "Common/Debug.h"
#include "Common/GameState.h"
#include "Common/GameStateMap.h"
#include "Common/Science.h"
#include "Common/FunctionLexicon.h"
#include "Common/CommandLine.h"
#include "Common/DamageFX.h"
#include "Common/MultiplayerSettings.h"
#include "Common/Recorder.h"
#include "Common/SpecialPower.h"
#include "Common/TerrainTypes.h"
#include "Common/Upgrade.h"
#include "Common/UserPreferences.h"
#include "Common/Xfer.h"
#include "Common/XferCRC.h"
#include "Common/GameLOD.h"
#include "Common/Registry.h"
#include "Common/GameCommon.h"	// FOR THE ALLOW_DEBUG_CHEATS_IN_RELEASE #define
#include "Common/System/SDL2_AppWindow.h"

#include "GameLogic/Armor.h"
#include "GameLogic/AI.h"
#include "GameLogic/CaveSystem.h"
#include "GameLogic/CrateSystem.h"
#include "GameLogic/Damage.h"
#include "GameLogic/VictoryConditions.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/RankInfo.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"

#include "GameClient/ClientInstance.h"
#include "GameClient/FXList.h"
#include "GameClient/GameClient.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Shell.h"
#include "GameClient/GameText.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Water.h"
#include "GameClient/TerrainRoads.h"
#include "GameClient/MetaEvent.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/Drawable.h"
#include "GameClient/GUICallbacks.h"

#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/WOLBrowser/WebBrowser.h"
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"

#include "Common/version.h"
#include "Utility/compat.h"


//-------------------------------------------------------------------------------------------------

#ifdef DEBUG_CRC
class DeepCRCSanityCheck : public SubsystemInterface
{
public:
	DeepCRCSanityCheck() {}
	virtual ~DeepCRCSanityCheck() {}

	virtual void init(void) {}
	virtual void reset(void);
	virtual void update(void) {}

protected:
};

DeepCRCSanityCheck* TheDeepCRCSanityCheck = NULL;

void DeepCRCSanityCheck::reset(void)
{
	static Int timesThrough = 0;
	static UnsignedInt lastCRC = 0;

	AsciiString fname;
	fname.format("%sCRCAfter%dMaps.dat", TheGlobalData->getPath_UserData().str(), timesThrough);
	UnsignedInt thisCRC = TheGameLogic->getCRC(CRC_RECALC, fname);

	DEBUG_LOG(("DeepCRCSanityCheck: CRC is %X", thisCRC));
	DEBUG_ASSERTCRASH(timesThrough == 0 || thisCRC == lastCRC,
		("CRC after reset did not match beginning CRC!\nNetwork games won't work after this.\nOld: 0x%8.8X, New: 0x%8.8X",
			lastCRC, thisCRC));
	lastCRC = thisCRC;

	timesThrough++;
}
#endif // DEBUG_CRC

//-------------------------------------------------------------------------------------------------
/// The GameEngine singleton instance
GameEngine* TheGameEngine = NULL;

//-------------------------------------------------------------------------------------------------
SubsystemInterfaceList* TheSubsystemList = NULL;

//-------------------------------------------------------------------------------------------------
template<class SUBSYSTEM>
void initSubsystem(
	SUBSYSTEM*& sysref,
	AsciiString name,
	SUBSYSTEM* sys,
	Xfer* pXfer,
	const char* path1 = NULL,
	const char* path2 = NULL)
{
	sysref = sys;
	TheSubsystemList->initSubsystem(sys, path1, path2, pXfer, name);
}

//-------------------------------------------------------------------------------------------------
extern HINSTANCE ApplicationHInstance;  ///< our application instance
extern CComModule _Module;

//-------------------------------------------------------------------------------------------------
static void updateTGAtoDDS();

//-------------------------------------------------------------------------------------------------
static void updateWindowTitle()
{
	// TheSuperHackers @tweak Now prints product and version information in the Window title.

	DEBUG_ASSERTCRASH(TheVersion != NULL, ("TheVersion is NULL"));
	DEBUG_ASSERTCRASH(TheGameText != NULL, ("TheGameText is NULL"));

	UnicodeString title;

	if (rts::ClientInstance::getInstanceId() > 1u)
	{
		UnicodeString str;
		str.format(L"Instance:%.2u", rts::ClientInstance::getInstanceId());
		title.concat(str);
	}

	UnicodeString productString = TheVersion->getUnicodeProductString();

	if (!productString.isEmpty())
	{
		if (!title.isEmpty())
			title.concat(L" ");
		title.concat(productString);
	}

#if RTS_GENERALS
	const WideChar* defaultGameTitle = L"Command and Conquer Generals";
#elif RTS_ZEROHOUR
	const WideChar* defaultGameTitle = L"Command and Conquer Generals Zero Hour";
#endif
	UnicodeString gameTitle = TheGameText->FETCH_OR_SUBSTITUTE("GUI:Command&ConquerGenerals", defaultGameTitle);

	if (!gameTitle.isEmpty())
	{
		UnicodeString gameTitleFinal;
		UnicodeString gameVersion = TheVersion->getUnicodeVersion();

		if (productString.isEmpty())
		{
			gameTitleFinal = gameTitle;
		}
		else
		{
			UnicodeString gameTitleFormat = TheGameText->FETCH_OR_SUBSTITUTE("Version:GameTitle", L"for %ls");
			gameTitleFinal.format(gameTitleFormat.str(), gameTitle.str());
		}

		if (!title.isEmpty())
			title.concat(L" ");
		title.concat(gameTitleFinal.str());
		title.concat(L" ");
		title.concat(gameVersion.str());
	}

	if (!title.isEmpty())
	{
		AsciiString titleA;
		titleA.translate(title);	//get ASCII version for Win 9x

		// Phase 40: Convert to SDL2 window title update
		SDL_Window* sdlWindow = (SDL_Window*)g_applicationWindow;
		if (sdlWindow) {
			SDL_SetWindowTitle(sdlWindow, titleA.str());
		}
	}
}

//-------------------------------------------------------------------------------------------------
GameEngine::GameEngine(void)
{
	// initialize to non garbage values
	m_logicTimeAccumulator = 0.0f;
	m_quitting = FALSE;
	m_isActive = FALSE;

	_Module.Init(NULL, ApplicationHInstance, NULL);
}

//-------------------------------------------------------------------------------------------------
GameEngine::~GameEngine()
{
	//extern std::vector<std::string>	preloadTextureNamesGlobalHack;
	//preloadTextureNamesGlobalHack.clear();

	delete TheMapCache;
	TheMapCache = NULL;

	//	delete TheShell;
	//	TheShell = NULL;

	TheGameResultsQueue->endThreads();

	// TheSuperHackers @fix helmutbuhler 03/06/2025
	// Reset all subsystems before deletion to prevent crashing due to cross dependencies.
	reset();

	TheSubsystemList->shutdownAll();
	delete TheSubsystemList;
	TheSubsystemList = NULL;

	delete TheNetwork;
	TheNetwork = NULL;

	delete TheCommandList;
	TheCommandList = NULL;

	delete TheNameKeyGenerator;
	TheNameKeyGenerator = NULL;

	delete TheFileSystem;
	TheFileSystem = NULL;

	delete TheGameLODManager;
	TheGameLODManager = NULL;

	Drawable::killStaticImages();

	_Module.Term();

#ifdef PERF_TIMERS
	PerfGather::termPerfDump();
#endif
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isTimeFrozen()
{
	// TheSuperHackers @fix The time can no longer be frozen in Network games. It would disconnect the player.
	if (TheNetwork != NULL)
		return false;

	if (TheTacticalView != NULL)
	{
		if (TheTacticalView->isTimeFrozen() && !TheTacticalView->isCameraMovementFinished())
			return true;
	}

	if (TheScriptEngine != NULL)
	{
		if (TheScriptEngine->isTimeFrozenDebug() || TheScriptEngine->isTimeFrozenScript())
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isGameHalted()
{
	if (TheNetwork != NULL)
	{
		if (TheNetwork->isStalling())
			return true;
	}
	else
	{
		if (TheGameLogic != NULL && TheGameLogic->isGamePaused())
			return true;
	}

	return false;
}

/** -----------------------------------------------------------------------------------------------
 * Initialize the game engine by initializing the GameLogic and GameClient.
 */
void GameEngine::init()
{
	try {
		//create an INI object to use for loading stuff
		INI ini;

#ifdef DEBUG_LOGGING
		if (TheVersion)
		{
			DEBUG_LOG(("================================================================================"));
			DEBUG_LOG(("Generals version %s", TheVersion->getAsciiVersion().str()));
			DEBUG_LOG(("Build date: %s", TheVersion->getAsciiBuildTime().str()));
			DEBUG_LOG(("Build location: %s", TheVersion->getAsciiBuildLocation().str()));
			DEBUG_LOG(("Build user: %s", TheVersion->getAsciiBuildUser().str()));
			DEBUG_LOG(("Build git revision: %s", TheVersion->getAsciiGitCommitCount().str()));
			DEBUG_LOG(("Build git version: %s", TheVersion->getAsciiGitTagOrHash().str()));
			DEBUG_LOG(("Build git commit time: %s", TheVersion->getAsciiGitCommitTime().str()));
			DEBUG_LOG(("Build git commit author: %s", Version::getGitCommitAuthorName()));
			DEBUG_LOG(("================================================================================"));
		}
#endif

#if defined(PERF_TIMERS) || defined(DUMP_PERF_STATS)
		DEBUG_LOG(("Calculating CPU frequency for performance timers."));
		InitPrecisionTimer();
#endif
#ifdef PERF_TIMERS
		PerfGather::initPerfDump("AAAPerfStats", PerfGather::PERF_NETTIME);
#endif




#ifdef DUMP_PERF_STATS////////////////////////////////////////////////////////////
		int64_t startTime64;//////////////////////////////////////////////////////////////
		int64_t endTime64, freq64;///////////////////////////////////////////////////////////
		GetPrecisionTimerTicksPerSec(&freq64);///////////////////////////////////////////////
		GetPrecisionTimer(&startTime64);////////////////////////////////////////////////////
		char Buf[256];//////////////////////////////////////////////////////////////////////
#endif//////////////////////////////////////////////////////////////////////////////


		TheSubsystemList = MSGNEW("GameEngineSubsystem") SubsystemInterfaceList;
		printf("GameMain()::init() Calling TheSubsystemList->addSubsystem(this);...\n");
		
		TheSubsystemList->addSubsystem(this);

		// initialize the random number system
		InitRandom();
		printf("GameMain()::init() Calling TheFileSystem = createFileSystem();...\n");
		
		// Create the low-level file system interface
		TheFileSystem = createFileSystem();

		//Kris: Patch 1.01 - November 17, 2003
		//I was unable to resolve the RTPatch method of deleting a shipped file. English, Chinese, and Korean
		//SKU's shipped with two INIZH.big files. One properly in the Run directory and the other in Run\INI\Data.
		//We need to toast the latter in order for the game to patch properly.
		try {
			std::filesystem::remove(std::filesystem::path("Data\\INI\\INIZH.big"));
		}
		catch (const std::filesystem::filesystem_error& e) {
			DEBUG_LOG(("GameEngine - Failed to delete Data\\INI\\INIZH.big: %s", e.what()));
		}

		// not part of the subsystem list, because it should normally never be reset!
		TheNameKeyGenerator = MSGNEW("GameEngineSubsystem") NameKeyGenerator;
		printf("GameMain()::init() Calling TheNameKeyGenerator->init();...\n");
		
		TheNameKeyGenerator->init();


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheNameKeyGenerator  = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameMain()::init() Calling TheCommandList->init();...\n");
		
		// not part of the subsystem list, because it should normally never be reset!
		TheCommandList = MSGNEW("GameEngineSubsystem") CommandList;
		TheCommandList->init();

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheCommandList  = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		XferCRC xferCRC;
		xferCRC.open("lightCRC");

		// Helper lambda to build cross-platform paths
		auto buildPath = [](const char* p1, const char* p2 = NULL, const char* p3 = NULL) -> AsciiString {
			AsciiString path;
			path.concat("Data");
			path.concat(GET_PATH_SEPARATOR());
			path.concat(p1);
			if (p2) {
				path.concat(GET_PATH_SEPARATOR());
				path.concat(p2);
			}
			if (p3) {
				path.concat(GET_PATH_SEPARATOR());
				path.concat(p3);
			}
			return path;
			};
		printf("GameMain()::init() Initializing TheLocalFileSystem...\n");
		
		initSubsystem(TheLocalFileSystem, "TheLocalFileSystem", createLocalFileSystem(), NULL);


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheLocalFileSystem  = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameMain()::init() Initializing TheArchiveFileSystem...\n");
		
		initSubsystem(TheArchiveFileSystem, "TheArchiveFileSystem", createArchiveFileSystem(), NULL); // this MUST come after TheLocalFileSystem creation

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheArchiveFileSystem  = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameMain()::init() Initializing TheWritableGlobalData...\n");
		
		DEBUG_ASSERTCRASH(TheWritableGlobalData, ("TheWritableGlobalData expected to be created"));
		initSubsystem(TheWritableGlobalData, "TheWritableGlobalData", TheWritableGlobalData, &xferCRC, buildPath("INI", "Default", "GameData").str(), buildPath("INI", "GameData").str());
		TheWritableGlobalData->parseCustomDefinition();


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After  TheWritableGlobalData = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////



#if defined(RTS_DEBUG)
		// If we're in Debug, load the Debug settings as well.
		ini.loadFileDirectory(buildPath("INI", "GameDataDebug"), INI_LOAD_OVERWRITE, NULL);
#endif

		// special-case: parse command-line parameters after loading global data
		CommandLine::parseCommandLineForEngineInit();

		TheArchiveFileSystem->loadMods();

		// doesn't require resets so just create a single instance here.
		TheGameLODManager = MSGNEW("GameEngineSubsystem") GameLODManager;
		TheGameLODManager->init();

		// after parsing the command line, we may want to perform dds stuff. Do that here.
		if (TheGlobalData->m_shouldUpdateTGAToDDS) {
			// update any out of date targas here.
			updateTGAtoDDS();
		}

		// read the water settings from INI (must do prior to initing GameClient, apparently)
		ini.loadFileDirectory(buildPath("INI", "Default", "Water"), INI_LOAD_OVERWRITE, &xferCRC);
		ini.loadFileDirectory(buildPath("INI", "Water"), INI_LOAD_OVERWRITE, &xferCRC);
		ini.loadFileDirectory(buildPath("INI", "Default", "Weather"), INI_LOAD_OVERWRITE, &xferCRC);
		ini.loadFileDirectory(buildPath("INI", "Weather"), INI_LOAD_OVERWRITE, &xferCRC);



#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After water INI's = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


#ifdef DEBUG_CRC
		initSubsystem(TheDeepCRCSanityCheck, "TheDeepCRCSanityCheck", MSGNEW("GameEngineSubystem") DeepCRCSanityCheck, NULL);
#endif // DEBUG_CRC
		printf("GameEngine::init() - About to init TheGameText\n"); 
		initSubsystem(TheGameText, "TheGameText", CreateGameTextInterface(), NULL);
		updateWindowTitle();
		printf("GameEngine::init() - TheGameText initialized successfully\n"); 

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheGameText = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - About to init TheScienceStore\n"); 
		initSubsystem(TheScienceStore, "TheScienceStore", MSGNEW("GameEngineSubsystem") ScienceStore(), &xferCRC, buildPath("INI", "Default", "Science").str(), buildPath("INI", "Science").str());
		printf("GameEngine::init() - TheScienceStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheMultiplayerSettings\n"); 
		initSubsystem(TheMultiplayerSettings, "TheMultiplayerSettings", MSGNEW("GameEngineSubsystem") MultiplayerSettings(), &xferCRC, buildPath("INI", "Default", "Multiplayer").str(), buildPath("INI", "Multiplayer").str());
		printf("GameEngine::init() - TheMultiplayerSettings initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheTerrainTypes\n"); 
		initSubsystem(TheTerrainTypes, "TheTerrainTypes", MSGNEW("GameEngineSubsystem") TerrainTypeCollection(), &xferCRC, buildPath("INI", "Default", "Terrain").str(), buildPath("INI", "Terrain").str());
		printf("GameEngine::init() - TheTerrainTypes initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheTerrainRoads\n"); 
		initSubsystem(TheTerrainRoads, "TheTerrainRoads", MSGNEW("GameEngineSubsystem") TerrainRoadCollection(), &xferCRC, buildPath("INI", "Default", "Roads").str(), buildPath("INI", "Roads").str());
		printf("GameEngine::init() - TheTerrainRoads initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheGlobalLanguageData\n"); 
		initSubsystem(TheGlobalLanguageData, "TheGlobalLanguageData", MSGNEW("GameEngineSubsystem") GlobalLanguage, NULL); // must be before the game text
		TheGlobalLanguageData->parseCustomDefinition();
		printf("GameEngine::init() - TheGlobalLanguageData initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheCDManager\n"); 
		initSubsystem(TheCDManager, "TheCDManager", CreateCDManager(), NULL);
		printf("GameEngine::init() - TheCDManager initialized successfully\n"); 
#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheCDManager = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////
		printf("GameEngine::init() - About to init TheAudio\n"); 
		initSubsystem(TheAudio, "TheAudio", TheGlobalData->m_headless ? NEW AudioManagerDummy : createAudioManager(), NULL);
		printf("GameEngine::init() - TheAudio initialized successfully\n"); 
		// Phase 54: On non-Windows platforms (macOS/Linux), music files are inside .big archives
		// and isMusicAlreadyLoaded() may return false. Don't quit the game in this case.
#if defined(_WIN32)
		if (!TheAudio->isMusicAlreadyLoaded())
			setQuitting(TRUE);
#else
		if (!TheAudio->isMusicAlreadyLoaded()) {
			printf("GameEngine::init() - WARNING: Music not loaded from disk, but continuing on non-Windows platform\n");
			
		}
#endif

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheAudio = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameEngine::init() - About to init TheFunctionLexicon\n"); 
		initSubsystem(TheFunctionLexicon, "TheFunctionLexicon", createFunctionLexicon(), NULL);
		printf("GameEngine::init() - TheFunctionLexicon initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheModuleFactory\n"); 
		initSubsystem(TheModuleFactory, "TheModuleFactory", createModuleFactory(), NULL);
		printf("GameEngine::init() - TheModuleFactory initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheMessageStream\n"); 
		initSubsystem(TheMessageStream, "TheMessageStream", createMessageStream(), NULL);
		printf("GameEngine::init() - TheMessageStream initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheSidesList\n"); 
		initSubsystem(TheSidesList, "TheSidesList", MSGNEW("GameEngineSubsystem") SidesList(), NULL);
		printf("GameEngine::init() - TheSidesList initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheCaveSystem\n"); 
		initSubsystem(TheCaveSystem, "TheCaveSystem", MSGNEW("GameEngineSubsystem") CaveSystem(), NULL);
		printf("GameEngine::init() - TheCaveSystem initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheRankInfoStore\n"); 
		initSubsystem(TheRankInfoStore, "TheRankInfoStore", MSGNEW("GameEngineSubsystem") RankInfoStore(), &xferCRC, NULL, buildPath("INI", "Rank").str());
		printf("GameEngine::init() - TheRankInfoStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init ThePlayerTemplateStore\n"); 
		initSubsystem(ThePlayerTemplateStore, "ThePlayerTemplateStore", MSGNEW("GameEngineSubsystem") PlayerTemplateStore(), &xferCRC, buildPath("INI", "Default", "PlayerTemplate").str(), buildPath("INI", "PlayerTemplate").str());
		printf("GameEngine::init() - ThePlayerTemplateStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheParticleSystemManager\n"); 
		initSubsystem(TheParticleSystemManager, "TheParticleSystemManager", createParticleSystemManager(), NULL);
		printf("GameEngine::init() - TheParticleSystemManager initialized successfully\n"); 

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheParticleSystemManager = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameEngine::init() - About to init TheFXListStore\n"); 
		initSubsystem(TheFXListStore, "TheFXListStore", MSGNEW("GameEngineSubsystem") FXListStore(), &xferCRC, buildPath("INI", "Default", "FXList").str(), buildPath("INI", "FXList").str());
		printf("GameEngine::init() - TheFXListStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheWeaponStore\n"); 
		initSubsystem(TheWeaponStore, "TheWeaponStore", MSGNEW("GameEngineSubsystem") WeaponStore(), &xferCRC, NULL, buildPath("INI", "Weapon").str());
		printf("GameEngine::init() - TheWeaponStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheObjectCreationListStore\n"); 
		initSubsystem(TheObjectCreationListStore, "TheObjectCreationListStore", MSGNEW("GameEngineSubsystem") ObjectCreationListStore(), &xferCRC, buildPath("INI", "Default", "ObjectCreationList").str(), buildPath("INI", "ObjectCreationList").str());
		printf("GameEngine::init() - TheObjectCreationListStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheLocomotorStore\n"); 
		initSubsystem(TheLocomotorStore, "TheLocomotorStore", MSGNEW("GameEngineSubsystem") LocomotorStore(), &xferCRC, NULL, buildPath("INI", "Locomotor").str());
		printf("GameEngine::init() - TheLocomotorStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheSpecialPowerStore\n"); 
		initSubsystem(TheSpecialPowerStore, "TheSpecialPowerStore", MSGNEW("GameEngineSubsystem") SpecialPowerStore(), &xferCRC, buildPath("INI", "Default", "SpecialPower").str(), buildPath("INI", "SpecialPower").str());
		printf("GameEngine::init() - TheSpecialPowerStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheDamageFXStore\n"); 
		initSubsystem(TheDamageFXStore, "TheDamageFXStore", MSGNEW("GameEngineSubsystem") DamageFXStore(), &xferCRC, NULL, buildPath("INI", "DamageFX").str());
		printf("GameEngine::init() - TheDamageFXStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheArmorStore\n"); 
		initSubsystem(TheArmorStore, "TheArmorStore", MSGNEW("GameEngineSubsystem") ArmorStore(), &xferCRC, NULL, buildPath("INI", "Armor").str());
		printf("GameEngine::init() - TheArmorStore initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheBuildAssistant\n"); 
		initSubsystem(TheBuildAssistant, "TheBuildAssistant", MSGNEW("GameEngineSubsystem") BuildAssistant, NULL);
		printf("GameEngine::init() - TheBuildAssistant initialized successfully\n"); 


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheBuildAssistant = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - About to init TheThingFactory\n"); 
		initSubsystem(TheThingFactory, "TheThingFactory", createThingFactory(), &xferCRC, buildPath("INI", "Default", "Object").str(), buildPath("INI", "Object").str());
		printf("GameEngine::init() - TheThingFactory initialized successfully\n"); 

#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheThingFactory = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameEngine::init() - About to init TheUpgradeCenter\n"); 
		initSubsystem(TheUpgradeCenter, "TheUpgradeCenter", MSGNEW("GameEngineSubsystem") UpgradeCenter, &xferCRC, buildPath("INI", "Default", "Upgrade").str(), buildPath("INI", "Upgrade").str());
		printf("GameEngine::init() - TheUpgradeCenter initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheGameClient\n"); 
		initSubsystem(TheGameClient, "TheGameClient", createGameClient(), NULL);
		printf("GameEngine::init() - TheGameClient initialized successfully\n"); 


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheGameClient = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameEngine::init() - About to init TheAI\n"); 
		initSubsystem(TheAI, "TheAI", MSGNEW("GameEngineSubsystem") AI(), &xferCRC, buildPath("INI", "Default", "AIData").str(), buildPath("INI", "AIData").str());
		printf("GameEngine::init() - TheAI initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheGameLogic\n"); 
		initSubsystem(TheGameLogic, "TheGameLogic", createGameLogic(), NULL);
		printf("GameEngine::init() - TheGameLogic initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheTeamFactory\n"); 
		initSubsystem(TheTeamFactory, "TheTeamFactory", MSGNEW("GameEngineSubsystem") TeamFactory(), NULL);
		printf("GameEngine::init() - TheTeamFactory initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheCrateSystem\n"); 
		initSubsystem(TheCrateSystem, "TheCrateSystem", MSGNEW("GameEngineSubsystem") CrateSystem(), &xferCRC, buildPath("INI", "Default", "Crate").str(), buildPath("INI", "Crate").str());
		printf("GameEngine::init() - TheCrateSystem initialized successfully\n"); 
		printf("GameEngine::init() - About to init ThePlayerList\n"); 
		initSubsystem(ThePlayerList, "ThePlayerList", MSGNEW("GameEngineSubsystem") PlayerList(), NULL);
		printf("GameEngine::init() - ThePlayerList initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheRecorder\n"); 
		initSubsystem(TheRecorder, "TheRecorder", createRecorder(), NULL);
		printf("GameEngine::init() - TheRecorder initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheRadar\n"); 
		initSubsystem(TheRadar, "TheRadar", TheGlobalData->m_headless ? NEW RadarDummy : createRadar(), NULL);
		printf("GameEngine::init() - TheRadar initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheVictoryConditions\n"); 
		initSubsystem(TheVictoryConditions, "TheVictoryConditions", createVictoryConditions(), NULL);
		printf("GameEngine::init() - TheVictoryConditions initialized successfully\n"); 



#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheVictoryConditions = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - About to init TheMetaMap\n"); 
		AsciiString fname;
		fname.concat("Data");
		fname.concat(GET_PATH_SEPARATOR());
		fname.concat(GetRegistryLanguage());
		fname.concat(GET_PATH_SEPARATOR());
		fname.concat("CommandMap");
		initSubsystem(TheMetaMap, "TheMetaMap", MSGNEW("GameEngineSubsystem") MetaMap(), NULL, fname.str(), buildPath("INI", "CommandMap").str());
		printf("GameEngine::init() - TheMetaMap initialized successfully\n"); 

		printf("GameEngine::init() - Generating MetaMap\n"); 
		TheMetaMap->generateMetaMap();
		printf("GameEngine::init() - MetaMap generated\n"); 

#if defined(RTS_DEBUG)
		ini.loadFileDirectory(buildPath("INI", "CommandMapDebug").str(), INI_LOAD_MULTIFILE, NULL);
#endif

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
		ini.loadFileDirectory(buildPath("INI", "CommandMapDemo").str(), INI_LOAD_MULTIFILE, NULL);
#endif

		printf("GameEngine::init() - About to init TheActionManager\n"); 
		initSubsystem(TheActionManager, "TheActionManager", MSGNEW("GameEngineSubsystem") ActionManager(), NULL);
		printf("GameEngine::init() - TheActionManager initialized successfully\n"); 
		//initSubsystem((CComObject<WebBrowser> *)TheWebBrowser,"(CComObject<WebBrowser> *)TheWebBrowser", (CComObject<WebBrowser> *)createWebBrowser(), NULL);
		printf("GameEngine::init() - About to init TheGameStateMap\n"); 
		initSubsystem(TheGameStateMap, "TheGameStateMap", MSGNEW("GameEngineSubsystem") GameStateMap, NULL);
		printf("GameEngine::init() - TheGameStateMap initialized successfully\n"); 
		printf("GameEngine::init() - About to init TheGameState\n"); 
		initSubsystem(TheGameState, "TheGameState", MSGNEW("GameEngineSubsystem") GameState, NULL);
		printf("GameEngine::init() - TheGameState initialized successfully\n"); 

		// Create the interface for sending game results
		printf("GameEngine::init() - About to init TheGameResultsQueue\n"); 
		printf("GameEngine::init() - Calling createNewGameResultsInterface...\n"); 
		GameResultsInterface* resultsInterface = GameResultsInterface::createNewGameResultsInterface();
		printf("GameEngine::init() - createNewGameResultsInterface returned %p\n", (void*)resultsInterface); 
		if (resultsInterface) {
			printf("GameEngine::init() - Calling initSubsystem for TheGameResultsQueue...\n"); 
			initSubsystem(TheGameResultsQueue, "TheGameResultsQueue", resultsInterface, NULL);
			printf("GameEngine::init() - TheGameResultsQueue initialized successfully\n"); 
		}
		else {
			printf("GameEngine::init() - WARNING: createNewGameResultsInterface returned NULL, skipping\n"); 
		}


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheGameResultsQueue = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - Closing xferCRC\n"); 
		xferCRC.close();
		TheWritableGlobalData->m_iniCRC = xferCRC.getCRC();
		DEBUG_LOG(("INI CRC is 0x%8.8X", TheGlobalData->m_iniCRC));

		printf("GameEngine::init() - Calling postProcessLoadAll\n"); 
		TheSubsystemList->postProcessLoadAll();
		printf("GameEngine::init() - postProcessLoadAll done\n"); 

		printf("GameEngine::init() - Setting frame pacer limit\n"); 
		TheFramePacer->setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);
		printf("GameEngine::init() - Frame pacer limit set\n"); 

		printf("GameEngine::init() - Setting audio options\n"); 
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_musicOn, AudioAffect_Music);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_soundsOn, AudioAffect_Sound);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_sounds3DOn, AudioAffect_Sound3D);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_speechOn, AudioAffect_Speech);
		printf("GameEngine::init() - Audio options set\n"); 

		// We're not in a network game yet, so set the network singleton to NULL.
		TheNetwork = NULL;

		//Create a default ini file for options if it doesn't already exist.
		//OptionPreferences prefs( TRUE );

		// If we turn m_quitting to FALSE here, then we throw away any requests to quit that
		// took place during loading. :-\ - jkmcd
		// If this really needs to take place, please make sure that pressing cancel on the audio
		// load music dialog will still cause the game to quit.
		// m_quitting = FALSE;

		// initialize the MapCache
		printf("GameEngine::init() - Creating TheMapCache\n"); 
		TheMapCache = MSGNEW("GameEngineSubsystem") MapCache;
		printf("GameEngine::init() - Calling TheMapCache->updateCache()\n"); 
		TheMapCache->updateCache();
		printf("GameEngine::init() - TheMapCache updated\n"); 


#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		sprintf(Buf, "----------------------------------------------------------------------------After TheMapCache->updateCache = %f seconds", ((double)(endTime64 - startTime64) / (double)(freq64)));
		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
#endif/////////////////////////////////////////////////////////////////////////////////////////////


		if (TheGlobalData->m_buildMapCache)
		{
			// just quit, since the map cache has already updated
			//populateMapListbox(NULL, true, true);
			m_quitting = TRUE;
		}

		// load the initial shell screen
		//TheShell->push( AsciiString("Menus\\MainMenu.wnd") );

		// This allows us to run a map from the command line
		if (TheGlobalData->m_initialFile.isEmpty() == FALSE)
		{
			AsciiString fname = TheGlobalData->m_initialFile;
			fname.toLower();

			if (fname.endsWithNoCase(".map"))
			{
				TheWritableGlobalData->m_shellMapOn = FALSE;
				TheWritableGlobalData->m_playIntro = FALSE;
				TheWritableGlobalData->m_pendingFile = TheGlobalData->m_initialFile;

				// shutdown the top, but do not pop it off the stack
	//			TheShell->hideShell();

				// send a message to the logic for a new game
				GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
				msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
				msg->appendIntegerArgument(DIFFICULTY_NORMAL);
				msg->appendIntegerArgument(0);
				InitRandom(0);
			}
		}

		//
		if (TheMapCache && TheGlobalData->m_shellMapOn)
		{
			AsciiString lowerName = TheGlobalData->m_shellMapName;
			lowerName.toLower();

			MapCache::const_iterator it = TheMapCache->find(lowerName);
			if (it == TheMapCache->end())
			{
				TheWritableGlobalData->m_shellMapOn = FALSE;
			}
		}

		if (!TheGlobalData->m_playIntro)
			TheWritableGlobalData->m_afterIntro = TRUE;

		//initDisabledMasks();

	}
	catch (ErrorCode ec)
	{
		if (ec == ERROR_INVALID_D3D)
		{
			RELEASE_CRASHLOCALIZED("ERROR:D3DFailurePrompt", "ERROR:D3DFailureMessage");
		}
	}
	catch (INIException e)
	{
		if (e.mFailureMessage)
			RELEASE_CRASH((e.mFailureMessage));
		else
			RELEASE_CRASH(("Uncaught Exception during initialization."));

	}
	catch (...)
	{
		RELEASE_CRASH(("Uncaught Exception during initialization."));
	}

	if (!TheGlobalData->m_playIntro)
		TheWritableGlobalData->m_afterIntro = TRUE;

	initKindOfMasks();
	initDisabledMasks();
	initDamageTypeFlags();

	resetSubsystems();

	HideControlBar();
}

/** -----------------------------------------------------------------------------------------------
	* Reset all necessary parts of the game engine to be ready to accept new game data
	*/
void GameEngine::reset(void)
{

	WindowLayout* background = TheWindowManager->winCreateLayout("Menus\\BlankWindow.wnd");
	DEBUG_ASSERTCRASH(background, ("We Couldn't Load Menus\\BlankWindow.wnd"));
	background->hide(FALSE);
	background->bringForward();
	background->getFirstWindow()->winClearStatus(WIN_STATUS_IMAGE);
	Bool deleteNetwork = false;
	if (TheGameLogic->isInMultiplayerGame())
		deleteNetwork = true;

	resetSubsystems();

	if (deleteNetwork)
	{
		DEBUG_ASSERTCRASH(TheNetwork, ("Deleting NULL TheNetwork!"));
		delete TheNetwork;
		TheNetwork = NULL;
	}
	if (background)
	{
		background->destroyWindows();
		deleteInstance(background);
		background = NULL;
	}
}

/// -----------------------------------------------------------------------------------------------
void GameEngine::resetSubsystems(void)
{
	// TheSuperHackers @fix xezon 09/06/2025 Reset GameLogic first to purge all world objects early.
	// This avoids potentially catastrophic issues when objects and subsystems have cross dependencies.
	TheGameLogic->reset();

	TheSubsystemList->resetAll();
}

/// -----------------------------------------------------------------------------------------------
Bool GameEngine::canUpdateGameLogic()
{
	// Must be first.
	TheGameLogic->preUpdate();

	TheFramePacer->setTimeFrozen(isTimeFrozen());
	TheFramePacer->setGameHalted(isGameHalted());

	if (TheNetwork != NULL)
	{
		return canUpdateNetworkGameLogic();
	}
	else
	{
		return canUpdateRegularGameLogic();
	}
}

/// -----------------------------------------------------------------------------------------------
Bool GameEngine::canUpdateNetworkGameLogic()
{
	DEBUG_ASSERTCRASH(TheNetwork != NULL, ("TheNetwork is NULL"));

	if (TheNetwork->isFrameDataReady())
	{
		// Important: The Network is definitely no longer stalling.
		TheFramePacer->setGameHalted(false);

		return true;
	}

	return false;
}

/// -----------------------------------------------------------------------------------------------
Bool GameEngine::canUpdateRegularGameLogic()
{
	const Bool enabled = TheFramePacer->isLogicTimeScaleEnabled();
	const Int logicTimeScaleFps = TheFramePacer->getLogicTimeScaleFps();
	const Int maxRenderFps = TheFramePacer->getFramesPerSecondLimit();

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
	const Bool useFastMode = TheGlobalData->m_TiVOFastMode;
#else	//always allow this cheat key if we're in a replay game.
	const Bool useFastMode = TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame();
#endif

	if (useFastMode || !enabled || logicTimeScaleFps >= maxRenderFps)
	{
		// Logic time scale is uncapped or larger equal Render FPS. Update straight away.
		return true;
	}
	else
	{
		// TheSuperHackers @tweak xezon 06/08/2025
		// The logic time step is now decoupled from the render update.
		const Real targetFrameTime = 1.0f / logicTimeScaleFps;
		m_logicTimeAccumulator += std::min(TheFramePacer->getUpdateTime(), targetFrameTime);

		if (m_logicTimeAccumulator >= targetFrameTime)
		{
			m_logicTimeAccumulator -= targetFrameTime;
			return true;
		}
	}

	return false;
}

/// -----------------------------------------------------------------------------------------------
DECLARE_PERF_TIMER(GameEngine_update)

/** -----------------------------------------------------------------------------------------------
 * Update the game engine by updating the GameClient and GameLogic singletons.
 */
void GameEngine::update(void)
{
	// Phase 54: Debug log
	static int updateCount = 0;
	if (updateCount < 3) {
		printf("GameEngine::update() - Starting update #%d\n", updateCount);
		
	}

	USE_PERF_TIMER(GameEngine_update)
	{
		{
			// VERIFY CRC needs to be in this code block.  Please to not pull TheGameLogic->update() inside this block.
			VERIFY_CRC

				if (updateCount < 3) {
					printf("GameEngine::update() - About to call TheRadar->UPDATE()\n");
					
				}
			TheRadar->UPDATE();

			/// @todo Move audio init, update, etc, into GameClient update
			if (updateCount < 3) {
				printf("GameEngine::update() - About to call TheAudio->UPDATE()\n");
				
			}
			TheAudio->UPDATE();
			if (updateCount < 3) {
				printf("GameEngine::update() - About to call TheGameClient->UPDATE()\n");
				
			}
			TheGameClient->UPDATE();
			if (updateCount < 3) {
				printf("GameEngine::update() - About to call TheMessageStream->propagateMessages()\n");
				
			}
			TheMessageStream->propagateMessages();

			if (TheNetwork != NULL)
			{
				TheNetwork->UPDATE();
			}

			TheCDManager->UPDATE();
		}

		const Bool canUpdate = canUpdateGameLogic();
		const Bool canUpdateLogic = canUpdate && !TheFramePacer->isGameHalted() && !TheFramePacer->isTimeFrozen();
		const Bool canUpdateScript = canUpdate && !TheFramePacer->isGameHalted();

		if (updateCount < 3) {
			printf("GameEngine::update() - canUpdate=%d, canUpdateLogic=%d, canUpdateScript=%d\n", canUpdate, canUpdateLogic, canUpdateScript);
			
			updateCount++;
		}

		if (canUpdateLogic)
		{
			TheGameClient->step();
			TheGameLogic->UPDATE();
		}
		else if (canUpdateScript)
		{
			// TheSuperHackers @info Still update the Script Engine to allow
			// for scripted camera movements while the time is frozen.
			TheScriptEngine->UPDATE();
		}
	}
}

// Horrible reference, but we really, really need to know if we are windowed.
extern bool DX8Wrapper_IsWindowed;

/** -----------------------------------------------------------------------------------------------
 * The "main loop" of the game engine. It will not return until the game exits.
 */
void GameEngine::execute(void)
{
	// Phase 54: Debug log for execute loop
	printf("GameEngine::execute() - Entering main loop, m_quitting=%d\n", m_quitting ? 1 : 0);
	
	int frameCount = 0;

#if defined(RTS_DEBUG)
	DWORD startTime = timeGetTime() / 1000;
#endif

	// pretty basic for now
	while (!m_quitting)
	{
		// Phase 54: Log first few frames
		if (frameCount < 5) {
			printf("GameEngine::execute() - Frame %d starting\n", frameCount);
			
		}

		//if (TheGlobalData->m_vTune)
		{
#ifdef PERF_TIMERS
			PerfGather::resetAll();
#endif
		}

		{

#if defined(RTS_DEBUG)
			{
				// enter only if in benchmark mode
				if (TheGlobalData->m_benchmarkTimer > 0)
				{
					DWORD currentTime = timeGetTime() / 1000;
					if (TheGlobalData->m_benchmarkTimer < currentTime - startTime)
					{
						if (TheGameLogic->isInGame())
						{
							if (TheRecorder->getMode() == RECORDERMODETYPE_RECORD)
							{
								TheRecorder->stopRecording();
							}
							TheGameLogic->clearGameData();
						}
						TheGameEngine->setQuitting(TRUE);
					}
				}
			}
#endif

			{
				try
				{
					// compute a frame
					update();
				}
				catch (INIException e)
				{
					// Release CRASH doesn't return, so don't worry about executing additional code.
					if (e.mFailureMessage)
						RELEASE_CRASH((e.mFailureMessage));
					else
						RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
				}
				catch (...)
				{
					// try to save info off
					try
					{
						if (TheRecorder && TheRecorder->getMode() == RECORDERMODETYPE_RECORD && TheRecorder->isMultiplayer())
							TheRecorder->cleanUpReplayFile();
					}
					catch (...)
					{
					}
					RELEASE_CRASH(("Uncaught Exception in GameEngine::update"));
				}
			}

			// Phase 54: Log after update
			if (frameCount < 5) {
				printf("GameEngine::execute() - Frame %d update() completed, m_quitting=%d\n", frameCount, m_quitting ? 1 : 0);
				
			}
			frameCount++;

			TheFramePacer->update();
		}

#ifdef PERF_TIMERS
		if (!m_quitting && TheGameLogic->isInGame() && !TheGameLogic->isInShellGame() && !TheGameLogic->isGamePaused())
		{
			PerfGather::dumpAll(TheGameLogic->getFrame());
			PerfGather::displayGraph(TheGameLogic->getFrame());
			PerfGather::resetAll();
		}
#endif

	}
}

/** -----------------------------------------------------------------------------------------------
	* Factory for the message stream
	*/
MessageStream* GameEngine::createMessageStream(void)
{
	// if you change this update the tools that use the engine systems
	// like GUIEdit, it creates a message stream to run in "test" mode
	return MSGNEW("GameEngineSubsystem") MessageStream;
}

//-------------------------------------------------------------------------------------------------
FileSystem* GameEngine::createFileSystem(void)
{
	return MSGNEW("GameEngineSubsystem") FileSystem;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isMultiplayerSession(void)
{
	return TheRecorder->isMultiplayer();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#define CONVERT_EXEC1	"..\\Build\\nvdxt -list buildDDS.txt -dxt5 -full -outdir Art\\Textures > buildDDS.out"

void updateTGAtoDDS()
{
	// Here's the scoop. We're going to traverse through all of the files in the Art\Textures folder
	// and determine if there are any .tga files that are newer than associated .dds files. If there
	// are, then we will re-run the compression tool on them.

	File* fp = TheLocalFileSystem->openFile("buildDDS.txt", File::WRITE | File::CREATE | File::TRUNCATE | File::TEXT);
	if (!fp) {
		return;
	}

	FilenameList files;
	AsciiString artPath;
	artPath.concat("Art");
	artPath.concat(GET_PATH_SEPARATOR());
	artPath.concat("Textures");
	artPath.concat(GET_PATH_SEPARATOR());
	TheLocalFileSystem->getFileListInDirectory(artPath.str(), "", "*.tga", files, TRUE);
	FilenameList::iterator it;
	for (it = files.begin(); it != files.end(); ++it) {
		AsciiString filenameTGA = *it;
		AsciiString filenameDDS = *it;
		FileInfo infoTGA;
		TheLocalFileSystem->getFileInfo(filenameTGA, &infoTGA);

		// skip the water textures, since they need to be NOT compressed
		filenameTGA.toLower();
		if (strstr(filenameTGA.str(), "caust"))
		{
			continue;
		}
		// and the recolored stuff.
		if (strstr(filenameTGA.str(), "zhca"))
		{
			continue;
		}

		// replace tga with dds
		filenameDDS.truncateBy(3); // tga
		filenameDDS.concat("dds");

		Bool needsToBeUpdated = FALSE;
		FileInfo infoDDS;
		if (TheFileSystem->doesFileExist(filenameDDS.str())) {
			TheFileSystem->getFileInfo(filenameDDS, &infoDDS);
			if (infoTGA.timestampHigh > infoDDS.timestampHigh ||
				(infoTGA.timestampHigh == infoDDS.timestampHigh &&
					infoTGA.timestampLow > infoDDS.timestampLow)) {
				needsToBeUpdated = TRUE;
			}
		}
		else {
			needsToBeUpdated = TRUE;
		}

		if (!needsToBeUpdated) {
			continue;
		}

		filenameTGA.concat("\n");
		fp->write(filenameTGA.str(), filenameTGA.getLength());
	}

	fp->close();

	system(CONVERT_EXEC1);
}

//-------------------------------------------------------------------------------------------------
// System things

// If we're using the Wide character version of MessageBox, then there's no additional
// processing necessary. Please note that this is a sleazy way to get this information,
// but pending a better one, this'll have to do.
extern const Bool TheSystemIsUnicode = (((void*)(::MessageBox)) == ((void*)(::MessageBoxW)));
