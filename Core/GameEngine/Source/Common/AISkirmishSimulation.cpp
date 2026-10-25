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

// GeneralsX @feature Claude 30/09/2026 AI-vs-AI skirmish runner (see header).

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/AISkirmishSimulation.h"

#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/LanNetplayOptions.h"
#include "Common/MessageStream.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/ScoreKeeper.h"
#include "GameClient/GameClient.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/AutopilotSnapshot.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"
#include "GameClient/View.h"
#include "GameClient/Display.h"
#include "GameLogic/VictoryConditions.h"
#include "GameNetwork/GameInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdarg>
#include "Common/Money.h"

//-------------------------------------------------------------------------------------------------
// GeneralsX @feature Claude 21/10/2026 Set by perFrame() when the solo run reaches -soloMinutes.
static Bool s_soloTimedOut = FALSE;
static AsciiString s_soloLastJson;
static Bool soloTimedOut() { return s_soloTimedOut; }

static SlotState difficultyCharToSlotState(char c)
{
	switch (toupper((unsigned char)c))
	{
		case 'E': return SLOT_EASY_AI;
		case 'M':
		case 'N': return SLOT_MED_AI;
		default:  return SLOT_BRUTAL_AI;	// 'H' / 'B' / anything else = Hard (Brutal)
	}
}

static const char* difficultyName(GameDifficulty d)
{
	switch (d)
	{
		case DIFFICULTY_EASY:   return "easy";
		case DIFFICULTY_NORMAL: return "normal";
		case DIFFICULTY_HARD:   return "hard";
		default:                return "unknown";
	}
}

static AsciiString jsonEscape(const AsciiString& in)
{
	AsciiString out;
	for (const char* p = in.str(); *p; ++p)
	{
		if (*p == '"' || *p == '\\') { out.concat('\\'); out.concat(*p); }
		else if (*p == '\n') out.concat("\\n");
		else out.concat(*p);
	}
	return out;
}

//-------------------------------------------------------------------------------------------------
AsciiString AISkirmishSimulation::resolveMapName(const AsciiString& wanted)
{
	if (!TheMapCache)
		return AsciiString::TheEmptyString;

	// GeneralsX @bugfix Claude 25/10/2026 MapCache::findMap() also matches display names and partial paths since the
	// October 2026 upstream sync, so a hit no longer means that "wanted" is itself a cache key
	// (e.g. "Maps\NorthAmerica\NorthAmerica.map"). Return the key it matched.
	if (const MapMetaData* found = TheMapCache->findMap(wanted))
	{
		for (MapCache::const_iterator it = TheMapCache->begin(); it != TheMapCache->end(); ++it)
		{
			if (&it->second == found)
				return it->first;
		}
		return wanted;
	}

	// Otherwise case-insensitive substring match on key or display name, multiplayer maps only.
	AsciiString needle = wanted;
	needle.toLower();
	AsciiString best;
	for (MapCache::const_iterator it = TheMapCache->begin(); it != TheMapCache->end(); ++it)
	{
		if (!it->second.m_isMultiplayer)
			continue;
		AsciiString key = it->first;
		key.toLower();
		AsciiString disp;
		disp.translate(it->second.m_displayName);
		disp.toLower();
		if (disp == needle)
			return it->first;	// exact display-name match wins outright
		if (best.isEmpty() && (strstr(key.str(), needle.str()) || strstr(disp.str(), needle.str())))
			best = it->first;
	}
	return best;
}

