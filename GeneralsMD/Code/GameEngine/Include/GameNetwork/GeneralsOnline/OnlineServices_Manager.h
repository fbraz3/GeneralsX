// GeneralsX @feature GeneralsOnline NGMP OnlineServices Manager header
// Thread-safe manager for Next-Gen Multiplayer protocol lifecycle and event dispatching.

#ifndef ONLINE_SERVICES_MANAGER_H
#define ONLINE_SERVICES_MANAGER_H

#include "GameNetwork/GeneralsOnline/NGMPWebSocket.h"
#include "Common/GameDefines.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <type_traits>

class NGMP_OnlineServices_AuthInterface;
class NGMP_OnlineServices_LobbyInterface;
class NGMP_OnlineServices_RoomsInterface;
class NGMP_OnlineServices_StatsInterface;
class NGMP_OnlineServices_SocialInterface;

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
        EVENT_LOBBY_JOIN_FAILED,
        EVENT_LOBBY_CREATED,
        EVENT_LOBBY_CREATE_FAILED,
        EVENT_LOBBY_LEFT,
        EVENT_HOST_MIGRATED,
        EVENT_PLAYERS_UPDATED
    };

    Type type = EVENT_NONE;
    std::string payload;
};

struct NGMPLobby {
    int64_t id = -1;
    std::string name;
    std::string mapName;
    std::string mapPath;
    bool isOfficial = true;
    int maxPlayers = 8;
    int currentPlayers = 0;
    bool hasPassword = false;
};

struct NGMPLobbyPlayer {
    int64_t id = -1;
    std::string name;
    bool isAdmin = false;
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

class WebSocket {
public:
    void SendData_ChangeName(UnicodeString& strNewName);
    void SendData_RoomChatMessage(UnicodeString& msg, bool bIsAction);
    void SendData_FriendMessage(UnicodeString& msg, int64_t target_user_id);
    void SendData_LobbyChatMessage(UnicodeString& msg, bool bIsAction, bool bIsAnnouncement, bool bShowAnnouncementToHost);
    void SendData_JoinNetworkRoom(int roomID);
    void SendData_LeaveNetworkRoom();
    void SendData_MarkReady(bool bReady);
    void SendData_StartGame();
    void SendData_CountdownStarted();
};

class NGMP_OnlineServicesManager {
public:
    static NGMP_OnlineServicesManager& getInstance();
    static NGMP_OnlineServicesManager* GetInstance() { return &getInstance(); }

    template<typename T>
    static T* GetInterface()
    {
        NGMP_OnlineServicesManager& mgr = getInstance();
        if constexpr (std::is_same_v<T, NGMP_OnlineServices_AuthInterface>)
            return mgr.m_pAuthInterface;
        else if constexpr (std::is_same_v<T, NGMP_OnlineServices_LobbyInterface>)
            return mgr.m_pLobbyInterface;
        else if constexpr (std::is_same_v<T, NGMP_OnlineServices_RoomsInterface>)
            return mgr.m_pRoomInterface;
        else if constexpr (std::is_same_v<T, NGMP_OnlineServices_StatsInterface>)
            return mgr.m_pStatsInterface;
        else if constexpr (std::is_same_v<T, NGMP_OnlineServices_SocialInterface>)
            return mgr.m_pSocialInterface;
        return nullptr;
    }

    static std::shared_ptr<WebSocket> GetWebSocket()
    {
        return getInstance().m_pWebSocketWrapper;
    }

    bool init();
    std::vector<NGMPEvent> pollEvents(); // Main thread UI tick dispatch
    void update(); // Main thread internal state tick

    void shutdown();

    // Browser-based gamecode login flow (macOS/Linux: uses SDL_OpenURL)
    void beginBrowserLogin();
    void cancelBrowserLogin();
    void tickBrowserLogin();

    // Token-based silent re-login
    void loginWithRefreshToken(const std::string& refreshToken);

    void logout();

