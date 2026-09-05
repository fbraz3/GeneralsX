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
#include <queue>
#include <cstring>
#include <cstdio>

#if defined(SAGE_USE_GAMENETWORKINGSOCKETS)

static std::atomic<bool> g_bNetworkMeshDestroying = false;
static std::mutex g_pendingDeletionMutex;
static std::vector<void*> g_pendingConnSignalingDeletions;

class CSignalingClient : public ISignalingClient
{
public:
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

private:
	struct QueuedSend
	{
		int64_t target_user_id;
		std::vector<uint8_t> vecPayload;
	};

	ISteamNetworkingSockets* const m_pSteamNetworkingSockets;
	std::mutex m_signalsMutex;
	std::deque<QueuedSend> m_queueSend;
	std::queue<std::vector<uint8_t>> m_pendingSignals;

public:
	CSignalingClient(ISteamNetworkingSockets* pSteamNetworkingSockets)
		: m_pSteamNetworkingSockets(pSteamNetworkingSockets)
	{
	}

	void Send(int64_t target_user_id, std::vector<uint8_t>& vecPayload)
	{
		std::lock_guard<std::mutex> lock(m_signalsMutex);
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
		int64_t user_id = -1;
		const char* peerIdStr = identityPeer.GetGenericString();
		if (peerIdStr) {
			try {
				user_id = std::stoll(peerIdStr);
			} catch (...) {}
		}
		return new ConnectionSignaling(this, user_id);
	}

	void PushIncomingSignal(const std::vector<uint8_t>& signalPayload) override
	{
		std::lock_guard<std::mutex> lock(m_signalsMutex);
		m_pendingSignals.push(signalPayload);
	}

	void Poll() override
	{
		NGMP_OnlineServicesManager* pMgr = NGMP_OnlineServicesManager::GetInstance();
		if (!pMgr)
			return;

		std::deque<QueuedSend> sendBatch;
		std::queue<std::vector<uint8_t>> recvBatch;

		{
			std::lock_guard<std::mutex> lock(m_signalsMutex);
			sendBatch.swap(m_queueSend);
			recvBatch.swap(m_pendingSignals);
		}

		while (!sendBatch.empty())
		{
			QueuedSend sendData = sendBatch.front();
			sendBatch.pop_front();

			nlohmann::json j;
			j["msg_id"] = 12; // NETWORK_SIGNAL
			j["target_user_id"] = sendData.target_user_id;
			j["payload"] = sendData.vecPayload;
			pMgr->sendRawWebSocketPayload(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
		}

		if (!recvBatch.empty())
		{
			struct Context : ISteamNetworkingSignalingRecvContext
			{
				CSignalingClient* m_pOwner;

				ISteamNetworkingConnectionSignaling* OnConnectRequest(
					HSteamNetConnection,
					const SteamNetworkingIdentity& identityPeer,
					int
				) override
				{
					SteamNetworkingErrMsg ignoreErrMsg;
					return m_pOwner->CreateSignalingForConnection(identityPeer, ignoreErrMsg);
				}

				void SendRejectionSignal(
					const SteamNetworkingIdentity&,
					const void*, int
				) override
				{
					fprintf(stderr, "[STEAM NETWORKING] Sending rejection signal\n");
					fflush(stderr);
				}
			};

			Context context;
			context.m_pOwner = this;

			while (!recvBatch.empty())
			{
				std::vector<uint8_t> signalData = recvBatch.front();
				recvBatch.pop();

				fprintf(stderr, "[STEAM NETWORKING] Processing incoming P2P signal (size=%zu)\n", signalData.size());
				fflush(stderr);

				m_pSteamNetworkingSockets->ReceivedP2PCustomSignal(signalData.data(), static_cast<int>(signalData.size()), &context);
			}
		}
	}

	void Release() override
	{
		std::lock_guard<std::mutex> lock(m_signalsMutex);
		m_queueSend.clear();
		while (!m_pendingSignals.empty()) recvBatch_clear();
	}

private:
	void recvBatch_clear()
	{
		while (!m_pendingSignals.empty()) m_pendingSignals.pop();
	}
};

static void CleanupPendingConnSignalingDeletions()
{
	std::vector<void*> objectsToDelete;
	{
		std::lock_guard<std::mutex> lock(g_pendingDeletionMutex);
		objectsToDelete.swap(g_pendingConnSignalingDeletions);
	}
	for (void* pObj : objectsToDelete)
	{
		delete static_cast<CSignalingClient::ConnectionSignaling*>(pObj);
	}
}

// GeneralsX @bugfix fbraz3 31/08/2026 Full SteamNetworkingSockets connection lifecycle and signalling retries
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
		fprintf(stderr, "[STEAM NETWORKING][%s] %s, reason %d: %s\n",
			pInfo->m_info.m_szConnectionDescription,
			(pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ? "closed by peer" : "problem detected locally"),
			pInfo->m_info.m_eEndReason,
			pInfo->m_info.m_szEndDebug
		);
		fflush(stderr);

