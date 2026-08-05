// GeneralsX @feature GeneralsOnline NGMP Helpers implementation
// Cross-platform OS abstraction using pure C++20 standard library.

#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <algorithm>

#ifndef NGMP_DEFAULT_HOST
#define NGMP_DEFAULT_HOST "localhost"
#endif

#ifndef NGMP_DEFAULT_PORT
#define NGMP_DEFAULT_PORT "9001"
#endif

namespace NGMP {

bool IsSSLEnabled() {
    const char* envSSL = std::getenv("NGMP_USE_SSL");
    if (envSSL) {
        std::string s(envSSL);
        return s == "ON" || s == "1" || s == "true" || s == "TRUE";
    }
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

std::string GetServerWSEndpoint() {
    const char* envHost = std::getenv("NGMP_DEFAULT_HOST");
    const char* envPort = std::getenv("NGMP_SERVER_PORT");
    std::string host = envHost ? envHost : NGMP_DEFAULT_HOST;
    std::string port = envPort ? envPort : NGMP_DEFAULT_PORT;

    if (IsSSLEnabled()) {
        return "wss://" + host + ":" + port + "/ws";
    }
    return "ws://" + host + ":" + port + "/ws";
}

std::string GetServerRESTEndpoint() {
    const char* envHost = std::getenv("NGMP_DEFAULT_HOST");
    const char* envPort = std::getenv("NGMP_SERVER_PORT");
    std::string host = envHost ? envHost : NGMP_DEFAULT_HOST;
    std::string port = envPort ? envPort : NGMP_DEFAULT_PORT;

    if (IsSSLEnabled()) {
        return "https://" + host + ":" + port + "/api";
    }
    return "http://" + host + ":" + port + "/api";
}

} // namespace NGMP
