// GeneralsX @feature GeneralsOnline NGMP Chat WebSocket implementation
// Persistent WS connection using libcurl WebSocket (>= 7.86.0) for bidirectional chat.

#include "GameNetwork/GeneralsOnline/NGMPWebSocket.h"
#include <cstdio>
#include <cstring>
#include "GameNetwork/GeneralsOnline/NGMP_json.h"

using json = nlohmann::json;

namespace NGMP {

NGMPWebSocket::~NGMPWebSocket() {
    disconnect();
}

bool NGMPWebSocket::connect(const std::string& wsUrl, const std::string& authToken) {
    if (m_running.load()) {
        return true; // Already connected
    }

    m_curl = curl_easy_init();
    if (!m_curl) {
        fprintf(stderr, "[NGMP-Chat] Failed to initialize libcurl for WebSocket\n");
        fflush(stderr);
        return false;
    }

    struct curl_slist* headers = nullptr;
    if (!authToken.empty()) {
        std::string authHeader = "Authorization: Bearer " + authToken;
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(m_curl, CURLOPT_URL, wsUrl.c_str());
    curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 2L); // WebSocket mode
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
    if (headers) {
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(m_curl);
    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        fprintf(stderr, "[NGMP-Chat] WebSocket connect failed: %s\n", curl_easy_strerror(res));
        fflush(stderr);
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
        return false;
    }

    fprintf(stderr, "[NGMP-Chat] WebSocket connected to %s\n", wsUrl.c_str());
    fflush(stderr);

    m_running = true;
    m_recvThread = std::thread(&NGMPWebSocket::receiveLoop, this);
    return true;
}

void NGMPWebSocket::disconnect() {
    m_running = false;
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        if (m_curl) {
            size_t sent = 0;
            curl_ws_send(m_curl, "", 0, &sent, 0, CURLWS_CLOSE);
        }
    }
    if (m_recvThread.joinable()) {
        m_recvThread.join();
    }
    {
        std::lock_guard<std::mutex> lock(m_sendMutex);
        if (m_curl) {
            curl_easy_cleanup(m_curl);
            m_curl = nullptr;
        }
    }
    fprintf(stderr, "[NGMP-Chat] WebSocket disconnected\n");
    fflush(stderr);
}

bool NGMPWebSocket::sendPayload(const std::string& payload) {
    if (!m_running.load()) {
        fprintf(stderr, "[NGMP-WebSocket] Cannot send payload, WS not running (running=%d)\n", m_running.load());
        fflush(stderr);
        return false;
    }

    std::lock_guard<std::mutex> lock(m_sendMutex);
    if (!m_curl) {
        fprintf(stderr, "[NGMP-WebSocket] Cannot send payload, null curl handle\n");
        fflush(stderr);
        return false;
    }

    size_t sent = 0;
    CURLcode res = curl_ws_send(m_curl, payload.c_str(), payload.size(), &sent, 0, CURLWS_TEXT);
    if (res != CURLE_OK) {
        fprintf(stderr, "[NGMP-WebSocket] Failed to send WS payload (%s): %s\n", payload.c_str(), curl_easy_strerror(res));
        fflush(stderr);
        return false;
    }
    fprintf(stderr, "[NGMP-WebSocket] Sent WS payload: %s\n", payload.c_str());
    fflush(stderr);
    return true;
}

