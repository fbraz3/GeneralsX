// GeneralsX @feature GeneralsOnline RoomsInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_RoomsInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"

NGMP_OnlineServices_RoomsInterface::NGMP_OnlineServices_RoomsInterface()
{
	m_vecRooms.push_back(NetworkRoom(0, "Global Lobby", ERoomFlags::ROOM_FLAGS_DEFAULT));
}

void NGMP_OnlineServices_RoomsInterface::GetRoomList(std::function<void(void)> cb)
{
	if (cb) cb();
}

void NGMP_OnlineServices_RoomsInterface::JoinRoom(int roomIndex, std::function<void()> onStartCallback, std::function<void()> onCompleteCallback)
{
	if (onStartCallback) onStartCallback();
	m_CurrentRoomID = roomIndex;
	NGMP_OnlineServicesManager::getInstance().changeNetworkRoom((int16_t)roomIndex);
	if (onCompleteCallback) onCompleteCallback();
}

void NGMP_OnlineServices_RoomsInterface::LeaveRoom()
{
	ResetCachedRoomData();
}

NetworkRoomMember* NGMP_OnlineServices_RoomsInterface::GetRoomMemberFromIndex(int index)
{
	if (index >= 0 && index < (int)m_mapMembers.size())
	{
		auto it = m_mapMembers.begin();
		std::advance(it, index);
		return &it->second;
	}
	return nullptr;
}

NetworkRoomMember* NGMP_OnlineServices_RoomsInterface::GetRoomMemberFromID(int64_t puid)
{
	auto it = m_mapMembers.find(puid);
	if (it != m_mapMembers.end())
	{
		return &it->second;
	}
	return nullptr;
}

void NGMP_OnlineServices_RoomsInterface::SendChatMessageToCurrentRoom(UnicodeString& strChatMsg, bool bIsAction)
{
	std::shared_ptr<WebSocket> pWS = NGMP_OnlineServicesManager::GetWebSocket();
	if (pWS != nullptr)
	{
		pWS->SendData_RoomChatMessage(strChatMsg, bIsAction);
	}
}

void NGMP_OnlineServices_RoomsInterface::OnRosterUpdated(const std::unordered_map<uint64_t, NetworkRoomMember>& mapMembers)
{
	m_mapMembers = mapMembers;
	std::scoped_lock<std::mutex> lock(m_rosterCallbackMutex);
	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}
}

void NGMP_OnlineServices_RoomsInterface::InvokeChatCallback(const UnicodeString& strMessage, Color color)
{
	if (m_OnChatCallback)
	{
		m_OnChatCallback(strMessage, color);
	}
}
