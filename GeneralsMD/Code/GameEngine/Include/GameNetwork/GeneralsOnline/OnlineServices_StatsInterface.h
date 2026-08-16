// GeneralsX @feature GeneralsOnline StatsInterface header
#pragma once

#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/RankPointValue.h"
#include <functional>
#include <cstdint>

enum class EStatsRequestPolicy
{
	CACHED_ONLY,
	RESPECT_CACHE_ALLOW_REQUEST,
	BYPASS_CACHE_FORCE_REQUEST
};

class NGMP_OnlineServices_StatsInterface
{
public:
	NGMP_OnlineServices_StatsInterface();

	void findPlayerStatsByID(int64_t userID, std::function<void(bool, PSPlayerStats)> callback, EStatsRequestPolicy policy = EStatsRequestPolicy::RESPECT_CACHE_ALLOW_REQUEST);
};
