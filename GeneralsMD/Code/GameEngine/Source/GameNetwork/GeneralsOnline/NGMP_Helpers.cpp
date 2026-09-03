// GeneralsX @feature GeneralsOnline NGMP Helpers implementation
// Cross-platform OS abstraction using pure C++20 standard library.

#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GeneralsOnline/ngmp_curl_utils.h"
#include "Common/UnicodeString.h"
#include "WWLib/utf8.h"
#include <chrono>
#include <cstdio>

#include <thread>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <format>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <SDL3/SDL.h>
#endif

#ifndef NGMP_DEFAULT_HOST
#define NGMP_DEFAULT_HOST "localhost"
#endif

#ifndef NGMP_DEFAULT_PORT
#define NGMP_DEFAULT_PORT "9001"
#endif

#ifndef NGMP_SERVER_ENV
#define NGMP_SERVER_ENV "dev"
#endif

#ifndef NGMP_CONTRACT_VERSION
#define NGMP_CONTRACT_VERSION "1"
#endif

namespace NGMP {

bool IsSSLEnabled() {
#if defined(NGMP_USE_SSL) && NGMP_USE_SSL
    return true;
#else
    return false;
#endif
}

uint32_t GetTicks() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()
    );
}

void Delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

std::string GetStoragePath() {
    std::string baseDir;
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE");
    }
    if (home) {
        baseDir = std::string(home) + "/.generals_online/";
    } else {
        baseDir = "./.generals_online/";
    }
    return baseDir;
}

bool SaveAuthToken(const std::string& token) {
    std::string path = GetStoragePath();
    std::filesystem::create_directories(path);
    std::string tokenFile = path + "session.token";
    std::ofstream out(tokenFile, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out << token;
    out.close();
    return true;
}

std::string LoadAuthToken() {
    std::string path = GetStoragePath();
    std::string tokenFile = path + "session.token";
    std::ifstream in(tokenFile);
    if (!in.is_open()) {
        return "";
    }
    std::string token;
    in >> token;
    return token;
}

bool SaveRefreshToken(const std::string& token) {
    std::string path = GetStoragePath();
    std::filesystem::create_directories(path);
    std::string tokenFile = path + "refresh.token";
    std::ofstream out(tokenFile, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    out << token;
    out.close();
    return true;
}

std::string LoadRefreshToken() {
    std::string path = GetStoragePath();
    std::string tokenFile = path + "refresh.token";
    std::ifstream in(tokenFile);
    if (!in.is_open()) {
        return "";
    }
    std::string token;
    in >> token;
    return token;
}

bool IsDevelopment() {
    std::string host = NGMP_DEFAULT_HOST;
    if (host == "localhost" || host == "127.0.0.1" ||
        host.rfind("192.168.", 0) == 0 ||
        host.rfind("10.", 0) == 0 ||
        host.rfind("172.16.", 0) == 0 ||
        host.rfind("172.17.", 0) == 0 ||
        host.rfind("172.18.", 0) == 0 ||
        host.rfind("172.19.", 0) == 0 ||
        host.rfind("172.20.", 0) == 0 ||
        host.rfind("172.21.", 0) == 0 ||
        host.rfind("172.22.", 0) == 0 ||
        host.rfind("172.23.", 0) == 0 ||
        host.rfind("172.24.", 0) == 0 ||
        host.rfind("172.25.", 0) == 0 ||
        host.rfind("172.26.", 0) == 0 ||
        host.rfind("172.27.", 0) == 0 ||
        host.rfind("172.28.", 0) == 0 ||
        host.rfind("172.29.", 0) == 0 ||
        host.rfind("172.30.", 0) == 0 ||
        host.rfind("172.31.", 0) == 0) {
        return true;
    }
    return false;
}

std::string GetServerEnv() {
    return IsDevelopment() ? "dev" : "live";
}

std::string GetServerWSEndpoint() {
    std::string port = NGMP_DEFAULT_PORT;
    if (IsSSLEnabled()) {
        if (port == "443" || port.empty()) {
            return "wss://" + std::string(NGMP_DEFAULT_HOST) + "/ws";
        }
        return "wss://" + std::string(NGMP_DEFAULT_HOST) + ":" + port + "/ws";
    }
    if (port == "80" || port.empty()) {
        return "ws://" + std::string(NGMP_DEFAULT_HOST) + "/ws";
    }
    return "ws://" + std::string(NGMP_DEFAULT_HOST) + ":" + port + "/ws";
}

std::string GetServerRESTEndpoint() {
    std::string port = NGMP_DEFAULT_PORT;
    if (IsSSLEnabled()) {
        if (port == "443" || port.empty()) {
            return "https://" + std::string(NGMP_DEFAULT_HOST);
        }
        return "https://" + std::string(NGMP_DEFAULT_HOST) + ":" + port;
    }
    if (port == "80" || port.empty()) {
        return "http://" + std::string(NGMP_DEFAULT_HOST);
    }
    return "http://" + std::string(NGMP_DEFAULT_HOST) + ":" + port;
}

std::string GetWebPortalURL() {
#if defined(NGMP_WEB_PORTAL_URL)
    std::string portalUrl = NGMP_WEB_PORTAL_URL;
    while (!portalUrl.empty() && portalUrl.back() == '/') {
        portalUrl.pop_back();
    }
    if (!portalUrl.empty()) {
        return portalUrl;
    }
#endif
    return GetServerRESTEndpoint();
}

std::string GetMOTDURL() {
    const char* envUrl = getenv("NGMP_MOTD_URL");
    if (envUrl && envUrl[0] != '\0') {
        return std::string(envUrl);
    }
#if defined(NGMP_MOTD_URL)
    std::string compileUrl = NGMP_MOTD_URL;
    if (!compileUrl.empty()) {
        return compileUrl;
    }
#endif
    return GetWebPortalURL() + "/motd.txt";
}

void FetchMOTD() {
    std::string url = GetMOTDURL();
    fprintf(stderr, "[NGMP] Fetching MOTD from %s...\n", url.c_str());
    fflush(stderr);

    CURL* curl = curl_easy_init();
    if (!curl) {
        return;
    }

    NGMP::Internal::CurlResponse resp;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NGMP::Internal::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "GeneralsX/" NGMP_CLIENT_ID);

    CURLcode res = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && httpCode == 200 && !resp.text.empty()) {
        fprintf(stderr, "[NGMP] MOTD fetched successfully (%zu bytes)\n", resp.text.size());
        fflush(stderr);
        if (TheGameSpyInfo) {
            TheGameSpyInfo->setMOTD(AsciiString(resp.text.c_str()));
        }
    } else {
        fprintf(stderr, "[NGMP] Failed to fetch MOTD (curl=%d, http=%ld)\n", res, httpCode);
        fflush(stderr);
    }
}

