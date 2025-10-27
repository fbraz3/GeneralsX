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

#include "Common/ActionManager.h"
#include "Common/AudioAffect.h"
#include "Common/AudioEventRTS.h"
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
#include "GameClient/ControlBar.h"
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

// Phase 28.9.3: SDL2 headers removed - initialization now done in W3DDisplay only

#include "Common/version.h"


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

DeepCRCSanityCheck *TheDeepCRCSanityCheck = NULL;

void DeepCRCSanityCheck::reset(void)
{
	static Int timesThrough = 0;
	static UnsignedInt lastCRC = 0;

	AsciiString fname;
	fname.format("%sCRCAfter%dMaps.dat", TheGlobalData->getPath_UserData().str(), timesThrough);
	UnsignedInt thisCRC = TheGameLogic->getCRC( CRC_RECALC, fname );

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
GameEngine *TheGameEngine = NULL;

//-------------------------------------------------------------------------------------------------
SubsystemInterfaceList* TheSubsystemList = NULL;

//-------------------------------------------------------------------------------------------------
template<class SUBSYSTEM>
void initSubsystem(
	SUBSYSTEM*& sysref,
	AsciiString name,
	SUBSYSTEM* sys,
	Xfer *pXfer,
	const char* path1 = NULL,
	const char* path2 = NULL)
{
	printf("initSubsystem - Entered for subsystem: %s\n", name.str());
	fflush(stdout);
	sysref = sys;
	printf("initSubsystem - sysref assigned for %s\n", name.str());
	fflush(stdout);
	
	printf("initSubsystem - About to call TheSubsystemList->initSubsystem for %s\n", name.str());
	printf("initSubsystem - Parameters: path1=%s, path2=%s\n", 
		   path1 ? path1 : "NULL", path2 ? path2 : "NULL");
	fflush(stdout);
	
	try {
		TheSubsystemList->initSubsystem(sys, path1, path2, pXfer, name);
		printf("initSubsystem - TheSubsystemList->initSubsystem completed successfully for %s\n", name.str());
		fflush(stdout);
	} catch (const std::exception& e) {
		printf("initSubsystem - std::exception caught for %s: %s\n", name.str(), e.what());
		fflush(stdout);
		throw;
	} catch (...) {
		printf("initSubsystem - Unknown exception caught for %s\n", name.str());
		fflush(stdout);
		throw;
	}
	
	printf("initSubsystem - Completed for %s\n", name.str());
	fflush(stdout);
}

//-------------------------------------------------------------------------------------------------
extern HINSTANCE ApplicationHInstance;  ///< our application instance
#ifdef _WIN32
extern CComModule _Module;
#endif

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

		extern HWND ApplicationHWnd;  ///< our application window handle
		if (ApplicationHWnd) {
			//Set it twice because Win 9x does not support SetWindowTextW.
			::SetWindowText(ApplicationHWnd, titleA.str());
			::SetWindowTextW(ApplicationHWnd, title.str());
		}
	}
}