    // Async lobby fetch — result delivered via EVENT_LOBBY_LIST_UPDATED
    void requestLobbyListAsync();
    void createLobbyAsync(const std::string& name, const std::string& mapName, const std::string& mapPath, bool isOfficial = true, int maxPlayers = 8, bool vanillaTeamsOnly = false, bool trackStats = false, uint32_t startingCash = 10000, bool isPassworded = false, const std::string& password = "", bool allowObservers = true);
    void joinLobbyAsync(int64_t lobbyId, const std::string& password);

    // Staging room / lobby details synchronization
    int64_t getCurrentLobbyId() const { return m_currentLobbyId; }
    void setCurrentLobbyId(int64_t id) { m_currentLobbyId = id; }
    bool isLobbyOwner() const { return m_isLobbyOwner; }

    void requestLobbyDetailsAsync(int64_t lobbyId = -1);

    // Staging room modification methods (matching ELobbyUpdateField)
    void updateLobbyMap(const std::string& mapName, const std::string& mapPath, bool isOfficial, int maxPlayers);
    void updateLobbyStartingCash(int startingCash);
    void updateLobbyLimitSuperweapons(bool limit);
    void updateLobbyMySide(int side, int startPos);
    void updateLobbyMyColor(int color);
    void updateLobbyMyTeam(int team);
    void updateLobbyMyStartPos(int startPos);
    void updateLobbySlotState(int slotIndex, int slotState);
    void updateLobbyAISide(int slot, int side, int startPos = -1);
    void updateLobbyAIColor(int slot, int color);
    void updateLobbyAITeam(int slot, int team);
    void updateLobbyAIStartPos(int slot, int startPos);
    void updateLobbyHasMap(bool hasMap);
    void updateLobbyForceStart();
    void updateLobbyLeave(int64_t lobbyId = -1);

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
    bool sendRawWebSocketPayload(const std::string& rawPayload);
    void changeNetworkRoom(int16_t roomID);

    bool isLoggedIn() const { return m_isLoggedIn; }
    std::string getAuthToken() const { return m_authToken; }
    std::string getUsername() const { return m_username; }
    int64_t getUserId() const { return m_userId; }
    const std::vector<NGMPLobby>& getLobbies() const { return m_lobbies; }
    const std::vector<NGMPLobbyPlayer>& getLobbyPlayers() const { return m_lobbyPlayers; }

    // Internal thread-safe event poster (called from worker threads)
    void postEvent(const NGMPEvent& event);

    NGMP_OnlineServices_AuthInterface* m_pAuthInterface = nullptr;
    NGMP_OnlineServices_LobbyInterface* m_pLobbyInterface = nullptr;
    NGMP_OnlineServices_RoomsInterface* m_pRoomInterface = nullptr;
    NGMP_OnlineServices_StatsInterface* m_pStatsInterface = nullptr;
    NGMP_OnlineServices_SocialInterface* m_pSocialInterface = nullptr;
    std::shared_ptr<WebSocket> m_pWebSocketWrapper;

private:
    NGMP_OnlineServicesManager();
    ~NGMP_OnlineServicesManager();

    NGMP_OnlineServicesManager(const NGMP_OnlineServicesManager&) = delete;
    NGMP_OnlineServicesManager& operator=(const NGMP_OnlineServicesManager&) = delete;

    bool m_initialized = false;
    bool m_isLoggedIn = false;
    std::string m_username;
    int64_t m_userId = 0;
    std::string m_authToken;
    std::string m_wsUri;
    std::vector<NGMPLobby> m_lobbies;
    std::vector<NGMPLobbyPlayer> m_lobbyPlayers;

    // Staging room state
    int64_t m_currentLobbyId = -1;
    int64_t m_hostUserId = -1;
    bool m_isLobbyOwner = false;

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
    std::queue<NGMPEvent> m_uiEventQueue;
};

#endif // ONLINE_SERVICES_MANAGER_H
