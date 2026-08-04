// GeneralsX @feature GeneralsOnline NGMP OnlineServices Manager header
// Thread-safe manager for Next-Gen Multiplayer protocol lifecycle and event dispatching.

#ifndef ONLINE_SERVICES_MANAGER_H
#define ONLINE_SERVICES_MANAGER_H

#include "GameNetwork/GeneralsOnline/NGMPChatSession.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>

struct NGMPEvent {
    enum Type {
        EVENT_NONE,
        EVENT_AUTH_SUCCESS,
        EVENT_AUTH_FAILURE,
        EVENT_LOBBY_LIST_UPDATED,
        EVENT_CHAT_MESSAGE_RECEIVED,
        EVENT_CHAT_CONNECTED,
        EVENT_CHAT_DISCONNECTED,
        EVENT_DISCONNECTED
    };

    Type type = EVENT_NONE;
    std::string payload;
};

struct NGMPLobby {
    std::string id;
    std::string name;
    std::string mapName;
    int currentPlayers = 0;
    int maxPlayers = 8;
};

class NGMP_OnlineServicesManager {
public:
    static NGMP_OnlineServicesManager& getInstance();

    bool init();
    void update(); // Main thread UI tick dispatch
    void shutdown();

    // Async login — result delivered via EVENT_AUTH_SUCCESS / EVENT_AUTH_FAILURE
    void loginAsync(const std::string& username, const std::string& password);
    bool loginWithToken(const std::string& token);
    void logout();

    // Async lobby fetch — result delivered via EVENT_LOBBY_LIST_UPDATED
    void requestLobbyListAsync();

    bool sendChatMessage(const std::string& room, const std::string& message);

    bool isLoggedIn() const { return m_isLoggedIn; }
    std::string getAuthToken() const { return m_authToken; }
    std::string getUsername() const { return m_username; }
    const std::vector<NGMPLobby>& getLobbies() const { return m_lobbies; }

    // Internal thread-safe event poster (called from worker threads)
    void postEvent(const NGMPEvent& event);

private:
    NGMP_OnlineServicesManager();
    ~NGMP_OnlineServicesManager();

    NGMP_OnlineServicesManager(const NGMP_OnlineServicesManager&) = delete;
    NGMP_OnlineServicesManager& operator=(const NGMP_OnlineServicesManager&) = delete;

    bool m_initialized = false;
    bool m_isLoggedIn = false;
    std::string m_username;
    std::string m_authToken;
    std::vector<NGMPLobby> m_lobbies;

    // Async login state
    std::atomic<bool> m_loginInFlight = false;
    std::thread m_loginThread;

    // Async lobby request state
    std::atomic<bool> m_lobbyRequestInFlight = false;
    std::thread m_lobbyThread;

    // Chat WebSocket session
    std::unique_ptr<NGMP::NGMPChatSession> m_chatSession;

    mutable std::mutex m_eventMutex;
    std::queue<NGMPEvent> m_eventQueue;
};

#endif // ONLINE_SERVICES_MANAGER_H
