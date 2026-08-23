// GeneralsX @feature GeneralsOnline NGMP Auth implementation
// Browser-based gamecode login flow (macOS/Linux via SDL_OpenURL).
// Mirrors GeneralsOnline reference: references/GameClient/GeneralsMD/Code/GameEngine/Source/GameNetwork/GeneralsOnline/OnlineServices_Auth.cpp

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/OnlineServices_Auth.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include <cstdio>
#include <thread>
#include <chrono>
#include "GameNetwork/GeneralsOnline/NGMP_json.h"

using json = nlohmann::json;

// Server-side result enum matching GenOnlineService EPendingLoginState
enum class ELoginPollResult : int {
    CODE_INVALID  = -1,
    WAITING       =  0,
    LOGIN_SUCCESS =  1,
    LOGIN_FAILED  =  2
};

// ──────────────────────────────────────────────────────────────────────────────
// beginBrowserLogin
// Generates a gamecode, opens the web portal in the system browser (SDL_OpenURL),
// shows a "please continue in your browser" message, and starts polling.
// ──────────────────────────────────────────────────────────────────────────────
void NGMP_OnlineServicesManager::beginBrowserLogin() {
    if (m_waitingBrowserLogin.exchange(true)) {
        fprintf(stderr, "[NGMP] Browser login already in progress\n");
        fflush(stderr);
        return;
    }

    //commented by debug purposes
    //m_gamecode = NGMP::GenerateGamecode();
    m_gamecode = "ILOVECODE";
    std::string loginURL = NGMP::GetBrowserLoginURL(m_gamecode);

    fprintf(stderr, "[NGMP] beginBrowserLogin: gamecode=%s url=%s\n",
            m_gamecode.c_str(), loginURL.c_str());
    fflush(stderr);

    // Open the browser so the user can authenticate
    // if (!SDL_OpenURL(loginURL.c_str())) {
    //     fprintf(stderr, "[NGMP] SDL_OpenURL failed: %s\n", SDL_GetError());
    //     fflush(stderr);
    // }

    // Start background polling thread
    m_pollThreadRunning = true;
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }

    m_pollThread = std::thread([this]() {
        const int64_t pollIntervalMs = 1000;

        fprintf(stderr, "[NGMP] Poll thread started for gamecode=%s\n", m_gamecode.c_str());
        fflush(stderr);

        while (m_pollThreadRunning && m_waitingBrowserLogin) {
            std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));

            if (!m_pollThreadRunning || !m_waitingBrowserLogin) {
                break;
            }

            // POST /env/dev/contract/1/CheckLogin
            std::string url = NGMP::GetAPIEndpoint("CheckLogin");

            json requestJson = {
                { "code",        m_gamecode },
                { "client_id",   NGMP_CLIENT_ID },
                { "reserved_0",  "" },
                { "reserved_1",  "" },
                { "reserved_2",  "" }
            };
            std::string requestBody = requestJson.dump();

            CURL* curl = curl_easy_init();
            if (!curl) {
                fprintf(stderr, "[NGMP] curl_easy_init failed in poll thread\n");
                fflush(stderr);
                continue;
            }

            NGMP::Internal::CurlResponse response;
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

            CURLcode res = curl_easy_perform(curl);
            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK) {
                fprintf(stderr, "[NGMP] Poll HTTP error: %s\n", curl_easy_strerror(res));
                fflush(stderr);
                continue;
            }

            fprintf(stderr, "[NGMP] CheckLogin response (%ld): %s\n", httpCode, response.text.c_str());
            fflush(stderr);

            try {
                auto respJson = json::parse(response.text);
                int resultCode = respJson.value("result", -1);
                ELoginPollResult pollResult = static_cast<ELoginPollResult>(resultCode);

                if (pollResult == ELoginPollResult::WAITING) {
                    fprintf(stderr, "[NGMP] Waiting for user to authenticate in browser...\n");
                    fflush(stderr);
                    continue;
                }
                else if (pollResult == ELoginPollResult::CODE_INVALID) {
                    fprintf(stderr, "[NGMP] Gamecode not recognized by server yet, retrying...\n");
                    fflush(stderr);
                    continue;
                }
                else if (pollResult == ELoginPollResult::LOGIN_SUCCESS) {
                    std::string sessionToken  = respJson.value("session_token",  "");
                    std::string refreshToken  = respJson.value("refresh_token",  "");
                    std::string displayName   = respJson.value("display_name",   "");
                    std::string wsUri         = respJson.value("ws_uri",         "");
                    int64_t     userId        = respJson.value("user_id",        int64_t(-1));

                    m_authToken  = sessionToken;
                    m_username   = displayName;
                    m_userId     = userId;
                    m_wsUri      = wsUri;
                    m_isLoggedIn = true;

                    NGMP::SaveAuthToken(sessionToken);
                    NGMP::SaveRefreshToken(refreshToken);

                    m_waitingBrowserLogin = false;
                    m_pollThreadRunning   = false;

                    fprintf(stderr, "[NGMP] Login successful! user=%s id=%lld\n",
                            displayName.c_str(), (long long)userId);
                    fflush(stderr);

                    NGMPEvent ev;
                    ev.type    = NGMPEvent::EVENT_AUTH_SUCCESS;
                    ev.payload = sessionToken;
                    postEvent(ev);
                    return;
                }
                else if (pollResult == ELoginPollResult::LOGIN_FAILED) {
                    fprintf(stderr, "[NGMP] Server reported login failure\n");
                    fflush(stderr);

                    m_waitingBrowserLogin = false;
                    m_pollThreadRunning   = false;

                    NGMPEvent ev;
                    ev.type    = NGMPEvent::EVENT_AUTH_FAILURE;
                    ev.payload = "Login failed";
                    postEvent(ev);
                    return;
                }
            }
            catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] CheckLogin JSON parse error: %s\n", e.what());
                fflush(stderr);
            }
        }

        fprintf(stderr, "[NGMP] Poll thread exiting\n");
        fflush(stderr);
    });
}

