// GeneralsX @feature GeneralsOnline NGMP Helpers implementation
// Cross-platform OS abstraction using pure C++20 standard library.

#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>

#ifndef NGMP_DEFAULT_HOST
#define NGMP_DEFAULT_HOST "192.168.1.120"
#endif

#ifndef NGMP_DEFAULT_PORT
#define NGMP_DEFAULT_PORT "9001"
#endif

namespace NGMP {

static std::string GetResolvedHost() {
    // 1. Check runtime environment variable NGMP_SERVER_HOST
    const char* envHost = std::getenv("NGMP_SERVER_HOST");
    if (envHost && *envHost) {
        return std::string(envHost);
    }

    // 2. Check local file .ngmp-server-host in working directory
    std::ifstream file(".ngmp-server-host");
    if (file.is_open()) {
        std::string line;
        if (std::getline(file, line) && !line.empty()) {
            size_t first = line.find_first_not_of(" \t\r\n");
            size_t last = line.find_last_not_of(" \t\r\n");
            if (first != std::string::npos && last != std::string::npos) {
                return line.substr(first, (last - first + 1));
            }
        }
    }

    // 3. Fallback to CMake build-time definition
    return NGMP_DEFAULT_HOST;
}

static std::string GetResolvedPort() {
    const char* envPort = std::getenv("NGMP_SERVER_PORT");
    if (envPort && *envPort) {
        return std::string(envPort);
    }
    return NGMP_DEFAULT_PORT;
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
    std::string host = GetResolvedHost();
    std::string port = GetResolvedPort();
    return "ws://" + host + ":" + port + "/ws";
}

std::string GetServerRESTEndpoint() {
    std::string host = GetResolvedHost();
    std::string port = GetResolvedPort();
    return "http://" + host + ":" + port + "/api";
}

} // namespace NGMP
