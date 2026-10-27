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

// GameMain.cpp
// The main entry point for the game
// Author: Michael S. Booth, April 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FramePacer.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/ReplaySimulation.h"
#include "GameNetwork/IPEnumeration.h"
#include "GameNetwork/LANAPI.h"
#include "GameNetwork/LANPlayer.h"
#include "GameNetwork/NetworkDefs.h"

#include <cstdlib>
#include <unistd.h>

// GeneralsX @feature cheviiot 24/10/2026 PLAN-023 Gate 6 headless LAN probe (no lobby UI).
extern LANAPI *TheLAN;

namespace
{
UnsignedInt parseDottedIPv4(const char *s)
{
	UnsignedInt a = 0, b = 0, c = 0, d = 0;
	if (s == nullptr || sscanf(s, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
		return 0;
	if (a > 255 || b > 255 || c > 255 || d > 255)
		return 0;
	return (a << 24) | (b << 16) | (c << 8) | d;
}

Int runLanProbe()
{
	fprintf(stderr, "[LAN_PROBE] begin\n");
	fflush(stderr);

	if (TheLAN == nullptr)
	{
		TheLAN = NEW LANAPI();
	}
	TheLAN->init();

	IPEnumeration ips;
	EnumeratedIP *ipList = ips.getAddresses();
	if (ipList == nullptr)
	{
		fprintf(stderr, "[LAN_PROBE] FAIL no local IPv4 addresses\n");
		fflush(stderr);
		return 1;
	}

	UnsignedInt ip = 0;
	// GeneralsX @feature cheviiot 24/10/2026 Optional ECHELON_LAN_PROBE_IP for dual-netns discovery smoke.
	const char *ipEnv = getenv("ECHELON_LAN_PROBE_IP");
	if (ipEnv && ipEnv[0])
	{
		ip = parseDottedIPv4(ipEnv);
		if (ip == 0)
		{
			fprintf(stderr, "[LAN_PROBE] FAIL bad ECHELON_LAN_PROBE_IP='%s'\n", ipEnv);
			fflush(stderr);
			return 1;
		}
	}
	else
	{
		ip = ipList->getIP();
	}

	if (TheLAN->SetLocalIP(ip) == FALSE)
	{
		fprintf(stderr, "[LAN_PROBE] FAIL SetLocalIP %d.%d.%d.%d\n", PRINTF_IP_AS_4_INTS(ip));
		fflush(stderr);
		return 1;
	}

	fprintf(stderr, "[LAN_PROBE] SetLocalIP ok %d.%d.%d.%d\n", PRINTF_IP_AS_4_INTS(ip));
	fflush(stderr);

	UnicodeString probeName(L"EchelonProbe");
	const char *nameEnv = getenv("ECHELON_LAN_PROBE_NAME");
	if (nameEnv && nameEnv[0])
	{
		probeName.translate(AsciiString(nameEnv));
	}
	TheLAN->RequestSetName(probeName);

	const Bool expectPeer = (getenv("ECHELON_LAN_PROBE_EXPECT_PEER") != nullptr);
	// GeneralsX @tweak cheviiot 24/10/2026 Dual discovery: stay up after first peer sighting
	// so the other instance can still hear our LOBBY_ANNOUNCE (early exit raced the peer).
	const Int maxTicks = expectPeer ? 500 : 150;
	const Int minTicksAfterPeer = expectPeer ? 120 : 0; // ~2.4s more announce time
	Int remotePeers = 0;
	Int firstPeerTick = -1;
	for (Int i = 0; i < maxTicks; ++i)
	{
		if ((i % 15) == 0)
			TheLAN->RequestLocations();
		TheLAN->update();

		remotePeers = 0;
		for (LANPlayer *p = TheLAN->GetLobbyPlayers(); p != nullptr; p = p->getNext())
		{
			if (p->getIP() == TheLAN->GetLocalIP())
				continue;
			++remotePeers;
			fprintf(stderr, "[LAN_PROBE] peer seen ip=%d.%d.%d.%d name=%ls\n",
				PRINTF_IP_AS_4_INTS(p->getIP()), p->getName().str());
			fflush(stderr);
		}
		if (remotePeers > 0 && firstPeerTick < 0)
			firstPeerTick = i;
		if (expectPeer && firstPeerTick >= 0 && (i - firstPeerTick) >= minTicksAfterPeer)
			break;
		usleep(20000);
	}

	if (expectPeer && firstPeerTick < 0)
	{
		fprintf(stderr, "[LAN_PROBE] FAIL no remote peer discovered\n");
		fflush(stderr);
		delete TheLAN;
		TheLAN = nullptr;
		return 2;
	}

	fprintf(stderr, "[LAN_PROBE] ok peers=%d\n", remotePeers > 0 ? remotePeers : (firstPeerTick >= 0 ? 1 : 0));
	fflush(stderr);

	delete TheLAN;
	TheLAN = nullptr;
	return 0;
}
} // namespace


/**
 * This is the entry point for the game system.
 */
Int GameMain()
{
	int exitcode = 0;
	// initialize the game engine using factory function
	TheFramePacer = new FramePacer();
	TheFramePacer->enableFramesPerSecondLimit(TRUE);
	TheGameEngine = CreateGameEngine();
	TheGameEngine->init();

	if (!TheGlobalData->m_simulateReplays.empty())
	{
		exitcode = ReplaySimulation::simulateReplays(TheGlobalData->m_simulateReplays, TheGlobalData->m_simulateReplayJobs);
	}
	else if (TheGlobalData->m_lanProbe)
	{
		// GeneralsX @feature cheviiot 24/10/2026 Skip shell/execute; probe LAN bind only.
		exitcode = runLanProbe();
	}
	else
	{
		// run it
		TheGameEngine->execute();
	}

	// since execute() returned, we are exiting the game
	delete TheFramePacer;
	TheFramePacer = nullptr;
	delete TheGameEngine;
	TheGameEngine = nullptr;

	return exitcode;
}

