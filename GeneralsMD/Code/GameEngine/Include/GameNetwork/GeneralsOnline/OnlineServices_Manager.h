// GeneralsX @feature GeneralsOnline NGMP OnlineServices Manager header
// Thread-safe manager for Next-Gen Multiplayer protocol lifecycle and event dispatching.

#ifndef ONLINE_SERVICES_MANAGER_H
#define ONLINE_SERVICES_MANAGER_H

#include "GameNetwork/GeneralsOnline/NGMPChatSession.h"
#include "Common/GameDefines.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>

struct NGMPEvent {
    enum Type {
        EVENT_NONE,
        EVENT_AUTH_SUCCESS,
        EVENT_AUTH_FAILURE,
        EVENT_AUTH_CANCELLED,
        EVENT_LOBBY_LIST_UPDATED,
        EVENT_CHAT_MESSAGE_RECEIVED,
        EVENT_CHAT_CONNECTED,
        EVENT_CHAT_DISCONNECTED,
        EVENT_DISCONNECTED,
        EVENT_GLOBAL_STATS_RECEIVED,
        EVENT_PLAYER_STATS_RECEIVED
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

struct GlobalStats {
    std::vector<int> wins;
    std::vector<int> matches;
};

class NGMP_OnlineServicesManager {
public:
    static NGMP_OnlineServicesManager& getInstance();

    bool init();
    std::vector<NGMPEvent> pollEvents(); // Main thread UI tick dispatch
    void shutdown();

    // Browser-based gamecode login flow (macOS/Linux: uses SDL_OpenURL)
    void beginBrowserLogin();
    void cancelBrowserLogin();
    // Call from main-thread update loop while waiting for browser login
    void tickBrowserLogin();

    // Token-based silent re-login
    void loginWithRefreshToken(const std::string& refreshToken);

    void logout();

    // Async lobby fetch — result delivered via EVENT_LOBBY_LIST_UPDATED
    void requestLobbyListAsync();

    // Async stats fetch
    void requestGlobalStatsAsync();
    bool hasGlobalStats() const;
    GlobalStats getGlobalStats() const;

    void requestPlayerStatsAsync(int64_t userID);
    bool getCachedPlayerStats(int64_t userID, PSPlayerStats& outStats) const;

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
    std::string m_wsUri;
    std::vector<NGMPLobby> m_lobbies;

    // Browser-based login state
    std::atomic<bool> m_waitingBrowserLogin = false;
    std::string m_gamecode;
    std::chrono::steady_clock::time_point m_lastPollTime;
    std::thread m_pollThread;
    std::atomic<bool> m_pollThreadRunning = false;

    // Async lobby request state
    std::atomic<bool> m_lobbyRequestInFlight = false;
    std::thread m_lobbyThread;

    // Async stats state
    std::atomic<bool> m_hasGlobalStats = false;
    std::atomic<bool> m_statsRequestInFlight = false;
    std::mutex m_statsMutex;
    GlobalStats m_globalStats;
    std::thread m_statsThread;

    mutable std::mutex m_playerStatsMutex;
    std::unordered_map<int64_t, PSPlayerStats> m_cachedPlayerStats;

    // Chat WebSocket session
    std::unique_ptr<NGMP::NGMPChatSession> m_chatSession;

    mutable std::mutex m_eventMutex;
    std::queue<NGMPEvent> m_eventQueue;
};

#endif // ONLINE_SERVICES_MANAGER_H
