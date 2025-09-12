#include "config_manager.h"
#include "ini_parser.h"
#include <iostream>
#include <filesystem>
#include <map>
#include <algorithm>

#ifdef _WIN32
#include <shlobj.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <pwd.h>
#include <unistd.h>
#elif defined(__linux__)
#include <pwd.h>
#include <unistd.h>
#endif

// Global instance
ConfigManager* g_configManager = nullptr;

ConfigManager::ConfigManager() : isZeroHour_(false), isDirty_(false) {
}

ConfigManager::~ConfigManager() {
    if (isDirty_) {
        save();
    }
}

bool ConfigManager::initialize(bool isZeroHour) {
    isZeroHour_ = isZeroHour;
    
    // Determine config file path
    std::string configDir = getConfigDirectory();
    configFilePath_ = configDir + "/" + (isZeroHour ? "cncgeneralszh.conf" : "cncgenerals.conf");
    
    // Create config directory if it doesn't exist
    std::filesystem::create_directories(configDir);
    
    // Load existing configuration
    bool loaded = loadFromFile();
    
    // If no config exists and we're on Windows, try to migrate from registry
    #ifdef _WIN32
    if (!loaded) {
        migrateFromRegistry();
    }
    #endif
    
    // Set default values if not present
    if (getString("Paths", "InstallPath", "").empty()) {
        setString("Paths", "InstallPath", getDefaultInstallPath());
    }
    
    if (getString("Paths", "UserDataPath", "").empty()) {
        setString("Paths", "UserDataPath", getDefaultUserDataPath());
    }
    
    // Save initial configuration
    return save();
}

std::string ConfigManager::getConfigDirectory() {
    #ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\CNC";
    }
    return ".";
    #elif defined(__APPLE__) || defined(__linux__)
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir) + "/.config";
    }
    return ".";
    #endif
}

std::string ConfigManager::getDefaultInstallPath() {
    #ifdef __APPLE__
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        std::string basePath = std::string(pw->pw_dir) + "/Library/Application Support/";
        return basePath + (isZeroHour_ ? "CNC_GeneralsZH" : "CNC_Generals");
    }
    #elif defined(__linux__)
    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        std::string basePath = std::string(pw->pw_dir) + "/Games/";
        return basePath + (isZeroHour_ ? "CNC_GeneralsZH" : "CNC_Generals");
    }
    #elif defined(_WIN32)
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
        return std::string(path) + "\\Command and Conquer Generals Data";
    }
    #endif
    
    return (isZeroHour_ ? "./CNC_GeneralsZH" : "./CNC_Generals");
}

std::string ConfigManager::getDefaultUserDataPath() {
    return getDefaultInstallPath(); // Same as install path for now
}

bool ConfigManager::getString(const std::string& section, const std::string& key, std::string& value) {
    auto sectionIt = sections_.find(section);
    if (sectionIt != sections_.end()) {
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            value = keyIt->second;
            return true;
        }
    }
    return false;
}

std::string ConfigManager::getString(const std::string& section, const std::string& key, const std::string& defaultValue) {
    std::string value;
    if (getString(section, key, value)) {
        return value;
    }
    return defaultValue;
}

bool ConfigManager::setString(const std::string& section, const std::string& key, const std::string& value) {
    sections_[section][key] = value;
    isDirty_ = true;
    return true;
}

bool ConfigManager::getUnsignedInt(const std::string& section, const std::string& key, unsigned int& value) {
    std::string strValue;
    if (getString(section, key, strValue)) {
        try {
            value = static_cast<unsigned int>(std::stoul(strValue));
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

unsigned int ConfigManager::getUnsignedInt(const std::string& section, const std::string& key, unsigned int defaultValue) {
    std::string strValue;
    if (getString(section, key, strValue)) {
        try {
            return static_cast<unsigned int>(std::stoul(strValue));
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool ConfigManager::setUnsignedInt(const std::string& section, const std::string& key, unsigned int value) {
    return setString(section, key, std::to_string(value));
}

std::string ConfigManager::registryPathToSection(const std::string& registryPath) {
    // Convert "SOFTWARE\Electronic Arts\EA Games\Generals" to "EA.Games.Generals"
    // Remove SOFTWARE\ prefix and replace \ with .
    std::string section = registryPath;
    
    // Remove SOFTWARE\ prefix
    if (section.find("SOFTWARE\\") == 0) {
        section = section.substr(9);
    }
    
    // Replace \ with .
    std::replace(section.begin(), section.end(), '\\', '.');
    
    return section;
}

bool ConfigManager::getStringFromRegistryPath(const std::string& registryPath, const std::string& key, std::string& value) {
    std::string section = registryPathToSection(registryPath);
    return getString(section, key, value);
}

bool ConfigManager::setStringInRegistryPath(const std::string& registryPath, const std::string& key, const std::string& value) {
    std::string section = registryPathToSection(registryPath);
    return setString(section, key, value);
}

bool ConfigManager::getUnsignedIntFromRegistryPath(const std::string& registryPath, const std::string& key, unsigned int& value) {
    std::string section = registryPathToSection(registryPath);
    std::string strValue;
    if (getString(section, key, strValue)) {
        try {
            value = static_cast<unsigned int>(std::stoul(strValue));
            return true;
        } catch (...) {
            return false;
        }
    }
    return false;
}

bool ConfigManager::setUnsignedIntInRegistryPath(const std::string& registryPath, const std::string& key, unsigned int value) {
    std::string section = registryPathToSection(registryPath);
    return setUnsignedInt(section, key, value);
}

std::string ConfigManager::getInstallPath() {
    return getString("Paths", "InstallPath", getDefaultInstallPath());
}

std::string ConfigManager::getUserDataPath() {
    return getString("Paths", "UserDataPath", getDefaultUserDataPath());
}

std::string ConfigManager::getConfigPath() {
    return configFilePath_;
}

bool ConfigManager::loadFromFile() {
    IniParser parser;
    if (parser.loadFromFile(configFilePath_)) {
        sections_ = parser.getSections();
        return true;
    }
    return false;
}

bool ConfigManager::saveToFile() {
    IniParser parser;
    
    // Copy our sections to the parser
    for (const auto& section : sections_) {
        for (const auto& keyValue : section.second) {
            parser.setValue(section.first, keyValue.first, keyValue.second);
        }
    }
    
    return parser.saveToFile(configFilePath_);
}

bool ConfigManager::save() {
    if (isDirty_) {
        bool success = saveToFile();
        if (success) {
            isDirty_ = false;
        }
        return success;
    }
    return true;
}

#ifdef _WIN32
void ConfigManager::migrateFromRegistry() {
    // TODO: Implement Windows Registry → INI migration
    // This would read existing registry values and convert them to INI format
    std::cout << "Registry migration not yet implemented" << std::endl;
}
#else
void ConfigManager::migrateFromRegistry() {
    // No-op on non-Windows platforms
}
#endif