//-------------------------------------------------------------------------------------------------
GameEngine::GameEngine( void )
{
	// initialize to non garbage values
	m_logicTimeAccumulator = 0.0f;
	m_quitting = FALSE;
	m_isActive = FALSE;

#ifdef _WIN32
	_Module.Init(NULL, ApplicationHInstance, NULL);
#endif
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

#ifdef _WIN32
	_Module.Term();
#endif

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
	printf("GameEngine::init() - METHOD ENTRY POINT\n");
	fflush(stdout);
	printf("GameEngine::init() - Starting initialization\n");
	try {
		printf("GameEngine::init() - Inside try block\n");
		//create an INI object to use for loading stuff
		INI ini;
		printf("GameEngine::init() - INI object created\n");

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
	__int64 startTime64;//////////////////////////////////////////////////////////////
	__int64 endTime64,freq64;///////////////////////////////////////////////////////////
	GetPrecisionTimerTicksPerSec(&freq64);///////////////////////////////////////////////
	GetPrecisionTimer(&startTime64);////////////////////////////////////////////////////
  char Buf[256];//////////////////////////////////////////////////////////////////////
	#endif//////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - Creating SubsystemList\n");
		TheSubsystemList = MSGNEW("GameEngineSubsystem") SubsystemInterfaceList;

		printf("GameEngine::init() - Adding subsystem\n");
		TheSubsystemList->addSubsystem(this);

		// initialize the random number system
		printf("GameEngine::init() - Initializing random number system\n");
		InitRandom();

		// Create the low-level file system interface
		printf("GameEngine::init() - Creating file system\n");
		TheFileSystem = createFileSystem();
		printf("GameEngine::init() - File system created\n");

		//Kris: Patch 1.01 - November 17, 2003
		//I was unable to resolve the RTPatch method of deleting a shipped file. English, Chinese, and Korean
		//SKU's shipped with two INIZH.big files. One properly in the Run directory and the other in Run\INI\Data.
		//We need to toast the latter in order for the game to patch properly.
#ifdef _WIN32
		DeleteFile( "Data\\INI\\INIZH.big" );
#else
		unlink("Data/INI/INIZH.big");
#endif

		// not part of the subsystem list, because it should normally never be reset!
		TheNameKeyGenerator = MSGNEW("GameEngineSubsystem") NameKeyGenerator;
		TheNameKeyGenerator->init();


    	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheNameKeyGenerator  = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		// not part of the subsystem list, because it should normally never be reset!
		TheCommandList = MSGNEW("GameEngineSubsystem") CommandList;
		TheCommandList->init();

    	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheCommandList  = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		XferCRC xferCRC;
		xferCRC.open("lightCRC");


		printf("GameEngine::init() - About to initialize TheLocalFileSystem\n");
		initSubsystem(TheLocalFileSystem, "TheLocalFileSystem", createLocalFileSystem(), NULL);
		printf("GameEngine::init() - TheLocalFileSystem initialized\n");
		
		printf("GameEngine::init() - About to initialize TheArchiveFileSystem\n");
		fflush(stdout);


    	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheLocalFileSystem  = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


	initSubsystem(TheArchiveFileSystem, "TheArchiveFileSystem", createArchiveFileSystem(), NULL); // this MUST come after TheLocalFileSystem creation
	printf("GameEngine::init() - TheArchiveFileSystem initialized\n");
	fflush(stdout);

	// Phase 28.9.3: SDL2 initialization removed from here - it will be done in W3DDisplay::init()
	// when actually needed for window creation. Early init was causing Cocoa infinite loop on macOS.

	printf("GameEngine::init() - IMMEDIATELY after TheArchiveFileSystem initialized\n");
	fflush(stdout);		printf("GameEngine::init() - About to enter second DUMP_PERF_STATS section\n");
		fflush(stdout);

    	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
		printf("GameEngine::init() - Inside second DUMP_PERF_STATS ifdef\n");
		fflush(stdout);
		GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
		printf("GameEngine::init() - GetPrecisionTimer completed for ArchiveFileSystem\n");
		fflush(stdout);
		sprintf(Buf,"----------------------------------------------------------------------------After TheArchiveFileSystem  = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
		printf("GameEngine::init() - sprintf completed for ArchiveFileSystem\n");
		fflush(stdout);
  		startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
		DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
		printf("GameEngine::init() - DEBUG_LOG completed for ArchiveFileSystem\n");
		fflush(stdout);
	#else
		printf("GameEngine::init() - DUMP_PERF_STATS not defined (ArchiveFileSystem section)\n");
		fflush(stdout);
	#endif/////////////////////////////////////////////////////////////////////////////////////////////

		printf("GameEngine::init() - Completed ArchiveFileSystem DUMP_PERF_STATS section\n");
		fflush(stdout);


		// Create TheWritableGlobalData if it doesn't exist
		printf("GameEngine::init() - Checking if TheWritableGlobalData is NULL\n");
		fflush(stdout);
		if (TheWritableGlobalData == NULL) {
			printf("GameEngine::init() - Creating TheWritableGlobalData\n");
			fflush(stdout);
			
			try {
				TheWritableGlobalData = NEW GlobalData;
				printf("GameEngine::init() - TheWritableGlobalData created successfully\n");
				fflush(stdout);
			}
			catch (const std::exception& e) {
				printf("GameEngine::init() - Exception during GlobalData creation: %s\n", e.what());
				fflush(stdout);
				throw;
			}
			catch (...) {
				printf("GameEngine::init() - Unknown exception during GlobalData creation\n");
				fflush(stdout);
				throw;
			}
		}
		// DEBUG_ASSERTCRASH(TheWritableGlobalData,("TheWritableGlobalData expected to be created"));
		printf("GameEngine::init() - TheWritableGlobalData check: %s\n", TheWritableGlobalData ? "OK" : "NULL");
		fflush(stdout);
		printf("GameEngine::init() - About to initialize TheWritableGlobalData\n");
		
		printf("GameEngine::init() - About to call initSubsystem for TheWritableGlobalData\n");
		printf("GameEngine::init() - TheWritableGlobalData pointer: %p\n", TheWritableGlobalData);
		printf("GameEngine::init() - xferCRC pointer: %p\n", &xferCRC);
		printf("GameEngine::init() - TheSubsystemList pointer: %p\n", TheSubsystemList);
		fflush(stdout);
		
		// Test string creation
		printf("GameEngine::init() - Testing string creation...\n");
		fflush(stdout);
		AsciiString test_name = "TheWritableGlobalData";
		printf("GameEngine::init() - String creation successful: %s\n", test_name.str());
		fflush(stdout);
		
		// Test the initSubsystem call step by step
		printf("GameEngine::init() - About to call initSubsystem with simple parameters\n");
		fflush(stdout);
		
		try {
			printf("GameEngine::init() - Entering initSubsystem call...\n");
			fflush(stdout);
			// Use upstream parameters but keep our debug logging
			initSubsystem(TheWritableGlobalData, test_name, TheWritableGlobalData, &xferCRC, "Data\\INI\\Default\\GameData", "Data\\INI\\GameData");
			printf("GameEngine::init() - initSubsystem call completed successfully for TheWritableGlobalData\n");
			fflush(stdout);
		} catch (const std::exception& e) {
			printf("GameEngine::init() - std::exception caught during initSubsystem for TheWritableGlobalData: %s\n", e.what());
			fflush(stdout);
			throw;
		} catch (...) {
			printf("GameEngine::init() - Unknown exception caught during initSubsystem for TheWritableGlobalData\n");
			fflush(stdout);
			throw;
		}
		printf("GameEngine::init() - TheWritableGlobalData initialized\n");
		fflush(stdout);
		TheWritableGlobalData->parseCustomDefinition();
		printf("GameEngine::init() - parseCustomDefinition completed\n");
		fflush(stdout);


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After  TheWritableGlobalData = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////



	#if defined(RTS_DEBUG)
		// If we're in Debug, load the Debug settings as well.
		ini.loadFileDirectory( AsciiString( "Data\\INI\\GameDataDebug" ), INI_LOAD_OVERWRITE, NULL );
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
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Default\\Water" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Water" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Default\\Weather" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.loadFileDirectory( AsciiString( "Data\\INI\\Weather" ), INI_LOAD_OVERWRITE, &xferCRC );



	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After water INI's = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


#ifdef DEBUG_CRC
		initSubsystem(TheDeepCRCSanityCheck, "TheDeepCRCSanityCheck", MSGNEW("GameEngineSubystem") DeepCRCSanityCheck, NULL);
#endif // DEBUG_CRC
		initSubsystem(TheGameText, "TheGameText", CreateGameTextInterface(), NULL);
		updateWindowTitle();

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheGameText = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheScienceStore,"TheScienceStore", MSGNEW("GameEngineSubsystem") ScienceStore(), &xferCRC, "Data\\INI\\Default\\Science", "Data\\INI\\Science");
		initSubsystem(TheMultiplayerSettings,"TheMultiplayerSettings", MSGNEW("GameEngineSubsystem") MultiplayerSettings(), &xferCRC, "Data\\INI\\Default\\Multiplayer", "Data\\INI\\Multiplayer");
		initSubsystem(TheTerrainTypes,"TheTerrainTypes", MSGNEW("GameEngineSubsystem") TerrainTypeCollection(), &xferCRC, "Data\\INI\\Default\\Terrain", "Data\\INI\\Terrain");
		initSubsystem(TheTerrainRoads,"TheTerrainRoads", MSGNEW("GameEngineSubsystem") TerrainRoadCollection(), &xferCRC, "Data\\INI\\Default\\Roads", "Data\\INI\\Roads");
		initSubsystem(TheGlobalLanguageData,"TheGlobalLanguageData",MSGNEW("GameEngineSubsystem") GlobalLanguage, NULL); // must be before the game text
		initSubsystem(TheCDManager,"TheCDManager", CreateCDManager(), NULL);
	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheCDManager = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////
	initSubsystem(TheAudio,"TheAudio", TheGlobalData->m_headless ? NEW AudioManagerDummy : createAudioManager(), NULL);

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheAudio = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheFunctionLexicon,"TheFunctionLexicon", createFunctionLexicon(), NULL);
		initSubsystem(TheModuleFactory,"TheModuleFactory", createModuleFactory(), NULL);
		initSubsystem(TheMessageStream,"TheMessageStream", createMessageStream(), NULL);
		initSubsystem(TheSidesList,"TheSidesList", MSGNEW("GameEngineSubsystem") SidesList(), NULL);
		initSubsystem(TheCaveSystem,"TheCaveSystem", MSGNEW("GameEngineSubsystem") CaveSystem(), NULL);
		initSubsystem(TheRankInfoStore,"TheRankInfoStore", MSGNEW("GameEngineSubsystem") RankInfoStore(), &xferCRC, NULL, "Data\\INI\\Rank");
		initSubsystem(ThePlayerTemplateStore,"ThePlayerTemplateStore", MSGNEW("GameEngineSubsystem") PlayerTemplateStore(), &xferCRC, "Data\\INI\\Default\\PlayerTemplate", "Data\\INI\\PlayerTemplate");
		initSubsystem(TheParticleSystemManager,"TheParticleSystemManager", createParticleSystemManager(), NULL);

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheParticleSystemManager = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheFXListStore,"TheFXListStore", MSGNEW("GameEngineSubsystem") FXListStore(), &xferCRC, "Data\\INI\\Default\\FXList", "Data\\INI\\FXList");
		initSubsystem(TheWeaponStore,"TheWeaponStore", MSGNEW("GameEngineSubsystem") WeaponStore(), &xferCRC, NULL, "Data\\INI\\Weapon");
		initSubsystem(TheObjectCreationListStore,"TheObjectCreationListStore", MSGNEW("GameEngineSubsystem") ObjectCreationListStore(), &xferCRC, "Data\\INI\\Default\\ObjectCreationList", "Data\\INI\\ObjectCreationList");
		initSubsystem(TheLocomotorStore,"TheLocomotorStore", MSGNEW("GameEngineSubsystem") LocomotorStore(), &xferCRC, NULL, "Data\\INI\\Locomotor");
		initSubsystem(TheSpecialPowerStore,"TheSpecialPowerStore", MSGNEW("GameEngineSubsystem") SpecialPowerStore(), &xferCRC, "Data\\INI\\Default\\SpecialPower", "Data\\INI\\SpecialPower");
		initSubsystem(TheDamageFXStore,"TheDamageFXStore", MSGNEW("GameEngineSubsystem") DamageFXStore(), &xferCRC, NULL, "Data\\INI\\DamageFX");
		initSubsystem(TheArmorStore,"TheArmorStore", MSGNEW("GameEngineSubsystem") ArmorStore(), &xferCRC, NULL, "Data\\INI\\Armor");
		initSubsystem(TheBuildAssistant,"TheBuildAssistant", MSGNEW("GameEngineSubsystem") BuildAssistant, NULL);


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheBuildAssistant = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////

	// Initialize TheUpgradeCenter BEFORE TheControlBar (CommandButton.ini needs it)
	printf("GameEngine::init() - Initializing TheUpgradeCenter before TheControlBar\n");
	fflush(stdout);
	initSubsystem(TheUpgradeCenter,"TheUpgradeCenter", MSGNEW("GameEngineSubsystem") UpgradeCenter, &xferCRC, "Data\\INI\\Default\\Upgrade", "Data\\INI\\Upgrade");
	printf("GameEngine::init() - TheUpgradeCenter initialized\n");
	fflush(stdout);

	// Initialize TheThingFactory BEFORE TheControlBar (CommandButton.ini line 2225 needs it: Object = AmericaCommandCenter)
	printf("GameEngine::init() - Initializing TheThingFactory before TheControlBar\n");
	fflush(stdout);
	
	try {
		printf("W3D PROTECTION: TheThingFactory initialization starting - Critical phase\n");
		fflush(stdout);
		
		// Create ThingFactory with extra protection
		ThingFactory* thingFactory = nullptr;
		try {
			printf("W3D PROTECTION: Calling createThingFactory()\n");
			fflush(stdout);
			thingFactory = createThingFactory();
			printf("W3D PROTECTION: createThingFactory() completed successfully\n");
			fflush(stdout);
		} catch (const std::exception& e) {
			printf("W3D PROTECTION: Exception in createThingFactory(): %s\n", e.what());
			fflush(stdout);
			throw;
		} catch (...) {
			printf("W3D PROTECTION: Unknown exception in createThingFactory()\n");
			fflush(stdout);
			throw;
		}
		
		if (!thingFactory) {
			printf("W3D PROTECTION: createThingFactory() returned NULL pointer\n");
			fflush(stdout);
			throw std::runtime_error("createThingFactory returned NULL");
		}
		
		printf("W3D PROTECTION: About to call initSubsystem with ThingFactory\n");
		fflush(stdout);
		
		initSubsystem(TheThingFactory,"TheThingFactory", thingFactory, &xferCRC, "Data\\INI\\Default\\Object", "Data\\INI\\Object");
		printf("GameEngine::init() - TheThingFactory initialized successfully\n");
		fflush(stdout);
	} catch (const std::exception& e) {
		printf("GameEngine::init() - Exception during TheThingFactory init: %s\n", e.what());
		fflush(stdout);
		// Continue with degraded functionality instead of crashing
		printf("W3D PROTECTION: Continuing without TheThingFactory - degraded mode\n");
		fflush(stdout);
	} catch (...) {
		printf("GameEngine::init() - Unknown exception during TheThingFactory init\n");
		fflush(stdout);
		// Continue with degraded functionality instead of crashing
		printf("W3D PROTECTION: Continuing without TheThingFactory - degraded mode (unknown exception)\n");
		fflush(stdout);
	}

	// W3D PROTECTION: Initialize TheControlBar AFTER TheThingFactory and TheUpgradeCenter (CommandButton.ini needs both)
	printf("GameEngine::init() - CRITICAL FIX: Initializing TheControlBar after dependencies\n");
	fflush(stdout);
	if (TheControlBar == NULL) {
		printf("GameEngine::init() - Creating TheControlBar instance\n");
		fflush(stdout);
		TheControlBar = NEW ControlBar;
		printf("GameEngine::init() - TheControlBar created, calling init()\n");
		fflush(stdout);
		TheControlBar->init();
		printf("GameEngine::init() - TheControlBar initialization completed successfully\n");
		fflush(stdout);
	} else {
		printf("GameEngine::init() - TheControlBar already exists, skipping creation\n");
		fflush(stdout);
	}

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheThingFactory = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		printf("GameEngine::init() - About to initialize TheGameClient\n");
		initSubsystem(TheGameClient,"TheGameClient", createGameClient(), NULL);
		printf("GameEngine::init() - TheGameClient initialized\n");


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheGameClient = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheAI,"TheAI", MSGNEW("GameEngineSubsystem") AI(), &xferCRC,  "Data\\INI\\Default\\AIData", "Data\\INI\\AIData");
		initSubsystem(TheGameLogic,"TheGameLogic", createGameLogic(), NULL);
		initSubsystem(TheTeamFactory,"TheTeamFactory", MSGNEW("GameEngineSubsystem") TeamFactory(), NULL);
		initSubsystem(TheCrateSystem,"TheCrateSystem", MSGNEW("GameEngineSubsystem") CrateSystem(), &xferCRC, "Data\\INI\\Default\\Crate", "Data\\INI\\Crate");
		initSubsystem(ThePlayerList,"ThePlayerList", MSGNEW("GameEngineSubsystem") PlayerList(), NULL);
		initSubsystem(TheRecorder,"TheRecorder", createRecorder(), NULL);
		initSubsystem(TheRadar,"TheRadar", TheGlobalData->m_headless ? NEW RadarDummy : createRadar(), NULL);
		initSubsystem(TheVictoryConditions,"TheVictoryConditions", createVictoryConditions(), NULL);



	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheVictoryConditions = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		AsciiString fname;
		fname.format("Data\\%s\\CommandMap", GetRegistryLanguage().str());
		initSubsystem(TheMetaMap,"TheMetaMap", MSGNEW("GameEngineSubsystem") MetaMap(), NULL, fname.str(), "Data\\INI\\CommandMap");

		// Generate default meta map entries with robust protection
		try {
			printf("GameEngine::init() - Generating default MetaMap bindings\n");
			TheMetaMap->generateMetaMap();
			printf("GameEngine::init() - MetaMap defaults generated\n");
		} catch (const std::exception& e) {
			printf("W3D PROTECTION: Exception in TheMetaMap->generateMetaMap(): %s\n", e.what());
			fflush(stdout);
		} catch (...) {
			printf("W3D PROTECTION: Unknown exception in TheMetaMap->generateMetaMap()\n");
			fflush(stdout);
		}

