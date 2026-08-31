#include "GameNetwork/GeneralsOnline/NetworkMesh.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_LobbyInterface.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_Auth.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/NGMP_json.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/ConnectionManager.h"

#if defined(SAGE_USE_GAMENETWORKINGSOCKETS)
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingcustomsignaling.h>
#endif

#include <atomic>
#include <deque>
#include <cstring>
#include <cstdio>

#if defined(SAGE_USE_GAMENETWORKINGSOCKETS)

static std::atomic<bool> g_bNetworkMeshDestroying = false;
static std::mutex g_pendingDeletionMutex;
static std::vector<void*> g_pendingConnSignalingDeletions;

static void CleanupPendingConnSignalingDeletions()
{
	std::vector<void*> objectsToDelete;
	{
		std::lock_guard<std::mutex> lock(g_pendingDeletionMutex);
		objectsToDelete.swap(g_pendingConnSignalingDeletions);
	}
	for (void* pObj : objectsToDelete)
	{
		delete static_cast<ISteamNetworkingConnectionSignaling*>(pObj);
	}
}

void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
	CleanupPendingConnSignalingDeletions();

	if (g_bNetworkMeshDestroying.load() || !pInfo)
	{
		return;
	}

	NetworkMesh* pMesh = NGMP_OnlineServicesManager::GetNetworkMesh();
	if (pMesh == nullptr)
	{
		return;
	}

	int64_t connectionID = -1;
	std::map<int64_t, PlayerConnection>& connections = pMesh->GetAllConnections();
	for (auto& kvPair : connections)
	{
		if (kvPair.second.m_hSteamConnection == pInfo->m_hConn)
		{
			connectionID = kvPair.first;
			break;
		}
	}

	switch (pInfo->m_info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		if (SteamNetworkingSockets())
		{
			SteamNetworkingSockets()->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		}
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.SetDisconnected(true, pMesh, false);
		}
		break;

	case k_ESteamNetworkingConnectionState_Connecting:
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.UpdateState(EConnectionState::CONNECTING_DIRECT, pMesh);
		}
		break;

	case k_ESteamNetworkingConnectionState_FindingRoute:
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.UpdateState(EConnectionState::FINDING_ROUTE, pMesh);
		}
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.UpdateState(EConnectionState::CONNECTED_DIRECT, pMesh);
		}
		break;

	default:
		break;
	}
}

class CSignalingClient : public ISignalingClient
{
	struct ConnectionSignaling : ISteamNetworkingConnectionSignaling
	{
		CSignalingClient* const m_pOwner;
		int64_t const m_targetUserID;

		ConnectionSignaling(CSignalingClient* owner, int64_t target_user_id)
			: m_pOwner(owner)
			, m_targetUserID(target_user_id)
		{
		}

		bool SendSignal(HSteamNetConnection, const SteamNetConnectionInfo_t&, const void* pMsg, int cbMsg) override
		{
			std::vector<uint8_t> vecPayload(cbMsg);
			std::memcpy(vecPayload.data(), pMsg, cbMsg);
			m_pOwner->Send(m_targetUserID, vecPayload);
			return true;
		}

		void Release() override
		{
			std::lock_guard<std::mutex> lock(g_pendingDeletionMutex);
			g_pendingConnSignalingDeletions.push_back(static_cast<void*>(this));
		}
	};

	struct QueuedSend
	{
		int64_t target_user_id;
		std::vector<uint8_t> vecPayload;
	};

	ISteamNetworkingSockets* const m_pSteamNetworkingSockets;
	std::deque<QueuedSend> m_queueSend;

public:
	CSignalingClient(ISteamNetworkingSockets* pSteamNetworkingSockets)
		: m_pSteamNetworkingSockets(pSteamNetworkingSockets)
	{
	}

	void Send(int64_t target_user_id, std::vector<uint8_t>& vecPayload)
	{
		while (m_queueSend.size() > 128)
		{
			m_queueSend.pop_front();
		}
		QueuedSend newEntry;
		newEntry.target_user_id = target_user_id;
		newEntry.vecPayload = vecPayload;
		m_queueSend.push_back(newEntry);
	}

	ISteamNetworkingConnectionSignaling* CreateSignalingForConnection(
		const SteamNetworkingIdentity& identityPeer,
		SteamNetworkingErrMsg&
	) override
	{
		int64_t user_id = std::stoll(identityPeer.GetGenericString());
		return new ConnectionSignaling(this, user_id);
	}

	void Poll() override
	{
		NGMP_OnlineServicesManager* pMgr = NGMP_OnlineServicesManager::GetInstance();
		if (!pMgr)
			return;

		while (!m_queueSend.empty())
		{
			QueuedSend sendData = m_queueSend.front();
			m_queueSend.pop_front();

			nlohmann::json j;
			j["msg_id"] = 16;
			j["target_user_id"] = sendData.target_user_id;
			j["signal"] = nlohmann::json::binary(sendData.vecPayload);
			pMgr->sendRawWebSocketPayload(j.dump());
		}
	}