std::string GetAPIEndpoint(const char* szEndpoint) {
    return std::format("{}/env/{}/contract/" NGMP_CONTRACT_VERSION "/{}",
        GetServerRESTEndpoint(), GetServerEnv(), szEndpoint);
}

std::string GetBrowserLoginURL(const std::string& gamecode) {
    return std::format("{}/login/?gamecode={}", GetWebPortalURL(), gamecode);
}

std::string GetMatchViewURL(uint64_t matchId) {
    return std::format("{}/viewmatch?match={}", GetWebPortalURL(), matchId);
}

std::string GenerateGamecode() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);

    // Set version to 4 (UUIDv4): time_hi_and_version [bits 12-15] = 0100b
    part1 = (part1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    // Set variant to RFC 4122 (10xx): clk_seq_hi_res [bits 6-7] = 10b
    part2 = (part2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

    char buf[37];
    std::snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%04x-%012llx",
        static_cast<uint32_t>(part1 >> 32),
        static_cast<uint16_t>((part1 >> 16) & 0xFFFF),
        static_cast<uint16_t>(part1 & 0xFFFF),
        static_cast<uint16_t>(part2 >> 48),
        static_cast<unsigned long long>(part2 & 0xFFFFFFFFFFFFULL));
    return std::string(buf);
}

std::string UnicodeToUTF8(const UnicodeString& ustr) {
    if (ustr.isEmpty()) {
        return std::string();
    }
    size_t len = static_cast<size_t>(ustr.getLength());
    size_t utf8Len = ::Wide_To_Utf8_Len(reinterpret_cast<const wchar_t*>(ustr.str()), len);
    if (utf8Len == 0 || utf8Len == UTF8_INVALID) {
        return std::string();
    }
    std::string result(utf8Len, '\0');
    ::Wide_To_Utf8(&result[0], utf8Len + 1, reinterpret_cast<const wchar_t*>(ustr.str()), len);
    return result;
}

UnicodeString UTF8ToUnicode(const std::string& utf8Str) {
    if (utf8Str.empty()) {
        return UnicodeString();
    }
    size_t wideLen = ::Utf8_To_Wide_Len(utf8Str.c_str(), utf8Str.length());
    if (wideLen == 0 || wideLen == UTF8_INVALID) {
        UnicodeString u;
        u.translate(AsciiString(utf8Str.c_str()));
        return u;
    }
    std::vector<wchar_t> wideBuf(wideLen + 1, L'\0');
    ::Utf8_To_Wide(&wideBuf[0], wideLen + 1, utf8Str.c_str(), utf8Str.length());
    return UnicodeString(reinterpret_cast<const WideChar*>(&wideBuf[0]));
}

void OpenURL(const std::string& url) {
    if (url.empty()) {
        return;
    }
#if defined(_WIN32)
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
    SDL_OpenURL(url.c_str());
#endif
}

} // namespace NGMP

