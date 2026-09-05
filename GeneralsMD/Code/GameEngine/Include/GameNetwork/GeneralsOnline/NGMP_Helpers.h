// GeneralsX @feature GeneralsOnline NGMP Helpers header
// Cross-platform abstraction for OS primitives, timing, and storage.

#ifndef NGMP_HELPERS_H
#define NGMP_HELPERS_H

#include <string>
#include <cstdint>

#include "Common/UnicodeString.h"

namespace NGMP {

#ifndef NGMP_CLIENT_ID
#if defined(__APPLE__)
#define NGMP_CLIENT_ID "generalsx_macos"
#elif defined(__linux__)
#define NGMP_CLIENT_ID "generalsx_linux"
#elif defined(_WIN32)
#define NGMP_CLIENT_ID "generalsx_windows"
#else
#define NGMP_CLIENT_ID "gen_online_30hz"
#endif
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

// Returns true if running in a local development environment (localhost, 192.168.*, 10.*)
bool IsDevelopment();

// Returns server environment name ("dev" or "live") based on IsDevelopment()
std::string GetServerEnv();

// Returns web portal URL for OAuth login and web features (or defaults to GetServerRESTEndpoint())
std::string GetWebPortalURL();

// Returns the MOTD URL (from NGMP_MOTD_URL env or default ${GetWebPortalURL()}/motd.txt)
std::string GetMOTDURL();

// Fetches the MOTD from GetMOTDURL() and sets it into TheGameSpyInfo
void FetchMOTD();

// Returns a full named API endpoint URL (e.g. /env/{env}/contract/1/CheckLogin)
std::string GetAPIEndpoint(const char* szEndpoint);

// Returns the browser login URL for a given gamecode
std::string GetBrowserLoginURL(const std::string& gamecode);

// Returns the match viewer URL for a given match ID
std::string GetMatchViewURL(uint64_t matchId);

// Generates a UUIDv4 gamecode
std::string GenerateGamecode();

// Opens a URL in the default browser (cross-platform: ShellExecute on Windows, SDL_OpenURL on Linux/macOS)
void OpenURL(const std::string& url);

// UTF-8 conversion helpers for UnicodeString
std::string UnicodeToUTF8(const UnicodeString& ustr);
UnicodeString UTF8ToUnicode(const std::string& utf8Str);

} // namespace NGMP

#endif // NGMP_HELPERS_H

