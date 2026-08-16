// GeneralsX @feature GeneralsOnline StatsInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_StatsInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"

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
