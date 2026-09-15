// GeneralsX @feature GeneralsOnline RoomsInterface header
#pragma once

#include "GameNetwork/GeneralsOnline/NGMP_types.h"
#include "Common/GameDefines.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <cstdint>

enum class ERoomFlags : int
{
	ROOM_FLAGS_DEFAULT = 0,
	ROOM_FLAGS_SHOW_ALL_MATCHES = 1
};

class NetworkRoom
{
public:
	NetworkRoom(int roomID, std::string strRoomName, ERoomFlags roomFlags)
	{
		m_RoomID = roomID;
		m_strRoomDisplayName.translate(AsciiString(strRoomName.c_str()));
		m_RoomFlags = roomFlags;
	}

	int GetRoomID() const { return m_RoomID; }
	UnicodeString GetRoomDisplayName() const { return m_strRoomDisplayName; }
	ERoomFlags GetRoomFlags() const { return m_RoomFlags; }

private:
	int m_RoomID;
	UnicodeString m_strRoomDisplayName;
	ERoomFlags m_RoomFlags = ERoomFlags::ROOM_FLAGS_DEFAULT;
};

class NetworkRoomMember : public NetworkMemberBase
{
public:
	bool IsValid() const { return user_id != -1; }
};

class NGMP_OnlineServices_RoomsInterface
{
public:
	NGMP_OnlineServices_RoomsInterface();

	void GetRoomList(std::function<void(void)> cb);
	void JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback);
	void LeaveRoom();

	void RegisterForChatCallback(std::function<void(UnicodeString strMessage, Color color)> cb)
	{
		m_OnChatCallback = cb;
	}
	void DeregisterForChatCallback()
	{
		m_OnChatCallback = nullptr;
	}

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

	NetworkRoomMember* GetRoomMemberFromIndex(int index);
	NetworkRoomMember* GetRoomMemberFromID(int64_t puid);

	std::unordered_map<uint64_t, NetworkRoomMember>& GetMembersListForCurrentRoom() { return m_mapMembers; }
	void SendChatMessageToCurrentRoom(UnicodeString& strChatMsg, bool bIsAction);

	void ResetCachedRoomData()
	{
		m_mapMembers.clear();
		std::scoped_lock<std::mutex> lock(m_rosterCallbackMutex);
		if (m_RosterNeedsRefreshCallback != nullptr)
		{
			m_RosterNeedsRefreshCallback();
		}
	}

	void Tick() {}

	std::vector<NetworkRoom> GetGroupRooms() { return m_vecRooms; }
	void OnRosterUpdated(const std::unordered_map<uint64_t, NetworkRoomMember>& mapMembers);
	int GetCurrentRoomID() const { return m_CurrentRoomID; }

	// Internal callback
	void InvokeChatCallback(const UnicodeString& strMessage, Color color);

private:
	int m_CurrentRoomID = 0;
	std::vector<NetworkRoom> m_vecRooms;
	std::unordered_map<uint64_t, NetworkRoomMember> m_mapMembers;
	std::function<void(UnicodeString, Color)> m_OnChatCallback = nullptr;
	std::function<void()> m_RosterNeedsRefreshCallback = nullptr;
	mutable std::mutex m_rosterCallbackMutex;
};
