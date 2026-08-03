// GeneralsX @feature GeneralsOnline NGMP OnlineServices Manager header
// Thread-safe manager for Next-Gen Multiplayer protocol lifecycle and event dispatching.

#ifndef ONLINE_SERVICES_MANAGER_H
#define ONLINE_SERVICES_MANAGER_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>

struct NGMPEvent {
    enum Type {
        EVENT_NONE,
        EVENT_AUTH_SUCCESS,
        EVENT_AUTH_FAILURE,
        EVENT_LOBBY_LIST_UPDATED,
        EVENT_CHAT_MESSAGE_RECEIVED,
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

    bool login(const std::string& username, const std::string& password);
    bool loginWithToken(const std::string& token);
    void logout();

    void requestLobbyList();
    bool sendChatMessage(const std::string& room, const std::string& message);

    bool isLoggedIn() const { return m_isLoggedIn; }
    std::string getAuthToken() const { return m_authToken; }
    std::string getUsername() const { return m_username; }
    const std::vector<NGMPLobby>& getLobbies() const { return m_lobbies; }

    // Internal thread-safe event poster
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

    mutable std::mutex m_eventMutex;
    std::queue<NGMPEvent> m_eventQueue;
};

#endif // ONLINE_SERVICES_MANAGER_H
