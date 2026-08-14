// GeneralsX @feature GeneralsOnline NGMP Manager implementation
// Lifecycle management, event queue processing, and lobby REST requests.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "GameNetwork/GeneralsOnline/NGMPWebSocket.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include <cstdio>
#include <thread>
#include <curl/curl.h>
#include "GameNetwork/GeneralsOnline/NGMP_json.h"

using json = nlohmann::json;

NGMP_OnlineServicesManager& NGMP_OnlineServicesManager::getInstance() {
    static NGMP_OnlineServicesManager instance;
    return instance;
}

NGMP_OnlineServicesManager::NGMP_OnlineServicesManager() = default;
NGMP_OnlineServicesManager::~NGMP_OnlineServicesManager() {
    shutdown();
}

void NGMP_OnlineServicesManager::postEvent(const NGMPEvent& event) {
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void NGMP_OnlineServicesManager::update() {
    std::queue<NGMPEvent> pendingEvents;
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        std::swap(pendingEvents, m_eventQueue);
    }

    std::vector<NGMPEvent> uiEvents;
    while (!pendingEvents.empty()) {
        NGMPEvent ev = pendingEvents.front();
        pendingEvents.pop();

        switch (ev.type) {
            case NGMPEvent::EVENT_AUTH_SUCCESS:
                fprintf(stderr, "[NGMP-MainThread] Event: Auth Success (user=%s id=%lld)\n", m_username.c_str(), (long long)m_userId);
                m_isLoggedIn = true;
                if (TheGameSpyInfo) {
                    TheGameSpyInfo->setLocalName(AsciiString(m_username.c_str()));
                    TheGameSpyInfo->setLocalProfileID(static_cast<Int>(m_userId));
                }
                if (!m_chatSession) {
                    m_chatSession.reset(new NGMP::NGMPWebSocket());
                    m_chatSession->setMessageCallback([this](const std::string& rawJson) {
                        NGMPEvent ev;
                        ev.type = NGMPEvent::EVENT_WEBSOCKET_MESSAGE;
                        ev.payload = rawJson;
                        postEvent(ev);
                    });
                }
                m_chatSession->connect(m_wsUri, m_authToken);
                break;
            case NGMPEvent::EVENT_AUTH_FAILURE:
                fprintf(stderr, "[NGMP-MainThread] Event: Auth Failure: %s\n", ev.payload.c_str());
                break;
            case NGMPEvent::EVENT_LOBBY_LIST_UPDATED:
                fprintf(stderr, "[NGMP-MainThread] Event: Lobby list updated (%zu lobbies)\n", m_lobbies.size());
                break;
            case NGMPEvent::EVENT_CHAT_MESSAGE_RECEIVED:
                fprintf(stderr, "[NGMP-MainThread] Event: Chat msg: %s\n", ev.payload.c_str());
                break;
            case NGMPEvent::EVENT_CHAT_CONNECTED:
                fprintf(stderr, "[NGMP-MainThread] Event: Chat connected\n");
                break;
            case NGMPEvent::EVENT_CHAT_DISCONNECTED:
                fprintf(stderr, "[NGMP-MainThread] Event: Chat disconnected\n");
                break;
            case NGMPEvent::EVENT_WEBSOCKET_MESSAGE:
                {
                    try {
                        auto jsonMsg = nlohmann::json::parse(ev.payload);
                        if (jsonMsg.contains("msg_id") && jsonMsg["msg_id"].is_number_integer()) {
                            int msgId = jsonMsg["msg_id"].get<int>();
                            
                            if (msgId == 2) { // NETWORK_ROOM_CHAT_FROM_SERVER
                                std::string msgText = "";
                                if (jsonMsg.contains("message") && jsonMsg["message"].is_string()) {
                                    msgText = jsonMsg["message"].get<std::string>();
                                }
                                NGMPEvent chatEv;
                                chatEv.type = NGMPEvent::EVENT_CHAT_MESSAGE_RECEIVED;
                                chatEv.payload = msgText;
                                uiEvents.push_back(chatEv);
                            } 
                            else if (msgId == 4) { // NETWORK_ROOM_MEMBER_LIST_UPDATE
                                if (jsonMsg.contains("members") && jsonMsg["members"].is_array()) {
                                    std::vector<NGMPLobbyPlayer> updatedPlayers;
                                    for (const auto& member : jsonMsg["members"]) {
                                        NGMPLobbyPlayer player;
                                        player.id = member.value("UserID", 0LL);
                                        player.name = member.value("Name", "");
                                        player.isAdmin = member.value("IsAdmin", false);
                                        updatedPlayers.push_back(player);
                                    }
                                    {
                                        std::lock_guard<std::mutex> lock(m_eventMutex);
                                        m_lobbyPlayers = std::move(updatedPlayers);
                                    }
                                    NGMPEvent playersEv;
                                    playersEv.type = NGMPEvent::EVENT_PLAYERS_UPDATED;
                                    uiEvents.push_back(playersEv);
                                }
                            }
                            else if (msgId == 7) { // NETWORK_ROOM_LOBBY_LIST_UPDATE
                                requestLobbyListAsync();
                            }
                        }
                    } catch (...) {
                        fprintf(stderr, "[NGMP] Failed to parse WS message: %s\n", ev.payload.c_str());
                    }
                }
                break;
            default:
                break;
        }
        if (ev.type != NGMPEvent::EVENT_WEBSOCKET_MESSAGE) {
            uiEvents.push_back(ev);
        }
    }
    
    if (!uiEvents.empty()) {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        for (const auto& ev : uiEvents) {
            m_uiEventQueue.push(ev);
        }
    }
}

