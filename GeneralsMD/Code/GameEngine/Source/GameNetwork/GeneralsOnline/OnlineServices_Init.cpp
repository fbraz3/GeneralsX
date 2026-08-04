// GeneralsX @feature GeneralsOnline NGMP Init implementation
// Handles initialization and teardown of NGMP Online Services backend components.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include <cstdio>

bool NGMP_OnlineServicesManager::init() {
    if (m_initialized) {
        return true;
    }

    fprintf(stderr, "[NGMP] Initializing NGMP Online Services (Endpoint: %s)\n", NGMP::GetServerWSEndpoint().c_str());
    fflush(stderr);

    // Auto load token if stored locally
    std::string savedToken = NGMP::LoadAuthToken();
    if (!savedToken.empty()) {
        if (loginWithToken(savedToken)) {
            // Connect chat WebSocket using the restored session token
            m_chatSession = std::make_unique<NGMP::NGMPChatSession>();
            m_chatSession->setMessageCallback([this](const std::string& room, const std::string& sender, const std::string& msg) {
                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_CHAT_MESSAGE_RECEIVED;
                ev.payload = "[" + room + "] " + sender + ": " + msg;
                postEvent(ev);
            });
            bool connected = m_chatSession->connect(NGMP::GetServerWSEndpoint(), savedToken);
            if (connected) {
                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_CHAT_CONNECTED;
                postEvent(ev);
            }
        }
    }

    m_initialized = true;
    return true;
}

void NGMP_OnlineServicesManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    fprintf(stderr, "[NGMP] Shutting down NGMP Online Services\n");
    fflush(stderr);

    // Disconnect chat first
    if (m_chatSession) {
        m_chatSession->disconnect();
        m_chatSession.reset();
        NGMPEvent ev;
        ev.type = NGMPEvent::EVENT_CHAT_DISCONNECTED;
        postEvent(ev);
    }

    // Join any in-flight threads before state is destroyed
    if (m_loginThread.joinable()) {
        m_loginThread.join();
    }
    if (m_lobbyThread.joinable()) {
        m_lobbyThread.join();
    }

    logout();
    m_initialized = false;
}
