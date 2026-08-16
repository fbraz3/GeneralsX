// GeneralsX @feature GeneralsOnline StatsInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_StatsInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "Common/ScoreKeeper.h"
#include <cinttypes>

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
		if (callback) callback(cached, stats);
		return;
	}

	// Trigger async request
	NGMP_OnlineServicesManager::getInstance().requestPlayerStatsAsync(userID);
	if (callback) callback(cached, stats);
}

void NGMP_OnlineServices_StatsInterface::CommitMyOutcome(ScoreKeeper* pScoreKeeper, bool bWon)
{
	NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
	if (pLobbyInterface != nullptr)
	{
		uint64_t currentMatchID = pLobbyInterface->GetCurrentMatchID();
		if (currentMatchID == 0)
			return;

		Int buildingsBuilt = pScoreKeeper ? pScoreKeeper->getTotalBuildingsBuilt() : 0;
		Int buildingsDestroyed = pScoreKeeper ? pScoreKeeper->getTotalBuildingsDestroyed() : 0;
		Int buildingsLost = pScoreKeeper ? pScoreKeeper->getTotalBuildingsLost() : 0;
		Int unitsBuilt = pScoreKeeper ? pScoreKeeper->getTotalUnitsBuilt() : 0;
		Int unitsDestroyed = pScoreKeeper ? pScoreKeeper->getTotalUnitsDestroyed() : 0;
		Int unitsLost = pScoreKeeper ? pScoreKeeper->getTotalUnitsLost() : 0;
		Int totalMoney = pScoreKeeper ? pScoreKeeper->getTotalMoneyEarned() : 0;

		fprintf(stderr, "[NGMP] CommitMyOutcome: won=%d matchID=%" PRIu64 " bldBuilt=%d bldKill=%d bldLost=%d unitBuilt=%d unitKill=%d unitLost=%d money=%d\n",
			bWon ? 1 : 0, currentMatchID, buildingsBuilt, buildingsDestroyed, buildingsLost, unitsBuilt, unitsDestroyed, unitsLost, totalMoney);
		fflush(stderr);
	}
}