// GeneralsX @bugfix fbraz3 15/08/2026 Accumulate fragmented WebSocket frames matching references/GameClient
void NGMPWebSocket::receiveLoop() {
    char buffer[8192 * 4];
    const struct curl_ws_frame* meta = nullptr;
    std::vector<char> partialBuffer;

    auto lastPingTime = std::chrono::steady_clock::now();

    while (m_running.load()) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastPingTime).count() >= 10) {
            lastPingTime = now;
            std::string pingPayload = "{\"msg_id\":14}"; // EWebSocketMessageID::PING = 14
            size_t sent = 0;
            std::lock_guard<std::mutex> lock(m_sendMutex);
            curl_ws_send(m_curl, pingPayload.c_str(), pingPayload.size(), &sent, 0, CURLWS_TEXT);
        }

        size_t received = 0;
        CURLcode res;
        
        {
            std::lock_guard<std::mutex> lock(m_sendMutex);
            if (!m_curl) break; // In case we disconnected
            res = curl_ws_recv(m_curl, buffer, sizeof(buffer) - 1, &received, &meta);
        }

        if (res == CURLE_AGAIN) {
            // No data ready — yield briefly to avoid busy-spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (res != CURLE_OK) {
            fprintf(stderr, "[NGMP-Chat] WS recv error: %s\n", curl_easy_strerror(res));
            fflush(stderr);
            m_running = false;
            break;
        }

        if (received > 0) {
            if (meta != nullptr) {
                if (meta->flags & CURLWS_PONG) {
                    // PONG frame received, connection alive
                    continue;
                }

                static constexpr size_t MAX_WS_PARTIAL_SIZE = 2 * 1024 * 1024; // 2 MB
                if (partialBuffer.size() + received > MAX_WS_PARTIAL_SIZE) {
                    fprintf(stderr, "[NGMP-Chat] Partial buffer overflow, discarding message\n");
                    fflush(stderr);
                    partialBuffer.clear();
                    continue;
                }

                size_t oldSize = partialBuffer.size();
                partialBuffer.resize(oldSize + received);
                std::memcpy(partialBuffer.data() + oldSize, buffer, received);

                bool bMessageComplete = !(meta->flags & CURLWS_CONT) && (meta->bytesleft == 0);
                if (bMessageComplete) {
                    std::string completeMsg(partialBuffer.data(), partialBuffer.size());
                    partialBuffer.clear();
                    if (m_messageCallback) {
                        m_messageCallback(completeMsg);
                    }
                }
            } else {
                if (m_messageCallback) {
                    m_messageCallback(std::string(buffer, received));
                }
            }
        }
    }
}

} // namespace NGMP

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"

void WebSocket::SendData_ChangeName(UnicodeString& strNewName)
{
    std::string utf8Name = NGMP::UnicodeToUTF8(strNewName);
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::PLAYER_NAME_CHANGE;
    payload["display_name"] = utf8Name;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_RoomChatMessage(UnicodeString& msg, bool bIsAction)
{
    std::string utf8Msg = NGMP::UnicodeToUTF8(msg);
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::NETWORK_ROOM_CHAT_FROM_CLIENT;
    payload["message"] = utf8Msg;
    payload["action"] = bIsAction;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_FriendMessage(UnicodeString& msg, int64_t target_user_id)
{
    std::string utf8Msg = NGMP::UnicodeToUTF8(msg);
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::SOCIAL_FRIEND_CHAT_MESSAGE_CLIENT_TO_SERVER;
    payload["target_user_id"] = target_user_id;
    payload["message"] = utf8Msg;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_LobbyChatMessage(UnicodeString& msg, bool bIsAction, bool bIsAnnouncement, bool bShowAnnouncementToHost)
{
    std::string utf8Msg = NGMP::UnicodeToUTF8(msg);
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::LOBBY_ROOM_CHAT_FROM_CLIENT;
    payload["message"] = utf8Msg;
    payload["action"] = bIsAction;
    payload["announcement"] = bIsAnnouncement;
    payload["show_announcement_to_host"] = bShowAnnouncementToHost;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_JoinNetworkRoom(int roomID)
{
    NGMP_OnlineServicesManager::getInstance().changeNetworkRoom((int16_t)roomID);
}

void WebSocket::SendData_LeaveNetworkRoom()
{
    NGMP_OnlineServicesManager::getInstance().changeNetworkRoom(-1);
}

void WebSocket::SendData_MarkReady(bool bReady)
{
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::NETWORK_ROOM_MARK_READY;
    payload["ready"] = bReady;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_StartGame()
{
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::START_GAME;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_CountdownStarted()
{
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::START_GAME_COUNTDOWN_STARTED;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}

void WebSocket::SendData_RequestSignalling(int64_t target_user_id)
{
    json payload;
    payload["msg_id"] = (int)NGMP::EWebSocketMessageID::NETWORK_CONNECTION_CLIENT_REQUEST_SIGNALLING;
    payload["target_user_id"] = target_user_id;
    NGMP_OnlineServicesManager::getInstance().sendRawWebSocketPayload(payload.dump(-1, ' ', false, json::error_handler_t::replace));
}


