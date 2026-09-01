#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/NextGenTransport.h"
#include "GameLogic/VictoryConditions.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/FileTransfer.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameText.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "Common/RandomValue.h"
#include "GameNetwork/NetworkInterface.h"
#include "Common/GlobalData.h"
#include "GameClient/View.h"
#include "GameClient/InGameUI.h"

void showNotificationBox( AsciiString nick, UnicodeString message);

NGMPGame* TheNGMPGame = nullptr;

NGMPGameSlot::NGMPGameSlot()
{
	m_profileID = 0;
	m_wins = 0;
	m_losses = 0;
	m_rankPoints = 0;
	m_favoriteSide = 0;
	m_pingInt = 0;
	m_pingStr.clear();
	m_userID = -1;
}

NGMPGame::NGMPGame()
{
	cleanUpSlotPointers();
	setLocalIP(0);
	m_ladderIP.clear();
	m_ladderPort = 0;

	enterGame();

	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface != nullptr)
	{
		SyncWithLobby(pLobbyInterface->GetCurrentLobby());
		UpdateSlotsFromCurrentLobby();
	}
}

NGMPGame::~NGMPGame()
{
	if (TheTacticalView)
	{
		TheTacticalView->setDefaultView(DEG_TO_RADF(TheGlobalData->m_cameraPitch),
			DEG_TO_RADF(TheGlobalData->m_cameraYaw),
			1.0f);
	}
}

void NGMPGame::SyncWithLobby(LobbyEntry& lobby)
{
	if (TheMapCache)
	{
		TheMapCache->updateCache();
		const MapMetaData *pMap = TheMapCache->findMap(lobby.map_path.c_str());
		if (!pMap && !lobby.map_name.empty())
		{
			pMap = TheMapCache->findMap(lobby.map_name.c_str());
		}

		if (pMap)
		{
			getGameSpySlot(0)->setMapAvailability(TRUE);
			setMapCRC(pMap->m_CRC);
			setMapSize(pMap->m_filesize);
			setMap(pMap->m_fileName.isNotEmpty() ? pMap->m_fileName : AsciiString(lobby.map_path.c_str()));
		}
		else
		{
			setMap(lobby.map_path.c_str());
		}
	}
	else
	{
		setMap(lobby.map_path.c_str());
	}

	setSuperweaponRestriction(lobby.limit_superweapons);
	setOldFactionsOnly(lobby.vanilla_teams);
	setUseStats(lobby.track_stats);
	setSeed(lobby.rng_seed);
	setAllowObservers(lobby.allow_observers);
	setHasPassword(lobby.passworded);
	setExeCRC(lobby.exe_crc);
	setIniCRC(lobby.ini_crc);

	UnicodeString lobbyName = NGMP::UTF8ToUnicode(lobby.name);
	setGameName(lobbyName);

	Money startingCash;
	startingCash.deposit(lobby.starting_cash, FALSE);
	setStartingCash(startingCash);
}

