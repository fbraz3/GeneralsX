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
#include "Common/BuildAssistant.h"
#include "Common/CRCDebug.h"
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
#include "Common/FrameRateLimit.h"
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
void initSubsystem(SUBSYSTEM*& sysref, AsciiString name, SUBSYSTEM* sys, Xfer *pXfer,  const char* path1 = NULL,
									 const char* path2 = NULL, const char* dirpath = NULL)
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
		TheSubsystemList->initSubsystem(sys, path1, path2, dirpath, pXfer, name);
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
	// Set the time slice size to 1 ms.
	timeBeginPeriod(1);

	// initialize to non garbage values
	m_maxFPS = BaseFps;
	m_logicTimeScaleFPS = LOGICFRAMES_PER_SECOND;
	m_updateTime = 0.0f;
	m_logicTimeAccumulator = 0.0f;
	m_quitting = FALSE;
	m_isActive = FALSE;
	m_enableLogicTimeScale = FALSE;

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

	if (TheGameLODManager)
		delete TheGameLODManager;

	Drawable::killStaticImages();

#ifdef _WIN32
	_Module.Term();
#endif

#ifdef PERF_TIMERS
	PerfGather::termPerfDump();
#endif

	// Restore the previous time slice for Windows.
	timeEndPeriod(1);
}

