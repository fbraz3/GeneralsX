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

// FILE: StagingRoomGameInfo.cpp //////////////////////////////////////////////////////
// Generals GameSpy GameInfo-related code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

	{
		TheNAT = NEW NAT();
		TheNAT->attachSlotList(m_slot, getLocalSlotNum(), m_localIP);
		TheNAT->establishConnectionPaths();
	}
}

AsciiString GameSpyStagingRoom::generateGameSpyGameResultsPacket( void )
{
	Int i;
	Int endFrame = TheVictoryConditions->getEndFrame();
	Int localSlotNum = getLocalSlotNum();
	Int winningTeam = -1;
	Int numHumans = 0;
	Int numPlayers = 0;
	Int numAIs = 0;
	Int numTeamsAtGameEnd = 0;
	Int lastTeamAtGameEnd = -1;
	for (i=0; i<MAX_SLOTS; ++i)
	{
		AsciiString playerName;
		playerName.format("player%d", i);
		Player *p = ThePlayerList->findPlayerWithNameKey(NAMEKEY(playerName));
		if (p)
		{
			++numHumans;
			if (TheVictoryConditions->hasAchievedVictory(p))
			{
				winningTeam = getSlot(i)->getTeamNumber();
			}

			// check if he lasted
			GameSlot *slot = getSlot(i);
			if (!slot->disconnected())
			{
				if (slot->getTeamNumber() != lastTeamAtGameEnd || numTeamsAtGameEnd == 0)
				{
					lastTeamAtGameEnd = slot->getTeamNumber();
					++numTeamsAtGameEnd;
				}
			}
		}
		else
		{
			if (m_GameSpySlot[i].isAI())
				++numAIs;
		}
	}
	numPlayers = numHumans + numAIs;

	AsciiString mapName;
	for (i=0; i<getMap().getLength(); ++i)
	{
		char c = getMap().getCharAt(i);
		if (c == '\\')
			c = '/';
		mapName.concat(c);
	}

	AsciiString results;
	results.format("\\seed\\%d\\hostname\\%s\\mapname\\%s\\numplayers\\%d\\duration\\%d\\gamemode\\exiting\\localplayer\\%d",
		getSeed(), m_GameSpySlot[0].getLoginName().str(), mapName.str(), numPlayers, endFrame, localSlotNum);

	Int playerID = 0;
	for (i=0; i<MAX_SLOTS; ++i)
	{
		AsciiString playerName;
		playerName.format("player%d", i);
		Player *p = ThePlayerList->findPlayerWithNameKey(NAMEKEY(playerName));
		if (p)
		{
			GameSpyGameSlot *slot = &(m_GameSpySlot[i]);
			AsciiString playerName = (slot->isHuman())?slot->getLoginName():"AIPlayer";
			Int gsPlayerID = slot->getProfileID();
			Bool disconnected = slot->disconnected();

			AsciiString result = "loss";
			if (disconnected)
				result = "discon";
			else if (TheNetwork->sawCRCMismatch())
				result = "desync";
			else if (TheVictoryConditions->hasAchievedVictory(p))
				result = "win";

			AsciiString side = p->getPlayerTemplate()->getSide();
			if (side == "America")
				side = "USA";  //conform to GameSpy

			AsciiString playerStr;
			playerStr.format("\\player_%d\\%s\\pid_%d\\%d\\team_%d\\%d\\result_%d\\%s\\side_%d\\%s",
				playerID, playerName.str(), playerID, gsPlayerID, playerID, slot->getTeamNumber(),
				playerID, result.str(), playerID, side.str());
			results.concat(playerStr);

			++playerID;
		}
	}

	return results;
}

