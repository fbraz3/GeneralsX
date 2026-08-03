// GeneralsX @feature GeneralsOnline NGMP Init implementation
// Handles initialization and teardown of NGMP Online Services backend components.

#include "GameNetwork/GeneralsOnline/OnlineServices_Manager.h"
#include "GameNetwork/GeneralsOnline/NGMP_Helpers.h"
#include <cstdio>

bool NGMP_OnlineServicesManager::init() {
    if (m_initialized) {
        return true;
    }

    fprintf(stderr, "[NGMP] Initializing NGMP Online Services (Endpoint: %s)\n", NGMP::GetServerWSEndpoint().c_str());
    fflush(stderr);

    // Auto load token if stored locally
    std::string savedToken = NGMP::LoadAuthToken();
    if (!savedToken.empty()) {
        loginWithToken(savedToken);
    }

    m_initialized = true;
    return true;
}

void NGMP_OnlineServicesManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    fprintf(stderr, "[NGMP] Shutting down NGMP Online Services\n");
    fflush(stderr);

    logout();
    m_initialized = false;
}