		if (SteamNetworkingSockets())
		{
			SteamNetworkingSockets()->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
		}

		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];

			const int numSignallingAttempts = 3;
			bool bShouldRetry = plrConnection.m_SignallingAttempts < numSignallingAttempts;
			bool bWasError = pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally || pInfo->m_info.m_eEndReason != k_ESteamNetConnectionEnd_App_Generic;
			plrConnection.SetDisconnected(bWasError, pMesh, bShouldRetry && bWasError);

			if (bWasError)
			{
				fprintf(stderr, "[STEAM NETWORKING][DISCONNECT HANDLER] Determined we didn't connect due to an error, Retrying: %d (currently at %d/%d attempts)\n",
					bShouldRetry ? 1 : 0, plrConnection.m_SignallingAttempts, numSignallingAttempts);
				fflush(stderr);

				if (bShouldRetry)
				{
					std::shared_ptr<WebSocket> pWS = NGMP_OnlineServicesManager::GetWebSocket();
					if (pWS)
					{
						NGMP_OnlineServices_AuthInterface* pAuthInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_AuthInterface>();
						if (pAuthInterface != nullptr)
						{
							int64_t myUserID = pAuthInterface->GetUserID();
							if (myUserID > plrConnection.m_userID)
							{
								fprintf(stderr, "[STEAM NETWORKING][DISCONNECT HANDLER] Send signal start request to %lld\n", (long long)plrConnection.m_userID);
								fflush(stderr);
								pWS->SendData_RequestSignalling(plrConnection.m_userID);
							}
							else
							{
								fprintf(stderr, "[STEAM NETWORKING][DISCONNECT HANDLER] Not sending signal start request, other player (%lld) should\n", (long long)plrConnection.m_userID);
								fflush(stderr);
							}
						}
					}
					else
					{
						bShouldRetry = false;
					}
				}

				if (!bShouldRetry)
				{
					fprintf(stderr, "[STEAM NETWORKING][DISCONNECT HANDLER] Not retrying, handling disconnect as failure for user %lld...\n", (long long)plrConnection.m_userID);
					fflush(stderr);

					NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
					if (pLobbyInterface != nullptr)
					{
						auto callbackCopy = pLobbyInterface->m_OnCannotConnectToLobbyCallback;
						if (callbackCopy != nullptr)
						{
							callbackCopy();
						}
					}
				}
			}
		}
		break;

	case k_ESteamNetworkingConnectionState_None:
		break;

	case k_ESteamNetworkingConnectionState_Connecting:
		if (pMesh->GetListenSocketHandle() != k_HSteamListenSocket_Invalid && pInfo->m_info.m_hListenSocket == pMesh->GetListenSocketHandle())
		{
			fprintf(stderr, "[STEAM NETWORKING][%s] Considering Accepting incoming connection\n", pInfo->m_info.m_szConnectionDescription);
			fflush(stderr);

			const char* remoteGenericStr = pInfo->m_info.m_identityRemote.GetGenericString();
			if (connections.count(connectionID))
			{
				PlayerConnection& plrConnection = connections[connectionID];
				plrConnection.UpdateState(EConnectionState::CONNECTING_DIRECT, pMesh);
				plrConnection.m_hSteamConnection = pInfo->m_hConn;
			}
			else
			{
				int64_t remoteUserID = -1;
				if (remoteGenericStr)
				{
					try {
						remoteUserID = std::stoll(remoteGenericStr);
					} catch (...) {}
				}

				if (remoteUserID > 0)
				{
					PlayerConnection newConn(remoteUserID, pInfo->m_hConn);
					newConn.UpdateState(EConnectionState::CONNECTING_DIRECT, pMesh);
					connections[remoteUserID] = newConn;
					connectionID = remoteUserID;
				}
			}

			NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
			bool bPlayerIsInLobby = false;
			if (pLobbyInterface != nullptr && remoteGenericStr != nullptr)
			{
				auto& currentLobby = pLobbyInterface->GetCurrentLobby();
				for (const auto& member : currentLobby.members)
				{
					if (std::to_string(member.user_id) == remoteGenericStr)
					{
						bPlayerIsInLobby = true;
						break;
					}
				}
			}

			if (bPlayerIsInLobby)
			{
				fprintf(stderr, "[STEAM NETWORKING][%s] Accepting - Player (%s) is in lobby\n",
					pInfo->m_info.m_szConnectionDescription, remoteGenericStr ? remoteGenericStr : "unknown");
				fflush(stderr);
				SteamNetworkingSockets()->AcceptConnection(pInfo->m_hConn);
			}
			else
			{
				fprintf(stderr, "[STEAM NETWORKING][%s] Rejecting - Player (%s) is not in lobby\n",
					pInfo->m_info.m_szConnectionDescription, remoteGenericStr ? remoteGenericStr : "unknown");
				fflush(stderr);
				SteamNetworkingSockets()->CloseConnection(pInfo->m_hConn, 1000, "Player is not in lobby (Rejected)", false);
			}
		}
		else
		{
			fprintf(stderr, "[STEAM NETWORKING][%s] Entered connecting state\n", pInfo->m_info.m_szConnectionDescription);
			fflush(stderr);
			if (connectionID != -1)
			{
				PlayerConnection& plrConnection = connections[connectionID];
				plrConnection.UpdateState(EConnectionState::CONNECTING_DIRECT, pMesh);
			}
		}
		break;

	case k_ESteamNetworkingConnectionState_FindingRoute:
		fprintf(stderr, "[STEAM NETWORKING][%s] finding route\n", pInfo->m_info.m_szConnectionDescription);
		fflush(stderr);
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.UpdateState(EConnectionState::FINDING_ROUTE, pMesh);
		}
		break;

	case k_ESteamNetworkingConnectionState_Connected:
		fprintf(stderr, "[STEAM NETWORKING][%s] CONNECTED successfully!\n", pInfo->m_info.m_szConnectionDescription);
		fflush(stderr);
		if (connectionID != -1)
		{
			PlayerConnection& plrConnection = connections[connectionID];
			plrConnection.UpdateState(EConnectionState::CONNECTED_DIRECT, pMesh);
		}
		break;

	default:
		fprintf(stderr, "[STEAM CALLBACK] Unhandled connection state: %d\n", (int)pInfo->m_info.m_eState);
		fflush(stderr);
		break;
	}
}