// ──────────────────────────────────────────────────────────────────────────────
// cancelBrowserLogin — called when the user clicks Cancel in the in-game dialog
// ──────────────────────────────────────────────────────────────────────────────
void NGMP_OnlineServicesManager::cancelBrowserLogin() {
    if (!m_waitingBrowserLogin) return;

    fprintf(stderr, "[NGMP] Browser login cancelled by user\n");
    fflush(stderr);

    m_pollThreadRunning   = false;
    m_waitingBrowserLogin = false;

    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }

    NGMPEvent ev;
    ev.type = NGMPEvent::EVENT_AUTH_CANCELLED;
    postEvent(ev);
}

// ──────────────────────────────────────────────────────────────────────────────
// loginWithRefreshToken — silent token re-login (no browser required)
// ──────────────────────────────────────────────────────────────────────────────
void NGMP_OnlineServicesManager::loginWithRefreshToken(const std::string& refreshToken) {
    if (refreshToken.empty()) return;

    // TODO: implement POST /LoginWithToken with the refresh token for silent re-auth.
    // For now, treat a non-empty saved refresh token as requiring a fresh browser login.
    fprintf(stderr, "[NGMP] Saved refresh token found but silent re-login not yet implemented; starting browser flow\n");
    fflush(stderr);
    beginBrowserLogin();
}

void NGMP_OnlineServicesManager::logout() {
    m_authToken.clear();
    m_username.clear();
    m_isLoggedIn = false;
    NGMP::SaveAuthToken("");
}

