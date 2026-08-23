// GeneralsX @feature GeneralsOnline NGMP Init implementation
// Handles initialization and teardown of NGMP Online Services backend components.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_interfaces.h"
#include "GameNetwork/GeneralsOnline/NGMPGame.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/RankPointValue.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include <cstdio>

// GeneralsX @bugfix fbraz3 23/08/2026 Prevent legacy UI callbacks from crashing when trying to send GameSpy requests by providing dummy message queues
// Dummy implementations to prevent legacy UI callbacks from crashing when trying to send GameSpy requests
class DummyGameSpyBuddyMessageQueue : public GameSpyBuddyMessageQueueInterface {
public:
    void startThread() override {}
    void endThread() override {}
    Bool isThreadRunning() override { return FALSE; }
    Bool isConnected() override { return FALSE; }
    Bool isConnecting() override { return FALSE; }
    void addRequest(const BuddyRequest&) override {}
    Bool getRequest(BuddyRequest&) override { return FALSE; }
    void addResponse(const BuddyResponse&) override {}
    Bool getResponse(BuddyResponse&) override { return FALSE; }
    GPProfile getLocalProfileID() override { return 0; }
};

class DummyGameSpyPeerMessageQueue : public GameSpyPeerMessageQueueInterface {
public:
    void startThread() override {}
    void endThread() override {}
    Bool isThreadRunning() override { return FALSE; }
    Bool isConnected() override { return FALSE; }
    Bool isConnecting() override { return FALSE; }
    void addRequest(const PeerRequest&) override {}
    Bool getRequest(PeerRequest&) override { return FALSE; }
    void addResponse(const PeerResponse&) override {}
    Bool getResponse(PeerResponse&) override { return FALSE; }
    SerialAuthResult getSerialAuthResult() override { return SERIAL_OK; }
};

class DummyGameSpyPSMessageQueue : public GameSpyPSMessageQueueInterface {
public:
    void startThread() override {}
    void endThread() override {}
    Bool isThreadRunning() override { return FALSE; }
    void addRequest(const PSRequest&) override {}
    Bool getRequest(PSRequest&) override { return FALSE; }
    void addResponse(const PSResponse&) override {}
    Bool getResponse(PSResponse&) override { return FALSE; }
    PSPlayerStats findPlayerStatsByID(Int profileID) override {
        PSPlayerStats s;
        s.id = profileID;
        return s;
    }
    void trackPlayerStats(PSPlayerStats) override {}
};

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

    // Initialize GameSpy and UI singletons to prevent legacy UI crashes (e.g. WOLWelcomeMenu, PopupPlayerInfo, WOLLobbyMenu)
    if (!TheGameSpyConfig) {
        TheGameSpyConfig = GameSpyConfigInterface::create("");
    }
    if (!TheGameSpyInfo) {
        TheGameSpyInfo = GameSpyInfoInterface::createNewGameSpyInfoInterface();
    }
    if (!TheRankPointValues) {
        TheRankPointValues = new RankPoints();
    }
    if (!TheLadderList) {
        TheLadderList = new LadderList();
    }
    if (!TheGameSpyBuddyMessageQueue) {
        TheGameSpyBuddyMessageQueue = new DummyGameSpyBuddyMessageQueue();
    }
    if (!TheGameSpyPeerMessageQueue) {
        TheGameSpyPeerMessageQueue = new DummyGameSpyPeerMessageQueue();
    }
    if (!TheGameSpyPSMessageQueue) {
        TheGameSpyPSMessageQueue = new DummyGameSpyPSMessageQueue();
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

    if (TheRankPointValues) {
        delete TheRankPointValues;
        TheRankPointValues = nullptr;
    }

    if (TheLadderList) {
        delete TheLadderList;
        TheLadderList = nullptr;
    }

    if (TheGameSpyConfig) {
        delete TheGameSpyConfig;
        TheGameSpyConfig = nullptr;
    }

    if (TheGameSpyInfo) {
        delete TheGameSpyInfo;
        TheGameSpyInfo = nullptr;
    }

    m_initialized = false;
}
