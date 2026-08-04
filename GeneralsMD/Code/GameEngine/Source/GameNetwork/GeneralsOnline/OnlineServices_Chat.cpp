// GeneralsX @feature GeneralsOnline NGMP Chat WebSocket implementation
// Persistent WS connection using libcurl WebSocket (>= 7.86.0) for bidirectional chat.

#include "GameNetwork/GeneralsOnline/NGMPChatSession.h"
#include <cstdio>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace NGMP {

NGMPChatSession::~NGMPChatSession() {
    disconnect();
}

bool NGMPChatSession::connect(const std::string& wsUrl, const std::string& authToken) {
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
    m_recvThread = std::thread(&NGMPChatSession::receiveLoop, this);
    return true;
}

void NGMPChatSession::disconnect() {
    m_running = false;
    if (m_recvThread.joinable()) {
        m_recvThread.join();
    }
    if (m_curl) {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
    }
    fprintf(stderr, "[NGMP-Chat] WebSocket disconnected\n");
    fflush(stderr);
}

bool NGMPChatSession::sendMessage(const std::string& room, const std::string& message) {
    if (!m_running.load() || !m_curl) {
        return false;
    }

    json payload = {
        {"type", "chat"},
        {"room", room},
        {"message", message}
    };
    std::string frame = payload.dump();

    size_t sent = 0;
    CURLcode res = curl_ws_send(m_curl, frame.c_str(), frame.size(), &sent, 0, CURLWS_TEXT);
    if (res != CURLE_OK) {
        fprintf(stderr, "[NGMP-Chat] Failed to send WS message: %s\n", curl_easy_strerror(res));
        fflush(stderr);
        return false;
    }
    return true;
}

void NGMPChatSession::receiveLoop() {
    char buffer[4096];
    const struct curl_ws_frame* meta = nullptr;

    while (m_running.load()) {
        size_t received = 0;
        CURLcode res = curl_ws_recv(m_curl, buffer, sizeof(buffer) - 1, &received, &meta);

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
            buffer[received] = '\0';
            try {
                auto msg = json::parse(buffer);
                std::string type = msg.value("type", "");
                if (type == "chat" && m_messageCallback) {
                    std::string room    = msg.value("room", "");
                    std::string sender  = msg.value("sender", "");
                    std::string content = msg.value("message", "");
                    m_messageCallback(room, sender, content);
                }
            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP-Chat] JSON parse error in WS frame: %s\n", e.what());
                fflush(stderr);
            }
        }
    }
}

} // namespace NGMP
