// GeneralsX @feature GeneralsOnline StatsInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_StatsInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/NGMP_json.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "Common/ScoreKeeper.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <thread>
#include <curl/curl.h>

using json = nlohmann::json;

// GeneralsX @bugfix fbraz3 03/10/2026 Resolve local player's side/faction, falling back to in-game PlayerTemplate if Random
static int ResolveLocalPlayerSide(NGMP_OnlineServices_LobbyInterface* pLobbyInterface)
{
	int resolvedSide = -1;
	if (pLobbyInterface != nullptr)
	{
		NGMPGame* myGame = pLobbyInterface->GetCurrentGame();
		if (myGame != nullptr)
		{
			GameSlot* pLocalSlot = myGame->getSlot(myGame->getLocalSlotNum());
			if (pLocalSlot != nullptr)
			{
				resolvedSide = pLocalSlot->getPlayerTemplate();
			}
		}
	}

	// If side was Random (-1) or invalid, resolve actual rolled faction from active in-game Player
	if (resolvedSide < 0 && ThePlayerList != nullptr)
	{
		Player* localPlayer = ThePlayerList->getLocalPlayer();
		if (localPlayer != nullptr)
		{
			const PlayerTemplate* myTemplate = localPlayer->getPlayerTemplate();
			if (myTemplate != nullptr && ThePlayerTemplateStore != nullptr)
			{
				for (Int ptIdx = 0; ptIdx < ThePlayerTemplateStore->getPlayerTemplateCount(); ++ptIdx)
				{
					if (ThePlayerTemplateStore->getNthPlayerTemplate(ptIdx) == myTemplate)
					{
						resolvedSide = ptIdx;
						break;
					}
				}
			}
		}
	}

	return resolvedSide;
}

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
	int resolvedSide = ResolveLocalPlayerSide(pLobbyInterface);

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
	uint32_t tokenVersion = NGMP_OnlineServicesManager::getInstance().getAuthTokenVersion();

	std::thread([url, payloadStr, authToken, tokenVersion]() {
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

		// GeneralsX @bugfix fbraz3 19/09/2026 Retry outcome post if token expired during long match (HTTP 401)
		if (httpCode == 401) {
			fprintf(stderr, "[NGMP] CommitMyOutcome: 401 Unauthorized (session expired mid-game), refreshing token...\n");
			fflush(stderr);
			if (NGMP_OnlineServicesManager::getInstance().refreshSessionTokenSync(tokenVersion)) {
				std::string freshToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
				curl = curl_easy_init();
				if (curl) {
					headers = nullptr;
					headers = curl_slist_append(headers, "Content-Type: application/json");
					if (!freshToken.empty()) {
						std::string authHeader = "Authorization: Bearer " + freshToken;
						headers = curl_slist_append(headers, authHeader.c_str());
					}
					response.text.clear();
					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
					curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

					res = curl_easy_perform(curl);
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
					curl_slist_free_all(headers);
					curl_easy_cleanup(curl);
				}
			}
		}

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

// GeneralsX @feature fbraz3 03/10/2026 Send initial or periodic match progress telemetry to server
void NGMP_OnlineServices_StatsInterface::SendMatchProgress(bool isInitial)
{
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface == nullptr)
	{
		return;
	}

	uint64_t currentMatchID = pLobbyInterface->GetCurrentMatchID();
	if (currentMatchID == 0)
	{
		return;
	}

	// Resolve local player's side/faction
	int resolvedSide = ResolveLocalPlayerSide(pLobbyInterface);

	Int buildingsBuilt = 0;
	Int buildingsDestroyed = 0;
	Int buildingsLost = 0;
	Int unitsBuilt = 0;
	Int unitsDestroyed = 0;
	Int unitsLost = 0;
	Int totalMoney = 0;

	if (ThePlayerList != nullptr)
	{
		Player* localPlayer = ThePlayerList->getLocalPlayer();
		if (localPlayer != nullptr)
		{
			ScoreKeeper* pScoreKeeper = localPlayer->getScoreKeeper();
			if (pScoreKeeper != nullptr)
			{
				buildingsBuilt = pScoreKeeper->getTotalBuildingsBuilt();
				buildingsDestroyed = pScoreKeeper->getTotalBuildingsDestroyed();
				buildingsLost = pScoreKeeper->getTotalBuildingsLost();
				unitsBuilt = pScoreKeeper->getTotalUnitsBuilt();
				unitsDestroyed = pScoreKeeper->getTotalUnitsDestroyed();
				unitsLost = pScoreKeeper->getTotalUnitsLost();
				totalMoney = pScoreKeeper->getTotalMoneyEarned();
			}
		}
	}

	fprintf(stderr, "[NGMP] SendMatchProgress: initial=%d matchID=%" PRIu64 " side=%d bldBuilt=%d bldKill=%d bldLost=%d unitBuilt=%d unitKill=%d unitLost=%d money=%d\n",
		isInitial ? 1 : 0, currentMatchID, resolvedSide, buildingsBuilt, buildingsDestroyed, buildingsLost, unitsBuilt, unitsDestroyed, unitsLost, totalMoney);
	fflush(stderr);

	json payload = {
		{"match_id", currentMatchID}
	};

	if (resolvedSide >= 0)
	{
		payload["side"] = resolvedSide;
	}

	payload["buildings_built"] = buildingsBuilt;
	payload["buildings_killed"] = buildingsDestroyed;
	payload["buildings_lost"] = buildingsLost;
	payload["units_built"] = unitsBuilt;
	payload["units_killed"] = unitsDestroyed;
	payload["units_lost"] = unitsLost;
	payload["total_money"] = totalMoney;

	std::string payloadStr = payload.dump(-1, ' ', false, json::error_handler_t::replace);
	std::string url = NGMP::GetAPIEndpoint("Lobby/MatchProgress");
	std::string authToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
	uint32_t tokenVersion = NGMP_OnlineServicesManager::getInstance().getAuthTokenVersion();

	// GeneralsX @bugfix fbraz3 03/10/2026 Defer progress report if no valid bearer token is available
	if (authToken.empty())
	{
		if (NGMP_OnlineServicesManager::getInstance().refreshSessionTokenSync(tokenVersion))
		{
			authToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
			tokenVersion = NGMP_OnlineServicesManager::getInstance().getAuthTokenVersion();
		}

		if (authToken.empty())
		{
			fprintf(stderr, "[NGMP] SendMatchProgress: no bearer auth token available, deferring progress report\n");
			fflush(stderr);
			return;
		}
	}

	static std::atomic<bool> s_progressInFlight{false};
	if (s_progressInFlight.exchange(true))
	{
		fprintf(stderr, "[NGMP] SendMatchProgress: previous progress request still in flight, skipping\n");
		fflush(stderr);
		return;
	}

	// GeneralsX @bugfix fbraz3 03/10/2026 Carry isInitial into worker and retry failed initial report with bounded backoff
	std::thread([url, payloadStr, authToken, tokenVersion, isInitial]() {
		int attemptsLeft = isInitial ? 3 : 1;
		bool success = false;
		std::string currentToken = authToken;

		while (attemptsLeft > 0 && !success) {
			attemptsLeft--;

			if (currentToken.empty()) {
				fprintf(stderr, "[NGMP] SendMatchProgress: empty bearer token, aborting request\n");
				fflush(stderr);
				break;
			}

			CURL* curl = curl_easy_init();
			if (!curl) {
				fprintf(stderr, "[NGMP] SendMatchProgress: failed to initialize curl\n");
				fflush(stderr);
				break;
			}

			NGMP::Internal::CurlResponse response;
			struct curl_slist* headers = nullptr;
			headers = curl_slist_append(headers, "Content-Type: application/json");
			std::string authHeader = "Authorization: Bearer " + currentToken;
			headers = curl_slist_append(headers, authHeader.c_str());

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

			if (httpCode == 401) {
				fprintf(stderr, "[NGMP] SendMatchProgress: 401 Unauthorized, refreshing token...\n");
				fflush(stderr);
				if (NGMP_OnlineServicesManager::getInstance().refreshSessionTokenSync(tokenVersion)) {
					currentToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
					if (!currentToken.empty()) {
						attemptsLeft++; // allow immediate retry with the fresh token
						continue;
					}
				}
				break;
			}

			if (res == CURLE_OK && httpCode == 200) {
				fprintf(stderr, "[NGMP] SendMatchProgress: server accepted progress update (HTTP 200)\n");
				fflush(stderr);
				success = true;
				break;
			} else {
				fprintf(stderr, "[NGMP] SendMatchProgress: POST failed (res=%d, HTTP %ld, body: %s)\n", (int)res, httpCode, response.text.c_str());
				fflush(stderr);
				if (attemptsLeft > 0) {
					fprintf(stderr, "[NGMP] SendMatchProgress: retrying initial progress in 2 seconds (%d attempts remaining)...\n", attemptsLeft);
					fflush(stderr);
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
			}
		}

		s_progressInFlight = false;
	}).detach();
}
