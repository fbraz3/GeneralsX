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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: MainMenuUtils.cpp
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy version check, patch download, etc utils
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include <fcntl.h>

//#include "Common/Registry.h"
#include "Common/UserPreferences.h"
#include "Common/version.h"
#include "GameClient/GameText.h"
#include "GameClient/MessageBox.h"
#include "GameClient/Shell.h"
#include "GameLogic/ScriptEngine.h"

#include "GameClient/ShellHooks.h"

#include "gamespy/ghttp/ghttp.h"

#include "GameNetwork/DownloadManager.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/MainMenuUtils.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"

#include "WWDownload/Registry.h"
#include "WWDownload/urlBuilder.h"
#include <Utility/compat.h>


///////////////////////////////////////////////////////////////////////////////////////

#ifdef ALLOW_NON_PROFILED_LOGIN
#endif // ALLOW_NON_PROFILED_LOGIN
#ifdef ALLOW_NON_PROFILED_LOGIN
#endif // ALLOW_NON_PROFILED_LOGIN

return GHTTPTrue;
}

///////////////////////////////////////////////////////////////////////////////////////

static GHTTPBool numPlayersOnlineCallback(GHTTPRequest request, GHTTPResult result, char* buffer, GHTTPByteCount bufferLen, void* param)
{
	DEBUG_LOG(("numPlayersOnlineCallback() - Result=%d, buffer=[%s], len=%d", result, buffer, bufferLen));
	if (result != GHTTPSuccess)
	{
		return GHTTPTrue;
	}

	AsciiString message = buffer;
	message.trim();
	const char* s = message.reverseFindPathSeparator();
	if (!s)
	{
		return GHTTPTrue;
	}

	if (*s == GET_PATH_SEPARATOR())
		++s;

	DEBUG_LOG(("Message was '%s', trimmed to '%s'=%d", buffer, s, atoi(s)));
	HandleNumPlayersOnline(atoi(s));

	return GHTTPTrue;
}

///////////////////////////////////////////////////////////////////////////////////////

