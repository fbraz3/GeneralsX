// GeneralsX @feature GeneralsOnline PluginInterfaces & connection state definitions
#pragma once

#include <cstdint>
#include <string>

enum class EConnectionState : uint8_t
{
    NOT_CONNECTED,
    CONNECTING_DIRECT,
    FINDING_ROUTE,
    CONNECTED_DIRECT,
    CONNECTION_FAILED,
    CONNECTION_DISCONNECTED
};

enum class ENetworkChannels : uint8_t
{
    Game = 0,
    Anticheat,
    Signalling
};

enum class EPacketReliability : int32_t
{
    PACKET_RELIABILITY_UNRELIABLE_UNORDERED = 0,
    PACKET_RELIABILITY_RELIABLE_UNORDERED = 1,
    PACKET_RELIABILITY_RELIABLE_ORDERED = 2
};

enum class EAnticheatActionType : int32_t
{
    NONE = 0,
    KICK = 1
};

enum class EAnticheatActionReason : int32_t
{
    Unknown = 0,
    InternalError = 1,
    InvalidMessage = 2,
    AuthFailure = 3,
    ACNotRunning = 4,
    HeartbeatTimedOut = 5,
    ClientViolation = 6,
    BackendViolation = 7,
    TempCooldown = 8,
    TempBanned = 9,
    PermaBanned = 10
};

class AnticheatPlugInterface
{
public:
    static bool DoesACPluginProvideSecureGameTransport() { return false; }
    static int GetNextRecvPacketSize(uint8_t) { return 0; }
    static bool RecvPacket(uint8_t**, uint8_t) { return false; }
    static void AC_NetworkMessageArrived(uint32_t, void*, uint32_t) {}
};
