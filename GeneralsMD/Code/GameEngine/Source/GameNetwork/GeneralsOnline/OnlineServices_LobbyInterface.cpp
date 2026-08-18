// GeneralsX @feature GeneralsOnline LobbyInterface implementation
#include "GameNetwork/GeneralsOnline/OnlineServices_LobbyInterface.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "Common/GlobalData.h"

NGMP_OnlineServices_LobbyInterface::NGMP_OnlineServices_LobbyInterface()
{
}

void NGMP_OnlineServices_LobbyInterface::StopMatchStartCountdownIfRunning()
{
	if (TheNGMPGame && TheNGMPGame->IsCountdownStarted())
	{
		TheNGMPGame->StopCountdown();
	}
}

void NGMP_OnlineServices_LobbyInterface::SearchForLobbies(std::function<void()> onStartCallback, std::function<void(std::vector<LobbyEntry>)> onCompleteCallback)
{
	if (onStartCallback) onStartCallback();
	m_fnCallbackSearchForLobbiesComplete = onCompleteCallback;
	NGMP_OnlineServicesManager::getInstance().requestLobbyListAsync();
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_Map(AsciiString strMap, AsciiString strMapPath, bool bIsOfficial, int newMaxPlayers)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyMap(strMap.str(), strMapPath.str(), bIsOfficial, newMaxPlayers);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_LimitSuperweapons(bool bLimitSuperweapons)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyLimitSuperweapons(bLimitSuperweapons);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_StartingCash(UnsignedInt startingCashValue)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyStartingCash((int)startingCashValue);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_HasMap()
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyHasMap(true);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_MySide(int side, int updatedStartPos)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyMySide(side, updatedStartPos);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_MyColor(int color)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyMyColor(color);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_MyStartPos(int startPos)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyMyStartPos(startPos);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_MyTeam(int team)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyMyTeam(team);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_AIColor(int slot, int color)
{
	// Slot update for AI
	if (TheNGMPGame)
	{
		NGMPGameSlot* pSlot = TheNGMPGame->getGameSpySlot(slot);
		if (pSlot) pSlot->setColor(color);
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_AISide(int slot, int side, int updatedStartPos)
{
	if (TheNGMPGame)
	{
		NGMPGameSlot* pSlot = TheNGMPGame->getGameSpySlot(slot);
		if (pSlot)
		{
			pSlot->setPlayerTemplate(side);
			if (updatedStartPos >= 0) pSlot->setStartPos(updatedStartPos);
		}
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_AITeam(int slot, int team)
{
	if (TheNGMPGame)
	{
		NGMPGameSlot* pSlot = TheNGMPGame->getGameSpySlot(slot);
		if (pSlot) pSlot->setTeamNumber(team);
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_AIStartPos(int slot, int startpos)
{
	if (TheNGMPGame)
	{
		NGMPGameSlot* pSlot = TheNGMPGame->getGameSpySlot(slot);
		if (pSlot) pSlot->setStartPos(startpos);
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_KickUser(int64_t userID, UnicodeString name)
{
	// Set slot state to open
	if (TheNGMPGame)
	{
		for (int i = 0; i < MAX_SLOTS; ++i)
		{
			NGMPGameSlot* slot = TheNGMPGame->getGameSpySlot(i);
			if (slot && slot->m_userID == userID)
			{
				UpdateCurrentLobby_SetSlotState(i, SlotState::SLOT_OPEN);
				break;
			}
		}
	}
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_SetSlotState(uint16_t slotIndex, uint16_t slotState)
{
	NGMP_OnlineServicesManager::getInstance().updateLobbySlotState(slotIndex, slotState);
}

void NGMP_OnlineServices_LobbyInterface::UpdateCurrentLobby_ForceReady()
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyForceStart();
}

void NGMP_OnlineServices_LobbyInterface::CreateLobby(UnicodeString strLobbyName, UnicodeString strInitialMapName, AsciiString strInitialMapPath, bool bIsOfficial, int initialMaxSize, bool bVanillaTeamsOnly, bool bTrackStats, uint32_t startingCash, bool bPassworded, std::string strPassword, bool bAllowObservers)
{
	AsciiString aName;
	aName.translate(strLobbyName);
	NGMP_OnlineServicesManager::getInstance().createLobbyAsync(aName.str(), strInitialMapPath.str(), strPassword, initialMaxSize);
}

void NGMP_OnlineServices_LobbyInterface::JoinLobby(LobbyEntry lobby, std::string strPassword)
{
	SetLobbyTryingToJoin(lobby);
	NGMP_OnlineServicesManager::getInstance().joinLobbyAsync(lobby.lobbyID, strPassword);
}

void NGMP_OnlineServices_LobbyInterface::LeaveCurrentLobby()
{
	NGMP_OnlineServicesManager::getInstance().updateLobbyLeave();
	ResetCachedRoomData();
}

UnicodeString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyDisplayName()
{
	UnicodeString name;
	name.translate(AsciiString(m_CurrentLobby.name.c_str()));
	return name;
}

UnicodeString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyMapDisplayName()
{
	UnicodeString name;
	name.translate(AsciiString(m_CurrentLobby.map_name.c_str()));
	return name;
}

AsciiString NGMP_OnlineServices_LobbyInterface::GetCurrentLobbyMapPath()
{
	return AsciiString(m_CurrentLobby.map_path.c_str());
}

void NGMP_OnlineServices_LobbyInterface::SendChatMessageToCurrentLobby(UnicodeString& strChatMsgUnicode, bool bIsAction)
{
	std::shared_ptr<WebSocket> pWS = NGMP_OnlineServicesManager::GetWebSocket();
	if (pWS != nullptr)
	{
		pWS->SendData_LobbyChatMessage(strChatMsgUnicode, bIsAction, false, false);
	}
}

NGMPGame* NGMP_OnlineServices_LobbyInterface::GetCurrentGame()
{
	return TheNGMPGame;
}

bool NGMP_OnlineServices_LobbyInterface::IsHost()
{
	return NGMP_OnlineServicesManager::getInstance().isLobbyOwner();
}

LobbyMemberEntry NGMP_OnlineServices_LobbyInterface::GetRoomMemberFromIndex(int index)
{
	if (index >= 0 && index < (int)m_CurrentLobby.members.size())
	{
		return m_CurrentLobby.members[index];
	}
	return LobbyMemberEntry();
}

LobbyMemberEntry NGMP_OnlineServices_LobbyInterface::GetRoomMemberFromID(int64_t userid)
{
	for (const auto& member : m_CurrentLobby.members)
	{
		if (member.user_id == userid)
			return member;
	}
	return LobbyMemberEntry();
}

void NGMP_OnlineServices_LobbyInterface::UpdateRoomDataCache(std::function<void(bool)> fnCallback)
{
	NGMP_OnlineServicesManager::getInstance().requestLobbyDetailsAsync(m_CurrentLobby.lobbyID);
	if (fnCallback) fnCallback(true);
}

void NGMP_OnlineServices_LobbyInterface::ResetCachedRoomData()
{
	m_CurrentLobby = LobbyEntry();
	std::scoped_lock<std::mutex> lock(m_rosterCallbackMutex);
	if (m_RosterNeedsRefreshCallback != nullptr)
	{
		m_RosterNeedsRefreshCallback();
	}
}

void NGMP_OnlineServices_LobbyInterface::Tick()
{
}

void NGMP_OnlineServices_LobbyInterface::InvokeCreateLobbyCallback(bool bSuccess)
{
	if (m_cb_CreateLobbyPendingCallback)
		m_cb_CreateLobbyPendingCallback(bSuccess);
}

void NGMP_OnlineServices_LobbyInterface::InvokeJoinLobbyCallback(EJoinLobbyResult result)
{
	if (m_callbackJoinedLobby)
		m_callbackJoinedLobby(result);
}

void NGMP_OnlineServices_LobbyInterface::InvokeChatCallback(const UnicodeString& strMessage, Color color)
{
	if (m_OnChatCallback)
		m_OnChatCallback(strMessage, color);
}
