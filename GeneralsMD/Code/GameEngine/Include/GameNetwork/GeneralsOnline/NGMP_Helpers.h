// GeneralsX @feature GeneralsOnline NGMP Helpers header
// Cross-platform abstraction for OS primitives, timing, and storage.

#ifndef NGMP_HELPERS_H
#define NGMP_HELPERS_H

#include <string>
#include <cstdint>

namespace NGMP {

// Returns time in milliseconds since application start using SDL3/chrono primitives
uint32_t GetTicks();

// Pauses execution for the specified milliseconds
void Delay(uint32_t ms);

// Returns absolute user storage directory path for GeneralsOnline data
std::string GetStoragePath();

// Saves authentication token to local user storage
bool SaveAuthToken(const std::string& token);

// Loads authentication token from local user storage
std::string LoadAuthToken();

// Returns true if SSL (HTTPS/WSS) is enabled
bool IsSSLEnabled();

// Returns default server WS endpoint URL
std::string GetServerWSEndpoint();

// Returns default server REST endpoint URL
std::string GetServerRESTEndpoint();

} // namespace NGMP

#endif // NGMP_HELPERS_H