AsciiString GameSpyStagingRoom::generateLadderGameResultsPacket( void )
{
	Int i;
	Int endFrame = TheVictoryConditions->getEndFrame();
	Int localSlotNum = getLocalSlotNum();
	Bool sawGameEnd = (endFrame > 0);
	Int winningTeam = -1;
	Int numPlayers = 0;
	Int numTeamsAtGameEnd = 0;
	Int lastTeamAtGameEnd = -1;
	Player* p[MAX_SLOTS];
	for (i=0; i<MAX_SLOTS; ++i)
	{
		AsciiString playerName;
		playerName.format("player%d", i);
		p[i] = ThePlayerList->findPlayerWithNameKey(NAMEKEY(playerName));
		if (p[i])
		{
			++numPlayers;
			if (TheVictoryConditions->hasAchievedVictory(p[i]))
			{
				winningTeam = getSlot(i)->getTeamNumber();
			}

			// check if he lasted
			GameSlot *slot = getSlot(i);
			if (!slot->disconnected())
			{
				if (slot->getTeamNumber() != lastTeamAtGameEnd || numTeamsAtGameEnd == 0)
				{
					lastTeamAtGameEnd = slot->getTeamNumber();
					++numTeamsAtGameEnd;
				}
			}
		}
	}

	AsciiString results;
	results.format("seed=%d,slotNum=%d,sawDesync=%d,sawGameEnd=%d,winningTeam=%d,disconEnd=%d,duration=%d,numPlayers=%d,isQM=%d,map=%s",
		getSeed(), localSlotNum, TheNetwork->sawCRCMismatch(), sawGameEnd, winningTeam, (numTeamsAtGameEnd < 2),
		endFrame, numPlayers, m_isQM, TheGameState->realMapPathToPortableMapPath(getMap()).str());

	AsciiString tempStr;
	tempStr.format(",ladderIP=%s,ladderPort=%d", getLadderIP().str(), getLadderPort());
	results.concat(tempStr);

	Int playerID = 0;
	for (i=0; i<MAX_SLOTS; ++i)
	{
		AsciiString playerName;
		playerName.format("player%d", i);
		if (p[i])
		{
			GameSpyGameSlot *slot = &(m_GameSpySlot[i]);
			ScoreKeeper *keeper = p[i]->getScoreKeeper();
			AsciiString playerName = slot->getLoginName();
			Int gsPlayerID = slot->getProfileID();
			PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(gsPlayerID);
			Int fps = TheNetwork->getAverageFPS();
			Int unitsKilled = keeper->getTotalUnitsDestroyed();
			Int unitsLost = keeper->getTotalUnitsLost();
			Int unitsBuilt = keeper->getTotalUnitsBuilt();
			Int buildingsKilled = keeper->getTotalBuildingsDestroyed();
			Int buildingsLost = keeper->getTotalBuildingsLost();
			Int buildingsBuilt = keeper->getTotalBuildingsBuilt();
			Int earnings = keeper->getTotalMoneyEarned();
			Int techCaptured = keeper->getTotalTechBuildingsCaptured();
			Bool disconnected = slot->disconnected();

			AsciiString playerStr;
			playerStr.format(",player%d=%s,playerID%d=%d,locale%d=%d",
				playerID, playerName.str(), playerID, gsPlayerID, playerID, stats.locale);
			results.concat(playerStr);
			playerStr.format(",unitsKilled%d=%d,unitsLost%d=%d,unitsBuilt%d=%d",
				playerID, unitsKilled, playerID, unitsLost, playerID, unitsBuilt);
			results.concat(playerStr);
			playerStr.format(",buildingsKilled%d=%d,buildingsLost%d=%d,buildingsBuilt%d=%d",
				playerID, buildingsKilled, playerID, buildingsLost, playerID, buildingsBuilt);
			results.concat(playerStr);
#if RTS_GENERALS
			playerStr.format(",fps%d=%d,cash%d=%d,capturedTech%d=%d,discon%d=%d,side%d=%s,team%d=%d",
				playerID, fps, playerID, earnings, playerID, techCaptured, playerID, disconnected, playerID, p[i]->getPlayerTemplate()->getSide().str(), playerID, slot->getTeamNumber());
#elif RTS_ZEROHOUR
			playerStr.format(",fps%d=%d,cash%d=%d,capturedTech%d=%d,discon%d=%d,side%d=%s",
				playerID, fps, playerID, earnings, playerID, techCaptured, playerID, disconnected, playerID, p[i]->getPlayerTemplate()->getSide().str());
#endif
			results.concat(playerStr);

			++playerID;
		}
	}

	// Add a trailing size value (so the server can ensure it got the entire packet)
	results.concat(",size=");
	int resultsLen = results.getLength()+10;
	AsciiString tail;
	tail.format("%10.10d", resultsLen);
	results.concat(tail);

	return results;
}

