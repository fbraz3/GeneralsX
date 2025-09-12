#pragma once

#include <string>
#include <unordered_map>
#include <fstream>

/**
 * @brief Simple INI file parser for cross-platform configuration management
 * 
 * This class provides functionality to read and write INI format configuration files.
 * It supports:
 * - Section-based organization ([section_name])
 * - Key-value pairs (key=value)
 * - Comments (lines starting with # or ;)
 * - Case-insensitive section and key names
 * - Quoted values for strings containing spaces
 * - Type-safe getters and setters for common types
 */
class IniParser {
public:
    /**
     * @brief Constructs an empty INI parser
     */
    IniParser();
    
    /**
     * @brief Destructor
     */
    ~IniParser();
    
    /**
     * @brief Load configuration from an INI file
     * @param filename Path to the INI file to load
     * @return true if file was loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief Save current configuration to an INI file
     * @param filename Path to the INI file to save
     * @return true if file was saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filename);
    
    /**
     * @brief Get a string value from the configuration
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     * @param defaultValue Default value to return if key is not found
     * @return The value if found, otherwise the default value
     */
    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "");
    
    /**
     * @brief Set a string value in the configuration
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     * @param value Value to set
     */
    void setValue(const std::string& section, const std::string& key, const std::string& value);
    
    /**
     * @brief Get an unsigned integer value from the configuration
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     * @param defaultValue Default value to return if key is not found or invalid
     * @return The value if found and valid, otherwise the default value
     */
    unsigned int getUnsignedInt(const std::string& section, const std::string& key, unsigned int defaultValue = 0);
    
    /**
     * @brief Set an unsigned integer value in the configuration
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     * @param value Value to set
     */
    void setUnsignedInt(const std::string& section, const std::string& key, unsigned int value);
    
    /**
     * @brief Check if a section exists
     * @param section Section name (case-insensitive)
     * @return true if section exists, false otherwise
     */
    bool hasSection(const std::string& section);
    
    /**
     * @brief Check if a key exists in a section
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     * @return true if key exists in the section, false otherwise
     */
    bool hasKey(const std::string& section, const std::string& key);
    
    /**
     * @brief Remove a key from a section
     * @param section Section name (case-insensitive)
     * @param key Key name (case-insensitive)
     */
    void removeKey(const std::string& section, const std::string& key);
    
    /**
     * @brief Remove an entire section
     * @param section Section name (case-insensitive)
     */
    void removeSection(const std::string& section);
    
    /**
     * @brief Clear all configuration data
     */
    void clear();

private:
    // Internal storage: section_name -> (key_name -> value)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections_;
    
    // Helper methods
    std::string trim(const std::string& str);
    std::string toLower(const std::string& str);
    std::string parseValue(const std::string& value);
    std::string escapeValue(const std::string& value);
    bool isComment(const std::string& line);
    bool isSection(const std::string& line, std::string& sectionName);
    bool isKeyValue(const std::string& line, std::string& key, std::string& value);
};