//-------------------------------------------------------------------------------------------------
int AISkirmishSimulation::run()
{
	const Bool headless = TheGlobalData->m_headless;

	AsciiString mapName = resolveMapName(TheGlobalData->m_aiSkirmishMap);
	if (mapName.isEmpty())
	{
		printf("AISkirmish: map \"%s\" not found. Multiplayer maps in cache:\n", TheGlobalData->m_aiSkirmishMap.str());
		if (TheMapCache)
		{
			for (MapCache::const_iterator it = TheMapCache->begin(); it != TheMapCache->end(); ++it)
			{
				if (it->second.m_isMultiplayer)
				{
					AsciiString disp;
					disp.translate(it->second.m_displayName);
					printf("  %dp  %-32s  %s\n", it->second.m_numPlayers, disp.str(), it->first.str());
				}
			}
		}
		fflush(stdout);
		return 1;
	}
	const MapMetaData* md = TheMapCache->findMap(mapName);

	// --- build TheSkirmishGameInfo the way the skirmish lobby does, minus the human slot ---
	if (!TheSkirmishGameInfo)
		TheSkirmishGameInfo = NEW SkirmishGameInfo;
	else if (TheSkirmishGameInfo->isInGame())
		TheSkirmishGameInfo->endGame();

	TheSkirmishGameInfo->init();
	TheSkirmishGameInfo->clearSlotList();
	TheSkirmishGameInfo->reset();
	TheSkirmishGameInfo->setLocalIP(TheSkirmishGameInfo->getSlot(0)->getIP());
	TheSkirmishGameInfo->enterGame();

	AsciiString slotSpec = TheGlobalData->m_aiSkirmishSlots;
	if (slotSpec.isEmpty())
		slotSpec = "H,H";

	Int slotNum = 0;
	AsciiString token;
	AsciiString slotSpecScan = slotSpec;	// nextToken consumes
	while (slotSpecScan.nextToken(&token, ",") && slotNum < MAX_SLOTS)
	{
		// token = "<E|M|H>[:<PlayerTemplateName>]"  e.g. "H" or "H:FactionAmericaAirForce"
		GameSlot s;
		s.setState(difficultyCharToSlotState(token.getCharAt(0)));
		s.setColor(-1);
		s.setStartPos(-1);
		s.setTeamNumber(-1);
		Int tmpl = PLAYERTEMPLATE_RANDOM;
		const char* colon = strchr(token.str(), ':');
		if (colon && *(colon + 1))
		{
			AsciiString factionName(colon + 1);
			tmpl = ThePlayerTemplateStore->getTemplateNumByName(factionName);
			if (tmpl < 0)
			{
				printf("AISkirmish: unknown faction template \"%s\" in slot %d, using random\n", factionName.str(), slotNum);
				tmpl = PLAYERTEMPLATE_RANDOM;
			}
		}
		s.setPlayerTemplate(tmpl);
		TheSkirmishGameInfo->setSlot(slotNum, s);
		++slotNum;
	}
	if (slotNum < 2)
	{
		printf("AISkirmish: need at least 2 AI slots (got %d) - use -aiskirmishSlots H,H\n", slotNum);
		return 1;
	}

	Int seed = (Int)GetTickCount();
	if (TheGlobalData->m_aiSkirmishSeed >= 0)
		seed = TheGlobalData->m_aiSkirmishSeed;
	else if (TheGlobalData->m_fixedSeed >= 0)
		seed = TheGlobalData->m_fixedSeed;
	TheSkirmishGameInfo->setSeed(seed);
	TheSkirmishGameInfo->setMap(mapName);
	TheSkirmishGameInfo->setMapCRC(md ? md->m_CRC : 0);
	TheSkirmishGameInfo->setMapSize(md ? md->m_filesize : 0);

	TheWritableGlobalData->m_mapName = mapName;
	TheSkirmishGameInfo->startGame(0);
	InitRandom(seed);

	printf("AISkirmish: map=\"%s\" seed=%d slots=%s headless=%d\n", mapName.str(), seed, slotSpec.str(), headless ? 1 : 0);
	fflush(stdout);

	if (headless)
	{
		// Same injection the headless replay player uses: straight onto the command list,
		// because the headless loop below never runs TheMessageStream->propagateMessages().
		GameMessage* msg = newInstance(GameMessage)(GameMessage::MSG_NEW_GAME);
		msg->appendIntegerArgument(GAME_SKIRMISH);
		msg->appendIntegerArgument(DIFFICULTY_NORMAL);	// unused for skirmish (per-slot difficulty rules)
		msg->appendIntegerArgument(0);					// rank points
		msg->appendIntegerArgument(0);					// no fps cap
		TheCommandList->appendMessage(msg);
	}
	else
	{
		// Windowed: mirror the "-file <map>" start path in GameEngine::init - keep the 3D shell
		// map from starting its own GAME_SHELL (onNewGame refuses while any game is in progress)
		// and go through TheMessageStream so the first engine frame propagates it normally.
		TheWritableGlobalData->m_shellMapOn = FALSE;
		GameMessage* msg = TheMessageStream->appendMessage(GameMessage::MSG_NEW_GAME);
		msg->appendIntegerArgument(GAME_SKIRMISH);
		msg->appendIntegerArgument(DIFFICULTY_NORMAL);
		msg->appendIntegerArgument(0);
		msg->appendIntegerArgument(30);					// normal game speed so it is watchable
	}

	const UnsignedInt startMs = GetTickCount();
	Bool timedOut = FALSE;

	if (!headless)
	{
		// Watch mode: let the normal engine loop render it. Returns when the user quits.
		TheGameEngine->execute();
	}
	else
	{
		const UnsignedInt maxFrames = (UnsignedInt)TheGlobalData->m_aiSkirmishMaxMinutes * 60u * LOGICFRAMES_PER_SECOND;
		const UnsignedInt progressInterval = 5u * 60u * LOGICFRAMES_PER_SECOND;
		Bool started = FALSE;
		for (;;)
		{
			TheGameClient->updateHeadless();
			TheGameLogic->UPDATE();

			if (TheGameLogic->isInGame())
			{
				started = TRUE;
				const UnsignedInt frame = TheGameLogic->getFrame();
				if (TheVictoryConditions && TheVictoryConditions->getEndFrame() != 0)
					break;
				if (frame != 0 && frame % progressInterval == 0)
				{
					const UnsignedInt gameSec = frame / LOGICFRAMES_PER_SECOND;
					const UnsignedInt realSec = (GetTickCount() - startMs) / 1000;
					printf("AISkirmish: game %02u:%02u  real %02u:%02u\n", gameSec / 60, gameSec % 60, realSec / 60, realSec % 60);
					fflush(stdout);
				}
				if (maxFrames != 0 && frame >= maxFrames)
				{
					timedOut = TRUE;
					break;
				}
			}
			else if (started)
			{
				break;	// game cleared itself
			}
		}
	}

	dumpStats(mapName, seed, timedOut, GetTickCount() - startMs);
	return 0;
}

