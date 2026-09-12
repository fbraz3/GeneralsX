// GeneralsX @feature GeneralsOnline NGMP Chat WebSocket session header
// Manages a persistent WebSocket connection to the NGMP backend chat server.

#ifndef NGMP_WEBSOCKET_H
#define NGMP_WEBSOCKET_H

#include <curl/curl.h>

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

#pragma pop_macro("max")
#pragma pop_macro("min")

namespace NGMP {

enum class EWebSocketMessageID {
    UNKNOWN = -1,
    NETWORK_ROOM_CHAT_FROM_CLIENT = 1,
    NETWORK_ROOM_CHAT_FROM_SERVER = 2,
    NETWORK_ROOM_CHANGE_ROOM = 3,
    NETWORK_ROOM_MEMBER_LIST_UPDATE = 4,
    NETWORK_ROOM_MARK_READY = 5,
    LOBBY_CURRENT_LOBBY_UPDATE = 6,
    NETWORK_ROOM_LOBBY_LIST_UPDATE = 7,
    ANTICHEAT_MESSAGE = 8,
    PLAYER_NAME_CHANGE = 9,
    LOBBY_ROOM_CHAT_FROM_CLIENT = 10,
    LOBBY_CHAT_FROM_SERVER = 11,
    NETWORK_SIGNAL = 12,
    START_GAME = 13,
    PING = 14,
    PONG = 15,
    PROBE = 16,
    NETWORK_CONNECTION_START_SIGNALLING = 17,
    NETWORK_CONNECTION_DISCONNECT_PLAYER = 18,
    NETWORK_CONNECTION_CLIENT_REQUEST_SIGNALLING = 19,
    MATCHMAKING_ACTION_JOIN_PREARRANGED_LOBBY = 20,
    MATCHMAKING_ACTION_START_GAME = 21,
    MATCHMAKING_MESSAGE = 22,
    START_GAME_COUNTDOWN_STARTED = 23,
    LOBBY_REMOVE_PASSWORD = 24,
    LOBBY_CHANGE_PASSWORD = 25,
    FULL_MESH_CONNECTIVITY_CHECK_HOST_REQUESTS_BEGIN = 26,
    FULL_MESH_CONNECTIVITY_CHECK_RESPONSE = 27,
    FULL_MESH_CONNECTIVITY_CHECK_RESPONSE_COMPLETE_TO_HOST = 28,
    SOCIAL_NEW_FRIEND_REQUEST = 29,
    SOCIAL_FRIEND_CHAT_MESSAGE_CLIENT_TO_SERVER = 30,
    SOCIAL_FRIEND_CHAT_MESSAGE_SERVER_TO_CLIENT = 31,
    SOCIAL_FRIEND_ONLINE_STATUS_CHANGED = 32,
    SOCIAL_SUBSCRIBE_REALTIME_UPDATES = 33,
    SOCIAL_UNSUBSCRIBE_REALTIME_UPDATES = 34,
    SOCIAL_FRIENDS_OVERALL_STATUS_UPDATE = 35,
    SOCIAL_FRIEND_FRIEND_REQUEST_ACCEPTED_BY_TARGET = 36,
};

using GenericMessageCallback = std::function<void(const std::string& rawJson)>;

class NGMPWebSocket {
public:
    NGMPWebSocket() = default;
    ~NGMPWebSocket();

    // Connect to the WebSocket endpoint (blocking until connected or failed)
    bool connect(const std::string& wsUrl, const std::string& authToken);

    // Disconnect from the WebSocket and stop the receiver thread
    void disconnect();

    // Returns true if currently connected
    bool isConnected() const { return m_running.load(); }

    // Send a generic string payload (like a serialized JSON)
    bool sendPayload(const std::string& payload);

    // Set the generic callback invoked on the receiver thread when any message arrives
    // The callback receives the raw JSON and must post to the NGMP event queue
    void setMessageCallback(GenericMessageCallback cb) { m_messageCallback = std::move(cb); }

private:
    void receiveLoop();

    CURL* m_curl = nullptr;
    std::thread m_recvThread;
    std::atomic<bool> m_running = false;
    GenericMessageCallback m_messageCallback;
    mutable std::mutex m_sendMutex;
};

} // namespace NGMP

#endif // NGMP_WEBSOCKET_H
