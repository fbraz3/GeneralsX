// GeneralsX @feature GeneralsOnline NGMP Chat WebSocket session header
// Manages a persistent WebSocket connection to the NGMP backend chat server.

#ifndef NGMP_CHAT_SESSION_H
#define NGMP_CHAT_SESSION_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <curl/curl.h>

namespace NGMP {

using ChatMessageCallback = std::function<void(const std::string& room, const std::string& sender, const std::string& message)>;

class NGMPChatSession {
public:
    NGMPChatSession() = default;
    ~NGMPChatSession();

    // Connect to the WebSocket endpoint (blocking until connected or failed)
    bool connect(const std::string& wsUrl, const std::string& authToken);

    // Disconnect from the WebSocket and stop the receiver thread
    void disconnect();

    // Returns true if currently connected
    bool isConnected() const { return m_running.load(); }

    // Send a chat message in the given room
    bool sendMessage(const std::string& room, const std::string& message);

    // Set the callback invoked on the receiver thread when a message arrives
    // NOTE: callback must post to the NGMP event queue, not touch UI directly
    void setMessageCallback(ChatMessageCallback cb) { m_messageCallback = std::move(cb); }

private:
    void receiveLoop();

    CURL* m_curl = nullptr;
    std::thread m_recvThread;
    std::atomic<bool> m_running = false;
    ChatMessageCallback m_messageCallback;
};

} // namespace NGMP

#endif // NGMP_CHAT_SESSION_H
