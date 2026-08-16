// GeneralsX @feature GeneralsOnline LobbyInterface header
#pragma once

#include "GameNetwork/GeneralsOnline/NGMP_types.h"
#include "GameNetwork/GameInfo.h"
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <cstdint>

class NGMPGame;

struct LobbyMemberEntry : public NetworkMemberBase
{
	int side = -1;
	int color = -1;
	int team = -1;
	int startpos = -1;
	bool has_map = false;

	uint16_t m_SlotIndex = 9999;
	uint16_t m_SlotState = SlotState::SLOT_OPEN;

	std::string region;
	int latency = 0;

	bool IsHuman() const
	{
		return user_id != -1 && m_SlotState == SlotState::SLOT_PLAYER;
	}
};

enum class ELobbyType
{
	UNKNOWN = -1,
	CustomGame = 0,
	QuickMatch = 1
};

struct LobbyEntry
{
	int64_t lobbyID = -1;
	int64_t owner = -1;
	std::string name;
	std::string map_name;
	std::string map_path;
	bool map_official = false;
	int current_players = 0;
	int max_players = 0;
	bool vanilla_teams = false;
	uint32_t starting_cash = 0;
	bool limit_superweapons = false;
	bool track_stats = false;
	bool allow_observers = false;

	uint32_t exe_crc = 0;
	uint32_t ini_crc = 0;
	uint64_t match_id = 0;
	ELobbyType lobby_type = ELobbyType::CustomGame;
	int rng_seed = -1;

	bool passworded = false;
	std::string password;

	std::vector<LobbyMemberEntry> members;
	std::string region;
	int latency = 0;
};

enum class EJoinLobbyResult
{
	JoinLobbyResult_Success,
	JoinLobbyResult_FullRoom,
	JoinLobbyResult_BadPassword,
	JoinLobbyResult_JoinFailed,
	JoinLobbyResult_AnticheatMismatch
};

enum class ELobbyJoinability
{
	LobbyJoinability_Public,
	LobbyJoinability_FriendsOnly,
};

class NGMP_OnlineServices_LobbyInterface
{
public:
	NGMP_OnlineServices_LobbyInterface();