#if defined(RTS_DEBUG)
		try {
			ini.loadFileDirectory("Data\\INI\\CommandMapDebug", INI_LOAD_MULTIFILE, NULL);
		} catch (const std::exception& e) {
			printf("GameEngine::init() - WARNING: Failed to load optional CommandMapDebug: %s\n", e.what());
			fflush(stdout);
		} catch (...) {
			printf("GameEngine::init() - WARNING: Unknown error loading optional CommandMapDebug\n");
			fflush(stdout);
		}
#endif

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
		try {
			ini.loadFileDirectory("Data\\INI\\CommandMapDemo", INI_LOAD_MULTIFILE, NULL);
		} catch (const std::exception& e) {
			printf("GameEngine::init() - WARNING: Failed to load optional CommandMapDemo: %s\n", e.what());
			fflush(stdout);
		} catch (...) {
			printf("GameEngine::init() - WARNING: Unknown error loading optional CommandMapDemo\n");
			fflush(stdout);
		}
#endif


		initSubsystem(TheActionManager,"TheActionManager", MSGNEW("GameEngineSubsystem") ActionManager(), NULL);
		//initSubsystem((CComObject<WebBrowser> *)TheWebBrowser,"(CComObject<WebBrowser> *)TheWebBrowser", (CComObject<WebBrowser> *)createWebBrowser(), NULL);
		initSubsystem(TheGameStateMap,"TheGameStateMap", MSGNEW("GameEngineSubsystem") GameStateMap, NULL );
		initSubsystem(TheGameState,"TheGameState", MSGNEW("GameEngineSubsystem") GameState, NULL );

		// Create the interface for sending game results
		initSubsystem(TheGameResultsQueue,"TheGameResultsQueue", GameResultsInterface::createNewGameResultsInterface(), NULL);


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheGameResultsQueue = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		xferCRC.close();
		TheWritableGlobalData->m_iniCRC = xferCRC.getCRC();
		DEBUG_LOG(("INI CRC is 0x%8.8X", TheGlobalData->m_iniCRC));

	TheSubsystemList->postProcessLoadAll();

	// Phase 32: Verify music files were loaded successfully
	// This check must occur AFTER postProcessLoadAll() because AudioManager::init() is called in postProcessLoad()
	printf("GameEngine::init() - AUDIO CHECK: isMusicAlreadyLoaded() = %s\n", 
		TheAudio->isMusicAlreadyLoaded() ? "true" : "false");
	fflush(stdout);
	
	if (!TheAudio->isMusicAlreadyLoaded())
	{
		printf("GameEngine::init() - WARNING: No music files found, continuing anyway (Phase 32 development)\n");
		// Original code would call: setQuitting(TRUE);
		// TODO Phase 32: Restore strict music requirement after full audio implementation
	}
	else
	{
		// Phase 32 TEST: Auto-start Shell music for playback verification
		printf("GameEngine::init() - TEST: Attempting to start 'Shell' menu music...\n");
		fflush(stdout);
		
		// Keep event alive for audio playback (static prevents re-creation)
		static AudioEventRTS* s_shellMusicTest = nullptr;
		if (!s_shellMusicTest) {
			s_shellMusicTest = new AudioEventRTS("Shell");
			if (s_shellMusicTest) {
				AudioHandle handle = TheAudio->addAudioEvent(s_shellMusicTest);
				printf("GameEngine::init() - TEST: Shell music addAudioEvent returned handle=%u (event kept alive)\n", handle);
				fflush(stdout);
			} else {
				printf("GameEngine::init() - TEST: Failed to create Shell AudioEventRTS\n");
				fflush(stdout);
			}
		}
	}

	// Upstream: Use FramePacer for FPS management
	TheFramePacer->setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

	// Phase 33: Debug audio enable/disable state
	printf("GameEngine::init() - Audio settings: audioOn=%d, musicOn=%d, soundsOn=%d, sounds3DOn=%d, speechOn=%d\n",
		   TheGlobalData->m_audioOn, TheGlobalData->m_musicOn, TheGlobalData->m_soundsOn,
		   TheGlobalData->m_sounds3DOn, TheGlobalData->m_speechOn);
	fflush(stdout);

	TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_musicOn, AudioAffect_Music);
	TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_soundsOn, AudioAffect_Sound);
	TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_sounds3DOn, AudioAffect_Sound3D);
	TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_speechOn, AudioAffect_Speech);
	
	printf("GameEngine::init() - Audio enable commands sent to TheAudio\n");
	fflush(stdout);		// We're not in a network game yet, so set the network singleton to NULL.
		TheNetwork = NULL;

		//Create a default ini file for options if it doesn't already exist.
		//OptionPreferences prefs( TRUE );

		// If we turn m_quitting to FALSE here, then we throw away any requests to quit that
		// took place during loading. :-\ - jkmcd
		// If this really needs to take place, please make sure that pressing cancel on the audio
		// load music dialog will still cause the game to quit.
		// m_quitting = FALSE;

		// initialize the MapCache
		printf("GAMEENGINE DEBUG: Creating TheMapCache\n");
		fflush(stdout);
		
		try {
			TheMapCache = MSGNEW("GameEngineSubsystem") MapCache;
			if (!TheMapCache) {
				printf("GAMEENGINE PROTECTION: Failed to create MapCache instance\n");
				fflush(stdout);
				return;
			}
			
			printf("GAMEENGINE DEBUG: MapCache created, calling updateCache\n");
			fflush(stdout);
			
			TheMapCache->updateCache();
			
			printf("GAMEENGINE DEBUG: MapCache updateCache completed successfully\n");
			fflush(stdout);
		}
		catch (std::exception& e) {
			printf("GAMEENGINE PROTECTION: Exception during MapCache initialization: %s\n", e.what());
			fflush(stdout);
			if (TheMapCache) {
				delete TheMapCache;
				TheMapCache = NULL;
			}
			return;
		}
		catch (...) {
			printf("GAMEENGINE PROTECTION: Unknown exception during MapCache initialization - continuing\n");
			fflush(stdout);
			if (TheMapCache) {
				delete TheMapCache;
				TheMapCache = NULL;
			}
			return;
		}


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheMapCache->updateCache = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
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
	//TheShell->push( AsciiString("Menus/MainMenu.wnd") );		// This allows us to run a map from the command line
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
				GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
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

		if(!TheGlobalData->m_playIntro)
			TheWritableGlobalData->m_afterIntro = TRUE;

		//initDisabledMasks();

	}
	catch (ErrorCode ec)
	{
		printf("GameEngine::init() - Caught ErrorCode exception: %d\n", ec);
		if (ec == ERROR_INVALID_D3D)
		{
			printf("GameEngine::init() - ERROR_INVALID_D3D detected\n");
			RELEASE_CRASHLOCALIZED("ERROR:D3DFailurePrompt", "ERROR:D3DFailureMessage");
		}
	}
	catch (INIException e)
	{
		printf("GameEngine::init() - Caught INIException\n");
		if (e.mFailureMessage)
			RELEASE_CRASH((e.mFailureMessage));
		else
			RELEASE_CRASH(("Uncaught Exception during initialization."));

	}
	catch (...)
	{
		printf("GameEngine::init() - Caught unknown exception\n");
		RELEASE_CRASH(("Uncaught Exception during initialization."));
	}

	if(!TheGlobalData->m_playIntro)
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
void GameEngine::reset( void )
{

	WindowLayout *background = TheWindowManager->winCreateLayout("Menus/BlankWindow.wnd");
	DEBUG_ASSERTCRASH(background,("We Couldn't Load Menus/BlankWindow.wnd"));
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
	if(background)
	{
		background->destroyWindows();
		deleteInstance(background);
		background = NULL;
	}
}