void GameSpyStagingRoom::launchGame( void )
{
	setGameInProgress(TRUE);

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSpyGameSlot *slot = getGameSpySlot(i);
		if (slot->isHuman())
		{
			if (TheGameSpyInfo->didPlayerPreorder(slot->getProfileID()))
				markPlayerAsPreorder(i);
		}
	}

	// Set up the game network
	AsciiString user;
	AsciiString userList;
	DEBUG_ASSERTCRASH(TheNetwork == NULL, ("For some reason TheNetwork isn't NULL at the start of this game.  Better look into that."));

	delete TheNetwork;
	TheNetwork = NULL;

	// Time to initialize TheNetwork for this game.
	TheNetwork = NetworkInterface::createNetwork();
	TheNetwork->init();

	TheNetwork->setLocalAddress(getLocalIP(), (TheNAT)?TheNAT->getSlotPort(getLocalSlotNum()):8888);
	if (TheNAT)
		TheNetwork->attachTransport(TheNAT->getTransport());
	else
		TheNetwork->initTransport();

	TheNetwork->parseUserList(this);


	if (TheGameLogic->isInGame()) {
		TheGameLogic->clearGameData();
	}

	Bool filesOk = DoAnyMapTransfers(this);

	// see if we really have the map.  if not, back out.
	TheMapCache->updateCache();
	if (!filesOk || TheMapCache->findMap(getMap()) == NULL)
	{
		DEBUG_LOG(("After transfer, we didn't really have the map.  Bailing..."));

		delete TheNetwork;
		TheNetwork = NULL;

		GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:CouldNotTransferMap"));

		void PopBackToLobby( void );
		PopBackToLobby();
		return;
	}


	// shutdown the top, but do not pop it off the stack
//		TheShell->hideShell();
	// setup the Global Data with the Map and Seed
	TheWritableGlobalData->m_pendingFile = TheGameSpyGame->getMap();

	// send a message to the logic for a new game
	GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
	msg->appendIntegerArgument(GAME_INTERNET);

	TheWritableGlobalData->m_useFpsLimit = false;

	// Set the random seed
	InitGameLogicRandom( getSeed() );
	DEBUG_LOG(("InitGameLogicRandom( %d )", getSeed()));

	// mark us as "Loading" in the buddy list
	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_SETSTATUS;
	req.arg.status.status = GP_PLAYING;
	strcpy(req.arg.status.statusString, "Loading");
	sprintf(req.arg.status.locationString, "%s", WideCharStringToMultiByte(TheGameSpyGame->getGameName().str()).c_str());
	TheGameSpyBuddyMessageQueue->addRequest(req);

	delete TheNAT;
	TheNAT = NULL;
}

void GameSpyStagingRoom::reset(void)
{
#ifdef DEBUG_LOGGING
	if (this == TheGameSpyGame)
	{
		WindowLayout *theLayout = TheShell->findScreenByFilename("Menus/GameSpyGameOptionsMenu.wnd");
		if (theLayout)
		{
			DEBUG_LOG(("Resetting TheGameSpyGame on the game options menu!"));
		}
	}
#endif
	GameInfo::reset();
}

#endif // _WIN32