NetworkMesh::NetworkMesh()
{
	NGMP_OnlineServicesManager* pMgr = NGMP_OnlineServicesManager::GetInstance();
	int64_t localUserID = pMgr ? pMgr->getUserId() : 0;

	SteamNetworkingIdentity identityLocal;
	identityLocal.Clear();
	std::string localUserIDStr = std::to_string(localUserID);
	identityLocal.SetGenericString(localUserIDStr.c_str());

	SteamDatagramErrMsg errMsg;
	if (!GameNetworkingSockets_Init(&identityLocal, errMsg))
	{
		fprintf(stderr, "[STEAM NETWORKING] GameNetworkingSockets_Init failed: %s\n", errMsg);
		fflush(stderr);
		return;
	}

	m_pSignaling = new CSignalingClient(SteamNetworkingSockets());

	SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(OnSteamNetConnectionStatusChanged);
	g_bNetworkMeshDestroying.store(false);

	int localPort = 0;
	SteamNetworkingConfigValue_t opt;
	opt.SetInt32(k_ESteamNetworkingConfig_SymmetricConnect, 1);
	m_hListenSock = SteamNetworkingSockets()->CreateListenSocketP2P(localPort, 1, &opt);

	fprintf(stderr, "[STEAM NETWORKING] NetworkMesh initialized for local user %lld (listenSock=%u)\n", (long long)localUserID, m_hListenSock);
	fflush(stderr);
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
		fprintf(stderr, "[STEAM NETWORKING] Could not create signalling object for remote user %lld: %s\n", (long long)remoteUserID, errMsg);
		fflush(stderr);
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
		fprintf(stderr, "[STEAM NETWORKING] ConnectP2PCustomSignaling returned k_HSteamNetConnection_Invalid for remote user %lld\n", (long long)remoteUserID);
		fflush(stderr);
		return;
	}

	PlayerConnection newConnection(remoteUserID, hConn);
	newConnection.m_ConnectionType = EConnectionType::BuiltIn_ValveSockets;
	newConnection.m_SignallingAttempts = 1;
	m_mapConnections[remoteUserID] = newConnection;

	fprintf(stderr, "[STEAM NETWORKING] ConnectP2PCustomSignaling started to user %lld (conn=%u, attempt=1)\n", (long long)remoteUserID, hConn);
	fflush(stderr);
}

void NetworkMesh::PushIncomingSignal(const std::vector<uint8_t>& signalPayload)
{
	if (m_pSignaling != nullptr)
	{
		m_pSignaling->PushIncomingSignal(signalPayload);
	}
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
	if (m_pSignaling)
	{
		m_pSignaling->Poll();
	}
	if (SteamNetworkingSockets())
	{
		SteamNetworkingSockets()->RunCallbacks();
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

void NetworkMesh::PushIncomingSignal(const std::vector<uint8_t>&)
{
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