std::vector<NGMPEvent> NGMP_OnlineServicesManager::pollEvents() {
    update();
    
    std::vector<NGMPEvent> events;
    std::lock_guard<std::mutex> lock(m_eventMutex);
    while (!m_uiEventQueue.empty()) {
        events.push_back(m_uiEventQueue.front());
        m_uiEventQueue.pop();
    }
    return events;
}

void NGMP_OnlineServicesManager::requestLobbyListAsync() {
    if (m_lobbyRequestInFlight.exchange(true)) {
        fprintf(stderr, "[NGMP] Lobby request already in flight, ignoring duplicate\n");
        fflush(stderr);
        return;
    }

    if (m_lobbyThread.joinable()) {
        m_lobbyThread.join();
    }

    m_lobbyThread = std::thread([this]() {
        CURL* curl = curl_easy_init();
        if (!curl) {
            m_lobbyRequestInFlight = false;
            return;
        }

        std::string url = NGMP::GetAPIEndpoint("Lobbies");
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        fprintf(stderr, "[NGMP-DEBUG] requestLobbyListAsync sending Token: %s\n", m_authToken.c_str());
        fflush(stderr);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode == 200) {
            try {
                fprintf(stderr, "[NGMP-DEBUG] Lobbies JSON: %s\n", response.text.c_str());
                fflush(stderr);
                auto jsonList = json::parse(response.text);
                std::vector<NGMPLobby> lobbies;
                if (jsonList.contains("lobbies") && jsonList["lobbies"].is_array()) {
                    for (const auto& item : jsonList["lobbies"]) {
                        NGMPLobby lobby;
                        lobby.id = item.value("lobbyID", 0LL);
                        lobby.name = item.contains("Name") ? item.value("Name", "Custom Lobby") : item.value("name", "Custom Lobby");
                        lobby.mapName = item.contains("MapName") ? item.value("MapName", "Tournament Desert") : item.value("map_name", "Tournament Desert");
                        lobby.currentPlayers = item.value("current_players", 1);
                        lobby.maxPlayers = item.value("max_players", 8);
                        lobby.hasPassword = item.value("has_password", false);
                        lobbies.push_back(lobby);
                    }
                }

                // Swap into member under the event mutex for safe handoff
                {
                    std::lock_guard<std::mutex> lock(m_eventMutex);
                    m_lobbies = std::move(lobbies);
                }

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_LOBBY_LIST_UPDATED;
                postEvent(ev);
            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] Lobby JSON parse exception: %s\n", e.what());
                fflush(stderr);
            }
        } else {
            fprintf(stderr, "[NGMP] Lobby request failed (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }

    m_lobbyRequestInFlight = false;
    });
}