//-------------------------------------------------------------------------------------------------
// GeneralsX @feature Claude 21/10/2026 Solo map (Challenge mode) with the local player on autopilot.
// The schedule and the stop are in perFrame(), so the windowed run (normal engine loop) behaves the same.
int AISkirmishSimulation::runSolo()
{
	const UnsignedInt startMs = GetTickCount();
	printf("AISkirmishSolo: map=\"%s\" schedule=%s minutes=%d headless=%d\n", TheGlobalData->m_initialFile.str(),
		lanNetplayOptions().m_soloSchedule.str(), lanNetplayOptions().m_soloMaxMinutes, TheGlobalData->m_headless ? 1 : 0);
	fflush(stdout);

	if (!TheGlobalData->m_headless)
	{
		TheGameEngine->execute();
	}
	else
	{
		Bool started = FALSE;
		UnsignedInt lastFrame = 0;
		while (!TheGameEngine->getQuitting())
		{
			// -file queued MSG_NEW_GAME on the message stream; the headless loop has to propagate it itself.
			TheMessageStream->propagateMessages();
			TheGameClient->updateHeadless();
			TheGameLogic->UPDATE();
			if (TheGameLogic->isInGame())
			{
				started = TRUE;
				lastFrame = TheGameLogic->getFrame();
				// The game clears itself when a script ends it, taking the stats along: keep a snapshot.
				if (lastFrame % (5u * LOGICFRAMES_PER_SECOND) == 0)
					s_soloLastJson = buildStatsJson(TheGlobalData->m_initialFile, 0, FALSE, GetTickCount() - startMs);
			}
			else if (started)
			{
				printf("AISkirmishSolo: the map ended the game at game second %u\n", lastFrame / LOGICFRAMES_PER_SECOND);
				break;
			}
		}
		if (!soloTimedOut() && s_soloLastJson.isNotEmpty())
		{
			printf("AISKIRMISH_RESULT %s\n", s_soloLastJson.str());
			fflush(stdout);
			return 0;
		}
	}

	dumpStats(TheGlobalData->m_initialFile, 0, soloTimedOut(), GetTickCount() - startMs);
	return 0;
}

