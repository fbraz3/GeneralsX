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

bool NGMPWebSocket::sendPayload(const std::string& payload) {
    if (!m_running.load() || !m_curl) {
        return false;
    }

    size_t sent = 0;
    CURLcode res = curl_ws_send(m_curl, payload.c_str(), payload.size(), &sent, 0, CURLWS_TEXT);
    if (res != CURLE_OK) {
        fprintf(stderr, "[NGMP-WebSocket] Failed to send WS payload: %s\n", curl_easy_strerror(res));
        fflush(stderr);
        return false;
    }
    return true;
}

void NGMPWebSocket::receiveLoop() {
    char buffer[4096];
    const struct curl_ws_frame* meta = nullptr;

    auto lastPingTime = std::chrono::steady_clock::now();

    while (m_running.load()) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastPingTime).count() >= 10) {
            lastPingTime = now;
            std::string pingPayload = "{\"msg_id\":8}";
            size_t sent = 0;
            curl_ws_send(m_curl, pingPayload.c_str(), pingPayload.size(), &sent, 0, CURLWS_TEXT);
        }

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
            if (m_messageCallback) {
                m_messageCallback(std::string(buffer));
            }
        }
    }
}

} // namespace NGMP