void NGMP_OnlineServicesManager::requestGlobalStatsAsync() {
    if (m_statsRequestInFlight.exchange(true)) {
        return;
    }

    if (m_statsThread.joinable()) {
        m_statsThread.join();
    }

    m_statsThread = std::thread([this]() {
        std::string url = NGMP::GetAPIEndpoint("GlobalStats");

        CURL* curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "[NGMP] curl_easy_init failed for GlobalStats\n");
            fflush(stderr);
            m_statsRequestInFlight = false;
            return;
        }

        NGMP::Internal::CurlResponse response;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!m_authToken.empty()) {
            std::string authHeader = "Authorization: Bearer " + m_authToken;
            headers = curl_slist_append(headers, authHeader.c_str());
        }

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
                auto jsonResponse = json::parse(response.text);
                auto globalStatsJson = jsonResponse.value("globalstats", json::object());

                std::vector<int> wins;
                if (globalStatsJson.contains("wins") && globalStatsJson["wins"].is_array()) {
                    wins = globalStatsJson["wins"].get<std::vector<int>>();
                }
                
                std::vector<int> matches;
                if (globalStatsJson.contains("matches") && globalStatsJson["matches"].is_array()) {
                    matches = globalStatsJson["matches"].get<std::vector<int>>();
                }

                {
                    std::lock_guard<std::mutex> lock(m_statsMutex);
                    m_globalStats.wins = wins;
                    m_globalStats.matches = matches;
                    m_hasGlobalStats = true;
                }

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_GLOBAL_STATS_RECEIVED;
                postEvent(ev);
                
                fprintf(stderr, "[NGMP] GlobalStats fetched successfully (wins:%zu matches:%zu)\n", wins.size(), matches.size());
                fflush(stderr);
            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] GlobalStats JSON parse error: %s\n", e.what());
                fflush(stderr);
            }
        } else {
            fprintf(stderr, "[NGMP] GlobalStats request failed (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }

        m_statsRequestInFlight = false;
    });
}

bool NGMP_OnlineServicesManager::getCachedPlayerStats(int64_t userID, PSPlayerStats& outStats) const {
    std::lock_guard<std::mutex> lock(m_playerStatsMutex);
    auto it = m_cachedPlayerStats.find(userID);
    if (it != m_cachedPlayerStats.end()) {
        outStats = it->second;
        return true;
    }
    return false;
}

