// GeneralsX @feature GeneralsOnline NGMP Init implementation
// Handles initialization and teardown of NGMP Online Services backend components.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include <cstdio>

bool NGMP_OnlineServicesManager::init() {
    if (m_initialized) {
        return true;
    }

    fprintf(stderr, "[NGMP] Initializing NGMP Online Services (server: %s)\n",
            NGMP::GetServerRESTEndpoint().c_str());
    fflush(stderr);

    // Initialize sub-interfaces
    if (!m_pAuthInterface) m_pAuthInterface = new NGMP_OnlineServices_AuthInterface();
    if (!m_pLobbyInterface) m_pLobbyInterface = new NGMP_OnlineServices_LobbyInterface();
    if (!m_pRoomInterface) m_pRoomInterface = new NGMP_OnlineServices_RoomsInterface();
    if (!m_pStatsInterface) m_pStatsInterface = new NGMP_OnlineServices_StatsInterface();
    if (!m_pSocialInterface) m_pSocialInterface = new NGMP_OnlineServices_SocialInterface();
    if (!m_pWebSocketWrapper) m_pWebSocketWrapper = std::make_shared<WebSocket>();

    if (!TheNGMPGame) {
        TheNGMPGame = new NGMPGame();
    }

    // Initialize GameSpy stubs to prevent legacy UI crashes (e.g. WOLWelcomeMenu)
    SetUpGameSpy("", "");

    m_initialized = true;
    return true;
}

void NGMP_OnlineServicesManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    fprintf(stderr, "[NGMP] Shutting down NGMP Online Services\n");
    fflush(stderr);

    // Stop any in-flight browser login poll
    m_pollThreadRunning   = false;
    m_waitingBrowserLogin = false;
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }

    // Disconnect chat first
    if (m_chatSession) {
        m_chatSession->disconnect();
        m_chatSession.reset();
        NGMPEvent ev;
        ev.type = NGMPEvent::EVENT_CHAT_DISCONNECTED;
        postEvent(ev);
    }

    if (m_lobbyThread.joinable()) {
        m_lobbyThread.join();
    }

    logout();

    if (TheNGMPGame) {
        delete TheNGMPGame;
        TheNGMPGame = nullptr;
    }

    delete m_pAuthInterface;
    m_pAuthInterface = nullptr;

    delete m_pLobbyInterface;
    m_pLobbyInterface = nullptr;

    delete m_pRoomInterface;
    m_pRoomInterface = nullptr;

    delete m_pStatsInterface;
    m_pStatsInterface = nullptr;

    delete m_pSocialInterface;
    m_pSocialInterface = nullptr;

    m_pWebSocketWrapper.reset();

    m_initialized = false;
}