void NGMPGame::UpdateSlotsFromCurrentLobby()
{
	if (m_inProgress)
	{
		return;
	}

	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface == nullptr)
	{
		return;
	}

	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		LobbyMemberEntry pLobbyMember = pLobbyInterface->GetRoomMemberFromIndex(i);

		int playerTemplate = -1;
		if (pLobbyMember.side == -1)
		{
			playerTemplate = PLAYERTEMPLATE_RANDOM;
		}
		else
		{
			playerTemplate = pLobbyMember.side;
		}

		if (pLobbyMember.m_SlotState != SlotState::SLOT_OPEN && pLobbyMember.m_SlotState != SlotState::SLOT_CLOSED)
		{
			bool bIsAI = (pLobbyMember.m_SlotState == SlotState::SLOT_EASY_AI || pLobbyMember.m_SlotState == SlotState::SLOT_MED_AI || pLobbyMember.m_SlotState == SlotState::SLOT_BRUTAL_AI);

			if (pLobbyMember.m_SlotIndex >= MAX_SLOTS)
			{
				continue;
			}

			NGMPGameSlot* slot = (NGMPGameSlot*)getSlot(pLobbyMember.m_SlotIndex);
			if (slot == nullptr)
			{
				continue;
			}

			UnicodeString uName = NGMP::UTF8ToUnicode(pLobbyMember.display_name);
			slot->setState((SlotState)pLobbyMember.m_SlotState, uName, pLobbyMember.m_SlotIndex);

			slot->setColor(pLobbyMember.color);
			slot->setTeamNumber(pLobbyMember.team);
			slot->setStartPos(pLobbyMember.startpos);
			slot->setPlayerTemplate(playerTemplate);
			slot->setIP(pLobbyMember.m_SlotIndex);

			if (!bIsAI)
			{
				if (pLobbyMember.m_bIsReady)
				{
					slot->setAccept();
				}
				else
				{
					slot->unAccept();
				}

				slot->setMapAvailability(pLobbyMember.has_map);
				slot->m_userID = pLobbyMember.user_id;
			}
			else
			{
				slot->setAccept();
				slot->setMapAvailability(true);
				slot->m_userID = -1;
			}
		}
		else
		{
			NGMPGameSlot* slot = (NGMPGameSlot*)getSlot(i);
			if (slot)
			{
				slot->setState((SlotState)pLobbyMember.m_SlotState);
			}
		}
	}
}

void NGMPGame::cleanUpSlotPointers(void)
{
	for (Int i = 0; i < MAX_SLOTS; ++i)
		setSlotPointer(i, &m_Slots[i]);
}

NGMPGameSlot* NGMPGame::getGameSpySlot(Int index)
{
	GameSlot* slot = getSlot(index);
	DEBUG_ASSERTCRASH(slot && (slot == &(m_Slots[index])), ("Bad game slot pointer\n"));
	return (NGMPGameSlot*)slot;
}

void NGMPGame::init(void)
{
	GameInfo::init();
	UpdateSlotsFromCurrentLobby();
}

void NGMPGame::setPingString(AsciiString pingStr)
{
	m_pingStr = pingStr;
	m_pingInt = 0;
}

Bool NGMPGame::amIHost(void) const
{
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	return pLobbyInterface == nullptr ? false : pLobbyInterface->IsHost();
}

void NGMPGame::resetAccepted(void)
{
	GameInfo::resetAccepted();
}

Int NGMPGame::getLocalSlotNum(void) const
{
	if (!m_inGame)
		return -1;

	NGMP_OnlineServices_AuthInterface* pAuthInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_AuthInterface>();
	if (pAuthInterface == nullptr)
	{
		return -1;
	}

	int64_t localUserID = pAuthInterface->GetUserID();

	for (Int i = 0; i < MAX_SLOTS; ++i)
	{
		const NGMPGameSlot* slot = (const NGMPGameSlot*)getConstSlot(i);
		if (slot == NULL) {
			continue;
		}
		if (slot->m_userID == localUserID)
		{
			return i;
		}
	}

	return -1;
}

void NGMPGame::startGame(Int gameID)
{
	// GeneralsX @bugfix fbraz3 30/08/2026 Sync slots from lobby before launching to
	//   ensure m_inGame is set and slot userIDs are populated for getLocalSlotNum()
	fprintf(stderr, "[NGMP] NGMPGame::startGame - synchronizing slots from lobby before launch (gameID=%d)\n", gameID);
	fflush(stderr);

	NGMP_OnlineServices_LobbyInterface* pLobbyInterface =
		NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface != nullptr)
	{
		LobbyEntry& currentLobby = pLobbyInterface->GetCurrentLobby();
		SyncWithLobby(currentLobby);
		UpdateSlotsFromCurrentLobby();
	}

	// Ensure m_inGame is set (may be false on Guest if enterGame() was never called)
	if (!m_inGame)
	{
		fprintf(stderr, "[NGMP] NGMPGame::startGame - m_inGame was false, calling enterGame()\n");
		fflush(stderr);
		enterGame();
	}

	UnsignedInt localIP = 1337;
	setLocalIP(localIP);

	DEBUG_LOG(("NGMPGame::startGame - game id = %d\n", gameID));
	launchGame();
}