void NGMP_OnlineServicesManager::requestPlayerStatsAsync(int64_t userID) {
    std::thread([this, userID]() {
        std::string url = NGMP::GetAPIEndpoint("PlayerStats") + "/" + std::to_string(userID);

        CURL* curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "[NGMP] curl_easy_init failed for PlayerStats\n");
            fflush(stderr);
            return;
        }

        NGMP::Internal::CurlResponse response;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!m_authToken.empty()) {
            std::string authHeader = "Authorization: Bearer " + m_authToken;
            headers = curl_slist_append(headers, authHeader.c_str());
        }

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
                auto jsonObject = json::parse(response.text);
                auto jsonObjectRoot = jsonObject["stats"];

                PSPlayerStats stats;
                if (jsonObjectRoot.contains("userID") && !jsonObjectRoot["userID"].is_null()) {
                    jsonObjectRoot["userID"].get_to(stats.id);
                }

                #define PROCESS_JSON_PER_GENERAL_RESULT(name) \
                    if (jsonObjectRoot.contains(#name) && jsonObjectRoot[#name].is_array()) { \
                        int i = 0; \
                        for (const auto& iter : jsonObjectRoot[#name]) { \
                            iter.get_to(stats.name[i++]); \
                        } \
                    }

                PROCESS_JSON_PER_GENERAL_RESULT(wins);
                PROCESS_JSON_PER_GENERAL_RESULT(losses);
                PROCESS_JSON_PER_GENERAL_RESULT(games);
                PROCESS_JSON_PER_GENERAL_RESULT(duration);
                PROCESS_JSON_PER_GENERAL_RESULT(unitsKilled);
                PROCESS_JSON_PER_GENERAL_RESULT(unitsLost);
                PROCESS_JSON_PER_GENERAL_RESULT(unitsBuilt);
                PROCESS_JSON_PER_GENERAL_RESULT(buildingsKilled);
                PROCESS_JSON_PER_GENERAL_RESULT(buildingsLost);
                PROCESS_JSON_PER_GENERAL_RESULT(buildingsBuilt);
                PROCESS_JSON_PER_GENERAL_RESULT(earnings);
                PROCESS_JSON_PER_GENERAL_RESULT(techCaptured);
                PROCESS_JSON_PER_GENERAL_RESULT(discons);
                PROCESS_JSON_PER_GENERAL_RESULT(desyncs);
                PROCESS_JSON_PER_GENERAL_RESULT(surrenders);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf2p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf3p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf4p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf5p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf6p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf7p);
                PROCESS_JSON_PER_GENERAL_RESULT(gamesOf8p);
                PROCESS_JSON_PER_GENERAL_RESULT(customGames);
                PROCESS_JSON_PER_GENERAL_RESULT(QMGames);

                #define PROCESS_JSON_STANDARD_RESULT(name) \
                    if (jsonObjectRoot.contains(#name) && !jsonObjectRoot[#name].is_null()) { \
                        jsonObjectRoot[#name].get_to(stats.name); \
                    }

                PROCESS_JSON_STANDARD_RESULT(locale);
                PROCESS_JSON_STANDARD_RESULT(gamesAsRandom);
                PROCESS_JSON_STANDARD_RESULT(options);
                PROCESS_JSON_STANDARD_RESULT(systemSpec);
                PROCESS_JSON_STANDARD_RESULT(lastFPS);
                PROCESS_JSON_STANDARD_RESULT(lastGeneral);
                PROCESS_JSON_STANDARD_RESULT(gamesInRowWithLastGeneral);
                PROCESS_JSON_STANDARD_RESULT(challengeMedals);
                PROCESS_JSON_STANDARD_RESULT(battleHonors);
                PROCESS_JSON_STANDARD_RESULT(QMwinsInARow);
                PROCESS_JSON_STANDARD_RESULT(maxQMwinsInARow);
                PROCESS_JSON_STANDARD_RESULT(winsInARow);
                PROCESS_JSON_STANDARD_RESULT(maxWinsInARow);
                PROCESS_JSON_STANDARD_RESULT(lossesInARow);
                PROCESS_JSON_STANDARD_RESULT(maxLossesInARow);
                PROCESS_JSON_STANDARD_RESULT(disconsInARow);
                PROCESS_JSON_STANDARD_RESULT(maxDisconsInARow);
                PROCESS_JSON_STANDARD_RESULT(desyncsInARow);
                PROCESS_JSON_STANDARD_RESULT(maxDesyncsInARow);
                PROCESS_JSON_STANDARD_RESULT(builtParticleCannon);
                PROCESS_JSON_STANDARD_RESULT(builtNuke);
                PROCESS_JSON_STANDARD_RESULT(builtSCUD);
                PROCESS_JSON_STANDARD_RESULT(lastLadderPort);
                PROCESS_JSON_STANDARD_RESULT(lastLadderHost);

                #undef PROCESS_JSON_PER_GENERAL_RESULT
                #undef PROCESS_JSON_STANDARD_RESULT

                {
                    std::lock_guard<std::mutex> lock(m_playerStatsMutex);
                    m_cachedPlayerStats[userID] = stats;
                }

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_PLAYER_STATS_RECEIVED;
                postEvent(ev);
                
                fprintf(stderr, "[NGMP] PlayerStats fetched successfully for userID=%lld\n", (long long)userID);
                fflush(stderr);
            } catch (const std::exception& e) {
                fprintf(stderr, "[NGMP] PlayerStats JSON parse error: %s\n", e.what());
                fflush(stderr);
            }
        } else {
            fprintf(stderr, "[NGMP] PlayerStats request failed (curl=%d, http=%ld)\n", res, httpCode);
            fflush(stderr);
        }
    }).detach();
}

NGMP_OnlineServices_AuthInterface::NGMP_OnlineServices_AuthInterface()
{
}

std::string NGMP_OnlineServices_AuthInterface::GetDisplayName()
{
    return NGMP_OnlineServicesManager::getInstance().getUsername();
}

std::wstring NGMP_OnlineServices_AuthInterface::GetDisplayNameW()
{
    std::string name = GetDisplayName();
    return std::wstring(name.begin(), name.end());
}

int64_t NGMP_OnlineServices_AuthInterface::GetUserID() const
{
    return NGMP_OnlineServicesManager::getInstance().getUserId();
}

std::string NGMP_OnlineServices_AuthInterface::GetAuthToken() const
{
    return NGMP_OnlineServicesManager::getInstance().getAuthToken();
}

bool NGMP_OnlineServices_AuthInterface::IsLoggedIn() const
{
    return NGMP_OnlineServicesManager::getInstance().isLoggedIn();
}

void NGMP_OnlineServices_AuthInterface::BeginLogin()
{
    NGMP_OnlineServicesManager::getInstance().beginBrowserLogin();
}

void NGMP_OnlineServices_AuthInterface::LogoutOfMyAccount()
{
    NGMP_OnlineServicesManager::getInstance().logout();
}


