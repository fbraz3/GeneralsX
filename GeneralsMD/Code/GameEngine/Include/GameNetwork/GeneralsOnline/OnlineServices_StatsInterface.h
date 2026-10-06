// GeneralsX @feature GeneralsOnline StatsInterface header
#pragma once

#include <functional>
#include <cstdint>
#include <thread>
#include <mutex>
#include "GameNetwork/RankPointValue.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

enum class EStatsRequestPolicy
{
	CACHED_ONLY,
	RESPECT_CACHE_ALLOW_REQUEST,
	BYPASS_CACHE_FORCE_REQUEST
};

class ScoreKeeper;

class NGMP_OnlineServices_StatsInterface
{
public:
	NGMP_OnlineServices_StatsInterface();
	~NGMP_OnlineServices_StatsInterface();

	void findPlayerStatsByID(int64_t userID, std::function<void(bool, PSPlayerStats)> callback, EStatsRequestPolicy policy = EStatsRequestPolicy::RESPECT_CACHE_ALLOW_REQUEST);
	void CommitMyOutcome(ScoreKeeper* pScoreKeeper, bool bWon);
	void quiesceOutcome();
	// GeneralsX @feature fbraz3 03/10/2026 Send initial or periodic match progress telemetry to server
	void SendMatchProgress(bool isInitial = false);
	void UpdateMyStats(PSPlayerStats stats) {}

private:
	std::thread m_outcomeThread;
	std::mutex m_outcomeMutex;
};