	void StopMatchStartCountdownIfRunning();
	void SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<LobbyEntry>)> onCompleteCallback);
	void DeregisterForSearchForLobbiesCallback() { m_fnCallbackSearchForLobbiesComplete = nullptr; }

	// Updates
	void UpdateCurrentLobby_Map(AsciiString strMap, AsciiString strMapPath, bool bIsOfficial, int newMaxPlayers);
	void UpdateCurrentLobby_LimitSuperweapons(bool bLimitSuperweapons);
	void UpdateCurrentLobby_StartingCash(UnsignedInt startingCashValue);
	void UpdateCurrentLobby_HasMap();

	void UpdateCurrentLobby_MySide(int side, int updatedStartPos);
	void UpdateCurrentLobby_MyColor(int color);
	void UpdateCurrentLobby_MyStartPos(int startPos);
	void UpdateCurrentLobby_MyTeam(int team);

	// AI
	void UpdateCurrentLobby_AIColor(int slot, int color);
	void UpdateCurrentLobby_AISide(int slot, int side, int updatedStartPos);
	void UpdateCurrentLobby_AITeam(int slot, int team);
	void UpdateCurrentLobby_AIStartPos(int slot, int startpos);

	void UpdateCurrentLobby_KickUser(int64_t userID, UnicodeString name);
	void UpdateCurrentLobby_SetSlotState(uint16_t slotIndex, uint16_t slotState);
	void UpdateCurrentLobby_ForceReady();

	void SetLobbyListDirty() { m_bLobbyListDirty = true; }
	void ConsumeLobbyListDirtyFlag() { m_bLobbyListDirty = false; }
	bool IsLobbyListDirty() const { return m_bLobbyListDirty; }

	void CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMapName, AsciiString strInitialMapPath, bool bIsOfficial, int initialMaxSize, bool bVanillaTeamsOnly, bool bTrackStats, uint32_t startingCash, bool bPassworded, std::string strPassword, bool bAllowObservers);
	void JoinLobby(LobbyEntry lobby, std::string strPassword);
	void LeaveCurrentLobby();

	UnicodeString GetCurrentLobbyDisplayName();
	UnicodeString GetCurrentLobbyMapDisplayName();
	AsciiString GetCurrentLobbyMapPath();

	void SendChatMessageToCurrentLobby(UnicodeString& strChatMsgUnicode, bool bIsAction);

	LobbyEntry& GetCurrentLobby() { return m_CurrentLobby; }
	const LobbyEntry& GetCurrentLobby() const { return m_CurrentLobby; }
	NGMPGame* GetCurrentGame();

	void RegisterForRosterNeedsRefreshCallback(std::function<void()> cb)
	{
		std::scoped_lock<std::mutex> lock(m_rosterCallbackMutex);
		m_RosterNeedsRefreshCallback = cb;
	}
	void DeregisterForRosterNeedsRefreshCallback()
	{
		std::scoped_lock<std::mutex> lock(m_rosterCallbackMutex);
		m_RosterNeedsRefreshCallback = nullptr;
	}

	void RegisterForCreateLobbyCallback(std::function<void(bool)> callback) { m_cb_CreateLobbyPendingCallback = callback; }
	void DeregisterForCreateLobbyCallback() { m_cb_CreateLobbyPendingCallback = nullptr; }

	void RegisterForJoinLobbyCallback(std::function<void(EJoinLobbyResult)> cb) { m_callbackJoinedLobby = cb; }
	void DeregisterForJoinLobbyCallback() { m_callbackJoinedLobby = nullptr; }

	void RegisterForChatCallback(std::function<void(UnicodeString strMessage, Color color)> cb) { m_OnChatCallback = cb; }
	void DeregisterForChatCallback() { m_OnChatCallback = nullptr; }

	bool IsHost();
	bool IsInLobby() const { return m_CurrentLobby.lobbyID != -1; }

	int64_t GetCurrentLobbyOwnerID() const { return m_CurrentLobby.owner; }

	LobbyMemberEntry GetRoomMemberFromIndex(int index);
	LobbyMemberEntry GetRoomMemberFromID(int64_t userid);
	std::vector<LobbyMemberEntry>& GetMembersListForCurrentRoom() { return m_CurrentLobby.members; }

	void UpdateRoomDataCache(std::function<void(bool)> fnCallback = nullptr);
	void ResetCachedRoomData();

	void ClearAutoReadyCountdown() { m_timeStartAutoReadyCountdown = -1; }
	bool HasAutoReadyCountdown() const { return m_timeStartAutoReadyCountdown != -1; }

	void SetLobbyTryingToJoin(LobbyEntry lobby) { m_LobbyTryingToJoin = lobby; }
	void ResetLobbyTryingToJoin() { m_LobbyTryingToJoin = LobbyEntry(); }
	LobbyEntry GetLobbyTryingToJoin() const { return m_LobbyTryingToJoin; }

	void Tick();

	// Internal callback dispatchers
	void InvokeCreateLobbyCallback(bool bSuccess);
	void InvokeJoinLobbyCallback(EJoinLobbyResult result);
	void InvokeChatCallback(const UnicodeString& strMessage, Color color);

	std::vector<LobbyEntry> m_vecLobbies;

private:
	std::function<void(bool)> m_cb_CreateLobbyPendingCallback = nullptr;
	std::function<void(EJoinLobbyResult)> m_callbackJoinedLobby = nullptr;
	std::function<void(std::vector<LobbyEntry>)> m_fnCallbackSearchForLobbiesComplete = nullptr;
	std::function<void(UnicodeString, Color)> m_OnChatCallback = nullptr;
	std::function<void()> m_RosterNeedsRefreshCallback = nullptr;
	mutable std::mutex m_rosterCallbackMutex;

	LobbyEntry m_CurrentLobby;
	LobbyEntry m_LobbyTryingToJoin;
	bool m_bLobbyListDirty = false;
	int64_t m_timeStartAutoReadyCountdown = -1;
};
