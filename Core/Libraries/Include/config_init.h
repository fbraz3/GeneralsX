#ifndef CONFIG_INIT_H
#define CONFIG_INIT_H

/**
 * Configuration system initialization
 * This should be called early in the game startup process
 */

#include "config_manager.h"

inline bool InitializeConfigSystem(bool isZeroHour = false) {
    if (!g_configManager) {
        g_configManager = new ConfigManager();
    }
    
    return g_configManager->initialize(isZeroHour);
}

inline void ShutdownConfigSystem() {
    if (g_configManager) {
        delete g_configManager;
        g_configManager = nullptr;
    }
}

#endif // CONFIG_INIT_H