void CheckOverallStats(void)
{
#if RTS_GENERALS
	const char* const url = "http://gamestats.gamespy.com/ccgenerals/display.html";
#elif RTS_ZEROHOUR
	const char* const url = "http://gamestats.gamespy.com/ccgenzh/display.html";
#endif
	ghttpGet(url, GHTTPFalse, overallStatsCallback, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////

void CheckNumPlayersOnline(void)
{
#if RTS_GENERALS
	const char* const url = "http://launch.gamespyarcade.com/software/launch/arcadecount2.dll?svcname=ccgenerals";
#elif RTS_ZEROHOUR
	const char* const url = "http://launch.gamespyarcade.com/software/launch/arcadecount2.dll?svcname=ccgenzh";
#endif
	ghttpGet(url, GHTTPFalse, numPlayersOnlineCallback, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI asyncGethostbynameThreadFunc(void* szName)
{
	HOSTENT* he = gethostbyname((const char*)szName);

	if (he)
	{
		s_asyncDNSThreadSucceeded = TRUE;
	}
	else
	{
		s_asyncDNSThreadSucceeded = FALSE;
	}

	s_asyncDNSThreadDone = TRUE;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

int asyncGethostbyname(char* szName)
{
	static int            stat = 0;
	static unsigned long  threadid;

	if (stat == 0)
	{
		/* Kick off gethostname thread */
		s_asyncDNSThreadDone = FALSE;
		s_asyncDNSThreadHandle = CreateThread(NULL, 0, asyncGethostbynameThreadFunc, szName, 0, &threadid);

		if (s_asyncDNSThreadHandle == NULL)
		{
			return(LOOKUP_FAILED);
		}
		stat = 1;
	}
	if (stat == 1)
	{
		if (s_asyncDNSThreadDone)
		{
			/* Thread finished */
			stat = 0;
			s_asyncDNSLookupInProgress = FALSE;
			s_asyncDNSThreadHandle = NULL;
			return((s_asyncDNSThreadSucceeded) ? LOOKUP_SUCCEEDED : LOOKUP_FAILED);
		}
	}

	return(LOOKUP_INPROGRESS);
}

///////////////////////////////////////////////////////////////////////////////////////

// GameSpy's HTTP SDK has had at least 1 crash bug, so we're going to just bail and
// never try again if they crash us.  We won't be able to get back online again (we'll
// time out) but at least we'll live.
static Bool isHttpOk = TRUE;

void HTTPThinkWrapper(void)
{
	if (s_asyncDNSLookupInProgress)
	{
		Char hostname[] = "servserv.generals.ea.com";
		Int ret = asyncGethostbyname(hostname);
		switch (ret)
		{
		case LOOKUP_FAILED:
			cantConnectBeforeOnline = TRUE;
			startOnline();
			break;
		case LOOKUP_SUCCEEDED:
			reallyStartPatchCheck();
			break;
		}
	}

	if (isHttpOk)
	{
		try
		{
			ghttpThink();
		}
		catch (...)
		{
			isHttpOk = FALSE; // we can't abort the login, since we might be done with the
			// required checks and are fetching extras.  If it is a required
			// check, we'll time out normally.
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void StopAsyncDNSCheck(void)
{
	if (s_asyncDNSThreadHandle)
	{
#ifdef DEBUG_CRASHING
		Int res =
#endif
			TerminateThread(s_asyncDNSThreadHandle, 0);
		DEBUG_ASSERTCRASH(res, ("Could not terminate the Async DNS Lookup thread!"));	// Thread still not killed!
	}
	s_asyncDNSThreadHandle = NULL;
	s_asyncDNSLookupInProgress = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////

void StartPatchCheck(void)
{
	checkingForPatchBeforeGameSpy = TRUE;
	cantConnectBeforeOnline = FALSE;
	timeThroughOnline++;
	checksLeftBeforeOnline = 0;

	onlineCancelWindow = MessageBoxCancel(TheGameText->fetch("GUI:CheckingForPatches"),
		TheGameText->fetch("GUI:CheckingForPatches"), CancelPatchCheckCallbackAndReopenDropdown);

	s_asyncDNSLookupInProgress = TRUE;
	Char hostname[] = "servserv.generals.ea.com";
	Int ret = asyncGethostbyname(hostname);
	switch (ret)
	{
	case LOOKUP_FAILED:
		cantConnectBeforeOnline = TRUE;
		startOnline();
		break;
	case LOOKUP_SUCCEEDED:
		reallyStartPatchCheck();
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

static void reallyStartPatchCheck(void)
{
	checksLeftBeforeOnline = 4;

	std::string gameURL, mapURL;
	std::string configURL, motdURL;

	FormatURLFromRegistry(gameURL, mapURL, configURL, motdURL);

	std::string proxy;
	if (GetStringFromRegistry("", "Proxy", proxy))
	{
		if (!proxy.empty())
		{
			ghttpSetProxy(proxy.c_str());
		}
	}

	// check for a patch first
	DEBUG_LOG(("Game patch check: [%s]", gameURL.c_str()));
	DEBUG_LOG(("Map patch check: [%s]", mapURL.c_str()));
	DEBUG_LOG(("Config: [%s]", configURL.c_str()));
	DEBUG_LOG(("MOTD: [%s]", motdURL.c_str()));
	ghttpGet(gameURL.c_str(), GHTTPFalse, gamePatchCheckCallback, (void*)timeThroughOnline);
	ghttpGet(mapURL.c_str(), GHTTPFalse, gamePatchCheckCallback, (void*)timeThroughOnline);
	ghttpHead(configURL.c_str(), GHTTPFalse, configHeadCallback, (void*)timeThroughOnline);
	ghttpGet(motdURL.c_str(), GHTTPFalse, motdCallback, (void*)timeThroughOnline);

	// check total game stats
	CheckOverallStats();

	// check the users online
	CheckNumPlayersOnline();
}

///////////////////////////////////////////////////////////////////////////////////////

#endif // _WIN32