//-------------------------------------------------------------------------------------------------
// GeneralsX @refactor Claude 20/10/2026 JSON building split out of dumpStats so the LAN netplay harness reuses it.
AsciiString AISkirmishSimulation::buildStatsJson(const AsciiString& mapName, Int seed, Bool timedOut, UnsignedInt realTimeMs)
{
	if (!ThePlayerList || !TheGameLogic)
		return AsciiString::TheEmptyString;

	const UnsignedInt frame = TheGameLogic->getFrame();
	const UnsignedInt endFrame = TheVictoryConditions ? TheVictoryConditions->getEndFrame() : 0;
	const PlayerTemplate* civTemplate = ThePlayerTemplateStore->findPlayerTemplate(NAMEKEY("FactionCivilian"));

	AsciiString json;
	AsciiString tmp;
	tmp.format("{\"map\":\"%s\",\"seed\":%d,\"frames\":%u,\"endFrame\":%u,\"gameSeconds\":%u,\"realSeconds\":%u,\"timedOut\":%s,\"players\":[",
		jsonEscape(mapName).str(), seed, frame, endFrame, frame / LOGICFRAMES_PER_SECOND, realTimeMs / 1000, timedOut ? "true" : "false");
	json.concat(tmp);

	Bool first = TRUE;
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player* p = ThePlayerList->getNthPlayer(i);
		if (!p || p == ThePlayerList->getNeutralPlayer() || !p->getPlayerTemplate())
			continue;
		if (p->getPlayerTemplate() == civTemplate || p->isPlayerObserver())
			continue;

		ScoreKeeper* sk = p->getScoreKeeper();
		AsciiString disp;
		disp.translate(p->getPlayerDisplayName());

		tmp.format("%s{\"name\":\"%s\",\"template\":\"%s\",\"side\":\"%s\",\"type\":\"%s\",\"difficulty\":\"%s\","
			"\"alive\":%s,\"victory\":%s,\"defeated\":%s,\"autopilot\":%d,"
			"\"moneyEarned\":%d,\"moneySpent\":%d,"
			"\"unitsBuilt\":%d,\"unitsLost\":%d,\"unitsDestroyed\":%d,"
			"\"buildingsBuilt\":%d,\"buildingsLost\":%d,\"buildingsDestroyed\":%d,"
			"\"techCaptured\":%d,\"factionBuildingsCaptured\":%d}",
			first ? "" : ",",
			jsonEscape(disp).str(),
			jsonEscape(p->getPlayerTemplate()->getName()).str(),
			jsonEscape(p->getSide()).str(),
			p->getPlayerType() == PLAYER_COMPUTER ? "ai" : "human",
			difficultyName(p->getPlayerDifficulty()),
			p->isPlayerActive() ? "true" : "false",
			(TheVictoryConditions && TheVictoryConditions->hasAchievedVictory(p)) ? "true" : "false",
			(TheVictoryConditions && TheVictoryConditions->hasBeenDefeated(p)) ? "true" : "false",
			p->getAutopilotLevel(),
			sk->getTotalMoneyEarned(), sk->getTotalMoneySpent(),
			sk->getTotalUnitsBuilt(), sk->getTotalUnitsLost(), sk->getTotalUnitsDestroyed(),
			sk->getTotalBuildingsBuilt(), sk->getTotalBuildingsLost(), sk->getTotalBuildingsDestroyed(),
			sk->getTotalTechBuildingsCaptured(), sk->getTotalFactionBuildingsCaptured());
		json.concat(tmp);
		first = FALSE;
	}
	json.concat("]}");
	return json;
}

//-------------------------------------------------------------------------------------------------
void AISkirmishSimulation::dumpStats(const AsciiString& mapName, Int seed, Bool timedOut, UnsignedInt realTimeMs)
{
	AsciiString json = buildStatsJson(mapName, seed, timedOut, realTimeMs);
	if (json.isEmpty())
		return;

	printf("AISKIRMISH_RESULT %s\n", json.str());
	fflush(stdout);

	if (TheGlobalData->m_aiSkirmishStatsFile.isNotEmpty())
	{
		FILE* f = fopen(TheGlobalData->m_aiSkirmishStatsFile.str(), "w");
		if (f)
		{
			fprintf(f, "%s\n", json.str());
			fclose(f);
		}
		else
		{
			printf("AISkirmish: could not write stats file %s\n", TheGlobalData->m_aiSkirmishStatsFile.str());
		}
	}
}

