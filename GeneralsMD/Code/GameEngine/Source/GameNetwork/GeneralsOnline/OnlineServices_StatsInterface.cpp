// GeneralsX @feature GeneralsOnline StatsInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_StatsInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/NGMP_json.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "Common/ScoreKeeper.h"
#include <cinttypes>
#include <thread>
#include <curl/curl.h>

using json = nlohmann::json;

NGMP_OnlineServices_StatsInterface::NGMP_OnlineServices_StatsInterface()
{
}

void NGMP_OnlineServices_StatsInterface::findPlayerStatsByID(int64_t userID, std::function<void(bool, PSPlayerStats)> callback, EStatsRequestPolicy policy)
{
	PSPlayerStats stats;
	bool cached = NGMP_OnlineServicesManager::getInstance().getCachedPlayerStats(userID, stats);

	if (cached && policy != EStatsRequestPolicy::BYPASS_CACHE_FORCE_REQUEST)
	{
		if (callback) callback(true, stats);
		return;
	}

	if (policy == EStatsRequestPolicy::CACHED_ONLY)
	{
		if (callback) callback(false, stats);
		return;
	}

	// Trigger async request
	NGMP_OnlineServicesManager::getInstance().requestPlayerStatsAsync(userID);
	if (callback) callback(cached, stats);
}

void NGMP_OnlineServices_StatsInterface::CommitMyOutcome(ScoreKeeper* pScoreKeeper, bool bWon)
{
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface == nullptr)
	{
		if (TheNGMPGame)
		{
			TheNGMPGame->SetCommittingOutcome(false);
		}
		return;
	}

	uint64_t currentMatchID = pLobbyInterface->GetCurrentMatchID();
	if (currentMatchID == 0)
	{
		fprintf(stderr, "[NGMP] CommitMyOutcome: matchID=0, skipping (AI game or no active match)\n");
		fflush(stderr);
		if (TheNGMPGame)
		{
			TheNGMPGame->SetCommittingOutcome(false);
		}
		return;
	}

	Int buildingsBuilt = pScoreKeeper ? pScoreKeeper->getTotalBuildingsBuilt() : 0;
	Int buildingsDestroyed = pScoreKeeper ? pScoreKeeper->getTotalBuildingsDestroyed() : 0;
	Int buildingsLost = pScoreKeeper ? pScoreKeeper->getTotalBuildingsLost() : 0;
	Int unitsBuilt = pScoreKeeper ? pScoreKeeper->getTotalUnitsBuilt() : 0;
	Int unitsDestroyed = pScoreKeeper ? pScoreKeeper->getTotalUnitsDestroyed() : 0;
	Int unitsLost = pScoreKeeper ? pScoreKeeper->getTotalUnitsLost() : 0;
	Int totalMoney = pScoreKeeper ? pScoreKeeper->getTotalMoneyEarned() : 0;

	// Resolve local player's side/faction
	int resolvedSide = -1;
	NGMPGame* myGame = pLobbyInterface->GetCurrentGame();
	if (myGame != nullptr)
	{
		GameSlot* pLocalSlot = myGame->getSlot(myGame->getLocalSlotNum());
		if (pLocalSlot != nullptr)
		{
			resolvedSide = pLocalSlot->getPlayerTemplate();
		}
	}

	fprintf(stderr, "[NGMP] CommitMyOutcome: won=%d matchID=%" PRIu64 " bldBuilt=%d bldKill=%d bldLost=%d unitBuilt=%d unitKill=%d unitLost=%d money=%d side=%d\n",
		bWon ? 1 : 0, currentMatchID, buildingsBuilt, buildingsDestroyed, buildingsLost, unitsBuilt, unitsDestroyed, unitsLost, totalMoney, resolvedSide);
	fflush(stderr);

	// GeneralsX @feature fbraz3 27/08/2026 POST match outcome to NGMP stats endpoint
	json payload = {
		{"match_id", currentMatchID},
		{"buildings_built", buildingsBuilt},
		{"buildings_killed", buildingsDestroyed},
		{"buildings_lost", buildingsLost},
		{"units_built", unitsBuilt},
		{"units_killed", unitsDestroyed},
		{"units_lost", unitsLost},
		{"total_money", totalMoney},
		{"won", bWon},
		{"side", resolvedSide},
		{"desynced", false}
	};
	std::string payloadStr = payload.dump(-1, ' ', false, json::error_handler_t::replace);
	std::string url = NGMP::GetAPIEndpoint("Lobby/Outcome");
	std::string authToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();

	std::thread([url, payloadStr, authToken]() {
		CURL* curl = curl_easy_init();
		if (!curl) {
			fprintf(stderr, "[NGMP] CommitMyOutcome: failed to initialize curl\n");
			fflush(stderr);
			NGMPEvent ev;
			ev.type = NGMPEvent::EVENT_OUTCOME_COMMITTED;
			ev.payload = "0";
			NGMP_OnlineServicesManager::getInstance().postEvent(ev);
			return;
		}

		NGMP::Internal::CurlResponse response;
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		if (!authToken.empty()) {
			std::string authHeader = "Authorization: Bearer " + authToken;
			headers = curl_slist_append(headers, authHeader.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

		CURLcode res = curl_easy_perform(curl);
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		NGMPEvent ev;
		ev.type = NGMPEvent::EVENT_OUTCOME_COMMITTED;
		if (res == CURLE_OK && httpCode == 200) {
			fprintf(stderr, "[NGMP] CommitMyOutcome: server accepted outcome (HTTP 200)\n");
			ev.payload = "1";
		} else {
			fprintf(stderr, "[NGMP] CommitMyOutcome: POST failed (res=%d, HTTP %ld, body: %s)\n", (int)res, httpCode, response.text.c_str());
			ev.payload = "0";
		}
		fflush(stderr);
		NGMP_OnlineServicesManager::getInstance().postEvent(ev);
	}).detach();
}