	void Release() override
	{
		m_queueSend.clear();
	}
};

NetworkMesh::NetworkMesh()
{
	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(nullptr, errMsg))
	{
		return;
	}

	m_pSignaling = new CSignalingClient(SteamNetworkingSockets());

	SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(OnSteamNetConnectionStatusChanged);
	g_bNetworkMeshDestroying.store(false);

	int localPort = 0;
	SteamNetworkingConfigValue_t opt;
	opt.SetInt32(k_ESteamNetworkingConfig_SymmetricConnect, 1);
	m_hListenSock = SteamNetworkingSockets()->CreateListenSocketP2P(localPort, 1, &opt);
}

void NetworkMesh::Flush()
{
	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);
	for (auto& connectionData : m_mapConnections)
	{
		if (connectionData.second.m_hSteamConnection != k_HSteamNetConnection_Invalid && SteamNetworkingSockets())
		{
			SteamNetworkingSockets()->FlushMessagesOnConnection(connectionData.second.m_hSteamConnection);
		}
	}
}

void NetworkMesh::RegisterConnectivity(int64_t)
{
}

void NetworkMesh::UpdateConnectivity(PlayerConnection*)
{
}

int NetworkMesh::SendGamePacket(void* pBuffer, uint32_t totalDataSize, int64_t userID)
{
	if (!pBuffer || totalDataSize == 0)
	{
		return -3;
	}

	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);
	auto it = m_mapConnections.find(userID);
	if (it != m_mapConnections.end())
	{
		return it->second.SendGamePacket(pBuffer, totalDataSize);
	}
	return -2;
}

void NetworkMesh::SendACPacket(uint32_t userID, const void* pData, uint32_t dataLen)
{
	if (dataLen == 0 || pData == nullptr)
	{
		return;
	}
	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);
	auto it = m_mapConnections.find(userID);
	if (it != m_mapConnections.end())
	{
		it->second.SendACPacket(pData, dataLen);
	}
}

void NetworkMesh::StartConnectionSignalling(const char* szMiddlewareID, int64_t remoteUserID, uint16_t preferredPort)
{
	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);

	auto it = m_mapConnections.find(remoteUserID);
	if (it != m_mapConnections.end())
	{
		if (it->second.m_hSteamConnection != k_HSteamNetConnection_Invalid && SteamNetworkingSockets())
		{
			SteamNetworkingSockets()->CloseConnection(it->second.m_hSteamConnection, 0, "New connection negotiating", false);
		}
		m_mapConnections.erase(it);
	}

	NGMP_OnlineServicesManager* pOnlineServicesMgr = NGMP_OnlineServicesManager::GetInstance();
	if (!pOnlineServicesMgr)
	{
		return;
	}

	if (remoteUserID == pOnlineServicesMgr->getUserId())
	{
		return;
	}

	SteamNetworkingIdentity identityRemote;
	identityRemote.Clear();
	std::string remoteUserIDStr = std::to_string(remoteUserID);
	identityRemote.SetGenericString(remoteUserIDStr.c_str());

	if (identityRemote.IsInvalid() || !m_pSignaling)
	{
		return;
	}

	std::vector<SteamNetworkingConfigValue_t> vecOpts;
	SteamNetworkingConfigValue_t opt;
	opt.SetInt32(k_ESteamNetworkingConfig_SymmetricConnect, 1);
	vecOpts.push_back(opt);

	SteamNetworkingErrMsg errMsg;
	ISteamNetworkingConnectionSignaling* pConnSignaling = m_pSignaling->CreateSignalingForConnection(identityRemote, errMsg);
	if (!pConnSignaling)
	{
		return;
	}

	HSteamNetConnection hConn = SteamNetworkingSockets()->ConnectP2PCustomSignaling(
		pConnSignaling,
		&identityRemote,
		0,
		static_cast<int>(vecOpts.size()),
		vecOpts.data()
	);

	if (hConn == k_HSteamNetConnection_Invalid)
	{
		return;
	}

	PlayerConnection newConnection(remoteUserID, hConn);
	newConnection.m_ConnectionType = EConnectionType::BuiltIn_ValveSockets;
	m_mapConnections[remoteUserID] = newConnection;
}

void NetworkMesh::DisconnectUser(int64_t remoteUserID)
{
	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);
	auto it = m_mapConnections.find(remoteUserID);
	if (it != m_mapConnections.end())
	{
		it->second.Close();
		m_mapConnections.erase(it);
	}
}