//-------------------------------------------------------------------------------------------------
void AISkirmishSimulation::updateWatchCamera()
{
	// GeneralsX @feature Claude 20/10/2026 Shared with the LAN netplay harness windowed mode.
	if ((!TheGlobalData->m_aiSkirmish && !lanNetplayOptions().m_active) || TheGlobalData->m_headless)
		return;
	if (!TheGameLogic || !TheGameLogic->isInGame() || !TheTacticalView || !TheTerrainLogic || !ThePlayerList)
		return;

	// Intro::doPostIntro() sets m_breakTheMovie, which W3DDisplay::draw treats as "render nothing".
	// It is normally cleared by MainMenuInit / ShellGameLoadScreen, neither of which runs on this
	// path (shell bypassed, multiplayer load screen), so the window stayed black. Clear it here.
	if (TheGlobalData->m_breakTheMovie)
		TheWritableGlobalData->m_breakTheMovie = FALSE;

	static UnsignedInt s_lastHopFrame = 0;
	static Int s_nextAI = 0;
	static Bool s_armed = FALSE;

	const UnsignedInt frame = TheGameLogic->getFrame();

	// Optional periodic in-engine screenshot (evidence / timelapse). Taken from the logic update like the
	// PrintScreen meta-event path, so it captures the last presented frame.
	const Int shotSeconds = TheGlobalData->m_aiSkirmish ? TheGlobalData->m_aiSkirmishScreenshotSeconds : lanNetplayOptions().m_screenshotSeconds;
	const UnsignedInt shotInterval = (UnsignedInt)shotSeconds * LOGICFRAMES_PER_SECOND;
	if (shotInterval != 0 && frame >= 2 * LOGICFRAMES_PER_SECOND && frame % shotInterval == 0 && TheDisplay)
	{
		TheDisplay->takeScreenShot(SCREENSHOT_PNG);
		printf("AISkirmish: screenshot at frame %u\n", frame);
		fflush(stdout);
	}

	const Int hopSeconds = TheGlobalData->m_aiSkirmish ? TheGlobalData->m_aiSkirmishCameraSeconds : lanNetplayOptions().m_cameraSeconds;
	const UnsignedInt hopInterval = (UnsignedInt)hopSeconds * LOGICFRAMES_PER_SECOND;

	if (frame < 2)
	{
		s_armed = TRUE;			// new game: hop once the engine's own start-of-game camera setup is done
		s_lastHopFrame = 0;
		s_nextAI = 0;
	}
	if (s_armed)
	{
		if (frame < LOGICFRAMES_PER_SECOND)	// let load/initial camera placement settle first
			return;
	}
	else if (hopInterval == 0 || frame - s_lastHopFrame < hopInterval)
		return;

	// Collect the AI players in order.
	Player* ais[MAX_PLAYER_COUNT];
	Int numAI = 0;
	// GeneralsX @feature Claude 20/10/2026 -lanCameraLocal: look at the local player's own base (a demo recording follows the human).
	if (lanNetplayOptions().m_active && lanNetplayOptions().m_cameraLocal)
	{
		Player* local = ThePlayerList->getLocalPlayer();
		if (local && !local->isPlayerObserver() && local->getMpStartIndex() >= 0)
			ais[numAI++] = local;
	}
	for (Int i = 0; i < ThePlayerList->getPlayerCount() && numAI < MAX_PLAYER_COUNT && !(lanNetplayOptions().m_active && lanNetplayOptions().m_cameraLocal); ++i)
	{
		Player* p = ThePlayerList->getNthPlayer(i);
		// isSkirmishAIPlayer excludes the civilian/neutral computer players that share start index 0
		if (p && p->isSkirmishAIPlayer() && p->isPlayerActive() && p->getMpStartIndex() >= 0)
			ais[numAI++] = p;
	}
	if (numAI == 0)
		return;

	Player* target = ais[s_nextAI % numAI];
	s_nextAI = (s_nextAI + 1) % numAI;

	AsciiString wpName;
	wpName.format("Player_%d_Start", target->getMpStartIndex() + 1);	// start waypoints are 1-based
	Waypoint* wp = TheTerrainLogic->getWaypointByName(wpName);
	if (!wp)
		return;

	Coord3D pos = *wp->getLocation();
	TheTacticalView->lookAt(&pos);
	printf("AISkirmish: camera -> %s (%s) at %.0f,%.0f frame %u\n", wpName.str(), target->getPlayerTemplate() ? target->getPlayerTemplate()->getName().str() : "?", pos.x, pos.y, frame);
	fflush(stdout);
	s_lastHopFrame = frame;
	s_armed = FALSE;
}

