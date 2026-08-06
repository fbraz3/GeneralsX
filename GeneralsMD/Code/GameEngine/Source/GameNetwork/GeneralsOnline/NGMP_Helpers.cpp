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
#include <random>
#include <format>

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

std::string GetServerWSEndpoint() {
    if (IsSSLEnabled()) {
        return "wss://" + std::string(NGMP_DEFAULT_HOST) + ":" + std::string(NGMP_DEFAULT_PORT) + "/ws";
    }
    return "ws://" + std::string(NGMP_DEFAULT_HOST) + ":" + std::string(NGMP_DEFAULT_PORT) + "/ws";
}

std::string GetServerRESTEndpoint() {
    if (IsSSLEnabled()) {
        return "https://" + std::string(NGMP_DEFAULT_HOST) + ":" + std::string(NGMP_DEFAULT_PORT);
    }
    return "http://" + std::string(NGMP_DEFAULT_HOST) + ":" + std::string(NGMP_DEFAULT_PORT);
}

std::string GetAPIEndpoint(const char* szEndpoint) {
    return std::format("{}/env/" NGMP_SERVER_ENV "/contract/" NGMP_CONTRACT_VERSION "/{}",
        GetServerRESTEndpoint(), szEndpoint);
}

std::string GetBrowserLoginURL(const std::string& gamecode) {
    // Use the web portal on the server for browser-based OAuth login
    return std::format("{}/login/?gamecode={}", GetServerRESTEndpoint(), gamecode);
}

std::string GenerateGamecode() {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t max_index = sizeof(charset) - 2; // -2: skip null terminator

    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(static_cast<unsigned long>(seed));
    std::uniform_int_distribution<size_t> distribution(0, max_index);

    std::string result;
    result.reserve(32);
    for (int i = 0; i < 32; ++i) {
        result += charset[distribution(generator)];
    }
    return result;
}

} // namespace NGMP
