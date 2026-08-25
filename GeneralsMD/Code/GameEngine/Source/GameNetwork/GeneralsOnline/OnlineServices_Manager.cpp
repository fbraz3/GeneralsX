// GeneralsX @feature GeneralsOnline NGMP Manager implementation
// Lifecycle management, event queue processing, and lobby REST requests.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "GameNetwork/GeneralsOnline/NGMPWebSocket.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_LobbyInterface.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_RoomsInterface.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"
#include "GameClient/MapUtil.h"
#include "Common/Money.h"
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
                        fprintf(stderr, "[NGMP] EVENT_WEBSOCKET_MESSAGE raw: %s\n", ev.payload.c_str());
                        fflush(stderr);
                        auto jsonMsg = nlohmann::json::parse(ev.payload);
                        if (jsonMsg.contains("msg_id") && jsonMsg["msg_id"].is_number_integer()) {
                            int msgId = jsonMsg["msg_id"].get<int>();
                            
                            // GeneralsX @refactor fbraz3 23/08/2026 Process WebSocket events into UI event queue for main-thread dispatch
                            if (msgId == 4) { // NETWORK_ROOM_MEMBER_LIST_UPDATE
                                if (jsonMsg.contains("members") && jsonMsg["members"].is_array()) {
                                    std::vector<NGMPLobbyPlayer> updatedPlayers;
                                    for (const auto& member : jsonMsg["members"]) {
                                        NGMPLobbyPlayer player;
                                        if (member.contains("UserID") && !member["UserID"].is_null()) {
                                            if (member["UserID"].is_number()) player.id = member["UserID"].get<int64_t>();
                                            else if (member["UserID"].is_string()) player.id = std::stoll(member["UserID"].get<std::string>());
                                        } else if (member.contains("user_id") && !member["user_id"].is_null()) {
                                            if (member["user_id"].is_number()) player.id = member["user_id"].get<int64_t>();
                                            else if (member["user_id"].is_string()) player.id = std::stoll(member["user_id"].get<std::string>());
                                        }

                                        if (member.contains("Name") && member["Name"].is_string()) {
                                            player.name = member["Name"].get<std::string>();
                                        } else if (member.contains("name") && member["name"].is_string()) {
                                            player.name = member["name"].get<std::string>();
                                        } else if (member.contains("display_name") && member["display_name"].is_string()) {
                                            player.name = member["display_name"].get<std::string>();
                                        } else if (member.contains("DisplayName") && member["DisplayName"].is_string()) {
                                            player.name = member["DisplayName"].get<std::string>();
                                        }

                                        if (member.contains("IsAdmin") && member["IsAdmin"].is_boolean()) {
                                            player.isAdmin = member["IsAdmin"].get<bool>();
                                        } else if (member.contains("is_admin") && member["is_admin"].is_boolean()) {
                                            player.isAdmin = member["is_admin"].get<bool>();
                                        }
                                        updatedPlayers.push_back(player);
                                    }
                                    {
                                        std::lock_guard<std::mutex> lock(m_eventMutex);
                                        m_lobbyPlayers = std::move(updatedPlayers);
                                    }
                                    fprintf(stderr, "[NGMP] Updated lobby player roster (%zu players)\n", m_lobbyPlayers.size());
                                    for (const auto& p : m_lobbyPlayers) {
                                        fprintf(stderr, "  [LobbyPlayer] id=%lld, name='%s', isAdmin=%d\n", (long long)p.id, p.name.c_str(), p.isAdmin);
                                    }
                                    fflush(stderr);

                                    NGMPEvent playersEv;
                                    playersEv.type = NGMPEvent::EVENT_PLAYERS_UPDATED;
                                    uiEvents.push_back(playersEv);
                                }
                            }
                            else if (msgId == 2) { // NETWORK_ROOM_CHAT_FROM_SERVER
                                std::string msgText = "";
                                if (jsonMsg.contains("message") && jsonMsg["message"].is_string()) {
                                    msgText = jsonMsg["message"].get<std::string>();
                                }
                                fprintf(stderr, "[NGMP] Room Chat received: '%s'\n", msgText.c_str());
                                fflush(stderr);

                                NGMPEvent chatEv;
                                chatEv.type = NGMPEvent::EVENT_CHAT_MESSAGE_RECEIVED;
                                chatEv.payload = msgText;
                                uiEvents.push_back(chatEv);
                            }
                            else if (msgId == 6) { // LOBBY_CURRENT_LOBBY_UPDATE
                                fprintf(stderr, "[NGMP] WS msg_id=6 (LOBBY_CURRENT_LOBBY_UPDATE), refreshing lobby %lld\n", (long long)m_currentLobbyId);
                                fflush(stderr);
                                if (m_currentLobbyId >= 0) {
                                    requestLobbyDetailsAsync(m_currentLobbyId);
                                }
                            }
                            else if (msgId == 7) { // NETWORK_ROOM_LOBBY_LIST_UPDATE
                                requestLobbyListAsync();
                                if (m_currentLobbyId >= 0 && !m_isLobbyOwner) {
                                    requestLobbyDetailsAsync(m_currentLobbyId);
                                }
                            }
                            else if (msgId == 11) { // LOBBY_CHAT_FROM_SERVER
                                std::string msgText = "";
                                if (jsonMsg.contains("message") && jsonMsg["message"].is_string()) {
                                    msgText = jsonMsg["message"].get<std::string>();
                                } else if (jsonMsg.contains("Message") && jsonMsg["Message"].is_string()) {
                                    msgText = jsonMsg["Message"].get<std::string>();
                                }
                                fprintf(stderr, "[NGMP] Lobby Chat received: '%s'\n", msgText.c_str());
                                fflush(stderr);

                                NGMPEvent chatEv;
                                chatEv.type = NGMPEvent::EVENT_CHAT_MESSAGE_RECEIVED;
                                chatEv.payload = msgText;
                                uiEvents.push_back(chatEv);
                            }
                            else if (msgId == 18) { // NETWORK_CONNECTION_DISCONNECT_PLAYER
                                int64_t disconnectedUserId = -1;
                                if (jsonMsg.contains("user_id") && jsonMsg["user_id"].is_number()) {
                                    disconnectedUserId = jsonMsg["user_id"].get<int64_t>();
                                }
                                fprintf(stderr, "[NGMP] WS msg_id=18 (NETWORK_CONNECTION_DISCONNECT_PLAYER): user %lld disconnected (host=%lld, me=%lld)\n",
                                    (long long)disconnectedUserId, (long long)m_hostUserId, (long long)m_userId);
                                fflush(stderr);

                                if (m_currentLobbyId >= 0) {
                                    requestLobbyDetailsAsync(m_currentLobbyId);
                                }
                            }
                        }
                    } catch (const std::exception& e) {
                        fprintf(stderr, "[NGMP] Failed to parse WS message (%s): %s\n", e.what(), ev.payload.c_str());
                        fflush(stderr);
                    } catch (...) {
                        fprintf(stderr, "[NGMP] Failed to parse WS message: %s\n", ev.payload.c_str());
                        fflush(stderr);
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
                // GeneralsX @bugfix fbraz3 15/08/2026 Parse PascalCase and camelCase lobby fields from server
                if (jsonList.contains("lobbies") && jsonList["lobbies"].is_array()) {
                    for (const auto& item : jsonList["lobbies"]) {
                        NGMPLobby lobby;
                        if (item.contains("LobbyID") && !item["LobbyID"].is_null()) {
                            lobby.id = item["LobbyID"].is_number() ? item["LobbyID"].get<int64_t>() : std::stoll(item["LobbyID"].get<std::string>());
                        } else if (item.contains("lobbyID") && !item["lobbyID"].is_null()) {
                            lobby.id = item["lobbyID"].is_number() ? item["lobbyID"].get<int64_t>() : std::stoll(item["lobbyID"].get<std::string>());
                        } else if (item.contains("lobby_id") && !item["lobby_id"].is_null()) {
                            lobby.id = item["lobby_id"].is_number() ? item["lobby_id"].get<int64_t>() : std::stoll(item["lobby_id"].get<std::string>());
                        }

                        if (item.contains("Name") && item["Name"].is_string()) {
                            lobby.name = item["Name"].get<std::string>();
                        } else if (item.contains("name") && item["name"].is_string()) {
                            lobby.name = item["name"].get<std::string>();
                        }

                        if (item.contains("MapName") && item["MapName"].is_string()) {
                            lobby.mapName = item["MapName"].get<std::string>();
                        } else if (item.contains("map_name") && item["map_name"].is_string()) {
                            lobby.mapName = item["map_name"].get<std::string>();
                        }

                        if (item.contains("NumCurrentPlayers") && item["NumCurrentPlayers"].is_number()) {
                            lobby.currentPlayers = item["NumCurrentPlayers"].get<int>();
                        } else if (item.contains("current_players") && item["current_players"].is_number()) {
                            lobby.currentPlayers = item["current_players"].get<int>();
                        }

                        if (item.contains("MaxPlayers") && item["MaxPlayers"].is_number()) {
                            lobby.maxPlayers = item["MaxPlayers"].get<int>();
                        } else if (item.contains("max_players") && item["max_players"].is_number()) {
                            lobby.maxPlayers = item["max_players"].get<int>();
                        }

                        if (item.contains("IsPassworded") && item["IsPassworded"].is_boolean()) {
                            lobby.hasPassword = item["IsPassworded"].get<bool>();
                        } else if (item.contains("is_passworded") && item["is_passworded"].is_boolean()) {
                            lobby.hasPassword = item["is_passworded"].get<bool>();
                        } else if (item.contains("has_password") && item["has_password"].is_boolean()) {
                            lobby.hasPassword = item["has_password"].get<bool>();
                        }
                        lobbies.push_back(lobby);
                    }
                }

                // Swap into member under the event mutex for safe handoff
                {
                    std::lock_guard<std::mutex> lock(m_eventMutex);
                    m_lobbies = lobbies;
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

void NGMP_OnlineServicesManager::createLobbyAsync(const std::string& name, const std::string& mapName, const std::string& mapPath, bool isOfficial, int maxPlayers, bool vanillaTeamsOnly, bool trackStats, uint32_t startingCash, bool isPassworded, const std::string& password, bool allowObservers) {
    std::thread([this, name, mapName, mapPath, isOfficial, maxPlayers, vanillaTeamsOnly, trackStats, startingCash, isPassworded, password, allowObservers]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint("Lobbies");
        NGMP::Internal::CurlResponse response;

        // Sanitize map path so server's FixMapPathForGame does not duplicate folder prefixes
        std::string sanitizedMapPath = mapPath;
        const char* lastSlash = strrchr(sanitizedMapPath.c_str(), '/');
        if (!lastSlash) lastSlash = strrchr(sanitizedMapPath.c_str(), '\\');
        if (lastSlash) {
            sanitizedMapPath = lastSlash + 1;
        }

        json payload = {
            {"name", name},
            {"map_name", mapName},
            {"map_path", sanitizedMapPath},
            {"map_official", isOfficial},
            {"max_players", maxPlayers},
            {"preferred_port", 0},
            {"vanilla_teams", vanillaTeamsOnly},
            {"track_stats", trackStats},
            {"starting_cash", startingCash},
            {"passworded", isPassworded || !password.empty()},
            {"password", password},
            {"allow_observers", allowObservers},
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
            int64_t createdLobbyId = -1;
            try {
                auto jsonResp = json::parse(response.text);
                if (jsonResp.contains("lobby_id")) {
                    createdLobbyId = jsonResp["lobby_id"].get<int64_t>();
                } else if (jsonResp.contains("LobbyID")) {
                    createdLobbyId = jsonResp["LobbyID"].get<int64_t>();
                }
            } catch (...) {}

            m_currentLobbyId = createdLobbyId;
            m_hostUserId = m_userId;
            m_isLobbyOwner = true;

            if (createdLobbyId >= 0) {
                requestLobbyDetailsAsync(createdLobbyId);
            }
            requestLobbyListAsync();
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_CREATED;
            postEvent(ev);
        } else {
            fprintf(stderr, "[NGMP] Create lobby failed (curl=%d, http=%ld, resp=%s)\n", res, httpCode, response.text.c_str());
            fflush(stderr);
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_CREATE_FAILED;
            postEvent(ev);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::joinLobbyAsync(int64_t lobbyId, const std::string& password) {
    m_currentLobbyId = lobbyId;
    m_hostUserId = -1;
    m_isLobbyOwner = false;

    std::thread([this, lobbyId, password]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint(("Lobby/" + std::to_string(lobbyId)).c_str());
        NGMP::Internal::CurlResponse response;

        json payload = {
            {"preferred_port", 0},
            {"anticheat_id", 0},
            {"has_map", true}
        };
        if (!password.empty()) {
            payload["password"] = password;
        }
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
            requestLobbyDetailsAsync(lobbyId);
            requestLobbyListAsync();
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_JOINED;
            postEvent(ev);
        } else {
            fprintf(stderr, "[NGMP] Join lobby failed (curl=%d, http=%ld, resp=%s)\n", res, httpCode, response.text.c_str());
            fflush(stderr);
            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_JOIN_FAILED;
            postEvent(ev);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::requestLobbyDetailsAsync(int64_t lobbyId) {
    int64_t targetId = (lobbyId >= 0) ? lobbyId : m_currentLobbyId;
    if (targetId < 0) return;

    std::thread([this, targetId]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint(("Lobby/" + std::to_string(targetId)).c_str());
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + m_authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK && httpCode == 200) {
            try {
                auto jsonRoot = json::parse(response.text);
                auto lobbyIter = jsonRoot.contains("lobby") ? jsonRoot["lobby"] : (jsonRoot.contains("Lobby") ? jsonRoot["Lobby"] : jsonRoot);

                int64_t ownerId = lobbyIter.value("Owner", lobbyIter.value("owner", int64_t(-1)));
                std::string mapName = lobbyIter.value("MapName", lobbyIter.value("map_name", ""));
                std::string mapPath = lobbyIter.value("MapPath", lobbyIter.value("map_path", ""));
                int startingCash = lobbyIter.value("StartingCash", lobbyIter.value("starting_cash", 10000));
                bool limitSuperweapons = lobbyIter.value("IsLimitSuperweapons", lobbyIter.value("is_limit_superweapons", false));
                bool allowObservers = lobbyIter.value("AllowObservers", lobbyIter.value("allow_observers", true));

                bool bHostMigrated = (m_hostUserId > 0 && ownerId != m_hostUserId);
                m_hostUserId = ownerId;
                m_currentLobbyId = targetId;
                m_isLobbyOwner = (ownerId == m_userId);

                int localSlotIndex = -1;
                std::vector<NGMPLobbyPlayer> updatedLobbyPlayers;


                std::vector<LobbyMemberEntry> lobbyMembers;

                // Populate members
                auto membersIter = lobbyIter.contains("Members") ? lobbyIter["Members"] : (lobbyIter.contains("members") ? lobbyIter["members"] : json::array());
                if (membersIter.is_array()) {
                    for (const auto& member : membersIter) {
                        int64_t memberUserId = member.value("UserID", member.value("user_id", int64_t(-1)));
                        int slotIdx = member.value("SlotIndex", member.value("slot_index", -1));
                        int slotState = member.value("SlotState", member.value("slot_state", 0));
                        std::string dispName = member.value("DisplayName", member.value("display_name", ""));
                        int side = member.value("Side", member.value("side", -1));
                        int color = member.value("Color", member.value("color", -1));
                        int team = member.value("Team", member.value("team", -1));
                        int startPos = member.value("StartingPosition", member.value("starting_position", -1));
                        bool hasMap = member.value("HasMap", member.value("has_map", true));
                        bool isReady = member.value("IsReady", member.value("is_ready", false));

                        if (memberUserId == m_userId && slotIdx >= 0) {
                            localSlotIndex = slotIdx;
                        }

                        if (memberUserId > 0 && !dispName.empty()) {
                            NGMPLobbyPlayer lp;
                            lp.id = memberUserId;
                            lp.name = dispName;
                            lp.isAdmin = (memberUserId == ownerId);
                            updatedLobbyPlayers.push_back(lp);
                        }

                        LobbyMemberEntry lme;
                        lme.user_id = memberUserId;
                        lme.display_name = dispName;
                        lme.side = side;
                        lme.color = color;
                        lme.team = team;
                        lme.startpos = startPos;
                        lme.has_map = hasMap;
                        lme.m_bIsReady = isReady;
                        lme.m_SlotState = static_cast<uint16_t>(slotState);
                        lme.m_SlotIndex = (slotIdx >= 0) ? static_cast<uint16_t>(slotIdx) : static_cast<uint16_t>(9999);
                        lobbyMembers.push_back(lme);

                    }
                }

                // Update NGMP LobbyInterface current lobby cache and roster under mutex
                {
                    std::lock_guard<std::mutex> lock(m_eventMutex);
                    NGMP_OnlineServices_LobbyInterface* pLobbyInterface = NGMP_OnlineServicesManager::GetInterface<NGMP_OnlineServices_LobbyInterface>();
                    if (pLobbyInterface) {
                        LobbyEntry& curLobby = pLobbyInterface->GetCurrentLobby();
                        curLobby.lobbyID = targetId;
                        curLobby.owner = ownerId;
                        curLobby.name = lobbyIter.value("Name", lobbyIter.value("name", ""));
                        curLobby.map_name = mapName;
                        curLobby.map_path = mapPath;
                        curLobby.map_official = lobbyIter.value("IsMapOfficial", lobbyIter.value("is_map_official", true));
                        curLobby.starting_cash = startingCash;
                        curLobby.limit_superweapons = limitSuperweapons;
                        curLobby.track_stats = lobbyIter.value("IsTrackingStats", lobbyIter.value("is_tracking_stats", true));
                        curLobby.allow_observers = allowObservers;
                        curLobby.rng_seed = lobbyIter.value("RNGSeed", lobbyIter.value("rng_seed", 0));
                        curLobby.exe_crc = lobbyIter.value("ExeCRC", lobbyIter.value("exe_crc", 0));
                        curLobby.ini_crc = lobbyIter.value("IniCRC", lobbyIter.value("ini_crc", 0));
                        curLobby.max_players = lobbyIter.value("MaxPlayers", lobbyIter.value("max_players", 8));
                        curLobby.current_players = lobbyIter.value("NumCurrentPlayers", lobbyIter.value("num_current_players", 1));
                        curLobby.members = std::move(lobbyMembers);
                    }

                    if (!updatedLobbyPlayers.empty()) {
                        m_lobbyPlayers = std::move(updatedLobbyPlayers);
                    }
                }

                // If guest is no longer part of the lobby members list, trigger lobby exit
                if (!m_isLobbyOwner && localSlotIndex < 0 && m_currentLobbyId == targetId) {
                    fprintf(stderr, "[NGMP] Local player %lld no longer in lobby %lld members roster\n",
                        (long long)m_userId, (long long)targetId);
                    fflush(stderr);
                    m_currentLobbyId = -1;
                    NGMPEvent leftEv;
                    leftEv.type = NGMPEvent::EVENT_LOBBY_LEFT;
                    postEvent(leftEv);
                    return;
                }

                fprintf(stderr, "[NGMP] Synchronized staging room with Lobby %lld (map=%s, cash=%d, superweapons=%d, isOwner=%d, localSlot=%d)\n",
                    (long long)targetId, mapName.c_str(), startingCash, limitSuperweapons, m_isLobbyOwner ? 1 : 0, localSlotIndex);
                fflush(stderr);

                if (bHostMigrated) {
                    fprintf(stderr, "[NGMP] Host migrated! New owner is %lld (amIHost=%d)\n", (long long)ownerId, m_isLobbyOwner ? 1 : 0);
                    fflush(stderr);
                    NGMPEvent migrateEv;
                    migrateEv.type = NGMPEvent::EVENT_HOST_MIGRATED;
                    migrateEv.payload = m_isLobbyOwner ? "1" : "0";
                    postEvent(migrateEv);
                }

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_PLAYERS_UPDATED;
                postEvent(ev);

            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] Failed to parse Lobby details (%s): %s\n", e.what(), response.text.c_str());
                fflush(stderr);
            }
        } else {
            fprintf(stderr, "[NGMP] GET Lobby %lld failed (curl=%d, http=%ld)\n", (long long)targetId, res, httpCode);
            fflush(stderr);
            if (!m_isLobbyOwner && (httpCode == 404 || httpCode == 400 || httpCode == 410)) {
                // Lobby was closed / host left
                m_currentLobbyId = -1;
                NGMPEvent leftEv;
                leftEv.type = NGMPEvent::EVENT_LOBBY_LEFT;
                postEvent(leftEv);
            }
        }
    }).detach();
}

static void sendLobbyPostUpdate(const std::string& authToken, int64_t lobbyId, const json& payload) {
    if (lobbyId < 0 || authToken.empty()) return;

    std::thread([authToken, lobbyId, payload]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint(("Lobby/" + std::to_string(lobbyId)).c_str());
        std::string payloadStr = payload.dump();
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + authToken;
        headers = curl_slist_append(headers, authHeader.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK || (httpCode != 200 && httpCode != 204)) {
            fprintf(stderr, "[NGMP] Lobby POST update failed (field=%d, curl=%d, http=%ld, resp=%s)\n",
                payload.value("field", -1), res, httpCode, response.text.c_str());
            fflush(stderr);
        }
    }).detach();
}

void NGMP_OnlineServicesManager::updateLobbyMap(const std::string& mapName, const std::string& mapPath, bool isOfficial, int maxPlayers) {
    std::string sanitizedMapPath = mapPath;
    const char* lastSlash = strrchr(sanitizedMapPath.c_str(), '/');
    if (!lastSlash) lastSlash = strrchr(sanitizedMapPath.c_str(), '\\');
    if (lastSlash) {
        sanitizedMapPath = lastSlash + 1;
    }

    json payload = {
        {"field", 0}, // LOBBY_MAP
        {"map", mapName},
        {"map_path", sanitizedMapPath},
        {"map_official", isOfficial},
        {"max_players", maxPlayers}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyStartingCash(int startingCash) {
    json payload = {
        {"field", 5}, // LOBBY_STARTING_CASH
        {"startingcash", startingCash}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyLimitSuperweapons(bool limit) {
    json payload = {
        {"field", 6}, // LOBBY_LIMIT_SUPERWEAPONS
        {"limit_superweapons", limit}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyMySide(int side, int startPos) {
    json payload = {
        {"field", 1}, // MY_SIDE
        {"side", side},
        {"start_pos", startPos}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyMyColor(int color) {
    json payload = {
        {"field", 2}, // MY_COLOR
        {"color", color}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyMyStartPos(int startPos) {
    json payload = {
        {"field", 3}, // MY_START_POS
        {"startpos", startPos}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyMyTeam(int team) {
    json payload = {
        {"field", 4}, // MY_TEAM
        {"team", team}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbySlotState(int slotIndex, int slotState) {
    json payload = {
        {"field", 12}, // HOST_ACTION_SET_SLOT_STATE
        {"slot_index", slotIndex},
        {"slot_state", slotState}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyHasMap(bool hasMap) {
    json payload = {
        {"field", 8}, // LOCAL_PLAYER_HAS_MAP
        {"has_map", hasMap}
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyForceStart() {
    json payload = {
        {"field", 7} // HOST_ACTION_FORCE_START
    };
    sendLobbyPostUpdate(m_authToken, m_currentLobbyId, payload);
}

void NGMP_OnlineServicesManager::updateLobbyLeave(int64_t lobbyId) {
    int64_t targetId = (lobbyId >= 0) ? lobbyId : m_currentLobbyId;
    if (targetId < 0 || m_authToken.empty()) return;

    std::thread([this, targetId]() {
        CURL* curl = curl_easy_init();
        if (!curl) return;

        std::string url = NGMP::GetAPIEndpoint(("Lobby/" + std::to_string(targetId)).c_str());
        NGMP::Internal::CurlResponse response;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
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

        if (res == CURLE_OK && (httpCode == 200 || httpCode == 204)) {
            fprintf(stderr, "[NGMP] Lobby deleted successfully\n");
            fflush(stderr);
        } else {
            fprintf(stderr, "[NGMP] Delete lobby request failed (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }

        m_currentLobbyId = -1;
        m_isLobbyOwner = false;
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
        fprintf(stderr, "[NGMP] sendChatMessage ignored: not logged in\n");
        fflush(stderr);
        return false;
    }
    if (m_chatSession && m_chatSession->isConnected()) {
        nlohmann::json payload = {
            {"msg_id", 1},
            {"action", false},
            {"message", message}
        };
        fprintf(stderr, "[NGMP] sendChatMessage: sending '%s'\n", message.c_str());
        fflush(stderr);
        return m_chatSession->sendPayload(payload.dump());
    }
    fprintf(stderr, "[NGMP] sendChatMessage called but no active chat session\n");
    fflush(stderr);
    return false;
}

bool NGMP_OnlineServicesManager::sendRawWebSocketPayload(const std::string& rawPayload) {
    if (!m_isLoggedIn) {
        fprintf(stderr, "[NGMP] sendRawWebSocketPayload ignored: not logged in\n");
        fflush(stderr);
        return false;
    }
    if (m_chatSession && m_chatSession->isConnected()) {
        fprintf(stderr, "[NGMP] sendRawWebSocketPayload: sending '%s'\n", rawPayload.c_str());
        fflush(stderr);
        return m_chatSession->sendPayload(rawPayload);
    }
    fprintf(stderr, "[NGMP] sendRawWebSocketPayload called but no active chat session\n");
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