/// -----------------------------------------------------------------------------------------------
void GameEngine::resetSubsystems( void )
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
		m_logicTimeAccumulator += min(TheFramePacer->getUpdateTime(), targetFrameTime);

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
void GameEngine::update( void )
{
	printf("GAMEENGINE DEBUG: update() ENTRY - Starting subsystem updates\n");
	fflush(stdout);
	
	USE_PERF_TIMER(GameEngine_update)
	{
		{
			// VERIFY CRC needs to be in this code block.  Please to not pull TheGameLogic->update() inside this block.
			printf("GAMEENGINE DEBUG: About to VERIFY_CRC\n");
			fflush(stdout);
			VERIFY_CRC

			printf("GAMEENGINE DEBUG: About to call TheRadar->UPDATE()\n");
			fflush(stdout);
			TheRadar->UPDATE();

			/// @todo Move audio init, update, etc, into GameClient update

			printf("GAMEENGINE DEBUG: About to call TheAudio->UPDATE()\n");
			fflush(stdout);
			TheAudio->UPDATE();
			
			printf("GAMEENGINE DEBUG: About to call TheGameClient->UPDATE()\n");
			fflush(stdout);
			TheGameClient->UPDATE();
			
			printf("GAMEENGINE DEBUG: About to call TheMessageStream->propagateMessages()\n");
			fflush(stdout);
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
extern HWND ApplicationHWnd;

/** -----------------------------------------------------------------------------------------------
 * The "main loop" of the game engine. It will not return until the game exits.
 */
void GameEngine::execute( void )
{
	printf("GameEngine::execute() - ENTRY POINT - About to create FrameRateLimit\n");
	fflush(stdout);
	FrameRateLimit* frameRateLimit = new FrameRateLimit();
	printf("GameEngine::execute() - FrameRateLimit created successfully\n");
	fflush(stdout);

#if defined(RTS_DEBUG)
	DWORD startTime = timeGetTime() / 1000;
#endif

	// Debug counter for limiting startup logs (Phase 27.2 debugging)
	int loopCount = 0;

	// pretty basic for now
	while( !m_quitting )
	{
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
				if (loopCount < 3) {
					printf("GameEngine::execute() - About to call update()\n");
					fflush(stdout);
				}
				try
				{
					// compute a frame
					update();
					if (loopCount < 3) {
						printf("GameEngine::execute() - update() completed\n");
						fflush(stdout);
					}
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

			{
				{
					Bool allowFpsLimit = TheTacticalView->getTimeMultiplier()<=1 && !TheScriptEngine->isTimeFast();

		// I'm disabling this in debug because many people need alt-tab capability.  If you happen to be
		// doing performance tuning, please just change this on your local system. -MDC
		#if defined(RTS_DEBUG)
					if (allowFpsLimit)
						::Sleep(1); // give everyone else a tiny time slice.
		#endif


		#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
					allowFpsLimit &= !(!TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode);
		#else	//always allow this cheat key if we're in a replay game.
					allowFpsLimit &= !(!TheGameLogic->isGamePaused() && TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame());
		#endif
				{
					// TheSuperHackers @bugfix xezon 05/08/2025 Re-implements the frame rate limiter
					// with higher resolution counters to cap the frame rate more accurately to the desired limit.
					allowFpsLimit &= TheGlobalData->m_useFpsLimit;
					const UnsignedInt maxFps = allowFpsLimit ? TheFramePacer->getFramesPerSecondLimit() : RenderFpsPreset::UncappedFpsValue;
					if (loopCount < 3) {
						printf("GameEngine::execute() - About to call wait(maxFps=%u)\n", maxFps);
						fflush(stdout);
					}
					float updateTime = frameRateLimit->wait(maxFps);
					if (loopCount < 3) {
						printf("GameEngine::execute() - wait() returned %f\n", updateTime);
						fflush(stdout);
					}
				}				}
			}

			// Upstream: Also call FramePacer update for compatibility
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

		// Increment loop counter for debug logging (Phase 27.2 debugging)
		loopCount++;
	}
}

/** -----------------------------------------------------------------------------------------------
	* Factory for the message stream
	*/
MessageStream *GameEngine::createMessageStream( void )
{
	// if you change this update the tools that use the engine systems
	// like GUIEdit, it creates a message stream to run in "test" mode
	return MSGNEW("GameEngineSubsystem") MessageStream;
}

//-------------------------------------------------------------------------------------------------
FileSystem *GameEngine::createFileSystem( void )
{
	return MSGNEW("GameEngineSubsystem") FileSystem;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isMultiplayerSession( void )
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

	File *fp = TheLocalFileSystem->openFile("buildDDS.txt", File::WRITE | File::CREATE | File::TRUNCATE | File::TEXT);
	if (!fp) {
		return;
	}

	FilenameList files;
	TheLocalFileSystem->getFileListInDirectory("Art\\Textures\\", "", "*.tga", files, TRUE);
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
		} else {
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
#ifdef _WIN32
extern const Bool TheSystemIsUnicode = (((void*) (::MessageBox)) == ((void*) (::MessageBoxW)));
#else
extern const Bool TheSystemIsUnicode = false; // macOS uses UTF-8 by default
#endif
