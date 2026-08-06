// GeneralsX @feature GeneralsOnline NGMP Helpers header
// Cross-platform abstraction for OS primitives, timing, and storage.

#ifndef NGMP_HELPERS_H
#define NGMP_HELPERS_H

#include <string>
#include <cstdint>

namespace NGMP {

// Default client identifier sent to the server during CheckLogin
#ifndef NGMP_CLIENT_ID
#define NGMP_CLIENT_ID "GeneralsXZH"
#endif

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

// Saves refresh token to local user storage
bool SaveRefreshToken(const std::string& token);

// Loads refresh token from local user storage
std::string LoadRefreshToken();

// Returns true if SSL (HTTPS/WSS) is enabled
bool IsSSLEnabled();

// Returns default server WS endpoint URL
std::string GetServerWSEndpoint();

// Returns default server REST endpoint URL (e.g. http://host:port)
std::string GetServerRESTEndpoint();

// Returns a full named API endpoint URL (e.g. /env/dev/contract/1/CheckLogin)
std::string GetAPIEndpoint(const char* szEndpoint);

// Returns the browser login URL for a given gamecode
std::string GetBrowserLoginURL(const std::string& gamecode);

// Generates a random 32-char alphanumeric gamecode
std::string GenerateGamecode();

} // namespace NGMP

#endif // NGMP_HELPERS_H