//-------------------------------------------------------------------------------------------------
void GameEngine::setFramesPerSecondLimit( Int fps )
{
	DEBUG_LOG(("GameEngine::setFramesPerSecondLimit() - setting max fps to %d (TheGlobalData->m_useFpsLimit == %d)", fps, TheGlobalData->m_useFpsLimit));
	m_maxFPS = fps;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getFramesPerSecondLimit( void )
{
	return m_maxFPS;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getUpdateTime()
{
	return m_updateTime;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getUpdateFps()
{
	return 1.0f / m_updateTime;
}

//-------------------------------------------------------------------------------------------------
void GameEngine::setLogicTimeScaleFps( Int fps )
{
	m_logicTimeScaleFPS = fps;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getLogicTimeScaleFps()
{
	return m_logicTimeScaleFPS;
}

//-------------------------------------------------------------------------------------------------
void GameEngine::enableLogicTimeScale( Bool enable )
{
	m_enableLogicTimeScale = enable;
}

//-------------------------------------------------------------------------------------------------
Bool GameEngine::isLogicTimeScaleEnabled()
{
	return m_enableLogicTimeScale;
}

//-------------------------------------------------------------------------------------------------
Int GameEngine::getActualLogicTimeScaleFps( void )
{
	if (TheNetwork != NULL)
	{
		return TheNetwork->getFrameRate();
	}
	else
	{
		const Bool enabled = isLogicTimeScaleEnabled();
		const Int logicTimeScaleFps = getLogicTimeScaleFps();
		const Int maxFps = getFramesPerSecondLimit();

		if (!enabled || logicTimeScaleFps >= maxFps)
		{
			return getFramesPerSecondLimit();
		}
		else
		{
			return logicTimeScaleFps;
		}
	}
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getActualLogicTimeScaleRatio()
{
	return (Real)getActualLogicTimeScaleFps() / LOGICFRAMES_PER_SECONDS_REAL;
}

//-------------------------------------------------------------------------------------------------
Real GameEngine::getActualLogicTimeScaleOverFpsRatio()
{
	// TheSuperHackers @info Clamps ratio to min 1, because the logic
	// frame rate is (typically) capped by the render frame rate.
	return min(1.0f, (Real)getActualLogicTimeScaleFps() / getUpdateFps());
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

		printf("GameEngine::init() - IMMEDIATELY after TheArchiveFileSystem initialized\n");
		fflush(stdout);

		printf("GameEngine::init() - About to enter second DUMP_PERF_STATS section\n");
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
			// Try just the basic call without INI files first
			initSubsystem(TheWritableGlobalData, test_name, TheWritableGlobalData, &xferCRC);
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
		ini.load( AsciiString( "Data\\INI\\GameDataDebug.ini" ), INI_LOAD_OVERWRITE, NULL );
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
		ini.load( AsciiString( "Data\\INI\\Default\\Water.ini" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.load( AsciiString( "Data\\INI\\Water.ini" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.load( AsciiString( "Data\\INI\\Default\\Weather.ini" ), INI_LOAD_OVERWRITE, &xferCRC );
		ini.load( AsciiString( "Data\\INI\\Weather.ini" ), INI_LOAD_OVERWRITE, &xferCRC );



	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After water INI's = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


#ifdef DEBUG_CRC
		initSubsystem(TheDeepCRCSanityCheck, "TheDeepCRCSanityCheck", MSGNEW("GameEngineSubystem") DeepCRCSanityCheck, NULL, NULL, NULL, NULL);
#endif // DEBUG_CRC
		initSubsystem(TheGameText, "TheGameText", CreateGameTextInterface(), NULL);
		updateWindowTitle();

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheGameText = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheScienceStore,"TheScienceStore", MSGNEW("GameEngineSubsystem") ScienceStore(), &xferCRC, "Data\\INI\\Default\\Science.ini", "Data\\INI\\Science.ini");
		initSubsystem(TheMultiplayerSettings,"TheMultiplayerSettings", MSGNEW("GameEngineSubsystem") MultiplayerSettings(), &xferCRC, "Data\\INI\\Default\\Multiplayer.ini", "Data\\INI\\Multiplayer.ini");
		initSubsystem(TheTerrainTypes,"TheTerrainTypes", MSGNEW("GameEngineSubsystem") TerrainTypeCollection(), &xferCRC, "Data\\INI\\Default\\Terrain.ini", "Data\\INI\\Terrain.ini");
		initSubsystem(TheTerrainRoads,"TheTerrainRoads", MSGNEW("GameEngineSubsystem") TerrainRoadCollection(), &xferCRC, "Data\\INI\\Default\\Roads.ini", "Data\\INI\\Roads.ini");
		initSubsystem(TheGlobalLanguageData,"TheGlobalLanguageData",MSGNEW("GameEngineSubsystem") GlobalLanguage, NULL); // must be before the game text
		initSubsystem(TheCDManager,"TheCDManager", CreateCDManager(), NULL);
	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheCDManager = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////
		initSubsystem(TheAudio,"TheAudio", TheGlobalData->m_headless ? NEW AudioManagerDummy : createAudioManager(), NULL);
		if (!TheAudio->isMusicAlreadyLoaded())
			setQuitting(TRUE);

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
		initSubsystem(TheRankInfoStore,"TheRankInfoStore", MSGNEW("GameEngineSubsystem") RankInfoStore(), &xferCRC, NULL, "Data\\INI\\Rank.ini");
		initSubsystem(ThePlayerTemplateStore,"ThePlayerTemplateStore", MSGNEW("GameEngineSubsystem") PlayerTemplateStore(), &xferCRC, "Data\\INI\\Default\\PlayerTemplate.ini", "Data\\INI\\PlayerTemplate.ini");
		initSubsystem(TheParticleSystemManager,"TheParticleSystemManager", createParticleSystemManager(), NULL);

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheParticleSystemManager = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheFXListStore,"TheFXListStore", MSGNEW("GameEngineSubsystem") FXListStore(), &xferCRC, "Data\\INI\\Default\\FXList.ini", "Data\\INI\\FXList.ini");
		initSubsystem(TheWeaponStore,"TheWeaponStore", MSGNEW("GameEngineSubsystem") WeaponStore(), &xferCRC, NULL, "Data\\INI\\Weapon.ini");
		initSubsystem(TheObjectCreationListStore,"TheObjectCreationListStore", MSGNEW("GameEngineSubsystem") ObjectCreationListStore(), &xferCRC, "Data\\INI\\Default\\ObjectCreationList.ini", "Data\\INI\\ObjectCreationList.ini");
		initSubsystem(TheLocomotorStore,"TheLocomotorStore", MSGNEW("GameEngineSubsystem") LocomotorStore(), &xferCRC, NULL, "Data\\INI\\Locomotor.ini");
		initSubsystem(TheSpecialPowerStore,"TheSpecialPowerStore", MSGNEW("GameEngineSubsystem") SpecialPowerStore(), &xferCRC, "Data\\INI\\Default\\SpecialPower.ini", "Data\\INI\\SpecialPower.ini");
		initSubsystem(TheDamageFXStore,"TheDamageFXStore", MSGNEW("GameEngineSubsystem") DamageFXStore(), &xferCRC, NULL, "Data\\INI\\DamageFX.ini");
		initSubsystem(TheArmorStore,"TheArmorStore", MSGNEW("GameEngineSubsystem") ArmorStore(), &xferCRC, NULL, "Data\\INI\\Armor.ini");
		initSubsystem(TheBuildAssistant,"TheBuildAssistant", MSGNEW("GameEngineSubsystem") BuildAssistant, NULL);


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheBuildAssistant = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////

	// W3D PROTECTION: Initialize TheControlBar before TheThingFactory to avoid parseCommandSetDefinition crash
	printf("GameEngine::init() - CRITICAL FIX: Initializing TheControlBar before INI parsing\n");
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


		printf("GameEngine::init() - About to initialize TheThingFactory\n");
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
			
			initSubsystem(TheThingFactory,"TheThingFactory", thingFactory, &xferCRC, "Data\\INI\\Default\\Object.ini", NULL, "Data\\INI\\Object");
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

	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheThingFactory = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheUpgradeCenter,"TheUpgradeCenter", MSGNEW("GameEngineSubsystem") UpgradeCenter, &xferCRC, "Data\\INI\\Default\\Upgrade.ini", "Data\\INI\\Upgrade.ini");
		printf("GameEngine::init() - About to initialize TheGameClient\n");
		initSubsystem(TheGameClient,"TheGameClient", createGameClient(), NULL);
		printf("GameEngine::init() - TheGameClient initialized\n");


	#ifdef DUMP_PERF_STATS///////////////////////////////////////////////////////////////////////////
	GetPrecisionTimer(&endTime64);//////////////////////////////////////////////////////////////////
	sprintf(Buf,"----------------------------------------------------------------------------After TheGameClient = %f seconds",((double)(endTime64-startTime64)/(double)(freq64)));
  startTime64 = endTime64;//Reset the clock ////////////////////////////////////////////////////////
	DEBUG_LOG(("%s", Buf));////////////////////////////////////////////////////////////////////////////
	#endif/////////////////////////////////////////////////////////////////////////////////////////////


		initSubsystem(TheAI,"TheAI", MSGNEW("GameEngineSubsystem") AI(), &xferCRC,  "Data\\INI\\Default\\AIData.ini", "Data\\INI\\AIData.ini");
		initSubsystem(TheGameLogic,"TheGameLogic", createGameLogic(), NULL);
		initSubsystem(TheTeamFactory,"TheTeamFactory", MSGNEW("GameEngineSubsystem") TeamFactory(), NULL);
		initSubsystem(TheCrateSystem,"TheCrateSystem", MSGNEW("GameEngineSubsystem") CrateSystem(), &xferCRC, "Data\\INI\\Default\\Crate.ini", "Data\\INI\\Crate.ini");
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
		fname.format("Data\\%s\\CommandMap.ini", GetRegistryLanguage().str());
		initSubsystem(TheMetaMap,"TheMetaMap", MSGNEW("GameEngineSubsystem") MetaMap(), NULL, fname.str(), "Data\\INI\\CommandMap.ini");

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
			ini.load("Data\\INI\\CommandMapDebug.ini", INI_LOAD_MULTIFILE, NULL);
		} catch (const std::exception& e) {
			printf("GameEngine::init() - WARNING: Failed to load optional CommandMapDebug.ini: %s\n", e.what());
			fflush(stdout);
		} catch (...) {
			printf("GameEngine::init() - WARNING: Unknown error loading optional CommandMapDebug.ini\n");
			fflush(stdout);
		}
#endif

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
		try {
			ini.load("Data\\INI\\CommandMapDemo.ini", INI_LOAD_MULTIFILE, NULL);
		} catch (const std::exception& e) {
			printf("GameEngine::init() - WARNING: Failed to load optional CommandMapDemo.ini: %s\n", e.what());
			fflush(stdout);
		} catch (...) {
			printf("GameEngine::init() - WARNING: Unknown error loading optional CommandMapDemo.ini\n");
			fflush(stdout);
		}
#endif


		initSubsystem(TheActionManager,"TheActionManager", MSGNEW("GameEngineSubsystem") ActionManager(), NULL);
		//initSubsystem((CComObject<WebBrowser> *)TheWebBrowser,"(CComObject<WebBrowser> *)TheWebBrowser", (CComObject<WebBrowser> *)createWebBrowser(), NULL);
		initSubsystem(TheGameStateMap,"TheGameStateMap", MSGNEW("GameEngineSubsystem") GameStateMap, NULL, NULL, NULL );
		initSubsystem(TheGameState,"TheGameState", MSGNEW("GameEngineSubsystem") GameState, NULL, NULL, NULL );

		// Create the interface for sending game results
		initSubsystem(TheGameResultsQueue,"TheGameResultsQueue", GameResultsInterface::createNewGameResultsInterface(), NULL, NULL, NULL, NULL);


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

		setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_musicOn, AudioAffect_Music);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_soundsOn, AudioAffect_Sound);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_sounds3DOn, AudioAffect_Sound3D);
		TheAudio->setOn(TheGlobalData->m_audioOn && TheGlobalData->m_speechOn, AudioAffect_Speech);

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
		//TheShell->push( AsciiString("Menus/MainMenu.wnd") );

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
}  // end init

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
		if (TheNetwork)
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
DECLARE_PERF_TIMER(GameEngine_update)

/** -----------------------------------------------------------------------------------------------
 * Update the game engine by updating the GameClient and GameLogic singletons.
 * @todo Allow the client to run as fast as possible, but limit the execution
 * of TheNetwork and TheGameLogic to a fixed framerate.
 */
void GameEngine::update( void )
{
	USE_PERF_TIMER(GameEngine_update)
	{

		{

			// VERIFY CRC needs to be in this code block.  Please to not pull TheGameLogic->update() inside this block.
			VERIFY_CRC

			TheRadar->UPDATE();

			/// @todo Move audio init, update, etc, into GameClient update

			TheAudio->UPDATE();
			TheGameClient->UPDATE();
			TheMessageStream->propagateMessages();

			if (TheNetwork != NULL)
			{
				TheNetwork->UPDATE();
			}

			TheCDManager->UPDATE();
		}

		TheGameLogic->preUpdate();

		if (TheNetwork != NULL)
		{
			if (TheNetwork->isFrameDataReady())
			{
				TheGameClient->step();
				TheGameLogic->UPDATE();
			}
		}
		else
		{
			if (!TheGameLogic->isGamePaused())
			{
				const Bool enabled = isLogicTimeScaleEnabled();
				const Int logicTimeScaleFps = getLogicTimeScaleFps();
				const Int maxRenderFps = getFramesPerSecondLimit();

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
				Bool useFastMode = TheGlobalData->m_TiVOFastMode;
#else	//always allow this cheat key if we're in a replay game.
				Bool useFastMode = TheGlobalData->m_TiVOFastMode && TheGameLogic->isInReplayGame();
#endif

				if (useFastMode || !enabled || logicTimeScaleFps >= maxRenderFps)
				{
					// Logic time scale is uncapped or larger equal Render FPS. Update straight away.
					TheGameClient->step();
					TheGameLogic->UPDATE();
				}
				else
				{
					// TheSuperHackers @tweak xezon 06/08/2025
					// The logic time step is now decoupled from the render update.
					const Real targetFrameTime = 1.0f / logicTimeScaleFps;
					m_logicTimeAccumulator += min(m_updateTime, targetFrameTime);

					if (m_logicTimeAccumulator >= targetFrameTime)
					{
						m_logicTimeAccumulator -= targetFrameTime;
						TheGameClient->step();
						TheGameLogic->UPDATE();
					}
				}
			}
		}

	}	// end perfGather

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

	// pretty basic for now
	printf("GameEngine::execute() - About to enter main loop (while !m_quitting)\n");
	fflush(stdout);
	int loopCount = 0;
	while( !m_quitting )
	{
		if (loopCount < 3) {
			printf("GameEngine::execute() - Loop iteration %d\n", loopCount);
			fflush(stdout);
		}
		loopCount++;

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
				}	// catch
			}	// perf

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
						const UnsignedInt maxFps = allowFpsLimit ? getFramesPerSecondLimit() : RenderFpsPreset::UncappedFpsValue;
						if (loopCount < 3) {
							printf("GameEngine::execute() - About to call wait(maxFps=%u)\n", maxFps);
							fflush(stdout);
						}
						m_updateTime = frameRateLimit->wait(maxFps);
						if (loopCount < 3) {
							printf("GameEngine::execute() - wait() returned %f\n", m_updateTime);
							fflush(stdout);
						}
					}

				}
			}

		}	// perfgather for execute_loop

#ifdef PERF_TIMERS
		if (!m_quitting && TheGameLogic->isInGame() && !TheGameLogic->isInShellGame() && !TheGameLogic->isGamePaused())
		{
			PerfGather::dumpAll(TheGameLogic->getFrame());
			PerfGather::displayGraph(TheGameLogic->getFrame());
			PerfGather::resetAll();
		}
#endif

	}

	delete frameRateLimit;
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
