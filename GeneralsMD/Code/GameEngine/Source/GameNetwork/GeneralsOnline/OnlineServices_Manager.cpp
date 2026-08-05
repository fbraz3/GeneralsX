// GeneralsX @feature GeneralsOnline NGMP Manager implementation
// Lifecycle management, event queue processing, and lobby REST requests.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "GameNetwork/GeneralsOnline/NGMPChatSession.h"
#include <cstdio>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

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

std::vector<NGMPEvent> NGMP_OnlineServicesManager::pollEvents() {
    std::queue<NGMPEvent> pendingEvents;
    {
        std::lock_guard<std::mutex> lock(m_eventMutex);
        std::swap(pendingEvents, m_eventQueue);
    }

    std::vector<NGMPEvent> events;
    while (!pendingEvents.empty()) {
        NGMPEvent ev = pendingEvents.front();
        pendingEvents.pop();

        switch (ev.type) {
            case NGMPEvent::EVENT_AUTH_SUCCESS:
                fprintf(stderr, "[NGMP-MainThread] Event: Auth Success\n");
                m_isLoggedIn = true;
                if (!m_chatSession) {
                    m_chatSession.reset(new NGMP::NGMPChatSession());
                }
                m_chatSession->connect(NGMP::GetServerWSEndpoint() + "/chat", m_authToken);
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
            default:
                break;
        }
        events.push_back(ev);
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

        std::string url = NGMP::GetServerRESTEndpoint() + "/lobbies";
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
                std::vector<NGMPLobby> lobbies;
                if (jsonList.is_array()) {
                    for (const auto& item : jsonList) {
                        NGMPLobby lobby;
                        lobby.id = item.value("id", "");
                        lobby.name = item.value("name", "Custom Lobby");
                        lobby.mapName = item.value("mapName", "Tournament Desert");
                        lobby.currentPlayers = item.value("currentPlayers", 1);
                        lobby.maxPlayers = item.value("maxPlayers", 8);
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

bool NGMP_OnlineServicesManager::sendChatMessage(const std::string& room, const std::string& message) {
    if (!m_isLoggedIn) {
        return false;
    }
    if (m_chatSession) {
        return m_chatSession->sendMessage(room, message);
    }
    fprintf(stderr, "[NGMP] sendChatMessage called but no active chat session\n");
    fflush(stderr);
    return false;
}
