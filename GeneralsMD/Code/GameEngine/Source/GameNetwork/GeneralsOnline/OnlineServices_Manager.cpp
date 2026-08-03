// GeneralsX @feature GeneralsOnline NGMP Manager implementation
// Lifecycle management, event queue processing, and lobby REST requests.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include <cstdio>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
    struct CurlResponse {
        std::string text;
    };

    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        CurlResponse* resp = static_cast<CurlResponse*>(userp);
        resp->text.append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
}

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

    while (!pendingEvents.empty()) {
        NGMPEvent ev = pendingEvents.front();
        pendingEvents.pop();

        switch (ev.type) {
            case NGMPEvent::EVENT_AUTH_SUCCESS:
                fprintf(stderr, "[NGMP-MainThread] Event: Auth Success\n");
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
            default:
                break;
        }
        fflush(stderr);
    }
}

void NGMP_OnlineServicesManager::requestLobbyList() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return;
    }

    std::string url = NGMP::GetServerRESTEndpoint() + "/lobbies";
    CurlResponse response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && httpCode == 200) {
        try {
            auto jsonList = json::parse(response.text);
            m_lobbies.clear();
            if (jsonList.is_array()) {
                for (const auto& item : jsonList) {
                    NGMPLobby lobby;
                    lobby.id = item.value("id", "");
                    lobby.name = item.value("name", "Custom Lobby");
                    lobby.mapName = item.value("mapName", "Tournament Desert");
                    lobby.currentPlayers = item.value("currentPlayers", 1);
                    lobby.maxPlayers = item.value("maxPlayers", 8);
                    m_lobbies.push_back(lobby);
                }
            }

            NGMPEvent ev;
            ev.type = NGMPEvent::EVENT_LOBBY_LIST_UPDATED;
            postEvent(ev);
        } catch (const std::exception& e) {
            fprintf(stderr, "[NGMP] Lobby JSON parse exception: %s\n", e.what());
            fflush(stderr);
        }
    }
}

bool NGMP_OnlineServicesManager::sendChatMessage(const std::string& room, const std::string& message) {
    if (!m_isLoggedIn) {
        return false;
    }
    fprintf(stderr, "[NGMP] Sending chat message in room '%s': %s\n", room.c_str(), message.c_str());
    fflush(stderr);
    return true;
}
