// GeneralsX @feature GeneralsOnline NetworkMesh header
#pragma once

#ifdef _WIN32
#include <ws2ipdef.h>
#endif
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <functional>
#include <cstdint>

#if defined(SAGE_USE_GAMENETWORKINGSOCKETS)
#include <steam/steamnetworkingsockets.h>
#include <steam/steamnetworkingcustomsignaling.h>
#include <steam/isteamnetworkingutils.h>
#else
typedef uint32_t HSteamNetConnection;
const HSteamNetConnection k_HSteamNetConnection_Invalid = 0;
typedef uint32_t HSteamListenSocket;
const HSteamListenSocket k_HSteamListenSocket_Invalid = 0;
struct SteamNetworkingMessage_t { void* m_pData; uint32_t m_cbSize; void Release() {} const void* GetData() const { return m_pData; } };
struct SteamNetworkingIdentity { void Clear() {} const char* GetGenericString() const { return ""; } };
struct SteamNetConnectionInfo_t { char m_szConnectionDescription[128]; struct { bool IsIPv4() const { return true; } } m_addrRemote; };
typedef char SteamNetworkingErrMsg[1024];
class ISteamNetworkingConnectionSignaling { public: virtual ~ISteamNetworkingConnectionSignaling() {} virtual void Release() {} };
#endif

#include "GameNetwork/GeneralsOnline/PluginInterfaces.h"
#include "Common/AsciiString.h"

// Trivial signalling client interface
class ISignalingClient
{
public:
	virtual ~ISignalingClient() = default;
	virtual ISteamNetworkingConnectionSignaling* CreateSignalingForConnection(const SteamNetworkingIdentity& identityPeer, SteamNetworkingErrMsg& errMsg) = 0;
	virtual void PushIncomingSignal(const std::vector<uint8_t>& signalPayload) = 0;
	virtual void Poll() = 0;
	virtual void Release() = 0;
};

enum class EConnectionType
{
	Unknown = -1,
	BuiltIn_ValveSockets = 0,
	MiddlewarePluginGeneric = 1
};

class NetworkMesh;

class PlayerConnection
{
public:
	PlayerConnection()
	{
		m_ConnectionType = EConnectionType::Unknown;
		m_hSteamConnection = k_HSteamNetConnection_Invalid;
		m_strMiddlewareID = std::string("NOT SET");
	}

	PlayerConnection(int64_t userID, HSteamNetConnection hSteamConnection);
	PlayerConnection(int64_t userID, const char* szMiddlewareID);

	EConnectionState GetState() const { return m_State; }

	int SendGamePacket(void* pBuffer, uint32_t totalDataSize);
	void SendACPacket(const void* pData, uint32_t dataLen);
	void UpdateLatencyHistogram();
	void Close();

	bool IsIPV4();
	bool IsDirect()
	{
		std::string strConnectionType = GetConnectionType();
		return strConnectionType.find("Relayed") == std::string::npos;
	}

	bool IsValid() const
	{
		return m_State != EConnectionState::NOT_CONNECTED &&
		       m_State != EConnectionState::CONNECTION_FAILED &&
		       m_State != EConnectionState::CONNECTION_DISCONNECTED;
	}

	int Recv(SteamNetworkingMessage_t** pMsg);

	int GetHighestHistoricalLatency()
	{
		int highestLatency = 0;
		for (int latencyHistory : m_vecLatencyHistory)
		{
			if (latencyHistory > highestLatency)
			{
				highestLatency = latencyHistory;
			}
		}
		return highestLatency;
	}

	std::vector<int> m_vecLatencyHistory;
	std::vector<float> m_vecQualityHistory;
	std::string GetStats();
	std::string GetConnectionType();

	void UpdateState(EConnectionState newState, NetworkMesh* pOwningMesh);
	void SetDisconnected(bool bWasError, NetworkMesh* pOwningMesh, bool bIsRetrying);

	int64_t m_userID = -1;
	EConnectionType m_ConnectionType = EConnectionType::Unknown;
	EConnectionState m_State = EConnectionState::NOT_CONNECTED;
	int64_t pingSent = -1;
	int m_SignallingAttempts = 0;

	int GetLatency();
	int GetJitter();
	float GetConnectionQuality();
	int ComputeConnectionScore();

	HSteamNetConnection m_hSteamConnection = k_HSteamNetConnection_Invalid;
	std::string m_strMiddlewareID = std::string("NOT SET");

	void LiteUpdateForAC();
};

class NetworkMesh
{
public:
	NetworkMesh();
	~NetworkMesh()
	{
		Disconnect();
		if (m_pSignaling != nullptr)
		{
			delete m_pSignaling;
			m_pSignaling = nullptr;
		}
	}

	void Flush();
	void RegisterConnectivity(int64_t userID);
	void UpdateConnectivity(PlayerConnection* connection);

	std::function<void(int64_t, std::wstring, PlayerConnection*)> m_cbOnConnected = nullptr;
	void RegisterForConnectionEvents(std::function<void(int64_t, std::wstring, PlayerConnection*)> cb)
	{
		m_cbOnConnected = cb;
	}

	void DeregisterForConnectionEvents()
	{
		m_cbOnConnected = nullptr;
	}

	int getMaximumLatency()
	{
		int highestLatency = 0;
		for (auto& kvPair : m_mapConnections)
		{
			PlayerConnection& conn = kvPair.second;
			if (conn.GetLatency() > highestLatency)
			{
				highestLatency = conn.GetLatency();
			}
		}
		return highestLatency;
	}

	float getMaximumHistoricalLatency()
	{
		int highestLatency = 0;
		for (auto& kvPair : m_mapConnections)
		{
			PlayerConnection& conn = kvPair.second;
			if (conn.GetHighestHistoricalLatency() > highestLatency)
			{
				highestLatency = conn.GetHighestHistoricalLatency();
			}
		}
		return static_cast<float>(highestLatency);
	}

	int SendGamePacket(void* pBuffer, uint32_t totalDataSize, int64_t userID);
	void SendACPacket(uint32_t userID, const void* pData, uint32_t dataLen);

	void StartConnectionSignalling(const char* szMiddlewareID, int64_t remoteUserID, uint16_t preferredPort);
	void PushIncomingSignal(const std::vector<uint8_t>& signalPayload);
	void DisconnectUser(int64_t remoteUserID);
	void Disconnect();

	void Tick();

	HSteamListenSocket GetListenSocketHandle() const { return m_hListenSock; }

	std::map<int64_t, PlayerConnection>& GetAllConnections()
	{
		return m_mapConnections;
	}

	PlayerConnection* GetConnectionForUser(int64_t user_id)
	{
		auto it = m_mapConnections.find(user_id);
		if (it != m_mapConnections.end())
		{
			return &it->second;
		}
		return nullptr;
	}

private:
	std::map<int64_t, PlayerConnection> m_mapConnections;
	mutable std::recursive_mutex m_mapConnectionsMutex;

	ISignalingClient* m_pSignaling = nullptr;
	HSteamListenSocket m_hListenSock = k_HSteamListenSocket_Invalid;
	bool m_bDisconnected = false;

	std::string m_strTurnUsername;
	std::string m_strTurnToken;
	std::string m_strTurnUsernameString;
	std::string m_strTurnTokenString;
};
