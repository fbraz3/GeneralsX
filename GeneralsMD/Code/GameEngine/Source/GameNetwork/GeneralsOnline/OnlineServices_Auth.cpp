// GeneralsX @feature GeneralsOnline NGMP Auth implementation
// Browser-based gamecode login flow (macOS/Linux via SDL_OpenURL).
// Mirrors GeneralsOnline reference: references/GameClient/GeneralsMD/Code/GameEngine/Source/GameNetwork/GeneralsOnline/OnlineServices_Auth.cpp

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

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

    m_gamecode = NGMP::GenerateGamecode();
    std::string loginURL = NGMP::GetBrowserLoginURL(m_gamecode);

    fprintf(stderr, "[NGMP] beginBrowserLogin: gamecode=%s url=%s\n",
            m_gamecode.c_str(), loginURL.c_str());
    fflush(stderr);

    // Open the browser so the user can authenticate
    if (!SDL_OpenURL(loginURL.c_str())) {
        fprintf(stderr, "[NGMP] SDL_OpenURL failed: %s\n", SDL_GetError());
        fflush(stderr);
    }

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
                    int64_t     userId        = respJson.value("user_id",        int64_t(-1));

                    m_authToken  = sessionToken;
                    m_username   = displayName;
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