//-------------------------------------------------------------------------------------------------
static FILE* s_aiLogFile = nullptr;
static Bool s_aiLogFileTried = FALSE;

void AISkirmishSimulation::aiLog(const Player* player, const char* fmt, ...)
{
	if (!TheGlobalData || !TheGlobalData->m_aiSkirmish || TheGlobalData->m_aiSkirmishLog <= 0)
		return;

	const UnsignedInt frame = TheGameLogic ? TheGameLogic->getFrame() : 0;
	const UnsignedInt sec = frame / LOGICFRAMES_PER_SECOND;

	char who[96] = "-";
	if (player)
	{
		// "China#1": side + 1-based start slot, since both AIs are called "Hard Army".
		AsciiString side = player->getSide();
		snprintf(who, sizeof(who), "%s#%d", side.isEmpty() ? "?" : side.str(), const_cast<Player*>(player)->getMpStartIndex() + 1);
	}

	char msg[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	char line[700];
	snprintf(line, sizeof(line), "AI[%02u:%02u] %-22s %s", sec / 60, sec % 60, who, msg);
	puts(line);
	fflush(stdout);

	if (TheGlobalData->m_aiSkirmishLogFile.isNotEmpty())
	{
		if (!s_aiLogFile && !s_aiLogFileTried)
		{
			s_aiLogFileTried = TRUE;
			s_aiLogFile = fopen(TheGlobalData->m_aiSkirmishLogFile.str(), "w");
		}
		if (s_aiLogFile)
		{
			fputs(line, s_aiLogFile);
			fputc('\n', s_aiLogFile);
			fflush(s_aiLogFile);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void AISkirmishSimulation::perFrame()
{
	if (TheGlobalData && !TheGlobalData->m_aiSkirmish && !lanNetplayOptions().m_active && lanNetplayOptions().m_soloSchedule.isNotEmpty())
	{
		// -soloAutopilot: apply the schedule to the local player and stop after -soloMinutes.
		if (!TheGameLogic || !TheGameLogic->isInGame() || !ThePlayerList || TheGameLogic->isInShellGame())
			return;
		static Bool s_parsed = FALSE;
		static UnsignedInt s_frames[16];
		static Int s_levels[16];
		static Int s_masks[16];	// -1 = the level's own module set
		static Int s_count = 0;
		static Int s_next = 0;
		if (!s_parsed)
		{
			s_parsed = TRUE;
			AsciiString spec = lanNetplayOptions().m_soloSchedule;
			AsciiString token;
			while (spec.nextToken(&token, ",") && s_count < 16)
			{
				int sec = 0, lvl = 0, mask = -1;
				const int fields = sscanf(token.str(), "%d:%d:%d", &sec, &lvl, &mask);
				if (fields >= 2)
				{
					s_frames[s_count] = (UnsignedInt)sec * LOGICFRAMES_PER_SECOND;
					s_levels[s_count] = lvl;
					s_masks[s_count] = fields == 3 ? mask : -1;
					++s_count;
				}
			}
		}
		const UnsignedInt frame = TheGameLogic->getFrame();
		Player* local = ThePlayerList->getLocalPlayer();
		static Bool s_rosterShown = FALSE;
		if (!s_rosterShown && frame >= 2)
		{
			s_rosterShown = TRUE;
			for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
			{
				Player* p = ThePlayerList->getNthPlayer(i);
				if (!p)
					continue;
				printf("AISkirmishSolo: player %d name=%s side=%s template=%s type=%s%s money=%u\n", i, p->getPlayerNameKey() != NAMEKEY_INVALID ? TheNameKeyGenerator->keyToName(p->getPlayerNameKey()).str() : "?",
					p->getSide().str(), p->getPlayerTemplate() ? p->getPlayerTemplate()->getName().str() : "-", p->getPlayerType() == PLAYER_COMPUTER ? "ai" : "human",
					p == local ? " LOCAL" : "", p->getMoney()->countMoney());
			}
			fflush(stdout);
		}
		static Int s_tallyAt = 2;
		if (frame >= (UnsignedInt)s_tallyAt && s_tallyAt <= 60)
		{
			s_tallyAt = s_tallyAt == 2 ? 60 : 1000;
			Int counts[MAX_PLAYER_COUNT] = {};
			for (Object* o = TheGameLogic->getFirstObject(); o; o = o->getNextObject())
				if (o->getControllingPlayer() && o->getControllingPlayer()->getPlayerIndex() < MAX_PLAYER_COUNT)
					++counts[o->getControllingPlayer()->getPlayerIndex()];
			for (Int i = 0; i < ThePlayerList->getPlayerCount() && i < MAX_PLAYER_COUNT; ++i)
				printf("AISkirmishSolo: frame %u player %d objects=%d\n", frame, i, counts[i]);
			fflush(stdout);
		}
		while (s_next < s_count && frame >= s_frames[s_next] && local && !local->isPlayerObserver())
		{
			// Headless: the message stream does not deliver it in this loop, so put it on the command list like the
			// replay simulation does. Windowed: the normal stream path (same as the hotkey).
			GameMessage* msg = TheGlobalData->m_headless ? newInstance(GameMessage)(GameMessage::MSG_SET_AUTOPILOT) : TheMessageStream->appendMessage(GameMessage::MSG_SET_AUTOPILOT);
			msg->appendIntegerArgument(s_levels[s_next]);
			if (s_masks[s_next] >= 0)
				msg->appendIntegerArgument(s_masks[s_next]);
			if (TheGlobalData->m_headless)
				TheCommandList->appendMessage(msg);
			printf("NETPLAY autopilot request level=%d at frame %u (%s)\n", s_levels[s_next], frame, local->getPlayerTemplate() ? local->getPlayerTemplate()->getName().str() : "?");
			fflush(stdout);
			++s_next;
		}
		const Int snapEvery = lanNetplayOptions().m_soloSnapshotSeconds;
		if (snapEvery > 0 && local && frame != 0 && frame % ((UnsignedInt)snapEvery * LOGICFRAMES_PER_SECOND) == 0)
		{
			printf("AUTOPILOT_SNAPSHOT %s\n", AutopilotSnapshot::json(local).str());
			fflush(stdout);
		}
		const Int maxMinutes = lanNetplayOptions().m_soloMaxMinutes;
		if (maxMinutes > 0 && frame >= (UnsignedInt)maxMinutes * 60u * LOGICFRAMES_PER_SECOND)
		{
			s_soloTimedOut = TRUE;
			TheGameEngine->setQuitting(TRUE);
		}
		return;
	}

	if (!TheGlobalData || (!TheGlobalData->m_aiSkirmish && !lanNetplayOptions().m_active))
		return;

	updateWatchCamera();

	// Once-a-minute economy/army snapshot per AI (level >= 1).
	if (TheGlobalData->m_aiSkirmishLog > 0 && TheGameLogic && TheGameLogic->isInGame() && ThePlayerList)
	{
		const UnsignedInt frame = TheGameLogic->getFrame();
		if (frame != 0 && frame % (60u * LOGICFRAMES_PER_SECOND) == 0)
		{
			for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
			{
				Player* p = ThePlayerList->getNthPlayer(i);
				if (!p || !p->isSkirmishAIPlayer())
					continue;
				const ScoreKeeper* sk = const_cast<Player*>(p)->getScoreKeeper();
				aiLog(p, "-- status: $%u  units built %d lost %d killed %d | buildings built %d lost %d destroyed %d",
					p->getMoney()->countMoney(),
					const_cast<ScoreKeeper*>(sk)->getTotalUnitsBuilt(), const_cast<ScoreKeeper*>(sk)->getTotalUnitsLost(), const_cast<ScoreKeeper*>(sk)->getTotalUnitsDestroyed(),
					const_cast<ScoreKeeper*>(sk)->getTotalBuildingsBuilt(), const_cast<ScoreKeeper*>(sk)->getTotalBuildingsLost(), const_cast<ScoreKeeper*>(sk)->getTotalBuildingsDestroyed());
			}
		}
	}
}
