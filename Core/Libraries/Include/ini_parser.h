#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <string>
#include <map>
#include <fstream>

/**
 * Lightweight INI file parser for cross-platform configuration
 * 
 * Supports standard INI format:
 * [Section]
 * key=value
 * key with spaces=value with spaces
 * 
 * Features:
 * - Section and key names are case-insensitive
 * - Supports comments with # or ;
 * - Handles quoted values
 * - Preserves original formatting when possible
 */
class IniParser {
public:
    IniParser();
    ~IniParser();
    
    // File operations
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename);
    
    // Value operations
    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "");
    void setValue(const std::string& section, const std::string& key, const std::string& value);
    
    unsigned int getUnsignedInt(const std::string& section, const std::string& key, unsigned int defaultValue = 0);
    void setUnsignedInt(const std::string& section, const std::string& key, unsigned int value);
    
    // Section operations
    bool hasSection(const std::string& section);
    bool hasKey(const std::string& section, const std::string& key);
    void removeKey(const std::string& section, const std::string& key);
    void removeSection(const std::string& section);
    
    // Access to internal structure for advanced operations
    const std::map<std::string, std::map<std::string, std::string>>& getSections() const { return sections_; }
    
    // Clear all data
    void clear();
    
private:
    std::map<std::string, std::map<std::string, std::string>> sections_;
    
    // Helper functions
    std::string trim(const std::string& str);
    std::string toLower(const std::string& str);
    std::string parseValue(const std::string& value);
    std::string escapeValue(const std::string& value);
    bool isComment(const std::string& line);
    bool isSection(const std::string& line, std::string& sectionName);
    bool isKeyValue(const std::string& line, std::string& key, std::string& value);
};

#endif // INI_PARSER_H