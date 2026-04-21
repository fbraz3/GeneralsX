/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// FILE: UpdateChecker.cpp ////////////////////////////////////////////////
// GeneralsX @feature BenderAI 21/04/2026 Non-blocking update checker via
// GitHub Releases API. Only active for tagged release builds with SAGE_USE_SDL3.

#ifdef SAGE_UPDATE_CHECK

#include "Common/UpdateChecker.h"
#include "Common/GlobalData.h"

#include "gitinfo.h"

#include <SDL3/SDL.h>
#include <curl/curl.h>

#include <string.h>
#include <string>

// ---------------------------------------------------------------------------
// File-static implementation state (not exposed in the header to avoid
// leaking SDL3 includes to every consumer of UpdateChecker.h)
// ---------------------------------------------------------------------------
static SDL_Thread*   s_thread    = nullptr;
static SDL_AtomicInt s_done      = {0};
static SDL_AtomicInt s_hasUpdate = {0};
static char          s_latestTag[128] = {0};

// ---------------------------------------------------------------------------
// GitHub API endpoint for latest release
// ---------------------------------------------------------------------------
const char* UpdateChecker::getReleasesUrl()
{
    return "https://api.github.com/repos/fbraz3/GeneralsX/releases/latest";
}

// ---------------------------------------------------------------------------
// libcurl write callback: accumulates response body into a std::string*
// ---------------------------------------------------------------------------
static size_t curlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    std::string* body = static_cast<std::string*>(userdata);
    const size_t total = size * nmemb;
    // Guard against enormous responses (GitHub API is < 32 KB in practice)
    if (body->size() + total > 65536)
        return 0; // signal error to curl
    body->append(ptr, total);
    return total;
}

// ---------------------------------------------------------------------------
// Simple tag_name extractor: parses "tag_name": "VALUE" from JSON response.
// No external JSON library required.
// ---------------------------------------------------------------------------
static bool extractTagName(const std::string& json, char* outTag, int outTagSize)
{
    // Look for "tag_name":"VALUE" or "tag_name": "VALUE"
    const char* key = "\"tag_name\"";
    const char* pos = strstr(json.c_str(), key);
    if (!pos)
        return false;

    pos += strlen(key);

    // Skip whitespace and colon
    while (*pos == ' ' || *pos == '\t' || *pos == ':' || *pos == ' ')
        ++pos;

    if (*pos != '"')
        return false;
    ++pos; // skip opening quote

    int i = 0;
    while (*pos && *pos != '"' && i < outTagSize - 1)
    {
        outTag[i++] = *pos++;
    }
    outTag[i] = '\0';
    return i > 0;
}

// ---------------------------------------------------------------------------
// Background thread: performs HTTP GET and fills s_latestTag
// ---------------------------------------------------------------------------
static int SDLCALL threadFunc(void* /*userData*/)
{
    std::string responseBody;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        SDL_SetAtomicInt(&s_done, 1);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, UpdateChecker::getReleasesUrl());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "GeneralsX/update-checker");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);         // total timeout (s)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);  // connect timeout (s)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        // Network error: fail silently
        SDL_SetAtomicInt(&s_done, 1);
        return 0;
    }

    char latestTag[128] = {0};
    if (!extractTagName(responseBody, latestTag, sizeof(latestTag)))
    {
        SDL_SetAtomicInt(&s_done, 1);
        return 0;
    }

    // Compare with the current build tag.
    // When GENERALS_FORCE_UPDATE_CHECK is set, skip the empty-tag guard and
    // treat any non-empty response from GitHub as "update available" so the
    // button shows in dev builds without a real tag.
    const bool forceCheck = SDL_getenv("GENERALS_FORCE_UPDATE_CHECK") != nullptr;
    if (!forceCheck && GitTag[0] == '\0')
    {
        SDL_SetAtomicInt(&s_done, 1);
        return 0;
    }

    if (forceCheck ? (latestTag[0] != '\0') : strcmp(latestTag, GitTag) != 0)
    {
        // Latest tag differs from current build tag: update available
        strncpy(s_latestTag, latestTag, sizeof(s_latestTag) - 1);
        s_latestTag[sizeof(s_latestTag) - 1] = '\0';
        SDL_SetAtomicInt(&s_hasUpdate, 1);
    }

    SDL_SetAtomicInt(&s_done, 1);
    return 0;
}

// ---------------------------------------------------------------------------
// UpdateChecker::start
// ---------------------------------------------------------------------------
void UpdateChecker::start()
{
    // Only run once per session
    if (s_thread != nullptr)
        return;

    // Only run for tagged, clean release builds.
    // Set env var GENERALS_FORCE_UPDATE_CHECK=1 to bypass the tag guard (for testing).
    const bool forceCheck = SDL_getenv("GENERALS_FORCE_UPDATE_CHECK") != nullptr;
    if (!forceCheck && (GitTag[0] == '\0' || GitUncommittedChanges))
        return;

    // Respect the user opt-out setting
    if (TheGlobalData && !TheGlobalData->m_checkForUpdates)
        return;

    SDL_SetAtomicInt(&s_done, 0);
    SDL_SetAtomicInt(&s_hasUpdate, 0);
    s_latestTag[0] = '\0';

    s_thread = SDL_CreateThread(threadFunc, "UpdateChecker", nullptr);
    if (!s_thread)
    {
        // Thread creation failed; fail silently
        SDL_SetAtomicInt(&s_done, 1);
    }
}

// ---------------------------------------------------------------------------
// UpdateChecker::poll
// ---------------------------------------------------------------------------
bool UpdateChecker::poll(const char** outLatestTag)
{
    if (SDL_GetAtomicInt(&s_done) == 0)
        return false; // still running

    if (SDL_GetAtomicInt(&s_hasUpdate) == 0)
        return false; // done but no update

    if (outLatestTag)
        *outLatestTag = s_latestTag;

    return true;
}

#endif // SAGE_UPDATE_CHECK