AsciiString NGMPGame::generateGameSpyGameResultsPacket(void)
{
	return AsciiString();
}

AsciiString NGMPGame::generateLadderGameResultsPacket(void)
{
	return AsciiString();
}

void NGMPGame::launchGame(void)
{
	setGameInProgress(TRUE);

	if (TheNetwork != NULL) {
		delete TheNetwork;
		TheNetwork = NULL;
	}

	TheNetwork = NetworkInterface::createNetwork();
	TheNetwork->init();
	TheNetwork->setLocalAddress(getLocalIP(), 8888);
	NextGenTransport* pTransport = new NextGenTransport;
	pTransport->init(getLocalIP(), 8888);
	TheNetwork->attachTransport(pTransport);
	TheNetwork->parseUserList(this);

	if (TheGameLogic->isInGame()) {
		TheGameLogic->clearGameData();
	}

	Bool filesOk = DoAnyMapTransfers(this);

	TheMapCache->updateCache();
	if (!filesOk || TheMapCache->findMap(getMap()) == NULL)
	{
		if (TheNetwork != NULL) {
			delete TheNetwork;
			TheNetwork = NULL;
		}
		GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:CouldNotTransferMap"));

		void PopBackToLobby(void);
		PopBackToLobby();
		return;
	}

	if (TheTacticalView)
	{
		TheTacticalView->setDefaultView(DEG_TO_RADF(TheGlobalData->m_cameraPitch),
			DEG_TO_RADF(TheGlobalData->m_cameraYaw),
			1.0f);
	}

	TheWritableGlobalData->m_pendingFile = getMap();

	GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
	msg->appendIntegerArgument(GAME_INTERNET);

	InitRandom(getSeed());

	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface != nullptr)
	{
		LobbyEntry& currentLobby = pLobbyInterface->GetCurrentLobby();
		std::string strMapName = currentLobby.map_name;
		const std::string strExt = ".map";
		size_t pos = strMapName.find(strExt);
		if (pos != std::string::npos) { strMapName.erase(pos, strExt.size()); }

		UnicodeString notifMsg;
		notifMsg.format(L"Map: %hs\nPress F5 or INSERT to open the communicator.", strMapName.c_str());
		showNotificationBox(AsciiString::TheEmptyString, notifMsg);
	}
}

void NGMPGame::reset(void)
{
	GameInfo::reset();
}

void NGMPGame::StartCountdown()
{
	m_bCountdownStarted = true;
	m_countdownStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	m_countdownLastCheckTime = m_countdownStartTime;

	std::shared_ptr<WebSocket> pWS = NGMP_OnlineServicesManager::GetWebSocket();
	if (pWS != nullptr)
	{
		pWS->SendData_CountdownStarted();
	}
}

Bool NGMPGame::canKickOnObserversDisabled()
{
	if (getAllowObservers() || TheGameLogic->getGameMode() != GAME_INTERNET || TheGameLogic->getFrame() < 2)
		return FALSE;

	Player *localPlayer = ThePlayerList->getLocalPlayer();
	if (!localPlayer || localPlayer->isPlayerObserver() || !TheVictoryConditions->hasSinglePlayerBeenDefeated(localPlayer))
		return FALSE;

	if (!amIHost())
		return TRUE;

	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *otherPlayer = ThePlayerList->getNthPlayer(i);
		if (!otherPlayer || otherPlayer == localPlayer)
			continue;

		if (otherPlayer->getRelationship(localPlayer->getDefaultTeam()) == ALLIES && !TheVictoryConditions->hasSinglePlayerBeenDefeated(otherPlayer))
			return TRUE;
	}

	return FALSE;
}
