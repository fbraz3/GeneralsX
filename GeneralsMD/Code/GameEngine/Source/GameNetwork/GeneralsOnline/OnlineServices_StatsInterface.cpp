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
#include <memory>
#include <mutex>
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

	std::thread([url, payloadStr, authToken, tokenVersion, currentMatchID]() {
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

			// GeneralsX @feature fbraz3 04/10/2026 Parse replay_url from match outcome response and notify OnlineServicesManager
			try {
				json outcomeJson = json::parse(response.text);
				if (outcomeJson.contains("replay_url") && outcomeJson["replay_url"].is_string()) {
					std::string replayUrl = outcomeJson["replay_url"].get<std::string>();
					if (!replayUrl.empty()) {
						fprintf(stderr, "[NGMP] CommitMyOutcome: received replay upload URL\n");
						fflush(stderr);
						NGMP_OnlineServicesManager::getInstance().setReplayUploadUrl(currentMatchID, replayUrl);
					}
				}
			} catch (const std::exception& e) {
				fprintf(stderr, "[NGMP] CommitMyOutcome: JSON parse exception: %s\n", e.what());
				fflush(stderr);
			}
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
	int64_t originatingUserId = NGMP_OnlineServicesManager::getInstance().getUserId();
	if (originatingUserId <= 0)
	{
		return;
	}

	struct QueuedProgressReport {
		std::string url;
		std::string payloadStr;
		std::string authToken;
		uint32_t tokenVersion;
		uint32_t sessionGen;
		int64_t userId;
	};

	static std::mutex s_progressMutex;
	static bool s_progressInFlight = false;
	static std::unique_ptr<QueuedProgressReport> s_pendingInitialReport;

	uint32_t originatingSessionGen = NGMP_OnlineServicesManager::getInstance().getSessionGeneration();

	{
		std::lock_guard<std::mutex> lock(s_progressMutex);
		if (s_progressInFlight)
		{
			if (isInitial)
			{
				fprintf(stderr, "[NGMP] SendMatchProgress: request in flight, queueing initial report for execution after active worker\n");
				fflush(stderr);
				s_pendingInitialReport = std::make_unique<QueuedProgressReport>(
					QueuedProgressReport{url, payloadStr, authToken, tokenVersion, originatingSessionGen, originatingUserId}
				);
			}
			else
			{
				fprintf(stderr, "[NGMP] SendMatchProgress: previous progress request still in flight, skipping periodic report\n");
				fflush(stderr);
			}
			return;
		}
		s_progressInFlight = true;
	}

	// GeneralsX @bugfix fbraz3 03/10/2026 Bind progress report and retries to originating session generation and retain queued initial reports
	std::thread([url, payloadStr, authToken, tokenVersion, originatingSessionGen, originatingUserId, isInitial]() mutable {
		std::string curUrl = std::move(url);
		std::string curPayloadStr = std::move(payloadStr);
		std::string curToken = std::move(authToken);
		uint32_t curTokenVersion = tokenVersion;
		uint32_t curSessionGen = originatingSessionGen;
		int64_t curUserId = originatingUserId;
		bool curIsInitial = isInitial;

		while (true) {
			int attemptsLeft = curIsInitial ? 3 : 1;
			bool success = false;
			bool refreshedOnce = false;

			if (curToken.empty()) {
				fprintf(stderr, "[NGMP] SendMatchProgress: auth token is empty, refreshing on worker thread...\n");
				fflush(stderr);
				if (NGMP_OnlineServicesManager::getInstance().getUserId() == curUserId &&
				    NGMP_OnlineServicesManager::getInstance().getSessionGeneration() == curSessionGen &&
				    NGMP_OnlineServicesManager::getInstance().refreshSessionTokenSync(curTokenVersion)) {
					if (NGMP_OnlineServicesManager::getInstance().getUserId() != curUserId ||
					    NGMP_OnlineServicesManager::getInstance().getSessionGeneration() != curSessionGen) {
						fprintf(stderr, "[NGMP] SendMatchProgress: account session changed during initial token refresh, aborting\n");
						fflush(stderr);
						attemptsLeft = 0;
					} else {
						curToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
						curTokenVersion = NGMP_OnlineServicesManager::getInstance().getAuthTokenVersion();
						refreshedOnce = true;
					}
				}
			}

			while (attemptsLeft > 0 && !success) {
				attemptsLeft--;

				if (NGMP_OnlineServicesManager::getInstance().getUserId() != curUserId ||
				    NGMP_OnlineServicesManager::getInstance().getSessionGeneration() != curSessionGen) {
					fprintf(stderr, "[NGMP] SendMatchProgress: account session changed or logged out, aborting progress report\n");
					fflush(stderr);
					break;
				}

				{
					std::lock_guard<std::mutex> lock(s_progressMutex);
					if (s_pendingInitialReport) {
						fprintf(stderr, "[NGMP] SendMatchProgress: newer initial report queued, abandoning retries of previous report\n");
						fflush(stderr);
						break;
					}
				}

				if (curToken.empty()) {
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
				std::string authHeader = "Authorization: Bearer " + curToken;
				headers = curl_slist_append(headers, authHeader.c_str());

				curl_easy_setopt(curl, CURLOPT_URL, curUrl.c_str());
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curPayloadStr.c_str());
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
					if (refreshedOnce) {
						fprintf(stderr, "[NGMP] SendMatchProgress: 401 Unauthorized after token refresh, aborting\n");
						fflush(stderr);
						break;
					}
					refreshedOnce = true;
					fprintf(stderr, "[NGMP] SendMatchProgress: 401 Unauthorized, refreshing token...\n");
					fflush(stderr);
					if (NGMP_OnlineServicesManager::getInstance().getUserId() == curUserId &&
					    NGMP_OnlineServicesManager::getInstance().getSessionGeneration() == curSessionGen &&
					    NGMP_OnlineServicesManager::getInstance().refreshSessionTokenSync(curTokenVersion)) {
						if (NGMP_OnlineServicesManager::getInstance().getUserId() != curUserId ||
						    NGMP_OnlineServicesManager::getInstance().getSessionGeneration() != curSessionGen) {
							fprintf(stderr, "[NGMP] SendMatchProgress: account session changed during 401 token refresh, aborting\n");
							fflush(stderr);
							break;
						}
						curToken = NGMP_OnlineServicesManager::getInstance().getAuthToken();
						curTokenVersion = NGMP_OnlineServicesManager::getInstance().getAuthTokenVersion();
						if (!curToken.empty()) {
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
						{
							std::lock_guard<std::mutex> lock(s_progressMutex);
							if (s_pendingInitialReport) {
								fprintf(stderr, "[NGMP] SendMatchProgress: newer initial report queued, abandoning retries of previous report\n");
								fflush(stderr);
								break;
							}
						}
						fprintf(stderr, "[NGMP] SendMatchProgress: retrying initial progress in 2 seconds (%d attempts remaining)...\n", attemptsLeft);
						fflush(stderr);
						std::this_thread::sleep_for(std::chrono::seconds(2));
					}
				}
			}

			{
				std::lock_guard<std::mutex> lock(s_progressMutex);
				if (s_pendingInitialReport) {
					curUrl = std::move(s_pendingInitialReport->url);
					curPayloadStr = std::move(s_pendingInitialReport->payloadStr);
					curToken = std::move(s_pendingInitialReport->authToken);
					curTokenVersion = s_pendingInitialReport->tokenVersion;
					curSessionGen = s_pendingInitialReport->sessionGen;
					curUserId = s_pendingInitialReport->userId;
					curIsInitial = true;
					s_pendingInitialReport.reset();
					continue;
				}
				s_progressInFlight = false;
				break;
			}
		}
	}).detach();
}