void NetworkMesh::Disconnect()
{
	std::lock_guard<std::recursive_mutex> lock(m_mapConnectionsMutex);
	for (auto& kvPair : m_mapConnections)
	{
		kvPair.second.Close();
	}
	m_mapConnections.clear();

	if (m_hListenSock != k_HSteamListenSocket_Invalid && SteamNetworkingSockets())
	{
		SteamNetworkingSockets()->CloseListenSocket(m_hListenSock);
		m_hListenSock = k_HSteamListenSocket_Invalid;
	}
}

void NetworkMesh::Tick()
{
	if (SteamNetworkingSockets())
	{
		SteamNetworkingSockets()->RunCallbacks();
	}
	if (m_pSignaling)
	{
		m_pSignaling->Poll();
	}
}

// --------------------------------------------------------------------------------------
// PlayerConnection Implementation
// --------------------------------------------------------------------------------------

PlayerConnection::PlayerConnection(int64_t userID, HSteamNetConnection hSteamConnection)
	: m_userID(userID)
	, m_hSteamConnection(hSteamConnection)
	, m_ConnectionType(EConnectionType::BuiltIn_ValveSockets)
{
}

PlayerConnection::PlayerConnection(int64_t userID, const char* szMiddlewareID)
	: m_userID(userID)
	, m_strMiddlewareID(szMiddlewareID ? szMiddlewareID : "")
	, m_ConnectionType(EConnectionType::MiddlewarePluginGeneric)
{
}

int PlayerConnection::SendGamePacket(void* pBuffer, uint32_t totalDataSize)
{
	if (totalDataSize == 0 || pBuffer == nullptr || m_hSteamConnection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets())
	{
		return -1;
	}

	std::vector<uint8_t> vecData(totalDataSize + 1);
	vecData[0] = static_cast<uint8_t>(ENetworkChannels::Game);
	std::memcpy(vecData.data() + 1, pBuffer, totalDataSize);

	int sendFlags = k_nSteamNetworkingSend_Reliable | k_nSteamNetworkingSend_AutoRestartBrokenSession;
	EResult r = SteamNetworkingSockets()->SendMessageToConnection(
		m_hSteamConnection, vecData.data(), static_cast<uint32_t>(vecData.size()), sendFlags, nullptr);

	return (r == k_EResultOK) ? 0 : -1;
}

void PlayerConnection::SendACPacket(const void* pData, uint32_t dataLen)
{
	if (dataLen == 0 || pData == nullptr || m_hSteamConnection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets())
	{
		return;
	}
	std::vector<uint8_t> vecData(dataLen + 1);
	vecData[0] = static_cast<uint8_t>(ENetworkChannels::Anticheat);
	std::memcpy(vecData.data() + 1, pData, dataLen);

	SteamNetworkingSockets()->SendMessageToConnection(
		m_hSteamConnection, vecData.data(), static_cast<uint32_t>(vecData.size()), k_nSteamNetworkingSend_Reliable, nullptr);
}

void PlayerConnection::UpdateLatencyHistogram()
{
	int currLatency = GetLatency();
	if (currLatency >= 0)
	{
		if (m_vecLatencyHistory.size() >= 60)
		{
			m_vecLatencyHistory.erase(m_vecLatencyHistory.begin());
		}
		m_vecLatencyHistory.push_back(currLatency);
	}
}

void PlayerConnection::Close()
{
	if (m_hSteamConnection != k_HSteamNetConnection_Invalid && SteamNetworkingSockets())
	{
		SteamNetworkingSockets()->CloseConnection(m_hSteamConnection, 0, "Graceful disconnect", false);
		m_hSteamConnection = k_HSteamNetConnection_Invalid;
	}
	m_State = EConnectionState::NOT_CONNECTED;
}

bool PlayerConnection::IsIPV4()
{
	if (m_hSteamConnection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets())
		return false;

	SteamNetConnectionInfo_t info;
	SteamNetworkingSockets()->GetConnectionInfo(m_hSteamConnection, &info);
	return info.m_addrRemote.IsIPv4();
}

int PlayerConnection::Recv(SteamNetworkingMessage_t** pMsg)
{
	if (m_hSteamConnection != k_HSteamNetConnection_Invalid && SteamNetworkingSockets())
	{
		return SteamNetworkingSockets()->ReceiveMessagesOnConnection(m_hSteamConnection, pMsg, 255);
	}
	return -1;
}

std::string PlayerConnection::GetStats()
{
	if (m_hSteamConnection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets())
		return "(disconnected)";

	char szBuf[2048] = { 0 };
	SteamNetworkingSockets()->GetDetailedConnectionStatus(m_hSteamConnection, szBuf, sizeof(szBuf));
	return std::string(szBuf);
}