void NGMP_OnlineServicesManager::createLobbyAsync(const std::string& name, const std::string& mapName, const std::string& password, int maxPlayers) {
    std::thread([this, name, mapName, password, maxPlayers]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint("Lobbies");
        NGMP::Internal::CurlResponse response;

        json payload = {
            {"name", name},
            {"map_name", mapName},
            {"map_path", mapName}, // Fallback for map path
            {"map_official", true},
            {"max_players", maxPlayers},
            {"preferred_port", 0},
            {"vanilla_teams", false},
            {"track_stats", false},
            {"starting_cash", 10000},
            {"passworded", !password.empty()},
            {"password", password},
            {"allow_observers", true},
            {"max_cam_height", 300},
            {"exe_crc", 0},
            {"ini_crc", 0},
            {"anticheat_id", 0}
        };
        std::string payloadStr = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && (httpCode == 200 || httpCode == 201)) {
            requestLobbyListAsync();
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_CREATED;
            postEvent(ev);
        } else {
            fprintf(stderr, "[NGMP] Create lobby failed (curl=%d, http=%ld, resp=%s)\n", res, httpCode, response.text.c_str());
            fflush(stderr);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::joinLobbyAsync(int64_t lobbyId, const std::string& password) {
    std::thread([this, lobbyId, password]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint(("Lobby/" + std::to_string(lobbyId)).c_str());
        NGMP::Internal::CurlResponse response;

        json payload = {
            {"preferred_port", 0},
            {"anticheat_id", 0},
            {"has_map", true},
            {"password", password}
        };
        std::string payloadStr = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && (httpCode == 200 || httpCode == 201)) {
            requestLobbyListAsync();
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_JOINED;
            postEvent(ev);
        } else {
            fprintf(stderr, "[NGMP] Join lobby failed (curl=%d, http=%ld, resp=%s)\n", res, httpCode, response.text.c_str());
            fflush(stderr);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::requestPlaylistsAsync() {
    if (m_playlistsRequestInFlight.exchange(true)) {
        fprintf(stderr, "[NGMP] Playlists request already in flight, ignoring duplicate\n");
        fflush(stderr);
        return;
    }

    if (m_playlistsThread.joinable()) {
        m_playlistsThread.join();
    }

    m_playlistsThread = std::thread([this]() {
        CURL* curl = curl_easy_init();
        if (!curl) {
            m_playlistsRequestInFlight = false;
            return;
        }

        std::string url = NGMP::GetAPIEndpoint("matchmaking/playlists");
        NGMP::Internal::CurlResponse response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode == 200) {
            try {
                auto jsonList = json::parse(response.text);
                std::vector<PlaylistEntry> playlists;
                if (jsonList.is_array()) {
                    for (const auto& item : jsonList) {
                        PlaylistEntry entry;
                        entry.PlaylistID = item.value("playlistID", -1);
                        entry.Name = item.value("name", "Unknown Playlist");
                        entry.MinPlayers = item.value("minPlayers", 2);
                        entry.DesiredPlayers = item.value("desiredPlayers", 2);
                        entry.MinSelectedMaps = item.value("minSelectedMaps", 1);
                        entry.AllowTeams = item.value("allowTeams", false);
                        entry.TeamSize = item.value("teamSize", -1);
                        entry.AllowArmySelection = item.value("allowArmySelection", true);
                        entry.GracePeriodAtMinPlayersMSec = item.value("gracePeriodAtMinPlayersMSec", 0);
                        
                        auto mapsArr = item.value("maps", json::array());
                        for (const auto& mapItem : mapsArr) {
                            PlaylistMapEntry mapEntry;
                            mapEntry.Name = mapItem.value("name", "");
                            mapEntry.Path = mapItem.value("path", "");
                            mapEntry.Custom = mapItem.value("custom", false);
                            entry.Maps.push_back(mapEntry);
                        }
                        playlists.push_back(entry);
                    }
                }

                // Swap into member under the event mutex for safe handoff
                {
                    std::lock_guard<std::mutex> lock(m_eventMutex);
                    m_playlists = std::move(playlists);
                }

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_PLAYLISTS_UPDATED;
                postEvent(ev);
            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] Playlists JSON parse exception: %s\n", e.what());
                fflush(stderr);
            }
        } else {
            fprintf(stderr, "[NGMP] Playlists request failed (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }

        m_playlistsRequestInFlight = false;
    });
}

void NGMP_OnlineServicesManager::startMatchmakingAsync(uint16_t playlistID, const std::vector<int>& selectedMapIndexes) {
    std::thread([this, playlistID, selectedMapIndexes]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        json payload;
        payload["playlist"] = playlistID;
        payload["maps"] = selectedMapIndexes;
        // payload["exe_crc"] = TheGlobalData->m_exeCRC;
        // payload["ini_crc"] = TheGlobalData->m_iniCRC;
        // payload["anticheat_id"] = "";

        std::string payloadStr = payload.dump();
        std::string url = NGMP::GetAPIEndpoint("matchmaking");
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode == 201) {
            fprintf(stderr, "[NGMP] Matchmaking started successfully\n");
            fflush(stderr);
        } else {
            fprintf(stderr, "[NGMP] Failed to start matchmaking (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::cancelMatchmakingAsync() {
    std::thread([this]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint("matchmaking");
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode == 200) {
            fprintf(stderr, "[NGMP] Matchmaking cancelled successfully\n");
            fflush(stderr);
        } else {
            fprintf(stderr, "[NGMP] Failed to cancel matchmaking (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }
    }).detach();
}

bool NGMP_OnlineServicesManager::hasGlobalStats() const {
    return m_hasGlobalStats;
}

GlobalStats NGMP_OnlineServicesManager::getGlobalStats() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_statsMutex));
    return m_globalStats;
}

bool NGMP_OnlineServicesManager::sendChatMessage(const std::string& room, const std::string& message) {
    if (!m_isLoggedIn) {
        return false;
    }
    if (m_chatSession) {
        // Build the chat message payload
        nlohmann::json payload = {
            {"msg_id", 1},
            {"action", "chat"},
            {"message", message}
        };
        return m_chatSession->sendPayload(payload.dump());
    }
    fprintf(stderr, "[NGMP] sendChatMessage called but no active chat session\n");
    fflush(stderr);
    return false;
}

void NGMP_OnlineServicesManager::changeNetworkRoom(int16_t roomID) {
    if (!m_isLoggedIn) {
        fprintf(stderr, "[NGMP] changeNetworkRoom(%d) ignored: not logged in\n", roomID);
        fflush(stderr);
        return;
    }
    if (m_chatSession && m_chatSession->isConnected()) {
        fprintf(stderr, "[NGMP] changeNetworkRoom(%d): sending msg_id=3 (room=%d)\n", roomID, roomID);
        fflush(stderr);
        nlohmann::json payload = {
            {"msg_id", 3}, // NETWORK_ROOM_CHANGE_ROOM
            {"room", roomID}
        };
        m_chatSession->sendPayload(payload.dump());
    } else {
        fprintf(stderr, "[NGMP] changeNetworkRoom(%d): WS chat session not active or not connected\n", roomID);
        fflush(stderr);
    }
}
