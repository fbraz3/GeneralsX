// GeneralsX @feature GeneralsOnline NGMP Auth implementation
// Handles user authentication and JWT session token persistence.

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

bool NGMP_OnlineServicesManager::login(const std::string& username, const std::string& password) {
    fprintf(stderr, "[NGMP] Attempting login for user: %s\n", username.c_str());
    fflush(stderr);

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[NGMP] Failed to initialize libcurl for auth\n");
        fflush(stderr);
        return false;
    }

    std::string url = NGMP::GetServerRESTEndpoint() + "/auth/login";
    json requestJson = {
        {"username", username},
        {"password", password}
    };
    std::string requestBody = requestJson.dump();

    CurlResponse response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && (httpCode == 200 || httpCode == 201)) {
        try {
            auto responseJson = json::parse(response.text);
            if (responseJson.contains("token")) {
                m_authToken = responseJson["token"].get<std::string>();
                m_username = username;
                m_isLoggedIn = true;

                NGMP::SaveAuthToken(m_authToken);

                NGMPEvent ev;
                ev.type = NGMPEvent::EVENT_AUTH_SUCCESS;
                ev.payload = m_authToken;
                postEvent(ev);

                fprintf(stderr, "[NGMP] Login successful for user: %s\n", username.c_str());
                fflush(stderr);
                return true;
            }
        } catch (const std::exception& e) {
            fprintf(stderr, "[NGMP] JSON parse exception during auth: %s\n", e.what());
            fflush(stderr);
        }
    }

    // Auth failed
    NGMPEvent ev;
    ev.type = NGMPEvent::EVENT_AUTH_FAILURE;
    ev.payload = "Invalid credentials or server unavailable";
    postEvent(ev);

    fprintf(stderr, "[NGMP] Login failed for user %s (HTTP %ld)\n", username.c_str(), httpCode);
    fflush(stderr);
    return false;
}

bool NGMP_OnlineServicesManager::loginWithToken(const std::string& token) {
    if (token.empty()) {
        return false;
    }
    m_authToken = token;
    m_isLoggedIn = true;

    NGMPEvent ev;
    ev.type = NGMPEvent::EVENT_AUTH_SUCCESS;
    ev.payload = token;
    postEvent(ev);

    fprintf(stderr, "[NGMP] Authenticated via saved token\n");
    fflush(stderr);
    return true;
}

void NGMP_OnlineServicesManager::logout() {
    m_authToken.clear();
    m_username.clear();
    m_isLoggedIn = false;
    NGMP::SaveAuthToken("");
}
