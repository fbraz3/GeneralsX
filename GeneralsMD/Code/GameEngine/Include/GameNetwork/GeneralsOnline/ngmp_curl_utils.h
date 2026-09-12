// GeneralsX @feature GeneralsOnline Internal libcurl write callback utilities
// Shared by OnlineServices_Auth.cpp and OnlineServices_Manager.cpp to avoid code duplication.

#pragma once
#include <string>
#include <curl/curl.h>

namespace NGMP {
namespace Internal {

struct CurlResponse {
    std::string text;
};

inline size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    CurlResponse* resp = static_cast<CurlResponse*>(userp);
    resp->text.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

} // namespace Internal
} // namespace NGMP
