// GeneralsX @feature GeneralsOnline NGMP OnlineServices Manager header
// Thread-safe manager for Next-Gen Multiplayer protocol lifecycle and event dispatching.

#ifndef ONLINE_SERVICES_MANAGER_H
#define ONLINE_SERVICES_MANAGER_H

#include "GameNetwork/GeneralsOnline/NGMPWebSocket.h"
#include "Common/GameDefines.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>

#pragma pop_macro("max")
#pragma pop_macro("min")

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
        EVENT_PLAYER_STATS_RECEIVED,
        EVENT_WEBSOCKET_MESSAGE,
        EVENT_PLAYLISTS_UPDATED,
        EVENT_LOBBY_JOINED,
        EVENT_LOBBY_CREATED,
        EVENT_PLAYERS_UPDATED
    };

    Type type = EVENT_NONE;
    std::string payload;
};

struct NGMPLobby {
    int64_t id;
    std::string name;
    std::string mapName;
    int maxPlayers;
    int currentPlayers;
    bool hasPassword;
};

struct NGMPLobbyPlayer {
    int64_t id;
    std::string name;
    bool isAdmin;
};

struct GlobalStats {
    std::vector<int> wins;
    std::vector<int> matches;
};

struct PlaylistMapEntry {
    std::string Name;
    std::string Path;
    bool Custom = false;
};

struct PlaylistEntry {
    uint16_t PlaylistID = -1;
    std::string Name;
    int MinPlayers = -1;
    int DesiredPlayers = -1;
    int MinSelectedMaps = 0;
    bool AllowTeams = false;
    int TeamSize = -1;
    bool AllowArmySelection = false;
    uint16_t GracePeriodAtMinPlayersMSec = 0;
    std::vector<PlaylistMapEntry> Maps;
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
    void createLobbyAsync(const std::string& name, const std::string& mapName, const std::string& password, int maxPlayers);
    void joinLobbyAsync(int64_t lobbyId, const std::string& password);

    // Async playlists fetch
    void requestPlaylistsAsync();
    const std::vector<PlaylistEntry>& getPlaylists() const { return m_playlists; }
    void startMatchmakingAsync(uint16_t playlistID, const std::vector<int>& selectedMapIndexes);
    void cancelMatchmakingAsync();

    // Async stats fetch
    void requestGlobalStatsAsync();
    bool hasGlobalStats() const;
    GlobalStats getGlobalStats() const;

    void requestPlayerStatsAsync(int64_t userID);
    bool getCachedPlayerStats(int64_t userID, PSPlayerStats& outStats) const;

    bool sendChatMessage(const std::string& room, const std::string& message);
    void changeNetworkRoom(int16_t roomID);

    bool isLoggedIn() const { return m_isLoggedIn; }
    std::string getAuthToken() const { return m_authToken; }
    std::string getUsername() const { return m_username; }
    const std::vector<NGMPLobby>& getLobbies() const { return m_lobbies; }
    const std::vector<NGMPLobbyPlayer>& getLobbyPlayers() const { return m_lobbyPlayers; }

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
    std::vector<NGMPLobbyPlayer> m_lobbyPlayers;

    // Browser-based login state
    std::atomic<bool> m_waitingBrowserLogin = false;
    std::string m_gamecode;
    std::chrono::steady_clock::time_point m_lastPollTime;
    std::thread m_pollThread;
    std::atomic<bool> m_pollThreadRunning = false;

    // Async lobby request state
    std::atomic<bool> m_lobbyRequestInFlight = false;
    std::thread m_lobbyThread;

    // Matchmaking state
    std::atomic<bool> m_playlistsRequestInFlight = false;
    std::vector<PlaylistEntry> m_playlists;
    std::thread m_playlistsThread;

    // Async stats state
    std::atomic<bool> m_hasGlobalStats = false;
    std::atomic<bool> m_statsRequestInFlight = false;
    std::mutex m_statsMutex;
    GlobalStats m_globalStats;
    std::thread m_statsThread;

    mutable std::mutex m_playerStatsMutex;
    std::unordered_map<int64_t, PSPlayerStats> m_cachedPlayerStats;

    // Chat WebSocket session
    std::unique_ptr<NGMP::NGMPWebSocket> m_chatSession;

    mutable std::mutex m_eventMutex;
    std::queue<NGMPEvent> m_eventQueue;
};

#endif // ONLINE_SERVICES_MANAGER_H
