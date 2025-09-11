#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>

/**
 * Cross-platform configuration manager that replaces Windows Registry
 * 
 * Design Philosophy:
 * - Unified INI-based configuration across all platforms
 * - Automatic migration from Windows Registry on first run
 * - Platform-specific paths but identical behavior
 * 
 * Usage:
 *   ConfigManager config;
 *   config.setString("Graphics", "ScreenWidth", "1920");
 *   std::string width = config.getString("Graphics", "ScreenWidth", "1024");
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Initialize configuration system for specific game
    bool initialize(bool isZeroHour = false);
    
    // String operations (replaces REG_SZ)
    bool getString(const std::string& section, const std::string& key, std::string& value);
    bool setString(const std::string& section, const std::string& key, const std::string& value);
    std::string getString(const std::string& section, const std::string& key, const std::string& defaultValue = "");
    
    // Integer operations (replaces REG_DWORD)
    bool getUnsignedInt(const std::string& section, const std::string& key, unsigned int& value);
    bool setUnsignedInt(const std::string& section, const std::string& key, unsigned int value);
    unsigned int getUnsignedInt(const std::string& section, const std::string& key, unsigned int defaultValue = 0);
    
    // Registry compatibility layer - maps registry paths to INI sections
    bool getStringFromRegistryPath(const std::string& registryPath, const std::string& key, std::string& value);
    bool setStringInRegistryPath(const std::string& registryPath, const std::string& key, const std::string& value);
    bool getUnsignedIntFromRegistryPath(const std::string& registryPath, const std::string& key, unsigned int& value);
    bool setUnsignedIntInRegistryPath(const std::string& registryPath, const std::string& key, unsigned int value);
    
    // Path resolution
    std::string getInstallPath();
    std::string getUserDataPath();
    std::string getConfigPath();
    
    // Save all changes to disk
    bool save();
    
private:
    std::map<std::string, std::map<std::string, std::string>> sections_;
    std::string configFilePath_;
    bool isZeroHour_;
    bool isDirty_;
    
    // Platform-specific path resolution
    std::string getConfigDirectory();
    std::string getDefaultInstallPath();
    std::string getDefaultUserDataPath();
    
    // INI file operations
    bool loadFromFile();
    bool saveToFile();
    
    // Registry migration (Windows only)
    void migrateFromRegistry();
    
    // Convert registry path to INI section
    std::string registryPathToSection(const std::string& registryPath);
};

// Global configuration manager instance
extern ConfigManager* g_configManager;

// Convenience functions for backward compatibility
inline bool GetStringFromRegistry(const std::string& path, const std::string& key, std::string& val) {
    return g_configManager->getStringFromRegistryPath(path, key, val);
}

inline bool GetUnsignedIntFromRegistry(const std::string& path, const std::string& key, unsigned int& val) {
    return g_configManager->getUnsignedIntFromRegistryPath(path, key, val);
}

inline bool SetStringInRegistry(const std::string& path, const std::string& key, const std::string& val) {
    return g_configManager->setStringInRegistryPath(path, key, val);
}

inline bool SetUnsignedIntInRegistry(const std::string& path, const std::string& key, unsigned int val) {
    return g_configManager->setUnsignedIntInRegistryPath(path, key, val);
}

#endif // CONFIG_MANAGER_H