std::string PlayerConnection::GetConnectionType()
{
	if (m_hSteamConnection == k_HSteamNetConnection_Invalid || !SteamNetworkingSockets())
		return "(disconnected)";

	SteamNetConnectionInfo_t info;
	if (SteamNetworkingSockets()->GetConnectionInfo(m_hSteamConnection, &info))
	{
		return info.m_szConnectionDescription;
	}
	return "(unknown)";
}

void PlayerConnection::UpdateState(EConnectionState newState, NetworkMesh* pOwningMesh)
{
	m_State = newState;
	if (pOwningMesh && pOwningMesh->m_cbOnConnected != nullptr)
	{
		pOwningMesh->m_cbOnConnected(m_userID, L"Player", this);
	}
}

void PlayerConnection::SetDisconnected(bool, NetworkMesh* pOwningMesh, bool)
{
	m_hSteamConnection = k_HSteamNetConnection_Invalid;
	UpdateState(EConnectionState::CONNECTION_DISCONNECTED, pOwningMesh);
}

int PlayerConnection::GetLatency()
{
	if (m_hSteamConnection != k_HSteamNetConnection_Invalid && SteamNetworkingSockets())
	{
		SteamNetConnectionRealTimeStatus_t status;
		if (SteamNetworkingSockets()->GetConnectionRealTimeStatus(m_hSteamConnection, &status, 0, nullptr) == k_EResultOK)
		{
			return status.m_nPing;
		}
	}
	return -1;
}

int PlayerConnection::GetJitter()
{
	return 0;
}

float PlayerConnection::GetConnectionQuality()
{
	return 1.0f;
}

int PlayerConnection::ComputeConnectionScore()
{
	return 100;
}

void PlayerConnection::LiteUpdateForAC()
{
}

#else // !defined(SAGE_USE_GAMENETWORKINGSOCKETS)

NetworkMesh::NetworkMesh()
{
}

void NetworkMesh::Flush()
{
}

void NetworkMesh::RegisterConnectivity(int64_t)
{
}

void NetworkMesh::UpdateConnectivity(PlayerConnection*)
{
}

int NetworkMesh::SendGamePacket(void*, uint32_t, int64_t)
{
	return -1;
}

void NetworkMesh::SendACPacket(uint32_t, const void*, uint32_t)
{
}

void NetworkMesh::StartConnectionSignalling(const char*, int64_t, uint16_t)
{
}

void NetworkMesh::DisconnectUser(int64_t)
{
}

void NetworkMesh::Disconnect()
{
	m_mapConnections.clear();
}

void NetworkMesh::Tick()
{
}

PlayerConnection::PlayerConnection(int64_t userID, HSteamNetConnection hSteamConnection)
	: m_userID(userID)
	, m_hSteamConnection(hSteamConnection)
	, m_ConnectionType(EConnectionType::BuiltIn_ValveSockets)
{
}

PlayerConnection::PlayerConnection(int64_t userID, const char* szMiddlewareID)
	: m_userID(userID)
	, m_strMiddlewareID(szMiddlewareID ? szMiddlewareID : "")
	, m_ConnectionType(EConnectionType::MiddlewarePluginGeneric)
{
}

int PlayerConnection::SendGamePacket(void*, uint32_t)
{
	return -1;
}

void PlayerConnection::SendACPacket(const void*, uint32_t)
{
}

void PlayerConnection::UpdateLatencyHistogram()
{
}

void PlayerConnection::Close()
{
	m_State = EConnectionState::NOT_CONNECTED;
}

bool PlayerConnection::IsIPV4()
{
	return true;
}

int PlayerConnection::Recv(SteamNetworkingMessage_t**)
{
	return 0;
}

std::string PlayerConnection::GetStats()
{
	return "(p2p mesh disabled)";
}

std::string PlayerConnection::GetConnectionType()
{
	return "(fallback)";
}

void PlayerConnection::UpdateState(EConnectionState newState, NetworkMesh* pOwningMesh)
{
	m_State = newState;
	if (pOwningMesh && pOwningMesh->m_cbOnConnected != nullptr)
	{
		pOwningMesh->m_cbOnConnected(m_userID, L"Player", this);
	}
}

void PlayerConnection::SetDisconnected(bool, NetworkMesh* pOwningMesh, bool)
{
	UpdateState(EConnectionState::CONNECTION_DISCONNECTED, pOwningMesh);
}

int PlayerConnection::GetLatency()
{
	return -1;
}

int PlayerConnection::GetJitter()
{
	return 0;
}

float PlayerConnection::GetConnectionQuality()
{
	return 1.0f;
}

int PlayerConnection::ComputeConnectionScore()
{
	return 100;
}

void PlayerConnection::LiteUpdateForAC()
{
}

#endif // defined(SAGE_USE_GAMENETWORKINGSOCKETS)
